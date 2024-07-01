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
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_LISTVIEW
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "../core/gt_style.h"
#include "../font/gt_font.h"

/* define ---------------------------------------------------------------*/

#ifndef GT_LISTVIEW_CUSTOM_FONT_STYLE
    /**
     * @brief 0[defalut]: Use listview default font style, only support one font style;
     *        1: One of the listview items uses custom font style, which is defined by user.
     */
    #define GT_LISTVIEW_CUSTOM_FONT_STYLE           0
#endif

#ifndef GT_LISTVIEW_USE_ELEMENT_TYPE_RECT
    /**
     * @brief 0[defalut]: Do not use rectangle element;
     */
    #define GT_LISTVIEW_USE_ELEMENT_TYPE_RECT       0
#endif

/* typedef --------------------------------------------------------------*/
typedef enum gt_listview_element_type_s {
    GT_LISTVIEW_ELEMENT_TYPE_IMG = 0,
    GT_LISTVIEW_ELEMENT_TYPE_LABEL,

#if GT_LISTVIEW_USE_ELEMENT_TYPE_RECT
    GT_LISTVIEW_ELEMENT_TYPE_RECT,
#endif

    _GT_LISTVIEW_ELEMENT_TYPE_MAX,
}gt_listview_element_type_st;

/**
 * @brief Only used by gt_listview_add_item_by_param() function.
 */
typedef struct gt_listview_param_s {
    char * left_icon;
    char * right_icon;
    char * text_ptr;
    uint16_t text_len;
#if GT_LISTVIEW_CUSTOM_FONT_STYLE
    /**
     * @brief Pointer NULL: use listview default font style;
     *       NOT NULL: use custom font style, which is defined by user.
     * [Warn] gt_font_info_init() is recommended for initialization of defined variables.
     */
    gt_font_info_st * font_info_p;
#endif
}gt_listview_param_st;

#if GT_LISTVIEW_USE_ELEMENT_TYPE_RECT
typedef struct gt_listview_rect_s {
    gt_color_t bg_color;
    gt_color_t border_color;
    uint16_t border;
    uint8_t radius;
    uint8_t is_fill;
}gt_listview_rect_st;
#endif

typedef struct gt_listview_custom_item_s {
    char * src;
    uint16_t src_len;
    uint16_t item_idx;      /** The item index of listview widget */
    gt_area_st area;        /** The position relative item location */
    uint8_t type;           /** @ref gt_listview_element_type_st */
    uint8_t element_idx;    /** The element index of item object */
#if GT_LISTVIEW_CUSTOM_FONT_STYLE
    /**
     * @brief Pointer NULL: use listview default font style;
     *       NOT NULL: use custom font style, which is defined by user.
     * [Warn] gt_font_info_init() is recommended for initialization of defined variables.
     */
    gt_font_info_st * font_info_p;
#endif
#if GT_LISTVIEW_USE_ELEMENT_TYPE_RECT
    gt_listview_rect_st rect;
#endif
}gt_listview_custom_item_st;


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
 * @brief Sets the number of elements that can be displayed in subsequent rows
 *
 * @param listview
 * @param column The subsequent rows will be displayed elements number of items, default is 1
 */
void gt_listview_set_next_row_item_count(gt_obj_st * listview, uint8_t count);

/**
 * @brief add item to listview obj
 *
 * @param listview listview obj
 * @param text text
 * @return item object, which child[0]: label
 */
gt_obj_st * gt_listview_add_item(gt_obj_st * listview, char const * text);

/**
 * @brief add item to listview obj with icon
 *
 * @param listview
 * @param img icon path
 * @param text text
 * @return item object, which child[0]: img, child[1]: label
 */
gt_obj_st * gt_listview_add_item_icon(gt_obj_st * listview, char * img, char const * text);

/**
 * @brief add item to listview obj with [ icon + text + icon ] mode
 *
 * @param listview
 * @param left_icon left icon path
 * @param text text
 * @param right_icon right icon path
 * @return item object, which child[0]: left img, child[1]: label, child[2]: right img
 */
gt_obj_st * gt_listview_add_item_icons(gt_obj_st * listview, char * left_icon, char const * text, char * right_icon);

/**
 * @brief add item to listview obj
 *
 * @param listview listview obj
 * @param param Must be reset NULL before used.
 * @return item object, which child[0]: label
 */
gt_obj_st * gt_listview_add_item_by_param(gt_obj_st * listview, gt_listview_param_st * param);

/**
 * @brief add item to listview obj with custom element
 *
 * @param listview
 * @param item
 * @return gt_obj_st*
 */
gt_obj_st * gt_listview_custom_item_set_element(gt_obj_st * listview, gt_listview_custom_item_st * item);

/**
 * @brief Clear all items in the listview
 *
 * @param listview
 */
void gt_listview_clear_all_items(gt_obj_st * listview);

/**
 * @brief set [ icon + text ] or [ text + icon ] mode scale
 *      within item area to display
 *      such as: icon_width = parent_width * icon_scale / (icon_scale + label_scale)
 *      [default icon: 25, label: 75]
 *
 * @param listview
 * @param icon 0 ~ 255
 * @param label 0 ~ 255
 */
void gt_listview_set_scale(gt_obj_st * listview, uint8_t icon, uint8_t label);

/**
 * @brief set only [ icon + text + icon ] mode scale within item area to display
 *      such as: left_width = parent_width * left_scale / (left_scale + center_scale + right_scale)
 *      [default left: 20, center: 60, right: 20]
 *
 * @param listview
 * @param left 0 ~ 255
 * @param center 0 ~ 255
 * @param right 0 ~ 255
 */
void gt_listview_set_scale_triple(gt_obj_st * listview, uint8_t left, uint8_t center, uint8_t right);

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
 * @brief Enabled the listview item selected effect
 *
 * @param listview
 * @param enabled 1[Default]: Enabled; 0: Disabled
 */
void gt_listview_set_selected_effect(gt_obj_st * listview, bool enabled);

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
 * @brief Set the listview background color
 *
 * @param listview
 * @param color
 */
void gt_listview_set_bg_color(gt_obj_st * listview, gt_color_t color);

/**
 * @brief Set the listview item background color
 *
 * @param listview
 * @param color
 */
void gt_listview_set_item_bg_color(gt_obj_st * listview, gt_color_t color);

/**
 * @brief Display the listview item background
 *
 * @param listview
 * @param show false[default]: hide; true: show
 */
void gt_listview_show_item_bg(gt_obj_st * listview, bool show);
/**
 * @brief Set the listview item reduce area value, default is 4
 *
 * @param listview
 * @param reduce REDUCE_DEFAULT[default]: 4
 */
void gt_listview_set_item_reduce(gt_obj_st * listview, uint8_t reduce);

/**
 * @brief Set the listview item radius
 *
 * @param listview
 * @param radius default: 10
 */
void gt_listview_set_item_radius(gt_obj_st * listview, uint8_t radius);

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
void gt_listview_set_font_align(gt_obj_st * listview, gt_align_et align);
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
void gt_listview_set_font_encoding(gt_obj_st * listview, gt_encoding_et encoding);

void gt_listview_set_space(gt_obj_st * listview, uint8_t space_x, uint8_t space_y);

void gt_listview_set_label_omit_single_line_by(gt_obj_st * listview, bool is_omit);
void gt_listview_set_label_auto_scroll_single_line_by(gt_obj_st * listview, bool is_auto_scroll);



#endif  /** GT_CFG_ENABLE_LISTVIEW */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_LIST_H_
