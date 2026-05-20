/**
 * @file gt_polygon.c
 * @author Feyoung
 * @brief Polygon widget: arbitrary convex/concave polygon with fill and stroke.
 *        Fill uses scanline algorithm (odd-even rule, no heap allocation).
 *        Hit-testing uses integer ray casting for precise touch area.
 * @version 0.1
 * @date 2026-05-19
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_polygon.h"

#if GT_CFG_ENABLE_POLYGON

#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../core/gt_style.h"
#include "../core/gt_event.h"
#include "../hal/gt_hal_indev.h"
#include "../others/gt_types.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_POLYGON
#define MY_CLASS    &_gt_polygon_class

#define _INIT_CAPACITY  8

/* private typedef ------------------------------------------------------*/
typedef struct _gt_polygon_s {
    gt_obj_st base;
    gt_point_st * points;
    uint16_t count;
    uint16_t capacity;
    gt_color_t fill_color;
    gt_opa_t fill_opa;
    gt_line_st line;
} _gt_polygon_st;

/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st _gt_polygon_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_polygon_st)
};

/* static functions -----------------------------------------------------*/

/* Update obj position/size from ALREADY bbox-relative vertices + current stroke margin.
 * Vertices are NOT modified here — they stay relative to the widget's top-left corner.
 * Called any time stroke width changes (or from _set_points_and_recalc after conversion). */
static GT_ATTRIBUTE_RAM_TEXT void _recalc_area(gt_obj_st * obj) {
    _gt_polygon_st * w = (_gt_polygon_st *)obj;
    if (0 == w->count || NULL == w->points) return;

    /* Vertices are bbox-relative: xmin/ymin are 0 (or close to 0) after conversion.
     * Recompute to get the true extent for size calculation. */
    int16_t xmin = w->points[0].x, xmax = w->points[0].x;
    int16_t ymin = w->points[0].y, ymax = w->points[0].y;
    for (uint16_t i = 1; i < w->count; i++) {
        if (w->points[i].x < xmin) xmin = w->points[i].x;
        if (w->points[i].x > xmax) xmax = w->points[i].x;
        if (w->points[i].y < ymin) ymin = w->points[i].y;
        if (w->points[i].y > ymax) ymax = w->points[i].y;
    }
    /* expand bounding box by half stroke width to avoid clipping */
    uint8_t const m = w->line.width ? (uint8_t)((w->line.width >> 1) + 1) : 0;
    /* Adjust obj origin so that relative (0,0) aligns with the stroke-expanded corner.
     * After initial conversion xmin==0, ymin==0, so dx/dy == -m (pure margin shift). */
    int16_t const dx = xmin - (int16_t)m;
    int16_t const dy = ymin - (int16_t)m;
    gt_obj_set_pos(obj,
        (int16_t)(obj->area.x + dx),
        (int16_t)(obj->area.y + dy));
    gt_obj_set_size(obj,
        (uint16_t)(xmax - xmin + 1 + (uint16_t)(m * 2)),
        (uint16_t)(ymax - ymin + 1 + (uint16_t)(m * 2)));
    /* Shift relative vertices to stay aligned with the new obj origin. */
    if (dx != 0 || dy != 0) {
        for (uint16_t i = 0; i < w->count; i++) {
            w->points[i].x -= dx;
            w->points[i].y -= dy;
        }
    }
}

/* Convert absolute screen coords to bbox-relative, then call _recalc_area.
 * Must be called ONCE after a fresh set of absolute points is loaded. */
static GT_ATTRIBUTE_RAM_TEXT void _set_points_and_recalc(gt_obj_st * obj) {
    _gt_polygon_st * w = (_gt_polygon_st *)obj;
    if (0 == w->count || NULL == w->points) return;

    /* Step 1: find absolute bounding box */
    int16_t xmin = w->points[0].x, xmax = w->points[0].x;
    int16_t ymin = w->points[0].y, ymax = w->points[0].y;
    for (uint16_t i = 1; i < w->count; i++) {
        if (w->points[i].x < xmin) xmin = w->points[i].x;
        if (w->points[i].x > xmax) xmax = w->points[i].x;
        if (w->points[i].y < ymin) ymin = w->points[i].y;
        if (w->points[i].y > ymax) ymax = w->points[i].y;
    }
    /* Step 2: place obj at (xmin-m, ymin-m) */
    uint8_t const m = w->line.width ? (uint8_t)((w->line.width >> 1) + 1) : 0;
    int16_t const abs_ox = xmin - (int16_t)m;
    int16_t const abs_oy = ymin - (int16_t)m;
    gt_obj_set_pos(obj, abs_ox, abs_oy);
    gt_obj_set_size(obj,
        (uint16_t)(xmax - xmin + 1 + (uint16_t)(m * 2)),
        (uint16_t)(ymax - ymin + 1 + (uint16_t)(m * 2)));
    /* Step 3: convert to relative coords (subtract obj origin, not xmin-m).
     * After this, relative vertex (0,0) == absolute (xmin-m, ymin-m) == obj top-left.
     * xmin relative = m, ymin relative = m (margin offset). */
    for (uint16_t i = 0; i < w->count; i++) {
        w->points[i].x -= abs_ox;
        w->points[i].y -= abs_oy;
    }
}

/** Integer ray-casting point-in-polygon test. No FPU required. */
static GT_ATTRIBUTE_RAM_TEXT bool _point_in_polygon(
    gt_point_st const * pts, uint16_t n,
    int16_t px, int16_t py)
{
    uint16_t crossings = 0;
    for (uint16_t i = 0; i < n; i++) {
        uint16_t j = (i + 1) % n;
        int16_t y0 = pts[i].y, y1 = pts[j].y;
        int16_t lo = GT_MIN(y0, y1);
        int16_t hi = GT_MAX(y0, y1);
        if (py < lo || py >= hi) continue;
        int32_t xi = (int32_t)pts[i].x +
                     (int32_t)(py - y0) * (int32_t)(pts[j].x - pts[i].x)
                     / (int32_t)(y1 - y0);
        if (xi > (int32_t)px) {
            crossings++;
        }
    }
    return (crossings & 1u) != 0u;
}

static void _init_cb(gt_obj_st * obj) {
    _gt_polygon_st * w = (_gt_polygon_st *)obj;
    if (w->count < 3 || NULL == w->points) return;

    gt_attr_polygon_st dsc = {
        .points     = w->points,
        .count      = w->count,
        .line       = w->line,
        .fill_color = w->fill_color,
        .fill_opa   = w->fill_opa,
    };
    gt_draw_polygon(obj->draw_ctx, &dsc, &obj->area);
}

static void _deinit_cb(gt_obj_st * obj) {
    _gt_polygon_st * w = (_gt_polygon_st *)obj;
    if (w->points) {
        gt_mem_free(w->points);
        w->points = NULL;
    }
    w->count = 0;
    w->capacity = 0;
}

static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);

    if (GT_EVENT_TYPE_INPUT_PRESSED == code) {
        _gt_polygon_st * w = (_gt_polygon_st *)obj;
        if (w->count >= 3 && NULL != w->points) {
            gt_indev_param_st * param = (gt_indev_param_st *)e->param;
            if (NULL != param) {
                /* pts are now bbox-relative coords; convert touch point to
                 * widget-local space by subtracting current obj->area origin. */
                if (!_point_in_polygon(w->points, w->count,
                                       (int16_t)(param->param.point.x - obj->area.x),
                                       (int16_t)(param->param.point.y - obj->area.y))) {
                    return;
                }
            }
        }
        gt_disp_invalid_area(obj);
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
        return;
    }
    if (GT_EVENT_TYPE_DRAW_START == code) {
        gt_disp_invalid_area(obj);
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
        return;
    }
    if (GT_EVENT_TYPE_UPDATE_VALUE == code) {
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
        return;
    }
}

/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_polygon_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) return obj;

    _gt_polygon_st * w = (_gt_polygon_st *)obj;
    w->points   = NULL;
    w->count    = 0;
    w->capacity = 0;
    w->fill_color = gt_color_white();
    w->fill_opa   = GT_OPA_COVER;
    w->line.color = gt_color_black();
    w->line.width = 0;
    w->line.opa   = GT_OPA_COVER;
    w->line.brush = GT_BRUSH_TYPE_ROUND;
    return obj;
}

void gt_polygon_set_points(gt_obj_st * obj, gt_point_st const * points, uint16_t count)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) return;
    _gt_polygon_st * w = (_gt_polygon_st *)obj;

    if (NULL != w->points) {
        gt_mem_free(w->points);
        w->points = NULL;
    }
    w->count = 0;
    w->capacity = 0;

    if (NULL == points || 0 == count) {
        gt_event_send(obj, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
        return;
    }

    w->points = (gt_point_st *)gt_mem_malloc(count * sizeof(gt_point_st));
    if (NULL == w->points) return;

    gt_memcpy(w->points, points, count * sizeof(gt_point_st));
    w->count    = count;
    w->capacity = count;

    _set_points_and_recalc(obj);
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_polygon_add_point(gt_obj_st * obj, int16_t x, int16_t y)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) return;
    _gt_polygon_st * w = (_gt_polygon_st *)obj;

    /* Convert existing relative vertices back to absolute before adding the new point,
     * so that _set_points_and_recalc can treat the entire array as absolute coords. */
    int16_t const ox = obj->area.x;
    int16_t const oy = obj->area.y;
    for (uint16_t i = 0; i < w->count; i++) {
        w->points[i].x += ox;
        w->points[i].y += oy;
    }

    if (w->count >= w->capacity) {
        uint16_t new_cap = (w->capacity < _INIT_CAPACITY)
                           ? _INIT_CAPACITY
                           : (uint16_t)(w->capacity * 2);
        gt_point_st * buf = (gt_point_st *)gt_mem_realloc(w->points,
                                new_cap * sizeof(gt_point_st));
        if (NULL == buf) {
            /* Restore relative coords on failure */
            for (uint16_t i = 0; i < w->count; i++) {
                w->points[i].x -= ox;
                w->points[i].y -= oy;
            }
            return;
        }
        w->points   = buf;
        w->capacity = new_cap;
    }

    w->points[w->count].x = x;
    w->points[w->count].y = y;
    w->count++;

    _set_points_and_recalc(obj);
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_polygon_clear_points(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) return;
    _gt_polygon_st * w = (_gt_polygon_st *)obj;

    if (NULL != w->points) {
        gt_mem_free(w->points);
        w->points = NULL;
    }
    w->count    = 0;
    w->capacity = 0;
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_polygon_set_fill_color(gt_obj_st * obj, gt_color_t color)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) return;
    _gt_polygon_st * w = (_gt_polygon_st *)obj;
    w->fill_color = color;
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_polygon_set_fill_opa(gt_obj_st * obj, gt_opa_t opa)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) return;
    _gt_polygon_st * w = (_gt_polygon_st *)obj;
    w->fill_opa = opa;
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_polygon_set_line_color(gt_obj_st * obj, gt_color_t color)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) return;
    _gt_polygon_st * w = (_gt_polygon_st *)obj;
    w->line.color = color;
    if (w->line.width > 0) _recalc_area(obj);
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_polygon_set_line_width(gt_obj_st * obj, uint8_t width)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) return;
    _gt_polygon_st * w = (_gt_polygon_st *)obj;
    w->line.width = width;
    _recalc_area(obj);
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_polygon_set_line_opa(gt_obj_st * obj, gt_opa_t opa)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) return;
    _gt_polygon_st * w = (_gt_polygon_st *)obj;
    w->line.opa = opa;
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

uint16_t gt_polygon_get_count(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) return 0;
    return ((_gt_polygon_st *)obj)->count;
}

gt_point_st const * gt_polygon_get_points(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) return NULL;
    return ((_gt_polygon_st *)obj)->points;
}

#endif  /** GT_CFG_ENABLE_POLYGON */

/* end ------------------------------------------------------------------*/
