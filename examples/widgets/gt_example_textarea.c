/**
 * @file gt_example_textarea.c
 * @author Feyoung
 * @brief Textarea widget example
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/widgets/gt_textarea.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create textarea with mixed style text.
 */
gt_obj_st * gt_examples_textarea(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);

    gt_obj_st * ta = gt_textarea_create(screen);
    gt_obj_set_pos(ta, 10, 10);
    gt_obj_set_size(ta, 300, 200);
    gt_textarea_set_font_size(ta, 16);
    gt_textarea_set_font_color(ta, gt_color_black());
    gt_textarea_set_bg_color(ta, gt_color_hex(0xF5F5F5));
    gt_textarea_set_border_width(ta, 1);
    gt_textarea_set_border_color(ta, gt_color_hex(0xCCCCCC));
    gt_textarea_set_radius(ta, 4);
    gt_textarea_set_space(ta, 0, 4);

    gt_textarea_set_text(ta, "This is a textarea example.\nLine 2 content.");
    gt_textarea_add_str(ta, "Bold text", GT_FONT_STYLE_BOLD, gt_color_hex(0xFF0000));
    gt_textarea_add_str(ta, " Normal text", GT_FONT_STYLE_NONE, gt_color_black());

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#endif
/* end ------------------------------------------------------------------*/
