/**
 * @file gt_obj_pos.h
 * @author yongg
 * @brief Set object position function
 * @version 0.1
 * @date 2022-06-15 14:34:27
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_OBJ_POS_H_
#define _GT_OBJ_POS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"
#include <stdbool.h>
#include "../gt_conf.h"
#include "../widgets/gt_obj.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief area value copy
 *
 * @param dst dst area
 * @param src src area
 */
void gt_area_copy(gt_area_st * dst, gt_area_st * src);

/**
 * @brief check the obj is visible
 *
 * @param obj object
 * @return true visible
 * @return false invisible
 */
bool gt_obj_is_visible(gt_obj_st * obj);

/**
 * @brief check the obj is visible in area
 *
 * @param obj locate object
 * @param area display area
 * @return true
 * @return false
 */
bool gt_obj_check_visible(gt_obj_st * obj, gt_area_st area);


/**
 * @brief gets the area of the object within the display area
 *
 * @param obj obj
 * @param area_act disp area active
 * @param area_valid valid area(x:start cpy x pos, y:start cpy y pos, w:how much cpy width,h:how much cpy height)
 */
void gt_obj_get_valid_area(gt_obj_st * obj, gt_area_st * area_act, gt_area_st *area_valid);

/**
 * @brief check obj was clicked by point
 *
 * @param obj check obj
 * @param point click point
 * @return true clicked
 * @return false not clicked
 */
bool gt_obj_check_is_clicked( gt_obj_st * obj, gt_point_st * point );

/**
 * @brief find smallest clicked obj from parent
 *
 * @param parent parent obj
 * @param point click point
 * @return gt_obj_st* smallest and clicked obj in parent
 */
gt_obj_st * gt_find_clicked_obj_by_point(gt_obj_st * parent, gt_point_st * point);

gt_obj_st* gt_find_clicked_obj_by_focus(gt_obj_st * parent);
void gt_obj_next_focus_change(gt_obj_st * cur_obj);
void gt_obj_prev_focus_change(gt_obj_st * cur_obj);

/**
 * @brief check obj is in the disp screen now
 *
 * @param obj need check obj
 * @return true yes
 * @return false no
 */
bool gt_obj_check_scr(gt_obj_st * obj);

void gt_obj_get_click_point_by_phy_point(gt_obj_st * obj, gt_point_st * point_phy, gt_point_st * point_ret);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_OBJ_POS_H_
