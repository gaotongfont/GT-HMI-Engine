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
#define KEYPAD_W    300
#define KEYPAD_H    170

/* typedef --------------------------------------------------------------*/
typedef void (* keypad_handler_cb_t)(gt_obj_st * );


struct _gt_keypad_map_s;
typedef struct _gt_keypad_info_s
{
    keypad_handler_cb_t handler_cb;
    struct _gt_keypad_map_s * map;

    /* data */
    uint8_t mode;       /*this mode can set GT_KEYPAD_MODE_XX*/
    uint8_t cnt_lines;  /*record how many lines of the keypad*/
    uint8_t per_line[32];/*record the per of the total width of a line */
}_gt_keypad_info_st;


typedef struct _gt_style_keypad_s
{
    gt_obj_st * target;
    char *      kv_selected;
    gt_color_t  color_board;  //board color
    gt_color_t  color_key;  //key color
    gt_color_t  color_ctrl;  //ctrl key color

    gt_color_t  font_color;  //font color
    gt_family_t font_family_cn;
    gt_family_t font_family_en;
    uint8_t     font_size;
    uint8_t     font_gray;

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
void gt_keypad_style_set_handler_cb(gt_obj_st * keypad, keypad_handler_cb_t cb);

void gt_keypad_style_set_color_board(gt_obj_st * keypad, gt_color_t color);
void gt_keypad_style_set_color_key(gt_obj_st * keypad, gt_color_t color);
void gt_keypad_style_set_color_ctrl_key(gt_obj_st * keypad, gt_color_t color);

void gt_keypad_style_set_font_color(gt_obj_st * keypad, gt_color_t color);
void gt_keypad_style_set_font_family_cn(gt_obj_st * keypad, gt_family_t family);
void gt_keypad_style_set_font_family_en(gt_obj_st * keypad, gt_family_t family);
void gt_keypad_style_set_font_size(gt_obj_st * keypad, uint8_t size);
void gt_keypad_style_set_font_gray(gt_obj_st * keypad, uint8_t gray);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_KEYPAD_H_
