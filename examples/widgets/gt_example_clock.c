/**
 * @file gt_example_clock.c
 * @author Feyoung
 * @brief Clock widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_clock.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a digital clock widget.
 */
gt_obj_st * gt_examples_clock(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    gt_size_t font_size = 16;
    gt_obj_st * clock = gt_clock_create(screen);
    gt_obj_set_pos(clock, 50, 50);
    gt_obj_set_size(clock, font_size * 10, font_size << 1);
    gt_clock_set_font_size(clock, font_size);
    gt_clock_set_time(clock, 14, 30, 0);
    gt_clock_set_mode(clock, GT_CLOCK_MODE_TIME);
    gt_clock_set_twinkle_colon_mode(clock, true);
    gt_clock_set_twinkle_colon_period(clock, 500);
    gt_clock_set_font_color(clock, gt_color_black());
    gt_clock_start(clock);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
