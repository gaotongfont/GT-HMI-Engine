/**
 * @file gt_view_pager.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2023-10-07 14:39:28
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_VIEW_PAGER_H_
#define _GT_VIEW_PAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_VIEW_PAGER
#include "stdbool.h"
#include "gt_obj.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief create a view pager object
 *
 * @param parent
 * @return gt_obj_st*
 */
gt_obj_st * gt_view_pager_create(gt_obj_st * parent);

/**
 * @brief Set the total number of fragments
 *
 * @param obj
 * @param count The total number of fragments, maximum @see _MAX_VIEW_PAGER_COUNT
 * @return int8_t The fragment count
 */
int8_t gt_view_pager_set_fragment_count(gt_obj_st * obj, uint8_t count);

/**
 * @brief add a fragment
 *
 * @param obj
 * @return int8_t
 */
int8_t gt_view_pager_add_fragment(gt_obj_st * obj);

/**
 * @brief Add widget into the view pager
 *
 * @param view_pager
 * @param fragment_idx
 * @param child
 */
void gt_view_pager_fragment_add_widget(gt_obj_st * view_pager, uint8_t fragment_idx, gt_obj_st * child);

/**
 * @brief Enabled or disabled the background glass effect
 *
 * @param obj
 * @param enabled
 */
void gt_view_pager_set_glass(gt_obj_st * obj, bool enabled);
bool gt_view_pager_get_glass(gt_obj_st * obj);

/**
 * @brief Set background glass color
 *
 * @param obj
 * @param color
 */
void gt_view_pager_set_glass_color(gt_obj_st * obj, gt_color_t color);
gt_color_t gt_view_pager_get_glass_color(gt_obj_st * obj);


#endif  /** GT_CFG_ENABLE_VIEW_PAGER */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_VIEW_PAGE_H_
