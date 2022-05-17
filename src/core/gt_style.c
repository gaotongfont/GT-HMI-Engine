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
void gt_obj_set_style_area(gt_obj_st * obj, gt_area_st * area){
    gt_memcpy(&obj->area, area, sizeof(gt_area_st));
}

void gt_obj_set_style_pos(gt_obj_st * obj, uint16_t x, uint16_t y){
    gt_obj_set_style_x(obj, x);
    gt_obj_set_style_y(obj, y);
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



/* end ------------------------------------------------------------------*/
