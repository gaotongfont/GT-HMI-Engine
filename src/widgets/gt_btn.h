/**
 * @file gt_btn.h
 * @author yongg
 * @brief button element
 * @version 0.1
 * @date 2022-05-11 18:39:54
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_BTN_H_
#define _GT_BTN_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "stdarg.h"
#include "stdlib.h"
#include "../utils/gt_vector.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief create button obj
 *
 * @param parent button's parent obj
 * @return gt_obj_st* button obj ptr
 */
gt_obj_st * gt_btn_create(gt_obj_st * parent);

/**
 * @brief set the color when the button is selected
 *
 * @param btn button obj
 * @param color selected color
 */
void gt_btn_set_color_pressed(gt_obj_st * btn, gt_color_t color);

/**
 * @brief set the color when the button is unselected
 *
 * @param btn button obj
 * @param color unselected color
 */
void gt_btn_set_color_background(gt_obj_st * btn, gt_color_t color);

/**
 * @brief set button text
 *
 * @param btn button obj
 * @param fmt text format
 */
void gt_btn_set_text(gt_obj_st * btn, const char * fmt, ...);

/**
 * @brief get button text
 *
 * @param btn button obj
 * @return char* button text
 */
char * gt_btn_get_text(gt_obj_st * btn);

/**
 * @brief set button font color
 *
 * @param btn button obj
 * @param color font color
 */
void gt_btn_set_font_color(gt_obj_st * btn, gt_color_t color);

void gt_btn_set_font_size(gt_obj_st * btn, uint8_t size);
void gt_btn_set_font_gray(gt_obj_st * btn, uint8_t gray);
void gt_btn_set_font_align(gt_obj_st * btn, uint8_t align);
void gt_btn_set_font_family_cn(gt_obj_st * btn, gt_family_t font_family_cn);
void gt_btn_set_font_family_en(gt_obj_st * btn, gt_family_t font_family_en);
void gt_btn_set_font_family_numb(gt_obj_st * btn, gt_family_t font_family_numb);
void gt_btn_set_font_thick_en(gt_obj_st * btn, uint8_t thick);
void gt_btn_set_font_thick_cn(gt_obj_st * btn, uint8_t thick);
void gt_btn_set_space(gt_obj_st * btn, uint8_t space_x, uint8_t space_y);

/**
 * @brief set btn radius
 *
 * @param btn button obj
 * @param radius radius size
 */
void gt_btn_set_radius(gt_obj_st * btn, uint8_t radius);

/**
 * @brief Button add switching status text content
 *
 * @param btn button obj
 * @param str
 * @return true Add successfully
 * @return false Add failed
 */
bool gt_btn_add_state_content(gt_obj_st * btn, const char * str);

/**
 * @brief Button remove switching status text content
 *
 * @param btn
 * @param str
 * @return true remove successfully
 * @return false remove failed
 */
bool gt_btn_remove_state_content(gt_obj_st * btn, const char * str);

/**
 * @brief Button clear switching status text content
 *
 * @param obj
 * @return true Clear all successfully
 * @return false Clear all failed
 */
bool gt_btn_clear_all_state_content(gt_obj_st * btn);

/**
 * @brief Get current content index
 *
 * @param obj
 * @return int16_t current content index: -1: no content
 */
int16_t gt_btn_get_state_content_index(gt_obj_st * obj);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_BTN_H_
