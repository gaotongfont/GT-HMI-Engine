/**
 * @file gt_listview.h
 * @author yongg
 * @brief list view
 * @version 0.1
 * @date 2022-07-18 13:39:51
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
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
 * @param text text
 */
void gt_listview_add_item(gt_obj_st * list, char * text);

/**
 * @brief add item to listview obj with icon
 *
 * @param listview
 * @param img icon path
 * @param text text
 */
void gt_listview_add_item_icon(gt_obj_st * listview, char * img, char * text);

/**
 * @brief item icon set to right side, default is left
 *
 * @param listview
 * @param enabled true: set to right side; false[default]: set to left side
 */
void gt_listview_set_icon_to_right(gt_obj_st * listview, bool enabled);

/**
 * @brief Set the listview item display height
 *
 * @param listview
 * @param height The height of the item
 */
void gt_listview_set_item_height(gt_obj_st * listview, uint16_t height);

/**
 * @brief Holding the selected item to highlight
 *
 * @param listview
 * @param enabled true: highlight the selected item; false: not highlight

 */
void gt_listview_set_highlight_mode(gt_obj_st * listview, bool enabled);

/**
 * @brief Highlight the selected item which is multiple selected.
 *
 * @param listview
 * @param enabled true: multiple selected; false: single selected
 */
void gt_listview_set_multiple(gt_obj_st * listview, bool enabled);

/**
 * @brief set the listview selected item by index
 *
 * @param listview listview obj
 * @param idx index
 */
void gt_listview_set_selected_item_by_idx(gt_obj_st * listview, gt_size_t idx);

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

void gt_listview_set_border_color(gt_obj_st * listview, gt_color_t color);
void gt_listview_set_border_width(gt_obj_st * listview, uint8_t width);

/**
 * @brief Sets the split line between each item to be displayed
 *
 * @param listview
 * @param enabled true: Enabled; false: Disabled
 */
void gt_listview_set_septal_line(gt_obj_st * listview, bool enabled);

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


void gt_listview_set_font_family_cn(gt_obj_st * listview, gt_family_t family);
void gt_listview_set_font_family_en(gt_obj_st * listview, gt_family_t family);
void gt_listview_set_font_family_fl(gt_obj_st * listview, gt_family_t family);
void gt_listview_set_font_family_numb(gt_obj_st * listview, gt_family_t family);

void gt_listview_set_font_thick_en(gt_obj_st * listview, uint8_t thick);
void gt_listview_set_font_thick_cn(gt_obj_st * listview, uint8_t thick);

void gt_listview_set_space(gt_obj_st * listview, uint8_t space_x, uint8_t space_y);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_LIST_H_
