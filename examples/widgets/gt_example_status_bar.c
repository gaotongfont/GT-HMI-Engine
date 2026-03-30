/**
 * @file gt_example_status_bar.c
 * @author Feyoung
 * @brief Status bar widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_status_bar.h"
#include "../../src/widgets/gt_label.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a status bar at the top of screen.
 */
gt_obj_st * gt_examples_status_bar(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    gt_obj_st * bar = gt_status_bar_create(true);
    gt_status_bar_set_height(30);

    gt_obj_st * lbl = gt_label_create(screen);
    gt_label_set_text(lbl, "Main content area");
    gt_obj_set_pos(lbl, 10, 60);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
