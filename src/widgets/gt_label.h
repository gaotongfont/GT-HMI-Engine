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
    char * text;
}_gt_style_label_st;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_label_create(gt_obj_st * parent);
void gt_label_style_set_text(gt_obj_st * label, char * text);
char * gt_label_style_get_text(gt_obj_st * label);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_LABEL_H_
