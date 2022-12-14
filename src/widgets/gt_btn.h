/**
 * @file gt_btn.h
 * @author yongg
 * @brief button element
 * @version 0.1
 * @date 2022-05-11 18:39:54
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_BTN_H_
#define _GT_BTN_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct _gt_style_btn_s
{
    char * text;

    gt_color_t color_background;
    gt_color_t color_pressed;

    gt_size_t   radius;

    gt_color_t      font_color;
    uint8_t         font_size;
    gt_family_t     font_family_cn;
    gt_family_t     font_family_en;

    uint8_t     fill;
}_gt_style_btn_st;



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
void gt_btn_style_set_color_pressed(gt_obj_st * btn, gt_color_t color);

/**
 * @brief set the color when the button is unselected
 *
 * @param btn button obj
 * @param color unselected color
 */
void gt_btn_style_set_color_background(gt_obj_st * btn, gt_color_t color);

/**
 * @brief set button text
 *
 * @param btn button obj
 * @param text text
 */
void gt_btn_style_set_text(gt_obj_st * btn, char *text);

/**
 * @brief get button text
 *
 * @param btn button obj
 * @return char* button text
 */
char * gt_btn_style_get_text(gt_obj_st * btn);

/**
 * @brief set button font color
 *
 * @param btn button obj
 * @param color font color
 */
void gt_btn_style_set_font_color(gt_obj_st * btn, gt_color_t color);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_BTN_H_
