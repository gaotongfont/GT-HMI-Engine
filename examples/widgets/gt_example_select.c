/**
 * @file gt_example_select.c
 * @author Feyoung
 * @brief Select (dropdown) widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_select.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/
static void _select_event_cb(gt_event_st * e)
{
    printf("select item changed\n");
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a dropdown select widget.
 */
gt_obj_st * gt_examples_select(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    gt_obj_st * sel = gt_select_create(screen);
    gt_obj_set_pos(sel, 10, 30);
    gt_obj_set_size(sel, 200, 40);
    gt_select_add_option(sel, "Option 1");
    gt_select_add_option(sel, "Option 2");
    gt_select_add_option(sel, "Option 3");
    gt_select_add_option(sel, "Option 4");
    gt_select_set_font_size(sel, 16);
    gt_select_set_font_color(sel, gt_color_black());
    gt_select_set_option_height(sel, 36);
    gt_obj_add_event_cb(sel, _select_event_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
