/**
 * @file gt_serial_var_rtc.h
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-10-23 09:47:46
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SERIAL_VAR_RTC_H_
#define _GT_SERIAL_VAR_RTC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_serial_var.h"

#if GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_res_t gt_serial_var_rtc_create(gt_var_st* var);
gt_res_t gt_serial_var_rtc_set_value(gt_var_st* var, gt_var_value_st* value);

#endif /* GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT */
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SERIAL_VAR_RTC_H_

/* end of file ----------------------------------------------------------*/


