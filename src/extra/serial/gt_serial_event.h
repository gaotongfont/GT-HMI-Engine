/**
 * @file gt_serial_event.h
 * @author Feyoung
 * @brief Add serial event by set callback function template
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
#include "../../utils/gt_serial.h"

#if GT_USE_SERIAL && GT_USE_BIN_CONVERT
#include "../../others/gt_types.h"
#include "../../widgets/gt_obj.h"



/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief Get event header data by address, default header data:
 *      [0x13, 0x48, 0x4d, 0x49, 0x5f, 0x45, 0x56, page_count_h, page_count_l, event_data_len, 0x00]
 * pages:
 *   0. [event_count, addr_h, addr_m, addr_l]
 *      ...
 *   n. [event_count, addr_h, addr_m, addr_l], n = page_count - 1.
 */
void gt_serial_event_init(void);

/**
 * @brief Dynamically adds a callback function event template to a
 *      control. When releasing a control, the global variable is released
 *
 * @param page_index
 * @param parent
 * @return gt_res_t
 */
gt_res_t gt_serial_event_init_by_page(uint16_t page_index, gt_obj_st * parent);

#endif  /** GT_USE_SERIAL && GT_USE_BIN_CONVERT */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SERIAL_EVENT_H_
