/**
 * @file gt_rand_widget_test.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-06-29 13:32:47
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_rand_widget_test.h"
#if _GT_USE_TEST
#if _GT_TEST_RAND_WIDGET_EVENT
#include "stdlib.h"
#include "time.h"
#include "../src/hal/gt_hal_tick.h"
#include "../src/others/gt_log.h"
#include "../src/core/gt_disp.h"
#include "../src/widgets/gt_obj_class.h"


/* private define -------------------------------------------------------*/
#define _GT_RAND_SEED_MIN   30
#define _GT_RAND_SEED_MAX   200


/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void _event_calling_recursive(gt_obj_st * p) {
    if (NULL == p) {
        return ;
    }
    srand(time(NULL));
    uint32_t rand_val = rand();
    if (0 == p->cnt_child) {
        GT_LOG_A(GT_LOG_TAG_TEST, "  ==== widget: %p[%d], event: %d",
            p, gt_obj_class_get_type(p), rand_val & 0x1 ? GT_EVENT_TYPE_INPUT_PRESSED : GT_EVENT_TYPE_INPUT_RELEASED);
        gt_event_send(p, rand_val & 0x1 ? GT_EVENT_TYPE_INPUT_PRESSED : GT_EVENT_TYPE_INPUT_RELEASED, NULL);
        GT_LOG_A(GT_LOG_TAG_TEST, "  ----");
        return;
    }
    uint32_t rand_widget = rand_val % (p->cnt_child);
    if (rand_val & 0x1) {
        _event_calling_recursive(p->child[rand_widget]);
        return ;
    }
    GT_LOG_A(GT_LOG_TAG_TEST, "  ==== cur: %p widget: %p[%d](%d/%d), event: %d",
        p, p->child[rand_widget], gt_obj_class_get_type(p->child[rand_widget]), rand_widget, p->cnt_child,
        rand_val & 0x1 ? GT_EVENT_TYPE_INPUT_PRESSED : GT_EVENT_TYPE_INPUT_RELEASED);
    gt_event_send(p->child[rand_widget], rand_val & 0x1 ? GT_EVENT_TYPE_INPUT_PRESSED : GT_EVENT_TYPE_INPUT_RELEASED, NULL);
    GT_LOG_A(GT_LOG_TAG_TEST, "  ----");
}


/* global functions / API interface -------------------------------------*/
void _gt_rand_widget_test(void)
{
    static uint32_t next_run_tick = 0;
    uint32_t cur_tick = gt_tick_get();
    if (cur_tick < next_run_tick) {
        return ;
    }
    srand(time(NULL));
    uint32_t rand_value = _GT_RAND_SEED_MIN + rand() % (_GT_RAND_SEED_MAX - _GT_RAND_SEED_MIN + 1);
    next_run_tick = rand_value + cur_tick;
    gt_obj_st * cur_scr = gt_disp_get_scr();
    GT_CHECK_BACK(cur_scr);
    GT_CHECK_BACK(cur_scr->child);
    uint32_t rand_widget = rand() % (cur_scr->cnt_child);
    GT_LOG_A(GT_LOG_TAG_TEST, "==== cur scr: %p, %d/%d, next time after: %d", cur_scr, rand_widget, cur_scr->cnt_child, rand_value);
    _event_calling_recursive(cur_scr->child[rand_widget]);
    GT_LOG_A(GT_LOG_TAG_TEST, "----");
}

#endif  /** _GT_TEST_RAND_WIDGET_EVENT */
#endif  /** _GT_USE_TEST */
/* end ------------------------------------------------------------------*/
