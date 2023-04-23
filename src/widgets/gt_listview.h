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
typedef struct _gt_style_list_s
{
    char ** item;

    gt_color_t color_act;
    gt_color_t color_ina;
    gt_color_t color_background;

    gt_color_t border_color;
    uint16_t   border_width;

    gt_size_t idx_selected; // -1: init status; 0 or > 0: normal select status;
    gt_size_t idx_show_start;  //When displaying the contents of the list, start from the first few options
    gt_size_t cnt;
    gt_size_t cnt_show;    //When displaying the contents of the list, end of options
    gt_size_t content_offset_y;

    gt_color_t  font_color_act;
    gt_color_t  font_color_ina;
    uint8_t     font_size;
    gt_family_t font_family_cn;
    gt_family_t font_family_en;
    uint8_t     font_gray;
    uint8_t     font_align;
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
void gt_listview_style_add_item(gt_obj_st * list, char * item);

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

/**
 * @brief get listview selected item content
 *
 * @param listview
 * @return char* the content value of the selected item
 */
char * gt_listview_style_get_selected_item_text(gt_obj_st * listview);

/**
 * @brief set listview show items number
 * @param listview
 * @param cnt_show
 */
void gt_listview_style_set_cnt_show(gt_obj_st * listview, uint8_t cnt_show);

/**
 * @brief set listview show font size
 * @param listview
 * @param size
 */
void gt_listview_style_set_font_size(gt_obj_st * listview, uint8_t size);
void gt_listview_style_set_font_gray(gt_obj_st * listview, uint8_t gray);
void gt_listview_style_set_font_align(gt_obj_st * listview, uint8_t align);
/**
 * @brief Set listview active font color
 *
 * @param listview
 * @param color
 */
void gt_listview_style_set_font_color(gt_obj_st * listview, gt_color_t color);


void gt_listview_style_set_font_family_en(gt_obj_st * listview, gt_family_t family);
void gt_listview_style_set_font_family_cn(gt_obj_st * listview, gt_family_t family);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_LIST_H_
