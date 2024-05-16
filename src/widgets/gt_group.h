/**
 * @file gt_group.h
 * @author yongg
 * @brief The group of members control
 * @version 0.1
 * @date 2022-07-20 15:51:11
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_GROUP_H_
#define _GT_GROUP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_GROUP
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

/**
 * @brief According to the type of the object, get the selected or active object
 *
 * @param group Group object
 * @param type  Widget type enum
 * @return gt_obj_st* The selected or active object
 */
gt_obj_st * gt_group_get_active_obj(gt_obj_st * group, gt_obj_type_et type);

/**
 * @brief According to the type of the object, only set the first one object active
 *
 * @param group
 * @param type Widget type enum
 * @return true set success
 * @return false set failed
 */
bool gt_group_reset_selected_state(gt_obj_st * group, gt_obj_type_et type);


#endif  /** GT_CFG_ENABLE_GROUP */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_GROUP_H_
