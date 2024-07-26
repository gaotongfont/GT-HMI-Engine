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

gt_obj_st * gt_graphs_create(gt_obj_st * parent);

void gt_graphs_set_axis(gt_obj_st * obj, gt_axis_st const * const axis);

void gt_graphs_set_series_line_type(gt_obj_st * obj, gt_graphs_type_et type_style);

gt_series_points_st * gt_graphs_add_series_line(gt_obj_st * obj, gt_line_st const * const line_style);

gt_res_t gt_graphs_series_add_point(gt_obj_st * obj, gt_series_points_st const * series, gt_point_st const * const point);
gt_res_t gt_graphs_series_add_point_by(gt_obj_st * obj, uint16_t series_index, gt_point_st const * const point);

#endif  /** GT_CFG_ENABLE_GRAPHS */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_GRAPHS_H_
