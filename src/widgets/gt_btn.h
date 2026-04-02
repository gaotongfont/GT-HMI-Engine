/**
 * @file gt_btn.h
 * @author yongg
 * @brief button element
 * @version 0.1
 * @date 2022-05-11 18:39:54
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_BTN_H_
#define _GT_BTN_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"
#include "../core/effects/gt_effect_click.h"

#if GT_CFG_ENABLE_BTN
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "stdarg.h"
#include "stdlib.h"
#include "../utils/gt_vector.h"
#include "../core/gt_style.h"
#include "../font/gt_font.h"
#include "../utils/gt_roll_anim.h"

/* define ---------------------------------------------------------------*/

#ifndef GT_BTN_DEFAULT_AUTO_SCROLL_MODE
    /**
     * @brief set default auto scroll mode
     *      [default: 0]
     */
    #define GT_BTN_DEFAULT_AUTO_SCROLL_MODE       0
#endif

#ifndef GT_BTN_DEFAULT_AUTO_SCROLL_TIME_MODE
    /**
     * @brief set default auto scroll time mode
     *      [default: 0]        Total rolling duration
     *                1         Duration of single character scrolling
     */
    #define GT_BTN_DEFAULT_AUTO_SCROLL_TIME_MODE       0
#endif

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
 * @brief set the border color
 *
 * @param btn button obj
 * @param color border color
 */
void gt_btn_set_color_border(gt_obj_st * btn, gt_color_t);

/**
 * @brief set the border color
 *
 * @param btn button obj
 * @param width border width
 */
void gt_btn_set_border_width(gt_obj_st * btn, uint8_t width);

/**
 * @brief set button text
 *
 * @param btn button obj
 * @param fmt text format
 */
void gt_btn_set_text(gt_obj_st * btn, const char * fmt, ...);

/**
 * @brief set button text by len
 *
 * @param btn
 * @param text
 * @param len the byte length of the text
 */
void gt_btn_set_text_by_len(gt_obj_st * btn, const char * text, uint16_t len);

void gt_btn_append_text(gt_obj_st * obj, const char * str);
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
void gt_btn_set_font_color_pressed(gt_obj_st * btn, gt_color_t color);
void gt_btn_set_font_size(gt_obj_st * btn, uint8_t size);
void gt_btn_set_font_gray(gt_obj_st * btn, uint8_t gray);
void gt_btn_set_font_align(gt_obj_st * btn, gt_align_et align);
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
void gt_btn_set_font_family_cn(gt_obj_st * btn, gt_family_t font_family_cn);
void gt_btn_set_font_family_en(gt_obj_st * btn, gt_family_t font_family_en);
void gt_btn_set_font_family_fl(gt_obj_st * btn, gt_family_t font_family_fl);
void gt_btn_set_font_family_numb(gt_obj_st * btn, gt_family_t font_family_numb);
#else
void gt_btn_set_font_family(gt_obj_st * btn, gt_family_t font_family);
void gt_btn_set_font_cjk(gt_obj_st* btn, gt_font_cjk_et cjk);
#endif
void gt_btn_set_font_thick_en(gt_obj_st * btn, uint8_t thick);
void gt_btn_set_font_thick_cn(gt_obj_st * btn, uint8_t thick);
void gt_btn_set_font_encoding(gt_obj_st * btn, gt_encoding_et encoding);
void gt_btn_set_font_style(gt_obj_st * btn, gt_font_style_et font_style);
void gt_btn_set_space(gt_obj_st * btn, uint8_t space_x, uint8_t space_y);

void gt_btn_set_indent(gt_obj_st * btn, uint16_t indent);

/**
 * @brief set btn radius
 *
 * @param btn button obj
 * @param radius radius size
 */
void gt_btn_set_radius(gt_obj_st * btn, gt_radius_t radius);

/**
 * @brief Control which corners of the button are rounded independently.
 *        Disabled corners are filled with the parent background color,
 *        effectively making them square.
 *
 * @param btn  button obj
 * @param tl   top-left corner rounded (true) or square (false)
 * @param tr   top-right corner rounded (true) or square (false)
 * @param bl   bottom-left corner rounded (true) or square (false)
 * @param br   bottom-right corner rounded (true) or square (false)
 */
void gt_btn_set_corner_radius(gt_obj_st * btn, bool tl, bool tr, bool bl, bool br);

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

/**
 * @brief Set the btn selected state
 *
 * @param obj
 */
void gt_btn_set_selected(gt_obj_st * obj);

/**
 * @brief Close state change function.
 *
 * @param obj
 * @param enabled 1[default]: enabled; 0: disalbed.
 */
void gt_btn_set_state_enabled(gt_obj_st * obj, bool enabled);

/**
 * @brief Get the font split line number
 *
 * @param obj
 * @param sp_line
 * @param ret_max_w
 * @return uint32_t
 */
gt_split_line_ret_st gt_btn_get_font_split_line_numb(gt_obj_st* obj, gt_font_split_line_st* sp_line);

/**
 * @brief Set auto scroll single line
 *
 * @param obj
 * @param is_auto_scroll true: auto scroll single line, false: omit single line
 */
void gt_btn_set_auto_scroll_single_line(gt_obj_st * obj, bool is_auto_scroll);

/**
 * @brief Set auto scroll speed
 *
 * @param btn
 * @param total_time_ms > 0: total time(ms) for auto scroll each time;
 *                     = 0: set default 2s.
 */
void gt_btn_set_auto_scroll_total_time(gt_obj_st * btn, uint32_t total_time_ms);

/**
 * @brief Set the duration required for a character to scroll
 *
 * @param btn
 * @param time_ms > 0: The time(ms) for a single character to scroll
 *                = 0: Set default total duration 2s
 */
void gt_btn_set_one_word_scroll_time(gt_obj_st * btn, uint32_t time_ms);

bool gt_btn_is_single_line(gt_obj_st * btn);
bool gt_btn_is_omit_single_line(gt_obj_st * btn);
bool gt_btn_is_auto_scroll_single_line(gt_obj_st * btn);

/**
 * @brief Set the offset of the display area when the btn is scrolling
 *
 * @param btn
 * @param roll_space
 */
void gt_btn_set_roll_space(gt_obj_st * btn, uint8_t roll_space);
uint8_t gt_btn_get_roll_space(gt_obj_st * btn);

/**
 * @brief
 *
 * @param btn
 * @param paused true: paused
 * @return gt_res_t
 */
gt_res_t gt_btn_scroll_pause(gt_obj_st * btn, bool paused);

#endif  /** GT_CFG_ENABLE_BTN */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_BTN_H_
