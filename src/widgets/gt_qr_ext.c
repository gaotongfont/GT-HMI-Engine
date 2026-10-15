/**
 * @file gt_qr_ext.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2026-01-05 11:08:52
 * @copyright Copyright (c) 2014-2026, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_qr_ext.h"
#if GT_CFG_ENABLE_QR_EXT
#include "../core/gt_mem.h"
#include "../hal/gt_hal_disp.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../core/gt_event.h"
#include "../others/gt_log.h"
#include "../extra/qrcodegen/qrcodegen.h"


/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_QR_EXT
#define MY_CLASS    &gt_qr_ext_class

//
#define _QR_EXT_VERSION_MIN                 (qrcodegen_VERSION_MIN)
#define _QR_EXT_VERSION_MAX                 (qrcodegen_VERSION_MAX)
#define _QR_EXT_BUFFER_SIZE_GET(version)    (qrcodegen_BUFFER_LEN_FOR_VERSION((version)))
/* private typedef ------------------------------------------------------*/
typedef struct _gt_qr_ext_s {
    gt_obj_st base;
    char* text;

    gt_color_t module_color;
    uint8_t module_size;
    uint8_t margin;
    uint8_t version;
    gt_qr_ecc_level_te ecc_level;
    gt_qr_mask_te       mask;
}_gt_qr_ext_st;


/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static void _qr_ext_init_cb(gt_obj_st * obj);
static void _qr_ext_deinit_cb(gt_obj_st * obj);
static void _qr_ext_event_cb(struct gt_obj_s * obj, gt_event_st * e);

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_qr_ext_class = {
    ._init_cb      = _qr_ext_init_cb,
    ._deinit_cb    = _qr_ext_deinit_cb,
    ._event_cb     = _qr_ext_event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_qr_ext_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static GT_ATTRIBUTE_RAM_TEXT
void _qr_ext_init_cb(gt_obj_st * obj)
{
    _gt_qr_ext_st * style = (_gt_qr_ext_st *)obj;

    uint8_t * qr_buf = NULL;
    uint8_t * tm_buf = NULL;
    uint32_t buf_len = _QR_EXT_BUFFER_SIZE_GET(style->version);
    if(buf_len <= 0) {
        GT_LOGE(GT_LOG_TAG_GUI, "buf_len <= 0");
        return ;
    }

    qr_buf = (uint8_t *)gt_mem_malloc(buf_len << 1);
    if(!qr_buf) {
        GT_LOGE(GT_LOG_TAG_GUI, "malloc failed!! len: %d", buf_len << 1);
        return ;
    }
    tm_buf = qr_buf + buf_len;

    bool flag = qrcodegen_encodeText(style->text, tm_buf, qr_buf,
                                    style->ecc_level, 1, style->version, style->mask, true);

    if(!flag) {
        GT_LOGE(GT_LOG_TAG_GUI, "encode failed!! text: <%s>", style->text);
        goto _free_buf;
    }
    int size = qrcodegen_getSize(qr_buf);
    if(size <= 0) {
        GT_LOGE(GT_LOG_TAG_GUI, "size <= 0");
        goto _free_buf;
    }
    size = size * style->module_size;
    uint32_t width = size + (style->margin << 1);
    uint16_t mag_w = style->margin, mag_h = style->margin;

    /* Auto-grow only when the user never set a size (area stays 0 from
     * create). Growing unconditionally would undo a later gt_obj_set_size()
     * shrink on every paint (P1-6). A user area smaller than the QR keeps
     * the QR centered; modules outside the area are clipped by the draw
     * pipeline (gt_area_intersect_get). */
    if(0 == obj->area.w) {
        obj->area.w = width;
    }

    if(0 == obj->area.h) {
        obj->area.h = width;
    }

    mag_w = (obj->area.w - size) >> 1;
    mag_h = (obj->area.h - size) >> 1;

    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill   = true;
    rect_attr.radius        = obj->radius;
    rect_attr.bg_opa        = obj->opa;
    rect_attr.border_width  = 0;
    rect_attr.fg_color      = obj->bgcolor;
    rect_attr.bg_color      = obj->bgcolor;

    if(obj->show_bg) {
        draw_bg(obj->draw_ctx, &rect_attr, &obj->area);
    }

    rect_attr.bg_color = style->module_color;

    gt_area_st md_area = {0};
    md_area.x = obj->area.x + mag_w;
    md_area.y = obj->area.y + mag_h;
    md_area.w = style->module_size;
    md_area.h = style->module_size;

    for(int x = 0; x < size; ++x) {
        for(int y = 0; y < size; ++y) {
            if(qrcodegen_getModule(qr_buf, x, y)) {
                draw_bg(obj->draw_ctx, &rect_attr, &md_area);
            }
            md_area.x += style->module_size;
        }
        md_area.x = obj->area.x + mag_w;
        md_area.y += style->module_size;
    }

_free_buf:
    if(qr_buf) {
        gt_mem_free(qr_buf);
        qr_buf = NULL;
        tm_buf = NULL;
    }
}

static GT_ATTRIBUTE_RAM_TEXT
void _qr_ext_deinit_cb(gt_obj_st * obj)
{
    _gt_qr_ext_st * style = (_gt_qr_ext_st *)obj;
    if(style->text) {
        gt_mem_free(style->text);
        style->text = NULL;
    }
}

static GT_ATTRIBUTE_RAM_TEXT
void _qr_ext_event_cb(struct gt_obj_s * obj, gt_event_st * e)
{
    gt_event_type_et code_val = gt_event_get_code(e);

    switch(code_val) {
        case GT_EVENT_TYPE_DRAW_START:
        case GT_EVENT_TYPE_UPDATE_STYLE:
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;
        default:
            break;
    }
}


/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_qr_ext_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    obj->radius = 0;
    obj->show_bg = true;
    obj->bgcolor = gt_color_white();

    _gt_qr_ext_st * style = (_gt_qr_ext_st *)obj;
    style->version = _QR_EXT_VERSION_MAX;
    style->ecc_level = GT_QR_ECC_LEVEL_HIGH;
    style->mask = GT_QR_MASK_0;
    style->margin = 2;
    style->module_size = 4;
    style->module_color = gt_color_black();

    return obj;
}

void gt_qr_ext_set_text_by_len(gt_obj_st * obj, const char * text, int len)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }

    if(!text || len <= 0) {
        return ;
    }
    _gt_qr_ext_st * style = (_gt_qr_ext_st *)obj;

    int old_len = style->text ? strlen(style->text) : 0;

    if(old_len == len && 0 == gt_memcmp(style->text, text, old_len)) {
        return ;
    }
    else if(old_len == len) {
        gt_memcpy(style->text, text, old_len);
        style->text[old_len] = '\0';
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
        return ;
    }

    char* old_text = NULL;
    if(style->text) {
        old_text = style->text;
    }

    char* new_text = (char *)gt_mem_malloc(len + 1);
    if(!new_text) {
        GT_LOGE(GT_LOG_TAG_GUI, "malloc failed!! len: %d", len);
        return ;
    }
    memset(new_text, 0, len + 1);
    gt_memcpy(new_text, text, len);
    new_text[len] = '\0';

    style->text = new_text;

    if(old_text) {
        gt_mem_free(old_text);
        old_text = NULL;
    }
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_qr_ext_set_text(gt_obj_st * obj, const char * fmt, ...)
{
    char buffer[8] = {0};
    va_list args;
    va_list args2;

    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }

    if (NULL == fmt) {
        return;
    }
    _gt_qr_ext_st * style = (_gt_qr_ext_st *)obj;

    va_start(args, fmt);
    va_copy(args2, args);
    uint16_t size = (NULL == fmt) ? 0 : (vsnprintf(buffer, sizeof(buffer), fmt, args));
    va_end(args);
    if (!size) {
        goto free_lb;
    }

    int old_len = style->text ? strlen(style->text) : 0;

    if(old_len == size && 0 == gt_memcmp(style->text, buffer, old_len)) {
        goto free_lb;
    }
    else if(old_len == size) {
        va_start(args2, fmt);
        vsnprintf(style->text,  size + 1, fmt, args2);
        va_end(args2);
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
        return ;
    }

    char* old_text = NULL;
    if(style->text) {
        old_text = style->text;
    }

    char* new_text = (char *)gt_mem_malloc(size + 1);
    if(!new_text) {
        GT_LOGE(GT_LOG_TAG_GUI, "malloc failed!! len: %d", size);
        return ;
    }
    memset(new_text, 0, size + 1);
    va_start(args2, fmt);
    vsnprintf(new_text, size + 1, fmt, args2);
    va_end(args2);

    style->text = new_text;

    if(old_text) {
        gt_mem_free(old_text);
        old_text = NULL;
    }

    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
    return ;

free_lb:
    va_end(args2);
}

void gt_qr_ext_set_module_size(gt_obj_st * obj, uint8_t size)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }

    _gt_qr_ext_st * style = (_gt_qr_ext_st *)obj;
    style->module_size = size;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_qr_ext_set_module_color(gt_obj_st * obj, gt_color_t color)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }

    _gt_qr_ext_st * style = (_gt_qr_ext_st *)obj;
    style->module_color = color;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_qr_ext_set_margin(gt_obj_st * obj, uint8_t margin)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }

    _gt_qr_ext_st * style = (_gt_qr_ext_st *)obj;
    style->margin = margin;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_qr_ext_set_version(gt_obj_st * obj, uint8_t version)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }

    if(version < _QR_EXT_VERSION_MIN || version > _QR_EXT_VERSION_MAX) {
        return ;
    }

    _gt_qr_ext_st * style = (_gt_qr_ext_st *)obj;
    style->version = version;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_qr_ext_set_ecc_level(gt_obj_st * obj, gt_qr_ecc_level_te ecc_level)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }

    _gt_qr_ext_st * style = (_gt_qr_ext_st *)obj;
    style->ecc_level = ecc_level;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_qr_ext_set_mask(gt_obj_st * obj, gt_qr_mask_te mask)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }

    _gt_qr_ext_st * style = (_gt_qr_ext_st *)obj;
    style->mask = mask;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}


/* end of file ----------------------------------------------------------*/
#endif //! GT_CFG_ENABLE_QR_EXT

