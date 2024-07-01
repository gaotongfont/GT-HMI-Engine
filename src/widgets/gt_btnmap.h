/**
 * @file gt_btnmap.h
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-03-04 11:11:23
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */
#ifndef _GT_BTNMAP_H_
#define _GT_BTNMAP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_BTNMAP
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "../core/gt_graph_base.h"
#include "../font/gt_font.h"

/* define ---------------------------------------------------------------*/
#define GT_BTNMAP_NEW_LINE    "\n"



/* typedef --------------------------------------------------------------*/
typedef struct _gt_map_s
{
    const char * kv;
    uint8_t w;
}gt_map_st;

/**
 * @brief disp special buttons
 *
 * @param kv buttons kv
 * @param rect_attr @gt_attr_rect_st
 * @param font_attr @gt_attr_font_st
 * @return  true: Revise  false: Not Revise
 */
typedef bool (* gt_btnmap_disp_special_btn_cb_t)(gt_obj_st* obj, const char* const kv, gt_attr_font_st* font_attr);

/**
 * @brief push button
 */
typedef void (* gt_btnmap_push_btn_kv_cb_t)(gt_obj_st* obj, gt_obj_st* input, const char* const kv);
/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief
 *
 * @param parent
 * @return gt_obj_st*
 */
gt_obj_st * gt_btnmap_create(gt_obj_st * parent);
void gt_btnmap_set_map(gt_obj_st* btnmap, gt_map_st* map, uint16_t map_type);
uint16_t gt_btnmap_get_map_type(gt_obj_st* btnmap);
void gt_btnmap_set_input(gt_obj_st * btnmap, gt_obj_st * input);

void gt_btnmap_set_radius(gt_obj_st* btnmap, uint8_t radius);
void gt_btnmap_set_btn_height(gt_obj_st* btnmap, uint16_t height);
void gt_btnmap_set_btn_height_auto_fill(gt_obj_st* btnmap, bool auto_fill);
bool gt_btnmap_get_btn_height_auto_fill(gt_obj_st* btnmap);
void gt_btnmap_set_btn_xy_space(gt_obj_st* btnmap, uint8_t x_space, uint8_t y_space);

void gt_btnmap_set_color_background(gt_obj_st * btnmap, gt_color_t color);
void gt_btnmap_set_border_width_and_color(gt_obj_st * btnmap, gt_size_t width, gt_color_t color);

void gt_btnmap_set_font_color(gt_obj_st * btnmap, gt_color_t color);
void gt_btnmap_set_font_size(gt_obj_st * btnmap, uint8_t size);
void gt_btnmap_set_font_gray(gt_obj_st * btnmap, uint8_t gray);
void gt_btnmap_set_font_family_cn(gt_obj_st * btnmap, gt_family_t family);
void gt_btnmap_set_font_family_en(gt_obj_st * btnmap, gt_family_t family);
void gt_btnmap_set_font_family_fl(gt_obj_st * btnmap, gt_family_t family);
void gt_btnmap_set_font_family_numb(gt_obj_st * btnmap, gt_family_t family);
void gt_btnmap_set_font_thick_en(gt_obj_st * btnmap, uint8_t thick);
void gt_btnmap_set_font_thick_cn(gt_obj_st * btnmap, uint8_t thick);
void gt_btnmap_set_font_encoding(gt_obj_st * btnmap, gt_encoding_et encoding);

void gt_btnmap_set_disp_special_btn_handler(gt_obj_st * btnmap, gt_btnmap_disp_special_btn_cb_t disp_special_btn_cb);
void gt_btnmap_set_push_btn_kv_handler(gt_obj_st * btnmap, gt_btnmap_push_btn_kv_cb_t push_btn_kv_cb);
void gt_btnmap_set_special_btn_color_background(gt_obj_st * btnmap, gt_color_t color);
void gt_btnmap_set_special_btn_border_width_and_color(gt_obj_st * btnmap, gt_size_t width, gt_color_t color);
void gt_btnmap_set_special_btn_font_color(gt_obj_st * btnmap, gt_color_t color);

void gt_btnmap_set_py_input_method(gt_obj_st * btnmap, gt_py_input_method_st* py_input_method);
gt_py_input_method_st* gt_btnmap_get_py_input_method(gt_obj_st * btnmap);

uint16_t gt_btnmap_get_max_key_count_of_lines(gt_obj_st * btnmap);
uint16_t gt_btnmap_get_line_count(gt_obj_st * btnmap);
uint16_t gt_btnmap_get_calc_minimum_width(gt_obj_st * btnmap);



#endif  /** GT_CFG_ENABLE_BTNMAP */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_BTNMAP_H_

/* end of file ----------------------------------------------------------*/


