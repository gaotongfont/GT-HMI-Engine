/**
 * @file gt_timer.h
 * @author Feyoung
 * @brief GUI task timer interface implementation
 * @version 0.1
 * @date 2022-06-14 14:19:29
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_TIMER_H_
#define _GT_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"
#include "stdbool.h"
#include "../gt_conf.h"
#include "../others/gt_list.h"



/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/

struct _gt_timer_s;

/**
 * @brief Timers executed by the timer manager.
 */
typedef void ( * gt_timer_cb_t)(struct _gt_timer_s *);

/**
 * @brief timer control block
 */
typedef struct _gt_timer_s {
    struct _gt_list_head list;
    uint32_t period;            // when run to 0 milliseconds, this timer should be run
    uint32_t last_run;          // last timestamp of the last run
    uint32_t pause_time_diff;   // The time elapsed since the last execution of the timer when it paused
    gt_timer_cb_t timer_cb;     // timer callback
    void * user_data;           // custom user data

    /* 1 << 31: paused, 1bit; 31bit: repeat_count */
    int32_t repeat_count: 31;   // -1 : infinity; n > 0: repeat time;such as: 1 -> it will run in One time;
    uint32_t paused : 1;
}_gt_timer_st;

/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

#if GT_USE_DISPLAY_PREF_IDLE
/**
 * @brief
 *
 * @return uint32_t
 */
uint32_t gt_timer_get_idle(void);
#endif

/**
 * @brief init the list header
 */
void _gt_timer_core_init(void);

/**
 * @brief Create timer node, which will be used to deal with callback.
 *
 * @param callback  function which will be called when timer count to 0.
 * @param period    timers when callback will be called.
 * @param user_data user custom data.
 */
_gt_timer_st * _gt_timer_create(gt_timer_cb_t callback, uint32_t period, void * user_data);

/**
 * @brief Set timer callback
 *
 * @param timer
 * @param callback The callback function to be set
 */
void _gt_timer_set_callback(_gt_timer_st * timer, gt_timer_cb_t callback);

/**
 * @brief Del timer node
 *
 * @param timer timer ptr
 */
void _gt_timer_del(_gt_timer_st * timer);


/**
 * @brief Timer scheduler to handle timer list task
 */
void _gt_timer_handler(void);

/**
 * @brief get laster create timer from timer task list
 *
 * @return _gt_timer_st* The newly created entry object
 */
_gt_timer_st * _gt_timer_get_laster_timer(void);

/**
 * @brief timer repeat times
 *
 * @param timer timer control block
 * @param repeat task repeat times and remove it from timer list,
 *          -1 : infinity; n > 0: repeat time;such as: 1 -> it will run in One time;
 */
void _gt_timer_set_repeat_count(_gt_timer_st * timer, int32_t repeat);

/**
 * @brief Get timer repeat times
 *
 * @param timer
 * @return int32_t
 */
int32_t _gt_timer_get_repeat_count(_gt_timer_st * timer);
/**
 * @brief set timer task period
 *
 * @param timer timer control block
 * @param period task time when period time to run
 */
void _gt_timer_set_period(_gt_timer_st * timer, uint32_t period);

uint32_t _gt_timer_get_period(_gt_timer_st * timer);

/**
 * @brief set timer task paused state
 *
 * @param timer timer control block
 * @param state paused state 0:run, 1:paused
 */
void _gt_timer_set_paused(_gt_timer_st * timer, bool state);

/**
 * @brief Get timer task paused state
 *
 * @param timer
 * @return true timer is paused
 * @return false timer is running
 */
bool _gt_timer_get_paused(_gt_timer_st * timer);

/**
 * @brief Get timer user data
 *
 * @param timer
 * @return void*
 */
void * _gt_timer_get_user_data(_gt_timer_st * timer);

/**
 * @brief Set timer user data
 *
 * @param timer
 * @param user_data
 */
void _gt_timer_set_user_data(_gt_timer_st * timer, void * user_data);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_TIMER_H_
