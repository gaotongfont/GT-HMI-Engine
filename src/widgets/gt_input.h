/**
 * @file gt_input.h
 * @author yongg
 * @brief The area to input content or password
 * @version 0.1
 * @date 2022-07-20 13:37:30
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_INPUT_H_
#define _GT_INPUT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct _gt_style_input_s
{
    char * value;
    char * placeholder;

    uint16_t pos_selected_start;
    uint16_t pos_selected_end;
    uint16_t pos_cursor;

    gt_point_st point_pos;
    gt_size_t border_width;
    gt_color_t border_color;

		gt_color_t bg_color;
    gt_color_t font_color;
    uint8_t font_size;
    gt_family_t font_family_cn;
    gt_family_t font_family_en;
    uint8_t     font_gray;
}_gt_style_input_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief create input obj
 *
 * @param parent input's parent obj
 * @return gt_obj_st* input obj ptr
 */
gt_obj_st * gt_input_create(gt_obj_st * parent);

/**
 * @brief set input obj value
 *
 * @param input input obj
 * @param value value
 */
void gt_input_style_set_value(gt_obj_st * input, char * value);

/**
 * @brief get input obj value
 *
 * @param input input obj
 * @return char* value of input
 */
char * gt_input_style_get_value(gt_obj_st * input);


/**
 * @brief append content at the end
 *
 * @param input input obj
 * @param value content
 */
void gt_input_style_append_value(gt_obj_st * input, char * value);
void gt_input_style_append_value_encoding(gt_obj_st * input, char * value , uint8_t encoding);

/**
 * @brief set input obj placeholder
 *
 * @param input input obj
 * @param placeholder placeholder content
 */
void gt_input_style_set_placeholder(gt_obj_st * input, const char * placeholder);

/**
 * @brief delete a value in the input obj at pos
 *
 * @param input input obj
 */
void gt_input_style_del_value(gt_obj_st * input);

/**
 * @brief move the cursor one position to the left
 *
 * @param input input obj
 */
void gt_input_style_move_left_pos_cursor(gt_obj_st * input);

/**
 * @brief move the cursor one position to the right
 *
 * @param input input obj
 */
void gt_input_style_move_right_pos_cursor(gt_obj_st * input);

void gt_input_style_set_font_color(gt_obj_st * input, gt_color_t color);
void gt_input_style_set_font_family_cn(gt_obj_st * input, gt_family_t family);
void gt_input_style_set_font_family_en(gt_obj_st * input, gt_family_t family);
void gt_input_style_set_font_size(gt_obj_st * input, uint8_t size);
void gt_input_style_set_font_gray(gt_obj_st * input, uint8_t gray);

void gt_input_style_set_bg_color(gt_obj_st * input, gt_color_t color);
void gt_input_style_set_border_width(gt_obj_st * input, gt_size_t width);
void gt_input_style_set_border_color(gt_obj_st * input, gt_color_t color);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_INPUT_H_
