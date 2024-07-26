/**
 * @file gt_graph.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-07-22 21:33:48
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_graphs.h"

#if GT_CFG_ENABLE_GRAPHS
#include "../core/gt_mem.h"
#include "../hal/gt_hal_disp.h"
#include "../others/gt_log.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"



/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_GRAPH
#define MY_CLASS    &gt_graphs_class


/* private typedef ------------------------------------------------------*/
typedef struct _gt_graphs_s {
    gt_obj_st obj;
    _gt_attr_graphs_st gp;
}_gt_graphs_st;

/* static variables -----------------------------------------------------*/
static void _graphs_init_cb(gt_obj_st * obj);
static void _graphs_deinit_cb(gt_obj_st * obj);
static void _graphs_event_cb(struct gt_obj_s * obj, gt_event_st * e);

static const gt_obj_class_st gt_graphs_class = {
    ._init_cb      = _graphs_init_cb,
    ._deinit_cb    = _graphs_deinit_cb,
    ._event_cb     = _graphs_event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_graphs_st)
};

static void _draw_axis(gt_obj_st * obj);
static void _draw_points_lines_curves(gt_obj_st * obj);



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _graphs_init_cb(gt_obj_st * obj) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill   = true;
    rect_attr.radius        = obj->radius;
    rect_attr.bg_opa        = obj->opa;
    // rect_attr.border_width  = style->border_width;
    rect_attr.fg_color      = obj->bgcolor;
    rect_attr.bg_color      = obj->bgcolor;
    // rect_attr.border_color  = style->color_border;

    gt_area_st area = gt_area_reduce(obj->area, gt_obj_get_reduce(obj));
    // 1:base shape
    draw_bg(obj->draw_ctx, &rect_attr, &area);

    /** draw axis */
    _draw_axis(obj);

    /** draw points / lines / curves */
    _draw_points_lines_curves(obj);
}

static void _free_series_lines(gt_series_points_st * series) {
    if (NULL == series) {
        return;
    }
    if (series->next_series_p) {
        _free_series_lines(series->next_series_p);
    }
    if (series->y_series) {
        gt_mem_free(series->y_series);
        series->y_series = NULL;
    }
    if (series->x_series) {
        gt_mem_free(series->x_series);
        series->x_series = NULL;
    }
    gt_mem_free(series->next_series_p);
    series->next_series_p = NULL;
}

static void _graphs_deinit_cb(gt_obj_st * obj) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    if (style->gp.series_head_p) {
        _free_series_lines(style->gp.series_head_p);
        gt_mem_free(style->gp.series_head_p);
        style->gp.series_head_p = NULL;
        style->gp.line_count = 0;
    }


}

static void _graphs_event_cb(struct gt_obj_s * obj, gt_event_st * e) {

}

static void _void_draw_grid_lines(gt_obj_st * obj, gt_point_st const * const padding, gt_point_st * points) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    gt_attr_line_st line_attr = { .line = style->gp.axis.grid };
    if (line_attr.line.opa < GT_OPA_MIN) {
        return;
    }
    gt_point_st grid_diff = {
        .x = gt_abs((style->gp.axis.hor.end - style->gp.axis.hor.start) / style->gp.axis.hor_unit),
        .y = gt_abs((style->gp.axis.ver.end - style->gp.axis.ver.start) / style->gp.axis.ver_unit),
    };
    if (0 == grid_diff.x) { grid_diff.x = 1; }  /** Avoid the denominator 0 */
    if (0 == grid_diff.y) { grid_diff.y = 1; }

    gt_point_st grid = {
        .x = (obj->area.w - (padding->x << 1)) / grid_diff.x,
        .y = (obj->area.h - (padding->y << 1)) / grid_diff.y,
    };

    grid_diff.x = grid_diff.x - 1;
    grid_diff.y = grid_diff.y - 1;

    line_attr.start = points[0];
    line_attr.start.x += obj->area.x;
    line_attr.start.y += obj->area.y;
    line_attr.end = points[1];
    line_attr.end.x += obj->area.x;
    line_attr.end.y += obj->area.y;
    /** draw hor grid's lines */
    for (gt_size_t i = 0; i < grid_diff.y; ++i) {
        line_attr.start.y += grid.y;
        line_attr.end.y += grid.y;
        gt_draw_lines(obj->draw_ctx, &line_attr, &obj->area);
    }

    line_attr.start = points[0];
    line_attr.start.x += obj->area.x;
    line_attr.start.y += obj->area.y;
    line_attr.end = points[3];
    line_attr.end.x += obj->area.x;
    line_attr.end.y += obj->area.y;
    /** draw ver grid's lines */
    for (gt_size_t i = 0; i < grid_diff.x; ++i) {
        line_attr.start.x += grid.x;
        line_attr.end.x += grid.x;
        gt_draw_lines(obj->draw_ctx, &line_attr, &obj->area);
    }
}

static void _draw_axis(gt_obj_st * obj) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    gt_point_st padding = { (gt_size_t)(obj->area.w * 0.2) >> 1, (gt_size_t)(obj->area.h * 0.2) >> 1 };

    gt_attr_line_st line_attr = { .line = style->gp.axis.scale };
    gt_point_st points[] = {
        { padding.x, padding.y }, { obj->area.w - padding.x, padding.y },
        { obj->area.w - padding.x, obj->area.h - padding.y }, { padding.x, obj->area.h - padding.y },
    };

    _void_draw_grid_lines(obj, &padding, points);

    if (line_attr.line.opa < GT_OPA_MIN) {
        return;
    }
    for (gt_size_t i = 0; i < 4; ++i) {
        line_attr.start = points[i];
        line_attr.start.x += obj->area.x;
        line_attr.start.y += obj->area.y;
        line_attr.end = points[(i + 1) % 4];
        line_attr.end.x += obj->area.x;
        line_attr.end.y += obj->area.y;
        gt_draw_lines(obj->draw_ctx, &line_attr, &obj->area);
    }
}

static void _draw_points_lines_curves(gt_obj_st * obj) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    gt_attr_point_st point_attr = { .line = style->gp.axis.scale };
    point_attr.pos.x = obj->area.x + 100;
    point_attr.pos.y = obj->area.y + 100;
    gt_draw_points(obj->draw_ctx, &point_attr, &obj->area);

#if 0
    /** test same point in line */
    gt_attr_line_st line_attr = { .line = style->gp.axis.scale };
    line_attr.start.x = obj->area.x + 50;
    line_attr.start.y = obj->area.y + 50;
    line_attr.end.x = obj->area.x + 50;
    line_attr.end.y = obj->area.y + 50;
    gt_draw_lines(obj->draw_ctx, &line_attr, &obj->area);
#endif
}

/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_graphs_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    style->gp.axis.scale.color = gt_color_red();
    style->gp.axis.scale.width = 8;
    style->gp.axis.scale.opa = GT_OPA_COVER;

    return obj;
}

void gt_graphs_set_axis(gt_obj_st * obj, gt_axis_st const * const axis)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    gt_memcpy(&style->gp.axis, axis, sizeof(gt_axis_st));
}

void gt_graphs_set_series_line_type(gt_obj_st * obj, gt_graphs_type_et type_style)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    style->gp.type = type_style;
}

gt_series_points_st * gt_graphs_add_series_line(gt_obj_st * obj, gt_line_st const * const line_style)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return NULL;
    }
    return NULL;    // FIX
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    uint16_t size = sizeof(gt_series_points_st);
    gt_series_points_st * series = gt_mem_malloc(size);
    GT_CHECK_BACK_VAL(series, NULL);
    gt_memset(series, 0, size);

    gt_memcpy(&series->line, line_style, sizeof(gt_line_st));

    ++style->gp.line_count;
    if (NULL == style->gp.series_head_p) {
        style->gp.series_head_p = series;
        return series;
    }
    gt_series_points_st * ptr = style->gp.series_head_p;
    while (ptr->next_series_p) { ptr = ptr->next_series_p; }
    ptr->next_series_p = series;
    return series;
}

gt_res_t gt_graphs_series_add_point(gt_obj_st * obj, gt_series_points_st const * series, gt_point_st const * const point)
{
    return GT_RES_OK;
}

gt_res_t gt_graphs_series_add_point_by(gt_obj_st * obj, uint16_t series_index, gt_point_st const * const point)
{
    return GT_RES_OK;
}

#endif  /** GT_CFG_ENABLE_GRAPHS */
/* end ------------------------------------------------------------------*/
