/**
 * @file gt_keypad.h
 * @author yongg
 * @brief keypad input device
 * @version 0.1
 * @date 2022-08-01 14:05:22
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
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



/* typedef --------------------------------------------------------------*/
enum {
    GT_KEYPAD_MODE_LOWER,
    GT_KEYPAD_MODE_UPPER,
    GT_KEYPAD_MODE_NUMBER,
    GT_KEYPAD_MODE_OTHER,
};

struct _gt_keypad_map_s;
typedef struct _gt_keypad_info_s
{
    /* data */
    uint8_t mode;       /*this mode can set GT_KEYPAD_MODE_XX*/
    uint8_t cnt_lines;  /*record how many lines of the keypad*/
    uint8_t per_line[6];/*record the per of the total width of a line */
    struct _gt_keypad_map_s * map;
}_gt_keypad_info_st;


typedef struct _gt_style_keypad_s
{
    gt_obj_st * target;
    char * keypad;
    char * kv_selected;
    gt_color_t bg_color;  //background color
    gt_color_t color_font;  //font color

    /*press flag*/
    uint8_t state;   /*press:1,release:0*/
    uint8_t cnt_keydown_timers;

    _gt_keypad_info_st info;
}_gt_style_keypad_st;

/**
 * @brief Record key information
 *
 */
typedef struct _gt_keypad_map_s
{
    char * kv;
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
void gt_keypad_style_set_target(gt_obj_st * keypad, gt_obj_st * ta);

/**
 * @brief set keypad map,this can remap keypad
 *
 * @param keypad keypad obj
 * @param map map
 */
void gt_keypad_style_set_map(gt_obj_st * keypad,  gt_keypad_map_st * map);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_KEYPAD_H_
