/**
 * @file gt_example_view_pager.c
 * @author Feyoung
 * @brief View pager widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_view_pager.h"
#include "../../src/widgets/gt_label.h"
#include "../../src/widgets/gt_rect.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a view pager with multiple pages.
 */
gt_obj_st * gt_examples_view_pager(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    gt_obj_st * vp = gt_view_pager_create(screen);
    gt_obj_set_pos(vp, 10, 10);
    gt_obj_set_size(vp, 300, 200);
    gt_view_pager_set_fragment_count(vp, 3);

    gt_obj_st * lbl0 = gt_label_create(screen);
    gt_label_set_text(lbl0, "Page 1");
    gt_obj_set_pos(lbl0, vp->area.w >> 1, vp->area.h >> 1);
    gt_view_pager_fragment_add_widget(vp, 0, lbl0);

    gt_obj_st * lbl1 = gt_label_create(screen);
    gt_label_set_text(lbl1, "Page 2");
    gt_obj_set_pos(lbl1, vp->area.w >> 1, vp->area.h >> 1);
    gt_view_pager_fragment_add_widget(vp, 1, lbl1);

    gt_obj_st * lbl2 = gt_label_create(screen);
    gt_label_set_text(lbl2, "Page 3");
    gt_obj_set_pos(lbl2, vp->area.w >> 1, vp->area.h >> 1);
    gt_view_pager_fragment_add_widget(vp, 2, lbl2);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
