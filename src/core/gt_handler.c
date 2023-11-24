/**
 * @file gt_handler.c
 * @author yongg
 * @brief GUI task management interface
 * @version 0.1
 * @date 2022-05-11 15:03:23
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

 /* include --------------------------------------------------------------*/
#include "../gt_conf.h"
#include "gt_mem.h"
#include "gt_timer.h"
#include "stdbool.h"
#include "../../driver/gt_port_disp.h"
#include "../../driver/gt_port_indev.h"
#include "../../driver/gt_port_vf.h"
#include "../../driver/gt_port_src.h"
#include "../others/gt_anim.h"
#include "gt_event.h"
#include "../extra/gt_extra.h"
#include "../core/gt_img_decoder.h"

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

static void _gt_port_init(void)
{
    gt_port_disp_init();
    gt_port_indev_init();
    gt_port_vf_init();
    gt_port_src_init();
}
/* global functions / API interface -------------------------------------*/

void gt_init(void)
{
    if (_gt_init_is_done()) {
        return ;
    }

    gt_mem_init();

    _gt_timer_core_init();

    _gt_event_init();

    _gt_anim_core_init();

    _gt_img_decoder_init();

    _gt_extra_init();

    _gt_init_set_flag(true);

    _gt_port_init();
}

void gt_task_handler(void)
{
    if (!_gt_init_is_done()) {
        return ;
    }
    _gt_timer_handler();
}



/* end ------------------------------------------------------------------*/
