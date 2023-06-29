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
typedef enum{
    GT_DIR_VER,
    GT_DIR_HOR,
}gt_dir_e;

typedef struct _gt_draw_board_s{
    gt_color_t * ctx;
    gt_area_st area;
}gt_draw_board_st;

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief load active screen
 *
 * @param scr The screen which want to be loaded
 */
void gt_load_scr(gt_obj_st * scr);

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

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_DISP_H_
