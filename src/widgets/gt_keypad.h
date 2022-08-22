/**
 * @file gt_keypad.h
 * @author yongg
 * @brief 
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
typedef struct _gt_style_keypad_s
{
    char * keypad;
    char code_selected;
    gt_color_t bg_color;  //background color
    gt_color_t color_font;  //font color
}_gt_style_keypad_st;


typedef struct _gt_keypad_map_s
{
    char ch;
    gt_area_st area;
}gt_keypad_map_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_keypad_create(gt_obj_st * parent);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_KEYPAD_H_
