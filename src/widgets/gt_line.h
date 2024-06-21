/**
 * @file gt_line.h
 * @author Li
 * @brief
 * @version 0.1
 * @date 2023-01-05 16:28:01
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_LINE_H_
#define _GT_LINE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_LINE
#include "gt_obj.h"
#include "gt_obj_class.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief create line obj
 *
 * @param parent line's parent obj
 * @return gt_obj_st* line obj ptr
 */
gt_obj_st * gt_line_create(gt_obj_st * parent);

void gt_line_set_color(gt_obj_st * line, gt_color_t color);
void gt_line_set_start_point(gt_obj_st * line, uint16_t xs, uint16_t ys);
void gt_line_set_end_point(gt_obj_st * line, uint16_t xe, uint16_t ye);
void gt_line_set_line_width(gt_obj_st * line, uint16_t line_width);


#endif  /** GT_CFG_ENABLE_LINE */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_LINE_H_
