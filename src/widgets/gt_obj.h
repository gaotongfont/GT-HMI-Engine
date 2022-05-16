/**
 * @file gt_obj.h
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-05-11 15:08:01
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_OBJ_H_
#define _GT_OBJ_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../others/gt_color.h"
#include "../others/gt_types.h"
#include "gt_obj_class.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/


typedef struct gt_area_s {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
}gt_area_st;


typedef struct _gt_obj_s {
    struct _gt_obj_s * parent;
    struct _gt_obj_s ** child;
    uint16_t cnt_child;
    const gt_obj_class_st * class;
#if GT_USE_USER_DATA
    void * user_data;
#endif
    gt_area_st area;
    style_widget_t style;
}gt_obj_st;


/* private typedef  */
typedef struct _gt_style_s
{
    uint8_t selected;
}_gt_style_obj_st;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st *gt_obj_create(gt_obj_st *parent);
void gt_obj_del(gt_obj_st * obj);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_OBJ_H_
