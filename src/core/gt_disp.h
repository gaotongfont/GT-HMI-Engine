/**
 * @file gt_disp.h
 * @author yongg
 * @brief Display implementation for the screen.
 * @version 0.1
 * @date 2022-05-11 15:05:29
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_DISP_H_
#define _GT_DISP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../widgets/gt_obj.h"
#include "../hal/gt_hal_disp.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/

/**
 * @brief The screen turn next screen animation type
 */
typedef enum {
    GT_SCR_ANIM_TYPE_NONE = 0,
    GT_SCR_ANIM_TYPE_MOVE_LEFT,     // Move the interface to the left
    GT_SCR_ANIM_TYPE_MOVE_RIGHT,    // Move the interface to the right
    GT_SCR_ANIM_TYPE_MOVE_UP,       // Move the interface to the up
    GT_SCR_ANIM_TYPE_MOVE_DOWN,     // Move the interface to the down

    GT_SCR_ANIM_TYPE_COVER_LEFT,    // Cover the interface from the left
    GT_SCR_ANIM_TYPE_COVER_RIGHT,   // Cover the interface from the right
    GT_SCR_ANIM_TYPE_COVER_UP,      // Cover the interface from the up
    GT_SCR_ANIM_TYPE_COVER_DOWN,    // Cover the interface from the down

    GT_SCR_ANIM_TYPE_TOTAL,
}gt_scr_anim_type_et;

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief Set display screen
 *
 * @param scr The screen which want to be loaded
 */
void gt_disp_set_scr(gt_obj_st * scr);

/**
 * @brief Get display screen
 *
 * @return gt_obj_st* The active display screen
 */
gt_obj_st * gt_disp_get_scr(void);

/**
 * @brief display screen
 *
 * @param scr screen obj
 */
void gt_disp_load_scr(gt_obj_st * scr);

void gt_disp_load_scr_anim(gt_obj_st * scr, gt_scr_anim_type_et type, uint32_t time, uint32_t delay, bool auto_del);

/**
 * @brief Update the display area
 *
 * @param coords area
 */
void gt_disp_ref_area(const gt_area_st * coords);

/**
 * @brief Get active display area
 *
 * @return gt_area_st* The active display screen
 */
gt_area_st * gt_disp_get_area_act(void);

gt_area_abs_st * gt_disp_get_area_max(void);

/**
 * @brief Set active display area
 *
 * @param area The active area
 */
void gt_disp_set_area_act(gt_area_st * area);

/**
 * @brief set active scroll direction and distance.
 *
 * @param dist_x distance of x
 * @param dist_y distance of y
 */
void gt_disp_scroll_area_act(int16_t dist_x, int16_t dist_y);

/**
 * @brief Detects if the control intersects with the screen and sets the redrawn area
 *
 * @param obj
 */
void gt_disp_invalid_area(gt_obj_st * obj);

/**
 * @brief Initialize the backoff page, and the depth of the backoff page is 1
 *
 * @param need_backoff_scr The screen which want to be backoff
 * @param init_cb The callback function of the screen initialization
 * @param time The time of the screen backoff animation
 * @param delay The delay time of the screen backoff animation
 */
void gt_disp_set_backoff_scr(gt_obj_st * need_backoff_scr, gt_scr_init_func_cb_t init_cb, uint32_t time, uint32_t delay);

/**
 * @brief Return to the previous screen. The `gt_disp_set_backoff_scr()`
 *      function needs to be executed before entering the current page
 */
void gt_disp_go_backoff_scr(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_DISP_H_
