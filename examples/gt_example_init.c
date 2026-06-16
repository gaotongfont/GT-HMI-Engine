/**
 * @file gt_example_init.c
 * @author Feyoung
 * @brief Example navigation system initialization
 * @version 0.1
 * @date 2026-03-27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_examples.h"
#include "../src/core/gt_scr_stack.h"

/* private define -------------------------------------------------------*/
#if GT_USE_EXAMPLES

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

static const gt_scr_list_st _example_scr_list[] = {
    { GT_EXAMPLE_SCR_MENU,          gt_examples_menu },
    { GT_EXAMPLE_SCR_ANIM1,         gt_examples_anim1 },
    { GT_EXAMPLE_SCR_BUTTON,        gt_examples_button },
    { GT_EXAMPLE_SCR_CHECKBOX,      gt_examples_checkbox },
    { GT_EXAMPLE_SCR_IMG_BUTTON,    gt_examples_img_button },
    { GT_EXAMPLE_SCR_IMG,           gt_examples_img },
    { GT_EXAMPLE_SCR_SVG,           gt_examples_svg },
    { GT_EXAMPLE_SCR_INPUT,         gt_example_input },
    { GT_EXAMPLE_SCR_KEYBOARD,      gt_examples_keyboard },
    { GT_EXAMPLE_SCR_LABEL,         gt_examples_label },
    { GT_EXAMPLE_SCR_LINE,          gt_examples_line },
    { GT_EXAMPLE_SCR_LISTVIEW,      gt_examples_listview },
    { GT_EXAMPLE_SCR_PROGRESS_BAR,  gt_examples_progress_bar },
    { GT_EXAMPLE_SCR_ROLLER,        gt_examples_roller },
    { GT_EXAMPLE_SCR_SLIDER,        gt_examples_slider },
    { GT_EXAMPLE_SCR_SLIDER_AB_REPEAT, gt_examples_slider_ab_repeat },
    { GT_EXAMPLE_SCR_SLIDER_TAG_RAW,  gt_examples_slider_tag_raw },
    { GT_EXAMPLE_SCR_TEXTAREA,      gt_examples_textarea },
    { GT_EXAMPLE_SCR_RADIO,         gt_examples_radio },
    { GT_EXAMPLE_SCR_SWITCH,        gt_examples_switch },
    { GT_EXAMPLE_SCR_CLOCK,         gt_examples_clock },
    { GT_EXAMPLE_SCR_RECT,          gt_examples_rect },
    { GT_EXAMPLE_SCR_ARC,           gt_examples_arc },
    { GT_EXAMPLE_SCR_SELECT,        gt_examples_select },
    { GT_EXAMPLE_SCR_TABLE,         gt_examples_table },
    { GT_EXAMPLE_SCR_GRAPHS,        gt_examples_graphs },
    { GT_EXAMPLE_SCR_DIALOG,        gt_examples_dialog },
    { GT_EXAMPLE_SCR_CALENDAR,      gt_examples_calendar },
    { GT_EXAMPLE_SCR_VIEW_PAGER,    gt_examples_view_pager },
    { GT_EXAMPLE_SCR_BTNMAP,        gt_examples_btnmap },
    { GT_EXAMPLE_SCR_STATUS_BAR,    gt_examples_status_bar },
    { GT_EXAMPLE_SCR_TOAST,         gt_examples_toast },
    { GT_EXAMPLE_SCR_INPUT_NUMBER,  gt_examples_input_number },
    { GT_EXAMPLE_SCR_BARCODE,       gt_examples_barcode },
    { GT_EXAMPLE_SCR_WORDART,       gt_examples_wordart },
    { GT_EXAMPLE_SCR_PLAYER,        gt_examples_player },
    { GT_EXAMPLE_SCR_CHAT,          gt_examples_chat },
    { GT_EXAMPLE_SCR_GROUP,         gt_examples_group },
    { GT_EXAMPLE_SCR_CROSS_LAYOUT,  gt_examples_cross_layout },
    { GT_EXAMPLE_SCR_OPTION,        gt_examples_option },
    { GT_EXAMPLE_SCR_FLEX_LAYOUT,   gt_examples_flex_layout },
    { GT_EXAMPLE_SCR_DYNAMIC_STYLE, gt_examples_dynamic_style },
    { GT_EXAMPLE_SCR_FORM,          gt_examples_form },
    { GT_EXAMPLE_SCR_BUTTON_DRIVER, gt_examples_button_driver },
    { GT_EXAMPLE_SCR_SPECTRUM,      gt_examples_spectrum },
    { GT_EXAMPLE_SCR_THEME,         gt_examples_theme },
    { GT_EXAMPLE_SCR_POLYGON,       gt_examples_polygon },
};

/* macros ---------------------------------------------------------------*/

/* class ----------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/

/* global functions / API interface -------------------------------------*/

void _gt_example_back_cb(gt_event_st * e)
{
    gt_disp_stack_go_back(1);
}

void gt_examples_init(void)
{
    gt_scr_id_t count = sizeof(_example_scr_list) / sizeof(_example_scr_list[0]);
    gt_scr_stack_register_id_list(_example_scr_list, count, 10);
    gt_scr_stack_set_home_scr_id(GT_EXAMPLE_SCR_MENU, true);
    gt_disp_stack_load_scr(GT_EXAMPLE_SCR_MENU);
}

#endif
/* end ------------------------------------------------------------------*/
