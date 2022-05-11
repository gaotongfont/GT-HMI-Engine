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
#include "../gt_conf.h"



/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/


typedef struct gt_area_s {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
}gt_area_st;

typedef struct gt_style_s {
    gt_color_t bcolor;
    gt_color_t fcolor;

    uint16_t radius;

    uint16_t pad_top;
    uint16_t pad_bottom;
    uint16_t pad_left;
    uint16_t pad_right;

    uint16_t margin_top;
    uint16_t margin_bottom;
    uint16_t margin_left;
    uint16_t margin_right;

    uint16_t border;
    gt_color_t border_color;
}gt_style_st;

typedef struct gt_obj_s {
    struct gt_obj_s *parent;
    void * class;
    void * user_data;
    gt_area_st area;
    gt_style_st style;
}gt_obj_st;




/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st *gt_obj_create(gt_obj_st *parent);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_OBJ_H_
