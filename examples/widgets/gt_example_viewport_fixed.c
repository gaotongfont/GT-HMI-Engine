/**
 * @file gt_example_viewport_fixed.c
 * @brief Screen-local viewport fixed widget test.
 */

#include "../gt_examples.h"
#include "../../src/widgets/gt_label.h"
#include "../../src/widgets/gt_btn.h"
#include "../../src/widgets/gt_rect.h"

#if GT_USE_EXAMPLES

static gt_obj_st * _float_target;
static gt_obj_st * _result_label;

static void _set_result(const char * text)
{
    gt_label_set_text(_result_label, text);
}

static void _float_target_cb(gt_event_st * e)
{
    (void)e;
    _set_result("Viewport-fixed target pressed");
}

static void _content_target_cb(gt_event_st * e)
{
    (void)e;
    _set_result("Content target pressed");
}

static void _toggle_float_cb(gt_event_st * e)
{
    bool enable = !gt_obj_get_viewport_fixed(_float_target);
    gt_obj_set_viewport_fixed(_float_target, enable);
    gt_btn_set_text(e->target, enable ? "Float: ON" : "Float: OFF");
    _set_result(enable ? "Viewport fixed enabled" : "Viewport fixed disabled");
}

gt_obj_st * gt_examples_viewport_fixed(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);
    gt_screen_set_bgcolor(screen, gt_color_hex(0xF4F6F8));
    gt_obj_set_scroll_dir(screen, GT_SCROLL_VERTICAL);

    gt_obj_st * title = gt_label_create(screen);
    gt_label_set_text(title, "Viewport Fixed Test");
    gt_obj_set_pos(title, 10, 8);
    gt_obj_set_size(title, GT_SCREEN_WIDTH - 20, 26);

    gt_obj_st * overlap = gt_btn_create(screen);
    gt_obj_set_pos(overlap, 8, 42);
    gt_obj_set_size(overlap, GT_SCREEN_WIDTH - 16, 38);
    gt_btn_set_text(overlap, "Normal content overlap");
    gt_obj_add_event_cb(overlap, _content_target_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    _float_target = gt_btn_create(screen);
    gt_obj_set_pos(_float_target, 8, 42);
    gt_obj_set_size(_float_target, GT_SCREEN_WIDTH - 16, 38);
    gt_btn_set_text(_float_target, "Viewport-fixed target");
    gt_obj_add_event_cb(_float_target, _float_target_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);
    gt_obj_set_viewport_fixed(_float_target, true);

    gt_obj_st * toggle = gt_btn_create(screen);
    gt_obj_set_pos(toggle, 8, 86);
    gt_obj_set_size(toggle, 110, 32);
    gt_btn_set_text(toggle, "Float: ON");
    gt_obj_add_event_cb(toggle, _toggle_float_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);
    gt_obj_set_viewport_fixed(toggle, true);

    gt_obj_st * back = gt_btn_create(screen);
    gt_obj_set_pos(back, GT_SCREEN_WIDTH - 78, 86);
    gt_obj_set_size(back, 70, 32);
    gt_btn_set_text(back, "Back >");
    gt_obj_add_event_cb(back, _gt_example_back_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);
    gt_obj_set_viewport_fixed(back, true);

    _result_label = gt_label_create(screen);
    gt_label_set_text(_result_label, "Scroll, then press controls");
    gt_obj_set_pos(_result_label, 8, 124);
    gt_obj_set_size(_result_label, GT_SCREEN_WIDTH - 16, 28);
    gt_obj_set_viewport_fixed(_result_label, true);

    for (uint8_t i = 0; i < 10; ++i) {
        gt_obj_st * row = gt_rect_create(screen);
        gt_obj_set_pos(row, 10, 180 + i * 58);
        gt_obj_set_size(row, GT_SCREEN_WIDTH - 20, 46);
        gt_rect_set_bg_color(row, (i & 1) ? gt_color_hex(0xD9E8F6) : gt_color_hex(0xFFFFFF));
        gt_rect_set_fill(row, true);

        gt_obj_st * label = gt_label_create(row);
        gt_label_set_text(label, "Scrollable content row");
        gt_obj_set_pos(label, row->area.x + 10, row->area.y + 14);
        gt_obj_set_size(label, row->area.w - 20, 22);
    }

    return screen;
}

#endif

