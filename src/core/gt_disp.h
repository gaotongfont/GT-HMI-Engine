/**
 * @file gt_disp.h
 * @author yongg
 * @brief Display implementation for the screen.
 * @version 0.1
 * @date 2022-05-11 15:05:29
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_DISP_H_
#define _GT_DISP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../widgets/gt_obj.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef enum {
    GT_DIR_VER,
    GT_DIR_HOR,
}gt_dir_e;

/**
 * @brief The screen turn next screen animation type
 */
typedef enum {
    GT_SCR_ANIM_TYPE_NONE = 0,
    GT_SCR_ANIM_TYPE_MOVE_LEFT,     // Move the interface to the left
    GT_SCR_ANIM_TYPE_MOVE_RIGHT,    // Move the interface to the right
    // GT_SCR_ANIM_TYPE_MOVE_UP,       // Move the interface to the up
    GT_SCR_ANIM_TYPE_MOVE_DOWN,     // Move the interface to the down

    GT_SCR_ANIM_TYPE_COVER_LEFT,    // Cover the interface from the left
    GT_SCR_ANIM_TYPE_COVER_RIGHT,   // Cover the interface from the right
    GT_SCR_ANIM_TYPE_COVER_UP,      // Cover the interface from the up
    // GT_SCR_ANIM_TYPE_COVER_DOWN,    // Cover the interface from the down

    GT_SCR_ANIM_TYPE_TOTAL,
}gt_scr_anim_type_et;

typedef struct _gt_draw_board_s{
    gt_color_t * ctx;
    gt_area_st area;
}gt_draw_board_st;

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

gt_obj_st * gt_disp_get_scr_prev(void);

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

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_DISP_H_
