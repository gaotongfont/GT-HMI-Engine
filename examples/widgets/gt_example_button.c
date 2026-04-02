/**
 * @file gt_example_button.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-09-28 11:41:32
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "string.h"
#include "../../src/core/gt_event.h"
#include "../../src/widgets/gt_btn.h"
#include "../../src/widgets/gt_label.h"


/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void btn_click_cb(gt_event_st * e)
{
	static int cnt_idx = 0;
	char buffer[100];

	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "btn %d", ++cnt_idx);
	printf("%s\n", buffer);
	gt_btn_set_text(e->target, buffer);
}

/** @brief 布局辅助：在 scr 上创建一个带标签的按钮行 */
static gt_obj_st * _make_btn(gt_obj_st * scr, int x, int y, int w, int h, const char * label)
{
    gt_obj_st * btn = gt_btn_create(scr);
    gt_obj_set_pos(btn, x, y);
    gt_obj_set_size(btn, w, h);
    gt_btn_set_text(btn, label);
    return btn;
}


/* global functions / API interface -------------------------------------*/

/**
 * @brief Button corner-radius test:
 *   Row 1 — full radius (all 4 corners)
 *   Row 2 — single corner enabled (TL / TR / BL / BR)
 *   Row 3 — adjacent pairs (TL+TR top-cap / BL+BR bottom-cap / TL+BL left-cap / TR+BR right-cap)
 *   Row 4 — diagonal pairs (TL+BR / TR+BL)
 *   Row 5 — three corners (all except one)
 *   Row 6 — no radius (square)
 *   Row 7 — original click-count demo
 */
gt_obj_st * gt_examples_button(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    int bw = 80, bh = 36, gap = 8;
    int x0 = 10, y = 10;

    /* --- Row 1: all corners --- */
    gt_obj_st * b = _make_btn(screen, x0, y, bw, bh, "All");
    gt_btn_set_radius(b, 10);
    gt_btn_set_corner_radius(b, true, true, true, true);

    /* --- Row 2: single corner --- */
    y += bh + gap;
    const char * single_lbl[4] = { "TL", "TR", "BL", "BR" };
    bool single_tl[4] = { true,  false, false, false };
    bool single_tr[4] = { false, true,  false, false };
    bool single_bl[4] = { false, false, true,  false };
    bool single_br[4] = { false, false, false, true  };
    for (int i = 0; i < 4; ++i) {
        b = _make_btn(screen, x0 + i * (bw + gap), y, bw, bh, single_lbl[i]);
        gt_btn_set_radius(b, 10);
        gt_btn_set_corner_radius(b, single_tl[i], single_tr[i], single_bl[i], single_br[i]);
    }

    /* --- Row 3: adjacent pairs --- */
    y += bh + gap;
    const char * pair_lbl[4] = { "TL+TR", "BL+BR", "TL+BL", "TR+BR" };
    bool pair_tl[4] = { true,  false, true,  false };
    bool pair_tr[4] = { true,  false, false, true  };
    bool pair_bl[4] = { false, true,  true,  false };
    bool pair_br[4] = { false, true,  false, true  };
    for (int i = 0; i < 4; ++i) {
        b = _make_btn(screen, x0 + i * (bw + gap), y, bw, bh, pair_lbl[i]);
        gt_btn_set_radius(b, 10);
        gt_btn_set_corner_radius(b, pair_tl[i], pair_tr[i], pair_bl[i], pair_br[i]);
    }

    /* --- Row 4: diagonal pairs --- */
    y += bh + gap;
    b = _make_btn(screen, x0, y, bw, bh, "TL+BR");
    gt_btn_set_radius(b, 10);
    gt_btn_set_corner_radius(b, true, false, false, true);

    b = _make_btn(screen, x0 + bw + gap, y, bw, bh, "TR+BL");
    gt_btn_set_radius(b, 10);
    gt_btn_set_corner_radius(b, false, true, true, false);

    /* --- Row 5: three corners --- */
    y += bh + gap;
    const char * tri_lbl[4] = { "!TL", "!TR", "!BL", "!BR" };
    bool tri_tl[4] = { false, true,  true,  true  };
    bool tri_tr[4] = { true,  false, true,  true  };
    bool tri_bl[4] = { true,  true,  false, true  };
    bool tri_br[4] = { true,  true,  true,  false };
    for (int i = 0; i < 4; ++i) {
        b = _make_btn(screen, x0 + i * (bw + gap), y, bw, bh, tri_lbl[i]);
        gt_btn_set_radius(b, 10);
        gt_btn_set_corner_radius(b, tri_tl[i], tri_tr[i], tri_bl[i], tri_br[i]);
    }

    /* --- Row 6: no radius (square) --- */
    y += bh + gap;
    b = _make_btn(screen, x0, y, bw, bh, "Square");
    gt_btn_set_corner_radius(b, false, false, false, false);

    /* --- Row 7: original click-count demo --- */
    y += bh + gap;
    b = _make_btn(screen, x0, y, bw, bh, "Click");
    gt_btn_set_radius(b, 10);
    gt_btn_set_corner_radius(b, true, true, true, true);
    gt_obj_add_event_cb(b, btn_click_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    /* --- original demo (preserved) --- */
    gt_obj_st * btn = gt_btn_create(screen);
    gt_effect_click_register(btn);
    gt_obj_set_pos(btn, 400, 30);
    gt_obj_set_size(btn, 80, 40);
    gt_btn_set_radius(btn, 20);
    gt_btn_set_corner_radius(btn, true, false, false, true);
    gt_obj_add_event_cb(btn, btn_click_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    gt_obj_st * btn1 = gt_btn_create(screen);
    gt_effect_click_register(btn1);
    gt_obj_set_pos(btn1, 400, 130);
    gt_obj_set_size(btn1, 80, 40);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
