/**
 * @file gt_select.h
 * @author Feyoung
 * @brief Combined controls are composed of input box controls and list controls.
 *      Listening GT_EVENT_TYPE_UPDATE_VALUE event to get the selected gt_input_hide_value.
 *      Get the select item text such as:
 *      ```
 *      static void _select_cb(gt_event_st * e) {
 *      	if (e->param) {
 *      	  GT_LOG_A("", "select: [%s]", e->param);
 *      	}
 *      }
 *      ```
 * @version 0.1
 * @date 2024-12-03 15:59:26
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SELECT_H_
#define _GT_SELECT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_SELECT
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "stdarg.h"
#include "stdlib.h"
#include "../core/gt_style.h"
#include "../font/gt_font.h"



/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief Create a select widget, detail @see gt_select.h's "brief" comments
 *
 * @param parent
 * @return gt_obj_st*
 */
gt_obj_st * gt_select_create(gt_obj_st * parent);

void gt_select_add_option(gt_obj_st * obj, const char * text);
void gt_select_clear_all_options(gt_obj_st * obj);
void gt_select_set_option_height(gt_obj_st * obj, uint16_t height);

void gt_select_set_font_color(gt_obj_st * select, gt_color_t color);
void gt_select_set_font_size(gt_obj_st * select, uint8_t size);
void gt_select_set_font_gray(gt_obj_st * select, uint8_t gray);
void gt_select_set_font_align(gt_obj_st * select, gt_align_et align);
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
void gt_select_set_font_family_cn(gt_obj_st * select, gt_family_t family);
void gt_select_set_font_family_en(gt_obj_st * select, gt_family_t family);
void gt_select_set_font_family_fl(gt_obj_st * select, gt_family_t family);
void gt_select_set_font_family_numb(gt_obj_st * select, gt_family_t family);
#else
void gt_select_set_font_family(gt_obj_st * select, gt_family_t family);
void gt_select_set_font_cjk(gt_obj_st* select, gt_font_cjk_et cjk);
#endif
void gt_select_set_font_thick_en(gt_obj_st * select, uint8_t thick);
void gt_select_set_font_thick_cn(gt_obj_st * select, uint8_t thick);
void gt_select_set_font_encoding(gt_obj_st * select, gt_encoding_et encoding);

void gt_select_set_font_style(gt_obj_st * select, gt_font_style_et font_style);

/**
 * @brief  Set the font to be reversed color when the item is selected
 *
 * @param select
 * @param enabled 0[default]: no font change color when item selected; 1: font change color when item selected
 */
void gt_select_set_selected_anti_font(gt_obj_st * select, bool enabled);
bool gt_select_is_selected_anti_font(gt_obj_st * select);
void gt_select_set_selected_anti_font_color(gt_obj_st * select, gt_color_t anti_color);
gt_color_t gt_select_get_selected_anti_font_color(gt_obj_st * select);

#endif  /** GT_CFG_ENABLE_SELECT */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SELECT_H_
