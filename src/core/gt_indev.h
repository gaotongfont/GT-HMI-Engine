/**
 * @file gt_indev.h
 * @author yongg
 * @brief input device handler
 * @version 0.1
 * @date 2022-06-06 14:06:12
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_INDEV_H_
#define _GT_INDEV_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../hal/gt_hal_indev.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

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
void gt_indev_handler(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_INDEV_H_
