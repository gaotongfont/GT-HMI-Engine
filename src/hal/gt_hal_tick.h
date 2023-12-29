/**
 * @file gt_hal_tick.h
 * @author Feyoung
 * @brief  Provide access to the system tick with 1 millisecond resolution
 * @version 0.1
 * @date 2022-05-19 17:26:21
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_HAL_TICK_H_
#define _GT_HAL_TICK_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief You have to call this function periodically
 *
 * @param ms the call period of this function in milliseconds
 */
void gt_tick_inc(uint32_t ms);

/**
 * @brief Get the elapsed milliseconds since start up
 *
 * @return uint32_t the elapsed milliseconds
 */
uint32_t gt_tick_get(void);

/**
 * @brief Get the elapsed milliseconds since a previous time stamp
 *
 * @param prev_tick a previous time stamp
 * @return uint32_t the elapsed milliseconds since 'prev_tick'
 */
uint32_t gt_tick_elapse(uint32_t prev_tick);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_TICK_H_
