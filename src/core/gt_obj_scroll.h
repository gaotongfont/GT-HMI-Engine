/**
 * @file gt_obj_scroll.h
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-08-31 14:25:09
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_OBJ_SCROLL_H_
#define _GT_OBJ_SCROLL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"
#include "../others/gt_types.h"

struct _gt_obj_s;
/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef enum{
    GT_ANIM_ON,
    GT_ANIM_OFF,
}gt_anim_enable_et;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief scroll obj to position x,y
 * 
 * @param obj obj
 * @param x x coordinate of virtual screen object
 * @param y y coordinate of virtual screen object
 * @param en enable scroll anim
 */
void gt_obj_scroll_to(struct _gt_obj_s * obj, gt_size_t x, gt_size_t y, gt_anim_enable_et en);

/**
 * @brief scroll obj to position x
 * 
 * @param obj obj
 * @param x x coordinate of virtual screen object
 * @param en enable scroll anim
 */
void gt_obj_scroll_to_x(struct _gt_obj_s * obj, gt_size_t x, gt_anim_enable_et en);

/**
 * @brief scroll obj to position y
 * 
 * @param obj obj
 * @param y y coordinate of virtual screen object
 * @param en enable scroll anim
 */
void gt_obj_scroll_to_y(struct _gt_obj_s * obj, gt_size_t y, gt_anim_enable_et en);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_OBJ_SCROLL_H_
