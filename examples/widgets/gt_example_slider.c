/**
 * @file gt_example_slider.c
 * @author Feyoung
 * @brief Slider widget example — demonstrates standard, inside-tag, vertical,
 *        and custom-color variants.
 * @version 0.2
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

/* static variables -----------------------------------------------------*/
static gt_obj_st * _lbl_val1 = NULL;
static gt_obj_st * _lbl_val2 = NULL;
static gt_obj_st * _lbl_val3 = NULL;

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

#endif
/* end ------------------------------------------------------------------*/
