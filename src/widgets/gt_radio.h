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
    char * text;
}_gt_style_radio_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_radio_create(gt_obj_st * parent);
void gt_radio_style_set_selceted(gt_obj_st * radio);
void gt_radio_style_set_unselceted(gt_obj_st * radio);
uint8_t gt_radio_style_get_selceted_state(gt_obj_st * radio);

void gt_radio_style_set_text(gt_obj_st * radio, char * text);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_RADIO_H_
