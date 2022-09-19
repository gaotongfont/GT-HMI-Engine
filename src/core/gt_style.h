/**
 * @file gt_style.h
 * @author yonogg
 * @brief 
 * @version 0.1
 * @date 2022-05-12 18:37:46
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_STYLE_H_
#define _GT_STYLE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../others/gt_color.h"
#include "../widgets/gt_obj.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef enum _gt_align_e
{ 
    GT_ALIGN_NONE = -1,
    GT_ALIGN_LEFT,
    GT_ALIGN_RIGHT,
    GT_ALIGN_CENTER,
}gt_align_et;

typedef enum _gt_text_long_mode_e
{
    GT_TEXT_LONG_MODE_WRAP,
    GT_TEXT_LONG_MODE_DOT,
    GT_TEXT_LONG_MODE_SCROLL,
}gt_text_long_mode_et;

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_obj_set_style_area(gt_obj_st * obj, gt_area_st area);
void gt_obj_set_style_pos(gt_obj_st * obj, gt_size_t x, gt_size_t y);
void gt_obj_set_style_size(gt_obj_st * obj, uint16_t w, uint16_t h);
void gt_obj_set_style_x(gt_obj_st * obj, uint16_t x);
void gt_obj_set_style_y(gt_obj_st * obj, uint16_t y);
void gt_obj_set_style_w(gt_obj_st * obj, uint16_t w);
void gt_obj_set_style_h(gt_obj_st * obj, uint16_t h);

uint16_t gt_obj_get_style_x(gt_obj_st * obj);
uint16_t gt_obj_get_style_y(gt_obj_st * obj);
uint16_t gt_obj_get_style_w(gt_obj_st * obj);
uint16_t gt_obj_get_style_h(gt_obj_st * obj);

void gt_obj_set_visible(gt_obj_st * obj, gt_visible_e is_visble);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_STYLE_H_
