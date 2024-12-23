/**
 * @file gt_graphs.h
 * @author Feyoung
 * @brief Display the graph, can display line, curve, bar, point, etc.
 *      Also can display the ruler of the coordinate system.
 *      FIFO mode can be used to display the flow of data, but the axis
 *      range must be proivded by the user.
 * @version 0.1
 * @date 2024-07-22 21:33:40
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_GRAPHS_H_
#define _GT_GRAPHS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_GRAPHS
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "stdint.h"
#include "stdarg.h"
#include "stdlib.h"
#include "../utils/gt_vector.h"
#include "../core/gt_style.h"
#include "../font/gt_font.h"
#include "../others/gt_types.h"
#include "../core/gt_graph_base.h"



/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief Create a graph object
 *
 * @param parent
 * @return gt_obj_st*
 */
gt_obj_st * gt_graphs_create(gt_obj_st * parent);

/**
 * @brief Set the full area of the graph
 *
 * @param obj
 * @param enabled true: using custom padding size, false: using default padding size
 * @param padding [default]20
 */
void gt_graphs_set_full_area(gt_obj_st * obj, bool enabled, uint8_t padding);

/**
 * @brief Set the coordinate system for adaptive range based on all points
 *
 * @param obj
 * @param enabled
 */
void gt_graphs_set_self_adaptive(gt_obj_st * obj, bool enabled);

/**
 * @brief Displays the ruler of the coordinate system
 *
 * @param obj
 * @param enabled
 */
void gt_graphs_set_show_ruler(gt_obj_st * obj, bool enabled);

/**
 * @brief Set the axis of the graph
 *
 * @param obj
 * @param axis Must be set axis->hor and axis->ver range value,
 *              when scale's opa is 0, the axis will not be displayed,
 *              when grid's opa is 0x0, the grid line will not be displayed
 */
void gt_graphs_set_axis(gt_obj_st * obj, gt_axis_st const * const axis);

/**
 * @brief Using FIFO mode to display the graph, the graph will
 *      automatically delete the first or oldest point.
 *      [Warning] Must disabled self adaptive mode, otherwise it will not work normally.
 *          x-axis will be automatically calculated by hor, which was provided start and
 *      end value ignore the x-axis was provided by the user;
 *          y-axis was provided by the user.
 *
 * @param obj
 * @param fifo_depth 0[default]: not use fifo, unlimited points; 1~65535: fifo depth
 */
void gt_graphs_set_fifo_max_points_count(gt_obj_st * obj, uint16_t fifo_depth);

/**
 * @brief Add a series to the graph
 *
 * @param obj
 * @param line_style
 * @return gt_series_points_st* series points object
 */
gt_series_points_st * gt_graphs_add_series_line(gt_obj_st * obj, gt_line_st const * const line_style);

/**
 * @brief Add a series to the graph
 *
 * @param obj
 * @param series The series to be added
 * @param point The point to be added
 * @param point_text The text of the point will to display near the point
 *                  [default]NULL: not display
 * @return gt_res_t
 */
gt_res_t gt_graphs_series_add_point(gt_obj_st * obj, gt_series_points_st * const series,
    gt_point_f_st const * const point, char const * const point_text);

/**
 * @brief Add a point to the series by index
 *
 * @param obj
 * @param series_index The index of the series to which the point is added
 * @param point_text The text of the point will to display near the point
 *                  [default]NULL: not display
 * @param point The point to be added
 * @return gt_res_t
 */
gt_res_t gt_graphs_series_add_point_by(gt_obj_st * obj, uint16_t series_index,
    gt_point_f_st const * const point, char const * const point_text);

/**
 * @brief Add a series's point array to the graph
 *
 * @param obj
 * @param series The series to be added
 * @param list The array of points to be added
 * @param text_list The array of text to be added, which is used to display the value of the point
 *                  [default]NULL: not display
 * @param count The count of the array items
 * @return gt_res_t
 */
gt_res_t gt_graphs_series_add_point_list(gt_obj_st * obj, gt_series_points_st * const series,
    gt_point_f_st const * const list, char * const text_list[], uint16_t count);

/**
 * @brief Add a series's point array to the graph by index
 *
 * @param obj
 * @param series_index The index of the series to which the point is added
 * @param list The array of points to be added
 * @param text_list The array of text to be added, which is used to display the value of the point
 *                  [default]NULL: not display
 * @param count The count of the array items
 * @return gt_res_t
 */
gt_res_t gt_graphs_series_add_point_list_by(gt_obj_st * obj, uint16_t series_index,
    gt_point_f_st const * const list, char * const text_list[], uint16_t count);

/**
 * @brief Remove all points of the series line by index, not remove the series line
 *
 * @param obj
 * @param series_index The index of the series to which want to remove all points
 * @return gt_res_t
 */
gt_res_t gt_graphs_series_remove_all_points(gt_obj_st * obj, uint16_t series_index);

/**
 * @brief Remove all series lines, must be re-added if you want to display the series line
 *
 * @param obj
 * @return gt_res_t
 */
gt_res_t gt_graphs_remove_all_series_lines(gt_obj_st * obj);

bool gt_graphs_is_self_adaptive(gt_obj_st * obj);

bool gt_graphs_is_show_ruler(gt_obj_st * obj);

void gt_graphs_set_anti_axis_hor_dir(gt_obj_st * obj, bool enabled);
void gt_graphs_set_anti_axis_ver_dir(gt_obj_st * obj, bool enabled);

/**
 * @brief Display the value of the point in the graph
 *
 * @param obj
 * @param series_index The index of the series to which the point is added
 * @param enabled [default]false: not display, true: display
 */
void gt_graphs_show_point_value(gt_obj_st * obj, uint16_t series_index, bool enabled);

/**
 * @brief Set the space between the value of the point and the line
 *
 * @param obj
 * @param series_index The index of the series to which the point is added
 * @param space default: 8
 */
void gt_graphs_set_point_value_space_between_line(gt_obj_st * obj, uint16_t series_index, uint8_t space);

/**
 * @brief Set the font color of the value of the point,
 *      which will to display near the point
 *
 * @param obj
 * @param series_index The index of the series to which the point is added
 * @param font_color The font color of the value of the point, not the axis font color.
 */
void gt_graphs_set_point_value_font_color(gt_obj_st * obj, uint16_t series_index, gt_color_t font_color);

/**
 * @brief Set the value of the point to be displayed at the bottom of the point or line.
 *
 * @param obj
 * @param series_index The index of the series to which the point is added
 * @param enabled [default]false: display at the top, true: display at the bottom
 */
void gt_graphs_set_point_value_bottom(gt_obj_st * obj, uint16_t series_index, bool enabled);

/**
 * @brief Is the value of the point displayed.
 *
 * @param obj
 * @param series_index The index of the series to which the point is added
 * @return true
 * @return false
 */
bool gt_graphs_is_show_point_value(gt_obj_st * obj, uint16_t series_index);

/**
 * @brief Set the font color of the graph axis
 *
 * @param graphs
 * @param color Only axis font color
 */
void gt_graphs_set_font_color(gt_obj_st * graphs, gt_color_t color);

void gt_graphs_set_font_size(gt_obj_st * graphs, uint8_t size);
void gt_graphs_set_font_gray(gt_obj_st * graphs, uint8_t gray);
void gt_graphs_set_font_align(gt_obj_st * graphs, gt_align_et align);
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
void gt_graphs_set_font_family_cn(gt_obj_st * graphs, gt_family_t family);
void gt_graphs_set_font_family_en(gt_obj_st * graphs, gt_family_t family);
void gt_graphs_set_font_family_fl(gt_obj_st * graphs, gt_family_t family);
void gt_graphs_set_font_family_numb(gt_obj_st * graphs, gt_family_t family);
#else
void gt_graphs_set_font_family(gt_obj_st * graphs, gt_family_t family);
void gt_graphs_set_font_cjk(gt_obj_st* graphs, gt_font_cjk_et cjk);
#endif
void gt_graphs_set_font_thick_en(gt_obj_st * graphs, uint8_t thick);
void gt_graphs_set_font_thick_cn(gt_obj_st * graphs, uint8_t thick);
void gt_graphs_set_font_encoding(gt_obj_st * graphs, gt_encoding_et encoding);
void gt_graphs_set_font_style(gt_obj_st * graphs, gt_font_style_et font_style);

uint8_t gt_graphs_get_font_size(gt_obj_st * graphs);

#endif  /** GT_CFG_ENABLE_GRAPHS */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_GRAPHS_H_
