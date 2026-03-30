/**
 * @file gt_example_arc.c
 * @author Feyoung
 * @brief Arc widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_arc.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/
static void _arc_event_cb(gt_event_st * e)
{
    printf("arc touched\n");
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create arc widgets demo.
 */
gt_obj_st * gt_examples_arc(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    gt_obj_st * arc = gt_arc_create(screen);
    gt_obj_set_pos(arc, 50, 30);
    gt_obj_set_size(arc, 150, 150);
    gt_arc_set_bg_color(arc, gt_color_hex(0xE0E0E0));
    gt_arc_set_bg_start_end_angle(arc, 0, 360);
    gt_arc_set_bg_width(arc, 12);
    gt_arc_set_act_color(arc, gt_color_hex(0x2196F3));
    gt_arc_set_act_start_end_angle(arc, 0, 270);
    gt_arc_set_act_width(arc, 12);
    gt_arc_set_rounded(arc, true);
    gt_arc_set_value_min_max(arc, 0, 100);
    gt_arc_set_value(arc, 75);
    gt_obj_add_event_cb(arc, _arc_event_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
