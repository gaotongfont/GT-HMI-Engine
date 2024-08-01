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


#define _GRAPHS_DEFAULT_PADDING_SIZE    20


/* private typedef ------------------------------------------------------*/
typedef struct _graphs_reg_s {
    uint8_t self_adaptive : 1;
    uint8_t show_ruler : 1;
}_graphs_reg_st;

typedef struct _gt_graphs_s {
    gt_obj_st obj;
    _gt_attr_graphs_st gp;
    _graphs_reg_st reg;
}_gt_graphs_st;

typedef struct _graph_css_s {
    gt_padding_st padding;
    gt_point_st range;
    gt_point_st total;
    gt_point_st start;
}_graph_css_st;

typedef void ( * _draw_ruler_pos_area_t)(gt_obj_st * , gt_padding_st const * const, gt_attr_font_st * );

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

static bool _is_self_adaptive(gt_obj_st * obj);
static void _draw_axis(gt_obj_st * obj);
static void _draw_points_lines_curves(gt_obj_st * obj);
static void _calc_self_adaptive_axis_range(gt_obj_st * obj);


/* macros ---------------------------------------------------------------*/
static inline bool _is_hor_range_zero(_gt_graphs_st const * const style) {
    return style->gp.axis.hor.end == style->gp.axis.hor.start;
}

static inline bool _is_ver_range_zero(_gt_graphs_st const * const style) {
    return style->gp.axis.ver.end == style->gp.axis.ver.start;
}



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _graphs_init_cb(gt_obj_st * obj) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill   = true;
    rect_attr.radius        = obj->radius;
    rect_attr.bg_opa        = obj->opa;
    rect_attr.fg_color      = obj->bgcolor;
    rect_attr.bg_color      = obj->bgcolor;

    gt_area_st area = gt_area_reduce(obj->area, gt_obj_get_reduce(obj));
    /** base shape */
    draw_bg(obj->draw_ctx, &rect_attr, &area);

    if (_is_self_adaptive(obj) || _is_hor_range_zero(style) || _is_ver_range_zero(style)) {
        _calc_self_adaptive_axis_range(obj);
    }

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
        gt_mem_free(series->next_series_p);
        series->next_series_p = NULL;
    }
    if (series->y_series) {
        gt_mem_free(series->y_series);
        series->y_series = NULL;
    }
    if (series->x_series) {
        gt_mem_free(series->x_series);
        series->x_series = NULL;
    }
}

static void _graphs_deinit_cb(gt_obj_st * obj) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    if (style->gp.series_head_p) {
        _free_series_lines(style->gp.series_head_p);
        gt_mem_free(style->gp.series_head_p);
        style->gp.series_head_p = NULL;
        style->gp.count_line = 0;
    }

}

static void _graphs_event_cb(struct gt_obj_s * obj, gt_event_st * e) {

}

static bool _is_self_adaptive(gt_obj_st * obj) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    return style->reg.self_adaptive;
}

static bool _is_show_ruler(gt_obj_st * obj) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    return style->reg.show_ruler;
}

static gt_point_st _get_display_pixel_range(gt_padding_st const * const padding, uint16_t w, uint16_t h) {
    gt_point_st ret = { .x = w - padding->left - padding->right, .y = h - padding->top - padding->bottom };
    return ret;
}

static gt_point_st _add_obj_offset(gt_point_st const * const p, gt_obj_st * obj) {
    gt_point_st ret = { .x = p->x + obj->area.x, .y = p->y + obj->area.y };
    return ret;
}

static gt_padding_st _add_obj_offset_padding(gt_padding_st const * const p, gt_obj_st * obj) {
    gt_padding_st ret = {
        .left = p->left + obj->area.x,
        .right = p->right,
        .top = p->top + obj->area.y,
        .bottom = p->bottom,
    };
    return ret;
}

static void _update_axis_range(gt_range_st * dir_range, gt_size_t min, gt_size_t max, uint16_t redundancy) {
    dir_range->start = min - redundancy;
    dir_range->end   = max + redundancy;
}

static void _calc_self_adaptive_axis_range(gt_obj_st * obj) {
    gt_point_st min = { 0x7fff, 0x7fff }, max = { -0x7fff, -0x7fff };
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    gt_series_points_st * ptr = style->gp.series_head_p;
    uint16_t max_count = 0;

    while (ptr) {
        for (gt_size_t i = 0; i < ptr->count_point; ++i) {
            min.x = GT_MIN(min.x, ptr->x_series[i]);
            min.y = GT_MIN(min.y, ptr->y_series[i]);
            max.x = GT_MAX(max.x, ptr->x_series[i]);
            max.y = GT_MAX(max.y, ptr->y_series[i]);
        }
        if (ptr->count_point > max_count) {
            max_count = ptr->count_point;
        }
        ptr = ptr->next_series_p;
    }
    /** set default unit */
    if (0 == style->gp.axis.hor_unit) {
        style->gp.axis.hor_unit = ((max.x - min.x) + max_count) / (max_count + 1);
    }
    if (0 == style->gp.axis.ver_unit) {
        style->gp.axis.ver_unit = ((max.y - min.y) + max_count) / (max_count + 1);
    }
    /** add redundancy */
    uint16_t hor_redundancy = ((max.x - min.x) + style->gp.axis.hor_unit - 1) / style->gp.axis.hor_unit;
    uint16_t ver_redundancy = ((max.y - min.y) + style->gp.axis.ver_unit - 1) / style->gp.axis.ver_unit;

    if (_is_self_adaptive(obj)) {
        _update_axis_range(&style->gp.axis.hor, min.x, max.x, hor_redundancy);
        _update_axis_range(&style->gp.axis.ver, min.y, max.y, ver_redundancy);
        return;
    }

    if (_is_hor_range_zero(style)) {
        _update_axis_range(&style->gp.axis.hor, min.x, max.x, hor_redundancy);
    }
    if (_is_ver_range_zero(style)) {
        _update_axis_range(&style->gp.axis.ver, min.y, max.y, ver_redundancy);
    }
}

static gt_point_st _calc_div_part(_gt_graphs_st * style) {
    gt_point_st div_part = {
        .x = gt_abs((style->gp.axis.hor.end - style->gp.axis.hor.start) / style->gp.axis.hor_unit),
        .y = gt_abs((style->gp.axis.ver.end - style->gp.axis.ver.start) / style->gp.axis.ver_unit),
    };
    if (0 == div_part.x) { div_part.x = 1; }  /** Avoid the denominator 0 */
    if (0 == div_part.y) { div_part.y = 1; }
    return div_part;
}

static void _void_draw_grid_lines(gt_obj_st * obj, gt_padding_st const * const padding, gt_point_st * points) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    gt_attr_line_st line_attr = { .line = style->gp.axis.grid };
    if (line_attr.line.opa < GT_OPA_MIN) {
        return;
    }
    gt_point_st div_part = _calc_div_part(style);

    gt_point_st grid = _get_display_pixel_range(padding, obj->area.w, obj->area.h);
    grid.x /= div_part.x;
    grid.y /= div_part.y;

    div_part.x = div_part.x - 1;
    div_part.y = div_part.y - 1;

    line_attr.start = _add_obj_offset(&points[0], obj);
    line_attr.end = _add_obj_offset(&points[1], obj);
    /** draw hor grid's lines */
    for (gt_size_t i = 0; i < div_part.y; ++i) {
        line_attr.start.y += grid.y;
        line_attr.end.y += grid.y;
        gt_draw_line(obj->draw_ctx, &line_attr, &obj->area);
    }

    line_attr.start = _add_obj_offset(&points[0], obj);
    line_attr.end = _add_obj_offset(&points[3], obj);
    /** draw ver grid's lines */
    for (gt_size_t i = 0; i < div_part.x; ++i) {
        line_attr.start.x += grid.x;
        line_attr.end.x += grid.x;
        gt_draw_line(obj->draw_ctx, &line_attr, &obj->area);
    }
}

static gt_padding_st _get_padding_by(gt_obj_st * obj) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    gt_padding_st padding = {
        .left = style->reg.show_ruler ? 60 : _GRAPHS_DEFAULT_PADDING_SIZE,
        .right = _GRAPHS_DEFAULT_PADDING_SIZE,
        .top = _GRAPHS_DEFAULT_PADDING_SIZE,
        .bottom = style->reg.show_ruler ? 40 : _GRAPHS_DEFAULT_PADDING_SIZE,
    };
    /** calc real right and bottom padding */
    gt_point_st div_part = _calc_div_part(style);
    gt_point_st grid = _get_display_pixel_range(&padding, obj->area.w, obj->area.h);

    /** resize really graphs size */
    padding.right += grid.x - (grid.x / div_part.x) * div_part.x;
    padding.bottom += grid.y - (grid.y / div_part.y) * div_part.y;

    return padding;
}

static void _draw_ruler_ver_start(gt_obj_st * obj, gt_padding_st const * const padding, gt_attr_font_st * font_attr) {
    font_attr->logical_area.x = obj->area.x + 6;
    font_attr->logical_area.y = obj->area.y + padding->top - 6;
    font_attr->logical_area.w = padding->left - 6;
    font_attr->logical_area.h = 20;
}

static void _draw_ruler_ver_end(gt_obj_st * obj, gt_padding_st const * const padding, gt_attr_font_st * font_attr) {
    font_attr->logical_area.x = obj->area.x + 6;
    font_attr->logical_area.y = obj->area.y + obj->area.h - padding->bottom - 6;
    font_attr->logical_area.w = padding->left - 6;
    font_attr->logical_area.h = 20;
}

static void _draw_ruler_hor_start(gt_obj_st * obj, gt_padding_st const * const padding, gt_attr_font_st * font_attr) {
    font_attr->logical_area.x = obj->area.x + padding->left - 6;
    font_attr->logical_area.y = obj->area.y + obj->area.h - padding->bottom + 10;
    font_attr->logical_area.w = padding->left - 6;
    font_attr->logical_area.h = 20;
}

static void _draw_ruler_hor_end(gt_obj_st * obj, gt_padding_st const * const padding, gt_attr_font_st * font_attr) {
    font_attr->logical_area.x = obj->area.x + obj->area.w - padding->right - 20;
    font_attr->logical_area.y = obj->area.y + obj->area.h - padding->bottom + 10;
    font_attr->logical_area.w = padding->left - 6;
    font_attr->logical_area.h = 20;
}

// TODO anti-order x/y axis
static void _draw_ruler_range(gt_obj_st * obj, gt_padding_st const * const padding) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    char buf[20] = {0};
    gt_font_st font = {0};
    gt_attr_font_st font_attr = {
        .font = &font,
        .font_color = style->gp.axis.scale.color,
        .opa = style->gp.axis.scale.opa,
    };
    _draw_ruler_pos_area_t pos_func_arr[] = {
        _draw_ruler_ver_start, _draw_ruler_ver_end, _draw_ruler_hor_start, _draw_ruler_hor_end
    };
    gt_size_t value[] = {
        style->gp.axis.ver.start, style->gp.axis.ver.end, style->gp.axis.hor.start, style->gp.axis.hor.end
    };
    gt_font_info_init(&font.info);
    gt_font_info_update_font_thick(&font.info);

    for (uint8_t i = 0; i < 4; ++i) {
        pos_func_arr[i](obj, padding, &font_attr);
        sprintf(buf, "%d", value[i]);
        font.utf8 = buf;
        font.len = strlen(buf);
        draw_text(obj->draw_ctx, &font_attr, &obj->area);
    }
}

static void _draw_axis(gt_obj_st * obj) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    gt_padding_st padding = _get_padding_by(obj);

    gt_attr_line_st line_attr = { .line = style->gp.axis.scale };
    gt_point_st points[] = {
        { padding.left, padding.top }, { obj->area.w - padding.right, padding.top },
        { obj->area.w - padding.right, obj->area.h - padding.bottom }, { padding.left, obj->area.h - padding.bottom },
    };

    _void_draw_grid_lines(obj, &padding, points);

    if (line_attr.line.opa < GT_OPA_MIN) {
        return;
    }
    /** graph's inside border line */
    for (gt_size_t i = 0; i < 4; ++i) {
        line_attr.start = _add_obj_offset(&points[i], obj);
        line_attr.end = _add_obj_offset(&points[(i + 1) % 4], obj);
        gt_draw_line(obj->draw_ctx, &line_attr, &obj->area);
    }

    if (_is_show_ruler(obj)) {
        _draw_ruler_range(obj, &padding);
    }
}

static inline bool _is_line_type(_gt_graphs_st * style, gt_graphs_type_et type) {
    return type == style->gp.type;
}

static inline gt_point_st _get_axis_scale_total_range(_gt_graphs_st * style) {
    gt_point_st scale_range = {
        .x = style->gp.axis.hor.end == style->gp.axis.hor.start ? 1 : (style->gp.axis.hor.end - style->gp.axis.hor.start),
        .y = style->gp.axis.ver.end == style->gp.axis.ver.start ? 1 : (style->gp.axis.ver.end - style->gp.axis.ver.start),
    };
    return scale_range;
}

static inline gt_point_st _get_start_value_by(_gt_graphs_st * style) {
    gt_point_st start_val = { .x = style->gp.axis.hor.start, .y = style->gp.axis.ver.start };
    return start_val;
}

static inline gt_size_t _calc_abs_point_x(gt_size_t p, _graph_css_st const * const css) {
    // TODO anti-order x/y axis
    return (p - css->start.x) * css->range.x / css->total.x + css->padding.left;
}

static inline gt_size_t _calc_abs_point_y(gt_size_t p, _graph_css_st const * const css) {
    // TODO anti-order x/y axis
    return (p - css->start.y) * css->range.y / css->total.y + css->padding.top;
}

static inline void _draw_point_by( gt_obj_st * obj, gt_series_points_st * ptr, gt_size_t idx, _graph_css_st const * const css) {
    gt_attr_point_st point_attr = { .line = ptr->line };
    point_attr.pos.x = _calc_abs_point_x(ptr->x_series[idx], css);
    point_attr.pos.y = _calc_abs_point_y(ptr->y_series[idx], css);
    gt_draw_point(obj->draw_ctx, &point_attr, &obj->area);
}

static _graph_css_st _get_css(gt_obj_st * obj) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    _graph_css_st css = {
        .padding = _get_padding_by(obj),
        .range = _get_display_pixel_range(&css.padding, obj->area.w, obj->area.h),
        .total = _get_axis_scale_total_range(style),
        .start = _get_start_value_by(style),
    };
    css.padding = _add_obj_offset_padding(&css.padding, obj);
    return css;
}

static void _graphs_draw_points(_gt_graphs_st * style) {
    gt_obj_st * obj = (gt_obj_st * )style;
    gt_series_points_st * ptr = style->gp.series_head_p;
    _graph_css_st css = _get_css(obj);

    while (ptr) {
        for (gt_size_t i = 0; i < ptr->count_point; ++i) {
            _draw_point_by(obj, ptr, i, &css);
        }
        ptr = ptr->next_series_p;
    }
}

static void _graphs_draw_line(_gt_graphs_st * style) {
    gt_obj_st * obj = (gt_obj_st * )style;
    gt_attr_line_st line_attr = {0};
    gt_series_points_st * ptr = style->gp.series_head_p;
    _graph_css_st css = _get_css(obj);

    while (ptr) {
        line_attr.line = ptr->line;
        if (0 == ptr->count_point) {
            ptr = ptr->next_series_p;
            continue;
        } else if (1 == ptr->count_point) {
            _draw_point_by(obj, ptr, 0, &css);
            ptr = ptr->next_series_p;
            continue;
        }
        line_attr.end.x = _calc_abs_point_x(ptr->x_series[0], &css);
        line_attr.end.y = _calc_abs_point_y(ptr->y_series[0], &css);
        for (gt_size_t i = 1; i < ptr->count_point; ++i) {
            line_attr.start = line_attr.end;
            line_attr.end.x = _calc_abs_point_x(ptr->x_series[i], &css);
            line_attr.end.y = _calc_abs_point_y(ptr->y_series[i], &css);
            gt_draw_line(obj->draw_ctx, &line_attr, &obj->area);
        }
        ptr = ptr->next_series_p;
    }
}

static void _graphs_draw_curve(_gt_graphs_st * style) {
    gt_obj_st * obj = (gt_obj_st * )style;
    gt_attr_curve_st bz_attr = {0};
    gt_series_points_st * ptr = style->gp.series_head_p;
    _graph_css_st css = _get_css(obj);

    while (ptr) {
        bz_attr.line = ptr->line;
        if (0 == ptr->count_point) {
            ptr = ptr->next_series_p;
            continue;
        } else if (1 == ptr->count_point) {
            _draw_point_by(obj, ptr, 0, &css);
            ptr = ptr->next_series_p;
            continue;
        } else if (2 == ptr->count_point) {
            gt_attr_line_st line_attr = {
                .line = ptr->line,
                .start = {
                    .x = _calc_abs_point_x(ptr->x_series[0], &css),
                    .y = _calc_abs_point_y(ptr->y_series[0], &css),
                },
                .end = {
                    .x = _calc_abs_point_x(ptr->x_series[1], &css),
                    .y = _calc_abs_point_y(ptr->y_series[1], &css),
                },
            };
            gt_draw_line(obj->draw_ctx, &line_attr, &obj->area);
            continue;
        }

        bz_attr.p1.x = _calc_abs_point_x(ptr->x_series[0], &css);
        bz_attr.p1.y = _calc_abs_point_y(ptr->y_series[0], &css);
        bz_attr.p2 = bz_attr.p1;
        bz_attr.p3.x = _calc_abs_point_x(ptr->x_series[1], &css);
        bz_attr.p3.y = _calc_abs_point_y(ptr->y_series[1], &css);

        for (gt_size_t i = 0; i + 1 < ptr->count_point; ++i) {
            bz_attr.p0 = bz_attr.p1;
            bz_attr.p1 = bz_attr.p2;
            bz_attr.p2 = bz_attr.p3;
            if (i + 2 < ptr->count_point) {
                bz_attr.p3.x = _calc_abs_point_x(ptr->x_series[i + 2], &css);
                bz_attr.p3.y = _calc_abs_point_y(ptr->y_series[i + 2], &css);
            }
            gt_draw_catmullrom(obj->draw_ctx, &bz_attr, &obj->area);
        }

        ptr = ptr->next_series_p;
    }
}

static void _graphs_draw_bar(_gt_graphs_st * style) {
    GT_LOG_A("GRAPHS", "WARN Bar has not been implemented yet!!!");
}

static void _draw_points_lines_curves(gt_obj_st * obj) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;

    if (_is_line_type(style, GT_GRAPHS_TYPE_LINE)) {
        _graphs_draw_line(style);
    } else if (_is_line_type(style, GT_GRAPHS_TYPE_CURVE)) {
        _graphs_draw_curve(style);
    } else if (_is_line_type(style, GT_GRAPHS_TYPE_BAR)) {
        _graphs_draw_bar(style);
    } else {
        _graphs_draw_points(style);
    }
}

static bool _is_series_valid(_gt_graphs_st * style, gt_series_points_st const * series) {
    if (NULL == style || NULL == series) {
        return false;
    }
    gt_series_points_st * ptr = style->gp.series_head_p;
    while (ptr) {
        if (ptr == series) { return true; }
        ptr = ptr->next_series_p;
    }
    return false;
}

static gt_series_points_st * _get_series_line_by(gt_obj_st * obj, uint16_t idx) {
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return NULL;
    }
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    if (idx >= style->gp.count_line) {
        return NULL;
    }
    gt_series_points_st * ptr = style->gp.series_head_p;
    while (ptr && idx-- > 0) {
        ptr = ptr->next_series_p;
    }
    return ptr;
}

/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_graphs_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    style->gp.axis.scale.color = gt_color_black();
    style->gp.axis.scale.width = 2;
    style->gp.axis.scale.opa = GT_OPA_COVER;

    return obj;
}

void gt_graphs_set_self_adaptive(gt_obj_st * obj, bool enabled)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    style->reg.self_adaptive = enabled;
}

void gt_graphs_set_show_ruler(gt_obj_st * obj, bool enabled)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    style->reg.show_ruler = enabled;
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
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    uint16_t size = sizeof(gt_series_points_st);
    gt_series_points_st * series = gt_mem_malloc(size);
    GT_CHECK_BACK_VAL(series, NULL);
    gt_memset(series, 0, size);

    gt_memcpy(&series->line, line_style, sizeof(gt_line_st));

    ++style->gp.count_line;
    if (NULL == style->gp.series_head_p) {
        style->gp.series_head_p = series;
        return series;
    }
    gt_series_points_st * ptr = style->gp.series_head_p;
    while (ptr->next_series_p) { ptr = ptr->next_series_p; }
    ptr->next_series_p = series;
    return series;
}

gt_res_t gt_graphs_series_add_point(gt_obj_st * obj, gt_series_points_st * const series, gt_point_st const * const point)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return GT_RES_FAIL;
    }
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    if (false == _is_series_valid(style, series)) {
        return GT_RES_FAIL;
    }
    uint16_t size = sizeof(gt_size_t);
    if (series->count_point) {
        series->x_series = gt_mem_realloc(series->x_series, (series->count_point + 1) * size);
        if (NULL == series->x_series) {
            goto x_fail_lb;
        }
        series->y_series = gt_mem_realloc(series->y_series, (series->count_point + 1) * size);
        if (NULL == series->y_series) {
            goto y_fail_lb;
        }
    } else {
        series->x_series = gt_mem_malloc(size);
        if (NULL == series->x_series) {
            return GT_RES_FAIL;
        }
        series->y_series = gt_mem_malloc(size);
        if (NULL == series->y_series) {
            gt_mem_free(series->x_series);
            series->x_series = NULL;
            return GT_RES_FAIL;
        }
    }
    series->x_series[series->count_point] = point->x;
    series->y_series[series->count_point] = point->y;
    ++series->count_point;

    return GT_RES_OK;

y_fail_lb:
    series->y_series = gt_mem_realloc(series->y_series, series->count_point * size);
x_fail_lb:
    series->x_series = gt_mem_realloc(series->x_series, series->count_point * size);
    return GT_RES_FAIL;
}

gt_res_t gt_graphs_series_add_point_by(gt_obj_st * obj, uint16_t series_index, gt_point_st const * const point)
{
    gt_series_points_st * series = _get_series_line_by(obj, series_index);
    return gt_graphs_series_add_point(obj, series, point);
}

bool gt_graphs_is_self_adaptive(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return false;
    }
    return _is_self_adaptive(obj);
}

bool gt_graphs_is_show_ruler(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return false;
    }
    return _is_show_ruler(obj);
}

#endif  /** GT_CFG_ENABLE_GRAPHS */
/* end ------------------------------------------------------------------*/
