/**
 * @file gt_example_o_optim_selftest.c
 * @author Feyoung
 * @brief Engine-side optimization items self-test (tasks O-1/O-2/O-3/O-4/O-5)
 * @version 0.1
 * @date 2026-10-15
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 *
 * Under test:
 *  - O-2  gt_checkbox_set_checked / gt_checkbox_get_checked
 *  - O-1  gt_checkbox_set_selected_color (selected color no longer hardcoded)
 *         gt_radio_set_selected_color shall request a redraw (P1-5 pattern)
 *  - O-5  gt_textarea_set_line_gap / gt_textarea_get_line_gap
 *  - O-3  gt_arc_set_knob_dot_color (independent knob-dot color source)
 *  - O-4  gt_barcode_get_natural_size (query API, front end can center)
 *
 * Redraw probe: DRAW_END event counter (only setters requesting a redraw
 * increment it; the periodic repaint does not emit the event).
 * Barcode area probe: the draw callback sizes the widget to its natural
 * size, so gt_obj_get_w/h must match get_natural_size after each repaint.
 *
 * Two-phase timer for the barcode repaint case. Console: [OPT_TC] lines.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"

#if GT_USE_EXAMPLES

#include "../../src/widgets/gt_checkbox.h"
#include "../../src/widgets/gt_radio.h"
#include "../../src/widgets/gt_textarea.h"
#include "../../src/widgets/gt_arc.h"
#include "../../src/widgets/gt_barcode.h"
#include "../../driver/gt_gui_driver.h"     /* barcode driver types */
#include "../../src/widgets/gt_label.h"
#include "../../src/widgets/gt_obj.h"
#include "../../src/core/gt_timer.h"
#include "../../src/core/gt_event.h"
#include "../../src/core/gt_style.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/* private define -------------------------------------------------------*/
#define _TC_BARCODE_MIN_H   (10)     /* BARCODE_LINE_NUMB floor in driver */

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/
static gt_obj_st * _cb = NULL;
static gt_obj_st * _rd = NULL;
static gt_obj_st * _ta = NULL;
static gt_obj_st * _arc = NULL;
static gt_obj_st * _bc = NULL;
static gt_obj_st * _result_lbl = NULL;
static int _pass = 0, _total = 0;
static int _cnt[5] = {0};                  /* DRAW_END counters */
static uint16_t _bc_nat_w1 = 0;            /* natural width at mode_w=2 */
static uint16_t _bc_area_w1 = 0;

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/
static void _tc_report(const char * name, bool ok, const char * detail)
{
    ++_total;
    if (ok) { ++_pass; }
    printf("[OPT_TC] %-46s %s | %s\n", name, ok ? "PASS" : "FAIL", detail);
    fflush(stdout);
}

static void _tc_check_redraw(const char * name, int idx, int exp)
{
    char buf[64];
    _tc_report(name, _cnt[idx] == exp,
        (_cnt[idx] == exp) ? "redraw ok" : "no redraw requested");
}

static void _draw_end_cb0(gt_event_st * e) { (void)e; ++_cnt[0]; }
static void _draw_end_cb1(gt_event_st * e) { (void)e; ++_cnt[1]; }
static void _draw_end_cb2(gt_event_st * e) { (void)e; ++_cnt[2]; }
static void _draw_end_cb3(gt_event_st * e) { (void)e; ++_cnt[3]; }
static void _draw_end_cb4(gt_event_st * e) { (void)e; ++_cnt[4]; }

static void _summary(void)
{
    char rtxt[48];
    printf("[OPT_TC] ===== %d/%d passed =====\n", _pass, _total);
    fflush(stdout);
    snprintf(rtxt, sizeof(rtxt), "self-test: %d/%d passed", _pass, _total);
    if (_result_lbl) { gt_label_set_text(_result_lbl, rtxt); }
}

/**
 * @brief phase machine: 0 = probes + barcode mutation, 1 = barcode re-check
 */
static void _selftest_timer_cb(_gt_timer_st * timer)
{
    if (0 == (uintptr_t)timer->user_data) {
        /* ---- O-2: set_checked / get_checked ---- */
        gt_checkbox_set_checked(_cb, true);
        {
            bool st = (GT_STATE_PRESSED == gt_obj_get_state(_cb));
            bool gt = gt_checkbox_get_checked(_cb);
            _tc_report("TC1 cb set_checked(true): state+getter",
                st && gt, st ? "state ok" : "state not PRESSED");
        }
        _tc_check_redraw("TC2 cb set_checked requests redraw", 0, 1);

        gt_checkbox_set_checked(_cb, false);
        {
            bool st = (GT_STATE_NONE == gt_obj_get_state(_cb));
            bool gt = !gt_checkbox_get_checked(_cb);
            _tc_report("TC3 cb set_checked(false): state+getter",
                st && gt, st ? "state ok" : "state not NONE");
        }

        /* ---- O-1: checkbox selected color setter ---- */
        gt_checkbox_set_selected_color(_cb, gt_color_hex(0xFF0000));
        _tc_check_redraw("TC4 cb set_selected_color requests redraw", 0, 3);

        /* ---- O-1: radio selected color shall request redraw ---- */
        gt_radio_set_selected_color(_rd, gt_color_hex(0x00AA00));
        _tc_check_redraw("TC5 rd set_selected_color requests redraw", 1, 1);

        /* ---- O-5: textarea line gap ---- */
        gt_textarea_set_line_gap(_ta, 8);
        {
            char buf[48];
            uint8_t gap = gt_textarea_get_line_gap(_ta);
            snprintf(buf, sizeof(buf), "gap=%u exp 8", gap);
            _tc_report("TC6 ta set/get_line_gap roundtrip", 8 == gap, buf);
        }
        _tc_check_redraw("TC7 ta set_line_gap requests redraw", 2, 1);

        /* ---- O-3: arc knob dot color ---- */
        gt_arc_set_knob_dot_color(_arc, gt_color_hex(0xFF00FF));
        _tc_check_redraw("TC8 arc set_knob_dot_color requests redraw", 3, 1);

        /* ---- O-4: natural size matches rendered area @mode_w=2 ---- */
        {
            uint16_t nw = 0, nh = 0;
            char buf[96];
            gt_barcode_get_natural_size(_bc, &nw, &nh);
            _bc_nat_w1 = nw;
            _bc_area_w1 = gt_obj_get_w(_bc);
            snprintf(buf, sizeof(buf), "nat=%ux%u area=%ux%u",
                nw, nh, gt_obj_get_w(_bc), gt_obj_get_h(_bc));
            _tc_report("TC9 bc natural size == rendered area (w=2)",
                nw == _bc_area_w1 && nh == gt_obj_get_h(_bc), buf);
        }

        /* ---- mutation: double the module width ---- */
        gt_barcode_set_mode_w(_bc, 4);

        timer->user_data = (void *)(uintptr_t)1;
        return ;
    }

    /* ---- phase 2: barcode repainted at mode_w=4 ---- */
    {
        uint16_t nw = 0, nh = 0;
        char buf[96];
        gt_barcode_get_natural_size(_bc, &nw, &nh);
        snprintf(buf, sizeof(buf), "nat %u->%u, area %u->%u",
            _bc_nat_w1, nw, _bc_area_w1, gt_obj_get_w(_bc));
        _tc_report("TC10 bc natural size grows & tracks area (w=4)",
            nw > _bc_nat_w1 && nw == gt_obj_get_w(_bc), buf);
    }

    _summary();
    /* repeat_count == 2 auto-removes the timer after this run */
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief optimization items self-test page
 */
gt_obj_st * gt_examples_o_optim_selftest(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);
    gt_screen_set_bgcolor(screen, gt_color_hex(0xffffff));

    gt_obj_st * title = gt_label_create(screen);
    gt_label_set_text(title, "O-1..O-5 engine-side self-test");
    gt_obj_set_pos(title, 10, 5);
    gt_obj_set_size(title, 400, 30);

    _result_lbl = gt_label_create(screen);
    gt_label_set_text(_result_lbl, "running...");
    gt_obj_set_pos(_result_lbl, 420, 5);
    gt_obj_set_size(_result_lbl, 300, 30);

    _cb = gt_checkbox_create(screen);
    gt_obj_set_pos(_cb, 30, 60);
    gt_obj_set_size(_cb, 140, 30);
    gt_checkbox_set_text(_cb, "cb");
    gt_obj_add_event_cb(_cb, _draw_end_cb0, GT_EVENT_TYPE_DRAW_END, NULL);

    _rd = gt_radio_create(screen);
    gt_obj_set_pos(_rd, 200, 60);
    gt_obj_set_size(_rd, 140, 30);
    gt_radio_set_text(_rd, "rd");
    gt_obj_add_event_cb(_rd, _draw_end_cb1, GT_EVENT_TYPE_DRAW_END, NULL);

    _ta = gt_textarea_create(screen);
    gt_obj_set_pos(_ta, 30, 110);
    gt_obj_set_size(_ta, 310, 100);
    gt_textarea_set_text(_ta, "line gap 8");
    gt_obj_add_event_cb(_ta, _draw_end_cb2, GT_EVENT_TYPE_DRAW_END, NULL);

    _arc = gt_arc_create(screen);
    gt_obj_set_pos(_arc, 600, 60);
    gt_obj_set_size(_arc, 160, 160);
    gt_arc_set_bg_width(_arc, 10);
    gt_arc_set_act_width(_arc, 14);
    gt_arc_set_act_start_end_angle(_arc, 0, 300);
    gt_arc_set_knob_width(_arc, 6);
    gt_arc_set_knob_dot(_arc, 10);
    gt_obj_add_event_cb(_arc, _draw_end_cb3, GT_EVENT_TYPE_DRAW_END, NULL);

    _bc = gt_barcode_create(screen);
    gt_obj_set_pos(_bc, 30, 240);
    gt_barcode_set_mode_w(_bc, 2);
    gt_barcode_set_mode_h(_bc, 64);
    gt_obj_add_event_cb(_bc, _draw_end_cb4, GT_EVENT_TYPE_DRAW_END, NULL);

    _gt_timer_st * tm = _gt_timer_create(_selftest_timer_cb, 150, NULL);
    _gt_timer_set_repeat_count(tm, 2);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
