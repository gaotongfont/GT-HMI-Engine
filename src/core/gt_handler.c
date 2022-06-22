/**
 * @file gt_handler.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-11 15:03:23
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

 /* include --------------------------------------------------------------*/
#include "../gt_conf.h"
#include "gt_mem.h"
#include "gt_timer.h"
#include "../others/gt_anim.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief gt gui init
 */
void gt_init(void)
{
    gt_mem_init();

    _gt_timer_init();

    _gt_anim_init();
}

void gt_task_handler(void)
{
    gt_timer_handler();
}



/* end ------------------------------------------------------------------*/
