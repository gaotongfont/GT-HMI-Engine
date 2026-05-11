/**
 * @file gt_example_theme.c
 * @author Feyoung
 * @brief Dark mode / theme switch demonstration.
 *        Shows btn, label, listview, checkbox, switch, progress_bar, slider,
 *        and a manual-colored rect side by side.
 *        The rect keeps its color after theme switch (use_theme=0).
 * @version 0.2
 * @date 2026-05-11
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/core/gt_theme.h"
#include "../../src/widgets/gt_label.h"
#include "../../src/widgets/gt_rect.h"
#include "../../src/widgets/gt_listview.h"
#include "../../src/widgets/gt_checkbox.h"
#include "../../src/widgets/gt_switch.h"
#include "../../src/widgets/gt_progress_bar.h"
#include "../../src/widgets/gt_slider.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private define -------------------------------------------------------*/
#define _COL_L_X    10
#define _COL_R_X    410
#define _COL_W      370
#define _TOP_Y      8
#define _ROW1_Y     55

/* static variables -----------------------------------------------------*/
static gt_obj_st * _scr        = NULL;
static gt_obj_st * _hint_label = NULL;

/* static functions -----------------------------------------------------*/

static void _update_hint(void) {
    if (!_hint_label) { return; }
    gt_label_set_text(_hint_label,
        gt_theme_get_id() == GT_THEME_DARK ? "Mode: Dark" : "Mode: Light");
}

static void _switch_theme_cb(gt_event_st * e) {
    gt_theme_id_et next = (gt_theme_get_id() == GT_THEME_DARK)
                          ? GT_THEME_LIGHT : GT_THEME_DARK;
    gt_theme_switch(next);
    _update_hint();
}

/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_examples_theme(void)
{
    _scr = gt_obj_create(NULL);

    /* ---- top bar ---- */
    gt_obj_st * title = gt_label_create(_scr);
    gt_obj_set_pos(title, _COL_L_X, _TOP_Y);
    gt_obj_set_size(title, 220, 26);
    gt_label_set_text(title, "Theme / Dark Mode");

    _hint_label = gt_label_create(_scr);
    gt_obj_set_pos(_hint_label, 240, _TOP_Y);
    gt_obj_set_size(_hint_label, 140, 26);
    _update_hint();

    gt_obj_st * btn_toggle = gt_btn_create(_scr);
    gt_obj_set_pos(btn_toggle, 390, _TOP_Y - 2);
    gt_obj_set_size(btn_toggle, 150, 30);
    gt_btn_set_text(btn_toggle, "Toggle Theme");
    gt_obj_add_event_cb(btn_toggle, _switch_theme_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    /* ---- divider line ---- */
    gt_obj_st * div = gt_rect_create(_scr);
    gt_obj_set_pos(div, 0, 44);
    gt_obj_set_size(div, GT_SCREEN_WIDTH, 2);
    gt_rect_set_bg_color(div, gt_color_hex(0xCCCCCC));
    gt_rect_set_fill(div, 1);
    div->use_theme = 0;

    /* ================================================================
     * LEFT COLUMN
     * ================================================================ */

    /* section label */
    gt_obj_st * lbl_lv = gt_label_create(_scr);
    gt_obj_set_pos(lbl_lv, _COL_L_X, _ROW1_Y);
    gt_obj_set_size(lbl_lv, _COL_W, 20);
    gt_label_set_text(lbl_lv, "ListView");

    /* listview — 5 items, height 160 */
    gt_obj_st * lv = gt_listview_create(_scr);
    gt_obj_set_pos(lv, _COL_L_X, _ROW1_Y + 22);
    gt_obj_set_size(lv, _COL_W, 160);
    gt_listview_set_item_height(lv, 32);
    const char * lv_items[] = {
        "Item Alpha", "Item Beta", "Item Gamma",
        "Item Delta", "Item Epsilon",
    };
    uint8_t i;
    for (i = 0; i < sizeof(lv_items) / sizeof(lv_items[0]); ++i) {
        gt_listview_add_item(lv, lv_items[i]);
    }

    /* section label */
    gt_obj_st * lbl_cb = gt_label_create(_scr);
    gt_obj_set_pos(lbl_cb, _COL_L_X, _ROW1_Y + 192);
    gt_obj_set_size(lbl_cb, _COL_W, 20);
    gt_label_set_text(lbl_cb, "Checkbox");

    /* checkbox × 2 */
    gt_obj_st * cb1 = gt_checkbox_create(_scr);
    gt_obj_set_pos(cb1, _COL_L_X, _ROW1_Y + 214);
    gt_obj_set_size(cb1, 170, 30);
    gt_checkbox_set_text(cb1, "Option A");

    gt_obj_st * cb2 = gt_checkbox_create(_scr);
    gt_obj_set_pos(cb2, _COL_L_X + 180, _ROW1_Y + 214);
    gt_obj_set_size(cb2, 170, 30);
    gt_checkbox_set_text(cb2, "Option B");
    gt_obj_set_state(cb2, GT_STATE_PRESSED);

    /* section label */
    gt_obj_st * lbl_sw = gt_label_create(_scr);
    gt_obj_set_pos(lbl_sw, _COL_L_X, _ROW1_Y + 256);
    gt_obj_set_size(lbl_sw, _COL_W, 20);
    gt_label_set_text(lbl_sw, "Switch");

    /* switch × 2 */
    gt_obj_st * sw1 = gt_switch_create(_scr);
    gt_obj_set_pos(sw1, _COL_L_X, _ROW1_Y + 278);
    gt_obj_set_size(sw1, 70, 30);

    gt_obj_st * sw2 = gt_switch_create(_scr);
    gt_obj_set_pos(sw2, _COL_L_X + 90, _ROW1_Y + 278);
    gt_obj_set_size(sw2, 70, 30);
    gt_obj_set_state(sw2, GT_STATE_PRESSED);

    /* ================================================================
     * RIGHT COLUMN
     * ================================================================ */

    /* section label */
    gt_obj_st * lbl_pb = gt_label_create(_scr);
    gt_obj_set_pos(lbl_pb, _COL_R_X, _ROW1_Y);
    gt_obj_set_size(lbl_pb, _COL_W, 20);
    gt_label_set_text(lbl_pb, "Progress Bar");

    /* progress bar 1 — 60% */
    gt_obj_st * pb1 = gt_progress_bar_create(_scr);
    gt_obj_set_pos(pb1, _COL_R_X, _ROW1_Y + 22);
    gt_obj_set_size(pb1, _COL_W, 24);
    gt_progress_bar_set_start_end(pb1, 0, 100);
    gt_progress_bar_set_pos(pb1, 60);

    /* progress bar 2 — 30% */
    gt_obj_st * pb2 = gt_progress_bar_create(_scr);
    gt_obj_set_pos(pb2, _COL_R_X, _ROW1_Y + 56);
    gt_obj_set_size(pb2, _COL_W, 24);
    gt_progress_bar_set_start_end(pb2, 0, 100);
    gt_progress_bar_set_pos(pb2, 30);

    /* section label */
    gt_obj_st * lbl_sl = gt_label_create(_scr);
    gt_obj_set_pos(lbl_sl, _COL_R_X, _ROW1_Y + 96);
    gt_obj_set_size(lbl_sl, _COL_W, 20);
    gt_label_set_text(lbl_sl, "Slider");

    /* slider */
    gt_obj_st * sl = gt_slider_create(_scr);
    gt_obj_set_pos(sl, _COL_R_X, _ROW1_Y + 118);
    gt_obj_set_size(sl, _COL_W, 30);
    gt_slider_set_start_end(sl, 0, 100);
    gt_slider_set_pos(sl, 65);
    gt_slider_set_step(sl, 5);
    gt_slider_set_tag_inside(sl, true);

    /* section label */
    gt_obj_st * lbl_btn = gt_label_create(_scr);
    gt_obj_set_pos(lbl_btn, _COL_R_X, _ROW1_Y + 164);
    gt_obj_set_size(lbl_btn, _COL_W, 20);
    gt_label_set_text(lbl_btn, "Button (theme) vs Fixed color");

    /* theme button */
    gt_obj_st * btn_theme = gt_btn_create(_scr);
    gt_obj_set_pos(btn_theme, _COL_R_X, _ROW1_Y + 186);
    gt_obj_set_size(btn_theme, 160, 34);
    gt_btn_set_text(btn_theme, "Theme Btn");

    /* fixed-color rect — use_theme=0, never changes */
    gt_obj_st * fixed_rect = gt_rect_create(_scr);
    gt_obj_set_pos(fixed_rect, _COL_R_X + 180, _ROW1_Y + 186);
    gt_obj_set_size(fixed_rect, 160, 34);
    gt_rect_set_bg_color(fixed_rect, gt_color_hex(0xFF6600));
    gt_rect_set_fill(fixed_rect, 1);
    gt_rect_set_radius(fixed_rect, 6);

    gt_obj_st * fixed_lbl = gt_label_create(_scr);
    gt_obj_set_pos(fixed_lbl, _COL_R_X + 180, _ROW1_Y + 224);
    gt_obj_set_size(fixed_lbl, 160, 20);
    gt_label_set_text(fixed_lbl, "Fixed #FF6600");

    /* section label */
    gt_obj_st * lbl_multi = gt_label_create(_scr);
    gt_obj_set_pos(lbl_multi, _COL_R_X, _ROW1_Y + 256);
    gt_obj_set_size(lbl_multi, _COL_W, 20);
    gt_label_set_text(lbl_multi, "Multi-line label (theme text)");

    gt_obj_st * multi = gt_label_create(_scr);
    gt_obj_set_pos(multi, _COL_R_X, _ROW1_Y + 278);
    gt_obj_set_size(multi, _COL_W, 60);
    gt_label_set_text(multi,
        "Primary text follows theme.\n"
        "Switch theme to see all widgets update.");

    GT_EXAMPLE_ADD_BACK_BTN(_scr);

    /* apply current theme immediately so colors match on first display */
    gt_theme_apply_no_redraw(_scr);

    return _scr;
}

#endif
/* end ------------------------------------------------------------------*/
