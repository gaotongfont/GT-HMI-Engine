/**
 * @file gt_switch.h
 * @author yongg
 * @brief switch controller
 * @version 0.1
 * @date 2022-07-21 14:34:33
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SWITCH_H_
#define _GT_SWITCH_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_SWITCH
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "../core/gt_style.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief create switch obj
 *
 * @param parent switch's parent obj
 * @return gt_obj_st* switch obj ptr
 */
gt_obj_st * gt_switch_create(gt_obj_st * parent);

void gt_switch_set_color_act(gt_obj_st * switcher, gt_color_t color);
void gt_switch_set_color_ina(gt_obj_st * switcher, gt_color_t color);
void gt_switch_set_color_point(gt_obj_st * switcher, gt_color_t color);


#endif  /** GT_CFG_ENABLE_SWITCH */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SWITCH_H_
