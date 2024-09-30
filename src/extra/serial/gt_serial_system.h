/**
 * @file gt_serial_system.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-09-27 15:35:59
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SERIAL_SYSTEM_H_
#define _GT_SERIAL_SYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../utils/gt_serial.h"

#if GT_USE_SERIAL && GT_USE_BIN_CONVERT
#include "../../others/gt_types.h"



/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

gt_res_t gt_serial_system_set_value(uint8_t const * const buffer, uint32_t len);

uint16_t gt_serial_system_get_value(uint8_t * const res, uint8_t const * const buffer, uint32_t len);

#endif  /** GT_USE_SERIAL */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SERIAL_SYSTEM_H_
