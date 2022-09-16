/**
 * @file gt_textarea.h
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-13 09:47:27
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_TEXTAREA_H_
#define _GT_TEXTAREA_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct _gt_txt_s{
    char * text;
    uint16_t len;
    uint8_t mask_style;
    gt_color_t color;
}_gt_txt_st;

typedef struct _gt_style_textarea_s{
    uint16_t cnt_txts;
    uint8_t space_x;
    uint8_t space_y;

    uint8_t font_style_cn;
    uint8_t font_style_en;
    uint8_t font_size;

    _gt_txt_st * txts;
}_gt_style_textarea_st;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief create a textarea obj
 *
 * @param parent textarea's parent element
 * @return gt_obj_st* textarea obj
 */
gt_obj_st * gt_textarea_create(gt_obj_st * parent);

/**
 * @brief add string to textarea widget with style
 *
 * @param textarea obj pointer
 * @param str text
 * @param style font style @ref gt_font_style_et
 * @param color font color
 */
void gt_textarea_style_add_str(gt_obj_st * textarea, char * str, uint8_t style, gt_color_t color);

/**
 * @brief set textarea space
 *
 * @param textarea obj pointer
 * @param space_x
 * @param space_y
 */
void gt_textarea_style_set_space(gt_obj_st * textarea, uint8_t space_x, uint8_t space_y);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_TEXTAREA_H_
