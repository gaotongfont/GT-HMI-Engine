/**
 * @file gt_label.h
 * @author yongg
 * @brief label for content description
 * @version 0.1
 * @date 2022-07-13 11:55:38
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_LABEL_H_
#define _GT_LABEL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"



/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief create label obj
 *
 * @param parent label's parent obj
 * @return gt_obj_st* label obj ptr
 */
gt_obj_st * gt_label_create(gt_obj_st * parent);

/**
 * @brief set label obj text
 *
 * @param label label obj
 * @param fmt text format
 */
void gt_label_set_text(gt_obj_st * label, const char * fmt, ...);

/**
 * @brief get label text
 *
 * @param label label obj
 * @return char* label's text
 */
char * gt_label_get_text(gt_obj_st * label);

/**
 * @brief set label font color
 *
 * @param label label obj
 * @param color color
 */
void gt_label_set_font_color(gt_obj_st * label, gt_color_t color);

void gt_label_set_font_size(gt_obj_st * label, uint8_t size);
void gt_label_set_font_gray(gt_obj_st * label, uint8_t gray);
void gt_label_set_font_align(gt_obj_st * label, uint8_t align);
void gt_label_set_font_family_cn(gt_obj_st * label, gt_family_t family);
void gt_label_set_font_family_en(gt_obj_st * label, gt_family_t family);
void gt_label_set_font_family_numb(gt_obj_st * label, gt_family_t family);
void gt_label_set_font_thick_en(gt_obj_st * label, uint8_t thick);
void gt_label_set_font_thick_cn(gt_obj_st * label, uint8_t thick);

void gt_label_set_space(gt_obj_st * label, uint8_t space_x, uint8_t space_y);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_LABEL_H_
