/**
 * @file gt_serial_area_light.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-10-18 16:30:47
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SERIAL_AREA_LIGHT_H_
#define _GT_SERIAL_AREA_LIGHT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_serial_var.h"

#if GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_res_t gt_serial_area_light_create(gt_var_st * var);
gt_res_t gt_serial_area_light_set_value(gt_var_st * var, gt_var_value_st * value);


#endif  /** GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SERIAL_AREA_LIGHT_H_
