/**
 * @file gt_indev.h
 * @author yongg
 * @brief input device handler
 * @version 0.1
 * @date 2022-06-06 14:06:12
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_INDEV_H_
#define _GT_INDEV_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../hal/gt_hal_indev.h"
#include "./gt_timer.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/\
#if GT_USE_UD_LR_TO_CONTROL_FOCUS_EN
typedef enum{
    GT_FOCUS_DIR_RIGHT = 0,
    GT_FOCUS_DIR_LEFT,
    GT_FOCUS_DIR_DOWN,
    GT_FOCUS_DIR_UP,
}gt_focus_dir_et;
#endif

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief Get indev button key value
 *
 * @return uint32_t
 */
uint32_t gt_indev_get_key(void);

/**
 * @brief Get physical point value
 *
 * @return gt_point_st
 */
gt_point_st gt_indev_get_point(void);

/**
 * @brief get click first point
 *
 * @param indev input device
 * @param point click first point
 */
void gt_indev_get_point_act(const gt_indev_st * indev, gt_point_st * point);

/**
 * @brief get clicking newly point
 *
 * @param indev input device
 * @param point clicking newly point
 */
void gt_indev_get_point_newly(const gt_indev_st * indev, gt_point_st * point);

/**
 * @brief Get the gesture of the input device
 *
 * @param indev input device
 * @return gt_dir_et Direction of gesture
 */
gt_dir_et gt_indev_get_gesture(const gt_indev_st * indev);

/**
 * @brief Input processing handle
 */
void gt_indev_handler(struct _gt_timer_s * timer);

/**
 * @brief disabled input device
 *
 * @param disabled 0: disabled, 1: enabled [default: 1]
 */
void gt_indev_set_enabled(bool enabled);

/**
 * @brief
 *
 * @return true
 * @return false
 */
bool gt_indev_is_enabled(void);

bool _gt_indev_remove_want_delate_target(gt_obj_st * target);

#if GT_USE_UD_LR_TO_CONTROL_FOCUS_EN

/**
 * @brief Release focus lock
 *
 */
void gt_indev_release_focus_lock(void);
#endif


#if GT_INDEV_SIMULATE_POINTER
void gt_indev_simulate_handler(gt_indev_state_et state, gt_size_t x, gt_size_t y);
#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_INDEV_H_
