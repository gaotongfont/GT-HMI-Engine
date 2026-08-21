/**
 * @file gt_example_line.c
 * @author Feyoung
 * @brief Line widget test: angles, widths, colors, opacity, brush types
 * @version 0.4
 * @date 2026-04-14
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/core/gt_event.h"
#include "../../src/widgets/gt_btn.h"
#include "../../src/widgets/gt_label.h"
#include "../../src/widgets/gt_line.h"
#include "../../src/core/gt_graph_base.h"


/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/**
 * Screen: 800 x 480, scrollable (total height ~1376px).
 *
 * Section layout — each section starts at _Yn, label height=30, gap=4:
 *   [1] Y=    0  H/V lines  w=1/3/6/10           total=100
 *   [2] Y=  100  16 angles 0~345 step15 L=80      total=214
 *   [3] Y=  314  Width round  w=1~12 7 lines       total=238
 *   [4] Y=  552  Width square w=1~12 7 lines       total=238
 *   [5] Y=  790  Colors+OPA  w=4 45deg             total=198
 *   [6] Y=  988  Near-H/V AA stress                total=154
 *   [7] Y= 1142  Special angles                    total=234
 *   Total: ~1376px
 */

/* Section Y starts */
#define _Y1      0
#define _Y2    100
#define _Y3    314
#define _Y4    600
#define _Y5    880
#define _Y6    1150
#define _Y7    1300
#define _Y8    1630
#define _Y9    2120

/* Label height + gap before content */
#define _LH    34   /* 30px label + 4px gap */

/* Color shortcuts */
#define C_W   gt_color_white()
#define C_Y   gt_color_yellow()
#define C_O   gt_color_orange()
#define C_R   gt_color_red()
#define C_G   gt_color_green()
#define C_B   gt_color_blue()

/* ── helpers ─────────────────────────────────────────────────────────── */

static gt_obj_st * _line(gt_obj_st * parent,
                          int16_t x1, int16_t y1,
                          int16_t x2, int16_t y2,
                          uint16_t width,
                          gt_color_t color,
                          uint8_t opa,
                          uint8_t brush)
{
    gt_obj_st * ln = gt_line_create(parent);
    if (NULL == ln) { return NULL; }
    gt_line_set_color(ln, color);
    gt_line_set_line_width(ln, width);
    gt_line_set_start_point(ln, (uint16_t)x1, (uint16_t)y1);
    gt_line_set_end_point(ln, (uint16_t)x2, (uint16_t)y2);
    gt_line_set_brush_type(ln, brush);
    gt_obj_set_opa(ln, opa);
    return ln;
}

static gt_obj_st * _polyline(gt_obj_st * parent, gt_point_st const * points,
                             uint16_t count, uint16_t width, gt_color_t color,
                             uint8_t opa)
{
    gt_obj_st * ln = gt_line_create(parent);
    if (NULL == ln) { return NULL; }
    gt_line_set_color(ln, color);
    gt_line_set_line_width(ln, width);
    gt_obj_set_opa(ln, opa);
    if (GT_RES_OK != gt_line_set_points(ln, points, count)) {
        gt_obj_destroy(ln);
        return NULL;
    }
    return ln;
}

static void _lbl(gt_obj_st * parent, uint16_t x, uint16_t y, char const * t)
{
    gt_obj_st * l = gt_label_create(parent);
    gt_label_set_font_size(l, 16);
    gt_obj_set_pos(l, x, y);
    gt_obj_set_size(l, (uint16_t)(796 - x), 30);
    gt_label_set_text(l, t);
}

/* ── Section 1: Horizontal & Vertical ──────────────────────────────── */
static void _sec1(gt_obj_st * scr)
{
    _lbl(scr, 4, _Y1, "[1] Horizontal & Vertical  w=1/3/6/10");

    uint16_t const y0 = _Y1 + _LH;
    uint8_t  const hw[] = {1, 3, 6, 10};
    gt_color_t const hc[] = {C_W, C_W, gt_color_hex(0x00BFFF), C_Y};

    /* Horizontal: 4 lines, 14px apart */
    for (int i = 0; i < 4; ++i) {
        uint16_t y = y0 + 4 + i * 15;
        _line(scr, 4, y, 620, y, hw[i], hc[i], 255, GT_BRUSH_TYPE_ROUND);
    }

    /* Vertical: 4 lines, spaced 40px apart */
    uint16_t const vx[] = {650, 690, 730, 775};
    for (int i = 0; i < 4; ++i) {
        _line(scr, vx[i], y0 + 2, vx[i], y0 + 60, hw[i], hc[i], 255, GT_BRUSH_TYPE_ROUND);
    }
}

/* ── Section 2: 16 angles (0~345 step 15) ──────────────────────────── */
static void _sec2(gt_obj_st * scr)
{
    _lbl(scr, 4, _Y2, "[2] 16 Angles 0~345 step 15  L=80  w=2  round");

    uint16_t const cy   = _Y2 + _LH + 90;
    uint16_t const cx_a = 100;
    uint16_t const cx_b = 500;

    static const int16_t fa_dx[] = { 80, 77, 69, 57, 40, 21,  0,-21,-40,-57,-69,-77};
    static const int16_t fa_dy[] = {  0, 21, 40, 57, 69, 77, 80, 77, 69, 57, 40, 21};
    static const uint32_t fa_col[] = {
        0xFFFFFF, 0xFF8800, 0xFFCC00, 0xFFFF00,
        0x00FF44, 0x00FF88, 0x00FFFF, 0x0088FF,
        0x0044FF, 0x8800FF, 0xCC00FF, 0xFF44AA
    };
    for (int i = 0; i < 12; ++i) {
        _line(scr, cx_a, cy,
              (int16_t)(cx_a + fa_dx[i]), (int16_t)(cy + fa_dy[i]),
              2, gt_color_hex(fa_col[i]), 255, GT_BRUSH_TYPE_ROUND);
    }

    static const int16_t fb_dx[] = {-80,-77,-69,-57,-40,-21,  0, 21, 40, 57, 69, 77};
    static const int16_t fb_dy[] = {  0,-21,-40,-57,-69,-77,-80,-77,-69,-57,-40,-21};
    static const uint32_t fb_col[] = {
        0xFFFFFF, 0xFF4488, 0xFF00FF, 0xFF0088,
        0xAA00FF, 0x4400FF, 0x0000FF, 0x0044FF,
        0x00AAFF, 0x00FF88, 0x88FF00, 0xFF4400
    };
    for (int i = 0; i < 12; ++i) {
        _line(scr, cx_b, cy,
              (int16_t)(cx_b + fb_dx[i]), (int16_t)(cy + fb_dy[i]),
              2, gt_color_hex(fb_col[i]), 255, GT_BRUSH_TYPE_ROUND);
    }
}

/* ── Section 3: Width round ─────────────────────────────────────────── */
/* Y=314, content Y=348~552
 * 7 lines, step=20px, dy=80: max_y = 348+4+6*20+80 = 552 OK */
static void _sec3(gt_obj_st * scr)
{
    _lbl(scr, 4, _Y3, "[3] Width 1/2/4/6/8/10/12  round  (white 45 | orange rev | yellow shallow | cyan steep)");

    uint8_t  const widths[] = {1, 2, 4, 6, 8, 10, 12};
    uint16_t const n = 7;
    uint16_t const y0 = _Y3 + _LH + 4;

    /* Forward 45deg, white */
    for (uint16_t i = 0; i < n; ++i) {
        uint16_t ys = y0 + i * 23;
        _line(scr, 16, ys, 84, ys + 80, widths[i], C_W, 255, GT_BRUSH_TYPE_ROUND);
    }
    /* Reverse 45deg, orange */
    for (uint16_t i = 0; i < n; ++i) {
        uint16_t ys = y0 + i * 23;
        _line(scr, 174, ys + 80, 254, ys, widths[i], C_O, 255, GT_BRUSH_TYPE_ROUND);
    }
    /* Shallow ~10deg, yellow */
    for (uint16_t i = 0; i < n; ++i) {
        uint16_t ys = y0 + i * 20;
        _line(scr, 290, ys, 490, ys + 20, widths[i], C_Y, 255, GT_BRUSH_TYPE_ROUND);
    }
    /* Steep ~80deg, cyan */
    for (uint16_t i = 0; i < n; ++i) {
        uint16_t xs = 520 + i * 36;
        _line(scr, xs, y0, xs + 14, y0 + 80, widths[i],
              gt_color_hex(0x00FFFF), 255, GT_BRUSH_TYPE_ROUND);
    }
}

/* ── Section 4: Width square ────────────────────────────────────────── */
/* Y=552, same geometry as sec3 */
static void _sec4(gt_obj_st * scr)
{
    _lbl(scr, 4, _Y4, "[4] Width 1/2/4/6/8/10/12  square  (cyan 45 | pink rev | magenta shallow | green steep)");

    uint8_t  const widths[] = {1, 2, 4, 6, 8, 10, 12};
    uint16_t const n = 7;
    uint16_t const y0 = _Y4 + _LH + 4;

    /* Forward 45deg, cyan */
    for (uint16_t i = 0; i < n; ++i) {
        uint16_t ys = y0 + i * 23;
        _line(scr, 16, ys, 84, ys + 80, widths[i],
              gt_color_hex(0x00FFFF), 255, GT_BRUSH_TYPE_RECT);
    }
    /* Reverse 45deg, pink */
    for (uint16_t i = 0; i < n; ++i) {
        uint16_t ys = y0 + i * 23;
        _line(scr, 174, ys + 80, 254, ys, widths[i],
              gt_color_hex(0xFF44AA), 255, GT_BRUSH_TYPE_RECT);
    }
    /* Shallow ~10deg, magenta */
    for (uint16_t i = 0; i < n; ++i) {
        uint16_t ys = y0 + i * 20;
        _line(scr, 290, ys, 490, ys + 20, widths[i],
              gt_color_hex(0xFF00FF), 255, GT_BRUSH_TYPE_RECT);
    }
    /* Steep ~80deg, green */
    for (uint16_t i = 0; i < n; ++i) {
        uint16_t xs = 520 + i * 36;
        _line(scr, xs, y0, xs + 14, y0 + 80, widths[i],
              gt_color_hex(0x44FF44), 255, GT_BRUSH_TYPE_RECT);
    }
}

/* ── Section 5: Colors + Opacity ────────────────────────────────────── */
/* Y=790, content Y=824~988
 * 6 colors, step=18px, dy=70: max_y = 824+4+5*18+70 = 988 OK */
static void _sec5(gt_obj_st * scr)
{
    _lbl(scr, 4, _Y5, "[5] Colors + OPA  w=4  45deg  round  (OPA: 255 | 128 | 64)");

    gt_color_t const palette[] = {
        C_R, C_G, C_B, C_Y,
        gt_color_hex(0x00FFFF), gt_color_hex(0xFF00FF)
    };
    uint8_t    const opas[]  = {255, 128, 64};
    uint16_t   const xs[]   = {  4, 264, 524};
    uint16_t   const xe[]   = {214, 474, 734};
    uint16_t   const y0     = _Y5 + _LH + 4;

    for (int c = 0; c < 3; ++c) {
        for (int i = 0; i < 6; ++i) {
            uint16_t ys = y0 + i * 18;
            _line(scr, xs[c], ys, xe[c], ys + 70,
                  4, palette[i], opas[c], GT_BRUSH_TYPE_ROUND);
        }
    }
}

/* ── Section 6: Near-H / Near-V AA stress ───────────────────────────── */
/* Y=988, content Y=1022~1142 */
static void _sec6(gt_obj_st * scr)
{
    _lbl(scr, 4, _Y6, "[6] Near-H / Near-V AA stress  w=1/2/4  round");

    uint16_t const y0 = _Y6 + _LH + 4;

    /* Near-horizontal: dx=200, dy=1/2/3/4/6/8 */
    static const int16_t nh_dy[] = {1, 2, 3, 4, 6, 8};
    static const uint8_t nh_w[]  = {1, 1, 2, 2, 4, 4};
    for (int i = 0; i < 6; ++i) {
        uint16_t ys = y0 + i * 16;
        _line(scr, 4, ys, 204, ys + nh_dy[i], nh_w[i], C_W, 255, GT_BRUSH_TYPE_ROUND);
    }

    /* Near-vertical: dy=80, dx=1/2/3/4/6/8 */
    static const int16_t nv_dx[] = {1, 2, 3, 4, 6, 8};
    for (int i = 0; i < 6; ++i) {
        uint16_t xs = 230 + i * 30;
        _line(scr, xs, y0, xs + nv_dx[i], y0 + 80,
              nh_w[i], C_O, 255, GT_BRUSH_TYPE_ROUND);
    }

    /* Exact 45deg reference: w=1/2/4/6/8/10 */
    static const uint8_t ref_w[] = {1, 2, 4, 6, 8, 10};
    for (int i = 0; i < 6; ++i) {
        uint16_t xs = 430 + i * 58;
        _line(scr, xs, y0, xs + 60, y0 + 60,
              ref_w[i], gt_color_hex(0x00FF88), 255, GT_BRUSH_TYPE_ROUND);
    }
}

/* ── Section 7: Special angles ──────────────────────────────────────── */
/* Y=1142, content Y=1176~1376
 * Tests near-axis and near-45 angles with w=3 round
 * Row A (near-H): 1°/3°/5°/10°  dx=200, dy computed
 * Row B (near-V): 80°/85°/87°/89°  dy=200, dx computed
 * Row C (near-45): 40°/43°/45°/47°/50°  L=100
 */
static void _sec7(gt_obj_st * scr)
{
    _lbl(scr, 4, _Y7, "[7] Special angles  w=3  round  (near-H | near-V | near-45)");

    uint16_t const y0 = _Y7 + _LH + 4;

    /* Row A: near-horizontal  1°/3°/5°/10°/15°/20°
     * dx=200, dy = round(200 * tan(angle_deg * pi/180))
     * 1°→3  3°→10  5°→17  10°→35  15°→54  20°→73 */
    static const int16_t ra_dy[] = {3, 10, 17, 35, 54, 73};
    static const uint32_t ra_col[] = {
        0xFF4444, 0xFF8844, 0xFFCC44, 0xFFFF44, 0x88FF44, 0x44FF88
    };
    for (int i = 0; i < 6; ++i) {
        uint16_t xs = 4 + i * 130;
        uint16_t ys = y0 + 4;
        _line(scr, xs, ys, xs + 120, ys + ra_dy[i],
              3, gt_color_hex(ra_col[i]), 255, GT_BRUSH_TYPE_ROUND);
    }

    /* Row B: near-vertical  70°/75°/80°/85°/87°/89°
     * dy=100, dx = round(100 * tan((90-angle)*pi/180))
     * 70°→36  75°→27  80°→18  85°→9  87°→5  89°→2 */
    static const int16_t rb_dx[] = {36, 27, 18, 9, 5, 2};
    static const uint32_t rb_col[] = {
        0x44FFFF, 0x44AAFF, 0x4466FF, 0x8844FF, 0xCC44FF, 0xFF44FF
    };
    for (int i = 0; i < 6; ++i) {
        uint16_t xs = 4 + i * 130;
        uint16_t ys = y0 + 60;
        _line(scr, xs, ys, xs + rb_dx[i], ys + 100,
              3, gt_color_hex(rb_col[i]), 255, GT_BRUSH_TYPE_ROUND);
    }

    /* Row C: near-45  40°/43°/45°/47°/50°/55°  L=100
     * dx = round(100*cos(a)), dy = round(100*sin(a))
     * 40°→(77,64)  43°→(73,68)  45°→(71,71)  47°→(68,73)  50°→(64,77)  55°→(57,82) */
    static const int16_t rc_dx[] = {77, 73, 71, 68, 64, 57};
    static const int16_t rc_dy[] = {64, 68, 71, 73, 77, 82};
    static const uint32_t rc_col[] = {
        0xFF8800, 0xFFCC00, 0xFFFFFF, 0x00FFCC, 0x00AAFF, 0x8800FF
    };
    for (int i = 0; i < 6; ++i) {
        uint16_t xs = 4 + i * 130;
        uint16_t ys = y0 + 180;
        _line(scr, xs, ys, xs + rc_dx[i], ys + rc_dy[i],
              3, gt_color_hex(rc_col[i]), 255, GT_BRUSH_TYPE_ROUND);
    }
}

static void _sec8(gt_obj_st * scr)
{
    _lbl(scr, 4, _Y8, "[8] Dash / Dot lines  (H / V / 45deg / dot)");

    uint16_t const y0 = _Y8 + _LH + 4;

    /* Row A: horizontal dashes — dash=10 gap=5, dash=6 gap=3, dash=2 gap=2 */
    _lbl(scr, 4, y0, "H dash");
    gt_obj_st * ln;
    ln = _line(scr, 80, y0 + 2, 380, y0 + 2, 2, C_R, 255, GT_BRUSH_TYPE_RECT);
    gt_line_set_dash(ln, 10, 5);
    ln = _line(scr, 80, y0 + 14, 380, y0 + 14, 2, C_O, 255, GT_BRUSH_TYPE_RECT);
    gt_line_set_dash(ln, 6, 3);
    ln = _line(scr, 80, y0 + 26, 380, y0 + 26, 2, C_Y, 255, GT_BRUSH_TYPE_RECT);
    gt_line_set_dash(ln, 2, 2);

    /* Row B: vertical dashes */
    _lbl(scr, 400, y0, "V dash");
    ln = _line(scr, 480, y0 + 2, 480, y0 + 80, 2, C_G, 255, GT_BRUSH_TYPE_RECT);
    gt_line_set_dash(ln, 8, 4);
    ln = _line(scr, 510, y0 + 2, 510, y0 + 80, 2, C_B, 255, GT_BRUSH_TYPE_RECT);
    gt_line_set_dash(ln, 4, 4);
    ln = _line(scr, 540, y0 + 2, 540, y0 + 80, 3, C_W, 255, GT_BRUSH_TYPE_RECT);
    gt_line_set_dash(ln, 2, 2);

    /* Row C: diagonal dashes (45°) */
    _lbl(scr, 4, y0 + 50, "Diag dash");
    ln = _line(scr, 80, y0 + 80, 230, y0 + 230, 2, C_R, 255, GT_BRUSH_TYPE_RECT);
    gt_line_set_dash(ln, 12, 6);
    ln = _line(scr, 250, y0 + 80, 400, y0 + 230, 2, C_G, 255, GT_BRUSH_TYPE_RECT);
    gt_line_set_dash(ln, 6, 6);

    /* Row D: dot lines (dash=1) */
    _lbl(scr, 4, y0 + 150, "Dot");
    ln = _line(scr, 80, y0 + 180, 380, y0 + 180, 3, C_W, 255, GT_BRUSH_TYPE_RECT);
    gt_line_set_dash(ln, 1, 3);
    ln = _line(scr, 80, y0 + 196, 380, y0 + 196, 3, C_Y, 255, GT_BRUSH_TYPE_RECT);
    gt_line_set_dash(ln, 1, 5);

    /* Row E: solid reference (dash=0, should look identical to original) */
    _lbl(scr, 400, y0 + 150, "Solid ref");
    ln = _line(scr, 480, y0 + 180, 780, y0 + 180, 2, C_W, 255, GT_BRUSH_TYPE_RECT);
    /* dash=0 → solid, backward-compat check */

    /* Row F: diagonal dot lines */
    _lbl(scr, 4, y0 + 220, "Diag dot");
    ln = _line(scr, 80, y0 + 250, 230, y0 + 400, 3, C_W, 255, GT_BRUSH_TYPE_RECT);
    gt_line_set_dash(ln, 1, 3);
    ln = _line(scr, 260, y0 + 250, 410, y0 + 400, 3, C_Y, 255, GT_BRUSH_TYPE_RECT);
    gt_line_set_dash(ln, 1, 5);
    ln = _line(scr, 440, y0 + 250, 590, y0 + 400, 3, gt_color_hex(0x00FFFF), 255, GT_BRUSH_TYPE_RECT);
    gt_line_set_dash(ln, 1, 8);
}

/* ── Section 9: translucent polyline joints ────────────────────────── */
static void _sec9(gt_obj_st * scr)
{
    static gt_point_st const zigzag[] = {
        {  40, _Y9 + 100 }, { 190, _Y9 +  45 }, { 340, _Y9 + 100 },
        { 490, _Y9 +  45 }, { 640, _Y9 + 100 }, { 780, _Y9 +  45 },
    };
    static gt_point_st const sharp_turn[] = {
        {  40, _Y9 + 230 }, { 220, _Y9 + 230 }, { 220, _Y9 + 150 },
        { 400, _Y9 + 150 }, { 400, _Y9 + 230 }, { 580, _Y9 + 230 },
        { 580, _Y9 + 150 }, { 760, _Y9 + 150 },
    };

    _lbl(scr, 4, _Y9, "[9] Polyline joints: 50% OPA, no darker overlap at vertices");
    _lbl(scr, 4, _Y9 + 28, "round vertices are drawn once; compare joint color with segment body");
    _polyline(scr, zigzag, sizeof(zigzag) / sizeof(zigzag[0]),
              16, gt_color_hex(0x00BFFF), GT_OPA_50);
    _polyline(scr, sharp_turn, sizeof(sharp_turn) / sizeof(sharp_turn[0]),
              16, gt_color_hex(0xFF8800), GT_OPA_50);
}



/**
 * @brief Line widget comprehensive test screen.
 *
 * 7 sections (scrollable, total ~1376px):
 *   [1] Horizontal & Vertical — w=1/3/6/10
 *   [2] 16 Angles 0~345 step 15 — two fans, L=80, w=2
 *   [3] Width 1~12 round — 45deg fwd/rev + shallow + steep
 *   [4] Width 1~12 square — same layout as [3]
 *   [5] Colors + OPA — 6 colors x 3 opacity levels
 *   [6] Near-H / Near-V AA stress — tiny angle increments
 *   [7] Special angles — near-H/V/45 with w=3
 *   [8] Dash / Dot lines — H/V/diagonal/dot styles
 *   [9] Polyline joints — translucent shared-vertex overlap check
 */
gt_obj_st * gt_examples_line(void)
{
    gt_obj_st * scr = gt_obj_create(NULL);
    gt_screen_set_bgcolor(scr, gt_color_gray());

    _sec1(scr);
    _sec2(scr);
    _sec3(scr);
    _sec4(scr);
    _sec5(scr);
    _sec6(scr);
    _sec7(scr);
    _sec8(scr);
    _sec9(scr);

    GT_EXAMPLE_ADD_BACK_BTN(scr);
    return scr;
}

#endif
/* end ------------------------------------------------------------------*/
