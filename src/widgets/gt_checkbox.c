/**
 * @file gt_checkbox.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-18 14:11:47
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_checkbox.h"

#if GT_CFG_ENABLE_CHECKBOX
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../core/gt_event.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_CHECKBOX
#define MY_CLASS    &gt_checkbox_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_checkbox_s {
    gt_obj_st   obj;
    char *      text;
    gt_color_t  font_color;
    gt_font_info_st font_info;
    uint8_t     space_x;
    uint8_t     space_y;

    gt_point_st font_point_offset;

}_gt_checkbox_st;

/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_checkbox_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_checkbox_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

/**
 * @brief obj init checkbox widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    _gt_checkbox_st * style = (_gt_checkbox_st * )obj;
    gt_font_st font = {
        .info     = style->font_info,
        .res      = NULL,
        .utf8     = style->text,
        .len      = strlen(style->text),

    };
    font.info.thick_en = style->font_info.thick_en == 0 ? style->font_info.size + 6: style->font_info.thick_en;
    font.info.thick_cn = style->font_info.thick_cn == 0 ? style->font_info.size + 6: style->font_info.thick_cn;

    // set default size
    if( obj->area.w == 0 || obj->area.h == 0){
        obj->area.w = style->font_info.size+4 + strlen(style->text)*11;
        obj->area.h = style->font_info.size+4;
    }

    /* base shape */
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.bg_opa = obj->opa;
    if (gt_obj_get_state(obj) == GT_STATE_PRESSED) {
        rect_attr.reg.is_fill = 1;
        rect_attr.border_width = 0;
        rect_attr.bg_color = gt_color_hex(0x409EFF);
        rect_attr.border_color = gt_color_hex(0x409EFF);
    } else {
        rect_attr.reg.is_fill = 1;
        rect_attr.border_width = 2;
        rect_attr.bg_color = gt_color_white();
        rect_attr.border_color = gt_color_hex(0x409EFF);
    }
    gt_area_st area = gt_area_reduce(obj->area , gt_obj_get_reduce(obj));

    gt_area_st area_box = area;
    area_box.w = style->font_info.size+4;
    area_box.h = area_box.w;
    // area_box.y = area.y + ((area.h - area_box.h) >> 1);
    area_box.x = area.x;

   /* base shape */
    draw_bg(obj->draw_ctx, &rect_attr, &area_box);

    /* font */
    gt_attr_font_st font_dsc = {
        .font           = &font,
        .space_x        = style->space_x,
        .space_y        = style->space_y,
        .font_color     = style->font_color,
        .align          = GT_ALIGN_NONE,
        .opa            = obj->opa,
    };
    area.x += (area_box.w + 2) + style->font_point_offset.x;
    if(area.x < area_box.x) {
        area.x =  area_box.x;
    }
    area.y += 2 + style->font_point_offset.y;
    if(area.y < area_box.y) {
        area.y =  area_box.y;
    }
    area.w = area.w > area_box.w ? (area.w - area_box.w) : 0;

    area.h = style->font_info.size;
    if(area.y + area.h > obj->area.y + obj->area.h){
        area.h = obj->area.y + obj->area.h - area.y;
    }

    font_dsc.logical_area = area;
    draw_text(obj->draw_ctx, &font_dsc, &area);

     // focus
    draw_focus(obj , 0);
}

/**
 * @brief obj deinit call back
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");
    if (NULL == obj) {
        return ;
    }

    _gt_checkbox_st * style_p = (_gt_checkbox_st * )obj;
    if (NULL != style_p->text) {
        gt_mem_free(style_p->text);
        style_p->text = NULL;
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
    switch(code) {
        case GT_EVENT_TYPE_DRAW_START:
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_RELEASED: /* click event finish */
            if (gt_obj_get_state(obj) != GT_STATE_NONE) {
                gt_obj_set_state(obj, GT_STATE_NONE);
            } else {
                gt_obj_set_state(obj, GT_STATE_PRESSED);
            }
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
}


/* global functions / API interface -------------------------------------*/

/**
 * @brief create a checkbox obj
 *
 * @param parent checkbox's parent element
 * @return gt_obj_st* checkbox obj
 */
gt_obj_st * gt_checkbox_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    _gt_checkbox_st * style = (_gt_checkbox_st * )obj;
    uint16_t size_byte = sizeof("checkbox");

    style->text = gt_mem_malloc(size_byte);
    GT_CHECK_BACK_VAL(style->text, obj);
    gt_memcpy(style->text, "checkbox", size_byte);

    gt_font_info_init(&style->font_info);
    style->font_color           = gt_color_black();
    style->space_x              = 0;
    style->space_y              = 0;

    return obj;
}


void gt_checkbox_set_text(gt_obj_st * checkbox, const char * fmt, ...)
{
    char buffer[8] = {0};
    va_list args;
    va_list args2;

    if (false == gt_obj_is_type(checkbox, OBJ_TYPE)) {
        return;
    }

    _gt_checkbox_st * style = (_gt_checkbox_st * )checkbox;
    va_start(args, fmt);
    va_copy(args2, args);
    uint16_t size = (NULL == fmt) ? 0 : (vsnprintf(buffer, sizeof(buffer), fmt, args) + 1);
    va_end(args);
    if (!size) {
        goto free_lb;
    }

    if (NULL == style->text) {
        style->text = gt_mem_malloc(size);
    } else if (size != strlen(style->text) + 1) {
        style->text = gt_mem_realloc(style->text, size);
    }
    if (NULL == style->text) {
        goto free_lb;
    }
    gt_memset(style->text, 0, size);

    va_start(args2, fmt);
    vsnprintf(style->text, size, fmt, args2);
    va_end(args2);

    gt_event_send(checkbox, GT_EVENT_TYPE_DRAW_START, NULL);

    return;

free_lb:
    va_end(args2);
}
char * gt_checkbox_get_text(gt_obj_st * checkbox)
{
    if (false == gt_obj_is_type(checkbox, OBJ_TYPE)) {
        return NULL;
    }
    _gt_checkbox_st * style = (_gt_checkbox_st * )checkbox;
    return style->text;
}

void gt_checkbox_set_font_family_cn(gt_obj_st * checkbox, gt_family_t family)
{
    if (false == gt_obj_is_type(checkbox, OBJ_TYPE)) {
        return;
    }
    _gt_checkbox_st * style = (_gt_checkbox_st * )checkbox;
    style->font_info.style_cn = family;
}

void gt_checkbox_set_font_family_en(gt_obj_st * checkbox, gt_family_t family)
{
    if (false == gt_obj_is_type(checkbox, OBJ_TYPE)) {
        return;
    }
    _gt_checkbox_st * style = (_gt_checkbox_st * )checkbox;
    style->font_info.style_en = family;
}

void gt_checkbox_set_font_family_fl(gt_obj_st * checkbox, gt_family_t family)
{
    if (false == gt_obj_is_type(checkbox, OBJ_TYPE)) {
        return;
    }
    _gt_checkbox_st * style = (_gt_checkbox_st * )checkbox;
    style->font_info.style_fl = family;
}

void gt_checkbox_set_font_family_numb(gt_obj_st * checkbox, gt_family_t family)
{
    if (false == gt_obj_is_type(checkbox, OBJ_TYPE)) {
        return;
    }
    _gt_checkbox_st * style = (_gt_checkbox_st * )checkbox;
    style->font_info.style_numb = family;
}

void gt_checkbox_set_font_size(gt_obj_st * checkbox, uint8_t size)
{
    _gt_checkbox_st * style = (_gt_checkbox_st * )checkbox;
    style->font_info.size = size;
}

void gt_checkbox_set_font_gray(gt_obj_st * checkbox, uint8_t gray)
{
    _gt_checkbox_st * style = (_gt_checkbox_st * )checkbox;
    style->font_info.gray = gray;
}

void gt_checkbox_set_font_color(gt_obj_st * checkbox, gt_color_t color)
{
    if (false == gt_obj_is_type(checkbox, OBJ_TYPE)) {
        return;
    }
    _gt_checkbox_st * style = (_gt_checkbox_st * )checkbox;
    style->font_color = color;
    gt_event_send(checkbox, GT_EVENT_TYPE_DRAW_START, NULL);
}
void gt_checkbox_set_font_thick_en(gt_obj_st * checkbox, uint8_t thick)
{
    if (false == gt_obj_is_type(checkbox, OBJ_TYPE)) {
        return;
    }
    _gt_checkbox_st * style = (_gt_checkbox_st * )checkbox;
    style->font_info.thick_en = thick;
}
void gt_checkbox_set_font_thick_cn(gt_obj_st * checkbox, uint8_t thick)
{
    if (false == gt_obj_is_type(checkbox, OBJ_TYPE)) {
        return;
    }
    _gt_checkbox_st * style = (_gt_checkbox_st * )checkbox;
    style->font_info.thick_cn = thick;
}

void gt_checkbox_set_font_encoding(gt_obj_st * checkbox, gt_encoding_et encoding)
{
    if (false == gt_obj_is_type(checkbox, OBJ_TYPE)) {
        return;
    }
    _gt_checkbox_st * style = (_gt_checkbox_st * )checkbox;
    style->font_info.encoding = encoding;
}

void gt_checkbox_set_space(gt_obj_st * checkbox, uint8_t space_x, uint8_t space_y)
{
    if (false == gt_obj_is_type(checkbox, OBJ_TYPE)) {
        return;
    }
    _gt_checkbox_st * style = (_gt_checkbox_st * )checkbox;
    style->space_x = space_x;
    style->space_y = space_y;
}

void gt_checkbox_set_font_point_offset_x(gt_obj_st * checkbox, gt_size_t x)
{
    if (false == gt_obj_is_type(checkbox, OBJ_TYPE)) {
        return;
    }
    _gt_checkbox_st * style = (_gt_checkbox_st * )checkbox;
    style->font_point_offset.x = x;
}

void gt_checkbox_set_font_point_offset_y(gt_obj_st * checkbox, gt_size_t y)
{
    if (false == gt_obj_is_type(checkbox, OBJ_TYPE)) {
        return;
    }
    _gt_checkbox_st * style = (_gt_checkbox_st * )checkbox;
    style->font_point_offset.y = y;
}

void gt_checkbox_set_font_point_offset(gt_obj_st * checkbox , gt_size_t x, gt_size_t y)
{
    if (false == gt_obj_is_type(checkbox, OBJ_TYPE)) {
        return;
    }
    gt_checkbox_set_font_point_offset_x( checkbox , x);
    gt_checkbox_set_font_point_offset_y( checkbox , y);
}


#endif  /** GT_CFG_ENABLE_CHECKBOX */
/* end ------------------------------------------------------------------*/
