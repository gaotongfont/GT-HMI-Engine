/**
 * @file gt_hal_tick.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-19 17:26:34
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

 /* include --------------------------------------------------------------*/
#include "gt_hal_tick.h"
#include "stdint.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static uint32_t _tick = 0;
static volatile uint8_t tick_irq_flag;

/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief set increase the tick
 *
 * @param ms milliseconds
 */
void gt_tick_inc(uint32_t ms)
{
    tick_irq_flag = 0;
    _tick += ms;
}

/**
 * @brief get task tick
 *
 * @return uint32_t the milliseconds of task
 */
uint32_t gt_tick_get()
{
    uint32_t ret;

    do {
        tick_irq_flag = 1;
        ret = _tick;
    } while(!tick_irq_flag);

    return ret;
}

/**
 * @brief get elapsed time of tick count
 *
 * @param prev_tick last tick stamp
 * @return uint32_t the elapsed tick count [ms]
 */
uint32_t gt_tick_elapse(uint32_t prev_tick)
{
    uint32_t tick = gt_tick_get();

    if (tick >= prev_tick) {
        return tick - prev_tick;
    }

    //overflow uint32_t
    prev_tick = UINT32_MAX - prev_tick + 1;
    return prev_tick + tick;
}

/* end ------------------------------------------------------------------*/
