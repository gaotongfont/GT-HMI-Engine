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



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _graphs_init_cb(gt_obj_st * obj) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    /* base shape */
    GT_LOG_A("", ">> %x", style->gp.axis.color.full);
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

    /** draw point / line / curve */
}

static void _graphs_deinit_cb(gt_obj_st * obj) {

}

static void _graphs_event_cb(struct gt_obj_s * obj, gt_event_st * e) {

}

static void _draw_axis(gt_obj_st * obj) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    gt_point_st padding = { (gt_size_t)(obj->area.w * 0.2) >> 1, (gt_size_t)(obj->area.h * 0.2) >> 1 };

    gt_attr_line_st line = {
        .color = style->gp.axis.color,
        .width = style->gp.axis.width,
        .opa = GT_OPA_COVER,
    };
    gt_point_st points[] = {
        { padding.x, padding.y }, { obj->area.w - padding.x, padding.y },
        { padding.x, obj->area.h - padding.y }, { obj->area.w - padding.x, obj->area.h - padding.y },
    };

    for (gt_size_t i = 0; i < 4; ++i) {
        line.start = points[i];
        line.start.x += obj->area.x;
        line.start.y += obj->area.y;
        line.end = points[(i + 1) % 4];
        line.end.x += obj->area.x;
        line.end.y += obj->area.y;
        gt_draw_lines(obj->draw_ctx, &line, &obj->area);
    }
}

/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_graphs_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    style->gp.axis.color = gt_color_red();
    style->gp.axis.width = 8;

    return obj;
}

void gt_graphs_set_axis(gt_obj_st * obj, gt_axis_st const * const axis)
{
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    if (NULL == style) {
        return;
    }
    gt_memcpy(&style->gp.axis, axis, sizeof(gt_axis_st));
}

#endif  /** GT_CFG_ENABLE_GRAPHS */
/* end ------------------------------------------------------------------*/
