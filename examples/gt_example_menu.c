/**
 * @file gt_example_menu.c
 * @author Feyoung
 * @brief Example navigation menu screen
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_examples.h"
#include "../src/widgets/gt_listview.h"
#include "../src/widgets/gt_label.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

typedef struct {
    const char * name;
    gt_example_scr_id_et scr_id;
} _menu_item_st;

/* static variables -----------------------------------------------------*/

static const _menu_item_st _menu_items[] = {
    { "Anim",           GT_EXAMPLE_SCR_ANIM1 },
    { "Button",         GT_EXAMPLE_SCR_BUTTON },
    { "Checkbox",       GT_EXAMPLE_SCR_CHECKBOX },
    { "ImgButton",      GT_EXAMPLE_SCR_IMG_BUTTON },
    { "Image",          GT_EXAMPLE_SCR_IMG },
    { "SVG",            GT_EXAMPLE_SCR_SVG },
    { "Input",          GT_EXAMPLE_SCR_INPUT },
    { "Keyboard",       GT_EXAMPLE_SCR_KEYBOARD },
    { "Label",          GT_EXAMPLE_SCR_LABEL },
    { "Line",           GT_EXAMPLE_SCR_LINE },
    { "ListView",       GT_EXAMPLE_SCR_LISTVIEW },
    { "ProgressBar",    GT_EXAMPLE_SCR_PROGRESS_BAR },
    { "Roller",         GT_EXAMPLE_SCR_ROLLER },
    { "Slider",         GT_EXAMPLE_SCR_SLIDER },
    { "Slider AB Repeat", GT_EXAMPLE_SCR_SLIDER_AB_REPEAT },
    { "Slider Tag Raw",  GT_EXAMPLE_SCR_SLIDER_TAG_RAW },
    { "Textarea",       GT_EXAMPLE_SCR_TEXTAREA },
    { "Radio",          GT_EXAMPLE_SCR_RADIO },
    { "Switch",         GT_EXAMPLE_SCR_SWITCH },
    { "Clock",          GT_EXAMPLE_SCR_CLOCK },
    { "Rect",           GT_EXAMPLE_SCR_RECT },
    { "Arc",            GT_EXAMPLE_SCR_ARC },
    { "Select",         GT_EXAMPLE_SCR_SELECT },
    { "Table",          GT_EXAMPLE_SCR_TABLE },
    { "Graphs",         GT_EXAMPLE_SCR_GRAPHS },
    { "Dialog",         GT_EXAMPLE_SCR_DIALOG },
    { "Calendar",       GT_EXAMPLE_SCR_CALENDAR },
    { "ViewPager",      GT_EXAMPLE_SCR_VIEW_PAGER },
    { "BtnMap",         GT_EXAMPLE_SCR_BTNMAP },
    { "StatusBar",      GT_EXAMPLE_SCR_STATUS_BAR },
    { "Toast",          GT_EXAMPLE_SCR_TOAST },
    { "InputNumber",    GT_EXAMPLE_SCR_INPUT_NUMBER },
    { "Barcode",        GT_EXAMPLE_SCR_BARCODE },
    { "WordArt",        GT_EXAMPLE_SCR_WORDART },
    { "Player",         GT_EXAMPLE_SCR_PLAYER },
    { "Chat",           GT_EXAMPLE_SCR_CHAT },
    { "Group",          GT_EXAMPLE_SCR_GROUP },
    { "CrossLayout",    GT_EXAMPLE_SCR_CROSS_LAYOUT },
    { "Option",         GT_EXAMPLE_SCR_OPTION },
    { "[Flex Layout]",  GT_EXAMPLE_SCR_FLEX_LAYOUT },
    { "[Dynamic Style]",GT_EXAMPLE_SCR_DYNAMIC_STYLE },
    { "[Form]",         GT_EXAMPLE_SCR_FORM },
    { "Button Driver",  GT_EXAMPLE_SCR_BUTTON_DRIVER },
    { "Spectrum",       GT_EXAMPLE_SCR_SPECTRUM },
    { "[Theme]",        GT_EXAMPLE_SCR_THEME },
    { "Polygon",        GT_EXAMPLE_SCR_POLYGON },
    { "Textarea file",  GT_EXAMPLE_SCR_TEXTAREA_FILE },
    { "Vector",         GT_EXAMPLE_SCR_VECTOR },
    { "Viewport Fixed", GT_EXAMPLE_SCR_VIEWPORT_FIXED },
};

#define _MENU_ITEM_COUNT  (sizeof(_menu_items) / sizeof(_menu_items[0]))

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/

static void _menu_item_cb(gt_event_st * e)
{
    gt_size_t idx = gt_listview_get_idx_selected(e->target);
    if (idx < 0 || idx >= (gt_size_t)_MENU_ITEM_COUNT) {
        return;
    }
    gt_disp_stack_load_scr_anim(
        _menu_items[idx].scr_id,
        GT_SCR_ANIM_TYPE_MOVE_LEFT,
        300, 0, true
    );
}

/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_examples_menu(void)
{
    gt_obj_st * screen = gt_obj_create(NULL);
    gt_screen_set_bgcolor(screen, gt_color_hex(0xFFFFFF));

    gt_obj_st * title = gt_label_create(screen);
    gt_label_set_text(title, "GT-HMI Examples");
    gt_obj_set_pos(title, 10, 5);
    gt_obj_set_size(title, 300, 30);

    gt_obj_st * lv = gt_listview_create(screen);
    gt_obj_set_pos(lv, 0, 40);
    gt_obj_set_size(lv, GT_SCREEN_WIDTH, GT_SCREEN_HEIGHT - 40);

    uint16_t i;
    for (i = 0; i < _MENU_ITEM_COUNT; i++) {
        gt_listview_add_item(lv, _menu_items[i].name);
    }
    gt_obj_add_event_cb(lv, _menu_item_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);

    return screen;
}

#endif
/* end ------------------------------------------------------------------*/

