/**
 * @file gt_line.c
 * @author Li
 * @brief
 * @version 0.1
 * @date 2023-01-05 16:28:01
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_line.h"

#if GT_CFG_ENABLE_LINE
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
#define OBJ_TYPE    GT_TYPE_LINE
#define MY_CLASS    &gt_line_class

/* private typedef ------------------------------------------------------*/

typedef struct _gt_line_s {
    gt_obj_st obj;
    gt_attr_line_st line;
}_gt_line_st;


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_line_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = (_gt_deinit_cb_t)NULL,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_line_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
/**
 * @brief obj init line widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    _gt_line_st * widget = (_gt_line_st * )obj;
    gt_attr_line_st * style = (gt_attr_line_st * )&widget->line;
    style->line.opa = obj->opa;
    style->line.type = GT_GRAPHS_TYPE_LINE;
    style->line.brush = GT_BRUSH_TYPE_ROUND;
    gt_draw_line(obj->draw_ctx, style, &obj->area);
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
    } else if (GT_EVENT_TYPE_UPDATE_VALUE == code_val) {
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
    }
}

static void _resize_area(gt_obj_st * line, gt_attr_line_st * style) {
    line->area.x = GT_MIN(style->start.x, style->end.x) - (style->line.width >> 1);
    line->area.y = GT_MIN(style->start.y, style->end.y) - (style->line.width >> 1);
    line->area.w = gt_abs(style->end.x - style->start.x) + style->line.width + 1;
    line->area.h = gt_abs(style->end.y - style->start.y) + style->line.width + 1;
}

/* global functions / API interface -------------------------------------*/
/**
 * @brief create a line obj
 *
 * @param parent line's parent element
 * @return gt_obj_st* line obj
 */
gt_obj_st * gt_line_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    _gt_line_st * widget = (_gt_line_st * )obj;
    gt_attr_line_st * style = (gt_attr_line_st * )&widget->line;

    style->line.width = 1;
    style->line.color = gt_color_black();
    return obj;
}

void gt_line_set_color(gt_obj_st * line, gt_color_t color)
{
    if (false == gt_obj_is_type(line, OBJ_TYPE)) {
        return ;
    }
    _gt_line_st * widget = (_gt_line_st * )line;
    gt_attr_line_st * style = (gt_attr_line_st * )&widget->line;
    style->line.color = color;
    gt_event_send(line, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_line_set_start_point(gt_obj_st * line, uint16_t xs, uint16_t ys)
{
    if (false == gt_obj_is_type(line, OBJ_TYPE)) {
        return ;
    }
    _gt_line_st * widget = (_gt_line_st * )line;
    gt_attr_line_st * style = (gt_attr_line_st * )&widget->line;
    style->start.x = xs;
    style->start.y = ys;

    _resize_area(line, style);
    gt_event_send(line, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_line_set_end_point(gt_obj_st * line, uint16_t xe, uint16_t ye)
{
    if (false == gt_obj_is_type(line, OBJ_TYPE)) {
        return ;
    }
    _gt_line_st * widget = (_gt_line_st * )line;
    gt_attr_line_st * style = (gt_attr_line_st * )&widget->line;
    style->end.x = xe;
    style->end.y = ye;

    _resize_area(line, style);
    gt_event_send(line, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_line_set_hor_line(gt_obj_st * line, uint16_t length)
{
    if (false == gt_obj_is_type(line, OBJ_TYPE)) {
        return ;
    }
    _gt_line_st * widget = (_gt_line_st * )line;
    gt_attr_line_st * style = (gt_attr_line_st * )&widget->line;
    style->start.x = line->area.x;
    style->start.y = line->area.y + (widget->line.line.width >> 1);
    style->end.x = line->area.x + length;
    style->end.y = style->start.y;

    _resize_area(line, style);
    gt_event_send(line, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_line_set_ver_line(gt_obj_st * line, uint16_t length)
{
    if (false == gt_obj_is_type(line, OBJ_TYPE)) {
        return ;
    }
    _gt_line_st * widget = (_gt_line_st * )line;
    gt_attr_line_st * style = (gt_attr_line_st * )&widget->line;
    style->start.x = line->area.x + (widget->line.line.width >> 1);
    style->start.y = line->area.y;
    style->end.x = line->area.x;
    style->end.y = style->start.y + length;

    _resize_area(line, style);
    gt_event_send(line, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_line_set_line_width(gt_obj_st * line, uint16_t line_width)
{
    if (false == gt_obj_is_type(line, OBJ_TYPE)) {
        return ;
    }
    _gt_line_st * widget = (_gt_line_st * )line;
    gt_attr_line_st * style = (gt_attr_line_st * )&widget->line;
    style->line.width = line_width;
    gt_event_send(line, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

#endif  /** GT_CFG_ENABLE_LINE */
/* end ------------------------------------------------------------------*/
