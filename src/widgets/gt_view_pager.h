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
typedef enum {
    GT_VIEW_PAGER_SCROLL_EFFECT_DEFAULT = 0,
    GT_VIEW_PAGER_SCROLL_EFFECT_STATIC,
    GT_VIEW_PAGER_SCROLL_EFFECT_NO_FINGER_FOLLOW,
    GT_VIEW_PAGER_SCROLL_EFFECT_MULTI_PAGE,
}gt_view_pager_scroll_effect_et;

typedef enum {
    GT_VIEW_PAGER_DIR_HORIZONTAL = 0,   /** horizontal scroll (default) */
    GT_VIEW_PAGER_DIR_VERTICAL,         /** vertical scroll */
}gt_view_pager_dir_et;

/**
 * @brief Tip bar position.
 *  Horizontal direction: BOTTOM(default) or TOP.
 *  Vertical direction:   RIGHT(default) or LEFT.
 */
typedef enum {
    GT_VIEW_PAGER_TIP_BAR_POS_DEFAULT = 0, /** bottom(hor) or right(ver) */
    GT_VIEW_PAGER_TIP_BAR_POS_OPPOSITE,    /** top(hor) or left(ver) */
}gt_view_pager_tip_bar_pos_et;



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
 * @brief view pager scroll to target fragment index
 *
 * @param obj
 * @param index The target fragment index
 */
void gt_view_pager_scroll_to_fragment(gt_obj_st * obj, gt_size_t index);

/**
 * @brief Set the view pager scroll effect. If statical is true, the view pager
 *      will not scroll anim. If no_finger_follow is true, the view pager will
 *      not follow the finger when the finger is pressing.
 *
 * @param obj
 * @param effect 0[default]: GT_VIEW_PAGER_SCROLL_EFFECT_DEFAULT
 */
void gt_view_pager_set_scroll_effect(gt_obj_st * obj, gt_view_pager_scroll_effect_et effect);

/**
 * @brief Get cur page
 *
 * @param obj
 * @return uint8_t
 */
uint8_t gt_view_pager_get_cur_page(gt_obj_st * obj);

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
 * @brief Get the fragment index of the child widget belong
 *
 * @param view_pager
 * @param child The child widget which want to be found
 * @return gt_size_t -1: not found
 */
gt_size_t gt_view_pager_get_widget_belong_fragment(gt_obj_st * view_pager, gt_obj_st * child);

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

/**
 * @brief Set the visible window size of the view pager.
 *      By default the window equals the parent area.
 *      Call this after gt_view_pager_create() and before adding fragments.
 *
 * @param obj
 * @param w  Visible window width  (page width for horizontal scroll)
 * @param h  Visible window height (page height for vertical scroll)
 */
void gt_view_pager_set_window_size(gt_obj_st * obj, uint16_t w, uint16_t h);

/**
 * @brief Set scroll direction: horizontal (default) or vertical.
 *      Must be called before adding fragments.
 *
 * @param obj
 * @param dir @ref gt_view_pager_dir_et
 */
void gt_view_pager_set_direction(gt_obj_st * obj, gt_view_pager_dir_et dir);
gt_view_pager_dir_et gt_view_pager_get_direction(gt_obj_st * obj);

/**
 * @brief Set tip bar position.
 *      Horizontal: DEFAULT=bottom, OPPOSITE=top.
 *      Vertical:   DEFAULT=right,  OPPOSITE=left.
 *
 * @param obj
 * @param pos @ref gt_view_pager_tip_bar_pos_et
 */
void gt_view_pager_set_tip_bar_pos(gt_obj_st * obj, gt_view_pager_tip_bar_pos_et pos);


#endif  /** GT_CFG_ENABLE_VIEW_PAGER */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_VIEW_PAGE_H_
