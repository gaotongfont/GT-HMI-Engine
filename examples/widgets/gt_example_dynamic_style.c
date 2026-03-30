/**
 * @file gt_example_dynamic_style.c
 * @author Feyoung
 * @brief Dynamic style change demonstration
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_rect.h"
#include "../../src/widgets/gt_label.h"
#include "../../src/widgets/gt_btn.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/
static gt_obj_st * _target_rect = NULL;
static gt_obj_st * _info_label = NULL;
static uint8_t _style_idx = 0;

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/

static const gt_color_t _colors[] = {
    { .full = 0xF800 },  /* red */
    { .full = 0x07E0 },  /* green */
    { .full = 0x001F },  /* blue */
    { .full = 0xFFE0 },  /* yellow */
};

static void _change_color_cb(gt_event_st * e)
{
    if (!_target_rect) return;
    _style_idx = (_style_idx + 1) % 4;
    gt_rect_set_bg_color(_target_rect, _colors[_style_idx]);
}

static void _change_size_cb(gt_event_st * e)
{
    if (!_target_rect) return;
    uint16_t w = _target_rect->area.w;
    if (w >= 200) {
        gt_obj_set_size(_target_rect, 80, 60);
    } else {
        gt_obj_set_size(_target_rect, w + 30, _target_rect->area.h + 20);
    }
}

static void _toggle_visible_cb(gt_event_st * e)
{
    if (!_target_rect) return;
    gt_visible_et vis = gt_obj_get_visible(_target_rect);
    gt_obj_set_visible(_target_rect, vis == GT_VISIBLE ? GT_INVISIBLE : GT_VISIBLE);
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Dynamic style demo: change color, size, visibility at runtime.
 */
gt_obj_st * gt_examples_dynamic_style(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);
    gt_screen_set_bgcolor(screen, gt_color_hex(0xFFFFFF));

    _style_idx = 0;

    _target_rect = gt_rect_create(screen);
    gt_obj_set_pos(_target_rect, 100, 50);
    gt_obj_set_size(_target_rect, 80, 60);
    gt_rect_set_bg_color(_target_rect, _colors[0]);
    gt_rect_set_fill(_target_rect, 1);
    gt_rect_set_radius(_target_rect, 8);

    gt_obj_st * btn_color = gt_btn_create(screen);
    gt_obj_set_pos(btn_color, 10, 160);
    gt_obj_set_size(btn_color, 90, 36);
    gt_btn_set_text(btn_color, "Color");
    gt_obj_add_event_cb(btn_color, _change_color_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * btn_size = gt_btn_create(screen);
    gt_obj_set_pos(btn_size, 110, 160);
    gt_obj_set_size(btn_size, 90, 36);
    gt_btn_set_text(btn_size, "Size");
    gt_obj_add_event_cb(btn_size, _change_size_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * btn_vis = gt_btn_create(screen);
    gt_obj_set_pos(btn_vis, 210, 160);
    gt_obj_set_size(btn_vis, 90, 36);
    gt_btn_set_text(btn_vis, "Visible");
    gt_obj_add_event_cb(btn_vis, _toggle_visible_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
