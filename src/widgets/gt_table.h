/**
 * @file gt_table.h
 * @author Feyoung
 * @brief Table widget, support multi-row and multi-column table
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
gt_res_t gt_table_add_header_text_by_len(gt_obj_st * table, char * text, uint16_t len, gt_align_et align);

/**
 * @brief Add a content text to the table, Automatically
 *      switches to the next line for content filling
 *
 * @param table
 * @param text
 * @param len
 * @return gt_res_t
 */
gt_res_t gt_table_add_content_text_by_len(gt_obj_st * table, char * text, uint16_t len);

gt_res_t gt_table_refresh(gt_obj_st * table);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  /** GT_CFG_ENABLE_TABLE */
#endif //!_GT_TABLE_H_
