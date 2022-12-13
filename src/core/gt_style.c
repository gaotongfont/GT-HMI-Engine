/**
 * @file gt_style.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-12 18:37:55
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

 /* include --------------------------------------------------------------*/
#include "gt_style.h"
#include "../hal/gt_hal_disp.h"
#include "gt_mem.h"
#include "string.h"

/* private define -------------------------------------------------------*/
#define _set_style_prop_val(obj, prop, val)     (obj->style->prop = val)
#define _set_area_prop_val(obj, prop, val)      (obj->area.prop = val)
#define _get_area_prop(obj, prop)               (obj->area.prop)

/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_obj_style_set_area(gt_obj_st * obj, gt_area_st area) {
    gt_obj_area_change(obj, &area);
}

void gt_obj_style_set_pos(gt_obj_st * obj, gt_size_t x, gt_size_t y) {
    gt_area_st area = obj->area;
    area.x = x;
    area.y = y;
    gt_obj_pos_change(obj, &area);
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_STYLE, NULL);
}

void gt_obj_style_set_size(gt_obj_st * obj, uint16_t w, uint16_t h) {
    gt_area_st area = obj->area;
    area.w = w;
    area.h = h;
    gt_obj_size_change(obj, &area);
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_STYLE, NULL);
}

void gt_obj_style_set_x(gt_obj_st * obj, uint16_t x) {
    _set_area_prop_val(obj, x, x);
}

void gt_obj_style_set_y(gt_obj_st * obj, uint16_t y) {
    // _set_area_prop_val(obj, y, y);
    gt_area_st area = obj->area;
    area.y = y;
    gt_obj_pos_change(obj, &area);
}

void gt_obj_style_set_w(gt_obj_st * obj, uint16_t w) {
    _set_area_prop_val(obj, w, w);
}

void gt_obj_style_set_h(gt_obj_st * obj, uint16_t h) {
    _set_area_prop_val(obj, h, h);
}

void gt_obj_style_set_opa(gt_obj_st * obj, gt_opa_t opa) {
    obj->opa = opa;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

uint16_t gt_obj_style_get_x(gt_obj_st * obj) {
    return _get_area_prop(obj, x);
}
uint16_t gt_obj_style_get_y(gt_obj_st * obj) {
    return _get_area_prop(obj, y);
}
uint16_t gt_obj_style_get_w(gt_obj_st * obj) {
    return _get_area_prop(obj, w);
}
uint16_t gt_obj_style_get_h(gt_obj_st * obj) {
    return _get_area_prop(obj, h);
}

void gt_obj_set_visible(gt_obj_st * obj, gt_visible_et is_visible)
{
    obj->visible = is_visible;
    _gt_disp_refr_append_area(&obj->area);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
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

void gt_obj_style_set_state(gt_obj_st * obj, gt_state_et state)
{
    obj->state = state;
}
gt_state_et gt_obj_style_get_state(gt_obj_st * obj)
{
    return obj->state;
}
/* end ------------------------------------------------------------------*/
