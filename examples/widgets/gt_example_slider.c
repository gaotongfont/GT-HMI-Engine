/**
 * @file gt_example_slider.c
 * @author Feyoung
 * @brief Slider widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_slider.h"
#include "../../src/widgets/gt_label.h"
#include "string.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/
static gt_obj_st * _label_val = NULL;

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/
static void _slider_event_cb(gt_event_st * e)
{
    gt_size_t pos = gt_slider_get_pos(e->target);
    char buf[32];
    memset(buf, 0, sizeof(buf));
    sprintf(buf, "Value: %d", pos);
    if (_label_val) {
        gt_label_set_text(_label_val, "%s", buf);
    }
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create slider with value display.
 */
gt_obj_st * gt_examples_slider(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    _label_val = gt_label_create(screen);
    gt_label_set_text(_label_val, "Value: 0");
    gt_obj_set_pos(_label_val, 10, 10);
    gt_obj_set_size(_label_val, 200, 30);

    gt_obj_st * slider = gt_slider_create(screen);
    gt_obj_set_pos(slider, 10, 60);
    gt_obj_set_size(slider, 300, 30);
    gt_slider_set_start_end(slider, 0, 100);
    gt_slider_set_pos(slider, 50);
    gt_slider_set_step(slider, 5);
    gt_slider_set_color_act(slider, gt_color_hex(0x2196F3));
    gt_slider_set_color_ina(slider, gt_color_hex(0xBDBDBD));
    gt_obj_add_event_cb(slider, _slider_event_cb, GT_EVENT_TYPE_UPDATE_VALUE, NULL);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
