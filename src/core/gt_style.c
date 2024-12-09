/**
 * @file gt_style.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-12 18:37:55
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

 /* include --------------------------------------------------------------*/
#include "gt_style.h"
#include "./gt_disp.h"
#include "../core/gt_mem.h"
#include "../core/gt_layout.h"
#include "../hal/gt_hal_disp.h"
#include "../others/gt_log.h"
#include "../others/gt_anim.h"

/* private define -------------------------------------------------------*/
#define _set_style_prop_val(obj, prop, val)     (obj->style->prop = val)
// #define _set_area_prop_val(obj, prop, val)      (obj->area.prop = val)  // unused
#define _get_area_prop(obj, prop)               (obj->area.prop)

/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static GT_ATTRIBUTE_RAM_TEXT void _calc_full_display_pos(gt_obj_st * obj, gt_area_st * new_area_p) {
    gt_obj_st * scr = gt_disp_get_scr();
    if (new_area_p->x < scr->area.x) {
        new_area_p->x = scr->area.x;
    } else if (new_area_p->x + obj->area.w > scr->area.x + scr->area.w) {
        new_area_p->x = scr->area.x + scr->area.w - obj->area.w;
    }
    if (new_area_p->y < scr->area.y) {
        new_area_p->y = scr->area.y;
    } else if (new_area_p->y + obj->area.h > scr->area.y + scr->area.h) {
        new_area_p->y = scr->area.y + scr->area.h - obj->area.h;
    }
}

#if GT_USE_LAYER_TOP
static GT_ATTRIBUTE_RAM_TEXT void _calc_layer_top_full_display_pos(gt_obj_st * obj, gt_disp_st * disp,
    gt_area_st * new_area_p) {
    if (new_area_p->x < 0) {
        new_area_p->x = 0;
    } else if (new_area_p->x + obj->area.w > disp->layer_top->area.w) {
        new_area_p->x = disp->layer_top->area.w - obj->area.w;
    }

    if (new_area_p->y < 0) {
        new_area_p->y = 0;
    } else if (new_area_p->y + obj->area.h > disp->layer_top->area.h) {
        new_area_p->y = disp->layer_top->area.h - obj->area.h;
    }
}

/**
 * @brief
 *
 * @param obj
 * @param x value reset to: x % width
 * @param y value reset to: y % height
 * @param is_full_display false[default]: normal position;
 *      true: always full display on the screen
 * @return true
 * @return false
 */
static GT_ATTRIBUTE_RAM_TEXT bool _update_layer_top_widget_area(gt_obj_st * obj, gt_area_st new_area, bool is_full_display) {
    GT_CHECK_BACK_VAL(obj, false);
    gt_disp_st * disp = gt_disp_get_default();
    if (NULL == disp->layer_top) {
        return false;
    }
    if (0 == disp->layer_top->cnt_child) {
        return false;
    }
    gt_obj_st * ptr = obj;
    while(ptr->parent) {
        ptr = ptr->parent;
    }
    if (ptr != disp->layer_top) {
        return false;
    }

    if (new_area.x > disp->layer_top->area.w) {
        new_area.x = new_area.x % disp->layer_top->area.w;
    }
    if (new_area.y > disp->layer_top->area.h) {
        new_area.y = new_area.y % disp->layer_top->area.h;
    }
    new_area.x += disp->layer_top->area.x;
    new_area.y += disp->layer_top->area.y;

    if (is_full_display) {
        _calc_layer_top_full_display_pos(obj, disp, &new_area);
    }
    _gt_obj_move_child_by(obj, new_area.x - gt_obj_get_x(obj), new_area.y - gt_obj_get_y(obj));

    gt_obj_pos_change(obj, &new_area);
    _gt_disp_update_max_area(&new_area, _gt_obj_is_ignore_calc_max_area(obj));
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_STYLE, NULL);

    return true;
}
#endif

static GT_ATTRIBUTE_RAM_TEXT void _opa_anim_exec_cb(void * obj, int32_t val) {
    gt_obj_st * tar = (gt_obj_st *)obj;
    tar->opa = (gt_opa_t)val;
    if (false == gt_obj_get_visible(tar)) {
        return;
    }
    gt_event_send(tar, GT_EVENT_TYPE_DRAW_START, NULL);
}

static GT_ATTRIBUTE_RAM_TEXT void _width_anim_exec_cb(void * obj, int32_t val) {
    gt_obj_st * tar = (gt_obj_st * )obj;
    gt_obj_set_size(tar, val, tar->area.h);
}

static GT_ATTRIBUTE_RAM_TEXT void _height_anim_exec_cb(void * obj, int32_t val) {
    gt_obj_st * tar = (gt_obj_st * )obj;
    gt_obj_set_size(tar, tar->area.w, val);
}

static GT_ATTRIBUTE_RAM_TEXT void _x_anim_exec_cb(void * obj, int32_t val) {
    gt_obj_st * tar = (gt_obj_st * )obj;
    gt_obj_set_pos(tar, val, tar->area.y);
}

static GT_ATTRIBUTE_RAM_TEXT void _y_anim_exec_cb(void * obj, int32_t val) {
    gt_obj_st * tar = (gt_obj_st * )obj;
    gt_obj_set_pos(tar, tar->area.x, val);
}

static GT_ATTRIBUTE_RAM_TEXT void _radius_anim_exec_cb(void * obj, int32_t val) {
    gt_obj_st * tar = (gt_obj_st * )obj;
    tar->radius = val;
    gt_event_send(tar, GT_EVENT_TYPE_DRAW_START, NULL);
}

/* global functions / API interface -------------------------------------*/
gt_res_t gt_obj_set_area(gt_obj_st * obj, gt_area_st area)
{
    GT_CHECK_BACK_VAL(obj, GT_RES_FAIL);
#if GT_USE_LAYER_TOP
    if (_update_layer_top_widget_area(obj, area, false)) {
        return GT_RES_OK;
    }
#endif
    gt_obj_pos_change(obj, &area);
    _gt_disp_update_max_area(&area, _gt_obj_is_ignore_calc_max_area(obj));
    return GT_RES_OK;
}

void gt_obj_set_pos(gt_obj_st * obj, gt_size_t x, gt_size_t y)
{
    GT_CHECK_BACK(obj);
    gt_area_st area = obj->area;
#if GT_USE_LAYER_TOP
    area.x = x;
    area.y = y;
    if (_update_layer_top_widget_area(obj, area, false)) {
        return ;
    }
#endif
    /** normal screen */
    area.x = x;
    area.y = y;
    area.w = obj->area.w;
    area.h = obj->area.h;
    _gt_obj_move_child_by(obj, area.x - obj->area.x, area.y - obj->area.y);
    if (gt_obj_get_virtual(obj)) {
        return ;
    }
    gt_obj_set_area(obj, area);
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_STYLE, NULL);
}

void gt_obj_set_pos_anim(gt_obj_st * obj, gt_size_t x, gt_size_t y)
{
    GT_CHECK_BACK(obj);
    gt_anim_st anim = {0};
    gt_anim_init(&anim);
    gt_anim_set_target(&anim, obj);
    gt_anim_set_path_type(&anim, GT_ANIM_PATH_TYPE_EASE_IN_OUT);
    gt_anim_set_time(&anim, GT_ANIM_PERIOD_TRANSITION_EFFECT_MS);
    if (obj->area.x != x) {
        gt_anim_set_value(&anim, obj->area.x, x);
        gt_anim_set_exec_cb(&anim, _x_anim_exec_cb);
        gt_anim_start(&anim);
    }
    if (obj->area.y != y) {
        gt_anim_set_value(&anim, obj->area.y, y);
        gt_anim_set_exec_cb(&anim, _y_anim_exec_cb);
        gt_anim_start(&anim);
    }
}

#if GT_USE_CUSTOM_TOUCH_EXPAND_SIZE
void gt_obj_set_touch_expand_area(gt_obj_st * obj, gt_size_t hor, gt_size_t ver)
{
    GT_CHECK_BACK(obj);
    obj->touch_expand_size.x = hor < 0 ? 0 : hor;
    obj->touch_expand_size.y = ver < 0 ? 0 : ver;
}
#endif  /** GT_USE_CUSTOM_TOUCH_EXPAND_SIZE */

void gt_obj_set_pos_relative(gt_obj_st * obj, gt_obj_st * target, gt_size_t diff_x, gt_size_t diff_y)
{
    GT_CHECK_BACK(obj);
    if (NULL == target) {
        target = obj->parent;
        GT_CHECK_BACK(target);
    }
    gt_area_st area = {
        .x = target->area.x + diff_x,
        .y = target->area.y + diff_y,
        .w = obj->area.w,
        .h = obj->area.h
    };
#if GT_USE_LAYER_TOP
    if (_update_layer_top_widget_area(obj, area, false)) {
        return ;
    }
#endif
    /** normal screen */
    _gt_obj_move_child_by(obj, area.x - obj->area.x, area.y - obj->area.y);
    if (gt_obj_get_virtual(obj)) {
        return ;
    }
    gt_obj_set_area(obj, area);
}

void gt_obj_set_pos_always_full_display(gt_obj_st * obj, gt_size_t x, gt_size_t y)
{
    GT_CHECK_BACK(obj);
    gt_area_st area = obj->area;
#if GT_USE_LAYER_TOP
    area.x = x;
    area.y = y;
    if (_update_layer_top_widget_area(obj, area, true)) {
        return ;
    }
#endif
    /** normal screen */
    area.x = x;
    area.y = y;
    area.w = obj->area.w;
    area.h = obj->area.h;
    _calc_full_display_pos(obj, &area);
    _gt_obj_move_child_by(obj, area.x - obj->area.x, area.y - obj->area.y);
    if (gt_obj_get_virtual(obj)) {
        return ;
    }
    gt_obj_set_area(obj, area);
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_STYLE, NULL);
}

void _gt_obj_move_child_by(gt_obj_st * obj, gt_size_t diff_x, gt_size_t diff_y)
{
    GT_CHECK_BACK(obj);
    if (0 == diff_x && 0 == diff_y) {
        return;
    }
    for (uint16_t i = 0, cnt = obj->cnt_child; i < cnt; i++) {
        gt_obj_st * child = obj->child[i];
        child->area.x += diff_x;
        child->area.y += diff_y;

        _gt_obj_move_child_by(child, diff_x, diff_y);
    }
}

void gt_obj_set_size(gt_obj_st * obj, uint16_t w, uint16_t h)
{
    GT_CHECK_BACK(obj);
    if (gt_obj_get_virtual(obj)) {
        return ;
    }
    gt_area_st area = obj->area;
    area.w = w;
    area.h = h;
    gt_obj_size_change(obj, &area);
    _gt_disp_update_max_area(&area, _gt_obj_is_ignore_calc_max_area(obj));
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_STYLE, NULL);
}

void gt_obj_set_size_anim(gt_obj_st * obj, uint16_t w, uint16_t h)
{
    GT_CHECK_BACK(obj);
    if (gt_obj_get_virtual(obj)) {
        return ;
    }
    gt_anim_st anim = {0};
    gt_anim_init(&anim);
    gt_anim_set_target(&anim, obj);
    gt_anim_set_path_type(&anim, GT_ANIM_PATH_TYPE_EASE_IN_OUT);
    gt_anim_set_time(&anim, GT_ANIM_PERIOD_TRANSITION_EFFECT_MS);
    if (obj->area.w != w) {
        gt_anim_set_value(&anim, obj->area.w, w);
        gt_anim_set_exec_cb(&anim, _width_anim_exec_cb);
        gt_anim_start(&anim);
    }
    if (obj->area.h != h) {
        gt_anim_set_value(&anim, obj->area.h, h);
        gt_anim_set_exec_cb(&anim, _height_anim_exec_cb);
        gt_anim_start(&anim);
    }
}

void gt_obj_set_x(gt_obj_st * obj, gt_size_t x)
{
    GT_CHECK_BACK(obj);
    gt_obj_set_pos(obj, x, obj->area.y);
}

void gt_obj_set_x_anim(gt_obj_st * obj, gt_size_t x)
{
    GT_CHECK_BACK(obj);
    gt_obj_set_pos_anim(obj, x, obj->area.y);
}

void gt_obj_set_y(gt_obj_st * obj, gt_size_t y)
{
    GT_CHECK_BACK(obj);
    gt_obj_set_pos(obj, obj->area.x, y);
}

void gt_obj_set_y_anim(gt_obj_st * obj, gt_size_t y)
{
    GT_CHECK_BACK(obj);
    gt_obj_set_pos_anim(obj, obj->area.x, y);
}

void gt_obj_set_w(gt_obj_st * obj, uint16_t w)
{
    gt_obj_set_size(obj, w, obj->area.h);
}

void gt_obj_set_w_anim(gt_obj_st * obj, uint16_t w)
{
    gt_obj_set_size_anim(obj, w, obj->area.h);
}

void gt_obj_set_h(gt_obj_st * obj, uint16_t h)
{
    gt_obj_set_size(obj, obj->area.w, h);
}

void gt_obj_set_h_anim(gt_obj_st * obj, uint16_t h)
{
    gt_obj_set_size_anim(obj, obj->area.w, h);
}

void gt_obj_set_opa(gt_obj_st * obj, gt_opa_t opa)
{
    GT_CHECK_BACK(obj);
    obj->opa = opa;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_obj_set_opa_anim(gt_obj_st * obj, gt_opa_t opa)
{
    GT_CHECK_BACK(obj);
    if (obj->opa == opa) {
        return;
    }
    gt_anim_st anim = {0};
    gt_anim_init(&anim);
    gt_anim_set_target(&anim, obj);
    gt_anim_set_path_type(&anim, GT_ANIM_PATH_TYPE_EASE_IN_OUT);
    gt_anim_set_time(&anim, GT_ANIM_PERIOD_TRANSITION_EFFECT_MS);
    gt_anim_set_value(&anim, obj->opa, opa);
    gt_anim_set_exec_cb(&anim, _opa_anim_exec_cb);
    gt_anim_start(&anim);
}

gt_size_t gt_obj_get_x(gt_obj_st * obj)
{
    GT_CHECK_BACK_VAL(obj, 0);
    return _get_area_prop(obj, x);
}

gt_size_t gt_obj_get_y(gt_obj_st * obj)
{
    GT_CHECK_BACK_VAL(obj, 0);
    return _get_area_prop(obj, y);
}
uint16_t gt_obj_get_w(gt_obj_st * obj)
{
    GT_CHECK_BACK_VAL(obj, 0);
    return _get_area_prop(obj, w);
}
uint16_t gt_obj_get_h(gt_obj_st * obj)
{
    GT_CHECK_BACK_VAL(obj, 0);
    return _get_area_prop(obj, h);
}

void gt_obj_set_visible(gt_obj_st * obj, gt_visible_et is_visible)
{
    GT_CHECK_BACK(obj);
    obj->visible = is_visible;
    gt_disp_invalid_area(obj);
    if (obj->visible) {
        _gt_disp_reload_max_area(gt_obj_within_which_scr(obj));
    }
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

gt_visible_et gt_obj_get_visible(gt_obj_st * obj)
{
    GT_CHECK_BACK_VAL(obj, GT_INVISIBLE);
    return (gt_visible_et)obj->visible;
}

void gt_obj_set_disabled(gt_obj_st * obj, gt_disabled_et is_disabled)
{
    GT_CHECK_BACK(obj);
    obj->disabled = is_disabled ? 1 : 0;
    obj->opa = obj->disabled ? GT_OPA_60 : GT_OPA_COVER;
    gt_disp_invalid_area(obj);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

bool gt_obj_is_disabled(gt_obj_st * obj)
{
    GT_CHECK_BACK_VAL(obj, false);
    return obj->disabled;
}

void gt_obj_set_focus(gt_obj_st * obj, bool is_focus)
{
    GT_CHECK_BACK(obj);
    obj->focus = is_focus;
    gt_disp_invalid_area(obj);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

bool gt_obj_is_focus(gt_obj_st * obj)
{
    return obj ? obj->focus : false;
}

void gt_obj_set_focus_disabled(gt_obj_st * obj, gt_disabled_et is_disabled)
{
    GT_CHECK_BACK(obj);
    obj->focus_dis = is_disabled ? 1 : 0;

    if (GT_DISABLED == obj->focus_dis) {
        gt_disp_invalid_area(obj);
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
    }
}

bool gt_obj_is_focus_disabled(gt_obj_st * obj)
{
    GT_CHECK_BACK_VAL(obj, false);
    return obj->focus_dis;
}

void gt_obj_set_focus_skip(gt_obj_st * obj, bool is_skip)
{
    GT_CHECK_BACK(obj);
    obj->focus_skip = is_skip;
}

bool gt_obj_is_focus_skip(gt_obj_st * obj)
{
    GT_CHECK_BACK_VAL(obj, false);
    return obj->focus_skip;
}

void gt_obj_size_change(gt_obj_st * obj, gt_area_st * area_new)
{
    GT_CHECK_BACK(obj);
    gt_area_st area_old = obj->area;
    gt_area_st area;
    if (area_new->x <= area_old.x) {
        area.x = area_new->x;
    } else {
        area.x = area_old.x;
    }

    if (area_new->y <= area_old.y) {
        area.y = area_new->y;
    } else {
        area.y = area_old.y;
    }

    /* need refactor */
    if (area_new->w <= area_old.w) {
        area.w = area_old.w;
    } else {
        area.w = area_new->w;
    }

    if (area_new->h <= area_old.h) {
        area.h = area_old.h;
    } else {
        area.h = area_new->h;
    }
    obj->area = *area_new;
    _gt_disp_refr_append_area(&area);
}

void gt_obj_pos_change(gt_obj_st * obj, gt_area_st * area_new)
{
    GT_CHECK_BACK(obj);
    GT_CHECK_BACK(area_new);
    gt_area_st area_old = obj->area;
    gt_area_st area;

    if (area_new->x <= area_old.x) {
        area.x = area_new->x;
    } else {
        area.x = area_old.x;
    }

    if (area_new->y <= area_old.y) {
        area.y = area_new->y;
    } else {
        area.y = area_old.y;
    }

    int y2_old = area_old.y + area_old.h, x2_old = area_old.x + area_old.w;
    int y2_new = area_new->y + area_new->h, x2_new = area_new->x + area_new->w;

    if (y2_old <= y2_new) {
        area.h = y2_new - area.y;
    } else {
        area.h = y2_old - area.y;
    }

    if (x2_old <= x2_new) {
        area.w = x2_new - area.x;
    } else {
        area.w = x2_old - area.x;
    }

    obj->area = *area_new;
    _gt_disp_refr_append_area(&area);
}

void gt_obj_child_set_prop(gt_obj_st * obj, gt_obj_prop_type_em type, uint8_t val)
{
    GT_CHECK_BACK(obj);
    uint16_t i, cnt = obj->cnt_child;
    for (i = 0; i < cnt; i++) {
        gt_obj_st * child = obj->child[i];
        switch (type) {
            case GT_OBJ_PROP_TYPE_VISIBLE: {
                gt_obj_set_visible(child, (gt_visible_et)val);
                break;
            }
            case GT_OBJ_PROP_TYPE_DELATE: {
                child->delate = val;
                break;
            }
            case GT_OBJ_PROP_TYPE_FOCUS: {
                gt_obj_set_focus(child, val);
                break;
            }
            case GT_OBJ_PROP_TYPE_FOCUS_DISABLED: {
                gt_obj_set_focus_disabled(child, (gt_disabled_et)val);
                break;
            }
            case GT_OBJ_PROP_TYPE_DISABLED: {
                gt_obj_set_disabled(child, val);
                break;
            }
            case GT_OBJ_PROP_TYPE_FIXED: {
                gt_obj_set_fixed(child, val);
                break;
            }
            case GT_OBJ_PROP_TYPE_OVERFLOW: {
                gt_obj_set_overflow(child, val);
                break;
            }
            case GT_OBJ_PROP_TYPE_INSIDE: {
                gt_obj_set_inside(child, val);
                break;
            }
            case GT_OBJ_PROP_TYPE_OPA: {
                gt_obj_set_opa(child, (gt_opa_t)val);
                break;
            }
            case GT_OBJ_PROP_TYPE_UNTOUCHABILITY: {
                gt_obj_set_untouchability(child, val);
                break;
            }
            default:
                break;
        }
        if (0 == child->cnt_child) {
            continue;
        }
        gt_obj_child_set_prop(child, type, val);
    }
}

void gt_obj_set_state(gt_obj_st * obj, gt_state_et state)
{
    if (NULL == obj) {
        return;
    }
    obj->state = state;
}

gt_state_et gt_obj_get_state(gt_obj_st * obj)
{
    GT_CHECK_BACK_VAL(obj, GT_STATE_NONE);
    return (gt_state_et)obj->state;
}

void gt_obj_set_fixed(gt_obj_st * obj, bool is_fixed)
{
    GT_CHECK_BACK(obj);
    obj->fixed = is_fixed;
}

bool gt_obj_get_fixed(gt_obj_st * obj)
{
    GT_CHECK_BACK_VAL(obj, false);
    return obj->fixed;
}

void gt_obj_set_overflow(gt_obj_st * obj, bool is_overflow)
{
    GT_CHECK_BACK(obj);
    obj->overflow = is_overflow;
}

bool gt_obj_get_overflow(gt_obj_st * obj)
{
    GT_CHECK_BACK_VAL(obj, false);
    return obj->overflow;
}

void gt_obj_set_inside(gt_obj_st * obj, bool is_inside)
{
    GT_CHECK_BACK(obj);
    obj->inside = is_inside;
}

bool gt_obj_get_inside(gt_obj_st * obj)
{
    GT_CHECK_BACK_VAL(obj, false);
    return obj->inside;
}

void gt_obj_set_mask_effect(gt_obj_st * obj, bool is_keep_alive)
{
    GT_CHECK_BACK(obj);
    obj->mask_effect = is_keep_alive;
}

bool gt_obj_get_mask_effect(gt_obj_st * obj)
{
    GT_CHECK_BACK_VAL(obj, false);
    return obj->mask_effect;
}

void gt_obj_set_touch_parent(gt_obj_st * obj, bool is_touch_parent)
{
    GT_CHECK_BACK(obj);
    obj->touch_parent = is_touch_parent;
}

bool gt_obj_get_touch_parent(gt_obj_st * obj)
{
    GT_CHECK_BACK_VAL(obj, false);
    return obj->touch_parent;
}

void gt_obj_set_trigger_mode(gt_obj_st * obj, gt_obj_trigger_mode_et is_trigger_mode)
{
    GT_CHECK_BACK(obj);
    obj->trigger_mode = is_trigger_mode;
}

gt_obj_trigger_mode_et gt_obj_get_trigger_mode(gt_obj_st * obj)
{
    if (NULL == obj) {
        return GT_OBJ_TRIGGER_MODE_HOLD_ON;
    }
    return (gt_obj_trigger_mode_et)obj->trigger_mode;
}

void gt_obj_set_virtual(gt_obj_st * obj, bool is_virtual)
{
    GT_CHECK_BACK(obj);
    obj->virtuality = is_virtual;
}

bool gt_obj_get_virtual(gt_obj_st * obj)
{
    GT_CHECK_BACK_VAL(obj, false);
    return obj->virtuality;
}

void gt_obj_set_scroll_dir(gt_obj_st * obj, gt_scroll_dir_et dir)
{
    GT_CHECK_BACK(obj);
    if (dir < GT_SCROLL_DISABLE || dir > GT_SCROLL_ALL) {
        obj->scroll_dir = GT_SCROLL_ALL;
        return;
    }

    obj->scroll_dir = dir;
}

gt_scroll_dir_et gt_obj_get_scroll_dir(gt_obj_st * obj)
{
    return (gt_scroll_dir_et)obj->scroll_dir;
}

bool gt_obj_is_scroll_dir(gt_obj_st * obj, gt_scroll_dir_et dir)
{
    return (obj->scroll_dir & dir) ? true : false;
}

void gt_obj_set_scroll_lr(gt_obj_st * obj, bool is_scroll_right)
{
    obj->scroll_l_r = is_scroll_right;
}

bool gt_obj_is_scroll_left(gt_obj_st * obj)
{
    return 0 == obj->scroll_l_r ? true : false;
}

bool gt_obj_is_scroll_right(gt_obj_st * obj)
{
    return obj->scroll_l_r ? true : false;
}

void gt_obj_set_scroll_ud(gt_obj_st * obj, bool is_scroll_down)
{
    obj->scroll_u_d = is_scroll_down;
}

bool gt_obj_is_scroll_up(gt_obj_st * obj)
{
    return 0 == obj->scroll_u_d ? true : false;
}

bool gt_obj_is_scroll_down(gt_obj_st * obj)
{
    return obj->scroll_u_d ? true : false;
}

void gt_obj_set_scroll_snap_x(gt_obj_st * obj, gt_scroll_snap_em snap)
{
    obj->scroll_snap_x = snap;
}

void gt_obj_set_scroll_snap_y(gt_obj_st * obj, gt_scroll_snap_em snap)
{
    obj->scroll_snap_y = snap;
}

gt_scroll_snap_em gt_obj_get_scroll_snap_x(gt_obj_st * obj)
{
    return (gt_scroll_snap_em)obj->scroll_snap_x;
}

gt_scroll_snap_em gt_obj_get_scroll_snap_y(gt_obj_st * obj)
{
    return (gt_scroll_snap_em)obj->scroll_snap_y;
}

bool _gt_obj_is_ignore_calc_max_area(gt_obj_st * obj)
{
    gt_obj_st * ptr = obj;
    if (NULL == obj) { return true; }

    while (ptr->parent) { ptr = ptr->parent; }

    if (obj->overflow) { return true; }
    if (obj->virtuality) { return true; }
    if (obj->parent == ptr) {
        return false;
    }

    if (obj->inside) { return true; }

    return false;
}

void gt_obj_set_bubble_notify(gt_obj_st * obj, bool is_bubble_notify)
{
    GT_CHECK_BACK(obj);
    obj->bubble_notify = is_bubble_notify ? true : false;
}

bool gt_obj_is_bubble_notify(gt_obj_st * obj)
{
    GT_CHECK_BACK_VAL(obj, false);
    return obj->bubble_notify;
}

void gt_obj_set_untouchability(gt_obj_st * obj, bool is_untouchability)
{
    GT_CHECK_BACK(obj);
    obj->untouchability = is_untouchability ? true : false;
}

bool gt_obj_is_untouchability(gt_obj_st * obj)
{
    return obj->untouchability;
}

void gt_obj_set_reduce(gt_obj_st * obj, uint8_t reduce)
{
    GT_CHECK_BACK(obj);
    obj->reduce = reduce;
}

uint8_t gt_obj_get_reduce(gt_obj_st * obj)
{
    GT_CHECK_BACK_VAL(obj, 0);
    return obj->reduce;
}

void gt_obj_set_radius(gt_obj_st * obj, gt_radius_t radius)
{
    GT_CHECK_BACK(obj);
    obj->radius = radius;
}

void gt_obj_set_radius_anim(gt_obj_st * obj, gt_radius_t radius)
{
    GT_CHECK_BACK(obj);
    gt_anim_st anim = {0};
    gt_anim_init(&anim);
    gt_anim_set_target(&anim, obj);
    gt_anim_set_path_type(&anim, GT_ANIM_PATH_TYPE_EASE_IN_OUT);
    gt_anim_set_time(&anim, GT_ANIM_PERIOD_TRANSITION_EFFECT_MS);
    if (radius != obj->radius) {
        gt_anim_set_value(&anim, obj->radius, radius);
        gt_anim_set_exec_cb(&anim, _radius_anim_exec_cb);
        gt_anim_start(&anim);
    }
}

gt_radius_t gt_obj_get_radius(gt_obj_st * obj)
{
    GT_CHECK_BACK_VAL(obj, 0);
    return obj->radius;
}

void gt_obj_set_row_layout(gt_obj_st * obj, bool is_row_layout)
{
    GT_CHECK_BACK(obj);
    if (false == is_row_layout) {
        /** default */
        obj->row_layout = false;
        obj->scroll_dir = GT_SCROLL_ALL;
        return;
    }
    obj->row_layout = true;
    obj->scroll_dir = GT_SCROLL_HORIZONTAL;

    if (obj->cnt_child) {
        gt_layout_row_grow(obj);
    }
}

void gt_obj_set_grow_invert(gt_obj_st * obj, bool is_grow_invert)
{
    GT_CHECK_BACK(obj);
    obj->grow_invert = is_grow_invert;

    if (obj->cnt_child) {
        gt_layout_row_grow(obj);
    }
}

/* end ------------------------------------------------------------------*/
