/**
 * @file gt_example_input.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-09-30 14:09:59
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/

#include "../gt_examples.h"
#include "../../src/core/gt_event.h"
#include "../../src/widgets/gt_input.h"


/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES


/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _click_cb(gt_event_st * e)
{
    gt_input_append_value(e->target, "test ");
}


/* global functions / API interface -------------------------------------*/

void gt_example_input(void)
{
    gt_obj_st * screen;
    screen = gt_obj_create(NULL);

    gt_obj_st * input = gt_input_create(screen);
    gt_input_set_placeholder(input, "This is placeholder content.");
	gt_obj_set_pos(input, 0, 0);
	gt_obj_set_size(input, 180, 80);
	gt_obj_add_event_cb(input, _click_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_disp_load_scr(screen);
}

#endif
/* end ------------------------------------------------------------------*/
