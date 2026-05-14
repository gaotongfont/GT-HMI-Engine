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

/**
 * @brief Get line start point
 *
 * @param line
 * @param ret_p
 * @return gt_res_t
 */
gt_res_t gt_line_get_start_point(gt_obj_st * line, gt_point_st * ret_p);

/**
 * @brief Get line end point
 *
 * @param line
 * @param ret_p
 * @return gt_res_t
 */
gt_res_t gt_line_get_end_point(gt_obj_st * line, gt_point_st * ret_p);

/**
 * @brief Set line length, set position by gt_obj_set_pos()
 *
 * @param line
 * @param length the length of the horizontal line
 */
void gt_line_set_hor_line(gt_obj_st * line, uint16_t length);

/**
 * @brief Set line length, set position by gt_obj_set_pos()
 *
 * @param line
 * @param length the length of the vertical line
 */
void gt_line_set_ver_line(gt_obj_st * line, uint16_t length);

void gt_line_set_line_width(gt_obj_st * line, uint16_t line_width);

/**
 * @brief Set line brush type (round cap / square cap)
 *
 * @param line line obj
 * @param type GT_BRUSH_TYPE_ROUND or GT_BRUSH_TYPE_RECT
 */
void gt_line_set_brush_type(gt_obj_st * line, uint8_t type);

/**
 * @brief Set dash style for the line.
 * @param line       The line object
 * @param dash_width Length of each dash segment in pixels; 0 = solid line
 * @param dash_gap   Length of gap between dashes in pixels; 0 = solid line
 */
void gt_line_set_dash(gt_obj_st * line, uint8_t dash_width, uint8_t dash_gap);


#endif  /** GT_CFG_ENABLE_LINE */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_LINE_H_
