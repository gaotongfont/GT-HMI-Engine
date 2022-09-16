/**
 * @file gt_listview.h
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-07-18 13:39:51
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_LIST_H_
#define _GT_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct _gt_style_list_s
{
    gt_color_t fg_color_selected;
    gt_color_t fg_color_unselected;
    gt_color_t bg_color;
    gt_color_t color_font_selected;
    gt_color_t color_font_unselected;
    gt_color_t color_border;
    uint16_t border_width;

    gt_size_t idx_selected;
    gt_size_t cnt;
    gt_size_t idx_show_start;  //When displaying the contents of the list, start from the first few options
    gt_size_t idx_show_cnt;    //When displaying the contents of the list, end of options

    uint8_t font_style_cn;
    uint8_t font_style_en;
    uint8_t font_size;
    
    char ** item;
}_gt_style_listview_st;



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
void gt_list_style_add_item(gt_obj_st * list, char * item);

/**
 * @brief set the listview selected item by index
 * 
 * @param listview listview obj
 * @param idx index
 */
void gt_listview_style_set_selected_item_by_idx(gt_obj_st * listview, int idx);

/**
 * @brief get cnt of listview all items
 * 
 * @param listview listview
 * @return gt_size_t cnt of count items
 */
gt_size_t gt_listview_style_get_list_items_cnt(gt_obj_st * listview);

/**
 * @brief get listview selected index
 * 
 * @param listview 
 * @return gt_size_t 
 */
gt_size_t gt_listview_style_get_idx_selected(gt_obj_st * listview);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_LIST_H_
