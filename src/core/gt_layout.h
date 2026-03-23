/**
 * @file gt_layout.h
 * @author Feyoung
 * @brief Logical handling of layers
 * @version 0.1
 * @date 2024-03-29 19:53:33
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_LAYOUT_H_
#define _GT_LAYOUT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../widgets/gt_obj.h"



/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief Arrange controls in a row arrangement,
 *  the width of the control is the width of the control itself,
 *  when object prop setup row_layout = true, Reverse order needs
 *  to be set grow_invert = true.
 *
 * @param obj The target object
 */
void gt_layout_row_grow(gt_obj_st * obj);

#if GT_USE_WIDGET_LAYOUT
void gt_layout_init(gt_obj_st * obj, gt_obj_container_st const * const container);

gt_res_t gt_layout_update_core(gt_obj_st * obj);

enum gt_layout_type_e gt_layout_get_type(gt_obj_st * obj);

bool gt_layout_is_type(gt_obj_st * obj, enum gt_layout_type_e type);

/**
 * @brief Mark a flex container as needing layout recalculation.
 *        Called when a child's position or size changes.
 *        Actual recalculation is deferred to gt_layout_flush_dirty().
 *
 * @param obj The flex container object
 */
void gt_layout_mark_dirty(gt_obj_st * obj);

/**
 * @brief Process all dirty flex containers in the active screen tree.
 *        Called once per frame in gt_refr_timer() before dirty area check.
 */
void gt_layout_flush_dirty(void);

/**
 * @brief 动态切换容器布局类型，支持 FIXED ↔ FLEX 运行时切换。
 *        FIXED→FLEX：保存所有子元素相对坐标，触发 flex 排列。
 *        FLEX→FIXED：恢复子元素原始相对坐标，触发重绘。
 *
 * @param obj  容器对象
 * @param type 目标布局类型 @ref gt_layout_type_e
 */
void gt_layout_set_type(gt_obj_st * obj, enum gt_layout_type_e type);
#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_LAYOUT_H_
