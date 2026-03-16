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
#include "stdarg.h"
#include "gt_obj_class.h"
#include "../font/gt_font.h"
#include "../core/gt_draw.h"

/* define ---------------------------------------------------------------*/

#ifndef GT_TEXTAREA_CUSTOM_FONT_STYLE
    /**
     * @brief 0[default]: Use textarea default font style, only support one font style;
     *        1: One of the textarea items uses custom font style, which is defined by user.
     */
    #define GT_TEXTAREA_CUSTOM_FONT_STYLE   0
#endif

#ifndef GT_TEXTAREA_SUB_STRING_SCROLL_CACHE
    /**
     * @brief Use sub string scroll cache, improve scrolling performance and takes up some memory
     *      0[default]: disable
     */
    #define GT_TEXTAREA_SUB_STRING_SCROLL_CACHE    0
#endif

#ifndef GT_TEXTAREA_LINE_CACHE
    /**
     * @brief Line-level scroll cache: records (byte_offset, y) checkpoints every
     *        GT_TEXTAREA_LINE_CACHE_STEP lines so that draw_text can skip
     *        gt_font_split_line_str calls for off-screen lines above the viewport.
     *      0[default]: disable
     *      1: enable
     */
    #define GT_TEXTAREA_LINE_CACHE  0
#endif

#ifndef GT_TEXTAREA_LINE_CACHE_STEP
    /**
     * @brief How many lines between two adjacent line-cache checkpoints.
     *        Smaller = faster scroll seek, more memory.
     *        Larger  = slower seek (re-scan up to STEP-1 lines), less memory.
     *      8[default]
     */
    #define GT_TEXTAREA_LINE_CACHE_STEP  8
#endif


/* typedef --------------------------------------------------------------*/
typedef struct gt_textarea_param_s {
    char *      text;
    uint16_t    len;
#if GT_TEXTAREA_CUSTOM_FONT_STYLE
    gt_font_info_st font_info;
#else
    gt_color_t  color;
#endif
    uint8_t mask_style : 7;     /** @ref gt_font_style_et */
    uint8_t new_line   : 1;     /** 0[default] */
}gt_textarea_param_st;


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
 * @brief Always scroll to the newest string, when add new string by
 *          anim event.
 *
 * @param textarea
 * @param always_scroll_to_newest
 * @return gt_res_t
 */
gt_res_t gt_textarea_set_always_scroll_to_newest(gt_obj_st * textarea, bool always_scroll_to_newest);

/**
 * @brief set string to textarea widget
 *
 * @param textarea obj pointer
 * @param text set text content
 */
// void gt_textarea_set_text(gt_obj_st * textarea, char * text);
void gt_textarea_set_text(gt_obj_st * textarea, const char * fmt, ...);

void gt_textarea_set_text_by_len(gt_obj_st * textarea, const char * text, uint16_t len);

/**
 * @brief hide textarea background
 *
 * @param textarea
 * @param hide true: hide; false[default]: show
 */
void gt_textarea_hide_background(gt_obj_st * textarea, bool hide);

/**
 * @brief Get string from textarea widget
 *
 * @param textarea
 * @param idx
 * @return char*
 */
char * gt_textarea_get_text(gt_obj_st * textarea,uint16_t idx);

/**
 * @brief Clear all string in textarea widget
 *
 * @param textarea
 */
void gt_textarea_clear_all_str(gt_obj_st * textarea);

/**
 * @brief add string to textarea widget with style
 *
 * @param textarea obj pointer
 * @param str text
 * @param style font style @ref gt_font_style_et
 * @param color font color
 */
void gt_textarea_add_str(gt_obj_st * textarea, char * str, gt_font_style_et style, gt_color_t color);
void gt_textarea_add_str_by_len(gt_obj_st * textarea, char * str, uint32_t len, gt_font_style_et style_mask, gt_color_t color);

uint16_t gt_textarea_get_str_count(gt_obj_st * textarea);

#if GT_TEXTAREA_CUSTOM_FONT_STYLE
/**
 * @brief Set text by custom font style
 *
 * @param textarea
 * @param param @ref gt_textarea_param_st
 */
void gt_textarea_add_str_by_param(gt_obj_st * textarea, gt_textarea_param_st * param);
#endif
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

void gt_textarea_set_indent(gt_obj_st * textarea, uint16_t indent);

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
 * @param width 2[default]: 2px
 */
void gt_textarea_set_border_width(gt_obj_st * textarea, uint8_t width);

/**
 * @brief Set the border color of the textarea
 *
 * @param textarea
 * @param color 0xc7c7c7[default]
 */
void gt_textarea_set_border_color(gt_obj_st * textarea, gt_color_t color);

/**
 * @brief Set the border radius of the textarea
 *
 * @param textarea
 * @param radius 4[default]
 */
void gt_textarea_set_radius(gt_obj_st * textarea, gt_radius_t radius);

/**
 * @brief set textarea chinese font style
 *
 * @param textarea obj pointer
 * @param font_family_cn chinese font style
 */
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
void gt_textarea_set_font_family_cn(gt_obj_st * textarea, gt_family_t family);
void gt_textarea_set_font_family_en(gt_obj_st * textarea, gt_family_t family);
void gt_textarea_set_font_family_fl(gt_obj_st * textarea, gt_family_t family);
void gt_textarea_set_font_family_numb(gt_obj_st * textarea, gt_family_t family);
#else
void gt_textarea_set_font_family(gt_obj_st * textarea, gt_family_t family);
void gt_textarea_set_font_cjk(gt_obj_st* textarea, gt_font_cjk_et cjk);
#endif
void gt_textarea_set_font_thick_en(gt_obj_st * textarea, uint8_t thick);
void gt_textarea_set_font_thick_cn(gt_obj_st * textarea, uint8_t thick);
void gt_textarea_set_font_encoding(gt_obj_st * textarea, gt_encoding_et encoding);

void gt_textarea_set_font_info(gt_obj_st * textarea, gt_font_info_st * font_info);
gt_font_info_st * gt_textarea_get_font_info(gt_obj_st * textarea);

/**
 * @brief get the total height of the textarea
 *
 * @param textarea
 * @return uint32_t height pixel value
 */
uint32_t gt_textarea_get_total_height(gt_obj_st * textarea);

/**
 * @brief get the display offset of the textarea
 *
 * @param textarea
 * @return uint32_t offset pixel value
 */
uint32_t gt_textarea_get_display_offset(gt_obj_st * textarea);

/**
 * @brief Get the scroll percentage of the textarea
 *
 * @param textarea
 * @return float range: [0.00%, 100.00%]
 */
float gt_textarea_get_scroll_percent(gt_obj_st * textarea);

/**
 * @brief set the textarea display area by percentage
 *
 * @param textarea
 * @param percent float range: [0.00%, 100.00%)
 * @return gt_res_t
 */
gt_res_t gt_textarea_display_by_precent(gt_obj_st * textarea, float percent);

#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
void gt_textarea_set_touch_single_chinese_word(gt_obj_st * textarea, bool is_single_cn);
gt_font_touch_word_st gt_textarea_get_touch_word(gt_obj_st * textarea);
#endif

/**
 * @brief set textarea font padding
 *
 * @param textarea
 * @param padding [default: 0] The width of padding
 */
void gt_textarea_set_padding(gt_obj_st * textarea, uint8_t padding);

/**
 * @brief get the total line number of the textarea
 *
 * @param obj textarea obj pointer
 * @param sp_line split line info
 * @return gt_split_line_ret_st
 */
gt_split_line_ret_st gt_textarea_get_font_split_line_numb(gt_obj_st* obj, gt_font_split_line_st* sp_line);

#if 0
/**
 * @brief append text to textarea
 *
 * @param obj
 * @param str
 */
void gt_textarea_append_text(gt_obj_st * obj, const char * str);
#endif


/**
 * @brief set textarea start point offset
 *
 * @param textarea
 * @param x_off
 * @param y_off
 */
void gt_textarea_set_start_point_offset(gt_obj_st * textarea, uint8_t x_off, uint8_t y_off);


#endif  /** GT_CFG_ENABLE_TEXTAREA */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_TEXTAREA_H_
