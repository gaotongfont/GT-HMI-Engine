/**
 * @file gt_examples.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-09-28 11:40:52
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_EXAMPLES_H_
#define _GT_EXAMPLES_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/

#include "../src/gt_conf.h"
#include "../src/others/gt_log.h"
#include "../src/core/gt_style.h"
#include "../src/core/gt_disp.h"
#include "../src/core/gt_scr_stack.h"
#include "../src/widgets/gt_obj.h"
#include "../src/widgets/gt_btn.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/

/** @brief Example 屏幕 ID 枚举 */
typedef enum {
    GT_EXAMPLE_SCR_MENU = 0,
    GT_EXAMPLE_SCR_ANIM1,
    GT_EXAMPLE_SCR_BUTTON,
    GT_EXAMPLE_SCR_CHECKBOX,
    GT_EXAMPLE_SCR_IMG_BUTTON,
    GT_EXAMPLE_SCR_IMG,
    GT_EXAMPLE_SCR_INPUT,
    GT_EXAMPLE_SCR_KEYBOARD,
    GT_EXAMPLE_SCR_LABEL,
    GT_EXAMPLE_SCR_LINE,
    GT_EXAMPLE_SCR_LISTVIEW,
    GT_EXAMPLE_SCR_PROGRESS_BAR,
    GT_EXAMPLE_SCR_ROLLER,
    GT_EXAMPLE_SCR_SLIDER,
    GT_EXAMPLE_SCR_TEXTAREA,
    GT_EXAMPLE_SCR_RADIO,
    GT_EXAMPLE_SCR_SWITCH,
    GT_EXAMPLE_SCR_CLOCK,
    GT_EXAMPLE_SCR_RECT,
    GT_EXAMPLE_SCR_ARC,
    GT_EXAMPLE_SCR_SELECT,
    GT_EXAMPLE_SCR_TABLE,
    GT_EXAMPLE_SCR_GRAPHS,
    GT_EXAMPLE_SCR_DIALOG,
    GT_EXAMPLE_SCR_CALENDAR,
    GT_EXAMPLE_SCR_VIEW_PAGER,
    GT_EXAMPLE_SCR_BTNMAP,
    GT_EXAMPLE_SCR_STATUS_BAR,
    GT_EXAMPLE_SCR_TOAST,
    GT_EXAMPLE_SCR_INPUT_NUMBER,
    GT_EXAMPLE_SCR_BARCODE,
    GT_EXAMPLE_SCR_WORDART,
    GT_EXAMPLE_SCR_PLAYER,
    GT_EXAMPLE_SCR_CHAT,
    GT_EXAMPLE_SCR_GROUP,
    GT_EXAMPLE_SCR_CROSS_LAYOUT,
    GT_EXAMPLE_SCR_OPTION,
    GT_EXAMPLE_SCR_FLEX_LAYOUT,
    GT_EXAMPLE_SCR_DYNAMIC_STYLE,
    GT_EXAMPLE_SCR_FORM,
    GT_EXAMPLE_SCR_TOTAL,
}gt_example_scr_id_et;



/* macros ---------------------------------------------------------------*/

/** @brief 在 example 屏幕上添加返回按钮 */
#define GT_EXAMPLE_ADD_BACK_BTN(parent) \
    do { \
        gt_obj_st * _back = gt_btn_create(parent); \
        gt_obj_set_pos(_back, GT_SCREEN_WIDTH - 70, 0); \
        gt_obj_set_size(_back, 70, 30); \
        gt_btn_set_text(_back, "Back >"); \
        gt_obj_set_fixed(_back, true); \
        gt_obj_add_event_cb(_back, _gt_example_back_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL); \
    } while(0)



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

#if GT_USE_EXAMPLES

/** @brief 通用返回按钮回调 */
void _gt_example_back_cb(gt_event_st * e);

/** @brief 初始化 example 导航系统（注册屏幕栈 + 加载菜单） */
void gt_examples_init(void);

/** @brief 导航菜单屏幕 */
gt_obj_st * gt_examples_menu(void);

/* --- 基础控件示例 --- */
gt_obj_st * gt_examples_anim1(void);
gt_obj_st * gt_examples_button(void);
gt_obj_st * gt_examples_checkbox(void);
gt_obj_st * gt_examples_img_button(void);
gt_obj_st * gt_examples_img(void);
gt_obj_st * gt_example_input(void);
gt_obj_st * gt_examples_keyboard(void);
gt_obj_st * gt_examples_label(void);
gt_obj_st * gt_examples_line(void);
gt_obj_st * gt_examples_listview(void);
gt_obj_st * gt_examples_progress_bar(void);
gt_obj_st * gt_examples_roller(void);
gt_obj_st * gt_examples_slider(void);
gt_obj_st * gt_examples_textarea(void);
gt_obj_st * gt_examples_radio(void);
gt_obj_st * gt_examples_switch(void);
gt_obj_st * gt_examples_clock(void);
gt_obj_st * gt_examples_rect(void);
gt_obj_st * gt_examples_arc(void);
gt_obj_st * gt_examples_select(void);
gt_obj_st * gt_examples_table(void);
gt_obj_st * gt_examples_graphs(void);
gt_obj_st * gt_examples_dialog(void);
gt_obj_st * gt_examples_calendar(void);
gt_obj_st * gt_examples_view_pager(void);
gt_obj_st * gt_examples_btnmap(void);
gt_obj_st * gt_examples_status_bar(void);
gt_obj_st * gt_examples_toast(void);
gt_obj_st * gt_examples_input_number(void);
gt_obj_st * gt_examples_barcode(void);
gt_obj_st * gt_examples_wordart(void);
gt_obj_st * gt_examples_player(void);
gt_obj_st * gt_examples_chat(void);
gt_obj_st * gt_examples_group(void);
gt_obj_st * gt_examples_cross_layout(void);
gt_obj_st * gt_examples_option(void);

/* --- 进阶示例 --- */
gt_obj_st * gt_examples_flex_layout(void);
gt_obj_st * gt_examples_dynamic_style(void);
gt_obj_st * gt_examples_form(void);

#endif  /* !GT_USE_EXAMPLES */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_EXAMPLES_H_
