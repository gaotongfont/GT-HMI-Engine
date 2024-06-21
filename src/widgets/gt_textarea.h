/**
 * @file gt_textarea.h
 * @author yongg
 * @brief Controllers for the text area
 * @version 0.1
 * @date 2022-07-13 09:47:27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_TEXTAREA_H_
#define _GT_TEXTAREA_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_TEXTAREA
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "../font/gt_font.h"
#include "../core/gt_draw.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief create a textarea obj
 *
 * @param parent textarea's parent obj
 * @return gt_obj_st* textarea obj
 */
gt_obj_st * gt_textarea_create(gt_obj_st * parent);

/**
 * @brief set string to textarea widget
 *
 * @param textarea obj pointer
 * @param text set text content
 */
void gt_textarea_set_text(gt_obj_st * textarea, char * text);

/**
 * @brief add string to textarea widget with style
 *
 * @param textarea obj pointer
 * @param str text
 * @param style font style @ref gt_font_family_et
 * @param color font color
 */
void gt_textarea_add_str(gt_obj_st * textarea, char * str, gt_font_style_et style, gt_color_t color);

/**
 * @brief set textarea space
 *
 * @param textarea obj pointer
 * @param space_x
 * @param space_y
 */
void gt_textarea_set_space(gt_obj_st * textarea, uint8_t space_x, uint8_t space_y);

/**
 * @brief set textarea fonts size
 *
 * @param textarea obj pointer
 * @param size  font size
 */
void gt_textarea_set_font_size(gt_obj_st * textarea, uint8_t size);
void gt_textarea_set_font_gray(gt_obj_st * textarea, uint8_t gray);

/**
 * @brief Set font alignment
 *      NOTE textarea only support GT_ALIGN_LEFT
 *
 * @param textarea
 * @param align
 */
void gt_textarea_set_font_align(gt_obj_st * textarea, gt_align_et align);
/**
 * @brief set textarea font color
 *
 * @param textarea obj pointer
 * @param color  font color
 */
void gt_textarea_set_font_color(gt_obj_st * textarea, gt_color_t color);

/**
 * @brief set textarea background color
 *
 * @param textarea obj pointer
 * @param color  font color
 */
void gt_textarea_set_bg_color(gt_obj_st * textarea, gt_color_t color);

/**
 * @brief set textarea background opacity
 *
 * @param textarea
 * @param opa @ref gt_color.h GT_OPA_0 ~ GT_OPA_100: 0 ~ 100% display
 */
void gt_textarea_set_bg_opa(gt_obj_st * textarea, uint8_t opa);

/**
 * @brief Set the border color of the textarea
 *
 * @param textarea
 * @param width 2[defalut]: 2px
 */
void gt_textarea_set_border_width(gt_obj_st * textarea, uint8_t width);

/**
 * @brief Set the border color of the textarea
 *
 * @param textarea
 * @param color 0xc7c7c7[defalut]
 */
void gt_textarea_set_border_color(gt_obj_st * textarea, gt_color_t color);

/**
 * @brief Set the border radius of the textarea
 *
 * @param textarea
 * @param radius 4[defalut]
 */
void gt_textarea_set_radius(gt_obj_st * textarea, gt_radius_t radius);

/**
 * @brief set textarea chinese font style
 *
 * @param textarea obj pointer
 * @param font_family_cn chinese font style
 */
void gt_textarea_set_font_family_cn(gt_obj_st * textarea, gt_family_t family);

/**
 * @brief set textarea english font style
 *
 * @param textarea obj pointer
 * @param font_family_cn english font style
 */
void gt_textarea_set_font_family_en(gt_obj_st * textarea, gt_family_t family);
void gt_textarea_set_font_family_fl(gt_obj_st * textarea, gt_family_t family);
void gt_textarea_set_font_family_numb(gt_obj_st * textarea, gt_family_t family);

void gt_textarea_set_font_thick_en(gt_obj_st * textarea, uint8_t thick);
void gt_textarea_set_font_thick_cn(gt_obj_st * textarea, uint8_t thick);
void gt_textarea_set_font_encoding(gt_obj_st * textarea, gt_encoding_et encoding);

#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
gt_font_touch_word_st gt_textarea_get_touch_word(gt_obj_st * textarea);
#endif


#endif  /** GT_CFG_ENABLE_TEXTAREA */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_TEXTAREA_H_
