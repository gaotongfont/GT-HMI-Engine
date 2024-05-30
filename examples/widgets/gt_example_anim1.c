/**
 * @file gt_example_anim1.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-11-29 19:43:40
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "string.h"
#include "../gt_examples.h"
#include "../../src/core/gt_mem.h"
#include "../../src/core/gt_event.h"
#include "../../src/widgets/gt_btn.h"
#include "../../src/widgets/gt_label.h"
#include "../../src/others/gt_anim.h"


/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _btn_exec_cb(void * target, int32_t value) {
	gt_obj_st * obj = (gt_obj_st * )target;
	// gt_obj_set_y(obj, value);
	gt_obj_set_pos(obj, obj->area.x, value);
	printf("_btn_exec_cb: %d\n", value);
}

static void btn_click_cb(gt_event_st * e) {
	static int cnt_idx = 0;
	char buffer[100];

	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "btn %d", ++cnt_idx);
	printf("%s\n", buffer);
	gt_btn_set_text(e->target, buffer);

	gt_anim_st anim;
	gt_anim_init(&anim);
	gt_anim_set_target(&anim, e->target);
	gt_anim_set_exec_cb(&anim, _btn_exec_cb);
    if (e->target->area.y == 30) {
        gt_anim_set_value(&anim, 30, 400);
        gt_anim_set_path_type(&anim, GT_ANIM_PATH_TYPE_OVERSHOOT);
    } else {
        gt_anim_set_value(&anim, 400, 30);
        gt_anim_set_path_type(&anim, GT_ANIM_PATH_TYPE_OVERSHOOT);
    }
	gt_anim_start(&anim);
}


/* global functions / API interface -------------------------------------*/

/**
 * @brief create a button, and click it will printf touch count and display
 *  animation
 */
void gt_examples_anim1(void)
{
    gt_obj_st * screen;
    screen = gt_obj_create(NULL);

    gt_obj_st * btn = gt_btn_create(screen);
	gt_obj_set_pos(btn, 0, 30);
	gt_obj_set_size(btn, 200, 100);
	gt_obj_add_event_cb(btn, btn_click_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_disp_load_scr(screen);
}

#endif
/* end ------------------------------------------------------------------*/
