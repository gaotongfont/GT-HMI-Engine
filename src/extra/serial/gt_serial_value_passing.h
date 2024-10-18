/**
 * @file gt_serial_value_passing.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-10-15 16:44:12
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SERIAL_VALUE_PASSING_H_
#define _GT_SERIAL_VALUE_PASSING_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/



/* define ---------------------------------------------------------------*/
#include "gt_serial_var.h"

#if GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_res_t gt_serial_value_passing_create(gt_var_st* var);
gt_res_t gt_serial_value_passing_set_value(gt_var_st* var, gt_var_value_st* value);


#endif  /** GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SERIAL_VALUE_PASSING_H_
