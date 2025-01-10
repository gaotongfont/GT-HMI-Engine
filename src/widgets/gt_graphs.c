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
#include "../font/gt_font.h"


/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_GRAPH
#define MY_CLASS    &gt_graphs_class


#define _GRAPHS_DEFAULT_PADDING_SIZE        20

#define _GRAPHS_DEFAULT_FONT_SPACE_LINE     8


/* private typedef ------------------------------------------------------*/
typedef struct _graphs_reg_s {
    uint8_t self_adaptive : 1;
    uint8_t show_ruler : 1;
    uint8_t anti_hor_axis_dir : 1;
    uint8_t anti_ver_axis_dir : 1;
    uint8_t full_area : 1;
}_graphs_reg_st;

typedef struct _gt_graphs_s {
    gt_obj_st obj;
    _gt_attr_graphs_st gp;
    gt_color_t font_color;
    gt_font_info_st font_info;
    uint16_t fifo_depth;        /** 0[default]: not use fifo mode */
    uint8_t font_align;         //@ref gt_align_et
    uint8_t custom_padding;
    _graphs_reg_st reg;
}_gt_graphs_st;

typedef struct _graph_css_s {
    gt_padding_st padding;
    gt_point_st range;
    gt_point_st total;
    gt_point_st start;
    gt_point_st end;
    _graphs_reg_st reg;
}_graph_css_st;

typedef void ( * _draw_ruler_pos_area_t)(gt_obj_st * , gt_padding_st const * const, gt_attr_font_st * );

/* static variables -----------------------------------------------------*/
static void _graphs_init_cb(gt_obj_st * obj);
static void _graphs_deinit_cb(gt_obj_st * obj);
static void _graphs_event_cb(struct gt_obj_s * obj, gt_event_st * e);

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_graphs_class = {
    ._init_cb      = _graphs_init_cb,
    ._deinit_cb    = _graphs_deinit_cb,
    ._event_cb     = _graphs_event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_graphs_st)
};

static GT_ATTRIBUTE_RAM_TEXT bool _is_self_adaptive(gt_obj_st * obj);
static GT_ATTRIBUTE_RAM_TEXT void _draw_axis(gt_obj_st * obj);
static GT_ATTRIBUTE_RAM_TEXT void _draw_points_lines_curves(gt_obj_st * obj);
static GT_ATTRIBUTE_RAM_TEXT void _calc_self_adaptive_axis_range(gt_obj_st * obj);
static GT_ATTRIBUTE_RAM_TEXT void _draw_point_value(gt_obj_st * obj);


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
    if (obj->show_bg) {
        gt_attr_rect_st rect_attr;
        gt_graph_init_rect_attr(&rect_attr);
        rect_attr.reg.is_fill   = true;
        rect_attr.radius        = obj->radius;
        rect_attr.bg_opa        = obj->opa;
        rect_attr.fg_color      = obj->bgcolor;
        rect_attr.bg_color      = obj->bgcolor;

        /** base shape */
        draw_bg(obj->draw_ctx, &rect_attr, &obj->area);
    }

    if (_is_self_adaptive(obj) || _is_hor_range_zero(style) || _is_ver_range_zero(style)) {
        _calc_self_adaptive_axis_range(obj);
    }

    /** draw axis */
    _draw_axis(obj);

    /** draw points / lines / curves */
    _draw_points_lines_curves(obj);

    /** draw point value */
    _draw_point_value(obj);
}

static GT_ATTRIBUTE_RAM_TEXT void _free_series_line_points(gt_series_points_st * series) {
    if (series->point_value_str) {
        for (gt_size_t i = series->count_point - 1; i >= 0; --i) {
            if (series->point_value_str[i]) {
                gt_mem_free(series->point_value_str[i]);
            }
            series->point_value_str[i] = NULL;
        }
        gt_mem_free(series->point_value_str);
        series->point_value_str = NULL;
    }
    if (series->y_series) {
        gt_mem_free(series->y_series);
        series->y_series = NULL;
    }
    if (series->x_series) {
        gt_mem_free(series->x_series);
        series->x_series = NULL;
    }
    series->count_point = 0;
}

static GT_ATTRIBUTE_RAM_TEXT void _free_all_series_lines(gt_series_points_st * series) {
    if (NULL == series) {
        return;
    }
    if (series->next_series_p) {
        _free_all_series_lines(series->next_series_p);
        gt_mem_free(series->next_series_p);
        series->next_series_p = NULL;
    }
    _free_series_line_points(series);
}

static GT_ATTRIBUTE_RAM_TEXT void _graphs_free_series_lines(_gt_graphs_st * style) {
    if (NULL == style->gp.series_head_p) {
        return;
    }
    _free_all_series_lines(style->gp.series_head_p);
    gt_mem_free(style->gp.series_head_p);
    style->gp.series_head_p = NULL;
    style->gp.count_line = 0;
}

static void _graphs_deinit_cb(gt_obj_st * obj) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    _graphs_free_series_lines(style);
}

static void _graphs_event_cb(struct gt_obj_s * obj, gt_event_st * e) {

}

static GT_ATTRIBUTE_RAM_TEXT bool _is_self_adaptive(gt_obj_st * obj) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    return style->reg.self_adaptive;
}

static GT_ATTRIBUTE_RAM_TEXT bool _is_show_ruler(gt_obj_st * obj) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    return style->reg.show_ruler;
}

/**
 * @brief Get display pixel range
 *
 * @param padding
 * @param w
 * @param h
 * @return gt_point_st
 */
static GT_ATTRIBUTE_RAM_TEXT gt_point_st _get_display_pixel_range(gt_padding_st const * const padding, uint16_t w, uint16_t h) {
    gt_point_st ret = { .x = w - padding->left - padding->right, .y = h - padding->top - padding->bottom };
    return ret;
}

static GT_ATTRIBUTE_RAM_TEXT gt_point_st _add_obj_offset(gt_point_st const * const p, gt_obj_st * obj) {
    gt_point_st ret = { .x = p->x + obj->area.x, .y = p->y + obj->area.y };
    return ret;
}

static GT_ATTRIBUTE_RAM_TEXT gt_padding_st _add_obj_offset_padding(gt_padding_st const * const p, gt_obj_st * obj) {
    gt_padding_st ret = {
        .left = p->left + obj->area.x,
        .right = p->right,
        .top = p->top + obj->area.y,
        .bottom = p->bottom,
    };
    return ret;
}

static GT_ATTRIBUTE_RAM_TEXT void _update_axis_range(gt_range_st * dir_range, gt_size_t min, gt_size_t max, uint16_t redundancy) {
    dir_range->start = min - redundancy;
    dir_range->end   = max + redundancy;
}

static GT_ATTRIBUTE_RAM_TEXT void _calc_self_adaptive_axis_range(gt_obj_st * obj) {
    gt_point_st min = { 0x7fff, 0x7fff }, max = { -0x7fff, -0x7fff };
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    gt_series_points_st * ptr = style->gp.series_head_p;
    uint16_t max_count = 0;

    while (ptr) {
        for (gt_size_t i = 0; i < ptr->count_point; ++i) {
            min.x = GT_MIN(min.x, (gt_size_t)ptr->x_series[i]);
            min.y = GT_MIN(min.y, (gt_size_t)ptr->y_series[i]);
            max.x = GT_MAX(max.x, (gt_size_t)ptr->x_series[i]);
            max.y = GT_MAX(max.y, (gt_size_t)ptr->y_series[i]);
        }
        if (ptr->count_point > max_count) {
            max_count = ptr->count_point;
        }
        ptr = ptr->next_series_p;
    }
    /** set default unit */
    if (0.0 == style->gp.axis.hor_unit) {
        style->gp.axis.hor_unit = ((max.x - min.x) + max_count) / (max_count + 1);
    }
    if (0.0 == style->gp.axis.ver_unit) {
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

static GT_ATTRIBUTE_RAM_TEXT gt_point_st _calc_div_part(_gt_graphs_st * style) {
    gt_point_st div_part = {
        .x = gt_abs((style->gp.axis.hor.end - style->gp.axis.hor.start) / style->gp.axis.hor_unit),
        .y = gt_abs((style->gp.axis.ver.end - style->gp.axis.ver.start) / style->gp.axis.ver_unit),
    };
    if (0 == div_part.x) { div_part.x = 1; }  /** Avoid the denominator 0 */
    if (0 == div_part.y) { div_part.y = 1; }
    return div_part;
}

static GT_ATTRIBUTE_RAM_TEXT void _void_draw_grid_lines(gt_obj_st * obj, gt_padding_st const * const padding, gt_point_st * points) {
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

static inline uint16_t _get_padding_size(_gt_graphs_st * style) {
    return style->reg.full_area ? style->custom_padding : _GRAPHS_DEFAULT_PADDING_SIZE;
}

static GT_ATTRIBUTE_RAM_TEXT gt_padding_st _get_padding_by(gt_obj_st * obj) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    gt_padding_st padding = {
        .left = style->reg.show_ruler ? 60 : _get_padding_size(style),
        .right = _get_padding_size(style),
        .top = _get_padding_size(style),
        .bottom = style->reg.show_ruler ? 40 : _get_padding_size(style),
    };
    /** calc real right and bottom padding */
    gt_point_st div_part = _calc_div_part(style);
    gt_point_st grid = _get_display_pixel_range(&padding, obj->area.w, obj->area.h);

    /** resize really graphs size */
    if (grid.x > div_part.x) {
        padding.right += grid.x - (grid.x / div_part.x) * div_part.x;
    }
    if (grid.y > div_part.y) {
        padding.bottom += grid.y - (grid.y / div_part.y) * div_part.y;
    }

    return padding;
}

static GT_ATTRIBUTE_RAM_TEXT void _set_point_value_pos(gt_obj_st * obj, gt_point_st * pointer, uint16_t str_width, gt_attr_font_st * font_attr) {
    font_attr->logical_area.x = pointer->x - (str_width >> 1);
    font_attr->logical_area.y = pointer->y;
    font_attr->logical_area.w = str_width + font_attr->font->info.size;
    font_attr->logical_area.h = font_attr->font->info.size;
}

static GT_ATTRIBUTE_RAM_TEXT void _draw_ruler_ver_start(gt_obj_st * obj, gt_padding_st const * const padding, gt_attr_font_st * font_attr) {
    font_attr->logical_area.x = obj->area.x + 4;
    font_attr->logical_area.y = obj->area.y + padding->top - 6;
    font_attr->logical_area.w = padding->left - 6;
    font_attr->logical_area.h = 20;
}

static GT_ATTRIBUTE_RAM_TEXT void _draw_ruler_ver_end(gt_obj_st * obj, gt_padding_st const * const padding, gt_attr_font_st * font_attr) {
    font_attr->logical_area.x = obj->area.x + 4;
    font_attr->logical_area.y = obj->area.y + obj->area.h - padding->bottom - 6;
    font_attr->logical_area.w = padding->left - 6;
    font_attr->logical_area.h = 20;
}

static GT_ATTRIBUTE_RAM_TEXT void _draw_ruler_hor_start(gt_obj_st * obj, gt_padding_st const * const padding, gt_attr_font_st * font_attr) {
    font_attr->logical_area.x = obj->area.x + padding->left - 6;
    font_attr->logical_area.y = obj->area.y + obj->area.h - padding->bottom + 10;
    font_attr->logical_area.w = padding->left - 6;
    font_attr->logical_area.h = 20;
}

static GT_ATTRIBUTE_RAM_TEXT void _draw_ruler_hor_end(gt_obj_st * obj, gt_padding_st const * const padding, gt_attr_font_st * font_attr) {
    font_attr->logical_area.x = obj->area.x + obj->area.w - padding->right - 40;
    font_attr->logical_area.y = obj->area.y + obj->area.h - padding->bottom + 10;
    font_attr->logical_area.w = padding->left - 6;
    font_attr->logical_area.h = 20;
}

static GT_ATTRIBUTE_RAM_TEXT void _draw_ruler_range(gt_obj_st * obj, gt_padding_st const * const padding) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    char buf[20] = {0};
    gt_font_st font = {
        .info = style->font_info,
    };
    gt_attr_font_st font_attr = {
        .align = style->font_align,
        .font = &font,
        .font_color = style->font_color,
        .opa = style->gp.axis.scale.opa,
        .logical_area = obj->area,
    };
    _draw_ruler_pos_area_t pos_func_arr[] = {
        _draw_ruler_ver_start, _draw_ruler_ver_end, _draw_ruler_hor_start, _draw_ruler_hor_end
    };
    gt_float_t value[] = {
        style->reg.anti_ver_axis_dir ? style->gp.axis.ver.start : style->gp.axis.ver.end,
        style->reg.anti_ver_axis_dir ? style->gp.axis.ver.end : style->gp.axis.ver.start,
        style->reg.anti_hor_axis_dir ? style->gp.axis.hor.end : style->gp.axis.hor.start,
        style->reg.anti_hor_axis_dir ? style->gp.axis.hor.start : style->gp.axis.hor.end,
    };
    gt_font_info_update_font_thick(&font.info);

    for (uint8_t i = 0; i < 4; ++i) {
        pos_func_arr[i](obj, padding, &font_attr);
        sprintf(buf, "%.2f", value[i]);
        font.utf8 = buf;
        font.len = strlen(buf);
        draw_text(obj->draw_ctx, &font_attr, &obj->area);
    }
}

static GT_ATTRIBUTE_RAM_TEXT void _draw_axis(gt_obj_st * obj) {
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

static inline bool _is_line_type(gt_series_points_st const * const series, gt_graphs_type_et type) {
    return type == series->line.type;
}

/**
 * @brief Get start to end range
 *
 * @param style
 * @return gt_point_st
 */
static inline gt_point_st _get_axis_scale_total_range(_gt_graphs_st * style) {
    gt_point_st scale_range = {
        .x = style->gp.axis.hor.end == style->gp.axis.hor.start ? 1 : (style->gp.axis.hor.end - style->gp.axis.hor.start),
        .y = style->gp.axis.ver.end == style->gp.axis.ver.start ? 1 : (style->gp.axis.ver.end - style->gp.axis.ver.start),
    };
    return scale_range;
}
/**
 * @brief Not the absolute position of the screen display position,
 *      but the display value of the x-axis, using by the fifo mode.
 *
 * @param style
 * @param index The fifo depth index
 * @return gt_float_t
 */
static inline gt_float_t _fifo_calc_average_x_pos(_gt_graphs_st * style, uint16_t index) {
    uint16_t div = style->fifo_depth > 1 ? style->fifo_depth - 1 : 1;
    return (gt_float_t)(index * (style->reg.anti_hor_axis_dir ?
        style->gp.axis.hor.start - style->gp.axis.hor.end : style->gp.axis.hor.end - style->gp.axis.hor.start) / div);
}

static inline gt_point_st _get_start_value_by(_gt_graphs_st * style) {
    gt_point_st start_val = { .x = style->gp.axis.hor.start, .y = style->gp.axis.ver.start };
    return start_val;
}

static inline gt_point_st _get_end_value_by(_gt_graphs_st * style) {
    gt_point_st end_val = { .x = style->gp.axis.hor.end, .y = style->gp.axis.ver.end };
    return end_val;
}

static inline gt_size_t _calc_abs_point_x(gt_float_t p, _graph_css_st const * const css) {
    gt_size_t offset = css->reg.anti_hor_axis_dir ? (css->end.x - p) : (p - css->start.x);
    return offset * css->range.x / css->total.x + css->padding.left;
}

static inline gt_size_t _calc_abs_point_y(gt_float_t p, _graph_css_st const * const css) {
    gt_size_t offset = css->reg.anti_ver_axis_dir ? (p - css->start.y) : (css->end.y - p);
    return offset * css->range.y / css->total.y + css->padding.top;
}

static inline void _draw_point_by(
    gt_obj_st * obj, gt_series_points_st const * const ptr, gt_size_t idx, _graph_css_st const * const css) {
    gt_attr_point_st point_attr = { .line = ptr->line };
    point_attr.pos.x = _calc_abs_point_x(ptr->x_series[idx], css);
    point_attr.pos.y = _calc_abs_point_y(ptr->y_series[idx], css);
    gt_draw_point(obj->draw_ctx, &point_attr, &obj->area);
}

static GT_ATTRIBUTE_RAM_TEXT _graph_css_st _get_css(gt_obj_st * obj) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    _graph_css_st css = {
        .padding = _get_padding_by(obj),
        .range = _get_display_pixel_range(&css.padding, obj->area.w, obj->area.h),
        .total = _get_axis_scale_total_range(style),
        .start = _get_start_value_by(style),
        .end   = _get_end_value_by(style),
        .reg   = style->reg,
    };
    css.padding = _add_obj_offset_padding(&css.padding, obj);
    return css;
}

static GT_ATTRIBUTE_RAM_TEXT void _graphs_draw_points(gt_obj_st * obj, gt_series_points_st const * const ptr) {
    _graph_css_st css = _get_css(obj);

    for (gt_size_t i = 0; i < ptr->count_point; ++i) {
        _draw_point_by(obj, ptr, i, &css);
    }
}

static GT_ATTRIBUTE_RAM_TEXT void _graphs_draw_line(gt_obj_st * obj, gt_series_points_st const * const ptr) {
    gt_attr_line_st line_attr = { .line = ptr->line };
    _graph_css_st css = _get_css(obj);

    if (0 == ptr->count_point) {
        return;
    } else if (1 == ptr->count_point) {
        _draw_point_by(obj, ptr, 0, &css);
        return;
    }
    line_attr.end.x = _calc_abs_point_x(ptr->x_series[0], &css);
    line_attr.end.y = _calc_abs_point_y(ptr->y_series[0], &css);
    for (gt_size_t i = 1; i < ptr->count_point; ++i) {
        line_attr.start = line_attr.end;
        line_attr.end.x = _calc_abs_point_x(ptr->x_series[i], &css);
        line_attr.end.y = _calc_abs_point_y(ptr->y_series[i], &css);
        gt_draw_line(obj->draw_ctx, &line_attr, &obj->area);
    }
}

static GT_ATTRIBUTE_RAM_TEXT void _graphs_draw_curve(gt_obj_st * obj, gt_series_points_st const * const ptr) {
    gt_attr_curve_st bz_attr = { .line = ptr->line };
    _graph_css_st css = _get_css(obj);

    if (0 == ptr->count_point) {
        return;
    } else if (1 == ptr->count_point) {
        _draw_point_by(obj, ptr, 0, &css);
        return;
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
        return;
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
}

static GT_ATTRIBUTE_RAM_TEXT void _graphs_draw_bar(gt_obj_st * obj, gt_series_points_st const * const ptr) {
    gt_attr_line_st line_attr = { .line = ptr->line };
    _graph_css_st css = _get_css(obj);

    if (0 == ptr->count_point) {
        return;
    }
    line_attr.end.y = _calc_abs_point_y(0, &css);

    for (gt_size_t i = 0; i < ptr->count_point; ++i) {
        line_attr.start.x = _calc_abs_point_x(ptr->x_series[i], &css);
        line_attr.start.y = _calc_abs_point_y(ptr->y_series[i], &css);
        line_attr.end.x = line_attr.start.x;
        gt_draw_line(obj->draw_ctx, &line_attr, &obj->area);
    }
}

static GT_ATTRIBUTE_RAM_TEXT void _draw_points_lines_curves(gt_obj_st * obj) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    gt_series_points_st * ptr = style->gp.series_head_p;

    while (ptr) {
        if (ptr->line.opa < GT_OPA_MIN) {
            ptr = ptr->next_series_p;
            continue;
        }
        if (_is_line_type(ptr, GT_GRAPHS_TYPE_LINE)) {
            _graphs_draw_line(obj, ptr);
        } else if (_is_line_type(ptr, GT_GRAPHS_TYPE_CURVE)) {
            _graphs_draw_curve(obj, ptr);
        } else if (_is_line_type(ptr, GT_GRAPHS_TYPE_BAR)) {
            _graphs_draw_bar(obj, ptr);
        } else {
            _graphs_draw_points(obj, ptr);
        }
        ptr = ptr->next_series_p;
    }
}

static GT_ATTRIBUTE_RAM_TEXT void _draw_point_value(gt_obj_st * obj) {
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    gt_series_points_st * ptr = style->gp.series_head_p;
    _graph_css_st css = _get_css(obj);
    gt_point_st pointer = { .x = 0, .y = 0 };
    char buf[20] = {0};
    gt_font_st font = {
        .utf8 = buf,
        .info = style->font_info,
    };
    gt_attr_font_st font_attr = {
        .align = style->font_align,
        .font = &font,
    };
    uint16_t offset = 0, total_width = 0;
    gt_font_info_update_font_thick(&font.info);

    while (ptr) {
        if (false == ptr->show_point_value || ptr->line.opa < GT_OPA_MIN) {
            ptr = ptr->next_series_p;
            continue;
        }
        offset = style->font_info.size + ptr->font_line_space;
        font_attr.opa = ptr->line.opa;
        font_attr.font_color = ptr->point_value_font_color;

        for (gt_size_t i = 0; i < ptr->count_point; ++i) {
            pointer.x = _calc_abs_point_x(ptr->x_series[i], &css);
            pointer.y = _calc_abs_point_y(ptr->y_series[i], &css);
            if (ptr->point_value_bottom) {
                pointer.y += ptr->font_line_space + (ptr->line.width >> 1);
            } else {
                pointer.y -= offset + (ptr->line.width >> 1);
            }
            font.utf8 = ptr->point_value_str[i];
            font.len = strlen(font.utf8);

            total_width = gt_font_get_longest_line_substring_width(&font.info, font.utf8, font_attr.space_x);
            _set_point_value_pos(obj, &pointer, total_width, &font_attr);
            draw_text(obj->draw_ctx, &font_attr, &obj->area);
        }
        ptr = ptr->next_series_p;
    }
}

static GT_ATTRIBUTE_RAM_TEXT bool _is_series_valid(_gt_graphs_st * style, gt_series_points_st const * series) {
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

static GT_ATTRIBUTE_RAM_TEXT gt_series_points_st * _get_series_line_by(gt_obj_st * obj, uint16_t idx) {
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

static GT_ATTRIBUTE_RAM_TEXT void _fifo_pop_item_by(gt_series_points_st * series, uint16_t pop_count) {
    gt_size_t i = 0, k = 0;
    if (0 == pop_count) {
        return;
    }
    for (i = pop_count, k = 0; i < series->count_point; ++i, ++k) {
        series->y_series[k] = series->y_series[i];
        if (k < pop_count && series->point_value_str[k]) {
            gt_mem_free(series->point_value_str[k]);
            series->point_value_str[k] = NULL;
        }
        series->point_value_str[k] = series->point_value_str[i];
    }
}

static GT_ATTRIBUTE_RAM_TEXT void _fifo_keep_latest_points(_gt_graphs_st * style) {
    gt_series_points_st * series = style->gp.series_head_p;
    gt_size_t i = 0, k = 0;
    if (NULL == series) {
        return;
    }
    if (0 == style->fifo_depth) {
        return;
    }
    while (series) {
        if (series->count_point < style->fifo_depth) {
            series = series->next_series_p;
            continue;
        }
        _fifo_pop_item_by(series, series->count_point - style->fifo_depth);
        series->count_point = style->fifo_depth;
        series = series->next_series_p;
    }
}

static GT_ATTRIBUTE_RAM_TEXT gt_res_t _resize_memory_for_series(gt_series_points_st * series, uint16_t total_count) {
    uint16_t size = sizeof(gt_float_t);
    if (total_count == series->count_point) {
        return GT_RES_OK;
    }
    if (series->count_point) {
        series->x_series = gt_mem_realloc(series->x_series, total_count * size);
        if (NULL == series->x_series) {
            goto x_fail_lb;
        }
        series->y_series = gt_mem_realloc(series->y_series, total_count * size);
        if (NULL == series->y_series) {
            goto y_fail_lb;
        }
        series->point_value_str = gt_mem_realloc(series->point_value_str, total_count * sizeof(char *));
        if (NULL == series->point_value_str) {
            goto point_val_fail_lb;
        }
    } else {
        series->x_series = gt_mem_malloc(total_count * size);
        if (NULL == series->x_series) {
            return GT_RES_FAIL;
        }
        series->y_series = gt_mem_malloc(total_count * size);
        if (NULL == series->y_series) {
            gt_mem_free(series->x_series);
            series->x_series = NULL;
            return GT_RES_FAIL;
        }
        series->point_value_str = gt_mem_malloc(total_count * sizeof(char *));
        if (NULL == series->point_value_str) {
            gt_mem_free(series->y_series);
            series->y_series = NULL;
            gt_mem_free(series->x_series);
            series->x_series = NULL;
            return GT_RES_FAIL;
        }
    }
    return GT_RES_OK;

point_val_fail_lb:
    series->point_value_str = gt_mem_realloc(series->point_value_str, series->count_point * sizeof(char *));
y_fail_lb:
    series->y_series = gt_mem_realloc(series->y_series, series->count_point * size);
x_fail_lb:
    series->x_series = gt_mem_realloc(series->x_series, series->count_point * size);
    return GT_RES_FAIL;
}

/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_graphs_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    obj->show_bg = true;
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    style->gp.axis.scale.color = gt_color_black();
    style->gp.axis.scale.width = 2;
    style->gp.axis.scale.opa = GT_OPA_COVER;
    style->custom_padding = _GRAPHS_DEFAULT_PADDING_SIZE;

    gt_font_info_init(&style->font_info);
    style->font_align = GT_ALIGN_LEFT;
    gt_font_info_update_font_thick(&style->font_info);

    return obj;
}

void gt_graphs_set_full_area(gt_obj_st * obj, bool enabled, uint8_t padding)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    style->reg.full_area = enabled;
    if (style->reg.full_area) {
        style->reg.show_ruler =  false;
    }
    style->custom_padding = padding;
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
    if (NULL == axis) {
        return;
    }
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    gt_memcpy(&style->gp.axis, axis, sizeof(gt_axis_st));
}

void gt_graphs_set_fifo_max_points_count(gt_obj_st * obj, uint16_t fifo_depth)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    style->fifo_depth = fifo_depth;
    _fifo_keep_latest_points(style);
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
    series->font_line_space = _GRAPHS_DEFAULT_FONT_SPACE_LINE;

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

gt_res_t gt_graphs_series_add_point(gt_obj_st * obj, gt_series_points_st * const series, gt_point_f_st const * const point, char const * const point_text)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return GT_RES_FAIL;
    }
    if (NULL == point) {
        return GT_RES_FAIL;
    }
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    if (false == _is_series_valid(style, series)) {
        return GT_RES_FAIL;
    }
    uint16_t size = sizeof(gt_float_t);
    if (style->fifo_depth) {
        _fifo_keep_latest_points(style);
        if (GT_RES_OK != _resize_memory_for_series(series, style->fifo_depth)) {
            return GT_RES_FAIL;
        }
        if (series->count_point >= style->fifo_depth) {
            _fifo_pop_item_by(series, 1);
            --series->count_point;
        }
    }
    else if (GT_RES_OK != _resize_memory_for_series(series, series->count_point + 1)) {
        return GT_RES_FAIL;
    }
    if (style->fifo_depth) {
        series->x_series[series->count_point] = _fifo_calc_average_x_pos(style, series->count_point);
    } else {
        series->x_series[series->count_point] = point->x;
    }
    series->y_series[series->count_point] = point->y;
    if (point_text) {
        uint16_t len = strlen(point_text);
        series->point_value_str[series->count_point] = gt_mem_malloc(len + 1);
        gt_memcpy(series->point_value_str[series->count_point], point_text, len);
        series->point_value_str[series->count_point][len] = '\0';
    } else {
        series->point_value_str[series->count_point] = NULL;
    }
    ++series->count_point;

    return GT_RES_OK;
}

gt_res_t gt_graphs_series_add_point_by(gt_obj_st * obj, uint16_t series_index, gt_point_f_st const * const point, char const * const point_text)
{
    gt_series_points_st * series = _get_series_line_by(obj, series_index);
    return gt_graphs_series_add_point(obj, series, point, point_text);
}

gt_res_t gt_graphs_series_add_point_list(gt_obj_st * obj, gt_series_points_st * const series,
    gt_point_f_st const * const list, char * const text_list[], uint16_t count)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return GT_RES_FAIL;
    }
    if (NULL == list) {
        return GT_RES_FAIL;
    }
    if (0 == count) {
        return GT_RES_OK;
    }
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    gt_size_t src_offset = 0;
    if (false == _is_series_valid(style, series)) {
        return GT_RES_FAIL;
    }
    if (style->fifo_depth) {
        _fifo_keep_latest_points(style);
        if (GT_RES_OK != _resize_memory_for_series(series, style->fifo_depth)) {
            return GT_RES_FAIL;
        }
        if (count > style->fifo_depth) {
            _fifo_pop_item_by(series, style->fifo_depth);
            src_offset = count - style->fifo_depth;
            series->count_point = 0;
        } else if (count + series->count_point > style->fifo_depth) {
            _fifo_pop_item_by(series, count + series->count_point - style->fifo_depth);
            series->count_point = style->fifo_depth - count;
        }
    }
    else if (GT_RES_OK != _resize_memory_for_series(series, series->count_point + count)) {
        return GT_RES_FAIL;
    }
    uint16_t len = 0;
    for (gt_size_t i = 0, offset = series->count_point; i + src_offset < count; ++i) {
        if (style->fifo_depth) {
            series->x_series[offset + i] = _fifo_calc_average_x_pos(style, offset + i);
        } else {
            series->x_series[offset + i] = list[i + src_offset].x;
        }
        series->y_series[offset + i] = list[i + src_offset].y;
        if (text_list && text_list[i + src_offset]) {
            len = strlen(text_list[i + src_offset]);
            series->point_value_str[offset + i] = gt_mem_malloc(len + 1);
            gt_memcpy(series->point_value_str[offset + i], text_list[i + src_offset], len);
            series->point_value_str[offset + i][len] = '\0';
        } else {
            series->point_value_str[offset + i] = NULL;
        }
        ++series->count_point;
    }

    return GT_RES_OK;
}

gt_res_t gt_graphs_series_add_point_list_by(gt_obj_st * obj, uint16_t series_index,
    gt_point_f_st const * const list, char * const text_list[], uint16_t count)
{
    gt_series_points_st * series = _get_series_line_by(obj, series_index);
    return gt_graphs_series_add_point_list(obj, series, list, text_list, count);
}

gt_res_t gt_graphs_series_remove_all_points(gt_obj_st * obj, uint16_t series_index)
{
    gt_series_points_st * series = _get_series_line_by(obj, series_index);
    if (NULL == series) {
        return GT_RES_FAIL;
    }
    _free_series_line_points(series);
    return GT_RES_OK;
}

gt_res_t gt_graphs_remove_all_series_lines(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return GT_RES_FAIL;
    }
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    _graphs_free_series_lines(style);
    return GT_RES_OK;
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

void gt_graphs_set_anti_axis_hor_dir(gt_obj_st * obj, bool enabled)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    style->reg.anti_hor_axis_dir = enabled;
}

void gt_graphs_set_anti_axis_ver_dir(gt_obj_st * obj, bool enabled)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    style->reg.anti_ver_axis_dir = enabled;
}

void gt_graphs_show_point_value(gt_obj_st * obj, uint16_t series_index, bool enabled)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_graphs_st * style = (_gt_graphs_st *)obj;
    gt_series_points_st * series = _get_series_line_by(obj, series_index);
    if (NULL == series) {
        return;
    }
    series->show_point_value = enabled;
}

void gt_graphs_set_point_value_space_between_line(gt_obj_st * obj, uint16_t series_index, uint8_t space)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    gt_series_points_st * series = _get_series_line_by(obj, series_index);
    if (NULL == series) {
        return;
    }
    series->font_line_space = space;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_graphs_set_point_value_font_color(gt_obj_st * obj, uint16_t series_index, gt_color_t font_color)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    gt_series_points_st * series = _get_series_line_by(obj, series_index);
    if (NULL == series) {
        return;
    }
    series->point_value_font_color = font_color;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_graphs_set_point_value_bottom(gt_obj_st * obj, uint16_t series_index, bool enabled)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    gt_series_points_st * series = _get_series_line_by(obj, series_index);
    if (NULL == series) {
        return;
    }
    series->point_value_bottom = enabled;
}

bool gt_graphs_is_show_point_value(gt_obj_st * obj, uint16_t series_index)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return false;
    }
    gt_series_points_st * series = _get_series_line_by(obj, series_index);
    if (NULL == series) {
        return false;
    }
    return series->show_point_value;
}

void gt_graphs_set_font_color(gt_obj_st * graphs, gt_color_t color)
{
    if (false == gt_obj_is_type(graphs, OBJ_TYPE)) {
        return ;
    }
    _gt_graphs_st * style = (_gt_graphs_st * )graphs;
    style->font_color = color;
    gt_event_send(graphs, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_graphs_set_font_size(gt_obj_st * graphs, uint8_t size)
{
    if (false == gt_obj_is_type(graphs, OBJ_TYPE)) {
        return ;
    }
    _gt_graphs_st * style = (_gt_graphs_st * )graphs;
    style->font_info.size = size;
}

void gt_graphs_set_font_gray(gt_obj_st * graphs, uint8_t gray)
{
    if (false == gt_obj_is_type(graphs, OBJ_TYPE)) {
        return ;
    }
    _gt_graphs_st * style = (_gt_graphs_st * )graphs;
    style->font_info.gray = gray;
}

void gt_graphs_set_font_align(gt_obj_st * graphs, gt_align_et align)
{
    if (false == gt_obj_is_type(graphs, OBJ_TYPE)) {
        return ;
    }
    _gt_graphs_st * style = (_gt_graphs_st * )graphs;
    style->font_align = align;
    gt_event_send(graphs, GT_EVENT_TYPE_DRAW_START, NULL);
}

#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
void gt_graphs_set_font_family_cn(gt_obj_st * graphs, gt_family_t family)
{
    if (false == gt_obj_is_type(graphs, OBJ_TYPE)) {
        return ;
    }
    _gt_graphs_st * style = (_gt_graphs_st * )graphs;
    style->font_info.style_cn = family;
}

void gt_graphs_set_font_family_en(gt_obj_st * graphs, gt_family_t family)
{
    if (false == gt_obj_is_type(graphs, OBJ_TYPE)) {
        return ;
    }
    _gt_graphs_st * style = (_gt_graphs_st * )graphs;
    style->font_info.style_en = family;
}

void gt_graphs_set_font_family_fl(gt_obj_st * graphs, gt_family_t family)
{
    if (false == gt_obj_is_type(graphs, OBJ_TYPE)) {
        return ;
    }
    _gt_graphs_st * style = (_gt_graphs_st * )graphs;
    style->font_info.style_fl = family;
}

void gt_graphs_set_font_family_numb(gt_obj_st * graphs, gt_family_t family)
{
    if (false == gt_obj_is_type(graphs, OBJ_TYPE)) {
        return ;
    }
    _gt_graphs_st * style = (_gt_graphs_st * )graphs;
    style->font_info.style_numb = family;
}
#else
void gt_graphs_set_font_family(gt_obj_st * graphs, gt_family_t family)
{
    if (false == gt_obj_is_type(graphs, OBJ_TYPE)) {
        return ;
    }
    _gt_graphs_st * style = (_gt_graphs_st * )graphs;
    gt_font_set_family(&style->font_info, family);
}

void gt_graphs_set_font_cjk(gt_obj_st* graphs, gt_font_cjk_et cjk)
{
    if (false == gt_obj_is_type(graphs, OBJ_TYPE)) {
        return ;
    }
    _gt_graphs_st * style = (_gt_graphs_st * )graphs;
    style->font_info.cjk = cjk;
}
#endif
void gt_graphs_set_font_thick_en(gt_obj_st * graphs, uint8_t thick)
{
    if (false == gt_obj_is_type(graphs, OBJ_TYPE)) {
        return ;
    }
    _gt_graphs_st * style = (_gt_graphs_st * )graphs;
    style->font_info.thick_en = thick;
}
void gt_graphs_set_font_thick_cn(gt_obj_st * graphs, uint8_t thick)
{
    if (false == gt_obj_is_type(graphs, OBJ_TYPE)) {
        return ;
    }
    _gt_graphs_st * style = (_gt_graphs_st * )graphs;
    style->font_info.thick_cn = thick;
}
void gt_graphs_set_font_encoding(gt_obj_st * graphs, gt_encoding_et encoding)
{
    if (false == gt_obj_is_type(graphs, OBJ_TYPE)) {
        return ;
    }
    _gt_graphs_st * style = (_gt_graphs_st * )graphs;
    style->font_info.encoding = encoding;
}
void gt_graphs_set_font_style(gt_obj_st * graphs, gt_font_style_et font_style)
{
    if (false == gt_obj_is_type(graphs, OBJ_TYPE)) {
        return ;
    }
    _gt_graphs_st * style = (_gt_graphs_st * )graphs;
    style->font_info.style.all = font_style;
}

uint8_t gt_graphs_get_font_size(gt_obj_st * graphs)
{
    if (false == gt_obj_is_type(graphs, OBJ_TYPE)) {
        return 0;
    }
    _gt_graphs_st * style = (_gt_graphs_st * )graphs;
    return style->font_info.size;
}
#endif  /** GT_CFG_ENABLE_GRAPHS */
/* end ------------------------------------------------------------------*/
