/**
 * @file gt_example_checkbox.c
 * @author Feyoung
 * @brief Checkbox example with style/color variants and self checks
 * @version 0.2
 * @date 2026-10-15
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 *
 * Variants (visual matrix, left column unchecked / right column checked):
 *  - default blue   (0x409EFF, engine default)
 *  - red   0xF56C6C (unchecked: border shall be RED, not blue)
 *  - green 0x67C23A (checked: fill shall be GREEN)
 *  - orange 0xE6A23C with bigger font + custom font color
 *  - teal  0x00AA9A with CJK text
 *
 * Self checks ([CBX_TC] lines):
 *  - O-2  set_checked/get_checked roundtrip and state mapping
 *  - O-1  set_selected_color requests a redraw (DRAW_END counter probe)
 */

/* include --------------------------------------------------------------*/

#include "../gt_examples.h"

#if GT_USE_EXAMPLES

#include "../../src/widgets/gt_checkbox.h"
#include "../../src/widgets/gt_label.h"
#include "../../src/widgets/gt_obj.h"
#include "../../src/core/gt_event.h"
#include "../../src/core/gt_style.h"
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
    printf("[CBX_TC] %-44s %s | %s\n", name, ok ? "PASS" : "FAIL", detail);
    fflush(stdout);
}

static void _draw_end_cb(gt_event_st * e) { (void)e; ++_draw_end_cnt; }

static void _gt_check_event_cb(gt_event_st * e)
{
    printf("checkbox state: %s\n", GT_STATE_PRESSED == gt_obj_get_state(e->target) ? "selected" : "unselected");
}

/** one row: unchecked sample + checked sample + caption */
static void _add_pair(gt_obj_st * screen, int row, uint32_t color_hex,
                      uint8_t font_size, uint32_t font_color_hex, const char * txt)
{
    int y = 60 + row * 70;

    gt_obj_st * cap = gt_label_create(screen);
    char capbuf[48];
    snprintf(capbuf, sizeof(capbuf), "#%06X", color_hex);
    gt_label_set_text(cap, capbuf);
    gt_obj_set_pos(cap, 10, y + 12);
    gt_obj_set_size(cap, 90, 30);

    gt_obj_st * cb_un = gt_checkbox_create(screen);
    gt_checkbox_set_text(cb_un, txt);
    gt_obj_set_pos(cb_un, 100, y);
    gt_obj_set_size(cb_un, 180, 44);
    gt_checkbox_set_font_size(cb_un, font_size);
    gt_checkbox_set_font_color(cb_un, gt_color_hex(font_color_hex));
    gt_checkbox_set_selected_color(cb_un, gt_color_hex(color_hex));
    gt_obj_add_event_cb(cb_un, _gt_check_event_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * cb_ck = gt_checkbox_create(screen);
    gt_checkbox_set_text(cb_ck, txt);
    gt_obj_set_pos(cb_ck, 320, y);
    gt_obj_set_size(cb_ck, 180, 44);
    gt_checkbox_set_font_size(cb_ck, font_size);
    gt_checkbox_set_font_color(cb_ck, gt_color_hex(font_color_hex));
    gt_checkbox_set_selected_color(cb_ck, gt_color_hex(color_hex));
    gt_checkbox_set_checked(cb_ck, true);     /* O-2 API */
}

/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_examples_checkbox(void)
{
    gt_obj_st * screen;
    screen = gt_obj_create(NULL);
    gt_screen_set_bgcolor(screen, gt_color_hex(0xffffff));

    gt_obj_st * title = gt_label_create(screen);
    gt_label_set_text(title, "checkbox style/color variants");
    gt_obj_set_pos(title, 10, 5);
    gt_obj_set_size(title, 400, 30);

    gt_obj_st * hint = gt_label_create(screen);
    gt_label_set_text(hint, "left: unchecked (border)   right: checked (fill)");
    gt_obj_set_pos(hint, 10, 32);
    gt_obj_set_size(hint, 420, 24);

    /* row 0: engine default blue, original 20pt sample */
    gt_obj_st * checkbox = gt_checkbox_create(screen);
    gt_checkbox_set_text(checkbox, "Enabled");
    gt_obj_set_pos(checkbox, 100, 60);
    gt_obj_set_size(checkbox, 80, 20);
    gt_obj_add_event_cb(checkbox, _gt_check_event_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    /* color/style matrix (row 1..5) */
    _add_pair(screen, 1, 0xF56C6C, 20, 0x303133, "Red");
    _add_pair(screen, 2, 0x67C23A, 20, 0x303133, "Green");
    _add_pair(screen, 3, 0xE6A23C, 28, 0x7A3E00, "Orange 28pt");
    _add_pair(screen, 4, 0x00AA9A, 20, 0x303133, "\xe9\x9d\x92\xe8\x89\xb2");  /* 青色 (UTF-8) */

    /* ---- self checks (O-1/O-2 API contract) ---- */
    {
        gt_obj_st * probe = checkbox;

        /* O-2: roundtrip via state mapping */
        gt_checkbox_set_checked(probe, true);
        bool ok_state = (GT_STATE_PRESSED == gt_obj_get_state(probe));
        bool ok_get   = gt_checkbox_get_checked(probe);
        _tc_report("TC1 set_checked(true) state+getter", ok_state && ok_get,
            ok_state ? "state ok" : "state not PRESSED");

        gt_checkbox_set_checked(probe, false);
        ok_state = (GT_STATE_NONE == gt_obj_get_state(probe));
        ok_get   = !gt_checkbox_get_checked(probe);
        _tc_report("TC2 set_checked(false) state+getter", ok_state && ok_get,
            ok_state ? "state ok" : "state not NONE");

        /* O-1: set_selected_color requests a redraw */
        gt_obj_add_event_cb(probe, _draw_end_cb, GT_EVENT_TYPE_DRAW_END, NULL);
        gt_checkbox_set_checked(probe, true);               /* count = 1 */
        gt_checkbox_set_selected_color(probe, gt_color_hex(0xF56C6C));
        _tc_report("TC3 set_selected_color requests redraw", 2 == _draw_end_cnt,
            2 == _draw_end_cnt ? "redraw ok" : "no redraw requested");

        /* leave probe checked so the border/fill color is visible */
    }

    printf("[CBX_TC] ===== %d/%d passed =====\n", _pass, _total);
    fflush(stdout);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}


#endif
/* end ------------------------------------------------------------------*/
