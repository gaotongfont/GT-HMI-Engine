/**
 * @file gt_status_bar.h
 * @author Feyoung
 * @brief Status bar widget
 *  |-----------|----------|-----------|
 *  |  left     |  center  |  right    |
 *  |-----------|----------|-----------|
 * left:   icon and text
 * center: only one text title
 * right:  icon and text
 *
 * @version 0.1
 * @date 2024-03-29 10:51:59
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_STATUS_BAR_H_
#define _GT_STATUS_BAR_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_STATUS_BAR
#include "./gt_obj.h"
#include "../core/gt_style.h"


/* define ---------------------------------------------------------------*/
#ifndef GT_STATUS_BAR_THREE_PART_SCALE_MODE
    /**
     * @brief Use three part scale mode, left center right part
     */
    #define GT_STATUS_BAR_THREE_PART_SCALE_MODE     01
#endif

#ifndef GT_STATUS_BAR_CUSTOM_MODE
    /**
     * @brief Use custom mode, use gt_obj_set_pos(), etc. to set
     *      custom position.
     */
    #define GT_STATUS_BAR_CUSTOM_MODE               01
#endif


/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a status bar object, the status bar is a global object
 *
 * @param have_background true: have background, white color, false: no background
 * @return gt_obj_st* status bar object
 */
gt_obj_st * gt_status_bar_create(bool have_background);

gt_obj_st * gt_status_bar_get_obj(void);

/**
 * @brief Hide or show status bar within the current screen
 *
 * @param hide true: hide, false[default]: show
 * @return true hide
 * @return false show
 */
bool gt_status_bar_set_hide(bool hide);

/**
 * @brief Get status bar hide status
 *
 * @return true hide
 * @return false show
 */
bool gt_status_bar_is_hide(void);

void gt_status_bar_set_height(uint16_t height);

/* ------------------- left center right part ------------------- */
#if GT_STATUS_BAR_THREE_PART_SCALE_MODE
/**
 * @brief Set three part of display scale value,
 * such: left_width = screen_width * left / (left + center + right)
 * [default left: 30, center: 40, right: 30]
 *
 * @param left 0 ~ 255
 * @param center 0 ~ 255
 * @param right 0 ~ 255
 */
void gt_status_bar_set_scale(uint8_t left, uint8_t center, uint8_t right);

gt_obj_st * gt_status_bar_left_find_by(char * text_or_src);
bool gt_status_bar_left_remove(gt_obj_st * target);
bool gt_status_bar_left_remove_all(void);

gt_obj_st * gt_status_bar_left_add_icon(char * src);
bool gt_status_bar_left_change_icon(gt_obj_st * icon, char * src);
gt_obj_st * gt_status_bar_left_add_text(char * text, uint16_t width);
bool gt_status_bar_left_change_text(gt_obj_st * lab, char * text);

gt_obj_st * gt_status_bar_right_find_by(char * text_or_src);
bool gt_status_bar_right_remove(gt_obj_st * target);
bool gt_status_bar_right_remove_all(void);

gt_obj_st * gt_status_bar_right_add_icon(char * src);
bool gt_status_bar_right_change_icon(gt_obj_st * icon, char * src);
gt_obj_st * gt_status_bar_right_add_text(char * text, uint16_t width);
bool gt_status_bar_right_change_text(gt_obj_st * lab, char * text);

bool gt_status_bar_center_remove_all(void);
gt_obj_st * gt_status_bar_center_set_text(char * text);

#endif  /** GT_STATUS_BAR_THREE_PART_SCALE_MODE */

/* ------------------- custom ------------------- */
#if GT_STATUS_BAR_CUSTOM_MODE

/**
 * @brief Find custom status bar object by text or src
 *
 * @param target
 * @return true
 * @return false
 */
gt_obj_st * gt_status_bar_custom_find_by(char * text_or_src);
bool gt_status_bar_custom_remove(gt_obj_st * target);
bool gt_status_bar_custom_remove_all(void);

gt_obj_st * gt_status_bar_custom_add_icon(char * src);
bool gt_status_bar_custom_change_icon(gt_obj_st * icon, char * src);
gt_obj_st * gt_status_bar_custom_add_text(char * text, uint16_t width);
bool gt_status_bar_custom_change_text(gt_obj_st * lab, char * text);

#endif  /** GT_STATUS_BAR_CUSTOM_MODE */

/* ------------------- common ------------------- */

/**
 * @brief Show status bar background
 *
 * @param show true: show, false: hide
 */
void gt_status_bar_show_bg(bool show);

/**
 * @brief Set status bar background opacity
 *
 * @param opa @ref gt_opa_t
 */
void gt_status_bar_set_bg_opa(gt_opa_t opa);

/**
 * @brief Set status bar background color
 *
 * @param color
 */
void gt_status_bar_set_bg_color(gt_color_t color);

void gt_status_bar_set_font_family_cn(gt_family_t family);
void gt_status_bar_set_font_family_en(gt_family_t family);
void gt_status_bar_set_font_family_fl(gt_family_t family);
void gt_status_bar_set_font_family_numb(gt_family_t family);
void gt_status_bar_set_font_size(uint8_t size);
void gt_status_bar_set_font_gray(uint8_t gray);
void gt_status_bar_set_font_color(gt_color_t color);
void gt_status_bar_set_font_thick_cn(uint8_t thick);
void gt_status_bar_set_font_thick_en(uint8_t thick);

#endif  /** GT_CFG_ENABLE_STATUS_BAR */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_STATUS_BAR_H_
