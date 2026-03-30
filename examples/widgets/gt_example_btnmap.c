/**
 * @file gt_example_btnmap.c
 * @author Feyoung
 * @brief Button map widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_btnmap.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/
static void _btnmap_event_cb(gt_event_st * e)
{
    printf("btnmap item clicked\n");
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a button map grid.
 */
gt_obj_st * gt_examples_btnmap(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    gt_obj_st * btnmap = gt_btnmap_create(screen);
    gt_obj_set_pos(btnmap, 10, 10);
    gt_obj_set_size(btnmap, 300, 200);
    gt_obj_add_event_cb(btnmap, _btnmap_event_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
