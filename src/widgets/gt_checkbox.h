/**
 * @file gt_checkbox.h
 * @author yongg
 * @brief checkbox element
 * @version 0.1
 * @date 2022-07-18 14:11:44
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_CHECKBOX_H_
#define _GT_CHECKBOX_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct _gt_style_checkbox_s
{
    gt_color_t  font_color;
    gt_family_t font_family_cn;
    gt_family_t font_family_en;
    uint8_t     font_size;
    char *      text;
}_gt_style_checkbox_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief create checkbox obj
 *
 * @param parent checkbox's parent obj
 * @return gt_obj_st* checkbox obj ptr
 */
gt_obj_st * gt_checkbox_create(gt_obj_st * parent);

/**
 * @brief set the checkbox text
 *
 * @param checkbox checkbox obj
 * @param text text
 */
void gt_checkbox_style_set_text(gt_obj_st * checkbox, char * text);
void gt_checkbox_style_get_text(gt_obj_st * checkbox, char * text);

void gt_checkbox_style_set_font_color(gt_obj_st * checkbox, gt_color_t color);
void gt_checkbox_style_set_font_family_cn(gt_obj_st * checkbox, gt_family_t family);
void gt_checkbox_style_set_font_family_en(gt_obj_st * checkbox, gt_family_t family);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_CHECKBOX_H_
