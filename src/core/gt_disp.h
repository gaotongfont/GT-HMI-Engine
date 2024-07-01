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

#ifndef GT_DISP_STACK_IGNORE_REPEAT_SCREEN_ID
    /**
     * @brief Ignore repeat screen id, if the screen id is the same as the
     *      before screen id, it will not be reloaded and free the last
     *      intermediate page with the same id and the current page.
     *      [Default]0: The same screen id will be reloaded before within stack list;
     *               1: Ignore the same screen id, create a new screen object to load.
     */
    #define GT_DISP_STACK_IGNORE_REPEAT_SCREEN_ID   0
#endif


/* typedef --------------------------------------------------------------*/

typedef struct gt_disp_stack_param_s {
    gt_scr_id_t scr_id;
    gt_scr_anim_type_et type;
    uint32_t time;
    uint32_t delay;
    /**
     * @brief 0: keep previous screen object alive; 1: free previous screen object
     */
    uint8_t del_prev_scr : 1;
#if GT_DISP_STACK_IGNORE_REPEAT_SCREEN_ID
    /**
     * @brief defalut 0: The same screen id will be reloaded before within stack list;
     *                1: Ignore the same screen id, create a new screen object to load.
     */
    uint8_t ignore_repeat_screen_id : 1;
#endif
    uint8_t reserved : 6;
}gt_disp_stack_param_st;


typedef struct gt_disp_stack_res_s {
    gt_obj_st * scr_old;
    gt_scr_stack_item_st new_item;
    uint8_t del_prev_scr : 1;
    uint8_t ok : 1;
}gt_disp_stack_res_st;

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief [Use screen stack manager] Go back to previous screen
 *
 * @param step 0: Only get current scr info; > 0: reload previous screen
 * @return gt_scr_id_t The previous screen id
 */
gt_scr_id_t gt_disp_stack_go_back(gt_stack_size_t step);

/**
 * @brief [Use screen stack manager] Use default param to load and display screen
 *
 * @param scr_id need to register id with init callback function @ref gt_scr_list_st
 */
void gt_disp_stack_load_scr(gt_scr_id_t scr_id);

/**
 * @brief [Use screen stack manager] Use Custom param to load and display screen
 *
 * @param scr_id need to register id with init callback function @ref gt_scr_list_st
 * @param type animation type
 * @param time animation whole time
 * @param delay delay time to start animation
 * @param del_prev_scr true: free previous screen object; false: keep previous screen object alive
 */
void gt_disp_stack_load_scr_anim(gt_scr_id_t scr_id, gt_scr_anim_type_et type, uint32_t time, uint32_t delay, bool del_prev_scr);

/**
 * @brief [Warning] Only push screen object into stack, but not display or reload screen.
 *
 * @param param
 * @return gt_disp_stack_res_st
 */
gt_disp_stack_res_st gt_disp_stack_push_scr_only_st(gt_disp_stack_param_st const * const param);

/**
 * @brief [Use screen stack manager] Use custom param to load and display screen
 *
 * @param param @ref gt_disp_stack_param_st
 */
void gt_disp_stack_load_scr_anim_st(gt_disp_stack_param_st const * const param);

/**
 * @brief [Unused screen stack manager] display screen by default param.
 *      [Warn] It is not recommended to use with screen stack api, please use
 *      gt_disp_stack_load_scr() instead.
 *
 * @param scr screen object
 */
void gt_disp_load_scr(gt_obj_st * scr);

/**
 * @brief [Unused screen stack manager] display screen by custom param.
 *      [Warn] It is not recommended to use with screen stack api, please use
 *      gt_disp_stack_load_scr_anim() instead.
 *
 * @param scr screen object
 * @param type loading screen animation type
 * @param time loading screen animation whole time
 * @param delay loading screen delay time to start animation
 * @param del_prev_scr true: free previous screen object; false: keep previous screen object alive,
 *                  must be free by yourself.
 */
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
 * @brief set active scroll direction and distance.
 *
 * @param dist_x distance of x
 * @param dist_y distance of y
 */
void gt_disp_scroll_area_act(gt_size_t dist_x, gt_size_t dist_y);

/**
 * @brief Detects if the control intersects with the screen and sets the redrawn area
 *
 * @param obj
 */
void gt_disp_invalid_area(gt_obj_st * obj);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_DISP_H_
