/**
 * @file gt_radio.h
 * @author yongg
 * @brief The implementation of the radio
 * @version 0.1
 * @date 2022-07-20 15:29:01
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_RADIO_H_
#define _GT_RADIO_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct _gt_style_radio_s
{
    char *      text;
    gt_color_t  font_color;
    gt_family_t font_family_cn;
    gt_family_t font_family_en;
    uint8_t     font_size;
    uint8_t     font_gray;
}_gt_style_radio_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief create radio obj
 *
 * @param parent radio's parent obj
 * @return gt_obj_st* radio obj ptr
 */
gt_obj_st * gt_radio_create(gt_obj_st * parent);

/**
 * @brief set radio state to selected
 *
 * @param radio radio obj
 */
void gt_radio_style_set_selected(gt_obj_st * radio);

/**
 * @brief set radio text
 *
 * @param radio radio obj
 * @param text text
 */
void gt_radio_style_set_text(gt_obj_st * radio, char * text);

void gt_radio_style_set_font_color(gt_obj_st * radio, gt_color_t color);
void gt_radio_style_set_font_size(gt_obj_st * radio, uint8_t size);
void gt_radio_style_set_font_gray(gt_obj_st * radio, uint8_t gray);
void gt_radio_style_set_font_family_cn(gt_obj_st * radio, gt_family_t family);
void gt_radio_style_set_font_family_en(gt_obj_st * radio, gt_family_t family);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_RADIO_H_
