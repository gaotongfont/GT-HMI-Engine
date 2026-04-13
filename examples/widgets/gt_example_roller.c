/**
 * @file gt_example_roller.c
 * @author Feyoung
 * @brief Roller widget examples: normal / infinite / inertia-snap / multi-column
 * @version 0.2
 * @date 2023-12-12 11:10:18
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_roller.h"
#include "../../src/widgets/gt_btn.h"
#include "../../src/widgets/gt_label.h"
#include "../../src/core/gt_event.h"
#include "../../src/others/gt_log.h"


/* private define -------------------------------------------------------*/

#if GT_USE_EXAMPLES


/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _roller_prev_cb(gt_event_st * e) {
    gt_obj_st * target = (gt_obj_st * )e->user_data;
    gt_roller_go_prev(target);
}

static void _roller_next_cb(gt_event_st * e) {
    gt_obj_st * target = (gt_obj_st * )e->user_data;
    gt_roller_go_next(target);
}

typedef struct {
    gt_obj_st * roller;
    gt_obj_st * label;
} _roller_label_pair_st;

static _roller_label_pair_st _pair5;

static void _roller_value_cb(gt_event_st * e) {
    _roller_label_pair_st * p = (_roller_label_pair_st * )e->user_data;
    char buf[32];
    gt_roller_get_selected_text(p->roller, buf, sizeof(buf));
    gt_label_set_text(p->label, buf);
}

/** ------------------------------------------------------------------ */
/** Case 1: NORMAL mode — basic prev/next buttons                       */
/** ------------------------------------------------------------------ */
static gt_obj_st * _case_normal(gt_obj_st * scr, gt_size_t x, gt_size_t y) {
    gt_obj_st * roller = gt_roller_create(scr);
    gt_obj_set_pos(roller, x, y);
    gt_roller_set_options(roller,
        "Mon\nTue\nWed\nThu\nFri\nSat\nSun",
        GT_ROLLER_MODE_NORMAL);
    gt_roller_set_display_item_count(roller, 3);

    gt_obj_st * prev = gt_btn_create(scr);
    gt_obj_set_pos(prev, x, y + 100);
    gt_obj_set_size(prev, 50, 28);
    gt_btn_set_text(prev, "<<");
    gt_obj_add_event_cb(prev, _roller_prev_cb, GT_EVENT_TYPE_INPUT_RELEASED, roller);

    gt_obj_st * next = gt_btn_create(scr);
    gt_obj_set_pos(next, x + 55, y + 100);
    gt_obj_set_size(next, 50, 28);
    gt_btn_set_text(next, ">>");
    gt_obj_add_event_cb(next, _roller_next_cb, GT_EVENT_TYPE_INPUT_RELEASED, roller);

    return roller;
}

/** ------------------------------------------------------------------ */
/** Case 2: INFINITE mode — wraps around seamlessly                     */
/** ------------------------------------------------------------------ */
static gt_obj_st * _case_infinite(gt_obj_st * scr, gt_size_t x, gt_size_t y) {
    gt_obj_st * roller = gt_roller_create(scr);
    gt_obj_set_pos(roller, x, y);
    gt_roller_set_options(roller,
        "0\n1\n2\n3\n4\n5\n6\n7\n8\n9",
        GT_ROLLER_MODE_INFINITE);
    gt_roller_set_display_item_count(roller, 5);

    gt_obj_st * prev = gt_btn_create(scr);
    gt_obj_set_pos(prev, x, y + 140);
    gt_obj_set_size(prev, 50, 28);
    gt_btn_set_text(prev, "<<");
    gt_obj_add_event_cb(prev, _roller_prev_cb, GT_EVENT_TYPE_INPUT_RELEASED, roller);

    gt_obj_st * next = gt_btn_create(scr);
    gt_obj_set_pos(next, x + 55, y + 140);
    gt_obj_set_size(next, 50, 28);
    gt_btn_set_text(next, ">>");
    gt_obj_add_event_cb(next, _roller_next_cb, GT_EVENT_TYPE_INPUT_RELEASED, roller);

    return roller;
}

/** ------------------------------------------------------------------ */
/** Case 3: Inertia snap — fast swipe skips to next item                */
/** ------------------------------------------------------------------ */
static gt_obj_st * _case_inertia(gt_obj_st * scr, gt_size_t x, gt_size_t y) {
    gt_obj_st * roller = gt_roller_create(scr);
    gt_obj_set_pos(roller, x, y);
    gt_roller_set_options(roller,
        "Jan\nFeb\nMar\nApr\nMay\nJun\nJul\nAug\nSep\nOct\nNov\nDec",
        GT_ROLLER_MODE_INFINITE);
    gt_roller_set_display_item_count(roller, 3);
    /* Enable inertia snap: fast swipe will advance to the next item */
    gt_roller_set_inertia_snap(roller, true);

    return roller;
}

/** ------------------------------------------------------------------ */
/** Case 4: Multi-column — hour / minute / second pickers side by side  */
/** ------------------------------------------------------------------ */
static void _case_multi_column(gt_obj_st * scr, gt_size_t x, gt_size_t y) {
    static const char * cols[] = {
        "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23",
        "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59",
        "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59",
    };
    static const char * sep[] = { ":", ":" };
    gt_size_t col_w = 60, sep_w = 12, gap = 4;
    gt_size_t cx = x;

    for (int i = 0; i < 3; i++) {
        gt_obj_st * roller = gt_roller_create(scr);
        gt_obj_set_pos(roller, cx, y);
        gt_obj_set_size(roller, col_w, 90);
        gt_roller_set_options(roller, (char *)cols[i], GT_ROLLER_MODE_INFINITE);
        gt_roller_set_display_item_count(roller, 3);
        gt_roller_set_inertia_snap(roller, true);
        cx += col_w + gap;

        if (i < 2) {
            gt_obj_st * lbl = gt_label_create(scr);
            gt_obj_set_pos(lbl, cx, y + 33);
            gt_label_set_text(lbl, (char *)sep[i]);
            cx += sep_w + gap;
        }
    }
}

/** ------------------------------------------------------------------ */
/** Case 5: Value display — label shows selected text on UPDATE_VALUE   */
/** ------------------------------------------------------------------ */
static void _case_value_display(gt_obj_st * scr, gt_size_t x, gt_size_t y) {
    gt_obj_st * label = gt_label_create(scr);
    gt_obj_set_pos(label, x + 110, y + 30);
    gt_label_set_text(label, "test0");

    gt_obj_st * roller = gt_roller_create(scr);
    gt_obj_set_pos(roller, x, y);
    gt_roller_set_options(roller,
        "test0\ntest1\ntest2\ntest3\ntest4",
        GT_ROLLER_MODE_NORMAL);
    gt_roller_set_display_item_count(roller, 3);

    _pair5.roller = roller;
    _pair5.label  = label;
    gt_obj_add_event_cb(roller, _roller_value_cb, GT_EVENT_TYPE_UPDATE_VALUE, &_pair5);
}


/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_examples_roller(void)
{
    gt_obj_st * screen_home = gt_obj_create(NULL);
    gt_screen_set_bgcolor(screen_home, gt_color_hex(0xffffff));
    gt_obj_set_pos(screen_home, 0, 0);

    /* Case 1: Normal mode */
    gt_obj_st * lbl1 = gt_label_create(screen_home);
    gt_obj_set_pos(lbl1, 10, 10);
    gt_label_set_text(lbl1, "Normal");
    _case_normal(screen_home, 10, 30);

    /* Case 2: Infinite mode */
    gt_obj_st * lbl2 = gt_label_create(screen_home);
    gt_obj_set_pos(lbl2, 140, 10);
    gt_label_set_text(lbl2, "Infinite");
    _case_infinite(screen_home, 140, 30);

    /* Case 3: Inertia snap */
    gt_obj_st * lbl3 = gt_label_create(screen_home);
    gt_obj_set_pos(lbl3, 280, 10);
    gt_label_set_text(lbl3, "Inertia");
    _case_inertia(screen_home, 280, 30);

    /* Case 4: Multi-column time picker */
    gt_obj_st * lbl4 = gt_label_create(screen_home);
    gt_obj_set_pos(lbl4, 10, 200);
    gt_label_set_text(lbl4, "HH:MM:SS");
    _case_multi_column(screen_home, 10, 220);

    /* Case 5: Value display via UPDATE_VALUE event */
    gt_obj_st * lbl5 = gt_label_create(screen_home);
    gt_obj_set_pos(lbl5, 10, 330);
    gt_label_set_text(lbl5, "Selected:");
    _case_value_display(screen_home, 10, 350);

    GT_EXAMPLE_ADD_BACK_BTN(screen_home);
    return screen_home;
}


#endif  /** GT_USE_EXAMPLES */
/* end ------------------------------------------------------------------*/
