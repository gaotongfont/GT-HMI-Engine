/**
 * @file gt_example_rect.c
 * @author Feyoung
 * @brief Rectangle widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
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
 * @brief Create rectangles with different styles.
 */
gt_obj_st * gt_examples_rect(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    gt_obj_st * rect1 = gt_rect_create(screen);
    gt_obj_set_pos(rect1, 10, 10);
    gt_obj_set_size(rect1, 100, 80);
    gt_rect_set_bg_color(rect1, gt_color_hex(0x2196F3));
    gt_rect_set_fill(rect1, 1);
    gt_rect_set_radius(rect1, 8);

    gt_obj_st * rect2 = gt_rect_create(screen);
    gt_obj_set_pos(rect2, 130, 10);
    gt_obj_set_size(rect2, 100, 80);
    gt_rect_set_bg_color(rect2, gt_color_white());
    gt_rect_set_color_border(rect2, gt_color_hex(0xFF5722));
    gt_rect_set_border(rect2, 2);
    gt_rect_set_fill(rect2, 0);
    gt_rect_set_radius(rect2, 0);

    gt_obj_st * rect3 = gt_rect_create(screen);
    gt_obj_set_pos(rect3, 250, 10);
    gt_obj_set_size(rect3, 100, 80);
    gt_rect_set_bg_color(rect3, gt_color_hex(0x4CAF50));
    gt_rect_set_fill(rect3, 1);
    gt_rect_set_radius(rect3, 50);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
