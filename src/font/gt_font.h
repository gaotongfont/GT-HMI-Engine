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
#include "../core/gt_style.h"
#include "../others/gt_color.h"


/* define ---------------------------------------------------------------*/
#define __GT_CHECK_NAME_IS_SAME(_name1, _name2)     (strcmp(#_name1,#_name2)==0)


/* typedef --------------------------------------------------------------*/

typedef unsigned long(* get_dot_bitmap_t)(unsigned long , unsigned int , unsigned char* );
typedef unsigned int (* get_vec_bitmap_t)(unsigned char *,unsigned char ,unsigned long ,unsigned char ,unsigned char , unsigned char );

typedef struct _gt_font_style_s{
    get_dot_bitmap_t get_dot_bitmap;
    get_vec_bitmap_t get_vec_bitmap;

    uint32_t width:8;
    uint32_t height:8;
    uint32_t is_vec:1;
    uint32_t is_nor:1;
}gt_font_style_st;

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

/**
 * @brief THe font resource structure
 */
typedef struct _gt_font_s
{
    uint32_t style_cn;     ///< set: font style
    uint32_t style_en;     ///< set: font style
    char * utf8;     ///< set: text: utf8 code arr
    uint8_t size;       ///< set: font size
    uint16_t len;       ///< set: utf8 arr len
    uint8_t * res;      ///< get: font data
}gt_font_st;

/**
 * @brief The styles of the font display
 */
typedef enum gt_font_style_e {
    GT_FONT_STYLE_NONE          = 0x00,     ///< default status
    GT_FONT_STYLE_UNDERLINE     = 0x01,     ///< underline
    GT_FONT_STYLE_STRIKETHROUGH = 0x02,     ///< strikethrough
}gt_font_style_et;

/**
 * @brief The font display info
 */
typedef struct _gt_attr_font_s {
    gt_font_st * font;      ///< Resource for font information
    gt_color_t font_color;  ///< The color of the font fore display

    uint8_t space_x;    ///< The width of the space between characters
    uint8_t space_y;    ///< The distance between rows

    uint16_t start_x;   ///< draw text start x from coords,if not need,must set 0
    uint16_t start_y;   ///< draw text start y from coords,if not need,must set 0

    gt_align_et align;  ///< Alignment
    uint8_t style;      ///< font style value @ref gt_font_style_et
}gt_attr_font_st;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief utf-8 encoding convert to unicode encoding
 *
 * @param utf8 utf-8 encoding
 * @param res unicode encoding
 * @return uint8_t THe number length bytes of utf-8 encoding
 */
uint8_t utf8_to_unicode(uint8_t * utf8,uint32_t *res);

/**
 * @brief Get the character data of a string
 *
 * @param font font basic information
 */
void gt_font_get_string_dot(gt_font_st * font);

/**
 * @brief Count the number of characters in a string based on the utf-8 encoding
 *
 * @param utf8 utf8 encoding array
 * @param len The size of the utf-8 encoding array in bytes
 * @return uint32_t The number of characters
 */
uint32_t gt_font_count_word(char * utf8, uint32_t len);

/**
 * @brief get string all width
 *
 * @param font string msg
 * @return uint32_t all width
 */
uint32_t gt_font_get_string_width(gt_font_st * font);

/**
 * @brief Get the width of a character
 *
 * @param unicode The unicode encoding of the character
 * @param font_size The font size of character
 * @return uint8_t The width of the character
 */
uint8_t gt_font_get_one_word_width(uint32_t unicode, uint8_t font_size);

/**
 * @brief get next word width from font_st
 *
 * @param font font msg
 * @param is_head this can be 0:start from cur, 1:start from head
 * @return uint8_t next word width
 */
uint8_t gt_font_get_next_word_width(gt_font_st * font, uint8_t is_head);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_FONT_H_
