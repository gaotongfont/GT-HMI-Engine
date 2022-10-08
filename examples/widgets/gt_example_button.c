/**
 * @file gt_example_button.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-09-28 11:41:32
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/core/gt_event.h"
#include "../../src/widgets/gt_btn.h"


/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void btn_click_cb(gt_event_st * e)
{
	static int cnt_idx = 0;

	printf("clicking %d\n", ++cnt_idx);
}


/* global functions / API interface -------------------------------------*/

/**
 * @brief create a button, and click it will printf touch count.
 */
void gt_example_button(void)
{
    gt_obj_st * screen;
    screen = gt_obj_create(NULL);
	gt_disp_set_scr(screen);

    gt_obj_st * btn = gt_btn_create(screen);
	gt_obj_set_style_pos(btn, 0, 0);
	gt_obj_set_style_size(btn, 80, 40);
	gt_obj_add_event_cb(btn, btn_click_cb, GT_EVENT_TYPE_INPUT_PROCESSED, NULL);

    gt_disp_load_scr(screen);
}

#endif
/* end ------------------------------------------------------------------*/
