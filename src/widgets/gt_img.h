/**
 * @file gt_img.h
 * @author yongg
 * @brief image display control
 * @version 0.1
 * @date 2022-07-05 17:36:46
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_IMG_H_
#define _GT_IMG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief  create img obj
 *
 * @param parent img's parent obj
 * @return gt_obj_st* img obj ptr
 */
gt_obj_st * gt_img_create(gt_obj_st * parent);

/**
 * @brief set the path of the img
 *
 * @param img img obj
 * @param src path of img
 */
void gt_img_set_src(gt_obj_st * img, char * src);

/**
 * @brief get img src of path
 *
 * @param img img obj
 * @return char* src of img
 */
char * gt_img_get_src(gt_obj_st * img);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_IMG_H_
