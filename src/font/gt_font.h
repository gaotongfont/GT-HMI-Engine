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
    extern int GT_Get_Font_Width(unsigned long fontcode, unsigned int font_option, unsigned int size, unsigned int gray, unsigned char* pBits);
    extern int GT_Get_Font_Height(unsigned long fontcode, unsigned int font_option, unsigned int size, unsigned int gray);
    extern char GT_Check_Is_Vec(unsigned int font_option);
    extern struct gt_font_func_s* GT_Get_Font_Func_S(unsigned int style);
    extern unsigned char GT_Get_Font_Type(unsigned int style);
#endif

/* define ---------------------------------------------------------------*/

#ifndef _GT_FONT_GET_WORD_BY_TOUCH_POINT
    /**
     * @brief 1: Get the word by touch point from draw text code run again, 0: disabled
     *      default: 0
     */
    #define _GT_FONT_GET_WORD_BY_TOUCH_POINT    0
#endif

#define __GT_CHECK_NAME_IS_SAME(_name1, _name2)     (strcmp(#_name1,#_name2)==0)

#define GT_PY_MAX_NUMB  (8)

/* typedef --------------------------------------------------------------*/

typedef struct _gt_font_info_s {
    gt_color_t palette;  ///< set: font color
    uint16_t style_cn;   ///< set: CJK font style
    uint16_t style_en;   ///< set: ascii font style
    uint16_t style_fl;   ///< set: font style foreign language
    uint16_t style_numb; ///< set: special number font style
    uint8_t size;        ///< set: font size
    uint8_t thick_en;    ///< set: font thick
    uint8_t thick_cn;    ///< set: font thick
    uint8_t gray;        ///< set: font gray
    uint8_t encoding;    ///< set: font encoding
    int8_t offset_y;    ///< 0: [default] don't move; < 0: move up; > 0: move down
}gt_font_info_st;

/**
 * @brief THe font resource structure
 */
typedef struct _gt_font_s {
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

typedef struct _gt_text_style_s {
    ///< 0[default]: Set the text beginning position by direct point, by start_x and start_y.
    uint8_t enabled_start   : 1;

    ///< font style value @ref gt_font_style_et
    uint8_t style           : 2;

    ///< 0[default]: multi-line, 1: single-line
    uint8_t single_line     : 1;

    ///< enabled when single-line set to 1.[0: display all word, 1: Characters are omitted at the end of lines]
    uint8_t omit_line       : 1;

#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
    ///< 0[default]: disabled touch point logical, 1: using touch point logical to get word
    uint8_t touch_point     : 1;
    ///< 0[default]: multi chinese words together, 1: single chinese word
    uint8_t single_cn       : 1;
#endif
    uint8_t reserved        : 1;
}_gt_text_style_st;

/**
 * @brief The font display info
 */
typedef struct _gt_attr_font_s {
    gt_font_st * font;      ///< Resource for font information
    gt_color_t font_color;  ///< The color of the font fore display

    /**
     * Single-line or multiple line text full display area required,
     * widget area is visible area window
     * defalut: logical_area = widget_area
     * |---------------------| <--- logical_area
     * | |--------| <--------+----- widget area
     * | |"this is|a window" |
     * | |visible |          |
     * | |--------|          |
     * |  invisible area     |
     * |---------------------|
     *  widget area is visible area window.
     */
    gt_area_st logical_area;

    gt_size_t start_x;  ///< draw text start x from coords,if not need,must set 0
    gt_size_t start_y;  ///< draw text start y from coords,if not need,must set 0

    uint8_t space_x;    ///< The width of the space between characters
    uint8_t space_y;    ///< The distance between rows

#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
    gt_point_st * touch_point;  ///< touch point logical, @ref reg.touch_point
#endif

    uint8_t align;      ///< Alignment @ gt_align_et
    gt_opa_t opa;       ///< font opa value @GT_OPA_X

    _gt_text_style_st reg;
}gt_attr_font_st;

typedef enum _gt_encoding_e {
    GT_ENCODING_UTF8 = 0,
    GT_ENCODING_GB,
}gt_encoding_et;

typedef enum _gt_encoding_convert_e {
    UTF8_2_GB = 0,
    GB_2_UTF8,
}gt_encoding_convert_et;

typedef enum _gt_font_lan_e {
    FONT_LAN_UNKNOWN = -1,
    FONT_LAN_CN = 0,
    FONT_LAN_ASCII,
    FONT_LAN_LATIN,
    FONT_LAN_GREEK,
    FONT_LAN_CYRILLIC,
    FONT_LAN_HEBREW,
    FONT_LAN_ARABIC,
    FONT_LAN_THAI,
    FONT_LAN_HINDI,
    FONT_LAN_JIAPANESE,
    FONT_LAN_KOREAN,

    //
    FONT_LAN_MAX_COUNT
}gt_font_lan_et;

typedef struct _gt_bidi_s {
    uint16_t idx;
    uint16_t len;
    uint8_t flag;   /** @ref gt_font_lan_et */
}gt_bidi_st;

typedef struct _gt_font_size_res_s {
    uint32_t font_buff_len;
    uint16_t dot_width;
    uint16_t font_per_size;
}_gt_font_size_res_st;

typedef struct gt_py_input_method_s {
#ifdef PINYIN_INPUT_METHOD_EN
    py_info_st * py_info;
#endif
    volatile uint8_t ascii_numb;
    volatile uint8_t chinese_numb;
    uint8_t ascii[GT_PY_MAX_NUMB];
    uint8_t chinese[GT_PY_MAX_NUMB][GT_PY_MAX_NUMB];
}gt_py_input_method_st;

typedef struct _gt_font_dot_ret_s {
    gt_font_type_em type;
    uint16_t size;
}_gt_font_dot_ret_st;

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/* py input method API interface -------------------------------------*/
gt_py_input_method_st* gt_py_input_method_create(void);
void gt_py_input_method_push_ascii(gt_py_input_method_st* py_input_method, uint8_t ch);
void gt_py_input_method_pop_ascii(gt_py_input_method_st* py_input_method);
uint8_t gt_py_input_method_get_ascii_numb(gt_py_input_method_st* py_input_method);
int gt_py_input_method_get_chinese(gt_py_input_method_st* py_input_method);
void gt_py_input_method_chinese_data_handler(gt_py_input_method_st* py_input_method);
void gt_py_input_method_last_page(gt_py_input_method_st* py_input_method);
void gt_py_input_method_next_page(gt_py_input_method_st* py_input_method);
void gt_py_input_method_select_text(gt_py_input_method_st* py_input_method, uint8_t index);
void gt_py_input_method_clean(gt_py_input_method_st* py_input_method);


void gt_font_info_init(gt_font_info_st *font_info);
void gt_font_info_update_font_thick(gt_font_info_st *font_info);

/**
 * @brief utf-8 encoding convert to unicode encoding
 *
 * @param utf8 utf-8 encoding
 * @param res unicode encoding
 * @return uint8_t THe number length bytes of utf-8 encoding
 */
uint8_t _gt_utf8_to_unicode(uint8_t * utf8, uint32_t * res);

/**
 * @brief unicode encoding convert to utf-8 encoding
 *
 * @param utf8 utf-8 encoding
 * @param unicode unicode encoding
 * @return uint8_t THe number length bytes of utf-8 encoding
 */
uint8_t gt_unicode_to_utf8(uint8_t * utf8, uint32_t unicode);

uint8_t gt_utf8_check_char(uint8_t * utf8);

/**
 * @brief Get one character encode dot data
 *
 * @param font [Warning] font->info.size will be change by real font size
 * @param unicode The unicode encoding of the character
 * @return _gt_font_dot_ret_st
 */
_gt_font_dot_ret_st gt_font_get_dot(gt_font_st * font, uint32_t unicode);

/**
 * @brief [Temporarily unused] Get the character data of a string
 *
 * @param font font basic information
 * @return font type and font size value enum @ref gt_font_type_em
 */
gt_font_type_em gt_font_get_string_dot(gt_font_st * font);

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
 * @param uni_or_gbk The unicode or gbk encoding of the character
 * @param font string msg
 * @return uint8_t The width of the character
 */
uint8_t gt_font_get_one_word_width(uint32_t uni_or_gbk, gt_font_st * font);

_gt_font_size_res_st gt_font_get_size_length_by_style(gt_font_info_st * info, uint8_t font_style, uint8_t langue, uint32_t text_len);

/**
 * @brief Get max substring line width
 *
 * @param info
 * @param text String
 * @param space x space
 * @return uint16_t The max width of the substring
 */
uint16_t gt_font_get_longest_line_substring_width(gt_font_info_st * info, const char * const text, uint16_t space);

void gt_project_encoding_set(gt_encoding_et charset);
gt_encoding_et gt_project_encoding_get(void);
int8_t _gt_font_get_type_group_offset_y(uint16_t cn_option, uint16_t en_option);

/**
 * @brief
 *
 * @param utf8
 * @param res
 * @param encoding
 * @return uint8_t The byte length of the encode
 */
uint8_t gt_font_one_char_code_len_get(uint8_t * utf8, uint32_t *res, uint8_t encoding);
uint8_t gt_encoding_table_one_char(uint8_t *src, uint8_t* dst, gt_encoding_convert_et tab);
uint8_t _gt_gb_font_one_char_code_len_get(uint8_t const * const utf8, uint32_t *res);
uint8_t gt_gb_check_char(const uint8_t *dst, uint16_t pos, uint32_t* font_code);

gt_font_lan_et _gt_is_style_cn_or_en(uint32_t unicode, uint8_t encoding);
bool _gt_font_is_convertor_language(gt_font_lan_et style_lang);
bool _gt_font_is_convertor_by(uint32_t unicode, uint8_t encoding);
uint16_t gt_font_get_word_width_figure_up(const uint8_t* data, uint16_t dot_w, uint16_t dot_h, uint8_t gray);

/**
 * @brief Get the font type
 *
 * @param font_style
 * @return uint8_t @ref gt_font_type_em
 */
uint8_t gt_font_type_get(unsigned int font_style);

int gt_font_code_transform(font_convertor_st *convert);
int gt_font_convertor_data_get(font_convertor_st *convert, uint32_t pos);

uint32_t gt_font_split(gt_font_st *fonts, uint32_t width, uint32_t dot_w, uint32_t space, uint32_t *ret_w, uint8_t * lan, uint32_t * lan_len);
bool gt_right_to_left_handler(const gt_font_st* fonts, uint8_t* ret_text, uint8_t r2l_lan);
/**
 * @brief Get the font style
 *
 * @param style
 * @return uint8_t @ref gt_font_lan_et
 */
uint8_t right_to_left_lan_get(uint16_t style);

uint32_t gt_font_split_line_numb(gt_font_info_st* info, const char * text, uint32_t max_w, uint16_t space, uint32_t * ret_max_w);


#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
/**
 * @brief legal char: 0-9, a-z, A-Z, Chinese, English, etc.
 *      Warn: "'" @ref _GT_FONT_APOSTROPHE_PUNCTUATION_MARK_LEGAL
 *      number legal enable @ref _GT_FONT_NUMBER_LEGAL
 *
 * @param uni_or_gbk
 * @return true illegal char
 * @return false legal char
 */
bool gt_font_is_illegal_char(uint32_t uni_or_gbk);
uint16_t gt_font_get_word_byte_length(char const * const text, uint16_t length, uint8_t encoding);
#endif



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_FONT_H_
