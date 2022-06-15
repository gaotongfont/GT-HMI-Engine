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



/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/

struct _gt_timer_s;

/**
 * @brief Timers executed by the timer manager.
 */
typedef void ( * gt_timer_cb_t)(struct _gt_timer_s *);

/**
 * @brief
 */
typedef struct _gt_timer_s {
    uint32_t period;        // when run to 0 milliseconds, this timer should be run
    uint32_t last_run;      // last timestamp of the last run
    gt_timer_cb_t timer_cb; // timer callback
    void * user_data;       // custom user data
    int32_t repeat_count;   // 1: One time; -1 : infinity; n > 0: repeat time
    uint32_t paused : 1;
}_gt_timer_st;

/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

void gt_timer_handler(void);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_TIMER_H_
