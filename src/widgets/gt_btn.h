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
    char * text;
    
    uint8_t selected;
}_gt_style_btn_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_btn_create(gt_obj_st * parent){
    
}

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_BTN_H_
