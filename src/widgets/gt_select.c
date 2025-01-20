/**
 * @file gt_select.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-12-03 15:59:13
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_select.h"

#if GT_CFG_ENABLE_SELECT
#include "../core/gt_mem.h"
#include "../hal/gt_hal_disp.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../core/gt_event.h"
#include "./gt_input.h"
#include "./gt_listview.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_SELECT
#define MY_CLASS    &gt_select_class

#define _DEFAULT_OPTION_COUNT       3


/* private typedef ------------------------------------------------------*/
typedef struct _gt_select_s {
    gt_obj_st obj;
    gt_obj_st * ipt_p;      /** input */
    gt_obj_st * lv_p;       /** listview */

}_gt_select_st;


/* static variables -----------------------------------------------------*/
static void _select_init_cb(gt_obj_st * obj);
static void _select_deinit_cb(gt_obj_st * obj);
static void _select_event_cb(struct gt_obj_s * obj, gt_event_st * e);

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_select_class = {
    ._init_cb      = _select_init_cb,
    ._deinit_cb    = _select_deinit_cb,
    ._event_cb     = _select_event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_select_st)
};



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void _select_init_cb(gt_obj_st * obj) {
    draw_focus(obj);
}

static void _select_deinit_cb(gt_obj_st * obj) {
    _gt_select_st * style = (_gt_select_st *)obj;
    if (style->lv_p) {
        gt_obj_destroy_immediately(style->lv_p);
        style->lv_p = NULL;
    }
}

static void _update_input_area(gt_obj_st * obj) {
    _gt_select_st * style = (_gt_select_st *)obj;
    if (NULL == style->ipt_p) {
        return;
    }
    gt_area_st area = obj->area;
    gt_obj_set_area(style->ipt_p, obj->area);
}


static void _update_lisview_area(gt_obj_st * obj) {
    _gt_select_st * style = (_gt_select_st *)obj;
    if (NULL == style->lv_p) {
        return;
    }
    gt_obj_set_size(style->lv_p, obj->area.w,
        gt_listview_get_resize_height(style->lv_p, _DEFAULT_OPTION_COUNT));
    gt_obj_set_pos_always_full_display(style->lv_p, obj->area.x, obj->area.y + obj->area.h);
}

static void _select_event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code_val = gt_event_get_code(e);
    if (GT_EVENT_TYPE_INPUT_RELEASED == code_val) {
        _gt_select_st * style = (_gt_select_st *)obj;
        if (style->lv_p) {
            gt_obj_set_visible(style->lv_p, true);
        }
    } else if (GT_EVENT_TYPE_UPDATE_STYLE == code_val) {
        _update_input_area(obj);
        _update_lisview_area(obj);
    }
}

static gt_obj_st * _create_input(gt_obj_st * obj) {
    gt_obj_st * ipt_p = gt_input_create(obj);
    gt_obj_set_size(ipt_p, obj->area.w, obj->area.h);
    gt_input_set_placeholder(ipt_p, "Please select");
    gt_obj_set_touch_parent(ipt_p, true);
    gt_obj_set_focus_disabled(ipt_p, GT_DISABLED);
    return ipt_p;
}

static void _select_item_cb(gt_event_st * e) {
    gt_obj_st * select = (gt_obj_st *)e->user_data;
    gt_obj_st * lv = e->target;
    _gt_select_st * style = (_gt_select_st *)select;
    if (false == gt_obj_is_type(select, OBJ_TYPE)) {
        return;
    }
    char * item = gt_listview_get_selected_item_text(lv);
    if (NULL == item) { return; }
    gt_input_set_value(style->ipt_p, item);
    gt_obj_set_visible(lv, false);
    gt_event_send(select, GT_EVENT_TYPE_UPDATE_VALUE, item);
}

static gt_obj_st * _create_listview(gt_obj_st * obj) {
    gt_obj_st * lv_p = gt_listview_create(gt_disp_get_layer_top());
    gt_obj_set_size(lv_p, obj->area.w, gt_listview_get_resize_height(lv_p, _DEFAULT_OPTION_COUNT));
    gt_obj_add_event_cb(lv_p, _select_item_cb, GT_EVENT_TYPE_INPUT_RELEASED, obj);
    gt_obj_set_visible(lv_p, false);
    gt_obj_set_focus_disabled(lv_p, GT_DISABLED);
    return lv_p;
}

/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_select_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    obj->area.w = 180;
    obj->area.h = 30;
    obj->focus_dis = GT_ENABLED;

    _gt_select_st * style = (_gt_select_st *)obj;
    style->ipt_p = _create_input(obj);
    style->lv_p = _create_listview(obj);

    return obj;
}

void gt_select_add_option(gt_obj_st * obj, const char * text)
{
    _gt_select_st * style = (_gt_select_st *)obj;
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    gt_listview_add_item(style->lv_p, text);
}

void gt_select_clear_all_options(gt_obj_st * obj)
{
    _gt_select_st * style = (_gt_select_st *)obj;
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    gt_listview_clear_all_items(style->lv_p);
}

void gt_select_set_option_height(gt_obj_st * obj, uint16_t height)
{
    _gt_select_st * style = (_gt_select_st *)obj;
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    gt_listview_set_item_height(style->lv_p, height);
}

void gt_select_set_font_color(gt_obj_st * select, gt_color_t color)
{
    if (false == gt_obj_is_type(select, OBJ_TYPE)) {
        return ;
    }
    _gt_select_st * style = (_gt_select_st * )select;
    gt_input_set_font_color(style->ipt_p, color);
    gt_listview_set_font_color(style->lv_p, color);
}

void gt_select_set_font_size(gt_obj_st * select, uint8_t size)
{
    if (false == gt_obj_is_type(select, OBJ_TYPE)) {
        return ;
    }
    _gt_select_st * style = (_gt_select_st * )select;
    gt_input_set_font_size(style->ipt_p, size);
    gt_listview_set_font_size(style->lv_p, size);
}

void gt_select_set_font_gray(gt_obj_st * select, uint8_t gray)
{
    if (false == gt_obj_is_type(select, OBJ_TYPE)) {
        return ;
    }
    _gt_select_st * style = (_gt_select_st * )select;
    gt_input_set_font_gray(style->ipt_p, gray);
    gt_listview_set_font_gray(style->lv_p, gray);
}

void gt_select_set_font_align(gt_obj_st * select, gt_align_et align)
{
    if (false == gt_obj_is_type(select, OBJ_TYPE)) {
        return ;
    }
    _gt_select_st * style = (_gt_select_st * )select;
    gt_input_set_font_align(style->ipt_p, align);
    gt_listview_set_font_align(style->lv_p, align);
}
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
void gt_select_set_font_family_cn(gt_obj_st * select, gt_family_t family)
{
    if (false == gt_obj_is_type(select, OBJ_TYPE)) {
        return ;
    }
    _gt_select_st * style = (_gt_select_st * )select;
    gt_input_set_font_family_cn(style->ipt_p, family);
    gt_listview_set_font_family_cn(style->lv_p, family);
}
void gt_select_set_font_family_en(gt_obj_st * select, gt_family_t family)
{
    if (false == gt_obj_is_type(select, OBJ_TYPE)) {
        return ;
    }
    _gt_select_st * style = (_gt_select_st * )select;
    gt_input_set_font_family_en(style->ipt_p, family);
    gt_listview_set_font_family_en(style->lv_p, family);
}

void gt_select_set_font_family_fl(gt_obj_st * select, gt_family_t family)
{
    if (false == gt_obj_is_type(select, OBJ_TYPE)) {
        return ;
    }
    _gt_select_st * style = (_gt_select_st * )select;
    gt_input_set_font_family_fl(style->ipt_p, family);
    gt_listview_set_font_family_fl(style->lv_p, family);
}
void gt_select_set_font_family_numb(gt_obj_st * select, gt_family_t family)
{
    if (false == gt_obj_is_type(select, OBJ_TYPE)) {
        return ;
    }
    _gt_select_st * style = (_gt_select_st * )select;
    gt_input_set_font_family_numb(style->ipt_p, family);
    gt_listview_set_font_family_numb(style->lv_p, family);
}
#else
void gt_select_set_font_family(gt_obj_st * select, gt_family_t family)
{
    if (false == gt_obj_is_type(select, OBJ_TYPE)) {
        return ;
    }
    _gt_select_st * style = (_gt_select_st * )select;
    gt_input_set_font_family(style->ipt_p, family);
    gt_listview_set_font_family(style->lv_p, family);
}
void gt_select_set_font_cjk(gt_obj_st* select, gt_font_cjk_et cjk)
{
    if (false == gt_obj_is_type(select, OBJ_TYPE)) {
        return ;
    }
    _gt_select_st * style = (_gt_select_st * )select;
    gt_input_set_font_cjk(style->ipt_p, cjk);
    gt_listview_set_font_cjk(style->lv_p, cjk);
}
#endif

void gt_select_set_font_thick_en(gt_obj_st * select, uint8_t thick)
{
    if (false == gt_obj_is_type(select, OBJ_TYPE)) {
        return ;
    }
    _gt_select_st * style = (_gt_select_st * )select;
    gt_input_set_font_thick_en(style->ipt_p, thick);
    gt_listview_set_font_thick_en(style->lv_p, thick);
}

void gt_select_set_font_thick_cn(gt_obj_st * select, uint8_t thick)
{
    if (false == gt_obj_is_type(select, OBJ_TYPE)) {
        return ;
    }
    _gt_select_st * style = (_gt_select_st * )select;
    gt_input_set_font_thick_cn(style->ipt_p, thick);
    gt_listview_set_font_thick_cn(style->lv_p, thick);
}

void gt_select_set_font_encoding(gt_obj_st * select, gt_encoding_et encoding)
{
    if (false == gt_obj_is_type(select, OBJ_TYPE)) {
        return ;
    }
    _gt_select_st * style = (_gt_select_st * )select;
    gt_input_set_font_encoding(style->ipt_p, encoding);
    gt_listview_set_font_encoding(style->lv_p, encoding);
}

void gt_select_set_font_style(gt_obj_st * select, gt_font_style_et font_style)
{
    if (false == gt_obj_is_type(select, OBJ_TYPE)) {
        return ;
    }
    _gt_select_st * style = (_gt_select_st * )select;
    gt_input_set_font_style(style->ipt_p, font_style);
    gt_listview_set_font_style(style->lv_p, font_style);
    gt_event_send(select, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_select_set_selected_anti_font(gt_obj_st * select, bool enabled)
{
    if (false == gt_obj_is_type(select, OBJ_TYPE)) {
        return ;
    }
    _gt_select_st * style = (_gt_select_st * )select;
    gt_listview_set_selected_anti_font(style->lv_p, enabled);
}

bool gt_select_is_selected_anti_font(gt_obj_st * select)
{
    if (false == gt_obj_is_type(select, OBJ_TYPE)) {
        return false;
    }
    _gt_select_st * style = (_gt_select_st * )select;
    return gt_listview_is_selected_anti_font(style->lv_p);
}
void gt_select_set_selected_anti_font_color(gt_obj_st * select, gt_color_t anti_color)
{
    if (false == gt_obj_is_type(select, OBJ_TYPE)) {
        return ;
    }
    _gt_select_st * style = (_gt_select_st * )select;
    gt_listview_set_selected_anti_font_color(style->lv_p, anti_color);
}
gt_color_t gt_select_get_selected_anti_font_color(gt_obj_st * select)
{
    if (false == gt_obj_is_type(select, OBJ_TYPE)) {
        return gt_color_black();
    }
    _gt_select_st * style = (_gt_select_st * )select;
    return gt_listview_get_selected_anti_font_color(style->lv_p);
}


#endif  /** GT_CFG_ENABLE_SELECT */
/* end ------------------------------------------------------------------*/
