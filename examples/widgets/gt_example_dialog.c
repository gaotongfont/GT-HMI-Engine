/**
 * @file gt_example_dialog.c
 * @author Feyoung
 * @brief Dialog widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_dialog.h"
#include "../../src/widgets/gt_btn.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/
static gt_obj_st * _dialog = NULL;

static void _dialog_confirm_cb(gt_event_st * e)
{
    printf("dialog confirmed\n");
}

static void _btn_show_dialog_cb(gt_event_st * e)
{
    gt_dialog_param_st param = {
        .title = "Notice",
        .content = "This is a dialog example.",
        .confirm_cb = _dialog_confirm_cb,
        .confirm_cb_user_data = NULL,
        .cancel_cb = NULL,
        .cancel_cb_user_data = NULL,
    };
    _dialog = gt_dialog_create_issue(true, &param);
    gt_dialog_show(_dialog);
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a button that triggers a dialog.
 */
gt_obj_st * gt_examples_dialog(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    gt_obj_st * btn = gt_btn_create(screen);
    gt_obj_set_pos(btn, 100, 100);
    gt_obj_set_size(btn, 120, 40);
    gt_btn_set_text(btn, "Show Dialog");
    gt_obj_add_event_cb(btn, _btn_show_dialog_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
