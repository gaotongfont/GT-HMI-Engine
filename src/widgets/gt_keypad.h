/**
 * @file gt_keypad.h
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-03-04 14:00:57
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */
#ifndef _GT_KEYPAD_H_
#define _GT_KEYPAD_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_KEYPAD
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "gt_btnmap.h"
/* define ---------------------------------------------------------------*/
#define _DEF_STYLE_26_KEY
// #define _DEF_STYLE_9_KEY     //unrealized
#define _DEF_STYLE_OTHER_4x5
#define _DEF_STYLE_OTHER_11x3

/* typedef --------------------------------------------------------------*/
typedef enum {
    GT_KEYPAD_TYPE_NONE = 0,
    GT_KEYPAD_TYPE_LOWER,
    GT_KEYPAD_TYPE_UPPER,
    GT_KEYPAD_TYPE_NUMBER,
    GT_KEYPAD_TYPE_SYMBOL_EN,
    GT_KEYPAD_TYPE_CH,
    GT_KEYPAD_TYPE_SYMBOL_CH,
    GT_KEYPAD_MODE_OTHER,
}gt_keypad_type_te;

typedef struct gt_keypad_map_s {
    gt_keypad_type_te type;
    const gt_map_st* map;
}gt_keypad_map_st;

/**
 * @brief The default style is 0 after map list is set
 */
typedef enum {
    GT_KEYPAD_STYLE_NONE = 0,
#ifdef _DEF_STYLE_26_KEY
    GT_KEYPAD_STYLE_26_KEY,     // 26-key keyboard
#endif
#ifdef _DEF_STYLE_9_KEY
    GT_KEYPAD_STYLE_9_KEY,      // 9-key keyboard
#endif
#ifdef _DEF_STYLE_OTHER_4x5
    GT_KEYPAD_STYLE_OTHER_4x5,  // col:4  row: 5
#endif
#ifdef _DEF_STYLE_OTHER_11x3
    GT_KEYPAD_STYLE_OTHER_11x3, // col:11 row: 3
#endif
}gt_keypad_default_style_st;
/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief create keypad obj
 *
 * @param parent keypad's parent obj
 * @return gt_obj_st* keypad obj
 */
gt_obj_st * gt_keypad_create(gt_obj_st * parent);

/**
 * @brief
 *
 * @param keypad
 * @param def_style The default style is 0 after map list is set
 */
void gt_keypad_set_default_style(gt_obj_st * keypad, gt_keypad_default_style_st def_style);
void gt_keypad_set_type(gt_obj_st * keypad, gt_keypad_type_te type);
void gt_keypad_set_map_list(gt_obj_st * keypad, gt_keypad_map_st* map_list, uint8_t map_num);

void gt_keypad_set_target(gt_obj_st * keypad, gt_obj_st * ta);
void gt_keypad_set_color_background(gt_obj_st * keypad, gt_color_t color);
void gt_keypad_set_border_width_and_color(gt_obj_st * keypad, gt_size_t width, gt_color_t color);
void gt_keypad_set_color_ctrl_key(gt_obj_st * keypad, gt_color_t color);
void gt_keypad_set_radius(gt_obj_st * keypad, uint8_t radius);

void gt_keypad_set_key_color_background(gt_obj_st * keypad, gt_color_t color);
void gt_keypad_set_key_border_width_and_color(gt_obj_st * keypad, gt_size_t width, gt_color_t color);
void gt_keypad_set_key_radius(gt_obj_st * keypad, uint8_t radius);
void gt_keypad_set_key_height(gt_obj_st * keypad, uint16_t height);
void gt_keypad_set_key_height_auto_fill(gt_obj_st* keypad, bool auto_fill);
void gt_keypad_set_key_xy_space(gt_obj_st* keypad, uint8_t x_space, uint8_t y_space);

void gt_keypad_set_font_color(gt_obj_st * keypad, gt_color_t color);
void gt_keypad_set_font_size(gt_obj_st * keypad, uint8_t size);
void gt_keypad_set_font_gray(gt_obj_st * keypad, uint8_t gray);
void gt_keypad_set_font_align(gt_obj_st * keypad, uint8_t align);
void gt_keypad_set_font_family_cn(gt_obj_st * keypad, gt_family_t family);
void gt_keypad_set_font_family_en(gt_obj_st * keypad, gt_family_t family);
void gt_keypad_set_font_family_fl(gt_obj_st * keypad, gt_family_t family);
void gt_keypad_set_font_family_numb(gt_obj_st * keypad, gt_family_t family);
void gt_keypad_set_font_thick_en(gt_obj_st * keypad, uint8_t thick);
void gt_keypad_set_font_thick_cn(gt_obj_st * keypad, uint8_t thick);

void gt_keypad_set_ctrl_key_color_background(gt_obj_st * keypad, gt_color_t color);
void gt_keypad_set_ctrl_key_border_width_and_color(gt_obj_st * keypad, gt_size_t width, gt_color_t color);
void gt_keypad_set_ctrl_key_font_color(gt_obj_st * keypad, gt_color_t color);

void gt_keypad_set_py_input_method(gt_obj_st * keypad, gt_py_input_method_st* py_input_method);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //GT_CFG_ENABLE_KEYPAD

#endif //!_GT_KEYPAD_H_

/* end of file ----------------------------------------------------------*/
