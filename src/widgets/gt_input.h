/**
 * @file gt_input.h
 * @author yongg
 * @brief 
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
    uint16_t pos_selected_start;
    uint16_t pos_selected_end;
    uint16_t pos_cursor;

    uint8_t font_style_cn;
    uint8_t font_style_en;
    uint8_t font_size;
    
    char * value;
    char * placeholder;
}_gt_style_input_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_input_create(gt_obj_st * parent);

void gt_input_style_set_value(gt_obj_st * input, char * value);
void gt_input_style_append_value(gt_obj_st * input, char * value);
void gt_input_style_set_placeholder(gt_obj_st * input, const char * placeholder);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_INPUT_H_
