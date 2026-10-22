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
#include "../font/gt_font.h"
#if GT_CFG_ENABLE_FORMULA && GT_LATEX_ENABLE
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

/**
 * @brief 公式渲染模式 / Formula render mode
 */
typedef enum {
    GT_FORMULA_MODE_LATEX = 0,  /* LaTeX 数学公式模式（默认）/ LaTeX math formula mode (default) */
    GT_FORMULA_MODE_MD    = 1,  /* Markdown 文档模式 / Markdown document mode */
} gt_formula_mode_t;

/**
 * @brief 公式缓存模式 / Formula cache mode
 */
typedef enum {
    GT_FORMULA_CACHE_MODE_PERSIST = 0,  /* 保留缓存,  / Keep cache */
    GT_FORMULA_CACHE_MODE_NONE,         /* 不保留缓存 / No cache retention */
} gt_formula_cache_mode_t;

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

/**
 * @brief 设置公式渲染模式 / Set formula render mode
 *
 * @param obj formula 对象 / formula object
 * @param mode GT_FORMULA_MODE_LATEX（LaTeX 数学公式）或 GT_FORMULA_MODE_MD（Markdown 文档）
 *             GT_FORMULA_MODE_LATEX (LaTeX math) or GT_FORMULA_MODE_MD (Markdown document)
 */
void gt_formula_set_mode(gt_obj_st* obj, gt_formula_mode_t mode);

/**
 * @brief 获取当前公式渲染模式 / Get current formula render mode
 *
 * @param obj formula 对象 / formula object
 * @return gt_formula_mode_t 当前模式 / current mode
 */
gt_formula_mode_t gt_formula_get_mode(gt_obj_st* obj);

/**
 * @brief 设置缓存模式 / Set formula cache mode
 *
 * @param obj formula 对象 / formula object
 * @param cache_mode GT_FORMULA_CACHE_MODE_PERSIST 或 GT_FORMULA_CACHE_MODE_NONE
 */
void gt_formula_set_cache_mode(gt_obj_st* obj, gt_formula_cache_mode_t cache_mode);

/**
 * @brief 获取缓存模式 / Get formula cache mode
 *
 * @param obj formula 对象 / formula object
 * @return gt_formula_cache_mode_t 当前缓存模式 / current cache mode
 */
gt_formula_cache_mode_t gt_formula_get_cache_mode(gt_obj_st* obj);

/**
 * @brief 设置 GT-MD 内部 LaTeX 渲染模式 / Set internal LaTeX render mode for GT-MD
 * @param obj formula 对象 / formula object
 * @param mode 目标模式 / Target mode @ gt_md_latex_mode_t
 *        GT_MD_LATEX_MODE_DIRECT: render 阶段直接调用 gt_latex_draw()
 *        GT_MD_LATEX_MODE_PREPARED: layout 阶段准备 cache，render 阶段只调用 gt_latex_render()
 *
 * @note 若 GT_MD_CFG_LATEX_MODE_FIXED == 1，则该接口不改变实际模式
 */
void gt_formula_set_md_latex_mode(gt_obj_st* obj, uint8_t mode);

#endif /* GT_CFG_ENABLE_FORMULA */
//
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_FORMULA_H_

/* end of file ----------------------------------------------------------*/


