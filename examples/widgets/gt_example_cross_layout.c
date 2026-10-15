/**
 * @file gt_example_cross_layout.c
 * @author Feyoung
 * @brief Cross layout widget example with layout self-test cases
 * @version 0.2
 * @date 2026-10-15
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_cross_layout.h"
#include "../../src/widgets/gt_label.h"
#include "../../src/widgets/gt_rect.h"
#include <stdio.h>

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/** ver area: left column */
#define _TC_VER_AREA_X          0
#define _TC_VER_AREA_Y          0
#define _TC_VER_AREA_W          200
#define _TC_VER_AREA_H          480

/** hor area: top-right strip */
#define _TC_HOR_AREA_X          200
#define _TC_HOR_AREA_Y          80
#define _TC_HOR_AREA_W          600
#define _TC_HOR_AREA_H          120

/** gap between items (both directions) */
#define _TC_GAP                 10

/** ver item: 160x40 label, centered in ver column */
#define _TC_VER_ITEM_W          160
#define _TC_VER_ITEM_H          40
#define _TC_VER_ITEM_EXP_X      (_TC_VER_AREA_X + ((_TC_VER_AREA_W - _TC_VER_ITEM_W) >> 1))   /* 20 */

/** hor item: 150x60, vertically centered in hor strip */
#define _TC_HOR_ITEM_W          150
#define _TC_HOR_ITEM_H          60
#define _TC_HOR_ITEM_EXP_Y      (_TC_HOR_AREA_Y + ((_TC_HOR_AREA_H - _TC_HOR_ITEM_H) >> 1))   /* 110 */

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/
static int _s_tc_pass = 0;
static int _s_tc_total = 0;

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/

/**
 * @brief Record one test case result and print it (flushed, log friendly).
 * @param name test case name
 * @param ok true if the assertion holds
 * @param detail expected/got description
 */
static void _tc_report(const char * name, bool ok, const char * detail)
{
    ++_s_tc_total;
    if (ok) { ++_s_tc_pass; }
    printf("[CROSS_LAYOUT_TC] %-32s %s | %s\n", name, ok ? "PASS" : "FAIL", detail);
    fflush(stdout);
}

/**
 * @brief Check an item is placed at the expected absolute position.
 * @param name test case name
 * @param item widget placed by cross layout
 * @param exp_x expected area x
 * @param exp_y expected area y
 */
static void _tc_check_pos(const char * name, gt_obj_st * item, gt_size_t exp_x, gt_size_t exp_y)
{
    char buf[64];
    gt_size_t x = gt_obj_get_x(item);
    gt_size_t y = gt_obj_get_y(item);
    snprintf(buf, sizeof(buf), "exp(%d,%d) got(%d,%d)", exp_x, exp_y, x, y);
    _tc_report(name, (x == exp_x && y == exp_y), buf);
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a cross layout page with self-test cases for hor/ver layout.
 *
 * API contract under test (see gt_cross_layout.h):
 *  - ver items are added first, hor items reference the ver group index;
 *  - hor/ver items must be children of gt_cross_layout_get_hor_obj()/
 *    gt_cross_layout_get_ver_obj();
 *  - hor group sub-vectors must stay aligned with ver_index even when a
 *    higher ver_index group is populated before a lower one.
 */
gt_obj_st * gt_examples_cross_layout(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);
    gt_screen_set_bgcolor(screen, gt_color_hex(0xffffff));

    gt_obj_st * title = gt_label_create(screen);
    gt_label_set_text(title, "cross_layout layout self-test");
    gt_obj_set_pos(title, 210, 20);
    gt_obj_set_size(title, 400, 30);

    gt_obj_st * layout = gt_cross_layout_create(screen);
    gt_obj_set_pos(layout, 0, 0);
    gt_obj_set_size(layout, 800, 480);
    gt_cross_layout_set_gap(layout, _TC_GAP, _TC_GAP);
    gt_cross_layout_set_ver_obj_area(layout, _TC_VER_AREA_X, _TC_VER_AREA_Y, _TC_VER_AREA_W, _TC_VER_AREA_H);
    gt_cross_layout_set_hor_obj_area(layout, _TC_HOR_AREA_X, _TC_HOR_AREA_Y, _TC_HOR_AREA_W, _TC_HOR_AREA_H);

    gt_obj_st * hor_parent = gt_cross_layout_get_hor_obj(layout);
    gt_obj_st * ver_parent = gt_cross_layout_get_ver_obj(layout);

    /* 1. ver items FIRST (ver_index must exist before hor add) */
    gt_obj_st * ver_items[2] = {0};
    for (int i = 0; i < 2; ++i) {
        ver_items[i] = gt_label_create(ver_parent);
        char txt[16];
        snprintf(txt, sizeof(txt), "Page %d", i);
        gt_label_set_text(ver_items[i], txt);
        gt_obj_set_size(ver_items[i], _TC_VER_ITEM_W, _TC_VER_ITEM_H);
        gt_cross_layout_ver_add_item(layout, ver_items[i]);
    }

    /* 2. hor group for ver_index 1 added BEFORE group 0:
     *    exercises hor sub-vector alignment with ver_index */
    gt_obj_st * g1_items[3] = {0};
    for (int i = 0; i < 3; ++i) {
        g1_items[i] = gt_label_create(hor_parent);
        char txt[16];
        snprintf(txt, sizeof(txt), "B%d", i);
        gt_label_set_text(g1_items[i], txt);
        gt_obj_set_size(g1_items[i], _TC_HOR_ITEM_W, _TC_HOR_ITEM_H);
        gt_cross_layout_hor_add_item(layout, g1_items[i], 1);
    }

    /* 3. hor group for ver_index 0: 5 items of 150px (5*150+4*10=790 > 600,
     *    so the strip is scrollable and set_hor_index can snap) */
    static const uint32_t g0_colors[] = {0xE3F2FD, 0xBBDEFB, 0x90CAF9, 0x64B5F6, 0x42A5F5};
    gt_obj_st * g0_items[5] = {0};
    for (int i = 0; i < 5; ++i) {
        g0_items[i] = gt_rect_create(hor_parent);
        gt_rect_set_bg_color(g0_items[i], gt_color_hex(g0_colors[i]));
        gt_rect_set_fill(g0_items[i], 1);
        gt_obj_set_size(g0_items[i], _TC_HOR_ITEM_W, _TC_HOR_ITEM_H);
        gt_cross_layout_hor_add_item(layout, g0_items[i], 0);
    }

    /* ---- TC1: ver items laid out in ver column ---- */
    _tc_check_pos("TC1 ver[0] layout", ver_items[0],
        _TC_VER_ITEM_EXP_X, _TC_VER_AREA_Y + 0 * (_TC_VER_ITEM_H + _TC_GAP));
    _tc_check_pos("TC1 ver[1] layout", ver_items[1],
        _TC_VER_ITEM_EXP_X, _TC_VER_AREA_Y + 1 * (_TC_VER_ITEM_H + _TC_GAP));

    /* ---- TC2: hor group 0 laid out in hor strip ---- */
    for (int i = 0; i < 5; ++i) {
        char name[32];
        snprintf(name, sizeof(name), "TC2 hor_g0[%d] layout", i);
        _tc_check_pos(name, g0_items[i],
            _TC_HOR_AREA_X + i * (_TC_HOR_ITEM_W + _TC_GAP), _TC_HOR_ITEM_EXP_Y);
    }

    /* ---- TC3: hor group counts (ver index still 0) ---- */
    {
        char buf[48];
        gt_size_t cnt = gt_cross_layout_get_hor_count(layout);
        snprintf(buf, sizeof(buf), "exp 5 got %d", cnt);
        _tc_report("TC3 hor_count(ver=0)==5", 5 == cnt, buf);
    }

    /* ---- TC4: set_hor_index(1) snaps item 1 to strip left edge ---- */
    gt_cross_layout_set_hor_index(layout, 1);
    _tc_check_pos("TC4 g0[1] snap after set_hor_idx", g0_items[1], _TC_HOR_AREA_X, _TC_HOR_ITEM_EXP_Y);
    {
        char buf[48];
        gt_size_t idx = gt_cross_layout_get_hor_index(layout);
        snprintf(buf, sizeof(buf), "exp 1 got %d", idx);
        _tc_report("TC4 get_hor_index==1", 1 == idx, buf);
    }

    /* ---- TC5: set_ver_index(1) switches to group 1 ---- */
    gt_cross_layout_set_ver_index(layout, 1);
    for (int i = 0; i < 3; ++i) {
        char name[32];
        snprintf(name, sizeof(name), "TC5 g1[%d] layout", i);
        _tc_check_pos(name, g1_items[i],
            _TC_HOR_AREA_X + i * (_TC_HOR_ITEM_W + _TC_GAP), _TC_HOR_ITEM_EXP_Y);
    }
    _tc_report("TC5 g1[0] visible after switch", GT_VISIBLE == g1_items[0]->visible,
        "g1[0] shall be visible");
    _tc_report("TC5 g0[0] hidden after switch", GT_VISIBLE != g0_items[0]->visible,
        "g0[0] shall be hidden");
    {
        char buf[48];
        gt_size_t cnt = gt_cross_layout_get_hor_count(layout);
        snprintf(buf, sizeof(buf), "exp 3 got %d", cnt);
        _tc_report("TC5 hor_count(ver=1)==3", 3 == cnt, buf);
    }

    printf("[CROSS_LAYOUT_TC] ===== %d/%d passed =====\n", _s_tc_pass, _s_tc_total);
    fflush(stdout);

    gt_obj_st * result = gt_label_create(screen);
    char rtxt[48];
    snprintf(rtxt, sizeof(rtxt), "self-test: %d/%d passed", _s_tc_pass, _s_tc_total);
    gt_label_set_text(result, rtxt);
    gt_obj_set_pos(result, 210, 40);
    gt_obj_set_size(result, 400, 30);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
