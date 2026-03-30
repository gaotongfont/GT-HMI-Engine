/**
 * @file gt_example_form.c
 * @author Feyoung
 * @brief Form combination example (input + checkbox + radio + button)
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_input.h"
#include "../../src/widgets/gt_checkbox.h"
#include "../../src/widgets/gt_radio.h"
#include "../../src/widgets/gt_label.h"
#include "../../src/widgets/gt_btn.h"
#include "../../src/widgets/gt_group.h"
#include "string.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/
static gt_obj_st * _input_name = NULL;
static gt_obj_st * _cb_agree = NULL;
static gt_obj_st * _result_label = NULL;

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/

static void _submit_cb(gt_event_st * e)
{
    if (!_input_name || !_cb_agree || !_result_label) return;

    char * name = gt_input_get_value(_input_name);
    gt_state_et agree = gt_obj_get_state(_cb_agree);

    if (!name || strlen(name) == 0) {
        gt_label_set_text(_result_label, "Please enter name");
        return;
    }
    if (GT_STATE_PRESSED != agree) {
        gt_label_set_text(_result_label, "Please agree terms");
        return;
    }
    gt_label_set_text(_result_label, "Submitted: %s", name);
}

static void _reset_cb(gt_event_st * e)
{
    if (_input_name) gt_input_set_value(_input_name, "");
    if (_result_label) gt_label_set_text(_result_label, "");
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Form demo: input + checkbox + submit/reset buttons.
 */
gt_obj_st * gt_examples_form(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);
    gt_screen_set_bgcolor(screen, gt_color_hex(0xFFFFFF));

    gt_obj_st * lbl_name = gt_label_create(screen);
    gt_label_set_text(lbl_name, "Name:");
    gt_obj_set_pos(lbl_name, 10, 45);
    gt_obj_set_size(lbl_name, 60, 30);

    _input_name = gt_input_create(screen);
    gt_obj_set_pos(_input_name, 75, 40);
    gt_obj_set_size(_input_name, 200, 36);
    gt_input_set_placeholder(_input_name, "Enter your name");

    _cb_agree = gt_checkbox_create(screen);
    gt_checkbox_set_text(_cb_agree, "I agree to the terms");
    gt_obj_set_pos(_cb_agree, 10, 90);
    gt_obj_set_size(_cb_agree, 250, 30);

    gt_obj_st * btn_submit = gt_btn_create(screen);
    gt_obj_set_pos(btn_submit, 30, 140);
    gt_obj_set_size(btn_submit, 100, 36);
    gt_btn_set_text(btn_submit, "Submit");
    gt_obj_add_event_cb(btn_submit, _submit_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * btn_reset = gt_btn_create(screen);
    gt_obj_set_pos(btn_reset, 150, 140);
    gt_obj_set_size(btn_reset, 100, 36);
    gt_btn_set_text(btn_reset, "Reset");
    gt_obj_add_event_cb(btn_reset, _reset_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    _result_label = gt_label_create(screen);
    gt_obj_set_pos(_result_label, 10, 195);
    gt_obj_set_size(_result_label, 300, 30);
    gt_label_set_font_color(_result_label, gt_color_hex(0x2196F3));

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
