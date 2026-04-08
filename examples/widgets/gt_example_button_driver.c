/**
 * @file gt_example_button_driver.c
 * @author Feyoung
 * @brief Physical button driver test (GT_INDEV_TYPE_BUTTON)
 * @version 0.1
 * @date 2026-04-07
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/core/gt_event.h"
#include "../../src/widgets/gt_btn.h"
#include "../../src/widgets/gt_label.h"
#include "../../src/widgets/gt_rect.h"
#include "../../src/core/gt_indev.h"
#include "../../src/others/gt_log.h"
#include <stdio.h>
#include <string.h>

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES && USE_SDL_BUTTON

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

static gt_obj_st * s_btn_test_scr = NULL;
static gt_obj_st * s_btn_test_labels[4] = {NULL};

#define _BTN_COLOR(idx) (idx == 0 ? gt_color_hex(0xE74C3C) : \
                         idx == 1 ? gt_color_hex(0x27AE60) : \
                         idx == 2 ? gt_color_hex(0x2980B9) : \
                                    gt_color_hex(0xE67E22))

#define _BTN_TEXT_COLOR(idx) gt_color_hex(0xFFFFFF)

/* static functions -----------------------------------------------------*/

static void _btn_test_set_label(uint16_t btn_id, char const * text)
{
    if (btn_id < 4 && s_btn_test_labels[btn_id]) {
        gt_btn_set_text(s_btn_test_labels[btn_id], text);
    }
}

static uint16_t _get_btn_id_from_event(gt_event_st * e)
{
    if (!e || !e->param) {
        return 0xFFFF;  /* invalid ID */
    }
    gt_indev_param_st * p = (gt_indev_param_st *)e->param;
    return p->param.button_id;
}

static void _on_btn_pressed(gt_event_st * e)
{
    if (!e) {
        GT_LOGE(GT_LOG_TAG_GUI, "_on_btn_pressed: e is NULL!");
        return;
    }
    GT_LOGI(GT_LOG_TAG_GUI, "_on_btn_pressed: e=%p, param=%p", e, e->param);
    uint16_t id = _get_btn_id_from_event(e);
    GT_LOGI(GT_LOG_TAG_GUI, "  btn_id=%u", id);
    if (id >= 4) {
        GT_LOGI(GT_LOG_TAG_GUI, "  invalid id, return");
        return;
    }
    char buf[32];
    snprintf(buf, sizeof(buf), "BTN_%u: PRESSED", id);
    _btn_test_set_label(id, buf);
}

static void _on_btn_long_pressed(gt_event_st * e)
{
    uint16_t id = _get_btn_id_from_event(e);
    if (id >= 4) {
        return;
    }
    char buf[32];
    snprintf(buf, sizeof(buf), "BTN_%u: LONG_PR", id);
    _btn_test_set_label(id, buf);
}

static void _on_btn_pressing(gt_event_st * e)
{
    uint16_t id = _get_btn_id_from_event(e);
    if (id >= 4) {
        return;
    }
    char buf[32];
    snprintf(buf, sizeof(buf), "BTN_%u: PRESSING", id);
    _btn_test_set_label(id, buf);
}

static void _on_btn_released(gt_event_st * e)
{
    uint16_t id = _get_btn_id_from_event(e);
    if (id >= 4) {
        return;
    }
    char buf[32];
    snprintf(buf, sizeof(buf), "BTN_%u: RELEASED", id);
    _btn_test_set_label(id, buf);
    /* Reset to READY after a short delay — simplified by just setting now */
    snprintf(buf, sizeof(buf), "BTN_%u: READY", id);
    if (s_btn_test_labels[id]) {
        gt_btn_set_text(s_btn_test_labels[id], buf);
    }
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Physical button driver test screen
 *
 * Tests GT_INDEV_TYPE_BUTTON via SDL key mapping:
 *   KEY_1 → button id 0
 *   KEY_2 → button id 1
 *   KEY_3 → button id 2
 *   KEY_4 → button id 3
 *
 * Each button area shows the last received event:
 *   "BTN_X: READY"    (default)
 *   "BTN_X: PRESSED"  (on GT_EVENT_TYPE_INPUT_PRESSED)
 *   "BTN_X: LONG_PR"  (on GT_EVENT_TYPE_INPUT_LONG_PRESSED)
 *   "BTN_X: PRESSING" (on GT_EVENT_TYPE_INPUT_PRESSING, repeated)
 *   "BTN_X: RELEASED" (on GT_EVENT_TYPE_INPUT_RELEASED)
 */
gt_obj_st * gt_examples_button_driver(void)
{
    s_btn_test_scr = gt_obj_create(NULL);
    gt_screen_set_bgcolor(s_btn_test_scr, gt_color_hex(0x1A1A2E));

    /* Title */
    gt_obj_st * title = gt_label_create(s_btn_test_scr);
    gt_obj_set_pos(title, 250, 10);
    gt_obj_set_size(title, 300, 40);
    gt_label_set_font_color(title, gt_color_hex(0xFFFFFF));
    gt_label_set_font_size(title, 28);
    gt_label_set_text(title, "BTN Driver Test [KEY: 1-4]");

    /* 4 button areas in a row */
    uint16_t btn_w = 160, btn_h = 120, btn_gap = 24;
    uint16_t total_w = 4 * btn_w + 3 * btn_gap;
    uint16_t start_x = (GT_SCREEN_WIDTH - total_w) / 2;
    uint16_t start_y = 70;

    for (int i = 0; i < 4; i++) {
        gt_obj_st * btn = gt_btn_create(s_btn_test_scr);
        gt_obj_set_pos(btn, start_x + i * (btn_w + btn_gap), start_y);
        gt_obj_set_size(btn, btn_w, btn_h);
        gt_btn_set_color_background(btn, _BTN_COLOR(i));
        gt_btn_set_font_color(btn, _BTN_TEXT_COLOR(i));
        gt_btn_set_font_size(btn, 22);
        gt_btn_set_text(btn, "BTN_: READY");

        /* Map physical button id: KEY_1->0, KEY_2->1, KEY_3->2, KEY_4->3 */
        gt_btn_set_button_id(btn, (uint8_t)i);

        /* Enable focus for button driver test (GT_INDEV_TYPE_BUTTON) */
        gt_obj_set_focus_disabled(btn, GT_ENABLED);
        gt_obj_set_focus(btn, 1);

        gt_obj_add_event_cb(btn, _on_btn_pressed,     GT_EVENT_TYPE_INPUT_PRESSED,      NULL);
        gt_obj_add_event_cb(btn, _on_btn_long_pressed, GT_EVENT_TYPE_INPUT_LONG_PRESSED, NULL);
        gt_obj_add_event_cb(btn, _on_btn_pressing,   GT_EVENT_TYPE_INPUT_PRESSING,     NULL);
        gt_obj_add_event_cb(btn, _on_btn_released,   GT_EVENT_TYPE_INPUT_RELEASED,     NULL);

        s_btn_test_labels[i] = btn;
    }

    /* Info label */
    gt_obj_st * info = gt_label_create(s_btn_test_scr);
    gt_obj_set_pos(info, 180, 220);
    gt_obj_set_size(info, 440, 30);
    gt_label_set_font_color(info, gt_color_hex(0xAAAAAA));
    gt_label_set_font_size(info, 18);
    gt_label_set_text(info, "Hold KEY 1-4 > 500ms triggers LONG_PRESSED");

    /* Separator (thin rect) */
    gt_obj_st * sep = gt_rect_create(s_btn_test_scr);
    gt_obj_set_pos(sep, 0, 260);
    gt_obj_set_size(sep, GT_SCREEN_WIDTH, 1);
    gt_rect_set_bg_color(sep, gt_color_hex(0x333333));

    /* Keypad test info */
    gt_obj_st * kp_info = gt_label_create(s_btn_test_scr);
    gt_obj_set_pos(kp_info, 180, 275);
    gt_obj_set_size(kp_info, 440, 30);
    gt_label_set_font_color(kp_info, gt_color_hex(0xAAAAAA));
    gt_label_set_font_size(kp_info, 18);
    gt_label_set_text(kp_info, "KEYPAD: Arrow/Focus keys + ENTER/ESC");

    /* Back button */
    gt_obj_st * back_btn = gt_btn_create(s_btn_test_scr);
    gt_obj_set_pos(back_btn, 320, 320);
    gt_obj_set_size(back_btn, 160, 50);
    gt_btn_set_text(back_btn, "< Back to Menu");
    gt_btn_set_color_background(back_btn, gt_color_hex(0x555555));
    gt_btn_set_font_color(back_btn, gt_color_hex(0xFFFFFF));
    gt_obj_add_event_cb(back_btn, _gt_example_back_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    /* Also add the standard example back button in top-right corner */
    GT_EXAMPLE_ADD_BACK_BTN(s_btn_test_scr);

    return s_btn_test_scr;
}

#else
/* Stub when button driver or examples are disabled */
gt_obj_st * gt_examples_button_driver(void) { return NULL; }
#endif
/* end ------------------------------------------------------------------*/