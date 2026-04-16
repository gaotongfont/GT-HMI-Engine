/**
 * @file gt_example_arc.c
 * @author Feyoung
 * @brief Arc widget example — solid / gradient / rainbow / fade combinations
 * @version 0.3
 * @date 2026-04-16
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_arc.h"
#include "../../src/widgets/gt_label.h"
#include "../../src/widgets/gt_switch.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/*
 * Scrollable 2-column layout.
 * Screen: 800×480, scrollable vertically.
 * Each cell: 380×230 px, gap 20px.
 * Arc widget: 160×160, centered in cell upper portion.
 * Label below arc, with 16px gap.
 * Total rows: 9, total height: 9*250 + 20 = 2270px.
 */
#define _CELL_W     380
#define _CELL_H     230
#define _GAP        20
#define _ARC_SZ     160
#define _LBL_H      32

/* Column X origins */
#define _COL_L      10
#define _COL_R      (_COL_L + _CELL_W + _GAP)

/* Arc X offset within cell to center it */
#define _ARC_OFF_X  ((_CELL_W - _ARC_SZ) / 2)
/* Arc Y offset within cell (top of cell) */
#define _ARC_OFF_Y  16

/* Row Y: row index → top-y of cell */
#define _ROW_Y(r)   (_GAP + (r) * (_CELL_H + _GAP))

/* macros ---------------------------------------------------------------*/

/** Place arc + label in a cell at (col_x, row_y) */
#define _PLACE(parent, col_x, row_y, arc_obj, label_text) \
    do { \
        gt_obj_set_pos((arc_obj), (col_x) + _ARC_OFF_X, (row_y) + _ARC_OFF_Y); \
        gt_obj_set_size((arc_obj), _ARC_SZ, _ARC_SZ); \
        gt_obj_st * _lbl = gt_label_create(parent); \
        gt_obj_set_pos(_lbl, (col_x), (row_y) + _ARC_OFF_Y + _ARC_SZ + 16); \
        gt_obj_set_size(_lbl, _CELL_W + 16, _LBL_H); \
        gt_label_set_text(_lbl, label_text); \
        gt_label_set_font_size(_lbl, 14); \
        gt_label_set_font_align(_lbl, GT_ALIGN_CENTER_MID); \
        if (_arc_count < 18) { \
            _lbl_objects[_arc_count] = _lbl; \
        } \
    } while(0)

/* static variables -----------------------------------------------------*/
static gt_obj_st* _arc_objects[18];
static gt_obj_st* _lbl_objects[18];
static int _arc_count = 0;
static bool _bg_dark = false;
static bool _draw_bg_enabled = true;
static gt_obj_st* _info_lbl = NULL;

/* static function declarations ----------------------------------------*/
static void _bg_toggle_cb(gt_event_st * e);
static void _draw_bg_toggle_cb(gt_event_st * e);

/* static functions -----------------------------------------------------*/

/* Background toggle callback */
static void _bg_toggle_cb(gt_event_st * e)
{
    _bg_dark = !_bg_dark;
    gt_obj_st * scr = gt_obj_get_parent(e->target);
    gt_screen_set_bgcolor(scr, _bg_dark ? gt_color_black() : gt_color_white());
    gt_btn_set_text(e->target, _bg_dark ? "BG:Black" : "BG:White");

    /* Update label colors for contrast */
    gt_color_t lbl_color = _bg_dark ? gt_color_white() : gt_color_black();
    gt_label_set_font_color(_info_lbl, lbl_color);
    for (int i = 0; i < _arc_count; i++) {
        if (_lbl_objects[i]) {
            gt_label_set_font_color(_lbl_objects[i], lbl_color);
        }
    }
}

/* Draw background toggle callback */
static void _draw_bg_toggle_cb(gt_event_st * e)
{
    _draw_bg_enabled = !_draw_bg_enabled;
    for (int i = 0; i < _arc_count; i++) {
        if (_arc_objects[i]) {
            gt_arc_set_draw_bg(_arc_objects[i], _draw_bg_enabled);
        }
    }
    gt_btn_set_text(e->target, _draw_bg_enabled ? "BG:ON" : "BG:OFF");
}

/* Row 0 left: Thin ring 75% */
static gt_obj_st * _make_thin_ring(gt_obj_st * scr)
{
    gt_obj_st * arc = gt_arc_create(scr);
    gt_arc_set_bg_color(arc, gt_color_hex(0xDDDDDD));
    gt_arc_set_bg_start_end_angle(arc, 135, 45);
    gt_arc_set_bg_width(arc, 8);
    gt_arc_set_act_color(arc, gt_color_hex(0x2196F3));
    gt_arc_set_act_start_end_angle(arc, 135, 45);
    gt_arc_set_act_width(arc, 8);
    gt_arc_set_rounded(arc, true);
    gt_arc_set_value_min_max(arc, 0, 100);
    gt_arc_set_value(arc, 75);
    if (_arc_count < 18) {
        _arc_objects[_arc_count++] = arc;
    }
    return arc;
}

/* Row 0 right: Medium ring 50% */
static gt_obj_st * _make_medium_ring(gt_obj_st * scr)
{
    gt_obj_st * arc = gt_arc_create(scr);
    gt_arc_set_bg_color(arc, gt_color_hex(0xDDDDDD));
    gt_arc_set_bg_start_end_angle(arc, 135, 45);
    gt_arc_set_bg_width(arc, 16);
    gt_arc_set_act_color(arc, gt_color_hex(0xFF5722));
    gt_arc_set_act_start_end_angle(arc, 135, 45);
    gt_arc_set_act_width(arc, 16);
    gt_arc_set_rounded(arc, true);
    gt_arc_set_value_min_max(arc, 0, 100);
    gt_arc_set_value(arc, 50);
    if (_arc_count < 18) {
        _arc_objects[_arc_count++] = arc;
    }
    return arc;
}

/* Row 1 left: Thick ring 100% */
static gt_obj_st * _make_thick_ring(gt_obj_st * scr)
{
    gt_obj_st * arc = gt_arc_create(scr);
    gt_arc_set_bg_color(arc, gt_color_hex(0xEEEEEE));
    gt_arc_set_bg_start_end_angle(arc, 0, 360);
    gt_arc_set_bg_width(arc, 26);
    gt_arc_set_act_color(arc, gt_color_hex(0x4CAF50));
    gt_arc_set_act_start_end_angle(arc, 0, 360);
    gt_arc_set_act_width(arc, 26);
    gt_arc_set_value_min_max(arc, 0, 100);
    gt_arc_set_value(arc, 100);
    if (_arc_count < 18) {
        _arc_objects[_arc_count++] = arc;
    }
    return arc;
}

/* Row 1 right: Sector (fan) */
static gt_obj_st * _make_sector(gt_obj_st * scr)
{
    gt_obj_st * arc = gt_arc_create(scr);
    gt_arc_set_bg_color(arc, gt_color_hex(0xEEEEEE));
    gt_arc_set_bg_start_end_angle(arc, 0, 360);
    gt_arc_set_bg_width(arc, 4);
    gt_arc_set_act_color(arc, gt_color_hex(0x9C27B0));
    gt_arc_set_act_start_end_angle(arc, 315, 225);
    gt_arc_set_act_width(arc, 0xFFFF);
    if (_arc_count < 18) {
        _arc_objects[_arc_count++] = arc;
    }
    return arc;
}

/* Row 2 left: Filled circle */
static gt_obj_st * _make_filled_circle(gt_obj_st * scr)
{
    gt_obj_st * arc = gt_arc_create(scr);
    gt_arc_set_bg_color(arc, gt_color_hex(0xEEEEEE));
    gt_arc_set_bg_start_end_angle(arc, 0, 360);
    gt_arc_set_bg_width(arc, 4);
    gt_arc_set_act_color(arc, gt_color_hex(0xE91E63));
    gt_arc_set_act_start_end_angle(arc, 0, 360);
    gt_arc_set_act_width(arc, 0xFFFF);
    if (_arc_count < 18) {
        _arc_objects[_arc_count++] = arc;
    }
    return arc;
}

/* Row 2 right: Semicircle */
static gt_obj_st * _make_semicircle(gt_obj_st * scr)
{
    gt_obj_st * arc = gt_arc_create(scr);
    gt_arc_set_bg_color(arc, gt_color_hex(0xDDDDDD));
    gt_arc_set_bg_start_end_angle(arc, 180, 360);
    gt_arc_set_bg_width(arc, 20);
    gt_arc_set_act_color(arc, gt_color_hex(0x00BCD4));
    gt_arc_set_act_start_end_angle(arc, 180, 360);
    gt_arc_set_act_width(arc, 20);
    gt_arc_set_rounded(arc, true);
    gt_arc_set_value_min_max(arc, 0, 100);
    gt_arc_set_value(arc, 100);
    if (_arc_count < 18) {
        _arc_objects[_arc_count++] = arc;
    }
    return arc;
}

/* Row 3 left: Gradient red→yellow */
static gt_obj_st * _make_grad_2stop(gt_obj_st * scr)
{
    gt_obj_st * arc = gt_arc_create(scr);
    gt_arc_set_bg_color(arc, gt_color_hex(0xDDDDDD));
    gt_arc_set_bg_start_end_angle(arc, 135, 45);
    gt_arc_set_bg_width(arc, 16);
    gt_arc_set_act_start_end_angle(arc, 135, 45);
    gt_arc_set_act_width(arc, 16);
    gt_arc_set_rounded(arc, true);
    gt_arc_set_value_min_max(arc, 0, 100);
    gt_arc_set_value(arc, 80);
    gt_gradient_dsc_st * g = gt_gradient_dsc_create(GT_GRAD_DIR_HOR);
    gt_gradient_dsc_add_stop(g, gt_color_hex(0xFF1744), gt_color_hex(0xFFD600), 60);
    gt_arc_set_act_gradient(arc, g);
    if (_arc_count < 18) {
        _arc_objects[_arc_count++] = arc;
    }
    return arc;
}

/* Row 3 right: Gradient blue→cyan→green */
static gt_obj_st * _make_grad_3stop(gt_obj_st * scr)
{
    gt_obj_st * arc = gt_arc_create(scr);
    gt_arc_set_bg_color(arc, gt_color_hex(0xDDDDDD));
    gt_arc_set_bg_start_end_angle(arc, 0, 360);
    gt_arc_set_bg_width(arc, 18);
    gt_arc_set_act_start_end_angle(arc, 0, 360);
    gt_arc_set_act_width(arc, 18);
    gt_gradient_dsc_st * g = gt_gradient_dsc_create(GT_GRAD_DIR_HOR);
    gt_gradient_dsc_add_stop(g, gt_color_hex(0x1565C0), gt_color_hex(0x00BCD4), 40);
    gt_gradient_dsc_add_stop(g, gt_color_hex(0x00BCD4), gt_color_hex(0x4CAF50), 40);
    gt_arc_set_act_gradient(arc, g);
    if (_arc_count < 18) {
        _arc_objects[_arc_count++] = arc;
    }
    return arc;
}

/* Row 4 left: Rainbow ring */
static gt_obj_st * _make_rainbow_ring(gt_obj_st * scr)
{
    gt_obj_st * arc = gt_arc_create(scr);
    gt_arc_set_bg_color(arc, gt_color_hex(0xEEEEEE));
    gt_arc_set_bg_start_end_angle(arc, 0, 360);
    gt_arc_set_bg_width(arc, 4);
    gt_arc_set_act_start_end_angle(arc, 0, 360);
    gt_arc_set_act_width(arc, 24);
    gt_arc_set_act_gradient_rainbow(arc, 30);
    if (_arc_count < 18) {
        _arc_objects[_arc_count++] = arc;
    }
    return arc;
}

/* Row 4 right: Rainbow sector */
static gt_obj_st * _make_rainbow_sector(gt_obj_st * scr)
{
    gt_obj_st * arc = gt_arc_create(scr);
    gt_arc_set_bg_color(arc, gt_color_hex(0xEEEEEE));
    gt_arc_set_bg_start_end_angle(arc, 0, 360);
    gt_arc_set_bg_width(arc, 4);
    gt_arc_set_act_start_end_angle(arc, 0, 360);
    gt_arc_set_act_width(arc, 0xFFFF);
    gt_arc_set_act_gradient_rainbow(arc, 25);
    if (_arc_count < 18) {
        _arc_objects[_arc_count++] = arc;
    }
    return arc;
}

/* Row 5 left: Gradient semicircle + inner+outer fade */
static gt_obj_st * _make_grad_semi_fade(gt_obj_st * scr)
{
    gt_obj_st * arc = gt_arc_create(scr);
    gt_arc_set_bg_color(arc, gt_color_hex(0xDDDDDD));
    gt_arc_set_bg_start_end_angle(arc, 0, 180);
    gt_arc_set_bg_width(arc, 24);
    gt_arc_set_act_start_end_angle(arc, 0, 180);
    gt_arc_set_act_width(arc, 24);
    gt_arc_set_rounded(arc, true);
    gt_gradient_dsc_st * g = gt_gradient_dsc_create(GT_GRAD_DIR_HOR);
    gt_gradient_dsc_add_stop(g, gt_color_hex(0xFF6F00), gt_color_hex(0xAD1457), 60);
    gt_arc_set_act_gradient(arc, g);
    gt_arc_set_act_fade(arc, 6, 6);
    if (_arc_count < 18) {
        _arc_objects[_arc_count++] = arc;
    }
    return arc;
}

/* Row 5 right: Rainbow ring + fade */
static gt_obj_st * _make_rainbow_fade(gt_obj_st * scr)
{
    gt_obj_st * arc = gt_arc_create(scr);
    gt_arc_set_bg_color(arc, gt_color_hex(0xEEEEEE));
    gt_arc_set_bg_start_end_angle(arc, 0, 360);
    gt_arc_set_bg_width(arc, 4);
    gt_arc_set_act_start_end_angle(arc, 0, 360);
    gt_arc_set_act_width(arc, 32);
    gt_arc_set_act_gradient_rainbow(arc, 20);
    gt_arc_set_act_fade(arc, 8, 8);
    if (_arc_count < 18) {
        _arc_objects[_arc_count++] = arc;
    }
    return arc;
}

/* Row 6 left: Outer fade only */
static gt_obj_st * _make_outer_fade(gt_obj_st * scr)
{
    gt_obj_st * arc = gt_arc_create(scr);
    gt_arc_set_bg_color(arc, gt_color_hex(0xDDDDDD));
    gt_arc_set_bg_start_end_angle(arc, 135, 45);
    gt_arc_set_bg_width(arc, 24);
    gt_arc_set_act_start_end_angle(arc, 135, 45);
    gt_arc_set_act_width(arc, 24);
    gt_arc_set_value_min_max(arc, 0, 100);
    gt_arc_set_value(arc, 90);
    gt_gradient_dsc_st * g = gt_gradient_dsc_create(GT_GRAD_DIR_HOR);
    gt_gradient_dsc_add_stop(g, gt_color_hex(0x00E5FF), gt_color_hex(0x7C4DFF), 60);
    gt_arc_set_act_gradient(arc, g);
    gt_arc_set_act_fade(arc, 0, 10);
    if (_arc_count < 18) {
        _arc_objects[_arc_count++] = arc;
    }
    return arc;
}

/* Row 6 right: Inner fade only */
static gt_obj_st * _make_inner_fade(gt_obj_st * scr)
{
    gt_obj_st * arc = gt_arc_create(scr);
    gt_arc_set_bg_color(arc, gt_color_hex(0xDDDDDD));
    gt_arc_set_draw_bg(arc, false);
    gt_arc_set_bg_start_end_angle(arc, 135, 45);
    gt_arc_set_bg_width(arc, 24);
    gt_arc_set_act_start_end_angle(arc, 135, 45);
    gt_arc_set_act_width(arc, 24);
    gt_arc_set_value_min_max(arc, 0, 100);
    gt_arc_set_value(arc, 90);
    gt_gradient_dsc_st * g = gt_gradient_dsc_create(GT_GRAD_DIR_HOR);
    gt_gradient_dsc_add_stop(g, gt_color_hex(0x76FF03), gt_color_hex(0xFF6D00), 60);
    gt_arc_set_act_gradient(arc, g);
    gt_arc_set_act_fade(arc, 10, 0);
    if (_arc_count < 18) {
        _arc_objects[_arc_count++] = arc;
    }
    return arc;
}

/* Row 7 left: Concentric rings */
static void _make_concentric(gt_obj_st * scr, int32_t cx, int32_t cy)
{
    gt_obj_st * arc1 = gt_arc_create(scr);
    gt_obj_set_pos(arc1, cx, cy);
    gt_obj_set_size(arc1, _ARC_SZ, _ARC_SZ);
    gt_arc_set_bg_color(arc1, gt_color_hex(0xEEEEEE));
    gt_arc_set_bg_start_end_angle(arc1, 0, 360);
    gt_arc_set_bg_width(arc1, 12);
    gt_arc_set_act_color(arc1, gt_color_hex(0xF44336));
    gt_arc_set_act_start_end_angle(arc1, 0, 360);
    gt_arc_set_act_width(arc1, 12);
    gt_arc_set_value_min_max(arc1, 0, 100);
    gt_arc_set_value(arc1, 70);
    if (_arc_count < 18) {
        _arc_objects[_arc_count++] = arc1;
    }

    int32_t off = 24;
    gt_obj_st * arc2 = gt_arc_create(scr);
    gt_obj_set_pos(arc2, cx + off, cy + off);
    gt_obj_set_size(arc2, _ARC_SZ - off * 2, _ARC_SZ - off * 2);
    gt_arc_set_bg_color(arc2, gt_color_hex(0xEEEEEE));
    gt_arc_set_bg_start_end_angle(arc2, 0, 360);
    gt_arc_set_bg_width(arc2, 10);
    gt_arc_set_act_color(arc2, gt_color_hex(0x2196F3));
    gt_arc_set_act_start_end_angle(arc2, 0, 360);
    gt_arc_set_act_width(arc2, 10);
    gt_arc_set_value_min_max(arc2, 0, 100);
    gt_arc_set_value(arc2, 45);
    if (_arc_count < 18) {
        _arc_objects[_arc_count++] = arc2;
    }
}

/* Row 7 right: Rainbow circle with inner fade */
static gt_obj_st * _make_rainbow_circle(gt_obj_st * scr)
{
    gt_obj_st * arc = gt_arc_create(scr);
    gt_arc_set_bg_color(arc, gt_color_hex(0xEEEEEE));
    gt_arc_set_bg_start_end_angle(arc, 0, 360);
    gt_arc_set_bg_width(arc, 4);
    gt_arc_set_act_start_end_angle(arc, 0, 360);
    gt_arc_set_act_width(arc, 0xFFFF);
    gt_arc_set_act_gradient_rainbow(arc, 20);
    gt_arc_set_act_fade(arc, 12, 0);
    if (_arc_count < 18) {
        _arc_objects[_arc_count++] = arc;
    }
    return arc;
}

/* Row 8 left: Blue→pink smooth ring (reference style) */
static gt_obj_st * _make_blue_pink_ring(gt_obj_st * scr)
{
    gt_obj_st * arc = gt_arc_create(scr);
    gt_arc_set_bg_color(arc, gt_color_hex(0xEEEEEE));
    gt_arc_set_bg_start_end_angle(arc, 0, 360);
    gt_arc_set_bg_width(arc, 4);
    gt_arc_set_act_start_end_angle(arc, 0, 360);
    gt_arc_set_act_width(arc, 30);
    gt_gradient_dsc_st * g = gt_gradient_dsc_create(GT_GRAD_DIR_HOR);
    gt_gradient_dsc_add_stop(g, gt_color_hex(0x1A237E), gt_color_hex(0x7B1FA2), 30);
    gt_gradient_dsc_add_stop(g, gt_color_hex(0x7B1FA2), gt_color_hex(0xE91E63), 30);
    gt_gradient_dsc_add_stop(g, gt_color_hex(0xE91E63), gt_color_hex(0x1A237E), 30);
    gt_arc_set_act_gradient(arc, g);
    gt_arc_set_act_fade(arc, 6, 6);
    if (_arc_count < 18) {
        _arc_objects[_arc_count++] = arc;
    }
    return arc;
}

/* Row 8 right: 270° arc with gradient */
static gt_obj_st * _make_270_grad(gt_obj_st * scr)
{
    gt_obj_st * arc = gt_arc_create(scr);
    gt_arc_set_bg_color(arc, gt_color_hex(0xDDDDDD));
    gt_arc_set_bg_start_end_angle(arc, 225, 315);
    gt_arc_set_bg_width(arc, 20);
    gt_arc_set_act_start_end_angle(arc, 225, 315);
    gt_arc_set_act_width(arc, 20);
    gt_arc_set_rounded(arc, true);
    gt_arc_set_value_min_max(arc, 0, 100);
    gt_arc_set_value(arc, 60);
    gt_gradient_dsc_st * g = gt_gradient_dsc_create(GT_GRAD_DIR_HOR);
    gt_gradient_dsc_add_stop(g, gt_color_hex(0x00BFA5), gt_color_hex(0xF57F17), 60);
    gt_arc_set_act_gradient(arc, g);
    if (_arc_count < 18) {
        _arc_objects[_arc_count++] = arc;
    }
    return arc;
}

/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_examples_arc(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);
    gt_obj_set_size(screen, 800, 2000);

    /* Control buttons: fixed at top */
    _arc_count = 0;
    gt_obj_st * bg_toggle_btn = gt_btn_create(screen);
    gt_obj_set_pos(bg_toggle_btn, 10, 10);
    gt_obj_set_size(bg_toggle_btn, 100, 30);
    gt_btn_set_text(bg_toggle_btn, "BG:White");
    gt_obj_add_event_cb(bg_toggle_btn, _bg_toggle_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * draw_bg_toggle_btn = gt_btn_create(screen);
    gt_obj_set_pos(draw_bg_toggle_btn, 120, 10);
    gt_obj_set_size(draw_bg_toggle_btn, 120, 30);
    gt_btn_set_text(draw_bg_toggle_btn, "BG:ON");
    gt_obj_add_event_cb(draw_bg_toggle_btn, _draw_bg_toggle_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    _info_lbl = gt_label_create(screen);
    gt_obj_set_pos(_info_lbl, 250, 10);
    gt_obj_set_size(_info_lbl, 200, 30);
    gt_label_set_text(_info_lbl, "Arc count: 0");
    gt_label_set_font_size(_info_lbl, 14);

    /* Total content height: 9 rows × (200+20) + 20 = 1980+20 = 2000px */
    /* Row 0 */
    {
        gt_obj_st * a = _make_thin_ring(screen);
        _PLACE(screen, _COL_L, _ROW_Y(0), a, "Thin ring 75%%");
        a = _make_medium_ring(screen);
        _PLACE(screen, _COL_R, _ROW_Y(0), a, "Medium ring 50%%");
    }
    /* Row 1 */
    {
        gt_obj_st * a = _make_thick_ring(screen);
        _PLACE(screen, _COL_L, _ROW_Y(1), a, "Thick ring 100%%");
        a = _make_sector(screen);
        _PLACE(screen, _COL_R, _ROW_Y(1), a, "Sector (fan)");
    }
    /* Row 2 */
    {
        gt_obj_st * a = _make_filled_circle(screen);
        _PLACE(screen, _COL_L, _ROW_Y(2), a, "Filled circle");
        a = _make_semicircle(screen);
        _PLACE(screen, _COL_R, _ROW_Y(2), a, "Semicircle");
    }
    /* Row 3 */
    {
        gt_obj_st * a = _make_grad_2stop(screen);
        _PLACE(screen, _COL_L, _ROW_Y(3), a, "Grad red->yellow");
        a = _make_grad_3stop(screen);
        _PLACE(screen, _COL_R, _ROW_Y(3), a, "Grad blue->green");
    }
    /* Row 4 */
    {
        gt_obj_st * a = _make_rainbow_ring(screen);
        _PLACE(screen, _COL_L, _ROW_Y(4), a, "Rainbow ring");
        a = _make_rainbow_sector(screen);
        _PLACE(screen, _COL_R, _ROW_Y(4), a, "Rainbow sector");
    }
    /* Row 5 */
    {
        gt_obj_st * a = _make_grad_semi_fade(screen);
        _PLACE(screen, _COL_L, _ROW_Y(5), a, "Grad+fade semi");
        a = _make_rainbow_fade(screen);
        _PLACE(screen, _COL_R, _ROW_Y(5), a, "Rainbow+fade");
    }
    /* Row 6 */
    {
        gt_obj_st * a = _make_outer_fade(screen);
        _PLACE(screen, _COL_L, _ROW_Y(6), a, "Outer fade only");
        a = _make_inner_fade(screen);
        _PLACE(screen, _COL_R, _ROW_Y(6), a, "Inner fade only");
    }
    /* Row 7 */
    {
        _make_concentric(screen,
            _COL_L + _ARC_OFF_X, _ROW_Y(7) + _ARC_OFF_Y);
        gt_obj_st * _lbl = gt_label_create(screen);
        gt_obj_set_pos(_lbl, _COL_L, _ROW_Y(7) + _ARC_OFF_Y + _ARC_SZ + 4);
        gt_obj_set_size(_lbl, _CELL_W, _LBL_H);
        gt_label_set_text(_lbl, "Concentric rings");
        gt_label_set_font_size(_lbl, 14);
        gt_label_set_font_align(_lbl, GT_ALIGN_CENTER_MID);

        gt_obj_st * a = _make_rainbow_circle(screen);
        _PLACE(screen, _COL_R, _ROW_Y(7), a, "Rainbow circle");
    }
    /* Row 8 */
    {
        gt_obj_st * a = _make_blue_pink_ring(screen);
        _PLACE(screen, _COL_L, _ROW_Y(8), a, "Blue->pink ring");
        a = _make_270_grad(screen);
        _PLACE(screen, _COL_R, _ROW_Y(8), a, "270deg gradient");
    }

    /* Update arc count display */
    char buf[32];
    snprintf(buf, sizeof(buf), "Arc count: %d", _arc_count);
    gt_label_set_text(_info_lbl, buf);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
