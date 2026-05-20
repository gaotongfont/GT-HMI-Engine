/**
 * @file gt_hal_tick.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-05-19 17:26:34
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

 /* include --------------------------------------------------------------*/
#include "gt_hal_tick.h"
#include "stdint.h"
#include "../core/gt_handler.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static volatile uint32_t _tick = 0;
static volatile uint8_t tick_irq_flag = 0;

/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_tick_inc(uint32_t ms)
{
    _tick += ms;
    tick_irq_flag = 0;
}

uint32_t gt_tick_get(void)
{
    uint32_t ret;
    tick_irq_flag = 1;
    ret = _tick;
    /* If an interrupt fired gt_tick_inc() between the two lines above,
     * tick_irq_flag was cleared to 0 — re-read once to get the updated value. */
    if (!tick_irq_flag) {
        ret = _tick;
    }
    return ret;
}

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

void gt_sleep_ms(uint32_t ms)
{
}

void gt_sleep_sec(uint32_t sec)
{
    gt_sleep_ms(sec * 1000);
}


/* end ------------------------------------------------------------------*/
