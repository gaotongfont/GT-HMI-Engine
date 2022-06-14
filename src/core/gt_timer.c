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


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief Create timer node, which will be used to deal with callback.
 *
 * @param callback  function which will be called when timer count to 0.
 * @param period    timers when callback will be called.
 * @param user_data user custom data.
 */
void gt_timer_create(gt_timer_cb_t callback, uint32_t period, void * user_data)
{

}

/**
 * @brief Timer scheduler to handle timer list task
 */
void gt_timer_handler(void)
{

}

/* end ------------------------------------------------------------------*/
