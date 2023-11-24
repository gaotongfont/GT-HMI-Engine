/**
 * @file gt_listview.h
 * @author yongg
 * @brief list view
 * @version 0.1
 * @date 2022-07-18 13:39:51
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_LISTVIEW_H_
#define _GT_LISTVIEW_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief create listview obj
 *
 * @param parent listview's parent obj
 * @return gt_obj_st* listview obj ptr
 */
gt_obj_st * gt_listview_create(gt_obj_st * parent);

/**
 * @brief add item to listview obj
 *
 * @param list listview obj
 * @param item item
 */
void gt_listview_add_item(gt_obj_st * list, char * item);

/**
 * @brief set the listview selected item by index
 *
 * @param listview listview obj
 * @param idx index
 */
void gt_listview_set_selected_item_by_idx(gt_obj_st * listview, int idx);

/**
 * @brief get cnt of listview all items
 *
 * @param listview listview
 * @return gt_size_t cnt of count items
 */
gt_size_t gt_listview_get_list_items_cnt(gt_obj_st * listview);

/**
 * @brief get listview selected index
 *
 * @param listview
 * @return gt_size_t
 */
gt_size_t gt_listview_get_idx_selected(gt_obj_st * listview);

/**
 * @brief get listview selected item content
 *
 * @param listview
 * @return char* the content value of the selected item
 */
char * gt_listview_get_selected_item_text(gt_obj_st * listview);

/**
 * @brief set listview show items number
 * @param listview
 * @param cnt_show
 */
void gt_listview_set_cnt_show(gt_obj_st * listview, uint8_t cnt_show);

/**
 * @brief set listview show font size
 * @param listview
 * @param size
 */
void gt_listview_set_font_size(gt_obj_st * listview, uint8_t size);
void gt_listview_set_font_gray(gt_obj_st * listview, uint8_t gray);
void gt_listview_set_font_align(gt_obj_st * listview, uint8_t align);
/**
 * @brief Set listview active font color
 *
 * @param listview
 * @param color
 */
void gt_listview_set_font_color(gt_obj_st * listview, gt_color_t color);


void gt_listview_set_font_family_en(gt_obj_st * listview, gt_family_t family);
void gt_listview_set_font_family_cn(gt_obj_st * listview, gt_family_t family);
void gt_listview_set_font_family_numb(gt_obj_st * listview, gt_family_t family);

void gt_listview_set_font_thick_en(gt_obj_st * listview, uint8_t thick);
void gt_listview_set_font_thick_cn(gt_obj_st * listview, uint8_t thick);

void gt_listview_set_space(gt_obj_st * listview, uint8_t space_x, uint8_t space_y);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_LIST_H_
