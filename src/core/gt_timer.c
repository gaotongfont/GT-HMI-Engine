/**
 * @file gt_timer.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-06-14 14:18:54
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

 /* include --------------------------------------------------------------*/
#include "gt_timer.h"
#include "stdint.h"
#include "stdbool.h"
#include "../core/gt_mem.h"
#include "../others/gt_gc.h"
#include "../others/gt_log.h"
#include "../hal/gt_hal_tick.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/

/**
 * @brief timer task list ready to run
 */
static bool _gt_timer_task_ready_flag = false;


/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static inline void _gt_timer_task_set_ready_flag(bool value)
{
    _gt_timer_task_ready_flag = value;
}

static inline bool _gt_timer_task_is_ready(void)
{
    return _gt_timer_task_ready_flag;
}

/**
 * @brief execute the timer tasks list
 */
static inline void _gt_timer_for_each_exec(void)
{
    _gt_timer_st * ptr      = NULL;
    uint32_t     period     = 0;
    uint32_t     start_time = 0;

    gt_list_for_each_entry(ptr, &_GT_GC_GET_ROOT(_gt_timer_ll), list) {
        if (ptr->paused) {
            continue;
        }

        start_time = gt_tick_get();
        period     = start_time - ptr->last_run;

        if (period < ptr->period) {
            continue;
        }

        if (ptr->timer_cb) {
            ptr->timer_cb(ptr);
            GT_LOGV(GT_LOG_TAG_TIM, "[%d, %d, +%d]", start_time, ptr->last_run, gt_tick_get() - ptr->last_run);
            ptr->last_run = gt_tick_get();
        }
    }
}


/* global functions / API interface -------------------------------------*/

/**
 * @brief init the list header
 */
void _gt_timer_init(void)
{
    GT_INIT_LIST_HEAD(&_GT_GC_GET_ROOT(_gt_timer_ll));
}

/**
 * @brief Create timer node, which will be used to deal with callback.
 *
 * @param callback  function which will be called when timer count to 0.
 * @param period    timers when callback will be called.
 * @param user_data user custom data.
 */
_gt_timer_st * _gt_timer_create(gt_timer_cb_t callback, uint32_t period, void * user_data)
{
    GT_CHECK(callback, NULL);

    _gt_timer_st * timer = gt_mem_malloc(sizeof(_gt_timer_st));
    GT_CHECK(timer, NULL);

    timer->period       = period;
    timer->last_run     = gt_tick_get();
    timer->timer_cb     = callback;
    timer->user_data    = user_data;
    timer->repeat_count = -1;
    timer->paused       = 0;
    GT_INIT_LIST_HEAD(&timer->list);

    gt_list_add(&timer->list, &_GT_GC_GET_ROOT(_gt_timer_ll));

    _gt_timer_task_set_ready_flag(true);

    GT_LOGV(GT_LOG_TAG_TIM, ">>> period: %d <<<", period);

    return timer;
}


/**
 * @brief Timer scheduler to handle timer list task
 */
void gt_timer_handler(void)
{
    if (!_gt_timer_task_is_ready()) {
        return ;
    }
    _gt_timer_for_each_exec();
}

/* end ------------------------------------------------------------------*/
