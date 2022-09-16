/**
 * @file gt_label.h
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-07-13 11:55:38
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_LABEL_H_
#define _GT_LABEL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"



/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct _gt_style_label_s{
    gt_color_t color_font;

    uint8_t font_style_cn;
    uint8_t font_style_en;
    uint8_t font_size;

    char * text;
}_gt_style_label_st;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief create label obj
 * 
 * @param parent label's parent obj
 * @return gt_obj_st* label obj ptr
 */
gt_obj_st * gt_label_create(gt_obj_st * parent);

/**
 * @brief set label obj text
 * 
 * @param label label obj
 * @param text text
 */
void gt_label_style_set_text(gt_obj_st * label, char * text);

/**
 * @brief get label text
 * 
 * @param label label obj
 * @return char* label's text
 */
char * gt_label_style_get_text(gt_obj_st * label);

/**
 * @brief set label font color
 * 
 * @param label label obj
 * @param color color
 */
void gt_label_style_set_font_color(gt_obj_st * label, gt_color_t color);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_LABEL_H_
