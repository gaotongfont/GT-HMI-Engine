/**
 * @file gt_font.h
 * @author yongg
 * @brief The font support implementation
 * @version 0.1
 * @date 2022-06-01 11:22:06
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
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
#include "../widgets/gt_conf_widgets.h"

/**
 * @brief GT_Get_Font_Type() return value
 */
typedef enum {
    GT_FONT_TYPE_FLAG_NONE                 = 0,
    GT_FONT_TYPE_FLAG_DOT_MATRIX           = 1,
    GT_FONT_TYPE_FLAG_DOT_MATRIX_NON_WIDTH = 2,
    GT_FONT_TYPE_FLAG_VEC                  = 3,
}gt_font_type_em;

#if GT_CFG_ENABLE_ZK_FONT == 1
#ifdef GT_CONFIG_GUI_DRIVER_LIB
    #include "../../driver/gt_gui_driver.h"
#endif /* GT_CONFIG_GUI_DRIVER_LIB */
    extern int GT_Get_Font_Width(unsigned long fontcode, unsigned int font_option, unsigned int size , unsigned int gray , unsigned char* pBits);
    extern int GT_Get_Font_Height(unsigned long fontcode, unsigned int font_option, unsigned int size , unsigned int gray);
    extern char GT_Check_Is_Vec(unsigned int font_option);
    extern struct gt_font_func_s* GT_Get_Font_Func_S(unsigned int style);
    extern unsigned char GT_Get_Font_Type(unsigned int style);
#endif

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

typedef struct _gt_font_info_s{
    uint16_t style_cn;  ///< set: font style
    uint16_t style_en;  ///< set: font style
    uint16_t style_fl;  ///< set: font style foreign language
    uint16_t style_numb; ///< set: font style
    uint8_t size;       ///< set: font size
    uint8_t gray;       ///< set: font gray
    uint8_t encoding;   ///< set: font encoding
    uint8_t thick_en;   ///< set: font thick
    uint8_t thick_cn;   ///< set: font thick
}gt_font_info_st;

/**
 * @brief THe font resource structure
 */
typedef struct _gt_font_s
{
    char * utf8;        ///< set: text: utf8 code arr
    uint8_t * res;      ///< get: font data
    uint16_t len;       ///< set: utf8 arr len
    gt_font_info_st info;
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

    uint16_t start_x;   ///< draw text start x from coords,if not need,must set 0
    uint16_t start_y;   ///< draw text start y from coords,if not need,must set 0

    uint8_t space_x;    ///< The width of the space between characters
    uint8_t space_y;    ///< The distance between rows

    uint8_t align;      ///< Alignment @ gt_align_et
    uint8_t style;      ///< font style value @ref gt_font_style_et
    gt_opa_t opa;       ///< font opa value @GT_OPA_X
}gt_attr_font_st;


typedef enum _gt_encoding_e {

    GT_ENCODING_UTF8 = 0,
    GT_ENCODING_GB ,
}gt_encoding_et;

typedef enum _gt_encoding_convert_e {

    UTF8_2_GB = 0,
    GB_2_UTF8 ,
}gt_encoding_convert_et;

typedef enum _gt_font_style_en_cn_e {
    STYLE_CN = 0,
    STYLE_EN_ASCII ,
    STYLE_EN_LATIN ,
    STYLE_EN_GREEK ,
    STYLE_EN_CYRILLIC,
    STYLE_EN_HEBREW ,
    STYLE_EN_ARABIC ,
    STYLE_EN_THAI ,
    STYLE_EN_HINDI ,
    STYLE_EN_Currency ,
}gt_font_style_en_cn_et;

typedef struct _gt_bidi_s
{
    uint16_t idx;
    uint16_t len;
    uint8_t flag;
}gt_bidi_st;

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief utf-8 encoding convert to unicode encoding
 *
 * @param utf8 utf-8 encoding
 * @param res unicode encoding
 * @return uint8_t THe number length bytes of utf-8 encoding
 */
uint8_t _gt_utf8_to_unicode(uint8_t * utf8,uint32_t *res);

/**
 * @brief unicode encoding convert to utf-8 encoding
 *
 * @param utf8 utf-8 encoding
 * @param unicode unicode encoding
 * @return uint8_t THe number length bytes of utf-8 encoding
 */
uint8_t gt_unicode_to_utf8(uint8_t* utf8 , uint32_t unicode);

uint8_t gt_utf8_check_char(uint8_t * utf8);

/**
 * @brief Get the character data of a string
 *
 * @param font font basic information
 * @return font type value enum
 */
uint16_t gt_font_get_string_dot(gt_font_st * font);

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
 * @param font string msg
 * @return uint8_t The width of the character
 */
uint8_t gt_font_get_one_word_width(uint32_t unicode, gt_font_st * font);

/**
 * @brief get next word width from font_st
 *
 * @param font font msg
 * @param is_head this can be 0:start from cur, 1:start from head
 * @return uint8_t next word width
 */
uint8_t gt_font_get_next_word_width(gt_font_st * font, uint8_t is_head);


void gt_project_encoding_set(gt_encoding_et charset);
gt_encoding_et gt_project_encoding_get(void);
uint8_t gt_font_one_char_code_len_get(uint8_t * utf8,uint32_t *res , uint8_t encoding);
uint8_t gt_encoding_table_one_char(uint8_t *src ,uint8_t* dst , gt_encoding_convert_et tab );
uint8_t _gt_gb_font_one_char_code_len_get(uint8_t const * const utf8, uint32_t *res);
uint8_t gt_gb_check_char(const uint8_t *dst , uint16_t pos , uint32_t* fontcode);

gt_font_style_en_cn_et _gt_is_style_cn_or_en(uint32_t unicode , uint8_t encoding);
bool gt_style_en_data_is_convertor(uint32_t unicode , uint8_t encoding);
uint16_t gt_font_get_word_width_figure_up(const uint8_t* data , uint16_t dot_w , uint16_t dot_h , uint8_t gray);
uint8_t gt_font_type_get(unsigned int font_style);

int gt_font_code_transform(font_convertor_st *convert);
int gt_font_convertor_data_get(font_convertor_st *convert , uint32_t pos);

uint32_t gt_font_split(gt_font_st *fonts , uint32_t width , uint32_t dot_w ,uint32_t space , uint32_t *ret_w , uint8_t *lan , uint32_t* lan_len);
bool gt_right_to_left_handler(const gt_font_st* fonts , uint8_t* ret_text , uint8_t r2l_lan);
bool is_convertor_language(gt_font_style_en_cn_et style_laug);
uint8_t right_to_left_lan_get(uint16_t style);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_FONT_H_
