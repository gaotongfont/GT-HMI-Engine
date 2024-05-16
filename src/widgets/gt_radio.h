/**
 * @file gt_radio.h
 * @author yongg
 * @brief The implementation of the radio
 * @version 0.1
 * @date 2022-07-20 15:29:01
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_RADIO_H_
#define _GT_RADIO_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_RADIO
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "stdarg.h"
#include "stdlib.h"
#include "../font/gt_font.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief create radio obj
 *
 * @param parent radio's parent obj
 * @return gt_obj_st* radio obj ptr
 */
gt_obj_st * gt_radio_create(gt_obj_st * parent);

/**
 * @brief set radio state to selected
 *
 * @param radio radio obj
 */
void gt_radio_set_selected(gt_obj_st * radio);

/**
 * @brief set radio text
 *
 * @param radio radio obj
 * @param fmt text format
 */
void gt_radio_set_text(gt_obj_st * radio, const char * fmt, ...);

void gt_radio_set_font_color(gt_obj_st * radio, gt_color_t color);
void gt_radio_set_font_size(gt_obj_st * radio, uint8_t size);
void gt_radio_set_font_gray(gt_obj_st * radio, uint8_t gray);
void gt_radio_set_font_family_cn(gt_obj_st * radio, gt_family_t family);
void gt_radio_set_font_family_en(gt_obj_st * radio, gt_family_t family);
void gt_radio_set_font_family_fl(gt_obj_st * radio, gt_family_t family);
void gt_radio_set_font_family_numb(gt_obj_st * radio, gt_family_t family);
void gt_radio_set_font_thick_en(gt_obj_st * radio, uint8_t thick);
void gt_radio_set_font_thick_cn(gt_obj_st * radio, uint8_t thick);
void gt_radio_set_font_encoding(gt_obj_st * radio, gt_encoding_et encoding);
void gt_radio_set_space(gt_obj_st * radio, uint8_t space_x, uint8_t space_y);

void gt_radio_set_font_point_offset_x(gt_obj_st * radio , gt_size_t x);
void gt_radio_set_font_point_offset_y(gt_obj_st * radio , gt_size_t y);
void gt_radio_set_font_point_offset(gt_obj_st * radio , gt_size_t x , gt_size_t y);


#endif  /** GT_CFG_ENABLE_RADIO */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_RADIO_H_
