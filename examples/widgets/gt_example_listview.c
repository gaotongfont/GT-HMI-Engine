/**
 * @file gt_example_listview.c
 * @author Feyoung
 * @brief listview delete API test: single column, multi-column, horizontal
 * @version 0.1
 * @date 2022-10-10 14:37:42
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_listview.h"
#include "../../src/widgets/gt_label.h"


/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* ---- vertical sections (top half) ---- */
#define _SECTION_W      390
#define _LV_X_PAD       5
#define _LV_Y_START     32
#define _LV_H           190
#define _BTN_H          32
#define _BTN_ROW1_Y     228
#define _BTN_ROW2_Y     265
#define _BTN_W3         118   /* width when 3 buttons per row */
#define _BTN_W2         183   /* width when 2 buttons per row */
#define _BTN_GAP        5

/* ---- horizontal section (bottom strip) ---- */
#define _HOR_TITLE_Y    302
#define _HOR_Y_START    322
#define _HOR_H          80
#define _HOR_ITEM_W     100
#define _HOR_BTN_Y      408
#define _HOR_BTN_W2     193


/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/

static uint16_t _s_add_cnt = 0;


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/

static void _del_last_cb(gt_event_st * e)
{
    gt_obj_st * lv = (gt_obj_st *)e->user_data;
    gt_size_t cnt = gt_listview_get_list_items_cnt(lv);
    if (cnt > 0) {
        gt_listview_del_item(lv, cnt - 1);
    }
}

static void _del_first_cb(gt_event_st * e)
{
    gt_obj_st * lv = (gt_obj_st *)e->user_data;
    gt_listview_del_item(lv, 0);
}

static void _del_mid_cb(gt_event_st * e)
{
    gt_obj_st * lv = (gt_obj_st *)e->user_data;
    gt_size_t cnt = gt_listview_get_list_items_cnt(lv);
    if (cnt > 0) {
        gt_listview_del_item(lv, cnt / 2);
    }
}

static void _clear_cb(gt_event_st * e)
{
    gt_obj_st * lv = (gt_obj_st *)e->user_data;
    gt_listview_clear_all_items(lv);
}

static void _add_cb(gt_event_st * e)
{
    gt_obj_st * lv = (gt_obj_st *)e->user_data;
    char buf[16];
    snprintf(buf, sizeof(buf), "new %d", ++_s_add_cnt);
    gt_listview_add_item(lv, buf);
}

static void _selected_cb(gt_event_st * e)
{
    gt_obj_st * lv = e->target;
    printf("selected idx=%d text=%s\n",
           (int)gt_listview_get_idx_selected(lv),
           gt_listview_get_selected_item_text(lv));
}


/* static functions -----------------------------------------------------*/

static gt_obj_st * _make_btn(gt_obj_st * parent, int x, int y, int w,
                              const char * text, gt_event_cb_t cb, gt_obj_st * lv)
{
    gt_obj_st * btn = gt_btn_create(parent);
    gt_obj_set_pos(btn, x, y);
    gt_obj_set_size(btn, w, _BTN_H);
    gt_btn_set_text(btn, text);
    gt_obj_add_event_cb(btn, cb, GT_EVENT_TYPE_INPUT_RELEASED, lv);
    return btn;
}

/**
 * @brief Build one vertical test section: title + listview + control buttons.
 *
 * @param screen     parent screen
 * @param sx         section left edge (absolute x)
 * @param title      section title text
 * @param col_count  1 = single column; >1 = multi-column
 * @param item_count initial item count
 */
static gt_obj_st * _make_section(gt_obj_st * screen, int sx,
                                  const char * title, int col_count, int item_count)
{
    /* title label */
    gt_obj_st * lbl = gt_label_create(screen);
    gt_obj_set_pos(lbl, sx + _LV_X_PAD, 5);
    gt_obj_set_size(lbl, _SECTION_W - _LV_X_PAD * 2, 22);
    gt_label_set_text(lbl, title);
    gt_label_set_font_size(lbl, 14);

    /* listview */
    gt_obj_st * lv = gt_listview_create(screen);
    gt_obj_set_pos(lv, sx + _LV_X_PAD, _LV_Y_START);
    gt_obj_set_size(lv, _SECTION_W - _LV_X_PAD * 2, _LV_H);
    gt_listview_set_item_height(lv, col_count > 1 ? 72 : 38);
    gt_listview_set_septal_line(lv, true);

    if (col_count > 1) {
        gt_listview_set_next_row_item_count(lv, col_count);
    }

    char buf[16];
    for (int i = 0; i < item_count; i++) {
        snprintf(buf, sizeof(buf), "item %d", i);
        gt_listview_add_item(lv, buf);
    }
    gt_obj_add_event_cb(lv, _selected_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    /* row 1: Del Last | Del First | Del Mid */
    int x0 = sx + _LV_X_PAD;
    _make_btn(screen, x0,                              _BTN_ROW1_Y, _BTN_W3, "Del Last",  _del_last_cb,  lv);
    _make_btn(screen, x0 + (_BTN_W3 + _BTN_GAP),      _BTN_ROW1_Y, _BTN_W3, "Del First", _del_first_cb, lv);
    _make_btn(screen, x0 + (_BTN_W3 + _BTN_GAP) * 2,  _BTN_ROW1_Y, _BTN_W3, "Del Mid",   _del_mid_cb,   lv);

    /* row 2: Clear All | Add Item */
    _make_btn(screen, x0,                    _BTN_ROW2_Y, _BTN_W2, "Clear All", _clear_cb, lv);
    _make_btn(screen, x0 + _BTN_W2 + _BTN_GAP, _BTN_ROW2_Y, _BTN_W2, "Add Item",  _add_cb,   lv);

    return lv;
}

/**
 * @brief Build the horizontal scrolling test section (bottom strip).
 *
 * @param screen     parent screen
 * @param item_count initial item count
 */
static gt_obj_st * _make_hor_section(gt_obj_st * screen, int item_count)
{
    /* title label */
    gt_obj_st * lbl = gt_label_create(screen);
    gt_obj_set_pos(lbl, _LV_X_PAD, _HOR_TITLE_Y);
    gt_obj_set_size(lbl, 400, 20);
    gt_label_set_text(lbl, "Horizontal Scroll Delete");
    gt_label_set_font_size(lbl, 14);

    /* horizontal listview spanning full width */
    gt_obj_st * lv = gt_listview_create(screen);
    gt_obj_set_pos(lv, _LV_X_PAD, _HOR_Y_START);
    gt_obj_set_size(lv, 800 - _LV_X_PAD * 2, _HOR_H);
    gt_listview_set_horizontal(lv, true);
    gt_listview_set_item_width(lv, _HOR_ITEM_W);
    gt_listview_set_item_height(lv, _HOR_H);
    gt_listview_set_septal_line(lv, true);

    char buf[16];
    for (int i = 0; i < item_count; i++) {
        snprintf(buf, sizeof(buf), "H%d", i);
        gt_listview_add_item(lv, buf);
    }
    gt_obj_add_event_cb(lv, _selected_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    /* buttons: Del Last | Del First | Del Mid | Clear All | Add Item */
    int x0 = _LV_X_PAD;
    int bw = (800 - _LV_X_PAD * 2 - _BTN_GAP * 4) / 5;
    _make_btn(screen, x0,                          _HOR_BTN_Y, bw, "Del Last",  _del_last_cb,  lv);
    _make_btn(screen, x0 + (bw + _BTN_GAP),        _HOR_BTN_Y, bw, "Del First", _del_first_cb, lv);
    _make_btn(screen, x0 + (bw + _BTN_GAP) * 2,    _HOR_BTN_Y, bw, "Del Mid",   _del_mid_cb,   lv);
    _make_btn(screen, x0 + (bw + _BTN_GAP) * 3,    _HOR_BTN_Y, bw, "Clear All", _clear_cb,     lv);
    _make_btn(screen, x0 + (bw + _BTN_GAP) * 4,    _HOR_BTN_Y, bw, "Add Item",  _add_cb,       lv);

    return lv;
}

gt_obj_st * gt_examples_listview(void)
{
    _s_add_cnt = 0;

    gt_obj_st * screen = gt_obj_create(NULL);

    /* sec1: single column, 6 items */
    _make_section(screen, 0, "Single Column Delete", 1, 6);

    /* sec2: 3-column, 9 items */
    _make_section(screen, _SECTION_W + _BTN_GAP, "Multi Column (3-col)", 3, 9);

    /* sec3: horizontal scroll, 10 items */
    _make_hor_section(screen, 10);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}


/* global functions / API interface -------------------------------------*/


#endif
/* end ------------------------------------------------------------------*/
