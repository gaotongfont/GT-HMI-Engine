/**
 * @file gt_option.h
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-18 13:59:53
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_OPTION_H_
#define _GT_OPTION_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct _gt_style_option_s
{
    char ** option;
    gt_color_t color_selected;
    gt_color_t color_unselected;
    gt_color_t color_back;
    uint8_t cnt;
    uint8_t idx;
}_gt_style_option_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_option_create(gt_obj_st * parent);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_OPTION_H_
