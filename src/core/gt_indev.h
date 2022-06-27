/**
 * @file gt_indev.h
 * @author yongg
 * @brief 
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
void gt_indev_get_point(const gt_indev_st * indev, gt_point_st * point);
gt_dir_et gt_indev_get_gesture(const gt_indev_st * indev);
void gt_indev_handler();

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_INDEV_H_
