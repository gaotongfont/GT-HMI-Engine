/**
 * @file gt_switch.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-21 14:34:25
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_switch.h"

#if GT_CFG_ENABLE_SWITCH
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_SWITCH
#define MY_CLASS    &gt_switcher_class

/* private typedef ------------------------------------------------------*/
typedef struct {
    uint16_t width;
    uint16_t height;
}_gt_switcher_divider_line_st;

typedef struct _gt_switcher_s {
    gt_obj_st obj;
    gt_color_t color_act;
    gt_color_t color_ina;
    gt_color_t color_point;
    gt_color_t color_divider;
    _gt_switcher_divider_line_st divider;

    uint8_t state : 1;          /** true: open; false: close */
    uint8_t sw_type : 2;        /** @ref gt_switch_style_et */
    uint8_t divider_line : 1;   /** true: show middle divider line */
}_gt_switcher_st;


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_switcher_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = (_gt_deinit_cb_t)NULL,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_switcher_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static inline uint16_t _calc_diameter_by(gt_switch_style_et type, uint16_t height) {
    return height >> (GT_SWITCH_STYLE_RECT == type ? 3 : 1);
}

/**
 * @brief obj init switcher widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    _gt_switcher_st * style = (_gt_switcher_st * )obj;
    gt_color_t fg_color;
    // set default size
    if (obj->area.w == 0 || obj->area.h == 0) {
        obj->area.w = 48;
        obj->area.h = 24;
    }

    if (gt_obj_get_state(obj) == GT_STATE_PRESSED) {
        fg_color = style->color_act;
    }else{
        fg_color = style->color_ina;
    }

    gt_area_st box_area = obj->area;
    gt_attr_rect_st rect_attr;
    uint16_t space = box_area.h >> 3;
    uint16_t diameter = _calc_diameter_by(style->sw_type, box_area.h);
    gt_area_st area_axis = box_area;
    if (GT_SWITCH_STYLE_AXIS == style->sw_type) {
        area_axis.x += diameter;
        area_axis.w -= diameter << 1;
        area_axis.y += space << 1;
        area_axis.h -= space << 2;
    }

    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill   = 1;
    rect_attr.bg_color      = fg_color;
    rect_attr.border_color  = fg_color;
    rect_attr.radius        = diameter;
    rect_attr.border_width  = 0;
    rect_attr.bg_opa        = obj->opa;

    draw_bg(obj->draw_ctx, &rect_attr, &area_axis);

    /* circle icon */
    gt_area_st area_circle = box_area;
    area_circle.y += space >> 1;
    area_circle.h -= space;
    area_circle.w = area_circle.h;
    if (gt_obj_get_state(obj) == GT_STATE_PRESSED) {
        area_circle.x += box_area.w - area_circle.w - (space >> 1);
    }else{
        area_circle.x += space >> 1;
    }

    if (style->divider_line && GT_SWITCH_STYLE_AXIS != style->sw_type) {
        gt_area_st area_line = area_circle;
        if (style->divider.width) {
            area_line.w = style->divider.width;
        } else {
            area_line.w = (box_area.w >> 5) ? (box_area.w >> 5) : 2;
        }
        area_line.x = box_area.x + ((box_area.w - area_line.w) >> 1);
        if (style->divider.height) {
            area_line.h = style->divider.height;
        } else {
            area_line.h -= area_circle.h >> 2;
        }
        area_line.y += (area_circle.h - area_line.h) >> 1;
        rect_attr.radius = area_line.w >> 1;
        rect_attr.bg_color = style->color_divider;
        draw_bg(obj->draw_ctx, &rect_attr, &area_line);
    }

    /** circle */
    rect_attr.radius        = _calc_diameter_by(style->sw_type, area_circle.h);
    rect_attr.bg_color      = style->color_point;
    rect_attr.border_width  = 0;
    draw_bg(obj->draw_ctx, &rect_attr, &area_circle);

    // focus
    draw_focus(obj);
}

/**
 * @brief obj event handler call back
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code_val = gt_event_get_code(e);

    if (GT_EVENT_TYPE_DRAW_START == code_val) {
        gt_disp_invalid_area(obj);
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
    } else if (GT_EVENT_TYPE_INPUT_RELEASED == code_val) {
        if (GT_STATE_PRESSED == gt_obj_get_state(obj)) {
            gt_obj_set_state(obj, GT_STATE_NONE);
        } else {
            gt_obj_set_state(obj, GT_STATE_PRESSED);
        }
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
        gt_event_send(obj, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
    }
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief create a switcher obj
 *
 * @param parent switcher's parent element
 * @return gt_obj_st* switcher obj
 */
gt_obj_st * gt_switch_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    GT_CHECK_BACK_VAL(obj, NULL);
    _gt_switcher_st * style = (_gt_switcher_st * )obj;

    style->color_ina     = gt_color_hex(0xebeef5);
    style->color_act     = gt_color_hex(0x13ce66);
    style->color_point   = gt_color_hex(0xffffff);
    style->color_divider = gt_color_hex(0xdcdfe6);

    return obj;
}

void gt_switch_set_color_act(gt_obj_st * switcher, gt_color_t color)
{
    if (false == gt_obj_is_type(switcher, OBJ_TYPE)) {
        return;
    }
    _gt_switcher_st * style = (_gt_switcher_st * )switcher;
    style->color_act = color;
    gt_event_send(switcher, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_switch_set_color_ina(gt_obj_st * switcher, gt_color_t color)
{
    if (false == gt_obj_is_type(switcher, OBJ_TYPE)) {
        return;
    }
    _gt_switcher_st * style = (_gt_switcher_st * )switcher;
    style->color_ina = color;
    gt_event_send(switcher, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_switch_set_color_point(gt_obj_st * switcher, gt_color_t color)
{
    if (false == gt_obj_is_type(switcher, OBJ_TYPE)) {
        return;
    }
    _gt_switcher_st * style = (_gt_switcher_st * )switcher;
    style->color_point = color;
    gt_event_send(switcher, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_switch_set_color_divider(gt_obj_st * switcher, gt_color_t color)
{
    if (false == gt_obj_is_type(switcher, OBJ_TYPE)) {
        return;
    }
    _gt_switcher_st * style = (_gt_switcher_st * )switcher;
    style->color_divider = color;
    gt_event_send(switcher, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_switch_set_style(gt_obj_st * switcher, gt_switch_style_et sw_style)
{
    if (false == gt_obj_is_type(switcher, OBJ_TYPE)) {
        return;
    }
    _gt_switcher_st * style = (_gt_switcher_st * )switcher;
    style->sw_type = sw_style;
    gt_event_send(switcher, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_switch_set_div_line(gt_obj_st * switcher, bool is_div_line)
{
    if (false == gt_obj_is_type(switcher, OBJ_TYPE)) {
        return;
    }
    _gt_switcher_st * style = (_gt_switcher_st * )switcher;
    style->divider_line = is_div_line ? 1 : 0;
    gt_event_send(switcher, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_switch_set_div_line_size(gt_obj_st * switcher, uint16_t width, uint16_t height)
{
    if (false == gt_obj_is_type(switcher, OBJ_TYPE)) {
        return;
    }
    _gt_switcher_st * style = (_gt_switcher_st * )switcher;
    style->divider.width = width;
    style->divider.height = height;
    gt_event_send(switcher, GT_EVENT_TYPE_DRAW_START, NULL);
}

#endif  /** GT_CFG_ENABLE_SWITCH */
/* end ------------------------------------------------------------------*/
