/**
 * @file gt_timer.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-06-14 14:19:29
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_TIMER_H_
#define _GT_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"
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
    uint32_t period;        // when run to 0 milliseconds, this timer should be run
    uint32_t last_run;      // last timestamp of the last run
    gt_timer_cb_t timer_cb; // timer callback
    void * user_data;       // custom user data
    int32_t repeat_count;   // 1: One time; -1 : infinity; n > 0: repeat time
    uint32_t paused : 1;
    struct gt_list_head list;
}_gt_timer_st;

/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief init the list header
 */
void _gt_timer_init(void);

/**
 * @brief Create timer node, which will be used to deal with callback.
 *
 * @param callback  function which will be called when timer count to 0.
 * @param period    timers when callback will be called.
 * @param user_data user custom data.
 */
_gt_timer_st * _gt_timer_create(gt_timer_cb_t callback, uint32_t period, void * user_data);

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
 * @param repeat task repeat times and remove it from timer list
 */
void _gt_timer_set_repeat_count(_gt_timer_st * timer, int32_t repeat);

/**
 * @brief set timer task period
 *
 * @param timer timer control block
 * @param period task time when period time to run
 */
void _gt_timer_set_period(_gt_timer_st * timer, uint32_t period);

/**
 * @brief set timer task paused state
 * 
 * @param timer timer control block
 * @param state paused state 0:run, 1:paused
 */
void _gt_timer_set_paused(_gt_timer_st * timer, bool state);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_TIMER_H_
