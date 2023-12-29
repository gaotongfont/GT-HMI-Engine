/**
 * @file gt_example_button.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-09-28 11:41:32
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "string.h"
#include "../../src/core/gt_event.h"
#include "../../src/widgets/gt_btn.h"
#include "../../src/widgets/gt_label.h"


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
	char buffer[100];

	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "btn %d", ++cnt_idx);
	printf("%s\n", buffer);
	gt_btn_set_text(e->target, buffer);
}


/* global functions / API interface -------------------------------------*/

/**
 * @brief create a button, and click it will printf touch count.
 */
void gt_examples_button(void)
{
    gt_obj_st * screen;
    screen = gt_obj_create(NULL);

    gt_obj_st * btn = gt_btn_create(screen);
	gt_obj_set_pos(btn, 0, 30);
	gt_obj_set_size(btn, 80, 40);
	gt_obj_add_event_cb(btn, btn_click_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_disp_load_scr(screen);
}

#endif
/* end ------------------------------------------------------------------*/
