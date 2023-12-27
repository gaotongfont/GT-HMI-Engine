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
#include "../hal/gt_hal_disp.h"
#include "./gt_disp.h"
#include "../others/gt_log.h"
#include "gt_mem.h"
#include "string.h"

/* private define -------------------------------------------------------*/
#define _set_style_prop_val(obj, prop, val)     (obj->style->prop = val)
// #define _set_area_prop_val(obj, prop, val)      (obj->area.prop = val)  // unused
#define _get_area_prop(obj, prop)               (obj->area.prop)

/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_obj_set_area(gt_obj_st * obj, gt_area_st area)
{
    gt_obj_area_change(obj, &area);
    _gt_disp_update_max_area(&area, _gt_obj_is_ignore_calc_max_area(obj));
}

void gt_obj_set_pos(gt_obj_st * obj, gt_size_t x, gt_size_t y)
{
    gt_area_st area = obj->area;
    area.x = x;
    area.y = y;
    gt_obj_pos_change(obj, &area);
    _gt_disp_update_max_area(&area, _gt_obj_is_ignore_calc_max_area(obj));
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_STYLE, NULL);
}

void gt_obj_move_to(gt_obj_st * obj, gt_size_t x, gt_size_t y)
{
    gt_area_st area = obj->area;
    gt_size_t dx = x - area.x;
    gt_size_t dy = y - area.y;
    area.x = x;
    area.y = y;
    gt_obj_move_child_by(obj, dx, dy);
    gt_obj_pos_change(obj, &area);
    _gt_disp_update_max_area(&area, _gt_obj_is_ignore_calc_max_area(obj));
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_STYLE, NULL);
}


void gt_obj_set_size(gt_obj_st * obj, uint16_t w, uint16_t h) {
    gt_area_st area = obj->area;
    area.w = w;
    area.h = h;
    gt_obj_size_change(obj, &area);
    _gt_disp_update_max_area(&area, _gt_obj_is_ignore_calc_max_area(obj));
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_STYLE, NULL);
}

void gt_obj_set_x(gt_obj_st * obj, gt_size_t x) {
    gt_area_st area = obj->area;
    area.x = x;
    gt_obj_pos_change(obj, &area);
    _gt_disp_update_max_area(&area, _gt_obj_is_ignore_calc_max_area(obj));
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_STYLE, NULL);
}

void gt_obj_set_y(gt_obj_st * obj, gt_size_t y) {
    gt_area_st area = obj->area;
    area.y = y;
    gt_obj_pos_change(obj, &area);
    _gt_disp_update_max_area(&area, _gt_obj_is_ignore_calc_max_area(obj));
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_STYLE, NULL);
}

void gt_obj_set_w(gt_obj_st * obj, uint16_t w) {
    gt_area_st area = obj->area;
    area.w = w;
    gt_obj_pos_change(obj, &area);
    _gt_disp_update_max_area(&area, _gt_obj_is_ignore_calc_max_area(obj));
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_STYLE, NULL);
}

void gt_obj_set_h(gt_obj_st * obj, uint16_t h) {
    gt_area_st area = obj->area;
    area.h = h;
    gt_obj_pos_change(obj, &area);
    _gt_disp_update_max_area(&area, _gt_obj_is_ignore_calc_max_area(obj));
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_STYLE, NULL);
}

void gt_obj_set_opa(gt_obj_st * obj, gt_opa_t opa) {
    obj->opa = opa;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

gt_size_t gt_obj_get_x(gt_obj_st * obj) {
    return _get_area_prop(obj, x);
}
gt_size_t gt_obj_get_y(gt_obj_st * obj) {
    return _get_area_prop(obj, y);
}
uint16_t gt_obj_get_w(gt_obj_st * obj) {
    return _get_area_prop(obj, w);
}
uint16_t gt_obj_get_h(gt_obj_st * obj) {
    return _get_area_prop(obj, h);
}

void gt_obj_set_visible(gt_obj_st * obj, gt_visible_et is_visible)
{
    obj->visible = is_visible;
    gt_disp_invalid_area(obj);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_obj_set_disabled(gt_obj_st * obj, gt_disabled_et is_disabled)
{
    obj->disabled = is_disabled ? 1 : 0;
    obj->opa = obj->disabled ? GT_OPA_60 : GT_OPA_COVER;
    gt_disp_invalid_area(obj);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

bool gt_obj_is_disabled(gt_obj_st * obj)
{
    return obj->disabled;
}

void gt_obj_set_focus(gt_obj_st * obj, bool is_focus)
{
    obj->focus = is_focus;
    gt_disp_invalid_area(obj);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

bool gt_obj_is_focus(gt_obj_st * obj)
{
    return obj->focus;
}

void gt_obj_set_focus_disabled(gt_obj_st * obj, gt_disabled_et is_disabled)
{
    obj->focus_dis = is_disabled ? 1 : 0;

    if(GT_DISABLED == obj->focus_dis){
        gt_disp_invalid_area(obj);
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
    }
}

bool gt_obj_is_focus_disabled(gt_obj_st * obj)
{
    return obj->focus_dis;
}

void gt_obj_size_change(gt_obj_st * obj, gt_area_st * area_new)
{
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

void gt_obj_area_change(gt_obj_st * obj, gt_area_st * area_new)
{
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

void gt_obj_move_child_by(gt_obj_st * obj, gt_size_t dx, gt_size_t dy)
{
    uint16_t i = 0, cnt = obj->cnt_child;

    if (false == gt_obj_get_overflow(obj)) {
        return;
    }

    for (i = 0; i < cnt; i++) {
        gt_obj_st * child = obj->child[i];
        if (false == gt_obj_get_overflow(child)) {
            continue;
        }
        child->area.x += dx;
        child->area.y += dy;

        gt_obj_move_child_by(child, dx, dy);
    }
}

void gt_obj_child_set_prop(gt_obj_st * obj, gt_obj_prop_type_em type, uint8_t val)
{
    uint16_t i, cnt = obj->cnt_child;
    for (i = 0; i < cnt; i++) {
        gt_obj_st * child = obj->child[i];
        switch (type) {
            case GT_OBJ_PROP_TYPE_VISIBLE: {
                gt_obj_set_visible(child, val);
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
                gt_obj_set_focus_disabled(child, val);
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
    obj->state = state;
}

gt_state_et gt_obj_get_state(gt_obj_st * obj)
{
    return obj->state;
}

void gt_obj_set_fixed(gt_obj_st * obj, bool is_fixed)
{
    obj->fixed = is_fixed;
}

bool gt_obj_get_fixed(gt_obj_st * obj)
{
    return obj->fixed;
}

void gt_obj_set_overflow(gt_obj_st * obj, bool is_overflow)
{
    obj->overflow = is_overflow;
}

bool gt_obj_get_overflow(gt_obj_st * obj)
{
    return obj->overflow;
}

void gt_obj_set_inside(gt_obj_st * obj, bool is_inside)
{
    obj->inside = is_inside;
}

bool gt_obj_get_inside(gt_obj_st * obj)
{
    return obj->inside;
}

void gt_obj_set_virtual(gt_obj_st * obj, bool is_virtual)
{
    obj->virtual = is_virtual;
}

bool gt_obj_get_virtual(gt_obj_st * obj)
{
    return obj->virtual;
}

void gt_obj_set_scroll_dir(gt_obj_st * obj, gt_scroll_dir_et dir)
{
    if (dir < GT_SCROLL_DISABLE || dir > GT_SCROLL_ALL) {
        obj->scroll_dir = GT_SCROLL_ALL;
        return;
    }

    obj->scroll_dir = dir;
}

gt_scroll_dir_et gt_obj_get_scroll_dir(gt_obj_st * obj)
{
    return obj->scroll_dir;
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
    return obj->scroll_snap_x;
}

gt_scroll_snap_em gt_obj_get_scroll_snap_y(gt_obj_st * obj)
{
    return obj->scroll_snap_y;
}

bool _gt_obj_is_ignore_calc_max_area(gt_obj_st * obj)
{
    if (NULL == obj) { return true; }
    if (obj->inside) { return true; }
    if (obj->overflow) { return true; }
    if (obj->virtual) { return true; }

    return false;
}

/* end ------------------------------------------------------------------*/
