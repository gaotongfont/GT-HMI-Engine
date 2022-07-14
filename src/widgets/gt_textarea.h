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
typedef struct _gt_style_textarea_s{
    char * text;
    gt_area_st area_back;
}_gt_style_textarea_st;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_TEXTAREA_H_
