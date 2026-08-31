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
#include "../others/gt_area.h"

/* private define -------------------------------------------------------*/
#define _get_area_prop(obj, prop)               (obj->area.prop)


/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static GT_ATTRIBUTE_RAM_TEXT inline bool _is_viewport_fixed_screen_child(const gt_obj_st * obj) {
    return obj && obj->viewport_fixed && obj->parent && gt_obj_is_type(obj->parent, GT_TYPE_SCREEN);
}
/* area_act belongs only to the active screen.  Keep inactive screens in their
 * own content coordinate space until they become active. */
static GT_ATTRIBUTE_RAM_TEXT inline bool _is_active_viewport_fixed_screen_child(const gt_obj_st * obj) {
    return _is_viewport_fixed_screen_child(obj) && obj->parent == gt_disp_get_scr();
}
static GT_ATTRIBUTE_RAM_TEXT inline gt_size_t _viewport_offset_x(void) {
    gt_area_st * area = gt_disp_get_area_act();
    return area ? area->x : 0;
}
static GT_ATTRIBUTE_RAM_TEXT inline gt_size_t _viewport_offset_y(void) {
    gt_area_st * area = gt_disp_get_area_act();
    return area ? area->y : 0;
}

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
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return false;
    }
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
    gt_size_t y = tar->area.y;
    if (_is_active_viewport_fixed_screen_child(tar)) {
        y -= _viewport_offset_y();
    }
    gt_obj_set_pos(tar, val, y);
}

static GT_ATTRIBUTE_RAM_TEXT void _y_anim_exec_cb(void * obj, int32_t val) {
    gt_obj_st * tar = (gt_obj_st * )obj;
    gt_size_t x = tar->area.x;
    if (_is_active_viewport_fixed_screen_child(tar)) {
        x -= _viewport_offset_x();
    }
    gt_obj_set_pos(tar, x, val);
}

static GT_ATTRIBUTE_RAM_TEXT void _radius_anim_exec_cb(void * obj, int32_t val) {
    gt_obj_st * tar = (gt_obj_st * )obj;
    tar->radius = val;
    gt_event_send(tar, GT_EVENT_TYPE_DRAW_START, NULL);
}

static void GT_ATTRIBUTE_RAM_TEXT
_common_pos_relative_handle(gt_obj_st * obj, gt_area_st * area_p) {
    if (_is_active_viewport_fixed_screen_child(obj)) {
        area_p->x += _viewport_offset_x();
        area_p->y += _viewport_offset_y();
    }
#if GT_USE_LAYER_TOP
    if (_update_layer_top_widget_area(obj, *area_p, false)) {
        return ;
    }
#endif
    /** normal screen */
    _gt_obj_move_child_by(obj, area_p->x - obj->area.x, area_p->y - obj->area.y);
    if (gt_obj_get_virtual(obj)) {
        return ;
    }
    gt_obj_set_area(obj, *area_p);
#if GT_USE_WIDGET_LAYOUT
    if (obj->parent && !_is_viewport_fixed_screen_child(obj)) { gt_layout_mark_dirty(obj->parent); }
    if (_gt_disp_refr_is_suspended()) { return; }
#endif
}

/* global functions / API interface -------------------------------------*/
gt_res_t gt_obj_set_area(gt_obj_st * obj, gt_area_st area)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return GT_RES_FAIL;
    }
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
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    gt_area_st area = obj->area;
    area.x = x;
    area.y = y;
    /* Floating screen children use viewport coordinates in the public API. */
    if (_is_active_viewport_fixed_screen_child(obj)) {
        area.x += _viewport_offset_x();
        area.y += _viewport_offset_y();
    }
#if GT_USE_LAYER_TOP
    if (_update_layer_top_widget_area(obj, area, false)) {
        return ;
    }
#endif
    /** normal screen; floating children already carry the viewport offset. */
    area.w = obj->area.w;
    area.h = obj->area.h;
    _gt_obj_move_child_by(obj, area.x - obj->area.x, area.y - obj->area.y);
    if (gt_obj_get_virtual(obj)) {
        return ;
    }
    gt_obj_set_area(obj, area);
#if GT_USE_WIDGET_LAYOUT
    if (obj->parent && !_is_viewport_fixed_screen_child(obj)) { gt_layout_mark_dirty(obj->parent); }
    if (_gt_disp_refr_is_suspended()) { return; }
#endif
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_STYLE, NULL);
}

void gt_obj_set_pos_anim(gt_obj_st * obj, gt_size_t x, gt_size_t y)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    gt_anim_st anim = {0};
    gt_anim_init(&anim);
    gt_anim_set_target(&anim, obj);
    gt_anim_set_path_type(&anim, GT_ANIM_PATH_TYPE_EASE_IN_OUT);
    gt_anim_set_time(&anim, GT_ANIM_PERIOD_TRANSITION_EFFECT_MS);
    gt_size_t cur_x = obj->area.x;
    gt_size_t cur_y = obj->area.y;
    if (_is_active_viewport_fixed_screen_child(obj)) {
        cur_x -= _viewport_offset_x();
        cur_y -= _viewport_offset_y();
    }
    if (cur_x != x) {
        gt_anim_set_value(&anim, cur_x, x);
        gt_anim_set_exec_cb(&anim, _x_anim_exec_cb);
        gt_anim_start(&anim);
    }
    if (cur_y != y) {
        gt_anim_set_value(&anim, cur_y, y);
        gt_anim_set_exec_cb(&anim, _y_anim_exec_cb);
        gt_anim_start(&anim);
    }
}

void gt_obj_set_pos_step(gt_obj_st * obj, gt_size_t step_x, gt_size_t step_y)
{
    gt_size_t x = obj->area.x;
    gt_size_t y = obj->area.y;
    if (_is_active_viewport_fixed_screen_child(obj)) {
        x -= _viewport_offset_x();
        y -= _viewport_offset_y();
    }
    gt_obj_set_pos(obj, x + step_x, y + step_y);
}

#if GT_USE_CUSTOM_TOUCH_EXPAND_SIZE
void gt_obj_set_touch_expand_area(gt_obj_st * obj, gt_size_t hor, gt_size_t ver)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    obj->touch_expand_size.x = hor < 0 ? 0 : hor;
    obj->touch_expand_size.y = ver < 0 ? 0 : ver;
}
#endif  /** GT_USE_CUSTOM_TOUCH_EXPAND_SIZE */

void gt_obj_set_pos_relative(gt_obj_st * obj, gt_obj_st * target, gt_size_t diff_x, gt_size_t diff_y)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    if (NULL == target) {
        target = obj->parent;
        type = gt_obj_class_get_type(target);
        if (GT_TYPE_UNKNOWN == type) {
            return;
        }
    }
    if (_is_active_viewport_fixed_screen_child(obj)) {
        gt_obj_set_pos(obj, target->area.x - _viewport_offset_x() + diff_x,
                       target->area.y - _viewport_offset_y() + diff_y);
        return;
    }
    gt_area_st area = {
        .x = target->area.x + diff_x,
        .y = target->area.y + diff_y,
        .w = obj->area.w,
        .h = obj->area.h
    };
    _common_pos_relative_handle(obj, &area);
}

void gt_obj_set_pos_relative_x(gt_obj_st * obj, gt_obj_st * target, gt_size_t diff_x)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    if (NULL == target) {
        target = obj->parent;
        type = gt_obj_class_get_type(target);
        if (GT_TYPE_UNKNOWN == type) {
            return;
        }
    }
    if (_is_active_viewport_fixed_screen_child(obj)) {
        gt_obj_set_pos(obj, target->area.x - _viewport_offset_x() + diff_x,
                       obj->area.y - _viewport_offset_y());
        return;
    }
    gt_area_st area = {
        .x = target->area.x + diff_x,
        .y = obj->area.y,
        .w = obj->area.w,
        .h = obj->area.h
    };
    _common_pos_relative_handle(obj, &area);
}

void gt_obj_set_pos_relative_y(gt_obj_st * obj, gt_obj_st * target, gt_size_t diff_y)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    if (NULL == target) {
        target = obj->parent;
        type = gt_obj_class_get_type(target);
        if (GT_TYPE_UNKNOWN == type) {
            return;
        }
    }
    if (_is_active_viewport_fixed_screen_child(obj)) {
        gt_obj_set_pos(obj, obj->area.x - _viewport_offset_x(),
                       target->area.y - _viewport_offset_y() + diff_y);
        return;
    }
    gt_area_st area = {
        .x = obj->area.x,
        .y = target->area.y + diff_y,
        .w = obj->area.w,
        .h = obj->area.h
    };
    _common_pos_relative_handle(obj, &area);
}

void gt_obj_set_pos_always_full_display(gt_obj_st * obj, gt_size_t x, gt_size_t y)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
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
    if (_is_active_viewport_fixed_screen_child(obj)) {
        area.x += _viewport_offset_x();
        area.y += _viewport_offset_y();
    }
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
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
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
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    if (gt_obj_get_virtual(obj)) {
        return ;
    }
    gt_area_st area = obj->area;
    area.w = w;
    area.h = h;
    gt_obj_size_change(obj, &area);
    _gt_disp_update_max_area(&area, _gt_obj_is_ignore_calc_max_area(obj));
#if GT_USE_WIDGET_LAYOUT
    if (obj->parent && !_is_viewport_fixed_screen_child(obj)) { gt_layout_mark_dirty(obj->parent); }
    if (_gt_disp_refr_is_suspended()) { return; }
#endif
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_STYLE, NULL);
}

void gt_obj_set_size_anim(gt_obj_st * obj, uint16_t w, uint16_t h)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
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

void gt_obj_set_size_step(gt_obj_st * obj, gt_size_t step_w, gt_size_t step_h)
{
    if (obj->area.w + step_w < 0) { step_w = -obj->area.w; }
    if (obj->area.h + step_h < 0) { step_h = -obj->area.h; }
    gt_obj_set_size(obj, obj->area.w + step_w, obj->area.h + step_h);
}

void gt_obj_set_x(gt_obj_st * obj, gt_size_t x)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    gt_size_t y = obj->area.y;
    if (_is_active_viewport_fixed_screen_child(obj)) { y -= _viewport_offset_y(); }
    gt_obj_set_pos(obj, x, y);
}

void gt_obj_set_x_anim(gt_obj_st * obj, gt_size_t x)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    gt_size_t y = obj->area.y;
    if (_is_active_viewport_fixed_screen_child(obj)) { y -= _viewport_offset_y(); }
    gt_obj_set_pos_anim(obj, x, y);
}

void gt_obj_set_x_step(gt_obj_st * obj, gt_size_t step_x)
{
    gt_obj_set_pos_step(obj, step_x, 0);
}

void gt_obj_set_y(gt_obj_st * obj, gt_size_t y)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    gt_size_t x = obj->area.x;
    if (_is_active_viewport_fixed_screen_child(obj)) { x -= _viewport_offset_x(); }
    gt_obj_set_pos(obj, x, y);
}

void gt_obj_set_y_anim(gt_obj_st * obj, gt_size_t y)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    gt_size_t x = obj->area.x;
    if (_is_active_viewport_fixed_screen_child(obj)) { x -= _viewport_offset_x(); }
    gt_obj_set_pos_anim(obj, x, y);
}

void gt_obj_set_y_step(gt_obj_st * obj, gt_size_t step_y)
{
    gt_obj_set_pos_step(obj, 0, step_y);
}

void gt_obj_set_w(gt_obj_st * obj, uint16_t w)
{
    gt_obj_set_size(obj, w, obj->area.h);
}

void gt_obj_set_w_anim(gt_obj_st * obj, uint16_t w)
{
    gt_obj_set_size_anim(obj, w, obj->area.h);
}

void gt_obj_set_w_step(gt_obj_st * obj, gt_size_t step_w)
{
    gt_obj_set_size(obj, obj->area.w + step_w, obj->area.h);
}

void gt_obj_set_h(gt_obj_st * obj, uint16_t h)
{
    gt_obj_set_size(obj, obj->area.w, h);
}

void gt_obj_set_h_anim(gt_obj_st * obj, uint16_t h)
{
    gt_obj_set_size_anim(obj, obj->area.w, h);
}

void gt_obj_set_h_step(gt_obj_st * obj, gt_size_t step_h)
{
    gt_obj_set_size(obj, obj->area.w, obj->area.h + step_h);
}

void gt_obj_set_opa(gt_obj_st * obj, gt_opa_t opa)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    obj->opa = opa;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

gt_anim_st * gt_obj_set_opa_anim(gt_obj_st * obj, gt_opa_t opa)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return NULL;
    }
    if (obj->opa == opa) {
        return NULL;
    }
    gt_anim_st anim = {0};
    gt_anim_init(&anim);
    gt_anim_set_target(&anim, obj);
    gt_anim_set_path_type(&anim, GT_ANIM_PATH_TYPE_EASE_IN_OUT);
    gt_anim_set_time(&anim, GT_ANIM_PERIOD_TRANSITION_EFFECT_MS);
    gt_anim_set_value(&anim, obj->opa, opa);
    gt_anim_set_exec_cb(&anim, _opa_anim_exec_cb);
    return gt_anim_start(&anim);
}

void gt_obj_set_opa_step(gt_obj_st * obj, gt_size_t step_opa)
{
    if (obj->opa + step_opa < 0) {
        step_opa = -(gt_size_t)obj->opa;
    } else if (obj->opa + step_opa > GT_OPA_COVER) {
        step_opa = (gt_size_t)GT_OPA_COVER - obj->opa;
    }
    gt_obj_set_opa(obj, (gt_opa_t)(obj->opa + step_opa));
}

gt_size_t gt_obj_get_x(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return 0;
    }
    gt_size_t x = _get_area_prop(obj, x);
    if (_is_active_viewport_fixed_screen_child(obj)) {
        x -= _viewport_offset_x();
    }
    return x;
}

gt_size_t gt_obj_get_y(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return 0;
    }
    gt_size_t y = _get_area_prop(obj, y);
    if (_is_active_viewport_fixed_screen_child(obj)) {
        y -= _viewport_offset_y();
    }
    return y;
}
uint16_t gt_obj_get_w(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return 0;
    }
    return _get_area_prop(obj, w);
}
uint16_t gt_obj_get_h(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return 0;
    }
    return _get_area_prop(obj, h);
}

void gt_obj_set_visible(gt_obj_st * obj, gt_visible_et is_visible)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    if (is_visible == obj->visible) {
        return;
    }
    obj->visible = is_visible;
    gt_disp_invalid_area(obj);
    if (obj->visible) {
        gt_obj_st * scr = gt_obj_within_which_scr(obj);
        /* area_max is global display state; an inactive screen must not
         * replace the active screen's scroll limits while being prepared. */
        if (scr == gt_disp_get_scr()) {
            _gt_disp_reload_max_area(scr);
        }
    }
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

gt_visible_et gt_obj_get_visible(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return GT_INVISIBLE;
    }
    return (gt_visible_et)obj->visible;
}

void gt_obj_set_disabled(gt_obj_st * obj, gt_disabled_et is_disabled)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    obj->disabled = is_disabled ? 1 : 0;
    obj->opa = obj->disabled ? GT_OPA_60 : GT_OPA_COVER;
    gt_disp_invalid_area(obj);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

bool gt_obj_is_disabled(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return false;
    }
    return obj->disabled;
}

void gt_obj_set_focus(gt_obj_st * obj, bool is_focus)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    obj->focus = is_focus;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

bool gt_obj_is_focus(gt_obj_st * obj)
{
    return obj ? obj->focus : false;
}

void gt_obj_set_focus_disabled(gt_obj_st * obj, gt_disabled_et is_disabled)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    obj->focus_dis = is_disabled ? 1 : 0;

    if (GT_DISABLED == obj->focus_dis) {
        gt_disp_invalid_area(obj);
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
    }
}

bool gt_obj_is_focus_disabled(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return false;
    }
    return obj->focus_dis;
}

void gt_obj_set_focus_skip(gt_obj_st * obj, bool is_skip)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    obj->focus_skip = is_skip;
}

bool gt_obj_is_focus_skip(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return false;
    }
    return obj->focus_skip;
}

void gt_obj_size_change(gt_obj_st * obj, gt_area_st * area_new)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    gt_area_st area;
    gt_area_join(&area, &obj->area, area_new);
    obj->area = *area_new;
    _gt_disp_refr_append_area(&area);
}

void gt_obj_pos_change(gt_obj_st * obj, gt_area_st * area_new)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    GT_CHECK_BACK(area_new);
    gt_area_st area;
    gt_area_join(&area, &obj->area, area_new);
    obj->area = *area_new;
    _gt_disp_refr_append_area(&area);
}

void gt_obj_child_set_prop(gt_obj_st * obj, gt_obj_prop_type_em type, uint8_t val)
{
    gt_obj_type_et t = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == t) {
        return;
    }
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
            case GT_OBJ_PROP_TYPE_BUBBLE_NOTIFY: {
                gt_obj_set_bubble_notify(child, val);
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
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return GT_STATE_NONE;
    }
    return (gt_state_et)obj->state;
}

void gt_obj_set_fixed(gt_obj_st * obj, bool is_fixed)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    obj->fixed = is_fixed;
}

bool gt_obj_get_fixed(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return false;
    }
    return obj->fixed;
}

void gt_obj_set_viewport_fixed(gt_obj_st * obj, bool enable)
{
    if (GT_TYPE_UNKNOWN == gt_obj_class_get_type(obj)) { return; }
    if (enable && (!obj->parent || !gt_obj_is_type(obj->parent, GT_TYPE_SCREEN))) {
        GT_LOGW(GT_LOG_TAG_UI, "viewport_fixed requires a direct screen child: %p(%d)",
                obj, gt_obj_class_get_type(obj));
        return;
    }
    if (obj->viewport_fixed == (enable ? 1 : 0)) { return; }
    obj->viewport_fixed = enable ? 1 : 0;
    if (obj->parent && gt_obj_is_type(obj->parent, GT_TYPE_SCREEN)) {
        gt_disp_invalid_area(obj);
        if (obj->parent == gt_disp_get_scr()) {
            _gt_disp_reload_max_area(obj->parent);
        }
        if (obj->parent->row_layout) {
            gt_layout_row_grow(obj->parent);
        }
#if GT_USE_WIDGET_LAYOUT
        gt_layout_mark_dirty(obj->parent);
#endif
    }
}

bool gt_obj_get_viewport_fixed(gt_obj_st * obj)
{
    if (GT_TYPE_UNKNOWN == gt_obj_class_get_type(obj)) { return false; }
    return obj->viewport_fixed != 0;
}

void gt_obj_set_overflow(gt_obj_st * obj, bool is_overflow)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    obj->overflow = is_overflow;
}

bool gt_obj_get_overflow(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return false;
    }
    return obj->overflow;
}

void gt_obj_set_inside(gt_obj_st * obj, bool is_inside)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    obj->inside = is_inside;
}

bool gt_obj_get_inside(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return false;
    }
    return obj->inside;
}

void gt_obj_set_mask_effect(gt_obj_st * obj, bool is_keep_alive)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    obj->mask_effect = is_keep_alive;
}

bool gt_obj_get_mask_effect(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return false;
    }
    return obj->mask_effect;
}

void gt_obj_set_touch_parent(gt_obj_st * obj, bool is_touch_parent)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    obj->touch_parent = is_touch_parent;
}

bool gt_obj_get_touch_parent(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return false;
    }
    return obj->touch_parent;
}

void gt_obj_set_trigger_mode(gt_obj_st * obj, gt_obj_trigger_mode_et is_trigger_mode)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
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
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    obj->virtuality = is_virtual;
}

bool gt_obj_get_virtual(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return false;
    }
    return obj->virtuality;
}

void gt_obj_set_scroll_dir(gt_obj_st * obj, gt_scroll_dir_et dir)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    if (dir < GT_SCROLL_DISABLE || dir > GT_SCROLL_DISABLE_BUT_NOTIFY) {
        obj->scroll_dir = GT_SCROLL_ALL;
        return;
    }

    obj->scroll_dir = dir;
    if (GT_SCROLL_DISABLE != dir || !gt_obj_is_type(obj, GT_TYPE_SCREEN) || obj != gt_disp_get_scr()) {
        return;
    }

    gt_area_st * act = gt_disp_get_area_act();
    if (!act) {
        obj->process_attr.scroll.x = 0;
        obj->process_attr.scroll.y = 0;
        gt_obj_set_scroll_prev(obj, 0, 0);
        return;
    }
    gt_size_t dx = -act->x;
    gt_size_t dy = -act->y;
    if (dx || dy) {
        gt_disp_scroll_area_act(0, 0);
        for (uint16_t i = 0; i < obj->cnt_child; ++i) {
            gt_obj_st * child = obj->child[i];
            if (!child->viewport_fixed) { continue; }
            gt_disp_invalid_area(child);
            child->area.x += dx;
            child->area.y += dy;
            _gt_obj_move_child_by(child, dx, dy);
            gt_disp_invalid_area(child);
        }
    }
    obj->process_attr.scroll.x = 0;
    obj->process_attr.scroll.y = 0;
    gt_obj_set_scroll_prev(obj, 0, 0);
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
    gt_obj_st * ancestor = obj;
    if (NULL == obj) { return true; }

    while (ancestor && ancestor->parent) {
        if (ancestor->viewport_fixed && gt_obj_is_type(ancestor->parent, GT_TYPE_SCREEN)) {
            return true;
        }
        ancestor = ancestor->parent;
    }
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
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    obj->bubble_notify = is_bubble_notify ? true : false;
}

bool gt_obj_is_bubble_notify(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return false;
    }
    return obj->bubble_notify;
}

void gt_obj_set_untouchability(gt_obj_st * obj, bool is_untouchability)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    obj->untouchability = is_untouchability ? true : false;
}

bool gt_obj_is_untouchability(gt_obj_st * obj)
{
    return obj->untouchability;
}

void gt_obj_set_focus_w(gt_obj_st * obj, uint8_t focus_w)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    obj->focus_w = focus_w;
}

uint8_t gt_obj_get_focus_w(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return 0;
    }
    return obj->focus_w;
}

void gt_obj_set_focus_gap(gt_obj_st * obj, uint8_t focus_gap)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    obj->focus_gap = focus_gap;
}

uint8_t gt_obj_get_focus_gap(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return 0;
    }
    return obj->focus_gap;
}

uint16_t gt_obj_get_focus_offset_value(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return 0;
    }
    return obj->focus_w + obj->focus_gap;
}

void gt_obj_set_radius(gt_obj_st * obj, gt_radius_t radius)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    obj->radius = radius;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_obj_set_radius_anim(gt_obj_st * obj, gt_radius_t radius)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
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

void  gt_obj_set_radius_step(gt_obj_st * obj, gt_radius_t step_radius)
{
    gt_obj_set_radius(obj, obj->radius + step_radius);
}

gt_radius_t gt_obj_get_radius(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return 0;
    }
    return obj->radius;
}

void gt_obj_set_row_layout(gt_obj_st * obj, bool is_row_layout)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
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
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    obj->grow_invert = is_grow_invert;

    if (obj->cnt_child) {
        gt_layout_row_grow(obj);
    }
}

void gt_obj_set_swipe_dir(gt_obj_st * obj, bool enabled)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    obj->swipe_dir = enabled ? true : false;
}

bool gt_obj_get_swipe_dir_enabled(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return false;
    }
    return obj->swipe_dir;
}

void gt_obj_set_ignore_throw(gt_obj_st * obj, bool is_ignore_throw)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    obj->ignore_throw = is_ignore_throw;
}

void gt_obj_set_rebound(gt_obj_st * obj, bool enabled)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    obj->rebound = enabled ? true : false;
}

bool gt_obj_is_rebound(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return false;
    }
    return obj->rebound;
}

void gt_obj_set_scroll_throw_limit(gt_obj_st * obj, uint8_t level)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    if (level > 15) {
        level = 15;
    }
    obj->scroll_throw_limit = level;
}

uint8_t gt_obj_get_scroll_throw_limit(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return 0;
    }
    return (uint8_t)obj->scroll_throw_limit;
}

void gt_obj_set_scroll_tip(gt_obj_st * obj, gt_scroll_tip_et mode)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    obj->scroll_tip = mode;
}

gt_scroll_tip_et gt_obj_get_scroll_tip(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return GT_SCROLL_TIP_NONE;
    }
    return obj->scroll_tip;
}

void gt_obj_set_scroll_tip_always(gt_obj_st * obj, bool always)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    obj->scroll_tip_always = always;
}

bool gt_obj_get_scroll_tip_always(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return false;
    }
    return obj->scroll_tip_always;
}

/* end ------------------------------------------------------------------*/
