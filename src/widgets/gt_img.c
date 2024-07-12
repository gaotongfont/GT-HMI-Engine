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
#include "string.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../core/gt_event.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_IMG
#define MY_CLASS    &gt_img_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_img_s {
    gt_obj_st obj;
    char * src;             // img src, It can be a virt file name/path or an array of C
    _gt_img_dsc_st raw;     // raw data buffer draw directly
#if GT_USE_FILE_HEADER
    gt_file_header_param_st fh;
#endif
}_gt_img_st;

/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_img_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_img_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static inline void _img_init_cb(gt_obj_st * obj) {
    _gt_img_st * style = (_gt_img_st * )obj;
    gt_attr_rect_st dsc = {
        .bg_img_src = style->src,
        .raw_img = style->src ? NULL : &style->raw,
        .bg_opa = obj->opa,
    };
#if GT_USE_FILE_HEADER
    dsc.file_header = gt_file_header_param_check_valid(&style->fh);
#endif

    /* start draw obj */
    draw_bg_img(obj->draw_ctx, &dsc, &obj->area);

    // focus
    draw_focus(obj , 0);
}
/**
 * @brief obj init img widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    _img_init_cb(obj);
}

/**
 * @brief obj deinit call back
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");
    GT_CHECK_BACK(obj);

    _gt_img_st * style_p = (_gt_img_st * )obj;
    if (NULL != style_p->src) {
        gt_mem_free(style_p->src);
        style_p->src = NULL;
    }
}

/**
 * @brief obj event handler call back
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);
    gt_area_st area;

    switch(code) {
        case GT_EVENT_TYPE_DRAW_START:
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_UPDATE_VALUE:
            area = obj->area;
            if (GT_FS_RES_FAIL == gt_fs_read_img_wh(gt_img_get_src(obj), &area.w, &area.h)) {
                return ;
            }
            if (0 == area.w || 0 == area.h) {
                return ;
            }
            gt_obj_size_change(obj, &area);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
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

#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&style->fh);
#endif

    gt_event_send(img, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_img_set_raw_data(gt_obj_st * img, gt_img_raw_st * raw)
{
    if (false == gt_obj_is_type(img, OBJ_TYPE)) {
        return ;
    }
    if (NULL == raw->buffer && NULL == raw->opa) {
        return ;
    }
    _gt_img_st * style = (_gt_img_st *)img;
    if (style->src) {
        gt_mem_free(style->src);
        style->src = NULL;
    }
    style->raw.img = (uint8_t * )raw->buffer;
    style->raw.alpha = (gt_opa_t * )raw->opa;
    style->raw.header.w = raw->width;
    style->raw.header.h = raw->height;
    style->raw.header.type = GT_IMG_DECODER_TYPE_RAM;
    style->raw.fill_color = raw->color;

    if (style->raw.img && style->raw.alpha) {
        style->raw.header.color_format = GT_IMG_CF_TRUE_COLOR_ALPHA;
    } else {
        style->raw.header.color_format = GT_IMG_CF_TRUE_COLOR;
    }

#if GT_USE_FILE_HEADER
    gt_file_header_param_init(&style->fh);
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
#if GT_USE_FILE_HEADER
    style->fh = *fh;
#endif

    gt_event_send(img, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
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
    if (GT_RES_OK != gt_img_decoder_get_info(obj->src, &info)) {
        return 0;
    }
    return info.w;
}

uint16_t gt_img_get_height(gt_obj_st * img)
{
    if (false == gt_obj_is_type(img, OBJ_TYPE)) {
        return 0;
    }
    _gt_img_st * obj = (_gt_img_st * )img;
    _gt_img_info_st info = {0};
    if (GT_RES_OK != gt_img_decoder_get_info(obj->src, &info)) {
        return 0;
    }
    return info.h;
}

#endif  /** #if GT_CFG_ENABLE_IMG */
/* end ------------------------------------------------------------------*/
