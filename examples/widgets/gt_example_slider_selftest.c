/**
 * @file gt_example_slider_selftest.c
 * @author Feyoung
 * @brief Slider offset recompute / underflow self-test (tasks P0-2 / P1-3)
 * @version 0.1
 * @date 2026-10-15
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 *
 * Probe: each slider uses a 4x4 raw-image tag; the widget repositions the
 * tag child object on every draw, so gt_obj_get_x/y of the tag (child #0)
 * reflects the internal offset after geometry/direction changes.
 *
 * Two-phase timer: phase 1 asserts the settled initial state and applies
 * the mutation (set_dir / set_size), phase 2 asserts the re-rendered state
 * at least one display refresh later. Console output: [SLIDER_TC] lines.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_slider.h"
#include "../../src/widgets/gt_label.h"
#include "../../src/widgets/gt_obj.h"
#include "../../src/core/gt_timer.h"
#include <stdio.h>
#include <stdint.h>

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/
static gt_obj_st * _s[6] = {0};          /* sliders under test */
static gt_obj_st * _result_lbl = NULL;
static int _pass = 0, _total = 0;

static const gt_color_t _tag_raw_buf[4 * 4] = {
    0x001f,0x001f,0x001f,0x001f, 0x001f,0x001f,0x001f,0x001f,
    0x001f,0x001f,0x001f,0x001f, 0x001f,0x001f,0x001f,0x001f,
};
static const gt_opa_t _tag_raw_opa[4 * 4] = {
    0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,
};
static gt_color_img_raw_st _tag_raw_dsc = {
    .buffer = _tag_raw_buf,
    .opa    = _tag_raw_opa,
    .width  = 4,
    .height = 4,
};

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/
static void _tc_report(const char * name, bool ok, const char * detail)
{
    ++_total;
    if (ok) { ++_pass; }
    printf("[SLIDER_TC] %-40s %s | %s\n", name, ok ? "PASS" : "FAIL", detail);
    fflush(stdout);
}

/** exact-position check on the tag img (child #0 of the slider) */
static void _tc_check_tag(const char * name, gt_obj_st * slider, int exp_x, int exp_y)
{
    char buf[64];
    gt_obj_st * tag = gt_obj_get_child_at(slider, 0);
    if (NULL == tag) {
        snprintf(buf, sizeof(buf), "no tag child");
        _tc_report(name, false, buf);
        return;
    }
    int x = gt_obj_get_x(tag), y = gt_obj_get_y(tag);
    snprintf(buf, sizeof(buf), "exp(%d,%d) got(%d,%d)", exp_x, exp_y, x, y);
    _tc_report(name, (x == exp_x && y == exp_y), buf);
}

/** range check (defensive cases: knob must stay near the widget) */
static void _tc_check_tag_range(const char * name, gt_obj_st * slider,
    int min_x, int max_x, int min_y, int max_y)
{
    char buf[96];
    gt_obj_st * tag = gt_obj_get_child_at(slider, 0);
    if (NULL == tag) {
        snprintf(buf, sizeof(buf), "no tag child");
        _tc_report(name, false, buf);
        return;
    }
    int x = gt_obj_get_x(tag), y = gt_obj_get_y(tag);
    snprintf(buf, sizeof(buf), "got(%d,%d) want x[%d~%d] y[%d~%d]", x, y, min_x, max_x, min_y, max_y);
    _tc_report(name, (x >= min_x && x <= max_x && y >= min_y && y <= max_y), buf);
}

static void _summary(void)
{
    char rtxt[48];
    printf("[SLIDER_TC] ===== %d/%d passed =====\n", _pass, _total);
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
        /* ---- phase 1: settled initial states ---- */
        _tc_check_tag("TC1 s1 hor init (320x20 @50)",      _s[0], 178, 48);
        _tc_check_tag("TC3 s2 ver init (20x260 @50)",      _s[1], 68, 168);
        _tc_check_tag("TC5 s3 hor init",                   _s[2], 578, 48);
        _tc_check_tag("TC7 s4 hor init",                   _s[3], 578, 148);
        _tc_check_tag_range("TC9 s5 ver-geom+hor-dir clamp", _s[4], 196, 224, 256, 284);
        _tc_check_tag("TC10 s6 hor init",                  _s[5], 398, 308);

        /* ---- mutations: NO compensating set_pos afterwards ---- */
        /* s1: 横→竖 set_dir+set_size（P0-2 表第 1 行） */
        gt_slider_set_dir(_s[0], GT_BAR_DIR_VER_U2D);
        gt_obj_set_size(_s[0], 20, 260);
        gt_event_send(_s[0], GT_EVENT_TYPE_DRAW_START, NULL);

        /* s2: 竖→横 set_dir+set_size（P0-2 表第 5 行） */
        gt_slider_set_dir(_s[1], GT_BAR_DIR_HOR_L2R);
        gt_obj_set_size(_s[1], 320, 20);
        gt_event_send(_s[1], GT_EVENT_TYPE_DRAW_START, NULL);

        /* s3: 仅切 dir，几何不动（P0-2 表第 4 行，应一直正常） */
        gt_slider_set_dir(_s[2], GT_BAR_DIR_HOR_R2L);

        /* s4: 仅切 dir 到竖向，横几何（P1-3 失配） */
        gt_slider_set_dir(_s[3], GT_BAR_DIR_VER_U2D);

        /* s6: 纯 set_size（P0-2 核心） */
        gt_obj_set_size(_s[5], 160, 20);
        gt_event_send(_s[5], GT_EVENT_TYPE_DRAW_START, NULL);

        timer->user_data = (void *)(uintptr_t)1;
        return ;
    }

    /* ---- phase 2: re-rendered states ---- */
    _tc_check_tag("TC2 s1 hor->ver no re-set pos",        _s[0], 28, 168);
    _tc_check_tag("TC4 s2 ver->hor no re-set pos",        _s[1], 218, 48);
    _tc_check_tag("TC6 s3 dir-flip same geometry",        _s[2], 578, 48);
    _tc_check_tag_range("TC8 s4 hor-geom+ver-dir clamp",  _s[3], 416, 744, 136, 164);
    _tc_check_tag("TC11 s6 set_size only recompute",      _s[5], 318, 308);

    _summary();
    /* repeat_count == 2 auto-removes the timer after this run */
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief slider offset/underflow self-test page
 */
gt_obj_st * gt_examples_slider_selftest(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);
    gt_screen_set_bgcolor(screen, gt_color_hex(0xffffff));

    gt_obj_st * title = gt_label_create(screen);
    gt_label_set_text(title, "slider offset/underflow self-test");
    gt_obj_set_pos(title, 10, 5);
    gt_obj_set_size(title, 400, 30);

    _result_lbl = gt_label_create(screen);
    gt_label_set_text(_result_lbl, "running...");
    gt_obj_set_pos(_result_lbl, 420, 5);
    gt_obj_set_size(_result_lbl, 300, 30);

    /* s1: horizontal 320x20, later switched to vertical 20x260 */
    _s[0] = gt_slider_create(screen);
    gt_obj_set_pos(_s[0], 20, 40);
    gt_obj_set_size(_s[0], 320, 20);
    gt_slider_set_tag_raw_data(_s[0], &_tag_raw_dsc);
    gt_slider_set_pos(_s[0], 50);

    /* s2: vertical 20x260, later switched to horizontal 320x20 */
    _s[1] = gt_slider_create(screen);
    gt_obj_set_pos(_s[1], 60, 40);
    gt_obj_set_size(_s[1], 20, 260);
    gt_slider_set_dir(_s[1], GT_BAR_DIR_VER_U2D);
    gt_slider_set_tag_raw_data(_s[1], &_tag_raw_dsc);
    gt_slider_set_pos(_s[1], 50);

    /* s3: horizontal, dir flip only */
    _s[2] = gt_slider_create(screen);
    gt_obj_set_pos(_s[2], 420, 40);
    gt_obj_set_size(_s[2], 320, 20);
    gt_slider_set_tag_raw_data(_s[2], &_tag_raw_dsc);
    gt_slider_set_pos(_s[2], 50);

    /* s4: horizontal geometry, dir switched to vertical without resize */
    _s[3] = gt_slider_create(screen);
    gt_obj_set_pos(_s[3], 420, 140);
    gt_obj_set_size(_s[3], 320, 20);
    gt_slider_set_tag_raw_data(_s[3], &_tag_raw_dsc);
    gt_slider_set_pos(_s[3], 50);

    /* s5: vertical geometry with default horizontal dir (P1-3 row 7) */
    _s[4] = gt_slider_create(screen);
    gt_obj_set_pos(_s[4], 200, 140);
    gt_obj_set_size(_s[4], 20, 260);
    gt_slider_set_tag_raw_data(_s[4], &_tag_raw_dsc);
    gt_slider_set_pos(_s[4], 50);

    /* s6: pure set_size shrink */
    _s[5] = gt_slider_create(screen);
    gt_obj_set_pos(_s[5], 240, 300);
    gt_obj_set_size(_s[5], 320, 20);
    gt_slider_set_tag_raw_data(_s[5], &_tag_raw_dsc);
    gt_slider_set_pos(_s[5], 50);

    _gt_timer_st * tm = _gt_timer_create(_selftest_timer_cb, 150, NULL);
    _gt_timer_set_repeat_count(tm, 2);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
