/**
 * @file gt_rect.h
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-11-02 09:50:08
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_RECT_H_
#define _GT_RECT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_RECT
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "../core/gt_gradient.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



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
void gt_rect_set_radius(gt_obj_st * rect, gt_radius_t radius);
void gt_rect_set_border(gt_obj_st * rect, uint16_t border);
void gt_rect_set_fill(gt_obj_st * rect, uint8_t is_full);

#if GT_USE_GRADIENT
/**
 * @brief Set gradient descriptor for rect background.
 *        The rect takes ownership of the descriptor — do NOT free it manually.
 *        Pass NULL to remove gradient and fall back to solid bg_color.
 * @param rect  Rect widget
 * @param grad  Gradient descriptor from gt_gradient_dsc_create(), or NULL
 */
void gt_rect_set_gradient(gt_obj_st * rect, gt_gradient_dsc_st * grad);
#endif

#if GT_USE_SHADOW
/**
 * @brief Set shadow descriptor for rect.
 *        The rect takes ownership of the descriptor — do NOT free it manually.
 *        Pass NULL to remove shadow.
 * @param rect    Rect widget
 * @param shadow  Shadow descriptor from gt_shadow_dsc_create(), or NULL
 */
void gt_rect_set_shadow(gt_obj_st * rect, gt_shadow_dsc_st * shadow);
#endif


#endif  /** GT_CFG_ENABLE_RECT */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_RECT_H_
