/**
 * @file gt_serial_event.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-10-29 17:50:26
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SERIAL_EVENT_H_
#define _GT_SERIAL_EVENT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_serial.h"

#if GT_USE_SERIAL && GT_USE_BIN_CONVERT
#include "../../others/gt_types.h"
#include "../../widgets/gt_obj.h"



/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

void gt_serial_event_init(void);

gt_res_t gt_serial_event_init_by_page(uint16_t page_index, gt_obj_st * parent);

#endif  /** GT_USE_SERIAL && GT_USE_BIN_CONVERT */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SERIAL_EVENT_H_
