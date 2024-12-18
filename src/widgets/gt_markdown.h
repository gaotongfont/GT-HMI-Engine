/**
 * @file gt_markdown.h
 * @author Feyoung
 * @brief Markdown widget, using md4c library, support basic markdown syntax
 *      such as: h1~h6, p, ul, ol, li, etc.
 * @version 0.1
 * @date 2024-12-10 10:24:44
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_MARKDOWN_H_
#define _GT_MARKDOWN_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_MARKDOWN
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "../font/gt_font.h"
#include "../others/gt_types.h"



/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief Create a markdown content object
 *
 * @param parent
 * @return gt_obj_st*
 */
gt_obj_st * gt_markdown_create(gt_obj_st * parent);

/**
 * @brief Set the markdown source text,
 *      [Warn] Must after gt_obj_set_size() be called.
 *
 * @param markdown
 * @param text Markdown source text
 * @param len The length of the text
 */
void gt_markdown_set_text(gt_obj_st * markdown, const char * text, uint32_t len);

void gt_markdown_set_font_info_large(gt_obj_st * markdown, gt_font_info_st * font_info);
void gt_markdown_set_font_info_middle(gt_obj_st * markdown, gt_font_info_st * font_info);
void gt_markdown_set_font_info_small(gt_obj_st * markdown, gt_font_info_st * font_info);

gt_font_info_st * gt_markdown_get_font_info_large(gt_obj_st * markdown);
gt_font_info_st * gt_markdown_get_font_info_middle(gt_obj_st * markdown);
gt_font_info_st * gt_markdown_get_font_info_small(gt_obj_st * markdown);



#endif  /** GT_CFG_ENABLE_MARKDOWN */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_MARKDOWN_H_
