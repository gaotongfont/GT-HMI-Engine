/**
 * @file gt_example_cross_layout.c
 * @author Feyoung
 * @brief Cross layout widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_cross_layout.h"
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
 * @brief Create a cross layout with horizontal and vertical items.
 */
gt_obj_st * gt_examples_cross_layout(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    gt_obj_st * layout = gt_cross_layout_create(screen);
    gt_obj_set_pos(layout, 0, 0);
    gt_obj_set_size(layout, 320, 400);
    gt_cross_layout_set_gap(layout, 10, 10);

    gt_obj_st * h1 = gt_label_create(screen);
    gt_label_set_text(h1, "Tab 1");
    gt_obj_set_size(h1, 80, 30);
    gt_cross_layout_hor_add_item(layout, h1, 0);

    gt_obj_st * h2 = gt_label_create(screen);
    gt_label_set_text(h2, "Tab 2");
    gt_obj_set_size(h2, 80, 30);
    gt_cross_layout_hor_add_item(layout, h2, 1);

    gt_obj_st * v1 = gt_label_create(screen);
    gt_label_set_text(v1, "Content A");
    gt_obj_set_size(v1, 200, 40);
    gt_cross_layout_ver_add_item(layout, v1);

    gt_obj_st * v2 = gt_label_create(screen);
    gt_label_set_text(v2, "Content B");
    gt_obj_set_size(v2, 200, 40);
    gt_cross_layout_ver_add_item(layout, v2);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
