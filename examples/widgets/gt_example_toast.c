/**
 * @file gt_example_toast.c
 * @author Feyoung
 * @brief Toast widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_toast.h"
#include "../../src/widgets/gt_btn.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/
static void _btn_toast_cb(gt_event_st * e)
{
    gt_toast_param_st param;
    gt_toast_reset(&param);
    param.message = "Hello, this is a toast!";
    param.duration = 2000;
    param.position = GT_TOAST_POS_MIDDLE;
    gt_toast(&param);
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a button that shows a toast message.
 */
gt_obj_st * gt_examples_toast(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    gt_obj_st * btn = gt_btn_create(screen);
    gt_obj_set_pos(btn, 100, 100);
    gt_obj_set_size(btn, 120, 40);
    gt_btn_set_text(btn, "Show Toast");
    gt_obj_add_event_cb(btn, _btn_toast_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
