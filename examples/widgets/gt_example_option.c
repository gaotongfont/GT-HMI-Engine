/**
 * @file gt_example_option.c
 * @author Feyoung
 * @brief Option widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_option.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/
static void _option_event_cb(gt_event_st * e)
{
    printf("option selected\n");
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create an option widget.
 */
gt_obj_st * gt_examples_option(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    gt_obj_st * opt = gt_option_create(screen);
    gt_obj_set_pos(opt, 10, 30);
    gt_obj_set_size(opt, 200, 150);
    gt_obj_add_event_cb(opt, _option_event_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
