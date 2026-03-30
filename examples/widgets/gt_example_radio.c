/**
 * @file gt_example_radio.c
 * @author Feyoung
 * @brief Radio button example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_radio.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/
static void _radio_event_cb(gt_event_st * e)
{
    printf("radio selected\n");
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create radio buttons demo.
 */
gt_obj_st * gt_examples_radio(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    gt_obj_st * radio1 = gt_radio_create(screen);
    gt_radio_set_text(radio1, "Option A");
    gt_obj_set_pos(radio1, 10, 30);
    gt_obj_set_size(radio1, 120, 30);
    gt_obj_add_event_cb(radio1, _radio_event_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * radio2 = gt_radio_create(screen);
    gt_radio_set_text(radio2, "Option B");
    gt_obj_set_pos(radio2, 10, 70);
    gt_obj_set_size(radio2, 120, 30);
    gt_obj_add_event_cb(radio2, _radio_event_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * radio3 = gt_radio_create(screen);
    gt_radio_set_text(radio3, "Option C");
    gt_obj_set_pos(radio3, 10, 110);
    gt_obj_set_size(radio3, 120, 30);
    gt_obj_add_event_cb(radio3, _radio_event_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
