/**
 * @file gt_processbar.h
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-07-22 14:13:28
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_PROCESSBAR_H_
#define _GT_PROCESSBAR_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct _gt_style_processbar_s
{
    gt_size_t start;
    gt_size_t end;
    gt_size_t pos;
    gt_color_t color_act;
    gt_color_t color_ina;

    gt_color_t bcolor;
}_gt_style_processbar_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_processbar_create(gt_obj_st * parent);

void gt_processbar_style_set_pos(gt_obj_st * processbar, gt_size_t pos);
void gt_processbar_style_set_start_end(gt_obj_st * processbar, gt_size_t start, gt_size_t end);
gt_size_t gt_processbar_style_get_pos(gt_obj_st * processbar);
gt_size_t gt_processbar_style_get_start(gt_obj_st * processbar);
gt_size_t gt_processbar_style_get_end(gt_obj_st * processbar);
gt_size_t gt_processbar_style_get_total(gt_obj_st * processbar);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_PROCESSBAR_H_
