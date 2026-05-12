/**
 * @file gt_example_slider.c
 * @author Feyoung
 * @brief Slider widget example — demonstrates standard, inside-tag, vertical,
 *        custom-color variants, and AB repeat mode.
 * @version 0.3
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_slider.h"
#include "../../src/widgets/gt_label.h"
#include "../../src/widgets/gt_btn.h"
#include "string.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* static variables -----------------------------------------------------*/
static gt_obj_st * _lbl_val1 = NULL;
static gt_obj_st * _lbl_val2 = NULL;
static gt_obj_st * _lbl_val3 = NULL;

/* AB repeat page static refs */
static gt_obj_st * _ab_slider   = NULL;
static gt_obj_st * _ab_lbl_pos  = NULL;
static gt_obj_st * _ab_lbl_info = NULL;

/* static functions -----------------------------------------------------*/
static void _cb1(gt_event_st * e) {
    char buf[24];
    sprintf(buf, "Val: %d", (int)gt_slider_get_pos(e->target));
    gt_label_set_text(_lbl_val1, buf);
}

static void _cb2(gt_event_st * e) {
    char buf[24];
    sprintf(buf, "Val: %d", (int)gt_slider_get_pos(e->target));
    gt_label_set_text(_lbl_val2, buf);
}

static void _cb3(gt_event_st * e) {
    char buf[24];
    sprintf(buf, "Val: %d", (int)gt_slider_get_pos(e->target));
    gt_label_set_text(_lbl_val3, buf);
}

/* AB repeat page callbacks */
static void _ab_slider_cb(gt_event_st * e) {
    char buf[32];
    sprintf(buf, "Pos: %d", (int)gt_slider_get_pos(e->target));
    gt_label_set_text(_ab_lbl_pos, buf);
}

static void _ab_btn_set_cb(gt_event_st * e) {
    gt_slider_set_AB_repeat_start_by(_ab_slider, 20);
    gt_slider_set_AB_repeat_stop_by(_ab_slider, 70);
    gt_label_set_text(_ab_lbl_info, "AB: 20 ~ 70");
}

static void _ab_btn_remark_cb(gt_event_st * e) {
    gt_slider_set_AB_repeat_remark_point(_ab_slider);
    gt_label_set_text(_ab_lbl_info, "Remark: tap 3x (A->B->off)");
}

static void _ab_btn_unset_cb(gt_event_st * e) {
    gt_slider_unset_AB_repeat(_ab_slider);
    gt_label_set_text(_ab_lbl_info, "AB: off");
}

/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_examples_slider(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    /* ---- title ---- */
    gt_obj_st * title = gt_label_create(screen);
    gt_obj_set_pos(title, 10, 8);
    gt_obj_set_size(title, 400, 26);
    gt_label_set_text(title, "Slider Examples");

    /* ================================================================
     * Row 1 — Standard (tag protrudes)
     * ================================================================ */
    gt_obj_st * lbl1 = gt_label_create(screen);
    gt_obj_set_pos(lbl1, 10, 46);
    gt_obj_set_size(lbl1, 250, 22);
    gt_label_set_text(lbl1, "Standard (tag protrudes)");

    _lbl_val1 = gt_label_create(screen);
    gt_obj_set_pos(_lbl_val1, 250, 46);
    gt_obj_set_size(_lbl_val1, 100, 22);
    gt_label_set_text(_lbl_val1, "Val: 40");

    gt_obj_st * sl1 = gt_slider_create(screen);
    gt_obj_set_pos(sl1, 10, 72);
    gt_obj_set_size(sl1, 460, 36);
    gt_slider_set_start_end(sl1, 0, 100);
    gt_slider_set_pos(sl1, 40);
    gt_slider_set_step(sl1, 5);
    gt_obj_add_event_cb(sl1, _cb1, GT_EVENT_TYPE_UPDATE_VALUE, NULL);

    /* ================================================================
     * Row 2 — Inside tag (switch-like)
     * ================================================================ */
    gt_obj_st * lbl2 = gt_label_create(screen);
    gt_obj_set_pos(lbl2, 10, 126);
    gt_obj_set_size(lbl2, 200, 22);
    gt_label_set_text(lbl2, "Inside tag (switch-like)");

    _lbl_val2 = gt_label_create(screen);
    gt_obj_set_pos(_lbl_val2, 220, 126);
    gt_obj_set_size(_lbl_val2, 100, 22);
    gt_label_set_text(_lbl_val2, "Val: 65");

    gt_obj_st * sl2 = gt_slider_create(screen);
    gt_obj_set_pos(sl2, 10, 152);
    gt_obj_set_size(sl2, 460, 36);
    gt_slider_set_start_end(sl2, 0, 100);
    gt_slider_set_pos(sl2, 65);
    gt_slider_set_step(sl2, 5);
    gt_slider_set_tag_inside(sl2, true);
    gt_obj_add_event_cb(sl2, _cb2, GT_EVENT_TYPE_UPDATE_VALUE, NULL);

    /* ================================================================
     * Row 3 — Custom colors + inside tag
     * ================================================================ */
    gt_obj_st * lbl3 = gt_label_create(screen);
    gt_obj_set_pos(lbl3, 10, 206);
    gt_obj_set_size(lbl3, 260, 22);
    gt_label_set_text(lbl3, "Custom colors (inside tag)");

    _lbl_val3 = gt_label_create(screen);
    gt_obj_set_pos(_lbl_val3, 280, 206);
    gt_obj_set_size(_lbl_val3, 100, 22);
    gt_label_set_text(_lbl_val3, "Val: 30");

    gt_obj_st * sl3 = gt_slider_create(screen);
    gt_obj_set_pos(sl3, 10, 232);
    gt_obj_set_size(sl3, 460, 36);
    gt_slider_set_start_end(sl3, 0, 100);
    gt_slider_set_pos(sl3, 30);
    gt_slider_set_step(sl3, 1);
    gt_slider_set_tag_inside(sl3, true);
    gt_slider_set_color_act(sl3, gt_color_hex(0xFF5722));
    gt_slider_set_color_ina(sl3, gt_color_hex(0x607D8B));
    gt_slider_set_color_tag(sl3, gt_color_hex(0xFFFFFF));
    gt_obj_add_event_cb(sl3, _cb3, GT_EVENT_TYPE_UPDATE_VALUE, NULL);

    /* ================================================================
     * Vertical sliders (right side)
     * ================================================================ */
    gt_obj_st * lbl_v = gt_label_create(screen);
    gt_obj_set_pos(lbl_v, 520, 46);
    gt_obj_set_size(lbl_v, 260, 22);
    gt_label_set_text(lbl_v, "Vertical");

    /* vertical standard */
    gt_obj_st * sv1 = gt_slider_create(screen);
    gt_obj_set_pos(sv1, 520, 72);
    gt_obj_set_size(sv1, 36, 340);
    gt_slider_set_start_end(sv1, 0, 100);
    gt_slider_set_pos(sv1, 70);
    gt_slider_set_step(sv1, 5);
    gt_slider_set_dir(sv1, GT_BAR_DIR_VER_D2U);

    /* vertical inside tag */
    gt_obj_st * sv2 = gt_slider_create(screen);
    gt_obj_set_pos(sv2, 600, 72);
    gt_obj_set_size(sv2, 36, 340);
    gt_slider_set_start_end(sv2, 0, 100);
    gt_slider_set_pos(sv2, 45);
    gt_slider_set_step(sv2, 5);
    gt_slider_set_dir(sv2, GT_BAR_DIR_VER_D2U);
    gt_slider_set_tag_inside(sv2, true);

    /* vertical custom color inside tag */
    gt_obj_st * sv3 = gt_slider_create(screen);
    gt_obj_set_pos(sv3, 680, 72);
    gt_obj_set_size(sv3, 36, 340);
    gt_slider_set_start_end(sv3, 0, 100);
    gt_slider_set_pos(sv3, 20);
    gt_slider_set_step(sv3, 5);
    gt_slider_set_dir(sv3, GT_BAR_DIR_VER_D2U);
    gt_slider_set_tag_inside(sv3, true);
    gt_slider_set_color_act(sv3, gt_color_hex(0x4CAF50));
    gt_slider_set_color_ina(sv3, gt_color_hex(0x37474F));
    gt_slider_set_color_tag(sv3, gt_color_hex(0xFFFFFF));

    /* vertical labels */
    gt_obj_st * lv1 = gt_label_create(screen);
    gt_obj_set_pos(lv1, 506, 420);
    gt_obj_set_size(lv1, 60, 20);
    gt_label_set_text(lv1, "Std");

    gt_obj_st * lv2 = gt_label_create(screen);
    gt_obj_set_pos(lv2, 586, 420);
    gt_obj_set_size(lv2, 60, 20);
    gt_label_set_text(lv2, "Inside");

    gt_obj_st * lv3 = gt_label_create(screen);
    gt_obj_set_pos(lv3, 666, 420);
    gt_obj_set_size(lv3, 80, 20);
    gt_label_set_text(lv3, "Custom");

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

/* ======================================================================
 * AB Repeat example screen
 * ====================================================================== */

gt_obj_st * gt_examples_slider_ab_repeat(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    /* ---- title ---- */
    gt_obj_st * title = gt_label_create(screen);
    gt_obj_set_pos(title, 10, 8);
    gt_obj_set_size(title, 500, 26);
    gt_label_set_text(title, "Slider — AB Repeat Mode");

    /* ================================================================
     * Row 1 — Preset AB via start_by / stop_by API
     *   Range 0~100, A=20, B=70, slider starts at 45 (inside AB zone)
     * ================================================================ */
    gt_obj_st * lbl_r1 = gt_label_create(screen);
    gt_obj_set_pos(lbl_r1, 10, 46);
    gt_obj_set_size(lbl_r1, 460, 22);
    gt_label_set_text(lbl_r1, "Preset AB (start_by/stop_by): A=20, B=70");

    _ab_lbl_pos = gt_label_create(screen);
    gt_obj_set_pos(_ab_lbl_pos, 10, 72);
    gt_obj_set_size(_ab_lbl_pos, 120, 22);
    gt_label_set_text(_ab_lbl_pos, "Pos: 45");

    _ab_lbl_info = gt_label_create(screen);
    gt_obj_set_pos(_ab_lbl_info, 140, 72);
    gt_obj_set_size(_ab_lbl_info, 320, 22);
    gt_label_set_text(_ab_lbl_info, "AB: 20 ~ 70");

    _ab_slider = gt_slider_create(screen);
    gt_obj_set_pos(_ab_slider, 10, 98);
    gt_obj_set_size(_ab_slider, 460, 36);
    gt_slider_set_start_end(_ab_slider, 0, 100);
    gt_slider_set_pos(_ab_slider, 45);
    gt_slider_set_step(_ab_slider, 1);
    /* highlight AB zone in orange */
    gt_slider_set_AB_repeat_color(_ab_slider, gt_color_hex(0xFF9800));
    gt_slider_set_AB_repeat_start_by(_ab_slider, 20);
    gt_slider_set_AB_repeat_stop_by(_ab_slider, 70);
    gt_obj_add_event_cb(_ab_slider, _ab_slider_cb, GT_EVENT_TYPE_UPDATE_VALUE, NULL);

    /* ================================================================
     * Control buttons: Set AB / Remark / Unset
     * ================================================================ */
    gt_obj_st * btn_set = gt_btn_create(screen);
    gt_obj_set_pos(btn_set, 10, 150);
    gt_obj_set_size(btn_set, 130, 34);
    gt_btn_set_text(btn_set, "Set AB(20~70)");
    gt_obj_add_event_cb(btn_set, _ab_btn_set_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * btn_remark = gt_btn_create(screen);
    gt_obj_set_pos(btn_remark, 155, 150);
    gt_obj_set_size(btn_remark, 150, 34);
    gt_btn_set_text(btn_remark, "Remark (A->B->off)");
    gt_obj_add_event_cb(btn_remark, _ab_btn_remark_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * btn_unset = gt_btn_create(screen);
    gt_obj_set_pos(btn_unset, 320, 150);
    gt_obj_set_size(btn_unset, 100, 34);
    gt_btn_set_text(btn_unset, "Unset AB");
    gt_obj_add_event_cb(btn_unset, _ab_btn_unset_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    /* ================================================================
     * Row 2 — Custom color AB, inside tag, A=30 B=80
     * ================================================================ */
    gt_obj_st * lbl_r2 = gt_label_create(screen);
    gt_obj_set_pos(lbl_r2, 10, 202);
    gt_obj_set_size(lbl_r2, 460, 22);
    gt_label_set_text(lbl_r2, "Custom color AB (inside tag): A=30, B=80, green zone");

    gt_obj_st * sl2 = gt_slider_create(screen);
    gt_obj_set_pos(sl2, 10, 228);
    gt_obj_set_size(sl2, 460, 36);
    gt_slider_set_start_end(sl2, 0, 100);
    gt_slider_set_pos(sl2, 55);
    gt_slider_set_step(sl2, 1);
    gt_slider_set_tag_inside(sl2, true);
    gt_slider_set_color_act(sl2, gt_color_hex(0x2196F3));
    gt_slider_set_color_ina(sl2, gt_color_hex(0x607D8B));
    gt_slider_set_color_tag(sl2, gt_color_hex(0xFFFFFF));
    gt_slider_set_AB_repeat_color(sl2, gt_color_hex(0x4CAF50));
    gt_slider_set_AB_repeat_start_by(sl2, 30);
    gt_slider_set_AB_repeat_stop_by(sl2, 80);

    /* ================================================================
     * Row 3 — Vertical AB repeat, A=25 B=75
     * ================================================================ */
    gt_obj_st * lbl_r3 = gt_label_create(screen);
    gt_obj_set_pos(lbl_r3, 530, 46);
    gt_obj_set_size(lbl_r3, 240, 22);
    gt_label_set_text(lbl_r3, "Vertical AB: A=25, B=75");

    gt_obj_st * sv = gt_slider_create(screen);
    gt_obj_set_pos(sv, 560, 72);
    gt_obj_set_size(sv, 36, 340);
    gt_slider_set_start_end(sv, 0, 100);
    gt_slider_set_pos(sv, 50);
    gt_slider_set_step(sv, 1);
    gt_slider_set_dir(sv, GT_BAR_DIR_VER_D2U);
    gt_slider_set_AB_repeat_color(sv, gt_color_hex(0xE91E63));
    gt_slider_set_AB_repeat_start_by(sv, 25);
    gt_slider_set_AB_repeat_stop_by(sv, 75);

    /* ================================================================
     * Row 4 — Edge cases: A=B boundary (stop ignored), then corrected
     *   A=50, stop_by(50) → ignored (stop must > begin)
     *   A=50, stop_by(51) → valid minimal interval
     * ================================================================ */
    gt_obj_st * lbl_r4 = gt_label_create(screen);
    gt_obj_set_pos(lbl_r4, 10, 282);
    gt_obj_set_size(lbl_r4, 460, 22);
    gt_label_set_text(lbl_r4, "Edge: A=50, stop(50) ignored, stop(51) valid");

    gt_obj_st * sl4 = gt_slider_create(screen);
    gt_obj_set_pos(sl4, 10, 308);
    gt_obj_set_size(sl4, 460, 36);
    gt_slider_set_start_end(sl4, 0, 100);
    gt_slider_set_pos(sl4, 50);
    gt_slider_set_step(sl4, 1);
    gt_slider_set_AB_repeat_color(sl4, gt_color_hex(0xFF5722));
    gt_slider_set_AB_repeat_start_by(sl4, 50);
    gt_slider_set_AB_repeat_stop_by(sl4, 50);  /* ignored: stop == begin */
    gt_slider_set_AB_repeat_stop_by(sl4, 51);  /* valid: minimal 1-unit interval */

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
