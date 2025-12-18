/**
 * @file gt_lrc.h
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2025-11-17 19:35:16
 * @copyright Copyright (c) 2014-2025, Company Genitop. Co., Ltd.
 */
#ifndef _GT_LRC_H_
#define _GT_LRC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_LRC
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "../font/gt_font.h"
/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief create a lrc widget
 *
 * @param parent
 * @return gt_obj_st*
 */
gt_obj_st * gt_lrc_create(gt_obj_st * parent);

/**
 * @brief set lrc border width
 *
 * @param obj
 * @param width
 */
void gt_lrc_set_border_width(gt_obj_st * obj, uint8_t width);

/**
 * @brief set lrc border color
 *
 * @param obj
 * @param color
 */
void gt_lrc_set_border_color(gt_obj_st * obj, gt_color_t color);

/**
 * @brief add a text to lrc
 *
 * @param obj
 * @param time  time in ms
 * @param text  text string
 * @return gt_obj_st*
 */
gt_obj_st* gt_lrc_add_text(gt_obj_st* obj, uint32_t time, const char* text);
gt_obj_st* gt_lrc_add_text_by_len(gt_obj_st* obj, uint32_t time, const char* text, uint16_t len);

/**
 * @brief clear all text
 *
 * @param obj
 */
void gt_lrc_clear_all_text(gt_obj_st* obj);

/**
 * @brief start lrc
 *
 * @param obj
 */
void gt_lrc_start(gt_obj_st* obj);

/**
 * @brief pause lrc
 *
 * @param obj
 */
void gt_lrc_paused(gt_obj_st* obj);

/**
 * @brief resume lrc
 *
 * @param obj
 */
void gt_lrc_resume(gt_obj_st* obj);

/**
 * @brief set select color
 *
 * @param obj
 * @param color
 */
void gt_lrc_set_select_color(gt_obj_st * obj, gt_color_t color);
/**
 * @brief set select index
 *
 * @param obj
 * @param index
 */
void gt_lrc_set_select_index(gt_obj_st* obj, uint32_t index);

/**
 * @brief set fixed line
 *
 * @param obj
 * @param line
 */
void gt_lrc_set_fixed_line(gt_obj_st* obj, uint32_t line);

/**
 * @brief correction time
 *
 * @param obj
 * @param time
 */
void gt_lrc_correction_time(gt_obj_st* obj, uint32_t time);

/**
 * @brief set space between text
 *
 * @param obj
 * @param space_x
 * @param space_y
 */
void gt_lrc_set_space(gt_obj_st * obj, uint8_t space_x, uint8_t space_y);

void gt_lrc_set_font_color(gt_obj_st* obj, gt_color_t color);
void gt_lrc_set_font_size(gt_obj_st * obj, uint8_t size);
void gt_lrc_set_font_gray(gt_obj_st * obj, uint8_t gray);
void gt_lrc_set_font_thick_en(gt_obj_st * obj, uint8_t thick);
void gt_lrc_set_font_thick_cn(gt_obj_st * obj, uint8_t thick);
void gt_lrc_set_font_encoding(gt_obj_st * obj, gt_encoding_et encoding);
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
void gt_lrc_set_font_family_cn(gt_obj_st* obj, gt_family_t family);
void gt_lrc_set_font_family_en(gt_obj_st* obj, gt_family_t family);
void gt_lrc_set_font_family_fl(gt_obj_st* obj, gt_family_t family);
void gt_lrc_set_font_family_numb(gt_obj_st* obj, gt_family_t family);
#else
void gt_lrc_set_font_family(gt_obj_st* obj, gt_family_t family);
void gt_lrc_set_font_cjk(gt_obj_st* obj, gt_font_cjk_et cjk);
#endif
void gt_lrc_set_font_align(gt_obj_st * obj, gt_align_et align);


//
#endif // !GT_CFG_ENABLE_LRC
//
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_LRC_H_

/* end of file ----------------------------------------------------------*/


