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
    GT_ALIGN_NONE = 0,
    GT_ALIGN_LEFT,
    GT_ALIGN_RIGHT,
    GT_ALIGN_CENTER,

    GT_ALIGN_REVERSE = 0x80,    //end of enum
}gt_align_et;

typedef enum _gt_text_long_mode_e
{
    GT_TEXT_LONG_MODE_WRAP,
    GT_TEXT_LONG_MODE_DOT,
    GT_TEXT_LONG_MODE_SCROLL,
}gt_text_long_mode_et;

typedef enum{
    GT_BAR_DIR_HOR_L2R,
    GT_BAR_DIR_HOR_R2L,
    GT_BAR_DIR_VER_U2D,
    GT_BAR_DIR_VER_D2U,
}gt_bar_dir_et;
/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_obj_set_area(gt_obj_st * obj, gt_area_st area);
void gt_obj_set_pos(gt_obj_st * obj, gt_size_t x, gt_size_t y);
void gt_obj_set_size(gt_obj_st * obj, uint16_t w, uint16_t h);
void gt_obj_set_x(gt_obj_st * obj, uint16_t x);
void gt_obj_set_y(gt_obj_st * obj, uint16_t y);
void gt_obj_set_w(gt_obj_st * obj, uint16_t w);
void gt_obj_set_h(gt_obj_st * obj, uint16_t h);

void gt_obj_set_opa(gt_obj_st * obj, gt_opa_t opa);

uint16_t gt_obj_get_x(gt_obj_st * obj);
uint16_t gt_obj_get_y(gt_obj_st * obj);
uint16_t gt_obj_get_w(gt_obj_st * obj);
uint16_t gt_obj_get_h(gt_obj_st * obj);

void gt_obj_set_visible(gt_obj_st * obj, gt_visible_et is_visible);
void gt_obj_set_disabled(gt_obj_st * obj, gt_disabled_et is_disabled);
bool gt_obj_is_disabled(gt_obj_st * obj);

void gt_screen_set_bgcolor(gt_obj_st * obj, gt_color_t color);
gt_color_t gt_screen_get_bgcolor(gt_obj_st * obj);

void gt_obj_size_change(gt_obj_st * obj, gt_area_st * area_new);
void gt_obj_pos_change(gt_obj_st * obj, gt_area_st * area_new);
void gt_obj_area_change(gt_obj_st * obj, gt_area_st * area_new);


void gt_obj_set_state(gt_obj_st * obj, gt_state_et state);
gt_state_et gt_obj_get_state(gt_obj_st * obj);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_STYLE_H_
