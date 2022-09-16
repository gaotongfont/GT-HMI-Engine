/**
 * @file gt_radio.h
 * @author yongg
 * @brief 
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
    uint8_t selected;
    
    uint8_t font_style_cn;
    uint8_t font_style_en;
    uint8_t font_size;

    char * text;
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
void gt_radio_style_set_selceted(gt_obj_st * radio);

/**
 * @brief set radio state to unselected
 * 
 * @param radio radio obj
 */
void gt_radio_style_set_unselceted(gt_obj_st * radio);

/**
 * @brief get radio state
 * 
 * @param radio radio obj
 */
uint8_t gt_radio_style_get_selceted_state(gt_obj_st * radio);

/**
 * @brief set radio text
 * 
 * @param radio radio obj
 * @param text text
 */
void gt_radio_style_set_text(gt_obj_st * radio, char * text);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_RADIO_H_
