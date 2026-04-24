/**
 * @file gt_formula.h
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2026-03-24 15:00:48
 * @copyright Copyright (c) 2014-2026, Company Genitop. Co., Ltd.
 */
#ifndef _GT_FORMULA_H_
#define _GT_FORMULA_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"
#if GT_CFG_ENABLE_FORMULA
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "stdarg.h"
#include "stdlib.h"
#include "../utils/gt_vector.h"
#include "../core/gt_style.h"
#include "../font/gt_font.h"
#include "../utils/gt_roll_anim.h"

/* define ---------------------------------------------------------------*/
#define GT_FORMULA_FONT_MAX_LEVEL       (GT_LATEX_FONT_MODE_5)


/* typedef --------------------------------------------------------------*/
typedef struct {
    gt_latex_font_mode_t font_mode;
    gt_font_info_st info [GT_FORMULA_FONT_MAX_LEVEL];
}gt_formula_font_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_formula_create(gt_obj_st * parent);
void gt_formula_set_text(gt_obj_st * obj, const char * fmt, ...);
void gt_formula_set_text_by_len(gt_obj_st * obj, const char * text, uint16_t len);


void gt_formula_set_font_color(gt_obj_st * obj, gt_color_t color);

/**
 * @brief 设置公式最大宽度 max_width = 0 时，自动计算/ set formula max width, max_width = 0, auto calculate
 *
 * @param obj
 * @param max_width
 */
void gt_formula_set_max_width(gt_obj_st* obj, uint16_t max_width);
/**
 * @brief 设置公式字体/字号; 不支持 GT_FONT_FAMILY_OLD_ENABLE
 *
 * @param obj
 * @param cfg
 */
void gt_formula_set_font_config(gt_obj_st * obj, const gt_formula_font_st *cfg);

/**
 * @brief 设置公式首行偏移 / set formula first line offset
 *
 * @param obj
 * @param off_x
 * @param off_y
 */
void gt_formula_set_first_line_offset(gt_obj_st* obj, uint16_t off_x, uint16_t off_y);



#endif /* GT_CFG_ENABLE_FORMULA */
//
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_FORMULA_H_

/* end of file ----------------------------------------------------------*/


