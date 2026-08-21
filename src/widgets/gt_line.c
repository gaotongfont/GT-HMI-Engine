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
    gt_obj_st base;
    gt_area_st prev_pos;
    gt_attr_line_st line;
    gt_point_st * points;
    uint16_t point_count;
}_gt_line_st;


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_line_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_line_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static inline bool
_is_move_pos(gt_area_st * prev, gt_area_st * cur) {
    return prev->x != cur->x || prev->y != cur->y;
}

static void GT_ATTRIBUTE_RAM_TEXT
_move_pos(gt_obj_st * obj) {
    _gt_line_st * widget = (_gt_line_st * )obj;
    gt_attr_line_st * style = (gt_attr_line_st * )&widget->line;
    gt_point_st diff = {
        .x = obj->area.x - widget->prev_pos.x,
        .y = obj->area.y - widget->prev_pos.y,
    };
    style->start.x += diff.x;
    style->start.y += diff.y;
    style->end.x += diff.x;
    style->end.y += diff.y;
    for (uint16_t i = 0; i < widget->point_count; i++) {
        widget->points[i].x += diff.x;
        widget->points[i].y += diff.y;
    }
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

static void _reset_points(_gt_line_st * widget) {
    if (widget->points) {
        gt_mem_free(widget->points);
        widget->points = NULL;
    }
    widget->point_count = 0;
}

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
    /* brush is set by gt_line_set_brush_type(); do NOT override here */

    if (_is_move_pos(&widget->prev_pos, &obj->area)) {
        _move_pos(obj);
    }
    if (widget->point_count >= 2) {
        gt_attr_line_st segment = *style;
        gt_attr_point_st point = {
            .line = style->line,
        };

        /* Segment bodies have square caps. Each vertex is then painted once,
         * preventing two translucent round caps from accumulating at joints. */
        segment.line.brush = GT_BRUSH_TYPE_RECT;
        for (uint16_t i = 1; i < widget->point_count; i++) {
            segment.start = widget->points[i - 1];
            segment.end = widget->points[i];
            gt_draw_line(obj->draw_ctx, &segment, &obj->area);
        }
        point.line.brush = GT_BRUSH_TYPE_ROUND;
        for (uint16_t i = 0; i < widget->point_count; i++) {
            point.pos = widget->points[i];
            gt_draw_point(obj->draw_ctx, &point, &obj->area);
        }
    } else {
        gt_draw_line(obj->draw_ctx, style, &obj->area);
    }
    gt_memcpy(&widget->prev_pos, &obj->area, sizeof(gt_area_st));
}

static void _deinit_cb(gt_obj_st * obj) {
    _reset_points((_gt_line_st *)obj);
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
    _gt_line_st * widget = (_gt_line_st * )line;
    uint16_t const w = style->line.width;
    /* +1: endpoint pixel is inclusive; +w: half-width margin on both sides.
     * For round caps the circle radius <= half_w, so half_w margin is sufficient. */
    gt_size_t min_x = style->start.x;
    gt_size_t min_y = style->start.y;
    gt_size_t max_x = style->end.x;
    gt_size_t max_y = style->end.y;
    if (widget->point_count) {
        min_x = max_x = widget->points[0].x;
        min_y = max_y = widget->points[0].y;
        for (uint16_t i = 1; i < widget->point_count; i++) {
            min_x = GT_MIN(min_x, widget->points[i].x);
            min_y = GT_MIN(min_y, widget->points[i].y);
            max_x = GT_MAX(max_x, widget->points[i].x);
            max_y = GT_MAX(max_y, widget->points[i].y);
        }
    }
    line->area.x = min_x - (w >> 1);
    line->area.y = min_y - (w >> 1);
    line->area.w = max_x - min_x + w + 1;
    line->area.h = max_y - min_y + w + 1;

    gt_memcpy(&widget->prev_pos, &line->area, sizeof(gt_area_st));
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
    style->line.brush = GT_BRUSH_TYPE_ROUND;  /* default: round cap */
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
    _reset_points(widget);
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
    _reset_points(widget);
    style->end.x = xe;
    style->end.y = ye;

    _resize_area(line, style);
    gt_event_send(line, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

gt_res_t gt_line_set_points(gt_obj_st * line, gt_point_st const * points, uint16_t count)
{
    if (false == gt_obj_is_type(line, OBJ_TYPE) || NULL == points || count < 2) {
        return GT_RES_INV;
    }

    gt_point_st * new_points = gt_mem_malloc((uint32_t)count * sizeof(gt_point_st));
    if (NULL == new_points) {
        return GT_RES_INV;
    }
    gt_memcpy(new_points, points, (uint32_t)count * sizeof(gt_point_st));

    _gt_line_st * widget = (_gt_line_st *)line;
    gt_attr_line_st * style = (gt_attr_line_st *)&widget->line;
    _reset_points(widget);
    widget->points = new_points;
    widget->point_count = count;
    style->start = points[0];
    style->end = points[count - 1];

    _resize_area(line, style);
    gt_event_send(line, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
    return GT_RES_OK;
}

gt_res_t gt_line_get_start_point(gt_obj_st * line, gt_point_st * ret_p)
{
    if (false == gt_obj_is_type(line, OBJ_TYPE)) {
        return GT_RES_FAIL;
    }
    if (NULL == ret_p) {
        return GT_RES_FAIL;
    }
    _gt_line_st * widget = (_gt_line_st * )line;
    gt_attr_line_st * style = (gt_attr_line_st * )&widget->line;
    ret_p->x = style->start.x;
    ret_p->y = style->start.y;
    return GT_RES_OK;
}

gt_res_t gt_line_get_end_point(gt_obj_st * line, gt_point_st * ret_p)
{
    if (false == gt_obj_is_type(line, OBJ_TYPE)) {
        return GT_RES_FAIL;
    }
    if (NULL == ret_p) {
        return GT_RES_FAIL;
    }
    _gt_line_st * widget = (_gt_line_st * )line;
    gt_attr_line_st * style = (gt_attr_line_st * )&widget->line;
    ret_p->x = style->end.x;
    ret_p->y = style->end.y;
    return GT_RES_OK;
}

void gt_line_set_hor_line(gt_obj_st * line, uint16_t length)
{
    if (false == gt_obj_is_type(line, OBJ_TYPE)) {
        return ;
    }
    _gt_line_st * widget = (_gt_line_st * )line;
    gt_attr_line_st * style = (gt_attr_line_st * )&widget->line;
    _reset_points(widget);
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
    _reset_points(widget);
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
    _resize_area(line, style);
    gt_event_send(line, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_line_set_brush_type(gt_obj_st * line, uint8_t type)
{
    if (false == gt_obj_is_type(line, OBJ_TYPE)) {
        return;
    }
    _gt_line_st * widget = (_gt_line_st *)line;
    gt_attr_line_st * style = (gt_attr_line_st *)&widget->line;
    style->line.brush = type;
    gt_event_send(line, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_line_set_dash(gt_obj_st * line, uint8_t dash_width, uint8_t dash_gap)
{
    if (false == gt_obj_is_type(line, OBJ_TYPE)) { return; }
    _gt_line_st * widget = (_gt_line_st *)line;
    widget->line.line.dash_width = dash_width;
    widget->line.line.dash_gap   = dash_gap;
    gt_event_send(line, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

#endif  /** GT_CFG_ENABLE_LINE */
/* end ------------------------------------------------------------------*/
