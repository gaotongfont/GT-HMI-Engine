/**
 * @file gt_graphs.h
 * @author Feyoung
 * @brief
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
 * @brief Set the axis of the graph
 *
 * @param obj
 * @param axis Must be set axis->hor and axis->ver range value,
 *              when scale's opa is 0, the axis will not be displayed,
 *              when grid's opa is 0x0, the grid line will not be displayed
 */
void gt_graphs_set_axis(gt_obj_st * obj, gt_axis_st const * const axis);

void gt_graphs_set_series_line_type(gt_obj_st * obj, gt_graphs_type_et type_style);

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
 * @return gt_res_t
 */
gt_res_t gt_graphs_series_add_point(gt_obj_st * obj, gt_series_points_st * const series, gt_point_st const * const point);

/**
 * @brief Add a point to the series by index
 *
 * @param obj
 * @param series_index The index of the series to which the point is added
 * @param point The point to be added
 * @return gt_res_t
 */
gt_res_t gt_graphs_series_add_point_by(gt_obj_st * obj, uint16_t series_index, gt_point_st const * const point);

#endif  /** GT_CFG_ENABLE_GRAPHS */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_GRAPHS_H_
