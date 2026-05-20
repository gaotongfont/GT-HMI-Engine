/**
 * @file gt_example_polygon.c
 * @author Feyoung
 * @brief Polygon widget example: fill, stroke, concave shape, opacity, touch hit-test
 * @version 0.1
 * @date 2026-05-19
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_polygon.h"
#include "../../src/widgets/gt_label.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

#if GT_CFG_ENABLE_POLYGON

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/

static void _polygon_click_cb(gt_event_st * e)
{
    gt_obj_st * poly = (gt_obj_st *)e->target;
    /* Cycle through 4 fill colours on each click */
    static const uint32_t _colors[] = {
        0xE53935, /* red   */
        0x43A047, /* green */
        0x1E88E5, /* blue  */
        0xFB8C00, /* orange*/
    };
    static uint8_t _idx = 0;
    _idx = (_idx + 1) & 3u;
    gt_polygon_set_fill_color(poly, gt_color_hex(_colors[_idx]));
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Polygon widget example screen.
 *
 *  Section layout (800 screen, total height ~1400px, scrollable):
 *  ┌─────────────────────────────────────────────────────┐
 *  │ Section 1 (y 10–230)   : Basic shapes – fill only  │
 *  │   triangle  pentagon  diamond  hexagon              │
 *  │ Section 2 (y 280–510)  : Stroke only (no fill)     │
 *  │   triangle  square  pentagon  arrow outline         │
 *  │ Section 3 (y 560–820)  : Fill + stroke combo       │
 *  │   hexagon   5-pointed star  concave arrow           │
 *  │ Section 4 (y 870–1060) : Opacity variants          │
 *  │   100% / 60% / 30% / stroke-only 80%               │
 *  │ Section 5 (y 1110–1350): Touch interaction         │
 *  │   click inside pentagon to change colour            │
 *  └─────────────────────────────────────────────────────┘
 */
gt_obj_st * gt_examples_polygon(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    /* ------------------------------------------------------------------ */
    /* Section label helper macro                                          */
    /* ------------------------------------------------------------------ */
#define _SEC_LABEL(txt_, x_, y_) \
    do { \
        gt_obj_st * _lb = gt_label_create(screen); \
        gt_obj_set_pos(_lb, (x_), (y_)); \
        gt_obj_set_size(_lb, 780, 24); \
        gt_label_set_text(_lb, (txt_)); \
    } while (0)

    /* ================================================================== */
    /* Section 1: Basic fill shapes  (y: 10 – 230)                        */
    /* ================================================================== */
    _SEC_LABEL("Section 1: Fill only", 10, 10);

    /* Triangle */
    {
        gt_point_st pts[] = { {80, 220}, {30, 80}, {130, 80} };
        gt_obj_st * p = gt_polygon_create(screen);
        gt_polygon_set_points(p, pts, 3);
        gt_polygon_set_fill_color(p, gt_color_hex(0xE53935));
    }

    /* Pentagon – centred at (260, 155), radius 75 */
    {
        gt_point_st pts[] = {
            {260,  80}, {331, 132}, {305, 218},
            {215, 218}, {189, 132},
        };
        gt_obj_st * p = gt_polygon_create(screen);
        gt_polygon_set_points(p, pts, 5);
        gt_polygon_set_fill_color(p, gt_color_hex(0x43A047));
    }

    /* Diamond */
    {
        gt_point_st pts[] = { {460, 80}, {520, 150}, {460, 220}, {400, 150} };
        gt_obj_st * p = gt_polygon_create(screen);
        gt_polygon_set_points(p, pts, 4);
        gt_polygon_set_fill_color(p, gt_color_hex(0x1E88E5));
    }

    /* Regular hexagon – centred at (650, 150), radius 70 */
    {
        gt_point_st pts[] = {
            {650,  80}, {711, 115}, {711, 185},
            {650, 220}, {589, 185}, {589, 115},
        };
        gt_obj_st * p = gt_polygon_create(screen);
        gt_polygon_set_points(p, pts, 6);
        gt_polygon_set_fill_color(p, gt_color_hex(0xFB8C00));
    }

    /* ================================================================== */
    /* Section 2: Stroke only  (y: 280 – 510)                             */
    /* ================================================================== */
    _SEC_LABEL("Section 2: Stroke only (no fill)", 10, 280);

    /* Triangle outline */
    {
        gt_point_st pts[] = { {80, 500}, {30, 340}, {130, 340} };
        gt_obj_st * p = gt_polygon_create(screen);
        gt_polygon_set_points(p, pts, 3);
        gt_polygon_set_fill_opa(p, GT_OPA_TRANSP);
        gt_polygon_set_line_color(p, gt_color_hex(0xE53935));
        gt_polygon_set_line_width(p, 3);
    }

    /* Square outline */
    {
        gt_point_st pts[] = { {200, 340}, {320, 340}, {320, 500}, {200, 500} };
        gt_obj_st * p = gt_polygon_create(screen);
        gt_polygon_set_points(p, pts, 4);
        gt_polygon_set_fill_opa(p, GT_OPA_TRANSP);
        gt_polygon_set_line_color(p, gt_color_hex(0x43A047));
        gt_polygon_set_line_width(p, 2);
    }

    /* Pentagon outline */
    {
        gt_point_st pts[] = {
            {460, 340}, {524, 386}, {500, 464},
            {420, 464}, {396, 386},
        };
        gt_obj_st * p = gt_polygon_create(screen);
        gt_polygon_set_points(p, pts, 5);
        gt_polygon_set_fill_opa(p, GT_OPA_TRANSP);
        gt_polygon_set_line_color(p, gt_color_hex(0x1E88E5));
        gt_polygon_set_line_width(p, 4);
    }

    /* Arrow / chevron outline */
    {
        gt_point_st pts[] = {
            {580, 390}, {640, 340}, {700, 390},
            {700, 430}, {640, 380}, {580, 430},
        };
        gt_obj_st * p = gt_polygon_create(screen);
        gt_polygon_set_points(p, pts, 6);
        gt_polygon_set_fill_opa(p, GT_OPA_TRANSP);
        gt_polygon_set_line_color(p, gt_color_hex(0xAB47BC));
        gt_polygon_set_line_width(p, 3);
    }

    /* ================================================================== */
    /* Section 3: Fill + stroke combo  (y: 560 – 820)                     */
    /* ================================================================== */
    _SEC_LABEL("Section 3: Fill + stroke", 10, 560);

    /* Hexagon with stroke */
    {
        gt_point_st pts[] = {
            {100, 600}, {160, 635}, {160, 705},
            {100, 740}, { 40, 705}, { 40, 635},
        };
        gt_obj_st * p = gt_polygon_create(screen);
        gt_polygon_set_points(p, pts, 6);
        gt_polygon_set_fill_color(p, gt_color_hex(0x26C6DA));
        gt_polygon_set_line_color(p, gt_color_hex(0x00838F));
        gt_polygon_set_line_width(p, 4);
    }

    /* 5-pointed star (concave polygon, odd-even fill rule) */
    /* Outer radius 80, inner radius 33, centre (320, 690) */
    {
        gt_point_st pts[] = {
            {320, 610}, {335, 660}, {400, 660},
            {347, 693}, {367, 750}, {320, 718},
            {273, 750}, {293, 693}, {240, 660},
            {305, 660},
        };
        gt_obj_st * p = gt_polygon_create(screen);
        gt_polygon_set_points(p, pts, 10);
        gt_polygon_set_fill_color(p, gt_color_hex(0xFFD600));
        gt_polygon_set_line_color(p, gt_color_hex(0xF9A825));
        gt_polygon_set_line_width(p, 2);
    }

    /* Irregular concave arrow shape */
    {
        gt_point_st pts[] = {
            {460, 610}, {580, 610}, {580, 650},
            {640, 690}, {580, 730}, {580, 770},
            {460, 770},
        };
        gt_obj_st * p = gt_polygon_create(screen);
        gt_polygon_set_points(p, pts, 7);
        gt_polygon_set_fill_color(p, gt_color_hex(0xEF5350));
        gt_polygon_set_line_color(p, gt_color_hex(0xB71C1C));
        gt_polygon_set_line_width(p, 3);
    }

    /* ================================================================== */
    /* Section 4: Opacity variants  (y: 870 – 1060)                       */
    /* ================================================================== */
    _SEC_LABEL("Section 4: Opacity variants  (100% / 60% / 30% / stroke-only 80%)", 10, 870);

    /* Fully opaque */
    {
        gt_point_st pts[] = { {100, 1040}, {40,  900}, {160,  900} };
        gt_obj_st * p = gt_polygon_create(screen);
        gt_polygon_set_points(p, pts, 3);
        gt_polygon_set_fill_color(p, gt_color_hex(0x1E88E5));
        gt_polygon_set_fill_opa(p, GT_OPA_COVER);
        gt_polygon_set_line_color(p, gt_color_hex(0x0D47A1));
        gt_polygon_set_line_width(p, 2);
    }

    /* Semi-transparent (60%) */
    {
        gt_point_st pts[] = { {300, 1040}, {240, 900}, {360, 900} };
        gt_obj_st * p = gt_polygon_create(screen);
        gt_polygon_set_points(p, pts, 3);
        gt_polygon_set_fill_color(p, gt_color_hex(0x1E88E5));
        gt_polygon_set_fill_opa(p, GT_OPA_60);
        gt_polygon_set_line_color(p, gt_color_hex(0x0D47A1));
        gt_polygon_set_line_width(p, 2);
        gt_polygon_set_line_opa(p, GT_OPA_60);
    }

    /* Low opacity (30%) */
    {
        gt_point_st pts[] = { {500, 1040}, {440, 900}, {560, 900} };
        gt_obj_st * p = gt_polygon_create(screen);
        gt_polygon_set_points(p, pts, 3);
        gt_polygon_set_fill_color(p, gt_color_hex(0x1E88E5));
        gt_polygon_set_fill_opa(p, GT_OPA_30);
        gt_polygon_set_line_color(p, gt_color_hex(0x0D47A1));
        gt_polygon_set_line_width(p, 2);
        gt_polygon_set_line_opa(p, GT_OPA_30);
    }

    /* Fill transparent, stroke only at 80% */
    {
        gt_point_st pts[] = { {700, 1040}, {640, 900}, {760, 900} };
        gt_obj_st * p = gt_polygon_create(screen);
        gt_polygon_set_points(p, pts, 3);
        gt_polygon_set_fill_opa(p, GT_OPA_TRANSP);
        gt_polygon_set_line_color(p, gt_color_hex(0x1E88E5));
        gt_polygon_set_line_width(p, 3);
        gt_polygon_set_line_opa(p, GT_OPA_80);
    }

    /* ================================================================== */
    /* Section 5: Touch interaction  (y: 1110 – 1380)                     */
    /* Click inside the pentagon to cycle fill colour;                     */
    /* clicking outside the bounding box (but inside widget area) = no-op */
    /* ================================================================== */
    _SEC_LABEL("Section 5: Click INSIDE the pentagon to change colour", 10, 1110);

    {
        /* Pentagon centred at (400, 1270), radius 120 */
        gt_point_st pts[] = {
            {400, 1150}, {514, 1233}, {472, 1370},
            {328, 1370}, {286, 1233},
        };
        gt_obj_st * p = gt_polygon_create(screen);
        gt_polygon_set_points(p, pts, 5);
        gt_polygon_set_fill_color(p, gt_color_hex(0xE53935));
        gt_polygon_set_line_color(p, gt_color_hex(0xB71C1C));
        gt_polygon_set_line_width(p, 3);
        gt_obj_add_event_cb(p, _polygon_click_cb,
                            GT_EVENT_TYPE_INPUT_RELEASED, NULL);
    }

#undef _SEC_LABEL

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif  /** GT_CFG_ENABLE_POLYGON */

#endif  /* GT_USE_EXAMPLES */

/* end ------------------------------------------------------------------*/
