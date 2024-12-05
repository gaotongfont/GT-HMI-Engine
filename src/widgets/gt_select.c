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
    draw_focus(obj, 0);
}

static void _select_deinit_cb(gt_obj_st * obj) {
    _gt_select_st * style = (_gt_select_st *)obj;
    if (style->lv_p) {
        gt_obj_set_visible(style->lv_p, false);
        gt_obj_destroy(style->lv_p);
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
    gt_obj_set_size(style->lv_p, obj->area.w, obj->area.h * 3);
    gt_obj_set_pos_always_full_display(style->lv_p, obj->area.x, obj->area.y + obj->area.h);
}

static void _select_event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code_val = gt_event_get_code(e);
    if (GT_EVENT_TYPE_INPUT_KEY_RELEASED == code_val || GT_EVENT_TYPE_INPUT_RELEASED == code_val) {
        _gt_select_st * style = (_gt_select_st *)obj;
        if (style->lv_p) {
            gt_obj_set_visible(style->lv_p, true);
            gt_obj_set_focus(gt_disp_get_layer_top(), true);
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

    char * item = gt_listview_get_selected_item_text(lv);
    if (NULL == item) { return; }
    gt_input_set_value(style->ipt_p, item);
    gt_obj_set_visible(lv, false);
    gt_event_send(select, GT_EVENT_TYPE_UPDATE_VALUE, item);
}

static gt_obj_st * _create_listview(gt_obj_st * obj) {
    gt_obj_st * lv_p = gt_listview_create(gt_disp_get_layer_top());
    gt_obj_set_size(lv_p, obj->area.w, obj->area.h * 3);
    gt_obj_add_event_cb(lv_p, _select_item_cb, GT_EVENT_TYPE_INPUT_RELEASED, obj);
    gt_obj_add_event_cb(lv_p, _select_item_cb, GT_EVENT_TYPE_INPUT_KEY_RELEASED, obj);
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
    if (NULL == style->lv_p) {
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
    if (NULL == style->lv_p) {
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
    if (NULL == style->lv_p) {
        return;
    }
    gt_listview_set_item_height(style->lv_p, height);
}

#endif  /** GT_CFG_ENABLE_SELECT */
/* end ------------------------------------------------------------------*/
