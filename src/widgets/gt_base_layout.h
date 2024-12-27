/**
 * @file gt_base_layout.h
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-12-23 16:02:31
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */
#ifndef _GT_BASE_LAYOUT_H_
#define _GT_BASE_LAYOUT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"
#if GT_CFG_ENABLE_BASE_LAYOUT
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "stdarg.h"
#include "stdlib.h"
#include "../others/gt_color.h"
#include "../core/gt_style.h"
#include "../font/gt_font.h"


/* define ---------------------------------------------------------------*/
// enable cross layout
#ifndef GT_USE_BASE_LAYOUT_CROSS
    #define GT_USE_BASE_LAYOUT_CROSS  1
#endif


/* typedef --------------------------------------------------------------*/
typedef enum {
    GT_BASE_LAYOUT_TYPE_NONE = 0,
    GT_BASE_LAYOUT_TYPE_CROSS,

}gt_base_layout_type_et;

#if GT_USE_BASE_LAYOUT_CROSS
typedef enum {
    GT_CROSS_ROLL_DEFAULT = 0,
    GT_CROSS_ROLL_LOOP,
}gt_cross_roll_mode_et;
#endif

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_base_layout_create(gt_obj_st * parent, gt_base_layout_type_et layout_type);
void gt_base_layout_set_hor_ver_numb(gt_obj_st* layout, uint8_t hor_numb, uint8_t ver_numb);
void gt_base_layout_set_gap(gt_obj_st* layout, uint8_t hor_gap, uint8_t ver_gap);



#if GT_USE_BASE_LAYOUT_CROSS
gt_obj_st* gt_base_layout_cross_get_hor_obj(gt_obj_st* layout);
gt_obj_st* gt_base_layout_cross_get_ver_obj(gt_obj_st* layout);
void gt_base_layout_cross_set_roll_mode(gt_obj_st* layout, gt_cross_roll_mode_et roll_mode);
void gt_base_layout_cross_set_hor_obj_area(gt_obj_st* layout, gt_size_t x, gt_size_t y, uint16_t w, uint16_t h);
void gt_base_layout_cross_set_ver_obj_area(gt_obj_st* layout, gt_size_t x, gt_size_t y, uint16_t w, uint16_t h);
void gt_base_layout_cross_hor_add_itme(gt_obj_st* layout, gt_obj_st* obj, uint8_t ver_index);
void gt_base_layout_cross_ver_add_item(gt_obj_st* layout, gt_obj_st* obj);
void gt_base_layout_cross_hor_clear_item(gt_obj_st* layout, uint8_t ver_index);
//
void gt_base_layout_cross_set_hor_index(gt_obj_st* layout, uint8_t hor_index);
void gt_base_layout_cross_set_ver_index(gt_obj_st* layout, uint8_t ver_index);
gt_size_t gt_base_layout_cross_get_hor_count(gt_obj_st* layout);

#endif

#endif //!GT_CFG_ENABLE_BASE_LAYOUT
//
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_BASE_LAYOUT_H_

/* end of file ----------------------------------------------------------*/


