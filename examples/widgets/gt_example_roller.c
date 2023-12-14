/**
 * @file gt_example_roller.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2023-12-12 11:10:18
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_examples.h"
#include "../../src/widgets/gt_roller.h"
#include "../../src/widgets/gt_btn.h"
#include "../../src/core/gt_event.h"
#include "../../src/others/gt_log.h"


/* private define -------------------------------------------------------*/

#if GT_USE_EXAMPLES


/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void gt_roller_prev_cb(gt_event_st * e) {
	gt_obj_st * target = (gt_obj_st * )e->user_data;
	GT_LOGD("", "prev");
	gt_roller_go_prev(target);
}

static void gt_roller_next_cb(gt_event_st * e) {
	gt_obj_st * target = (gt_obj_st * )e->user_data;
	GT_LOGD("", "next");
	gt_roller_go_next(target);
}


/* global functions / API interface -------------------------------------*/

void gt_examples_roller(void)
{
	gt_obj_st * screen_home = gt_obj_create(NULL);
	gt_screen_set_bgcolor(screen_home, gt_color_hex(0xffffff));
	gt_obj_set_pos(screen_home, 0, 0);

	gt_obj_st * roller = gt_roller_create(screen_home);
	gt_obj_set_pos(roller, 20, 100);
	// gt_roller_set_display_item_count(roller, 5);
#if 0
	gt_roller_set_options(roller,
		"test0\ntest1\ntest2\ntest3\ntest4\ntest5\ntest6\ntest7\ntest8\ntest9\ntest10",
		GT_ROLLER_MODE_INFINITE);
#else
	gt_roller_set_options(roller,
		"test0\ntest1\ntest2\ntest3\ntest4\ntest5\ntest6\ntest7\ntest8\ntest9\ntest10",
		GT_ROLLER_MODE_NORMAL);
#endif
	// gt_roller_set_line_space(roller, 20);

	gt_obj_st * prev = gt_btn_create(screen_home);
	gt_obj_set_pos(prev, 0, 0);
	gt_btn_set_text(prev, "prev");
	gt_obj_add_event_cb(prev, gt_roller_prev_cb, GT_EVENT_TYPE_INPUT_RELEASED, roller);

	gt_obj_st * next = gt_btn_create(screen_home);
	gt_obj_set_pos(next, 100, 0);
	gt_btn_set_text(next, "next");
	gt_obj_add_event_cb(next, gt_roller_next_cb, GT_EVENT_TYPE_INPUT_RELEASED, roller);
}


#endif  /** GT_USE_EXAMPLES */
/* end ------------------------------------------------------------------*/
