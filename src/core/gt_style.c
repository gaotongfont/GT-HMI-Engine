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
void gt_obj_set_style_area(gt_obj_st * obj, gt_area_st area){
    gt_obj_set_style_x(obj, area.x);
    gt_obj_set_style_y(obj, area.y);
    gt_obj_set_style_w(obj, area.w);
    gt_obj_set_style_h(obj, area.h);
}

void gt_obj_set_style_pos(gt_obj_st * obj, gt_size_t x, gt_size_t y){
    gt_obj_set_style_x(obj, x);
    gt_obj_set_style_y(obj, y);
}

void gt_obj_set_style_size(gt_obj_st * obj, uint16_t w, uint16_t h){
    gt_area_st area = obj->area;
    area.w = w;
    area.h = h;
    gt_obj_size_change(obj, &area);
    gt_event_send(obj, GT_EVENT_TYPE_UPDATE_STYLE, NULL);
}

void gt_obj_set_style_x(gt_obj_st * obj, uint16_t x){
    _set_area_prop_val(obj, x, x);
}

void gt_obj_set_style_y(gt_obj_st * obj, uint16_t y){
    _set_area_prop_val(obj, y, y);
}

void gt_obj_set_style_w(gt_obj_st * obj, uint16_t w){
    _set_area_prop_val(obj, w, w);
}

void gt_obj_set_style_h(gt_obj_st * obj, uint16_t h){
    _set_area_prop_val(obj, h, h);
}

void gt_obj_set_style_opa(gt_obj_st * obj, gt_opa_t opa){
    obj->opa = opa;
}

uint16_t gt_obj_get_style_x(gt_obj_st * obj){
    return _get_area_prop(obj, x);
}
uint16_t gt_obj_get_style_y(gt_obj_st * obj){
    return _get_area_prop(obj, y);
}
uint16_t gt_obj_get_style_w(gt_obj_st * obj){
    return _get_area_prop(obj, w);
}
uint16_t gt_obj_get_style_h(gt_obj_st * obj){
    return _get_area_prop(obj, h);
}

void gt_obj_set_visible(gt_obj_st * obj, gt_visible_e is_visible)
{
    obj->visible = is_visible;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}


void gt_obj_size_change(gt_obj_st * obj, gt_area_st * area_new)
{
    extern void gt_disp_ref_area(gt_area_st * area);

    gt_area_st area_old = obj->area;
    gt_area_st area;
    if( area_new->x <= area_old.x ){
        area.x = area_new->x;
    }else{
        area.x = area_old.x;
    }

    if( area_new->y <= area_old.y ){
        area.y = area_new->y;
    }else{
        area.y = area_old.y;
    }

    /* need refactor */
    if( area_new->w <= area_old.w ){
        area.w = area_old.w;
    }else{
        area.w = area_new->w;
    }

    if( area_new->h <= area_old.h ){
        area.h = area_old.h;
    }else{
        area.h = area_new->h;
    }
    obj->area = *area_new;
    gt_disp_ref_area(&area);
}
/* end ------------------------------------------------------------------*/
