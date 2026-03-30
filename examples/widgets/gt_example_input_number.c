/**
 * @file gt_example_input_number.c
 * @author Feyoung
 * @brief Input number widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_input_number.h"
#include "../../src/widgets/gt_btn.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/
static gt_obj_st * _input_num = NULL;

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/
static void _btn_inc_cb(gt_event_st * e)
{
    if (_input_num) {
        gt_input_number_increase(_input_num);
    }
}

static void _btn_dec_cb(gt_event_st * e)
{
    if (_input_num) {
        gt_input_number_decrease(_input_num);
    }
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create input number with increase/decrease buttons.
 */
gt_obj_st * gt_examples_input_number(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    _input_num = gt_input_number_create(screen);
    gt_obj_set_pos(_input_num, 80, 50);
    gt_obj_set_size(_input_num, 120, 40);
    gt_input_number_set_value(_input_num, 50);
    gt_input_number_set_min(_input_num, 0);
    gt_input_number_set_max(_input_num, 100);
    gt_input_number_set_step(_input_num, 1);
    gt_input_number_set_font_size(_input_num, 20);
    gt_input_number_set_font_color(_input_num, gt_color_black());
    gt_input_number_set_display_integer_length(_input_num, 3);

    gt_obj_st * btn_dec = gt_btn_create(screen);
    gt_obj_set_pos(btn_dec, 10, 50);
    gt_obj_set_size(btn_dec, 60, 40);
    gt_btn_set_text(btn_dec, "-");
    gt_obj_add_event_cb(btn_dec, _btn_dec_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * btn_inc = gt_btn_create(screen);
    gt_obj_set_pos(btn_inc, 210, 50);
    gt_obj_set_size(btn_inc, 60, 40);
    gt_btn_set_text(btn_inc, "+");
    gt_obj_add_event_cb(btn_inc, _btn_inc_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
