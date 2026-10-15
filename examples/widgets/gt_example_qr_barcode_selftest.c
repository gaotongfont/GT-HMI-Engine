/**
 * @file gt_example_qr_barcode_selftest.c
 * @author Feyoung
 * @brief qr_ext / barcode setter invalidation & set_size self-test
 *        (tasks P1-5 / P1-6)
 * @version 0.1
 * @date 2026-10-15
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 *
 * Probes:
 *  - P1-5 (setter sends no invalidation): the widget _event_cb answers
 *    GT_EVENT_TYPE_DRAW_START by marking the display area dirty and sending
 *    GT_EVENT_TYPE_DRAW_END synchronously. The periodic repaint does NOT
 *    emit these events, so a user event callback counting DRAW_END is a
 *    clean "did this setter request a redraw" probe.
 *  - P1-6 (qr_ext area only grows): gt_obj_get_w/h of the qr object after
 *    set_size + one refresh cycle. Pre-fix the draw callback grows the area
 *    back to the natural QR size on every paint.
 *
 * Two-phase timer: phase 1 asserts the settled initial state and applies
 * the mutation, phase 2 asserts the re-rendered state at least one display
 * refresh later. Console output: [QRBC_TC] lines.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"

#if GT_USE_EXAMPLES

#include "../../src/widgets/gt_qr_ext.h"
#include "../../src/widgets/gt_barcode.h"
#include "../../driver/gt_gui_driver.h"     /* GT_FAMILY_BARCODE_* enums */
#include "../../src/widgets/gt_label.h"
#include "../../src/widgets/gt_obj.h"
#include "../../src/core/gt_timer.h"
#include "../../src/core/gt_event.h"
#include <stdio.h>
#include <stdint.h>

/* private define -------------------------------------------------------*/

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/
static gt_obj_st * _qr = NULL;          /* qr under test */
static gt_obj_st * _qr_nosize = NULL;   /* qr never set_size (auto-grow guard) */
static gt_obj_st * _bc = NULL;          /* barcode under test */
static gt_obj_st * _result_lbl = NULL;
static int _pass = 0, _total = 0;
static int _qr_draw_cnt = 0;            /* DRAW_END counter for _qr */
static int _bc_draw_cnt = 0;            /* DRAW_END counter for _bc */

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/
static void _tc_report(const char * name, bool ok, const char * detail)
{
    ++_total;
    if (ok) { ++_pass; }
    printf("[QRBC_TC] %-44s %s | %s\n", name, ok ? "PASS" : "FAIL", detail);
    fflush(stdout);
}

/** DRAW_END counter probe (see file header) */
static void _qr_draw_end_cb(gt_event_st * e)
{
    (void)e;
    ++_qr_draw_cnt;
}

static void _bc_draw_end_cb(gt_event_st * e)
{
    (void)e;
    ++_bc_draw_cnt;
}

/** assert a setter fired exactly exp redraw requests */
static void _tc_check_redraw(const char * name, int before, int after, int exp)
{
    char buf[64];
    int got = after - before;
    snprintf(buf, sizeof(buf), "exp %d redraw req, got %d", exp, got);
    _tc_report(name, got == exp, buf);
}

static void _summary(void)
{
    char rtxt[48];
    printf("[QRBC_TC] ===== %d/%d passed =====\n", _pass, _total);
    fflush(stdout);
    snprintf(rtxt, sizeof(rtxt), "self-test: %d/%d passed", _pass, _total);
    if (_result_lbl) { gt_label_set_text(_result_lbl, rtxt); }
}

/**
 * @brief phase machine: 0 = assert initial + mutate, 1 = assert final
 */
static void _selftest_timer_cb(_gt_timer_st * timer)
{
    if (0 == (uintptr_t)timer->user_data) {
        /* ---- phase 1 ---- */
        int c0 = _qr_draw_cnt;

        /* TC1: sanity baseline — object rendered at least once (w/h > 0) */
        {
            char buf[64];
            uint16_t w = gt_obj_get_w(_qr), h = gt_obj_get_h(_qr);
            snprintf(buf, sizeof(buf), "settled size %ux%u", w, h);
            _tc_report("TC1 qr initial auto-grow (no set_size)",
                w > 0 && h > 0, buf);
        }

        /* TC2: auto-grow sets the NATURAL (square) size */
        {
            char buf[64];
            uint16_t w = gt_obj_get_w(_qr), h = gt_obj_get_h(_qr);
            snprintf(buf, sizeof(buf), "w=%u h=%u shall be equal", w, h);
            _tc_report("TC2 qr auto-grow is square", w == h, buf);
        }

        /* ---- P1-5: qr_ext setters shall request a redraw ---- */
        c0 = _qr_draw_cnt;
        gt_qr_ext_set_module_color(_qr, gt_color_hex(0x0000FF));
        _tc_check_redraw("TC3 qr set_module_color redraws", c0, _qr_draw_cnt, 1);

        c0 = _qr_draw_cnt;
        gt_qr_ext_set_module_size(_qr, 3);
        _tc_check_redraw("TC4 qr set_module_size redraws", c0, _qr_draw_cnt, 1);

        c0 = _qr_draw_cnt;
        gt_qr_ext_set_margin(_qr, 3);
        _tc_check_redraw("TC5 qr set_margin redraws", c0, _qr_draw_cnt, 1);

        c0 = _qr_draw_cnt;
        gt_qr_ext_set_version(_qr, 5);
        _tc_check_redraw("TC6 qr set_version redraws", c0, _qr_draw_cnt, 1);

        c0 = _qr_draw_cnt;
        gt_qr_ext_set_ecc_level(_qr, GT_QR_ECC_LEVEL_MEDIUM);
        _tc_check_redraw("TC7 qr set_ecc_level redraws", c0, _qr_draw_cnt, 1);

        c0 = _qr_draw_cnt;
        gt_qr_ext_set_mask(_qr, GT_QR_MASK_1);
        _tc_check_redraw("TC8 qr set_mask redraws", c0, _qr_draw_cnt, 1);

        c0 = _qr_draw_cnt;
        gt_qr_ext_set_text(_qr, "HELL0-GT-W0RLD");  /* same len(14), new content */
        _tc_check_redraw("TC9 qr set_text same-len new-str redraws",
            c0, _qr_draw_cnt, 1);

        /* ---- P1-5: barcode setters shall request a redraw ---- */
        int b0 = _bc_draw_cnt;
        gt_barcode_set_code_text(_bc, "4006381333931");
        _tc_check_redraw("TC10 bc set_code_text redraws", b0, _bc_draw_cnt, 1);

        b0 = _bc_draw_cnt;
        gt_barcode_set_hri_type(_bc, GT_FAMILY_BARCODE_HRI_TYPE_NONE);
        _tc_check_redraw("TC11 bc set_hri_type redraws", b0, _bc_draw_cnt, 1);

        b0 = _bc_draw_cnt;
        gt_barcode_set_upc_e_sys_code(_bc, 1);
        _tc_check_redraw("TC12 bc set_upc_e_sys_code redraws",
            b0, _bc_draw_cnt, 1);

        /* already-working setters: regression guard (shall stay at 1) */
        b0 = _bc_draw_cnt;
        gt_barcode_set_mode_w(_bc, 3);
        _tc_check_redraw("TC13 bc set_mode_w redraws (regress)", b0, _bc_draw_cnt, 1);

        /* ---- P1-6 mutation: shrink the qr, no compensating call ---- */
        gt_obj_set_size(_qr, 60, 60);

        /* enlarge a second qr-free case is not needed; keep single object */
        timer->user_data = (void *)(uintptr_t)1;
        return ;
    }

    /* ---- phase 2: after at least one refresh cycle ---- */
    {
        char buf[64];
        uint16_t w = gt_obj_get_w(_qr), h = gt_obj_get_h(_qr);
        snprintf(buf, sizeof(buf), "exp 60x60, got %ux%u", w, h);
        _tc_report("TC14 qr set_size shrink sticks (P1-6)", w == 60 && h == 60, buf);
    }
    {
        char buf[64];
        uint16_t w = gt_obj_get_w(_qr_nosize), h = gt_obj_get_h(_qr_nosize);
        snprintf(buf, sizeof(buf), "untouched qr keeps natural size %ux%u", w, h);
        _tc_report("TC15 qr auto-grow unaffected (regress)", w > 0 && h == w, buf);
    }

    _summary();
    /* repeat_count == 2 auto-removes the timer after this run */
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief qr_ext / barcode setter & set_size self-test page
 */
gt_obj_st * gt_examples_qr_barcode_selftest(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);
    gt_screen_set_bgcolor(screen, gt_color_hex(0xffffff));

    gt_obj_st * title = gt_label_create(screen);
    gt_label_set_text(title, "qr/barcode setter+size self-test");
    gt_obj_set_pos(title, 10, 5);
    gt_obj_set_size(title, 400, 30);

    _result_lbl = gt_label_create(screen);
    gt_label_set_text(_result_lbl, "running...");
    gt_obj_set_pos(_result_lbl, 420, 5);
    gt_obj_set_size(_result_lbl, 300, 30);

    /* qr under test: NO set_size — auto-grow fills in the natural size */
    _qr = gt_qr_ext_create(screen);
    gt_obj_set_pos(_qr, 40, 60);
    gt_qr_ext_set_text(_qr, "https://gt.com");
    gt_obj_add_event_cb(_qr, _qr_draw_end_cb, GT_EVENT_TYPE_DRAW_END, NULL);

    /* second qr: never touched later, guards the auto-grow path */
    _qr_nosize = gt_qr_ext_create(screen);
    gt_obj_set_pos(_qr_nosize, 40, 260);
    gt_qr_ext_set_text(_qr_nosize, "GT-HMI");

    _bc = gt_barcode_create(screen);
    gt_obj_set_pos(_bc, 420, 80);
    gt_obj_add_event_cb(_bc, _bc_draw_end_cb, GT_EVENT_TYPE_DRAW_END, NULL);

    _gt_timer_st * tm = _gt_timer_create(_selftest_timer_cb, 150, NULL);
    _gt_timer_set_repeat_count(tm, 2);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
