/**
 * @file gt_input_number.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2023-05-26 11:50:34
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_INPUT_NUMBER_H_
#define _GT_INPUT_NUMBER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_INPUT_NUMBER
#include "gt_obj.h"
#include "../font/gt_font.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_input_number_create(gt_obj_st * parent);

/**
 * @brief Set the default value, [default: 0]
 *
 * @param obj
 * @param value The value to be set
 */
void gt_input_number_set_value(gt_obj_st * obj, double value);

/**
 * @brief Get the current value
 *
 * @param obj
 * @return double The current value
 */
double gt_input_number_get_value(gt_obj_st * obj);

/**
 * @brief The value is preceded by a 0 to the number of characters displayed
 *  example: value is "100.50", fill zero front is "4", then display "0100.50".
 *
 * @param obj
 * @param enabled true: enabled; false: disabled
 */
void gt_input_number_set_fill_zero_front(gt_obj_st * obj, bool enabled);

/**
 * @brief
 *
 * @param obj
 * @return true
 * @return false
 */
bool gt_input_number_get_fill_zero_front(gt_obj_st * obj);

/**
 * @brief Maximum value that can be set
 *
 * @param obj
 * @param max
 */
void gt_input_number_set_max(gt_obj_st * obj, double max);

/**
 * @brief The minimum configurable value
 *
 * @param obj
 * @param min
 */
void gt_input_number_set_min(gt_obj_st * obj, double min);

/**
 * @brief The amount of change per modification
 *  example:
 *      value is "100.50", step is "0.1":
 *          click up button, value is "100.60",
 *          click down button, value is "100.40".
 *
 * @param obj
 * @param step number of change per modification
 */
void gt_input_number_set_step(gt_obj_st * obj, double step);

/**
 * @brief Increment the value of step to the current value
 *
 * @param obj
 */
double gt_input_number_increase(gt_obj_st * obj);

/**
 * @brief The current value reduces the value of step
 *
 * @param obj
 */
double gt_input_number_decrease(gt_obj_st * obj);

/**
 * @brief Sets the number of characters displayed in an integer data segment
 *  example:
 *      value is "123.456", integer length set to 6, and set fill zero front it:
 *      result is "000123.456", otherwise "123.456".
 *
 * @param obj
 * @param length 0: default, auto calculate; other: set length; [max: 64]
 */
void gt_input_number_set_display_integer_length(gt_obj_st * obj, uint8_t length);

/**
 * @brief Sets the number of characters displayed in the decimal data segment
 *  example:
 *      value is "123456.7890", decimal length set to 2:
 *      result is "123456.78".
 *
 * @param obj
 * @param length 2: default; other: set length;  [max: 64]
 */
void gt_input_number_set_display_decimal_length(gt_obj_st * obj, uint8_t length);


void gt_input_number_set_font_color(gt_obj_st * obj, gt_color_t color);

void gt_input_number_set_font_size(gt_obj_st * obj, uint8_t size);

void gt_input_number_set_font_gray(gt_obj_st * obj, uint8_t gray);

void gt_input_number_set_font_align(gt_obj_st * obj, gt_align_et align);

void gt_input_number_set_font_family_cn(gt_obj_st * obj, gt_family_t family);

void gt_input_number_set_font_family_en(gt_obj_st * obj, gt_family_t family);

void gt_input_number_set_font_family_fl(gt_obj_st * obj, gt_family_t family);

void gt_input_number_set_font_family_numb(gt_obj_st * obj, gt_family_t family);

void gt_input_number_set_font_thick_en(gt_obj_st * obj, uint8_t thick);

void gt_input_number_set_font_thick_cn(gt_obj_st * obj, uint8_t thick);

void gt_input_number_set_space(gt_obj_st * obj, uint8_t space_x, uint8_t space_y);

#endif /** #if GT_CFG_ENABLE_INPUT_NUMBER */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_INPUT_NUMBER_H_
