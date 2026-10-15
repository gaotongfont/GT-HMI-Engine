/**
 * @file gt_spectrum_line.h
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2025-05-28 15:39:29
 * @copyright Copyright (c) 2014-2025, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SPECTRUM_LINE_H_
#define _GT_SPECTRUM_LINE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"
//
#if GT_CFG_ENABLE_SPECTRUM_LINE
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "../core/gt_style.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
/**
 * @brief One key point of a spectrum line.
 *
 * @note Coordinate convention (easy to mistake for a bug): the Y used for
 *       drawing is taken from get_gen_next(&gen), NOT from point.y — so
 *       gen.min / gen.max / gen.cur must be in the SAME coordinate system
 *       as point.y (screen-absolute Y, e.g. a band like 140~180). Feeding
 *       small local values (e.g. ±18) places the whole line outside the
 *       widget and it is fully clipped = looks like "not rendering".
 */
typedef struct {
    gt_point_st point;
    gt_generator_st gen;
}gt_sp_point_st;


typedef int32_t (*get_generator_next)(gt_generator_st *);

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief create a spectrum line
 *
 * @param parent parent widget
 * @return gt_obj_st*
 */
gt_obj_st * gt_spectrum_line_create(gt_obj_st * parent);

/**
 * @brief add a line to spectrum line
 *
 * @param obj spectrum line
 * @param width line width
 * @param color line color
 * @return gt_size_t line index
 */
gt_size_t gt_spectrum_line_add(gt_obj_st* obj, uint8_t width, gt_color_t color);

/**
 * @brief add a point to line
 *
 * @param obj spectrum line
 * @param line_index line index
 * @param sp_ping point
 * @return gt_size_t point index
 */
gt_size_t gt_spectrum_line_add_point(gt_obj_st* obj, uint16_t line_index ,gt_sp_point_st sp_ping);

/**
 * @brief remove a line by index
 *
 * @param obj spectrum line
 * @param line_index line index
 * @return true remove success
 * @return false remove failed
 */
bool gt_spectrum_line_remove(gt_obj_st * obj, uint16_t line_index);

/**
 * @brief remove a point from line by index
 *
 * @param obj spectrum line
 * @param line_index line index
 * @param point_index point index
 * @return true remove success
 * @return false remove failed
 */
bool gt_spectrum_line_remove_point(gt_obj_st * obj, uint16_t line_index, uint16_t point_index);

/**
 * @brief Based on the first line, replicate the other lines according to the movement pattern.
 *
 * @param obj spectrum line
 * @param count Total number of lines
 * @param start Gradient starting color
 * @param end   Gradient ending color
 */
void gt_spectrum_line_move_copy(gt_obj_st* obj, uint16_t count, gt_color_t start, gt_color_t end);

/**
 * @brief set the period of the spectrum line
 *
 * @param obj spectrum line
 * @param period period
 */
void gt_spectrum_line_set_period(gt_obj_st * obj, uint32_t period);

/**
 * @brief set the speed of the spectrum line
 *
 * @param obj
 */
void gt_spectrum_line_play(gt_obj_st * obj);

/**
 * @brief stop the spectrum line
 *
 * @param obj spectrum line
 */
void gt_spectrum_line_stop(gt_obj_st * obj);

/**
 * @brief Set the key point fluctuation function, default @ gt_triangle_generator_next()
 *
 * @param obj spectrum line
 * @param func next function
 */
void gt_spectrum_line_set_gen_next_func(gt_obj_st * obj, get_generator_next func);


#endif // !GT_CFG_ENABLE_SPECTRUM_LINE
//
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SPECTRUM_LINE_H_

/* end of file ----------------------------------------------------------*/


