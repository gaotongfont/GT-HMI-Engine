/**
 * @file gt_example_calendar.c
 * @author Feyoung
 * @brief Calendar widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_calendar.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/
static void _calendar_event_cb(gt_event_st * e)
{
    printf("calendar date selected\n");
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a calendar widget.
 */
gt_obj_st * gt_examples_calendar(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    gt_obj_st * cal = gt_calendar_create(screen);
    gt_obj_set_pos(cal, 10, 10);
    gt_obj_set_size(cal, 320, 280);
    gt_calendar_set_today(cal, 2026, 3, 27);
    gt_calendar_set_font_size(cal, 14);
    gt_calendar_set_font_color(cal, gt_color_black());
    gt_calendar_set_font_color_to_today(cal, gt_color_hex(0x2196F3));
    gt_calendar_set_font_color_to_weekend(cal, gt_color_hex(0xFF5722));
    gt_calendar_set_font_color_to_other_month(cal, gt_color_hex(0xBDBDBD));
    gt_calendar_set_border_width(cal, 1);
    gt_calendar_set_border_color(cal, gt_color_hex(0xE0E0E0));
    gt_obj_add_event_cb(cal, _calendar_event_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
