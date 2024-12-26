/**
 * @file gt_table.h
 * @author Feyoung
 * @brief Table widget, support multi-row and multi-column table.
 *      Called gt_table_refresh() after add header and content text.
 * @version 0.1
 * @date 2024-12-19 17:35:47
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_TABLE_H_
#define _GT_TABLE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "./gt_conf_widgets.h"

#if GT_CFG_ENABLE_TABLE
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "../core/gt_style.h"
#include "../font/gt_font.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief Create a table / form object
 *
 * @param parent
 * @return gt_obj_st*
 */
gt_obj_st * gt_table_create(gt_obj_st * parent);

/**
 * @brief Add a header text to the table, Determines the
 *      number of columns in the list
 *
 * @param table
 * @param text
 * @param len
 * @param align @ref gt_align_et recommend:
 *      GT_ALIGN_LEFT_MID / GT_ALIGN_CENTER_MID / GT_ALIGN_RIGHT_MID
 * @return gt_res_t
 */
gt_res_t gt_table_add_header_text_by_len(gt_obj_st * table, const char * text, uint16_t len, gt_align_et align);

/**
 * @brief Add a content text to the table, Automatically
 *      switches to the next line for content filling
 *
 * @param table
 * @param text
 * @param len
 * @return gt_res_t
 */
gt_res_t gt_table_add_content_text_by_len(gt_obj_st * table, const char * text, uint16_t len);

/**
 * @brief Need to call this function after adding the header and content,
 *      calc the max width and height of each column
 *
 * @param table
 * @return gt_res_t
 */
gt_res_t gt_table_refresh(gt_obj_st * table);

void gt_table_set_font_color(gt_obj_st * table, gt_color_t color);

gt_color_t gt_table_get_font_color(gt_obj_st * table);

void gt_table_set_font_size(gt_obj_st * table, uint8_t size);
void gt_table_set_font_gray(gt_obj_st * table, uint8_t gray);
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
void gt_table_set_font_family_cn(gt_obj_st * table, gt_family_t family);
void gt_table_set_font_family_en(gt_obj_st * table, gt_family_t family);
void gt_table_set_font_family_fl(gt_obj_st * table, gt_family_t family);
void gt_table_set_font_family_numb(gt_obj_st * table, gt_family_t family);
#else
void gt_table_set_font_family(gt_obj_st * table, gt_family_t family);
void gt_table_set_font_cjk(gt_obj_st* table, gt_font_cjk_et cjk);
#endif
void gt_table_set_font_thick_en(gt_obj_st * table, uint8_t thick);
void gt_table_set_font_thick_cn(gt_obj_st * table, uint8_t thick);
void gt_table_set_font_encoding(gt_obj_st * table, gt_encoding_et encoding);

void gt_table_set_font_style(gt_obj_st * table, gt_font_style_et font_style);

uint8_t gt_table_get_font_size(gt_obj_st * table);

void gt_table_set_font_info(gt_obj_st * table, gt_font_info_st * font_info);
gt_font_info_st * gt_table_get_font_info(gt_obj_st * table);

/**
 * @brief Header title bold style
 *
 * @param table
 * @param is_bold
 */
void gt_table_set_header_bold(gt_obj_st * table, bool is_bold);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  /** GT_CFG_ENABLE_TABLE */
#endif //!_GT_TABLE_H_
