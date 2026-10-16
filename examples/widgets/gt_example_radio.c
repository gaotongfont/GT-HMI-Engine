/**
 * @file gt_example_radio.c
 * @author Feyoung
 * @brief Radio example with style/color variants and self checks
 * @version 0.2
 * @date 2026-10-16
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 *
 * Variants:
 *  - group (gt_group, mutual exclusion on click): default blue 0x0F97FF,
 *    one option selected programmatically
 *  - color matrix rows: red 0xF56C6C / green 0x67C23A / orange 0xE6A23C,
 *    each row = unchecked ring + selected dot pair; the ring color shall
 *    follow gt_radio_set_selected_color (same accent rule as checkbox)
 *
 * Self checks ([RDO_TC] lines):
 *  - set_selected state mapping + group mutual exclusion
 *  - gt_radio_get_selected roundtrip
 *  - set_selected_color requests a redraw (DRAW_END counter probe)
 */

/* include --------------------------------------------------------------*/

#include "../gt_examples.h"

#if GT_USE_EXAMPLES

#include "../../src/widgets/gt_radio.h"
#include "../../src/widgets/gt_group.h"
#include "../../src/widgets/gt_label.h"
#include "../../src/widgets/gt_obj.h"
#include "../../src/core/gt_event.h"
#include <stdio.h>

/* private define -------------------------------------------------------*/

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/
static int _pass = 0, _total = 0;
static int _draw_end_cnt = 0;

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/
static void _tc_report(const char * name, bool ok, const char * detail)
{
    ++_total;
    if (ok) { ++_pass; }
    printf("[RDO_TC] %-44s %s | %s\n", name, ok ? "PASS" : "FAIL", detail);
    fflush(stdout);
}

static void _draw_end_cb(gt_event_st * e) { (void)e; ++_draw_end_cnt; }

static void _radio_event_cb(gt_event_st * e)
{
    printf("radio state: %s\n",
        GT_STATE_PRESSED == gt_obj_get_state(e->target) ? "selected" : "unselected");
}

/** one color row: unchecked sample + selected sample + caption */
static void _add_pair(gt_obj_st * screen, int row, uint32_t color_hex,
                      uint8_t font_size, const char * txt)
{
    int y = 200 + row * 70;

    gt_obj_st * cap = gt_label_create(screen);
    char capbuf[48];
    snprintf(capbuf, sizeof(capbuf), "#%06X", color_hex);
    gt_label_set_text(cap, capbuf);
    gt_obj_set_pos(cap, 10, y + 10);
    gt_obj_set_size(cap, 90, 30);

    gt_obj_st * r_un = gt_radio_create(screen);
    gt_radio_set_text(r_un, txt);
    gt_obj_set_pos(r_un, 100, y);
    gt_obj_set_size(r_un, 180, 44);
    gt_radio_set_font_size(r_un, font_size);
    gt_radio_set_selected_color(r_un, gt_color_hex(color_hex));
    gt_obj_add_event_cb(r_un, _radio_event_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * r_ck = gt_radio_create(screen);
    gt_radio_set_text(r_ck, txt);
    gt_obj_set_pos(r_ck, 320, y);
    gt_obj_set_size(r_ck, 180, 44);
    gt_radio_set_font_size(r_ck, font_size);
    gt_radio_set_selected_color(r_ck, gt_color_hex(color_hex));
    gt_radio_set_selected(r_ck);
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create radio demo with style/color variants and self checks.
 */
gt_obj_st * gt_examples_radio(void)
{
    gt_obj_st * screen;
    screen = gt_obj_create(NULL);
    gt_screen_set_bgcolor(screen, gt_color_hex(0xffffff));

    gt_obj_st * title = gt_label_create(screen);
    gt_label_set_text(title, "radio style/color variants");
    gt_obj_set_pos(title, 10, 5);
    gt_obj_set_size(title, 400, 30);

    gt_obj_st * hint = gt_label_create(screen);
    gt_label_set_text(hint, "group: click to switch (mutual exclusion)");
    gt_obj_set_pos(hint, 10, 32);
    gt_obj_set_size(hint, 420, 24);

    /* group: mutual exclusion on click, default blue */
    gt_obj_st * grp = gt_group_create(screen);
    gt_obj_set_pos(grp, 10, 60);
    gt_obj_set_size(grp, 220, 130);

    gt_obj_st * r1 = gt_radio_create(grp);
    gt_radio_set_text(r1, "Option A");
    gt_obj_set_pos(r1, 0, 0);
    gt_obj_set_size(r1, 200, 40);
    gt_obj_add_event_cb(r1, _radio_event_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * r2 = gt_radio_create(grp);
    gt_radio_set_text(r2, "Option B");
    gt_obj_set_pos(r2, 0, 45);
    gt_obj_set_size(r2, 200, 40);
    gt_obj_add_event_cb(r2, _radio_event_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * r3 = gt_radio_create(grp);
    gt_radio_set_text(r3, "Option C");
    gt_obj_set_pos(r3, 0, 90);
    gt_obj_set_size(r3, 200, 40);
    gt_obj_add_event_cb(r3, _radio_event_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    /* color matrix (unchecked ring + selected dot) */
    _add_pair(screen, 0, 0xF56C6C, 20, "Red");
    _add_pair(screen, 1, 0x67C23A, 20, "Green");
    _add_pair(screen, 2, 0xE6A23C, 28, "Orange 28pt");

    /* ---- self checks ---- */
    {
        /* TC1: set_selected state mapping + group mutual exclusion */
        gt_radio_set_selected(r1);
        bool ok_a = gt_radio_get_selected(r1);
        bool ok_b = !gt_radio_get_selected(r2);
        _tc_report("TC1 set_selected state + group exclusion", ok_a && ok_b,
            ok_a && ok_b ? "exclusive ok" : (ok_a ? "r2 still selected" : "r1 not selected"));

        /* TC2: getter roundtrip (switch selection inside the group) */
        gt_radio_set_selected(r3);
        bool ok_round = gt_radio_get_selected(r3) && !gt_radio_get_selected(r1);
        _tc_report("TC2 get_selected roundtrip", ok_round,
            ok_round ? "roundtrip ok" : "roundtrip broken");

        /* TC3: set_selected_color requests a redraw */
        gt_obj_add_event_cb(r3, _draw_end_cb, GT_EVENT_TYPE_DRAW_END, NULL);
        _draw_end_cnt = 0;
        gt_radio_set_selected_color(r3, gt_color_hex(0xF56C6C));   /* count = 1 */
        _tc_report("TC3 set_selected_color requests redraw", 1 == _draw_end_cnt,
            1 == _draw_end_cnt ? "redraw ok" : "no redraw requested");
    }

    printf("[RDO_TC] ===== %d/%d passed =====\n", _pass, _total);
    fflush(stdout);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
