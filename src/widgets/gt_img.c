/**
 * @file gt_img.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-11 15:03:35
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_img.h"

#if GT_CFG_ENABLE_IMG
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../core/gt_fs.h"
#include "../core/gt_img_decoder.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../core/gt_event.h"
#if GT_USE_IMG_TRANSFORM
#include "../extra/draw/gt_img_transform.h"
#endif

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_IMG
#define MY_CLASS    &gt_img_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_img_s {
    gt_obj_st base;
    char * src;             // img src, It can be a virt file name/path or an array of C
    _gt_img_dsc_st raw;     // raw data buffer draw directly
    gt_color_masking_st masking;
    gt_color_t border_color;
    uint8_t    border_width;
#if GT_USE_FILE_HEADER
    gt_file_header_param_st fh;
#endif
#if GT_USE_DIRECT_ADDR
    gt_addr_t addr;
#endif
#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    gt_direct_addr_custom_size_st custom_addr;
#endif
#if GT_USE_IMG_TRANSFORM
    uint16_t angle;         // rotation angle, unit 0.1 degree (0~3599)
    uint16_t zoom;          // scale: 256 = 1.0x
    uint16_t prev_angle;    // angle at last draw, used to detect motion for NN/bilinear switch
    int16_t  pivot_x;       // pivot x relative to image top-left (-1 = center)
    int16_t  pivot_y;       // pivot y relative to image top-left (-1 = center)
    /* Decoded pixel cache for transform — avoids re-decoding every frame */
    gt_color_t * transform_cache;   // decoded RGB pixels, w*h*sizeof(gt_color_t)
    gt_opa_t   * alpha_cache;       // decoded alpha channel, w*h bytes (NULL if no alpha)
    uint16_t cache_w;
    uint16_t cache_h;
    uint8_t  cache_is_rom : 1;      // 1 = transform_cache is a ROM/RAM pointer, do NOT free
#endif /* GT_USE_IMG_TRANSFORM */
}_gt_img_st;

/* static variables -----------------------------------------------------*/
static void _img_init_cb(gt_obj_st * obj);
static void _img_deinit_cb(gt_obj_st * obj);
static void _img_event_cb(struct gt_obj_s * obj, gt_event_st * e);

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_img_class = {
    ._init_cb      = _img_init_cb,
    ._deinit_cb    = _img_deinit_cb,
    ._event_cb     = _img_event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_img_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void _img_draw_border(gt_obj_st * obj) {
    _gt_img_st * style = (_gt_img_st *)obj;
    if (style->border_width == 0) { return; }
    gt_attr_rect_st dsc = {0};
    gt_graph_init_rect_attr(&dsc);
    dsc.reg.is_fill  = 0;
    dsc.bg_opa       = obj->opa;
    dsc.border_width = style->border_width;
    dsc.border_color = style->border_color;
    dsc.radius       = obj->radius;
    draw_bg(obj->draw_ctx, &dsc, &obj->area);
}

/**
 * @brief obj init img widget call back
 *
 * @param obj
 */
static void _img_init_cb(gt_obj_st * obj) {
    _gt_img_st * style = (_gt_img_st * )obj;
    bool is_val = false;
    gt_attr_rect_st dsc = {
        .bg_img_src = style->src,
        .bg_opa = obj->opa,
        .masking = &style->masking,
    };

#if GT_USE_FILE_HEADER
    dsc.file_header = gt_file_header_param_check_valid(&style->fh);
    if (dsc.file_header) { is_val = true; }
#endif

#if GT_USE_DIRECT_ADDR
    dsc.addr = style->addr;
    if (false == gt_hal_is_invalid_addr(dsc.addr)) { is_val = true; }
#endif

#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    dsc.custom_addr = &style->custom_addr;
    if (false == gt_hal_is_invalid_custom_size_addr(dsc.custom_addr)) { is_val = true; }
#endif

    if (false == is_val) {
        dsc.raw_img = style->src ? NULL : &style->raw;
    }

#if GT_USE_IMG_TRANSFORM
    bool need_transform = (style->angle != GT_ANGLE_NONE) || (style->zoom != GT_ZOOM_NONE);
    if (!need_transform) {
        /* No transform: clear cache and draw normally */
        if (style->transform_cache) {
            if (!style->cache_is_rom) { gt_mem_free(style->transform_cache); }
            style->transform_cache = NULL;
            style->cache_w = 0;
            style->cache_h = 0;
            style->cache_is_rom = 0;
        }
        if (style->alpha_cache) { gt_mem_free(style->alpha_cache); style->alpha_cache = NULL; }
        draw_bg_img(obj->draw_ctx, &dsc, &obj->area);
        _img_draw_border(obj);
        draw_focus(obj);
        return;
    }

    /* Transform path: build pixel cache on first call or after src change */
    if (!style->transform_cache) {
        bool is_rom = false;
        style->transform_cache = gt_img_decode_to_cache(&dsc, &style->cache_w, &style->cache_h,
                                                        &is_rom, &style->alpha_cache);
        style->cache_is_rom = is_rom ? 1 : 0;
        if (!style->transform_cache) {
            _img_draw_border(obj);
            draw_focus(obj);
            return;
        }
    }

    int16_t px = (style->pivot_x < 0) ? (int16_t)(style->cache_w / 2) : style->pivot_x;
    int16_t py = (style->pivot_y < 0) ? (int16_t)(style->cache_h / 2) : style->pivot_y;

    /* Use nearest-neighbor while rotating (fast), bilinear when stopped (smooth).
     * Threshold: 5 degrees (50 units of 0.1-degree) of change triggers NN mode. */
    uint16_t angle_delta = (uint16_t)((3600 + style->angle - style->prev_angle) % 3600);
    if (angle_delta > 1800) angle_delta = 3600 - angle_delta; /* shortest arc */
    bool use_nn = (angle_delta >= 50);
    style->prev_angle = style->angle;

    gt_img_transform_draw_cached(obj->draw_ctx, &obj->area, obj->opa,
                                 style->transform_cache, style->alpha_cache,
                                 style->cache_w, style->cache_h,
                                 style->angle, style->zoom, px, py, use_nn,
                                 &style->masking);
    _img_draw_border(obj);
#else
    draw_bg_img(obj->draw_ctx, &dsc, &obj->area);
    _img_draw_border(obj);
#endif /* GT_USE_IMG_TRANSFORM */
    draw_focus(obj);
}

/**
 * @brief obj deinit call back
 *
 * @param obj
 */
static void _img_deinit_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }

    _gt_img_st * style_p = (_gt_img_st * )obj;
    if (NULL != style_p->src) {
        gt_mem_free(style_p->src);
        style_p->src = NULL;
    }
#if GT_USE_IMG_TRANSFORM
    if (NULL != style_p->transform_cache) {
        if (!style_p->cache_is_rom) { gt_mem_free(style_p->transform_cache); }
        style_p->transform_cache = NULL;
    }
    if (NULL != style_p->alpha_cache) {
        gt_mem_free(style_p->alpha_cache);
        style_p->alpha_cache = NULL;
    }
#endif /* GT_USE_IMG_TRANSFORM */
}

static GT_ATTRIBUTE_RAM_TEXT void _resize_by_img(gt_obj_st * obj) {
    _gt_img_st * style = (_gt_img_st * )obj;
    _gt_img_info_st info = {0};
    gt_area_st area = obj->area;
    bool is_ok = false;

    if (GT_FS_RES_OK == gt_fs_read_img_wh(style->src, &area.w, &area.h) &&
        area.w > 0 && area.h > 0) {
        is_ok = true;
#if GT_USE_FILE_HEADER
    } else if (GT_FS_RES_OK == gt_fs_fh_read_img_wh(&style->fh, &area.w, &area.h)) {
        is_ok = true;
#endif
#if GT_USE_DIRECT_ADDR
    } else if (GT_FS_RES_OK == gt_fs_direct_addr_read_img_wh(style->addr, &area.w, &area.h)) {
        is_ok = true;
#endif
#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    } else if (GT_FS_RES_OK == gt_fs_direct_addr_custom_size_read_img_wh(&style->custom_addr, &area.w, &area.h)) {
        is_ok = true;
#endif
    } else if (style->raw.raw_p &&
        GT_FS_RES_OK == gt_img_decoder_get_info_raw(style->raw.raw_p, style->raw.raw_len, &info)) {
        area.w = info.w;
        area.h = info.h;
        is_ok = true;
    } else if(style->raw.img) {
        area.w = style->raw.header.w;
        area.h = style->raw.header.h;
        is_ok = true;
    } else if (style->src &&
        GT_RES_OK == gt_img_decoder_get_info(style->src, &info) &&
        info.w > 0 && info.h > 0) {
        area.w = info.w;
        area.h = info.h;
        is_ok = true;
    }
    if (false == is_ok) { return; }
    if (0 == area.w || 0 == area.h) {
        return;
    }
    gt_obj_size_change(obj, &area);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

/**
 * @brief obj event handler call back
 *
 * @param obj
 * @param e event
 */
static void _img_event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code_val = gt_event_get_code(e);

    switch(code_val) {
        case GT_EVENT_TYPE_DRAW_START:
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_UPDATE_VALUE:
            _resize_by_img(obj);
            break;

        case GT_EVENT_TYPE_DRAW_REDRAW: {
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;
        }
        case GT_EVENT_TYPE_INPUT_RELEASED: /* click event finish */
            // gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief create a img obj
 *
 * @param parent img's parent element
 * @return gt_obj_st* img obj
 */
gt_obj_st * gt_img_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    _gt_img_st * style = (_gt_img_st *)obj;
#if GT_USE_IMG_TRANSFORM
    style->angle   = GT_ANGLE_NONE;
    style->zoom    = GT_ZOOM_NONE;
    style->pivot_x = -1;  /* -1 means use image center */
    style->pivot_y = -1;
    style->cache_is_rom = 0;
#endif

#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&style->addr);
#endif

#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&style->fh);
#endif

#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    gt_hal_custom_size_addr_init(&style->custom_addr);
#endif
    return obj;
}

void gt_img_set_src(gt_obj_st * img, char * src)
{
    if (false == gt_obj_is_type(img, OBJ_TYPE)) {
        return ;
    }
    if (NULL == src) {
        return ;
    }
    _gt_img_st * style = (_gt_img_st *)img;
    if (NULL != style->src && !strcmp(src, style->src)) {
        return ;
    }
    gt_memset(&style->raw, 0, sizeof(_gt_img_dsc_st));

    uint16_t len = src == NULL ? 0 : strlen(src);
    style->src = style->src ? gt_mem_realloc(style->src, len + 1) : gt_mem_malloc(len + 1);
    if (NULL == style->src) {
        return ;
    }
    gt_memcpy(style->src, src, len);
    style->src[len] = '\0';

    /* Invalidate transform cache — src changed */
#if GT_USE_IMG_TRANSFORM
    if (style->transform_cache) {
        if (!style->cache_is_rom) { gt_mem_free(style->transform_cache); }
        style->transform_cache = NULL;
        style->cache_w = 0;
        style->cache_h = 0;
        style->cache_is_rom = 0;
    }
    if (style->alpha_cache) { gt_mem_free(style->alpha_cache); style->alpha_cache = NULL; }
#endif /* GT_USE_IMG_TRANSFORM */

    gt_img_dsc_raw_init(&style->raw);
#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&style->fh);
#endif

#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&style->addr);
#endif

    gt_event_send(img, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_img_set_raw_data(gt_obj_st * img, gt_color_img_raw_st * raw)
{
    if (false == gt_obj_is_type(img, OBJ_TYPE)) {
        return ;
    }
    if (NULL == raw->file_raw_p && NULL == raw->buffer && NULL == raw->opa) {
        return ;
    }
    _gt_img_st * style = (_gt_img_st *)img;
    if (style->src) {
        gt_mem_free(style->src);
        style->src = NULL;
    }
    gt_img_dsc_raw_set(&style->raw, raw);

    if (NULL == raw->file_raw_p && raw->buffer) {
        gt_obj_set_size(img, style->raw.header.w, style->raw.header.h);
    }

#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&style->fh);
#endif

#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&style->addr);
#endif

#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    gt_hal_custom_size_addr_init(&style->custom_addr);
#endif

    gt_event_send(img, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

#if GT_USE_FILE_HEADER
void gt_img_set_by_file_header(gt_obj_st * img, gt_file_header_param_st * fh)
{
    if (false == gt_obj_is_type(img, OBJ_TYPE)) {
        return ;
    }
    if (NULL == fh) {
        return ;
    }
    _gt_img_st * style = (_gt_img_st *)img;
    gt_memset(&style->raw, 0, sizeof(_gt_img_dsc_st));
    if (NULL != style->src) {
        gt_mem_free(style->src);
        style->src = NULL;
    }
    style->fh = *fh;
    gt_img_dsc_raw_init(&style->raw);
#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&style->addr);
#endif
#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    gt_hal_custom_size_addr_init(&style->custom_addr);
#endif

    gt_event_send(img, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

gt_file_header_param_st* gt_img_get_file_header_param(gt_obj_st * img)
{
    if (false == gt_obj_is_type(img, OBJ_TYPE)) {
        return NULL;
    }
    _gt_img_st * style = (_gt_img_st *)img;
    return &style->fh;
}
#endif

#if GT_USE_DIRECT_ADDR
void gt_img_set_by_direct_addr(gt_obj_st * img, gt_addr_t addr)
{
    if (false == gt_obj_is_type(img, OBJ_TYPE)) {
        return ;
    }
    _gt_img_st * style = (_gt_img_st *)img;
    gt_memset(&style->raw, 0, sizeof(_gt_img_dsc_st));
    if (NULL != style->src) {
        gt_mem_free(style->src);
        style->src = NULL;
    }
    gt_img_dsc_raw_init(&style->raw);
#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&style->fh);
#endif
#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    gt_hal_custom_size_addr_init(&style->custom_addr);
#endif
    style->addr = addr;

    gt_event_send(img, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

gt_addr_t gt_img_get_direct_addr(gt_obj_st * img)
{
    if (false == gt_obj_is_type(img, OBJ_TYPE)) {
        return 0;
    }
    _gt_img_st * style = (_gt_img_st *)img;
    return style->addr;
}
#endif

#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
void gt_img_set_by_custom_size_addr(gt_obj_st * img, gt_direct_addr_custom_size_st * dac)
{
    if (false == gt_obj_is_type(img, OBJ_TYPE)) {
        return ;
    }
    _gt_img_st * style = (_gt_img_st *)img;
    gt_memset(&style->raw, 0, sizeof(_gt_img_dsc_st));
    if (NULL != style->src) {
        gt_mem_free(style->src);
        style->src = NULL;
    }
#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&style->fh);
#endif
#if GT_USE_DIRECT_ADDR
    gt_hal_direct_addr_init(&style->addr);
#endif
    style->custom_addr = *dac;

    gt_event_send(img, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

gt_direct_addr_custom_size_st * gt_img_get_custom_size_addr(gt_obj_st * img)
{
    if (false == gt_obj_is_type(img, OBJ_TYPE)) {
        return NULL;
    }
    _gt_img_st * style = (_gt_img_st *)img;
    return &style->custom_addr;
}
#endif

char * gt_img_get_src(gt_obj_st * img)
{
    if (false == gt_obj_is_type(img, OBJ_TYPE)) {
        return NULL;
    }
    _gt_img_st * style = (_gt_img_st *)img;
    /** raw data mode src is NULL */
    return style->src;
}

uint16_t gt_img_get_width(gt_obj_st * img)
{
    if (false == gt_obj_is_type(img, OBJ_TYPE)) {
        return 0;
    }
    _gt_img_st * obj = (_gt_img_st * )img;
    _gt_img_info_st info = {0};
    if (obj->src) {
        return (GT_RES_OK == gt_img_decoder_get_info(obj->src, &info)) ? info.w : 0;
    }
    return (GT_RES_OK == gt_img_decoder_get_info_raw(obj->raw.raw_p, obj->raw.raw_len, &info)) ? info.w : 0;
}

uint16_t gt_img_get_height(gt_obj_st * img)
{
    if (false == gt_obj_is_type(img, OBJ_TYPE)) {
        return 0;
    }
    _gt_img_st * obj = (_gt_img_st * )img;
    _gt_img_info_st info = {0};
    if (obj->src) {
        return (GT_RES_OK == gt_img_decoder_get_info(obj->src, &info)) ? info.h : 0;
    }
    return (GT_RES_OK == gt_img_decoder_get_info_raw(obj->raw.raw_p, obj->raw.raw_len, &info)) ? info.h : 0;
}

gt_res_t gt_img_set_masking(gt_obj_st * img, gt_color_masking_st * masking)
{
    if (false == gt_obj_is_type(img, OBJ_TYPE)) {
        return GT_RES_FAIL;
    }
    _gt_img_st * style = (_gt_img_st * )img;
    if (NULL == masking) {
        return GT_RES_FAIL;
    }
    style->masking = *masking;
    gt_event_send(img, GT_EVENT_TYPE_DRAW_START, NULL);
    return GT_RES_OK;
}

#if GT_USE_IMG_TRANSFORM
void gt_img_set_angle(gt_obj_st * img, uint16_t angle)
{
    if (false == gt_obj_is_type(img, OBJ_TYPE)) { return; }
    _gt_img_st * style = (_gt_img_st *)img;
    style->angle = angle % 3600;
    gt_event_send(img, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_img_set_zoom(gt_obj_st * img, uint16_t zoom)
{
    if (false == gt_obj_is_type(img, OBJ_TYPE)) { return; }
    if (zoom == 0) { return; }
    _gt_img_st * style = (_gt_img_st *)img;
    style->zoom = zoom;
    gt_event_send(img, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_img_set_pivot(gt_obj_st * img, int16_t pivot_x, int16_t pivot_y)
{
    if (false == gt_obj_is_type(img, OBJ_TYPE)) { return; }
    _gt_img_st * style = (_gt_img_st *)img;
    style->pivot_x = pivot_x;
    style->pivot_y = pivot_y;
    gt_event_send(img, GT_EVENT_TYPE_DRAW_START, NULL);
}
#endif

void gt_img_set_border_width(gt_obj_st * img, uint8_t width)
{
    if (false == gt_obj_is_type(img, OBJ_TYPE)) { return; }
    _gt_img_st * style = (_gt_img_st *)img;
    style->border_width = width;
    gt_event_send(img, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_img_set_border_color(gt_obj_st * img, gt_color_t color)
{
    if (false == gt_obj_is_type(img, OBJ_TYPE)) { return; }
    _gt_img_st * style = (_gt_img_st *)img;
    style->border_color = color;
    gt_event_send(img, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_img_set_border_radius(gt_obj_st * img, uint16_t radius)
{
    if (false == gt_obj_is_type(img, OBJ_TYPE)) { return; }
    gt_obj_set_radius(img, radius);
}

#endif  /** #if GT_CFG_ENABLE_IMG */
/* end ------------------------------------------------------------------*/
