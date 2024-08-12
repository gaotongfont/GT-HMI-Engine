/**
 * @file gt_example_checkbox.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-09-29 18:58:51
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/

#include "../gt_examples.h"
#include "../../src/widgets/gt_checkbox.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES


/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _gt_check_event_cb(gt_event_st * e)
{
    printf("checkbox state: %s\n", GT_STATE_PRESSED == gt_obj_get_state(e->target) ? "selected" : "unselected");
}


/* global functions / API interface -------------------------------------*/
void gt_examples_checkbox(void)
{
    gt_obj_st * screen;
    screen = gt_obj_create(NULL);

    gt_obj_st * checkbox = gt_checkbox_create(screen);
    gt_checkbox_set_text(checkbox, "Enabled");
	gt_obj_set_pos(checkbox, 0, 0);
	gt_obj_set_size(checkbox, 80, 20);
	gt_obj_add_event_cb(checkbox, _gt_check_event_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_disp_load_scr(screen);
}


#endif
/* end ------------------------------------------------------------------*/
