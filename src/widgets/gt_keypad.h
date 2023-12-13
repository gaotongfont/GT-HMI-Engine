/**
 * @file gt_keypad.h
 * @author yongg
 * @brief keypad input device
 * @version 0.1
 * @date 2022-08-01 14:05:22
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_KEYPAD_H_
#define _GT_KEYPAD_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"

/* define ---------------------------------------------------------------*/
#define KEYPAD_W    300
#define KEYPAD_H    170

/* typedef --------------------------------------------------------------*/
typedef void (* keypad_handler_cb_t)(gt_obj_st * );

/**
 * @brief Record key information
 *
 */
typedef struct _gt_keypad_map_s
{
    char const * const kv;
    uint8_t w;
}gt_keypad_map_st;


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
 * @brief set obj for keyboard input
 *
 * @param keypad keypad obj
 * @param ta obj for keyboard input
 */
void gt_keypad_set_target(gt_obj_st * keypad, gt_obj_st * ta);

/**
 * @brief set keypad map,this can remap keypad
 *
 * @param keypad keypad obj
 * @param map map
 */
void gt_keypad_set_map(gt_obj_st * keypad,  gt_keypad_map_st * map);
void gt_keypad_set_handler_cb(gt_obj_st * keypad, keypad_handler_cb_t callback);

void gt_keypad_set_color_board(gt_obj_st * keypad, gt_color_t color);
void gt_keypad_set_color_key(gt_obj_st * keypad, gt_color_t color);
void gt_keypad_set_color_ctrl_key(gt_obj_st * keypad, gt_color_t color);

void gt_keypad_set_font_color(gt_obj_st * keypad, gt_color_t color);
void gt_keypad_set_font_family_cn(gt_obj_st * keypad, gt_family_t family);
void gt_keypad_set_font_family_en(gt_obj_st * keypad, gt_family_t family);
void gt_keypad_set_font_family_fl(gt_obj_st * keypad, gt_family_t family);
void gt_keypad_set_font_family_numb(gt_obj_st * keypad, gt_family_t family);
void gt_keypad_set_font_size(gt_obj_st * keypad, uint8_t size);
void gt_keypad_set_font_gray(gt_obj_st * keypad, uint8_t gray);
void gt_keypad_set_font_align(gt_obj_st * keypad, uint8_t align);
void gt_keypad_set_font_thick_en(gt_obj_st * keypad, uint8_t thick);
void gt_keypad_set_font_thick_cn(gt_obj_st * keypad, uint8_t thick);
void gt_keypad_set_space(gt_obj_st * keypad, uint8_t space_x, uint8_t space_y);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_KEYPAD_H_
