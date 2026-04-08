/**
 * @file gt_example_spectrum.c
 * @author Feyoung
 * @brief Spectrum and SpectrumLine widget examples
 * @version 0.1
 * @date 2026-04-08
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_spectrum.h"
#include "../../src/widgets/gt_spectrum_line.h"
#include "../../src/widgets/gt_label.h"
#include "../../src/others/gt_math.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* Spectrum layout */
#define _SP_W           180
#define _SP_H           120
#define _SP_GAP_X       20
#define _SP_GAP_Y       10
#define _SP_ROW1_Y      40
#define _SP_ROW2_Y      (_SP_ROW1_Y + _SP_H + _SP_GAP_Y + 20)

/* SpectrumLine layout */
#define _SL_X           10
#define _SL_Y           (_SP_ROW2_Y + _SP_H + _SP_GAP_Y + 20)
#define _SL_W           (GT_SCREEN_WIDTH - 20)
#define _SL_H           100

/* Spectrum common params */
#define _ITEM_NUMB      24
#define _ITEM_GAP       2
#define _RANGE_MAX      120
#define _RANGE_STEP     8
#define _PERIOD_MS      200

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/

/** @brief Helper: create a label above a widget */
static void _add_label(gt_obj_st * parent, gt_obj_st * ref, const char * text)
{
    gt_obj_st * lbl = gt_label_create(parent);
    gt_obj_set_pos(lbl, ref->area.x, ref->area.y - 18);
    gt_obj_set_size(lbl, _SP_W, 18);
    gt_label_set_text(lbl, text);
}

/** @brief Helper: apply 3-segment rainbow gradient (red→yellow→cyan→purple) */
static void _set_rainbow_gradient(gt_obj_st * sp)
{
    gt_spectrum_add_gradient_color(sp, gt_color_hex(0xCE0600), gt_color_hex(0xE9E520), _ITEM_NUMB / 3);
    gt_spectrum_add_gradient_color(sp, gt_color_hex(0xE9E520), gt_color_hex(0x12EEEA), _ITEM_NUMB / 3);
    gt_spectrum_add_gradient_color(sp, gt_color_hex(0x12EEEA), gt_color_hex(0xAB11BF), _ITEM_NUMB / 3);
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Spectrum and SpectrumLine widget examples.
 *
 * Row 1: RECT (color by numb, moving) | RECT_UD (color by range)
 * Row 2: CIRCLE                       | STAIR (reflect)
 * Row 3: SpectrumLine (multi-line Catmull-Rom curve)
 */
gt_obj_st * gt_examples_spectrum(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);
    gt_screen_set_bgcolor(screen, gt_color_hex(0x1A1A2E));

    /* ------------------------------------------------------------------ */
    /* Row 1 — RECT: color by numb, color moves right                     */
    /* ------------------------------------------------------------------ */
    gt_obj_st * sp1 = gt_spectrum_create(screen);
    gt_obj_set_pos(sp1, _SP_GAP_X, _SP_ROW1_Y);
    gt_obj_set_size(sp1, _SP_W, _SP_H);
    gt_obj_set_radius(sp1, 4);
    gt_spectrum_set_type(sp1, GT_SPECTRUM_TYPE_RECT);
    gt_spectrum_set_period(sp1, _PERIOD_MS);
    gt_spectrum_set_item_numb(sp1, _ITEM_NUMB, _ITEM_GAP);
    gt_spectrum_set_item_range(sp1, _RANGE_MAX, _RANGE_STEP);
    gt_spectrum_set_color_type(sp1, GT_SPECTRUM_COLOR_TYPE_NUMB);
    gt_spectrum_set_color_move(sp1, true, false, 1);
    _set_rainbow_gradient(sp1);
    gt_spectrum_play(sp1);
    _add_label(screen, sp1, "RECT (numb color)");

    /* ------------------------------------------------------------------ */
    /* Row 1 — RECT_UD: color by range, no move                           */
    /* ------------------------------------------------------------------ */
    gt_obj_st * sp2 = gt_spectrum_create(screen);
    gt_obj_set_pos(sp2, _SP_GAP_X * 2 + _SP_W, _SP_ROW1_Y);
    gt_obj_set_size(sp2, _SP_W, _SP_H);
    gt_obj_set_radius(sp2, 4);
    gt_spectrum_set_type(sp2, GT_SPECTRUM_TYPE_RECT_UD);
    gt_spectrum_set_period(sp2, _PERIOD_MS);
    gt_spectrum_set_item_numb(sp2, _ITEM_NUMB, _ITEM_GAP);
    gt_spectrum_set_item_range(sp2, _RANGE_MAX, _RANGE_STEP);
    gt_spectrum_set_color_type(sp2, GT_SPECTRUM_COLOR_TYPE_RANGE);
    gt_spectrum_add_gradient_color(sp2, gt_color_hex(0x0066FF), gt_color_hex(0x00FFCC), _RANGE_STEP);
    gt_spectrum_play(sp2);
    _add_label(screen, sp2, "RECT_UD (range color)");

    /* ------------------------------------------------------------------ */
    /* Row 2 — CIRCLE                                                      */
    /* ------------------------------------------------------------------ */
    gt_obj_st * sp3 = gt_spectrum_create(screen);
    gt_obj_set_pos(sp3, _SP_GAP_X, _SP_ROW2_Y);
    gt_obj_set_size(sp3, _SP_H, _SP_H);   /* CIRCLE must be square */
    gt_spectrum_set_type(sp3, GT_SPECTRUM_TYPE_CIRCLE);
    gt_spectrum_set_period(sp3, _PERIOD_MS);
    gt_spectrum_set_item_numb(sp3, _ITEM_NUMB, 3);
    gt_spectrum_set_item_range(sp3, _SP_H / 2, _RANGE_STEP);
    gt_spectrum_set_color_type(sp3, GT_SPECTRUM_COLOR_TYPE_NUMB);
    gt_spectrum_set_color_move(sp3, true, true, 1);
    _set_rainbow_gradient(sp3);
    gt_spectrum_play(sp3);
    _add_label(screen, sp3, "CIRCLE");

    /* ------------------------------------------------------------------ */
    /* Row 2 — STAIR with reflect                                          */
    /* ------------------------------------------------------------------ */
    gt_obj_st * sp4 = gt_spectrum_create(screen);
    gt_obj_set_pos(sp4, _SP_GAP_X * 2 + _SP_W, _SP_ROW2_Y);
    gt_obj_set_size(sp4, _SP_W, _SP_H);
    gt_obj_set_radius(sp4, 2);
    gt_spectrum_set_type(sp4, GT_SPECTRUM_TYPE_STAIR);
    gt_spectrum_set_period(sp4, _PERIOD_MS);
    gt_spectrum_set_item_numb(sp4, _ITEM_NUMB, _ITEM_GAP);
    gt_spectrum_set_item_range(sp4, _RANGE_MAX, _RANGE_STEP);
    gt_spectrum_set_item_range_gap(sp4, 1);
    gt_spectrum_set_item_range_reflect(sp4, true);
    gt_spectrum_set_color_type(sp4, GT_SPECTRUM_COLOR_TYPE_NUMB);
    gt_spectrum_set_color_move(sp4, true, false, 1);
    _set_rainbow_gradient(sp4);
    gt_spectrum_play(sp4);
    _add_label(screen, sp4, "STAIR (reflect)");

#if GT_CFG_ENABLE_SPECTRUM_LINE
    /* ------------------------------------------------------------------ */
    /* Row 3 — SpectrumLine: 8 Catmull-Rom curves, gradient start→end     */
    /* ------------------------------------------------------------------ */
    gt_obj_st * lbl_sl = gt_label_create(screen);
    gt_obj_set_pos(lbl_sl, _SL_X, _SL_Y - 18);
    gt_obj_set_size(lbl_sl, 200, 18);
    gt_label_set_text(lbl_sl, "SpectrumLine");

    gt_obj_st * sl = gt_spectrum_line_create(screen);
    gt_obj_set_pos(sl, _SL_X, _SL_Y);
    gt_obj_set_size(sl, _SL_W, _SL_H);

    /* Add the first (master) line and its key points */
    gt_spectrum_line_add(sl, 2, gt_color_hex(0xFF0000));

    int32_t center_y = _SL_Y + _SL_H / 2;
    uint16_t pt_count = 9;
    uint16_t pt_step  = _SL_W / (pt_count - 1);

    for (uint16_t i = 0; i < pt_count; ++i) {
        gt_sp_point_st pt = {
            .point = { .x = _SL_X + i * pt_step, .y = center_y },
            .gen   = {
                .min_val = center_y - _SL_H / 2 + 4,
                .max_val = center_y + _SL_H / 2 - 4,
                .step    = 4 + (i % 3) * 2,   /* vary step per point */
                .cur_val = center_y,
                .dir     = (i % 2 == 0) ? 1 : -1,
            },
        };
        gt_spectrum_line_add_point(sl, 0, pt);
    }

    /* Replicate into 8 lines with gradient red → purple */
    gt_spectrum_line_move_copy(sl, 8,
        gt_color_hex(0xFF2200),
        gt_color_hex(0x8800FF));

    gt_spectrum_line_set_period(sl, 80);
    gt_spectrum_line_play(sl);
#endif /* GT_CFG_ENABLE_SPECTRUM_LINE */

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
