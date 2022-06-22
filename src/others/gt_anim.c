/**
 * @file gt_anim.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-06-21 17:59:29
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_anim.h"
#include "../core/gt_timer.h"
#include "../others/gt_log.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _gt_anim_task_handler(struct _gt_timer_s * timer)
{
    GT_LOGD(GT_LOG_TAG_GUI, "call back");
}


/* global functions / API interface -------------------------------------*/

void _gt_anim_init(void)
{
    _gt_timer_create(_gt_anim_task_handler, GT_TASK_PERIOD_TIME_ANIM, NULL);
}


/* end ------------------------------------------------------------------*/
