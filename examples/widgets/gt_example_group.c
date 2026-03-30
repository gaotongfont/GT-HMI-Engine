/**
 * @file gt_example_group.c
 * @author Feyoung
 * @brief Group widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_group.h"
#include "../../src/widgets/gt_radio.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/
static void _group_event_cb(gt_event_st * e)
{
    gt_obj_st * active = gt_group_get_active_obj(e->target, GT_TYPE_RADIO);
    if (active) {
        printf("group: radio selected\n");
    }
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a group with radio buttons for mutual exclusion.
 */
gt_obj_st * gt_examples_group(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    gt_obj_st * group = gt_group_create(screen);
    gt_obj_set_pos(group, 10, 10);
    gt_obj_set_size(group, 200, 150);
    gt_obj_add_event_cb(group, _group_event_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * r1 = gt_radio_create(group);
    gt_radio_set_text(r1, "Choice A");
    gt_obj_set_pos(r1, 10, 10);
    gt_obj_set_size(r1, 150, 30);

    gt_obj_st * r2 = gt_radio_create(group);
    gt_radio_set_text(r2, "Choice B");
    gt_obj_set_pos(r2, 10, 50);
    gt_obj_set_size(r2, 150, 30);

    gt_obj_st * r3 = gt_radio_create(group);
    gt_radio_set_text(r3, "Choice C");
    gt_obj_set_pos(r3, 10, 90);
    gt_obj_set_size(r3, 150, 30);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
