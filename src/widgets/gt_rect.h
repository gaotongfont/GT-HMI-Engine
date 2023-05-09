/**
 * @file gt_rect.h
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-11-02 09:50:08
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_RECT_H_
#define _GT_RECT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct _gt_style_rect_s
{
    gt_color_t  color_background;
    gt_color_t  color_border;
    uint16_t    radius;
    uint16_t    border;
    uint8_t     fill;
}_gt_style_rect_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief create rect obj
 *
 * @param parent rect's parent obj
 * @return gt_obj_st* rect obj ptr
 */
gt_obj_st * gt_rect_create(gt_obj_st * parent);

void gt_rect_set_bg_color(gt_obj_st * rect, gt_color_t color);
void gt_rect_set_color_border(gt_obj_st * rect, gt_color_t color);
void gt_rect_set_radius(gt_obj_st * rect, uint16_t radius);
void gt_rect_set_border(gt_obj_st * rect, uint16_t border);
void gt_rect_set_fill(gt_obj_st * rect, uint8_t is_full);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_RECT_H_
