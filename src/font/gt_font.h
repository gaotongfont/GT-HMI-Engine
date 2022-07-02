/**
 * @file gt_font.h
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-06-01 11:22:06
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_FONT_H_
#define _GT_FONT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct _gt_font_res_s{
    uint8_t * dot;
    uint8_t w;
    uint8_t h;
}gt_font_res_st;

typedef struct _gt_font_s
{
    uint32_t style_cn;     /* set: font style */
    uint32_t style_en;     /* set: font style */
    uint8_t * utf8;     /* set: text: utf8 code arr */
    uint8_t size;       /* set: font size */
    uint16_t len;       /* set: utf8 arr len */
    uint8_t * res;      /* get: font data */
}gt_font_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_font_get_text(gt_font_st * font);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_FONT_H_
