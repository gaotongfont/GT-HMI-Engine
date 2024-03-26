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
#include "../others/gt_types.h"
#include "./gt_scr_stack.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief Go back to previous screen
 *
 * @param step 0: Only get current scr info; > 0: reload previous screen
 * @return gt_scr_id_t The previous screen id
 */
gt_scr_id_t gt_disp_stack_go_back(gt_stack_size_t step);

/**
 * @brief Use default param to load and display screen
 *
 * @param scr_id need to register id with init callback function @ref gt_scr_list_st
 */
void gt_disp_stack_load_scr(gt_scr_id_t scr_id);

/**
 * @brief Use Custom param to load and display screen
 *
 * @param scr_id need to register id with init callback function @ref gt_scr_list_st
 * @param type animation type
 * @param time animation whole time
 * @param delay delay time to start animation
 * @param del_prev_scr true: free previous screen object; false: keep previous screen object alive
 */
void gt_disp_stack_load_scr_anim(gt_scr_id_t scr_id, gt_scr_anim_type_et type, uint32_t time, uint32_t delay, bool del_prev_scr);

/**
 * @brief display screen
 *
 * @param scr screen obj
 */
void gt_disp_load_scr(gt_obj_st * scr);

void gt_disp_load_scr_anim(gt_obj_st * scr, gt_scr_anim_type_et type, uint32_t time, uint32_t delay, bool del_prev_scr);

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
 * @brief Get the top layer of the display, such as: the top layer of the dialog, popup, etc.
 *
 * @return gt_obj_st* The top layer of the display virtual screen
 */
gt_obj_st * gt_disp_get_layer_top(void);

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
