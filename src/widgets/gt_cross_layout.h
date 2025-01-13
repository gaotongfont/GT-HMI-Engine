/**
 * @file gt_cross_layout.h
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
#if GT_CFG_ENABLE_CROSS_LAYOUT
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "stdarg.h"
#include "stdlib.h"
#include "../others/gt_color.h"
#include "../core/gt_style.h"
#include "../font/gt_font.h"


/* define ---------------------------------------------------------------*/


/* typedef --------------------------------------------------------------*/
typedef enum {
    GT_CROSS_ROLL_DEFAULT = 0,
    GT_CROSS_ROLL_LOOP,
}gt_cross_roll_mode_et;

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_cross_layout_create(gt_obj_st * parent);
void gt_cross_layout_set_gap(gt_obj_st* layout, uint8_t hor_gap, uint8_t ver_gap);
void gt_cross_layout_set_roll_mode(gt_obj_st* layout, gt_cross_roll_mode_et roll_mode);

gt_obj_st* gt_cross_layout_get_hor_obj(gt_obj_st* layout);
gt_obj_st* gt_cross_layout_get_ver_obj(gt_obj_st* layout);
void gt_cross_layout_set_hor_obj_area(gt_obj_st* layout, gt_size_t x, gt_size_t y, uint16_t w, uint16_t h);
void gt_cross_layout_set_ver_obj_area(gt_obj_st* layout, gt_size_t x, gt_size_t y, uint16_t w, uint16_t h);

void gt_cross_layout_set_hor_offset(gt_obj_st* layout, gt_size_t offset);
void gt_cross_layout_set_ver_offset(gt_obj_st* layout, gt_size_t offset);

void gt_cross_layout_hor_add_itme(gt_obj_st* layout, gt_obj_st* obj, uint8_t ver_index);
void gt_cross_layout_ver_add_item(gt_obj_st* layout, gt_obj_st* obj);
//
void gt_cross_layout_set_hor_index(gt_obj_st* layout, uint8_t hor_index);
void gt_cross_layout_set_ver_index(gt_obj_st* layout, uint8_t ver_index);
gt_size_t gt_cross_layout_get_hor_count(gt_obj_st* layout);
gt_size_t gt_cross_layout_get_hor_index(gt_obj_st* layout);
gt_obj_st* gt_cross_layout_get_hor_select_obj(gt_obj_st* layout);
gt_size_t gt_cross_layout_get_ver_count(gt_obj_st* layout);
gt_size_t gt_cross_layout_get_ver_index(gt_obj_st* layout);
gt_obj_st* gt_cross_layout_get_ver_select_obj(gt_obj_st* layout);


#endif //!GT_CFG_ENABLE_CROSS_LAYOUT
//
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_BASE_LAYOUT_H_

/* end of file ----------------------------------------------------------*/


