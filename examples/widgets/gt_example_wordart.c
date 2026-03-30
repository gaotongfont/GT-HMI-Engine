/**
 * @file gt_example_wordart.c
 * @author Feyoung
 * @brief Wordart widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_wordart.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a wordart widget displaying a number.
 */
gt_obj_st * gt_examples_wordart(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    gt_obj_st * wa = gt_wordart_create(screen);
    gt_obj_set_pos(wa, 50, 50);
    gt_obj_set_size(wa, 200, 60);
    gt_wordart_set_text(wa, "Hello");
    gt_wordart_set_space(wa, 2, 0);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
