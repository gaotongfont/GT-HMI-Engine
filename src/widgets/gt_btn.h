/**
 * @file gt_btn.h
 * @author yongg
 * @brief 
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
    gt_color_t color_selected;
    gt_color_t color_unselected;
    
    gt_size_t radius;

    gt_size_t border_width;
    gt_color_t border_bcolor;
    gt_color_t border_fcolor;

    char * text;

    uint8_t isfill:1;
    uint8_t selected:1;
}_gt_style_btn_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_btn_create(gt_obj_st * parent);
void gt_btn_style_set_color_selected(gt_obj_st * btn, gt_color_t color);
void gt_btn_style_set_color_unselected(gt_obj_st * btn, gt_color_t color);
void gt_btn_style_set_selected(gt_obj_st * btn, uint8_t selected);
void gt_btn_style_set_text(gt_obj_st * btn, char *text);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_BTN_H_
