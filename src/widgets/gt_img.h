/**
 * @file gt_img.h
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-07-05 17:36:46
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_IMG_H_
#define _GT_IMG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct _gt_style_img_s
{
    gt_color_t color_selected;
    gt_color_t color_unselected;

    uint32_t selected;

    char * src;         // img src, It can be a virt file name/path or an array of C

}_gt_style_img_st;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_img_style_set_selected(gt_obj_st * img, uint8_t selected);
void gt_img_style_set_color_unselected(gt_obj_st * img, gt_color_t color);
void gt_img_style_set_color_selected(gt_obj_st * img, gt_color_t color);
gt_obj_st * gt_img_create(gt_obj_st * parent);
void gt_img_style_set_src(gt_obj_st * img, char * src);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_IMG_H_
