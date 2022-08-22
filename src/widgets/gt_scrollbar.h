/**
 * @file gt_scrollbar.h
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-07-21 19:53:55
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SCROLLBAR_H_
#define _GT_SCROLLBAR_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct _gt_style_scrollbar_s
{
    gt_color_t color_act;
    gt_color_t color_ina;

    gt_color_t bg_color;
    
    gt_size_t start;
    gt_size_t end;
    gt_size_t pos;
    
}_gt_style_scrollbar_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_scrollbar_create(gt_obj_st * parent);

void gt_scrollbar_style_set_pos(gt_obj_st * scrollbar, gt_size_t pos);
void gt_scrollbar_style_set_start_end(gt_obj_st * scrollbar, gt_size_t start, gt_size_t end);
gt_size_t gt_scrollbar_style_get_pos(gt_obj_st * scrollbar);
gt_size_t gt_scrollbar_style_get_start(gt_obj_st * scrollbar);
gt_size_t gt_scrollbar_style_get_end(gt_obj_st * scrollbar);
gt_size_t gt_scrollbar_style_get_total(gt_obj_st * scrollbar);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SCROLLBAR_H_
