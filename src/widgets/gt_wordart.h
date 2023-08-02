/**
 * @file gt_wordart.h
 * @author Yang (your@email.com)
 * @brief 
 * @version 0.1
 * @date 2023-07-21 14:10:24
 * @copyright Copyright (c) 2014-2023, Company Genitop. Co., Ltd.
 */
#ifndef _GT_WORDART_H_
#define _GT_WORDART_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"
#include "./gt_conf_widgets.h"

#if GT_CFG_ENABLE_WORDART
/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_wordart_create(gt_obj_st * parent) ;

/**
 * @brief Example Initialize the maximum number of items
 * 
 * @param wordart obj
 * @param total maximum number
 */
void gt_wordart_items_init(gt_obj_st* wordart , uint32_t total);

/**
 * @brief add item
 * 
 * @param wordart obj
 * @param coding unicode
 * @param img image
 */
void gt_wordart_add_item(gt_obj_st* wordart , uint32_t coding , char* img);

void gt_wordart_set_text(gt_obj_st * wordart, const char * fmt, ...);

char * gt_wordart_get_text(gt_obj_st * wordart);

void gt_wordart_set_space(gt_obj_st * wordart, uint8_t space_x, uint8_t space_y);

#endif /* GT_CFG_ENABLE_WORDART */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_WORDART_H_

/* end of file ----------------------------------------------------------*/


