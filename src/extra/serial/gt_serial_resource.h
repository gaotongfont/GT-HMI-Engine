/**
 * @file gt_serial_resource.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-11-11 18:03:00
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SERIAL_RESOURCE_H_
#define _GT_SERIAL_RESOURCE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../../gt_conf.h"
#include "../../others/gt_types.h"

#if GT_USE_SERIAL && GT_USE_BIN_CONVERT


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

gt_res_t gt_serial_resource_init(void);

/**
 * @brief Get the address of the resource by the file id
 *
 * @param file_id
 * @return uint32_t 0xFFFFFFFF: failed; other: success
 */
uint32_t gt_serial_resource_get_addr_by(uint8_t file_id);


#endif  /** GT_USE_SERIAL */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SERIAL_RESOURCE_H_
