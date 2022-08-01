/**
 * @file gt_checkbox.h
 * @author yongg
 * @brief 
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
    uint8_t selected;
    char * text;
}_gt_style_checkbox_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_checkbox_create(gt_obj_st * parent);
void gt_checkbox_style_set_selceted(gt_obj_st * checkbox);
void gt_checkbox_style_set_unselceted(gt_obj_st * checkbox);
uint8_t gt_checkbox_style_get_selceted_state(gt_obj_st * checkbox);

void gt_checkbox_style_set_text(gt_obj_st * checkbox, char * text);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_CHECKBOX_H_
