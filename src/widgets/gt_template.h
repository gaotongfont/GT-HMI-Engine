/**
 * @file gt_template.h
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-18 13:55:20
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_TEMPLATE_H_
#define _GT_TEMPLATE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_TEMPLATE
#include "gt_obj.h"
#include "gt_obj_class.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_template_create(gt_obj_st * parent);

#endif  /** GT_CFG_ENABLE_TEMPLATE */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_TEMPLATE_H_
