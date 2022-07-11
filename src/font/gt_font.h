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
#define __GT_CHECK_NAME_IS_SAME(_name1, _name2)     (strcmp(#_name1,#_name2)==0)


/* typedef --------------------------------------------------------------*/
typedef enum{
    GT_FONT_SIZE_8x8    = 8,
    GT_FONT_SIZE_16x16  = 16,
    GT_FONT_SIZE_24x24  = 24,
    GT_FONT_SIZE_32x32  = 32,
}gt_font_size_et;

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
void gt_font_get_string_dot(gt_font_st * font);

uint32_t gt_font_count_word(char * utf8, uint32_t len);
uint32_t gt_font_get_string_width(gt_font_st * font);
uint8_t gt_font_get_one_word_width(uint32_t unicode, uint8_t font_size);
uint8_t gt_font_get_next_word_width(gt_font_st * font, uint8_t is_head);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_FONT_H_
