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
#include "stdbool.h"
#include "../others/gt_anim.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/

static GT_ATTRIBUTE_LARGE_RAM_ARRAY bool _gt_init_done_flag = false;


/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _gt_init_set_flag(bool flag)
{
    _gt_init_done_flag = flag;
}

static bool _gt_init_is_done(void)
{
    return _gt_init_done_flag;
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief gt gui init
 */
void gt_init(void)
{
    if (_gt_init_is_done()) {
        return ;
    }

    gt_mem_init();

    _gt_timer_init();

    _gt_anim_init();


    _gt_init_set_flag(true);
}

void gt_task_handler(void)
{
    if (!_gt_init_is_done()) {
        return ;
    }
    _gt_timer_handler();
}



/* end ------------------------------------------------------------------*/
