/**
 * @file gt_example_progress_bar.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-10-25 15:24:38
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_progress_bar.h"



/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES


/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static int value = 0;


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void _progress_bar_event_cb(gt_event_st * e)
{
    gt_progress_bar_set_pos(e->target, ++value);
    if (value > gt_progress_bar_get_total(e->target)) {
        value = 0;
    }
}

/* global functions / API interface -------------------------------------*/
void gt_examples_progress_bar(void)
{
    gt_obj_st * screen;

    screen = gt_obj_create(NULL);

    gt_obj_st * progress_bar = gt_progress_bar_create(screen);
    gt_progress_bar_set_start_end(progress_bar, 0, 200);
    gt_progress_bar_set_pos(progress_bar, value);

	gt_obj_set_pos(progress_bar, 0, 60);
    gt_obj_set_size(progress_bar, 380, 40);
    gt_obj_add_event_cb(progress_bar, _progress_bar_event_cb, GT_EVENT_TYPE_INPUT_PRESSING, NULL);

    gt_disp_load_scr(screen);
}


#endif
/* end ------------------------------------------------------------------*/
