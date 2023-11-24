/**
 * @file gt_group.h
 * @author yongg
 * @brief The group of members control
 * @version 0.1
 * @date 2022-07-20 15:51:11
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_GROUP_H_
#define _GT_GROUP_H_

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
 * @brief create group obj
 *
 * @param parent group's parent obj
 * @return gt_obj_st* group obj ptr
 */
gt_obj_st * gt_group_create(gt_obj_st * parent);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_GROUP_H_
