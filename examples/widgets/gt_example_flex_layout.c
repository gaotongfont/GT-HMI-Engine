/**
 * @file gt_example_flex_layout.c
 * @author Feyoung
 * @brief Flex layout demonstration
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_examples.h"
#include "../../src/core/gt_layout.h"
#include "../../src/widgets/gt_rect.h"
#include "../../src/widgets/gt_label.h"
#include "../../src/widgets/gt_btn.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES && GT_USE_WIDGET_LAYOUT

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/
static gt_obj_st * _container = NULL;

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/
static void _toggle_layout_cb(gt_event_st * e)
{
    if (!_container) return;
    if (gt_layout_is_type(_container, GT_LAYOUT_TYPE_FLEX)) {
        gt_layout_set_type(_container, GT_LAYOUT_TYPE_FIXED);
        gt_btn_set_text(e->target, "FIXED->FLEX");
    } else {
        gt_layout_set_type(_container, GT_LAYOUT_TYPE_FLEX);
        gt_btn_set_text(e->target, "FLEX->FIXED");
    }
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Flex layout demo: ROW + SPACE_BETWEEN + toggle FIXED/FLEX.
 */
gt_obj_st * gt_examples_flex_layout(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);
    gt_screen_set_bgcolor(screen, gt_color_hex(0xFFFFFF));

    gt_obj_st * title = gt_label_create(screen);
    gt_label_set_text(title, "Flex Layout Demo");
    gt_obj_set_pos(title, 70, 5);

    _container = gt_obj_create(screen);
    gt_obj_set_pos(_container, 10, 50);
    gt_obj_set_size(_container, 300, 120);

    gt_obj_container_st ct = {
        .layout_type     = GT_LAYOUT_TYPE_FLEX,
        .flex_direction  = GT_LAYOUT_FLEX_DIR_ROW,
        .justify_content = GT_LAYOUT_JUSTIFY_CONTENT_SPACE_BETWEEN,
        .align_items     = GT_LAYOUT_ALIGN_ITEMS_CENTER,
        .shrink          = GT_LAYOUT_SHRINK_DISABLE,
    };
    gt_layout_init(_container, &ct);

    gt_obj_st * r1 = gt_rect_create(_container);
    gt_obj_set_size(r1, 60, 60);
    gt_rect_set_bg_color(r1, gt_color_hex(0x2196F3));
    gt_rect_set_fill(r1, 1);
    gt_rect_set_radius(r1, 4);

    gt_obj_st * r2 = gt_rect_create(_container);
    gt_obj_set_size(r2, 60, 80);
    gt_rect_set_bg_color(r2, gt_color_hex(0xFF5722));
    gt_rect_set_fill(r2, 1);
    gt_rect_set_radius(r2, 4);

    gt_obj_st * r3 = gt_rect_create(_container);
    gt_obj_set_size(r3, 60, 40);
    gt_rect_set_bg_color(r3, gt_color_hex(0x4CAF50));
    gt_rect_set_fill(r3, 1);
    gt_rect_set_radius(r3, 4);

    gt_obj_st * btn = gt_btn_create(screen);
    gt_obj_set_pos(btn, 80, 200);
    gt_obj_set_size(btn, 160, 40);
    gt_btn_set_text(btn, "FLEX->FIXED");
    gt_obj_add_event_cb(btn, _toggle_layout_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}

#else
gt_obj_st * gt_examples_flex_layout(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);
    GT_EXAMPLE_ADD_BACK_BTN(screen);
    return screen;
}
#endif
/* end ------------------------------------------------------------------*/
