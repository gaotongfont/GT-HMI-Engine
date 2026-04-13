/**
 * @file gt_example_view_pager.c
 * @author Feyoung
 * @brief View pager widget example
 * @version 0.2
 * @date 2026-04-12
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_view_pager.h"
#include "../../src/widgets/gt_label.h"
#include "../../src/widgets/gt_btn.h"
#include "../../src/widgets/gt_rect.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/** Custom window size shared by all cases */
#define _VP_W   300
#define _VP_H   160
#define _VP_VW  160
#define _VP_VH  200

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/
static const gt_color_t _page_colors[3] = {
    /* page 0: steel blue, page 1: olive, page 2: coral */
    { .full = 0x4682 },   /* RGB565 steel-blue  */
    { .full = 0x8C40 },   /* RGB565 olive       */
    { .full = 0xFB8C },   /* RGB565 coral       */
};

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/

/**
 * @brief Add a colored btn + label pair into one fragment of a view_pager.
 *
 * @param vp        view_pager object
 * @param scr       parent screen (btn/label are created on screen first)
 * @param frag_idx  fragment index (0-based)
 * @param label_txt text shown on the label
 * @param btn_txt   text shown on the button
 * @param color     background color of the button
 */
static void _add_page(gt_obj_st * vp, gt_obj_st * scr,
                      uint8_t frag_idx, const char * label_txt,
                      const char * btn_txt, gt_color_t color)
{
    /* label: centered in the visible window */
    gt_obj_st * lbl = gt_label_create(scr);
    gt_label_set_text(lbl, label_txt);
    gt_obj_set_pos(lbl, 4, 4);
    gt_view_pager_fragment_add_widget(vp, frag_idx, lbl);

    /* button: below the label */
    gt_obj_st * btn = gt_btn_create(scr);
    gt_btn_set_text(btn, btn_txt);
    gt_obj_set_size(btn, 80, 30);
    gt_obj_set_pos(btn, 4, 28);
    gt_btn_set_color_background(btn, color);
    gt_view_pager_fragment_add_widget(vp, frag_idx, btn);
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Four view_pager test cases on one screen:
 *
 *  Case A (top-left)    : horizontal, 300×160 window, tip bar BOTTOM (default)
 *  Case B (top-right)   : horizontal, 300×160 window, tip bar TOP
 *  Case C (bottom-left) : vertical,   160×200 window, tip bar RIGHT  (default)
 *  Case D (bottom-right): vertical,   160×200 window, tip bar LEFT
 *
 *  Each case has 3 pages with a label + colored button per page.
 */
gt_obj_st * gt_examples_view_pager(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    /* ------------------------------------------------------------------ */
    /* Case A: horizontal, tip bar BOTTOM (default)                        */
    /* ------------------------------------------------------------------ */
    gt_obj_st * lbl_a = gt_label_create(screen);
    gt_label_set_text(lbl_a, "A: Hor / Tip-Bottom");
    gt_obj_set_pos(lbl_a, 10, 4);
    gt_obj_set_size(lbl_a, 200, 30);

    gt_obj_st * vp_a = gt_view_pager_create(screen);
    gt_obj_set_pos(vp_a, 10, 24);
    gt_view_pager_set_window_size(vp_a, _VP_W, _VP_H);
    gt_view_pager_set_fragment_count(vp_a, 3);
    gt_view_pager_set_glass_color(vp_a, gt_color_make(0xFF, 0xE0, 0x00));   /* yellow */
    /* direction: horizontal (default), tip bar: bottom (default) */

    _add_page(vp_a, screen, 0, "A-Page1", "Btn A1", _page_colors[0]);
    _add_page(vp_a, screen, 1, "A-Page2", "Btn A2", _page_colors[1]);
    _add_page(vp_a, screen, 2, "A-Page3", "Btn A3", _page_colors[2]);

    /* ------------------------------------------------------------------ */
    /* Case B: horizontal, tip bar TOP                                     */
    /* ------------------------------------------------------------------ */
    gt_obj_st * lbl_b = gt_label_create(screen);
    gt_label_set_text(lbl_b, "B: Hor / Tip-Top");
    gt_obj_set_pos(lbl_b, 420, 4);
    gt_obj_set_size(lbl_b, 200, 30);

    gt_obj_st * vp_b = gt_view_pager_create(screen);
    gt_obj_set_pos(vp_b, 420, 24);
    gt_view_pager_set_window_size(vp_b, _VP_W, _VP_H);
    gt_view_pager_set_fragment_count(vp_b, 3);
    gt_view_pager_set_glass_color(vp_b, gt_color_make(0xFF, 0xE0, 0x00));   /* yellow */
    gt_view_pager_set_tip_bar_pos(vp_b, GT_VIEW_PAGER_TIP_BAR_POS_OPPOSITE);

    _add_page(vp_b, screen, 0, "B-Page1", "Btn B1", _page_colors[0]);
    _add_page(vp_b, screen, 1, "B-Page2", "Btn B2", _page_colors[1]);
    _add_page(vp_b, screen, 2, "B-Page3", "Btn B3", _page_colors[2]);

    /* ------------------------------------------------------------------ */
    /* Case C: vertical, tip bar RIGHT (default)                           */
    /* ------------------------------------------------------------------ */
    gt_obj_st * lbl_c = gt_label_create(screen);
    gt_label_set_text(lbl_c, "C: Ver / Tip-Right");
    gt_obj_set_pos(lbl_c, 10, 200);
    gt_obj_set_size(lbl_c, 200, 30);

    gt_obj_st * vp_c = gt_view_pager_create(screen);
    gt_obj_set_pos(vp_c, 10, 220);
    gt_view_pager_set_window_size(vp_c, _VP_VW, _VP_VH);
    gt_view_pager_set_direction(vp_c, GT_VIEW_PAGER_DIR_VERTICAL);
    gt_view_pager_set_fragment_count(vp_c, 3);
    gt_view_pager_set_glass_color(vp_c, gt_color_make(0xFF, 0xE0, 0x00));   /* yellow */
    /* tip bar: right (default for vertical) */

    _add_page(vp_c, screen, 0, "C-Page1", "Btn C1", _page_colors[0]);
    _add_page(vp_c, screen, 1, "C-Page2", "Btn C2", _page_colors[1]);
    _add_page(vp_c, screen, 2, "C-Page3", "Btn C3", _page_colors[2]);

    /* ------------------------------------------------------------------ */
    /* Case D: vertical, tip bar LEFT                                      */
    /* ------------------------------------------------------------------ */
    gt_obj_st * lbl_d = gt_label_create(screen);
    gt_label_set_text(lbl_d, "D: Ver / Tip-Left");
    gt_obj_set_pos(lbl_d, 220, 200);
    gt_obj_set_size(lbl_d, 200, 30);

    gt_obj_st * vp_d = gt_view_pager_create(screen);
    gt_obj_set_pos(vp_d, 220, 220);
    gt_view_pager_set_window_size(vp_d, _VP_VW, _VP_VH);
    gt_view_pager_set_direction(vp_d, GT_VIEW_PAGER_DIR_VERTICAL);
    gt_view_pager_set_fragment_count(vp_d, 3);
    gt_view_pager_set_glass_color(vp_d, gt_color_make(0xFF, 0xE0, 0x00));   /* yellow */
    gt_view_pager_set_tip_bar_pos(vp_d, GT_VIEW_PAGER_TIP_BAR_POS_OPPOSITE);

    _add_page(vp_d, screen, 0, "D-Page1", "Btn D1", _page_colors[0]);
    _add_page(vp_d, screen, 1, "D-Page2", "Btn D2", _page_colors[1]);
    _add_page(vp_d, screen, 2, "D-Page3", "Btn D3", _page_colors[2]);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
