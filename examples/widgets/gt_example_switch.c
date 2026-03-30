/**
 * @file gt_example_switch.c
 * @author Feyoung
 * @brief Switch widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_switch.h"
#include "../../src/widgets/gt_label.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/
static void _switch_event_cb(gt_event_st * e)
{
    gt_state_et state = gt_obj_get_state(e->target);
    printf("switch state: %s\n", GT_STATE_PRESSED == state ? "ON" : "OFF");
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create switch widgets with different styles.
 */
gt_obj_st * gt_examples_switch(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    gt_obj_st * label1 = gt_label_create(screen);
    gt_label_set_text(label1, "Default:");
    gt_obj_set_pos(label1, 10, 30);
    gt_obj_set_size(label1, 80, 30);

    gt_obj_st * sw1 = gt_switch_create(screen);
    gt_obj_set_pos(sw1, 100, 30);
    gt_obj_set_size(sw1, 60, 30);
    gt_obj_add_event_cb(sw1, _switch_event_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * label2 = gt_label_create(screen);
    gt_label_set_text(label2, "Custom:");
    gt_obj_set_pos(label2, 10, 80);
    gt_obj_set_size(label2, 80, 30);

    gt_obj_st * sw2 = gt_switch_create(screen);
    gt_obj_set_pos(sw2, 100, 80);
    gt_obj_set_size(sw2, 60, 30);
    gt_switch_set_color_act(sw2, gt_color_hex(0x00CC00));
    gt_switch_set_color_ina(sw2, gt_color_hex(0xCCCCCC));
    gt_switch_set_color_point(sw2, gt_color_white());
    gt_obj_add_event_cb(sw2, _switch_event_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
