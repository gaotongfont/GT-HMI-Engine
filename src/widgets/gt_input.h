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
#include "stdarg.h"
#include "stdlib.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct _gt_input_s
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
    uint8_t     font_align;
}_gt_input_st;



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
 * @param fmt value string format
 */
void gt_input_set_value(gt_obj_st * input, const char * fmt, ...);

/**
 * @brief get input obj value
 *
 * @param input input obj
 * @return char* value of input
 */
char * gt_input_get_value(gt_obj_st * input);


/**
 * @brief append content at the end
 *
 * @param input input obj
 * @param value content
 */
void gt_input_append_value(gt_obj_st * input, char * value);
void gt_input_append_value_encoding(gt_obj_st * input, char * value , uint8_t encoding);

/**
 * @brief set input obj placeholder
 *
 * @param input input obj
 * @param placeholder placeholder content
 */
void gt_input_set_placeholder(gt_obj_st * input, const char * placeholder);

/**
 * @brief delete a value in the input obj at pos
 *
 * @param input input obj
 */
void gt_input_del_value(gt_obj_st * input);

/**
 * @brief move the cursor one position to the left
 *
 * @param input input obj
 */
void gt_input_move_left_pos_cursor(gt_obj_st * input);

/**
 * @brief move the cursor one position to the right
 *
 * @param input input obj
 */
void gt_input_move_right_pos_cursor(gt_obj_st * input);

void gt_input_set_font_color(gt_obj_st * input, gt_color_t color);
void gt_input_set_font_family_cn(gt_obj_st * input, gt_family_t family);
void gt_input_set_font_family_en(gt_obj_st * input, gt_family_t family);
void gt_input_set_font_size(gt_obj_st * input, uint8_t size);
void gt_input_set_font_gray(gt_obj_st * input, uint8_t gray);
void gt_input_set_font_align(gt_obj_st * input, uint8_t align);

void gt_input_set_bg_color(gt_obj_st * input, gt_color_t color);
void gt_input_set_border_width(gt_obj_st * input, gt_size_t width);
void gt_input_set_border_color(gt_obj_st * input, gt_color_t color);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_INPUT_H_
