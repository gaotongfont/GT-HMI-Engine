/**
 * @file gt_obj_scroll.h
 * @author yongg
 * @brief Scroll bar animation
 * @version 0.1
 * @date 2022-08-31 14:25:09
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_OBJ_SCROLL_H_
#define _GT_OBJ_SCROLL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"
#include "../others/gt_types.h"
#include "../widgets/gt_obj.h"

struct gt_obj_s;
/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef enum{
    GT_ANIM_OFF = 0,
    GT_ANIM_ON,
}gt_anim_enable_et;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief scroll obj to position x,y
 *
 * @param obj obj
 * @param dx x coordinate of virtual screen object
 * @param dy y coordinate of virtual screen object
 * @param en enable scroll anim
 */
void gt_obj_scroll_to(struct gt_obj_s * obj, gt_size_t dx, gt_size_t dy, gt_anim_enable_et en);

/**
 * @brief scroll obj to position x
 *
 * @param obj obj
 * @param x x coordinate of virtual screen object
 * @param en enable scroll anim
 */
void gt_obj_scroll_to_x(struct gt_obj_s * obj, gt_size_t x, gt_anim_enable_et en);

/**
 * @brief scroll obj to position y
 *
 * @param obj obj
 * @param y y coordinate of virtual screen object
 * @param en enable scroll anim
 */
void gt_obj_scroll_to_y(struct gt_obj_s * obj, gt_size_t y, gt_anim_enable_et en);


gt_size_t gt_obj_scroll_get_x(gt_obj_st * obj);
gt_size_t gt_obj_scroll_get_y(gt_obj_st * obj);

void _gt_obj_set_process_point(gt_obj_st * obj, gt_point_st * point);
void _gt_obj_set_process_scroll_xy(gt_obj_st * obj, gt_size_t x_scroll, gt_size_t y_scroll);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_OBJ_SCROLL_H_
