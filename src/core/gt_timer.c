/**
 * @file gt_timer.c
 * @author Feyoung
 * @brief GUI task timer interface implementation
 * @version 0.1
 * @date 2022-06-14 14:18:54
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

 /* include --------------------------------------------------------------*/
#include "gt_timer.h"
#include "stdint.h"
#include "../core/gt_mem.h"
#include "../others/gt_gc.h"
#include "../others/gt_log.h"
#include "../hal/gt_hal_tick.h"

/* private define -------------------------------------------------------*/
#define _IDLE_PERIOD    500 /* [ms] */


/* private typedef ------------------------------------------------------*/

/**
 * @brief timer task list ready to run
 */
static GT_ATTRIBUTE_LARGE_RAM_ARRAY bool _gt_timer_task_ready_flag = false;


#if GT_USE_DISPLAY_PREF_IDLE
static uint32_t idle_last = 0;
#endif

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

static void _gt_timer_remove_task(_gt_timer_st *ptr)
{
    GT_CHECK_BACK(ptr);
    _gt_list_del(&ptr->list);
    gt_mem_free(ptr);
}

/**
 * @brief execute the timer tasks list
 */
static inline uint32_t _gt_timer_for_each_exec(void)
{
    _gt_timer_st * ptr        = NULL;
    _gt_timer_st * backup_ptr = NULL;
    uint32_t     period       = 0;
    uint32_t     start_time   = gt_tick_get();

    _gt_list_for_each_entry_safe(ptr, backup_ptr, &_GT_GC_GET_ROOT(_gt_timer_ll), _gt_timer_st, list) {
        if (!ptr->repeat_count) {
            _gt_timer_remove_task(ptr);
            continue;
        }
        if (ptr->paused) {
            continue;
        }

        period = gt_tick_get() - ptr->last_run;

        if (period < ptr->period) {
            continue;
        }

        if (ptr->timer_cb) {

            if (ptr->repeat_count > 0) {
                --ptr->repeat_count;
            }

            ptr->timer_cb(ptr);
            // GT_LOGD(GT_LOG_TAG_TIM, "[%d, %d, +%d]", ptr->last_run, start_time, gt_tick_get() - ptr->last_run);
            ptr->last_run = gt_tick_get();
        }
    }

    return start_time;
}


/* global functions / API interface -------------------------------------*/

#if GT_USE_DISPLAY_PREF_IDLE
uint32_t gt_timer_get_idle(void)
{
    return idle_last;
}
#endif

void _gt_timer_core_init(void)
{
    _GT_INIT_LIST_HEAD(&_GT_GC_GET_ROOT(_gt_timer_ll));
}

_gt_timer_st * _gt_timer_create(gt_timer_cb_t callback, uint32_t period, void * user_data)
{
    GT_CHECK_BACK_VAL(callback, NULL);
    _gt_timer_st * timer = gt_mem_malloc(sizeof(_gt_timer_st));
    if (!timer) {
        return NULL;
    }

    _GT_INIT_LIST_HEAD(&timer->list);

    timer->period          = period;
    timer->timer_cb        = callback;
    timer->user_data       = user_data;
    timer->repeat_count    = -1;
    timer->paused          = 0;
    timer->pause_time_diff = 0;
    timer->last_run        = gt_tick_get();

    _gt_list_add(&timer->list, &_GT_GC_GET_ROOT(_gt_timer_ll));
    _gt_timer_task_set_ready_flag(true);
    return timer;
}

void _gt_timer_set_callback(_gt_timer_st * timer, gt_timer_cb_t callback)
{
    GT_CHECK_BACK(timer);
    timer->timer_cb = callback;
}

void _gt_timer_del(_gt_timer_st * timer)
{
    GT_CHECK_BACK(timer);
    _gt_timer_set_paused(timer, true);
    _gt_timer_set_repeat_count(timer, 0);   /** remove timer by core handler */
    _gt_timer_set_callback(timer, NULL);
    _gt_timer_set_user_data(timer, NULL);
}

void _gt_timer_handler(void)
{
    if (!_gt_timer_task_is_ready()) {
        return;
    }
#if GT_USE_DISPLAY_PREF_IDLE
    uint32_t handler_start = _gt_timer_for_each_exec();

    static uint32_t busy_time = 0;
    static uint32_t idle_period_start = 0;

    busy_time += gt_tick_elapse(handler_start);
    uint32_t idle_period_time = gt_tick_elapse(idle_period_start);
    if (idle_period_time >= _IDLE_PERIOD) {
        idle_last = (busy_time * 100) / idle_period_time;
        idle_last = idle_last > 100 ? 0 : 100 - idle_last;
        GT_LOGD(GT_LOG_TAG_GUI, "CPU used: %d/100\n", 100 - idle_last);
        busy_time = 0;
        idle_period_start = gt_tick_get();
    }
#else
    _gt_timer_for_each_exec();
#endif
}

_gt_timer_st * _gt_timer_get_laster_timer(void)
{
    return _gt_list_first_entry(&_GT_GC_GET_ROOT(_gt_timer_ll), _gt_timer_st, list);
}

void _gt_timer_set_repeat_count(_gt_timer_st * timer, int32_t repeat)
{
    GT_CHECK_BACK(timer);
    timer->repeat_count = repeat;
}

int32_t _gt_timer_get_repeat_count(_gt_timer_st * timer)
{
    GT_CHECK_BACK_VAL(timer, 0);
    return timer->repeat_count;
}

void _gt_timer_set_period(_gt_timer_st * timer, uint32_t period)
{
    GT_CHECK_BACK(timer);
    timer->period = period;
}

uint32_t _gt_timer_get_period(_gt_timer_st * timer)
{
    GT_CHECK_BACK_VAL(timer, 0);
    return timer->period;
}

void _gt_timer_set_paused(_gt_timer_st * timer, bool state)
{
    GT_CHECK_BACK(timer);
    if (state) {
        /** remark current time more then last run time */
        timer->pause_time_diff = gt_tick_get() - timer->last_run;
        if (timer->pause_time_diff > timer->period) {
            timer->pause_time_diff = 0;
        }
    } else {
        /** reset last run timestamp to now */
        timer->last_run = gt_tick_get() - timer->pause_time_diff;
        timer->pause_time_diff = 0;
    }
    timer->paused = (uint8_t)state;
}

bool _gt_timer_get_paused(_gt_timer_st * timer)
{
    GT_CHECK_BACK_VAL(timer, true);
    return (bool)timer->paused;
}

void _gt_timer_set_user_data(_gt_timer_st * timer, void * user_data)
{
    GT_CHECK_BACK(timer);
    timer->user_data = user_data;
}

void * _gt_timer_get_user_data(_gt_timer_st * timer)
{
    GT_CHECK_BACK_VAL(timer, NULL);
    return timer->user_data;
}

/* end ------------------------------------------------------------------*/
