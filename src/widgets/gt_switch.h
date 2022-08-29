/**
 * @file gt_switch.h
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-07-21 14:34:33
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SWITCH_H_
#define _GT_SWITCH_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct _gt_style_switcher_s
{
    gt_color_t color_open;
    gt_color_t color_close;

    bool state;             //switcher state ->true:open,false:close
}_gt_style_switcher_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_switch_create(gt_obj_st * parent);
void gt_switch_style_set_state(gt_obj_st * switcher, bool state);
bool gt_switch_style_get_state(gt_obj_st * switcher);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SWITCH_H_
