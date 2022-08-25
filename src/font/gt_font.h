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
#include "gt_color.h"
#include "../core/gt_style.h"


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
    char * utf8;     /* set: text: utf8 code arr */
    uint8_t size;       /* set: font size */
    uint16_t len;       /* set: utf8 arr len */
    uint8_t * res;      /* get: font data */
}gt_font_st;



typedef enum{
    GT_FONT_STYLE_NONE          = 0x00,
    GT_FONT_STYLE_UNDERLINE     = 0x01,
    GT_FONT_STYLE_STRIKETHROUGH = 0x02,
}gt_font_style_et;

typedef struct _gt_attr_font_s{
    gt_font_st * font;
    gt_color_t font_color;

    uint8_t space_x;
    uint8_t space_y;
    uint16_t start_x;
    uint16_t start_y;

    gt_align_et align;
    uint8_t style;      /*font style value @gt_font_style_et*/
}gt_attr_font_st;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
uint8_t utf8_to_unicode(uint8_t * utf8,uint32_t *res);

void gt_font_get_string_dot(gt_font_st * font);

uint32_t gt_font_count_word(char * utf8, uint32_t len);
uint32_t gt_font_get_string_width(gt_font_st * font);
uint8_t gt_font_get_one_word_width(uint32_t unicode, uint8_t font_size);
uint8_t gt_font_get_next_word_width(gt_font_st * font, uint8_t is_head);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_FONT_H_
