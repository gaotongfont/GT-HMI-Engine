/**
 * @file gt_font.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-06-01 11:22:22
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_font.h"
#include "stdint.h"
#include "stdbool.h"
#include "../others/gt_log.h"
#include "../core/gt_mem.h"
#include "../others/gt_gc.h"
#include "gt_symbol.h"
/* private define -------------------------------------------------------*/

#ifndef GT_CFG_USE_FONT_16_ASCII
    /**
     * @brief
     */
    #define GT_CFG_USE_FONT_16_ASCII    01
#endif

#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
#ifndef _GT_FONT_APOSTROPHE_PUNCTUATION_MARK_LEGAL
    /**
     * @brief 1: The apostrophe punctuation mark is a legal character, 0: disabled
     *      "'" such as: "don't" [1: "don't", 0: "don"]
     *      default: 01
     */
    #define _GT_FONT_APOSTROPHE_PUNCTUATION_MARK_LEGAL    01
#endif
#endif

#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
#ifndef _GT_FONT_NUMBER_LEGAL
    /**
     * @brief 1: Numbers are legal characters, 0: disabled
     *      such as: "1234" [1: "1234", 0: ""]
     *      default: 0
     */
    #define _GT_FONT_NUMBER_LEGAL    0
#endif
#endif

#define GT_FONT_FAMILY_CHECK(_list, _family) ((!_list.fam_list) || (_list.count == 0) || ((_family) >= (_list.count)))
/* private typedef ------------------------------------------------------*/
/**
 * @brief 0: defalut, < 0: up, > 0: down
 */
typedef struct {
    int8_t y;
}_font_type_group_offset_st;

/**
 * @brief Adjust the position of the ASCII font type with the Chinese font type
 */
typedef struct {
    uint16_t cn_opt;
    uint16_t en_opt;
    _font_type_group_offset_st offset;
}_font_type_group_adjust_ascii_pos_st;

typedef struct {
    uint32_t start;
    uint32_t end;
    gt_font_lan_et language;
}_unicode_range_st;

/* static variables -----------------------------------------------------*/
static GT_ATTRIBUTE_RAM_DATA const uint8_t _ascii_16[];

static GT_ATTRIBUTE_RAM_DATA const uint8_t _latin_16x16_width[] = {
    // latin
    // 1.1:0x0020UL<=unicode<=0x007FUL
    0x04,  0x04,  0x08,  0x09,  0x09,  0x10,  0x0d,  0x04,  0x06,  0x05,  0x09,  0x09,  0x04,  0x06,  0x04,  0x05,  0x09,  0x07,  0x09,  0x09,  0x09,  0x09,  0x09,  0x09,  0x09,  0x09,  0x06,  0x06,  0x09,  0x09,  0x09,  0x0a,  0x10,  0x0c,  0x0c,  0x0c,  0x0c,  0x0b,  0x0a,  0x0c,  0x0c,  0x04,  0x09,  0x0c,  0x0a,  0x0d,  0x0c,  0x0c,  0x0b,  0x0c,  0x0d,  0x0b,  0x0b,  0x0c,  0x0c,  0x10,  0x0b,  0x0b,  0x0a,  0x06,  0x05,  0x06,  0x09,  0x0a,  0x04,  0x0a,  0x0a,  0x09,  0x0a,  0x09,  0x06,  0x0a,  0x0a,  0x04,  0x04,  0x09,  0x04,  0x0e,  0x0a,  0x0a,  0x0a,  0x0a,  0x07,  0x09,  0x06,  0x0a,  0x09,  0x0e,  0x09,  0x0a,  0x09,  0x07,  0x04,  0x07,  0x0a,
    // 1.2:0x00A0UL<=unicode<=0x017FUL
    0x04,  0x04,  0x09,  0x0a,  0x0a,  0x09,  0x04,  0x0a,  0x06,  0x0d,  0x06,  0x09,  0x09,  0x06,  0x0d,  0x0a,  0x06,  0x09,  0x06,  0x06,  0x05,  0x09,  0x0a,  0x04,  0x06,  0x05,  0x07,  0x09,  0x0e,  0x0d,  0x0e,  0x0a,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x10,  0x0c,  0x0b,  0x0b,  0x0b,  0x0b,  0x04,  0x05,  0x05,  0x05,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x08,  0x0d,  0x0c,  0x0c,  0x0c,  0x0c,  0x0b,  0x0b,  0x0a,  0x0a,  0x0a,  0x0a,  0x0a,  0x0a,  0x0a,  0x0e,  0x09,  0x09,  0x09,  0x09,  0x09,  0x04,  0x05,  0x05,  0x05,  0x0a,  0x0a,  0x0a,  0x0a,  0x0a,  0x0a,  0x0a,  0x09,  0x0a,  0x0a,  0x0a,  0x0a,  0x0a,  0x0a,  0x0a,  0x0a,  0x0c,  0x0a,  0x0c,  0x0a,  0x0c,  0x0a,  0x0c,  0x09,  0x0c,  0x09,  0x0c,  0x09,  0x0c,  0x09,  0x0c,  0x0d,  0x0c,  0x0b,  0x0b,  0x09,  0x0b,  0x09,  0x0b,  0x09,  0x0b,  0x09,  0x0b,  0x09,  0x0c,  0x0a,  0x0c,  0x0a,  0x0c,  0x0a,  0x0c,  0x0a,  0x0c,  0x0a,  0x0d,  0x0a,  0x05,  0x05,  0x05,  0x05,  0x05,  0x05,  0x05,  0x05,  0x04,  0x04,  0x0d,  0x09,  0x0a,  0x05,  0x0c,  0x09,  0x0a,  0x0a,  0x05,  0x0a,  0x05,  0x0b,  0x07,  0x0a,  0x07,  0x0a,  0x05,  0x0c,  0x0a,  0x0c,  0x0a,  0x0c,  0x0a,  0x0c,  0x0c,  0x0a,  0x0c,  0x0a,  0x0c,  0x0a,  0x0c,  0x0a,  0x10,  0x0f,  0x0d,  0x07,  0x0d,  0x07,  0x0d,  0x07,  0x0b,  0x09,  0x0b,  0x09,  0x0b,  0x09,  0x0b,  0x09,  0x0a,  0x06,  0x0a,  0x09,  0x0b,  0x06,  0x0c,  0x0a,  0x0c,  0x0a,  0x0c,  0x0a,  0x0c,  0x0a,  0x0c,  0x0a,  0x0c,  0x0b,  0x10,  0x0e,  0x0b,  0x0a,  0x0b,  0x0a,  0x09,  0x0a,  0x09,  0x0a,  0x09,
    // 1.3:0x01A0UL<=unicode<=0x01CFUL
    0x0e,  0x0c,  0x0d,  0x0d,  0x0c,  0x09,  0x0d,  0x0b,  0x09,  0x0a,  0x08,  0x08,  0x0c,  0x07,  0x0c,  0x0e,  0x0d,  0x0e,  0x0c,  0x0a,  0x0c,  0x0b,  0x09,  0x0b,  0x0b,  0x09,  0x0a,  0x0b,  0x0a,  0x09,  0x09,  0x0a,  0x04,  0x05,  0x09,  0x04,  0x10,  0x10,  0x10,  0x10,  0x0d,  0x08,  0x10,  0x10,  0x0e,  0x0c,  0x0a,
    // 1.4:0x01F0UL<=unicode<=0x01FFUL
    0x07,  0x10,  0x10,  0x10,  0x0c,  0x0a,  0x10,  0x0b,  0x0c,  0x0a,  0x0c,  0x0a,  0x10,  0x0e,  0x0d,
    // 1.5:0x0210UL<=unicode<=0x021FUL
    0x0d,  0x07,  0x0d,  0x07,  0x0c,  0x0a,  0x0c,  0x0a,  0x0b,  0x09,  0x0b,  0x07,  0x0b,  0x08,  0x0c,
    // 1.6:0x1EA0UL<=unicode<=0x1EFFUL
    0x0c,  0x0a,  0x0c,  0x0a,  0x0c,  0x0b,  0x0c,  0x0a,  0x0c,  0x0b,  0x0c,  0x0a,  0x0c,  0x0a,  0x0c,  0x0a,  0x0c,  0x0a,  0x0c,  0x0a,  0x0c,  0x0a,  0x0c,  0x0a,  0x0b,  0x09,  0x0b,  0x09,  0x0b,  0x09,  0x0c,  0x0b,  0x0b,  0x09,  0x0d,  0x0c,  0x0b,  0x09,  0x0b,  0x09,  0x05,  0x05,  0x04,  0x04,  0x0c,  0x0a,  0x0c,  0x0a,  0x0d,  0x0c,  0x0c,  0x0a,  0x0d,  0x0c,  0x0c,  0x0a,  0x0c,  0x0a,  0x0c,  0x0b,  0x0d,  0x0b,  0x0d,  0x0b,  0x0d,  0x0b,  0x0c,  0x0a,  0x0c,  0x0a,  0x0c,  0x0a,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0c,  0x0b,  0x0a,  0x0b,  0x0a,  0x0b,  0x0a,  0x0b,  0x0a,  0x0a,  0x0a,  0x0a,  0x0a,  0x0a,
};

static GT_ATTRIBUTE_RAM_DATA const uint32_t _latin_encoding_range[] = {
    //start     end
    0x0020UL, 0x007FUL,
    0x00A0UL, 0x017FUL,
    0x01A0UL, 0x01CFUL,
    0x01F0UL, 0x01FFUL,
    0x0210UL, 0x021FUL,
    0x1EA0UL, 0x1EFFUL,
};

/**
 * @brief Adjust the ascii display position according to the match between Chinese and ascii
 */
static GT_ATTRIBUTE_RAM_DATA const _font_type_group_adjust_ascii_pos_st _font_type_group_arr[] = {
    /** 16 */
#if defined(GT_FAMILY_GB2312_16_ST) && defined(GT_FAMILY_ASCII_16x16_T)
    {.cn_opt = gt_family_gb2312_16_st, .en_opt = gt_family_ascii_16x16_t, .offset = { .y = 1 } },
#endif
#if defined(GT_FAMILY_GB2312_16_ST) && defined(GT_FAMILY_ASCII_16x16_A)
    {.cn_opt = gt_family_gb2312_16_st, .en_opt = gt_family_ascii_16x16_a, .offset = { .y = 1 } },
#endif
#if defined(GT_FAMILY_GBK_16_ST) && defined(GT_FAMILY_ASCII_16x16_T)
    {.cn_opt = gt_family_gbk_16_st, .en_opt = gt_family_ascii_16x16_t, .offset = { .y = 1 } },
#endif
#if defined(GT_FAMILY_GBK_16_ST) && defined(GT_FAMILY_ASCII_16x16_A)
    {.cn_opt = gt_family_gbk_16_st, .en_opt = gt_family_ascii_16x16_a, .offset = { .y = 1 } },
#endif
#if defined(GT_FAMILY_GB18030_16_ST) && defined(GT_FAMILY_ASCII_16x16_T)
    {.cn_opt = gt_family_gb18030_16_st, .en_opt = gt_family_ascii_16x16_t, .offset = { .y = 1 } },
#endif
#if defined(GT_FAMILY_GB18030_16_ST) && defined(GT_FAMILY_ASCII_16x16_A)
    {.cn_opt = gt_family_gb18030_16_st, .en_opt = gt_family_ascii_16x16_a, .offset = { .y = 1 } },
#endif
#if defined(GT_FAMILY_GB2312_16_YH) && defined(GT_FAMILY_ASCII_16x16_T)
    {.cn_opt = gt_family_gb2312_16_yh, .en_opt = gt_family_ascii_16x16_t, .offset = { .y = 1 } },
#endif
#if defined(GT_FAMILY_GB2312_16_YH) && defined(GT_FAMILY_ASCII_16x16_A)
    {.cn_opt = gt_family_gb2312_16_yh, .en_opt = gt_family_ascii_16x16_a, .offset = { .y = 1 } },
#endif
    /** 32 */
#if defined(GT_FAMILY_GB2312_32_ST) && defined(GT_FAMILY_ASCII_32x32_T)
    {.cn_opt = gt_family_gb2312_32_st, .en_opt = gt_family_ascii_32x32_t, .offset = { .y= 2 } },
#endif
#if defined(GT_FAMILY_GB2312_32_ST) && defined(GT_FAMILY_ASCII_32x32_A)
    {.cn_opt = gt_family_gb2312_32_st, .en_opt = gt_family_ascii_32x32_a, .offset = { .y= 2 } },
#endif
#if defined(GT_FAMILY_GBK_32_ST) && defined(GT_FAMILY_ASCII_32x32_T)
    {.cn_opt = gt_family_gbk_32_st, .en_opt = gt_family_ascii_32x32_t, .offset = { .y= 2 } },
#endif
#if defined(GT_FAMILY_GBK_32_ST) && defined(GT_FAMILY_ASCII_32x32_A)
    {.cn_opt = gt_family_gbk_32_st, .en_opt = gt_family_ascii_32x32_a, .offset = { .y= 2 } },
#endif
#if defined(GT_FAMILY_GB18030_32_ST) && defined(GT_FAMILY_ASCII_32x32_T)
    {.cn_opt = gt_family_gb18030_32_st, .en_opt = gt_family_ascii_32x32_t, .offset = { .y= 2 } },
#endif
#if defined(GT_FAMILY_GB18030_32_ST) && defined(GT_FAMILY_ASCII_32x32_A)
    {.cn_opt = gt_family_gb18030_32_st, .en_opt = gt_family_ascii_32x32_a, .offset = { .y= 2 } },
#endif
#if defined(GT_FAMILY_GB2312_32_HT) && defined(GT_FAMILY_ASCII_32x32_T)
    {.cn_opt = gt_family_gb2312_32_ht, .en_opt = gt_family_ascii_32x32_t, .offset = { .y= 2 } },
#endif
#if defined(GT_FAMILY_GB2312_32_HT) && defined(GT_FAMILY_ASCII_32x32_A)
    {.cn_opt = gt_family_gb2312_32_ht, .en_opt = gt_family_ascii_32x32_a, .offset = { .y= 2 } },
#endif

    /** gray */
    /** 16 */
#if defined(GT_FAMILY_GB2312_16_HT_4bit) && defined(GT_FAMILY_ASCII_16x16_HT_4bit)
    {.cn_opt = gt_family_gb2312_16_ht_4bit, .en_opt = gt_family_ascii_16x16_ht_4bit, .offset = { .y= 1 } },
#endif
#if defined(GT_FAMILY_GB2312_16_HT_4bit) && defined(GT_FAMILY_ASCII_16x16_ZK_4bit)
    {.cn_opt = gt_family_gb2312_16_ht_4bit, .en_opt = gt_family_ascii_16x16_zk_4bit, .offset = { .y= 2 } },
#endif
#if defined(GT_FAMILY_GB2312_16_ZK_4bit) && defined(GT_FAMILY_ASCII_16x16_HT_4bit)
    {.cn_opt = gt_family_gb2312_16_zk_4bit, .en_opt = gt_family_ascii_16x16_ht_4bit, .offset = { .y= 1 } },
#endif
#if defined(GT_FAMILY_GB2312_16_ZK_4bit) && defined(GT_FAMILY_ASCII_16x16_ZK_4bit)
    {.cn_opt = gt_family_gb2312_16_zk_4bit, .en_opt = gt_family_ascii_16x16_zk_4bit, .offset = { .y= 2 } },
#endif
#if defined(GT_FAMILY_GBK_16_HT_4bit) && defined(GT_FAMILY_ASCII_16x16_HT_4bit)
    {.cn_opt = gt_family_gbk_16_ht_4bit, .en_opt = gt_family_ascii_16x16_ht_4bit, .offset = { .y= 1 } },
#endif
#if defined(GT_FAMILY_GBK_16_HT_4bit) && defined(GT_FAMILY_ASCII_16x16_ZK_4bit)
    {.cn_opt = gt_family_gbk_16_ht_4bit, .en_opt = gt_family_ascii_16x16_zk_4bit, .offset = { .y= 2 } },
#endif
#if defined(GT_FAMILY_GBK_16_ZK_4bit) && defined(GT_FAMILY_ASCII_16x16_HT_4bit)
    {.cn_opt = gt_family_gbk_16_zk_4bit, .en_opt = gt_family_ascii_16x16_ht_4bit, .offset = { .y= 1 } },
#endif
#if defined(GT_FAMILY_GBK_16_ZK_4bit) && defined(GT_FAMILY_ASCII_16x16_ZK_4bit)
    {.cn_opt = gt_family_gbk_16_zk_4bit, .en_opt = gt_family_ascii_16x16_zk_4bit, .offset = { .y= 2 } },
#endif
    /** 24 */
#if defined(GT_FAMILY_GB2312_24_HT_4bit) && defined(GT_FAMILY_ASCII_24x24_HT_4bit)
    {.cn_opt = gt_family_gb2312_24_ht_4bit, .en_opt = gt_family_ascii_24x24_ht_4bit, .offset = { .y= 2 } },
#endif
#if defined(GT_FAMILY_GB2312_24_HT_4bit) && defined(GT_FAMILY_ASCII_24x24_ZK_4bit)
    {.cn_opt = gt_family_gb2312_24_ht_4bit, .en_opt = gt_family_ascii_24x24_zk_4bit, .offset = { .y= 2 } },
#endif
#if defined(GT_FAMILY_GBK_24_HT_4bit) && defined(GT_FAMILY_ASCII_24x24_HT_4bit)
    {.cn_opt = gt_family_gbk_24_ht_4bit, .en_opt = gt_family_ascii_24x24_ht_4bit, .offset = { .y= 2 } },
#endif
#if defined(GT_FAMILY_GBK_24_HT_4bit) && defined(GT_FAMILY_ASCII_24x24_ZK_4bit)
    {.cn_opt = gt_family_gbk_24_ht_4bit, .en_opt = gt_family_ascii_24x24_zk_4bit, .offset = { .y= 2 } },
#endif
    /** 32 */
#if defined(GT_FAMILY_GB2312_32_HT_4bit) && defined(GT_FAMILY_ASCII_32x32_HT_4bit)
    {.cn_opt = gt_family_gb2312_32_ht_4bit, .en_opt = gt_family_ascii_32x32_ht_4bit, .offset = { .y= 2 } },
#endif
#if defined(GT_FAMILY_GB2312_32_HT_4bit) && defined(GT_FAMILY_ASCII_32x32_ZK_4bit)
    {.cn_opt = gt_family_gb2312_32_ht_4bit, .en_opt = gt_family_ascii_32x32_zk_4bit, .offset = { .y= 2 } },
#endif
    /** null value */
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
    { .cn_opt = GT_CFG_DEFAULT_FONT_FAMILY_CN, .en_opt = GT_CFG_DEFAULT_FONT_FAMILY_EN, .offset = { .y = 0 } },
#else
    { .cn_opt = GT_CFG_DEFAULT_FONT_FAMILY, .en_opt = GT_CFG_DEFAULT_FONT_FAMILY, .offset = { .y = 0 } },
#endif
};

static GT_ATTRIBUTE_RAM_DATA const gt_size_t _font_type_group_arr_len = sizeof(_font_type_group_arr) / sizeof(_font_type_group_adjust_ascii_pos_st) - 1;


static gt_font_family_list_st _font_family_list = {NULL, 0};

/* macros ---------------------------------------------------------------*/
static gt_encoding_et _gt_project_encoding = GT_ENCODING_UTF8;
#ifdef PINYIN_INPUT_METHOD_EN
static GT_ATTRIBUTE_RAM_DATA py_info_st* _gt_py_info;
#endif
/* static functions -----------------------------------------------------*/
/**
 * @brief get font width by cache
 *
 * @param uni_or_gbk
 * @param option
 * @param font_size
 * @param gray
 * @return gt_size_t
 */
static GT_ATTRIBUTE_RAM_TEXT inline gt_size_t gt_font_get_font_width_inline(uint32_t uni_or_gbk, uint16_t option, uint16_t font_size, uint8_t gray, uint8_t * res) {
    gt_size_t width = 0;
    if (GT_CFG_DEFAULT_FONT_FAMILY == option) {
        return width;
    }
#if GT_FONT_USE_ASCII_WIDTH_CACHE
    _gt_gc_ascii_width_cache_st * const cache = _gt_gc_get_ascii_width_cache();
    if (uni_or_gbk < 0x20|| uni_or_gbk > 0x7F) {
        return GT_Get_Font_Width(uni_or_gbk, option, font_size, gray, res);
    }
    if (cache->option != option || cache->size != font_size || cache->gray != gray) {
        // check cache valid
        gt_memset(cache->ascii_width_cache, 0, sizeof(gt_font_width_t) * _FONT_ASCII_WIDTH_CACHE_COUNT);
        cache->option = option;
        cache->size = font_size;
        cache->gray = gray;
        width = GT_Get_Font_Width(uni_or_gbk, option, font_size, gray, res);
        cache->ascii_width_cache[uni_or_gbk - 0x20] = (gt_font_width_t)width;
    } else {
        // read width from cache
        width = cache->ascii_width_cache[uni_or_gbk - 0x20];
    }
    if (0 == width) {
        width = GT_Get_Font_Width(uni_or_gbk, option, font_size, gray, res);
        cache->ascii_width_cache[uni_or_gbk - 0x20] = (gt_font_width_t)width;
    }
#else
    width = GT_Get_Font_Width(uni_or_gbk, option, font_size, gray, res);
#endif
    return width;
}

/**
 * @brief get idx in unicode_arr by unicode (for )
 *
 * @param unicode_arr
 * @param depth
 * @param unicode
 * @return uint32_t
 */
static GT_ATTRIBUTE_RAM_TEXT uint32_t _get_idx(uint32_t const * const unicode_arr, uint16_t depth, uint32_t unicode) {
    uint16_t idx = 0;
    while (idx < depth) {
        if (unicode >= unicode_arr[idx << 2] && unicode <= unicode_arr[(idx << 2) + 1]) {
            idx += unicode - unicode_arr[idx << 2];
            break;
        } else {
            idx += unicode_arr[(idx << 2) + 1] - unicode_arr[idx << 2];
        }
    }
    return idx;
}

/**
 * @brief check unicode is latin
 *
 * @param unicode unicode
 * @return true :the unicode is latin
 * @return false :the unicode not is latin
 */
static GT_ATTRIBUTE_RAM_TEXT bool _gt_font_latin_check(uint32_t unicode) {
    bool res = false;
    const uint32_t * encoding_range_p = _latin_encoding_range;
    uint16_t len = (sizeof(_latin_encoding_range) / sizeof(_latin_encoding_range[0])) >> 1;
    for (uint16_t i = 0; i < len; i++) {
        if (unicode >= *encoding_range_p && unicode <= *(encoding_range_p + 1)) {
            res = true;
        }
        encoding_range_p += 2;
    }
    return res;
}

/**
 * @brief Inline chars get one latin word width
 *
 * @param unicode latin unicode
 * @param font_size font size
 * @return uint8_t the latin unicode font width
 */
static GT_ATTRIBUTE_RAM_TEXT uint8_t _gt_font_latin_get_width(uint32_t unicode, uint8_t font_size) {
    uint16_t len = (sizeof(_latin_encoding_range) / sizeof(_latin_encoding_range[0])) >> 1;
    uint32_t idx = _get_idx(_latin_encoding_range, len, unicode);

    /** default */
    return _latin_16x16_width[idx];
}

static GT_ATTRIBUTE_RAM_TEXT bool _gt_number_symbol_font(uint32_t unicode) {
    // 0123456789,.:'$€￥
    if ((unicode >= 0x30 && unicode <= 0x39) || \
        0x3A == unicode || 0x2C == unicode || 0x2E == unicode || \
        0x27 == unicode || 0x24 == unicode || 0xFFE5 == unicode || 0x20AC == unicode \
    ) {
        return true;
    }

    return false;
}

static GT_ATTRIBUTE_RAM_TEXT uint32_t _get_font_option(gt_font_st * font, uint32_t unicode)
{
    gt_font_lan_et font_lan = FONT_LAN_UNKNOWN;

#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))

    uint16_t font_style = font->info.style_fl;

    if (GT_CFG_DEFAULT_FONT_FAMILY_NUMB != font->info.style_numb && _gt_number_symbol_font(unicode)) {
        return font->info.style_numb;
    }
    font_lan = gt_font_lan_get(unicode, font->info.encoding);

    if(IS_CN_FONT_LAN(font_lan)){
        font_style = font->info.style_cn;
    }
    else if (FONT_LAN_ASCII == font_lan){
        font_style = font->info.style_en;
    }
    return font_style;
#else

    if(_gt_number_symbol_font(unicode)){
        uint32_t tmp_option = gt_font_family_get_option(font->info.family, FONT_LAN_NUMBER, font->info.cjk);
        if(GT_CFG_DEFAULT_FONT_FAMILY != tmp_option){
            return tmp_option;
        }
    }

    font_lan = gt_font_lan_get(unicode, font->info.encoding);
    return gt_font_family_get_option(font->info.family, font_lan, font->info.cjk);
#endif
}

/* global functions / API interface -------------------------------------*/

void gt_font_family_init(const gt_font_family_st* fam_list, uint16_t count)
{
    _font_family_list.fam_list = fam_list;
    _font_family_list.count = count;
}

bool gt_font_family_is_one_style(uint16_t fam)
{
    return (fam & GT_FONT_FAMILY_ONE_TYPE);
}

uint16_t gt_font_family_get_size(uint16_t fam)
{
    if(GT_FONT_FAMILY_CHECK(_font_family_list , fam)){
        return GT_CFG_DEFAULT_FONT_SIZE;
    }
    return _font_family_list.fam_list[fam].size;
}

uint16_t gt_font_family_get_option(uint16_t fam, int16_t lan, uint8_t cjk)
{
    if(gt_font_family_is_one_style(fam)){
        return (fam & (~(GT_FONT_FAMILY_ONE_TYPE)));
    }

    if(GT_FONT_FAMILY_CHECK(_font_family_list , fam)){
        return GT_CFG_DEFAULT_FONT_FAMILY;
    }

    if((lan != FONT_LAN_CJK_UNIFIED) && (lan >= FONT_LAN_MAX_COUNT)) return GT_CFG_DEFAULT_FONT_FAMILY;

    if(FONT_LAN_CJK_UNIFIED == lan) {
        if(GT_FONT_CJK_C == cjk){
            lan = FONT_LAN_CN;
        }else{
            lan = cjk == GT_FONT_CJK_J ? FONT_LAN_JAPANESE : FONT_LAN_KOREAN;
        }
    }

    return _font_family_list.fam_list[fam].option[lan];
}

void gt_font_set_family(gt_font_info_st *font_info, gt_family_t fam)
{
    if(!font_info) return ;
#if !(defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
    font_info->family = fam;
    if(!gt_font_family_is_one_style(fam)){
        font_info->size = gt_font_family_get_size(fam);
    }
#endif
}

gt_size_t gt_font_family_get_id_by(uint16_t font_size)
{
    if (NULL == _font_family_list.fam_list || 0 == _font_family_list.count) {
        return -1;
    }
    for (uint16_t i = 0; i < _font_family_list.count; ++i) {
        if (_font_family_list.fam_list[i].size == font_size) {
            return i;
        }
    }
    return -1;
}

void gt_font_info_init(gt_font_info_st *font_info)
{
    font_info->palette    = gt_color_black();
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
    font_info->style_cn   = GT_CFG_DEFAULT_FONT_FAMILY_CN;
    font_info->style_en   = GT_CFG_DEFAULT_FONT_FAMILY_EN;
    font_info->style_fl   = GT_CFG_DEFAULT_FONT_FAMILY_FL;
    font_info->style_numb = GT_CFG_DEFAULT_FONT_FAMILY_NUMB;
    font_info->size       = GT_CFG_DEFAULT_FONT_SIZE;
#else
    font_info->cjk = GT_FONT_CJK_C;
    font_info->family = GT_CFG_DEFAULT_FONT_FAMILY;
    font_info->size = gt_font_family_get_size(font_info->family);
#endif
    font_info->gray       = 1;
    font_info->thick_en   = 0;
    font_info->thick_cn   = 0;
    font_info->encoding   = gt_project_encoding_get();
    font_info->offset_y   = 0;
    font_info->style.all  = 0;
}

void gt_font_info_update_font_thick(gt_font_info_st *font_info)
{
    font_info->thick_en = font_info->thick_en == 0 ? font_info->size + 6: font_info->thick_en;
    font_info->thick_cn = font_info->thick_cn == 0 ? font_info->size + 6: font_info->thick_cn;
}

uint8_t _gt_gb_font_one_char_code_len_get(uint8_t const * const gbk_code, uint32_t *res)
{
    if (NULL == gbk_code) { return 0; }

    uint32_t len = strlen((char * )gbk_code);
    uint16_t high_val = 0;

    if (len >= 1 && gbk_code[0] <= 0x7F) {
        if (res) { *res = gbk_code[0]; }
        return 1;
    }

    high_val = (gbk_code[0] << 8) | gbk_code[1];
    if( len >= 4 && ((high_val == 0x8139) || (high_val >= 0x8230 && high_val <= 0x8235))) {
        if (res) {
            *res = (high_val << 16) | (gbk_code[2] << 8) | gbk_code[3];
        }
        return 4;
    }
    else if (len >= 2) {
        if (res) { *res = high_val; }
        return 2;
    }
    return len;
}

uint8_t _gt_sjis_oen_char_code_len_get(uint8_t const * const gbk_code, uint32_t *res)
{
    if(NULL == gbk_code) { return 0; }
    uint32_t len = strlen((char * )gbk_code);

    if (len >= 1 && gbk_code[0] <= 0x7F) {
        if (res) { *res = gbk_code[0]; }
        return 1;
    }
    else if(len >= 2){
        if (res) { *res = (gbk_code[0] << 8) | gbk_code[1]; }
        return 2;
    }

    return len;
}



/**
 * @brief unicode encoding convert to utf-8 encoding
 *
 * @param utf8 utf-8 encoding
 * @param unicode unicode encoding
 * @return uint8_t THe number length bytes of utf-8 encoding
 */
uint8_t gt_unicode_to_utf8(uint8_t* utf8, uint32_t unicode)
{
    if(NULL == utf8){
        return 0;
    }

    if(unicode <= 0x007FUL){
        *utf8 = (unicode & 0x7F);
        return 1;
    }
    else if( unicode >= 0x0080UL && unicode <= 0x07FFUL ){
        *(utf8+1) = (unicode & 0x3F) | 0x80;
        *utf8     = ((unicode >> 6) & 0x1F) | 0xC0;
        return 2;
    }
    else if ( unicode >= 0x0800UL && unicode <= 0xFFFFUL )
    {
        *(utf8+2) = (unicode & 0x3F) | 0x80;
        *(utf8+1) = ((unicode >>  6) & 0x3F) | 0x80;
        *utf8     = ((unicode >> 12) & 0x0F) | 0xE0;
        return 3;
    }
    else if ( unicode >= 0x00010000  && unicode <= 0x001FFFFF ){
        *(utf8+3) = (unicode & 0x3F) | 0x80;
        *(utf8+2) = ((unicode >>  6) & 0x3F) | 0x80;
        *(utf8+1) = ((unicode >> 12) & 0x3F) | 0x80;
        *utf8     = ((unicode >> 18) & 0x07) | 0xF0;
        return 4;
    }
    else if ( unicode >= 0x200000UL && unicode <= 0x3FFFFFFUL )
    {
        *(utf8+4) = (unicode & 0x3F) | 0x80;
        *(utf8+3) = ((unicode >>  6) & 0x3F) | 0x80;
        *(utf8+2) = ((unicode >> 12) & 0x3F) | 0x80;
        *(utf8+1) = ((unicode >> 18) & 0x3F) | 0x80;
        *utf8     = ((unicode >> 24) & 0x03) | 0xF8;
        return 5;
    }
    else if ( unicode >= 0x4000000UL && unicode <= 0x7FFFFFFFUL )
    {
        *(utf8+5) = (unicode & 0x3F) | 0x80;
        *(utf8+4) = ((unicode >>  6) & 0x3F) | 0x80;
        *(utf8+3) = ((unicode >> 12) & 0x3F) | 0x80;
        *(utf8+2) = ((unicode >> 18) & 0x3F) | 0x80;
        *(utf8+1) = ((unicode >> 24) & 0x3F) | 0x80;
        *utf8     = ((unicode >> 30) & 0x01) | 0xFC;
        return 6;
    }
    return 0;
}




uint8_t _gt_utf8_to_unicode(uint8_t * utf8, uint32_t *res)
{
	if (utf8[0] < 0x80) {
		*res = utf8[0];
		return 1;
	} else if (utf8[0] < 0xE0) {
		*res = ((utf8[0] & 0x1F) << 6) | (utf8[1] & 0x3F);
		return 2;
	} else if (utf8[0] < 0xF0) {
		*res = ((utf8[0] & 0x0F) << 12) | ((utf8[1] & 0x3F) << 6) | (utf8[2] & 0x3F);
		return 3;
	} else if (0xF0 == (utf8[0] & 0xF0)) {
		*res = ((utf8[0] & 0x07) << 18) | ((utf8[1] & 0x3F) << 12) | ((utf8[2] & 0x3F) << 6) | (utf8[3] & 0x3F);
		return 4;
	}
	return 0;
}

int8_t _gt_font_get_type_group_offset_y(uint16_t cn_option, uint16_t en_option)
{
    int8_t offset = 0;
    for (gt_size_t i = _font_type_group_arr_len - 1; i >= 0; --i) {
        if (cn_option == _font_type_group_arr[i].cn_opt && en_option == _font_type_group_arr[i].en_opt) {
            offset = _font_type_group_arr[i].offset.y;
            break;
        }
    }
    return offset;
}

uint8_t gt_font_one_char_code_len_get(uint8_t * utf8_or_bgk, uint32_t *res, uint8_t encoding)
{
#if GT_CFG_ENABLE_ZK_FONT == 1
    GT_Set_Chinese_Charset_Type(encoding);
#endif
    if(GT_ENCODING_GB == encoding) {
        return _gt_gb_font_one_char_code_len_get(utf8_or_bgk, res);
    }
    else if(GT_ENCODING_SJIS == encoding) {
        return _gt_sjis_oen_char_code_len_get(utf8_or_bgk, res);
    }

    return _gt_utf8_to_unicode(utf8_or_bgk, res);
}

static GT_ATTRIBUTE_RAM_TEXT uint8_t _gt_gb_code_len_get(uint32_t gb_code)
{
    if(gb_code <= 0x7F){
        return 1;
    }
    else if(0x0000 != (gb_code >> 16))
    {
        return 4;
    }
    return 2;
}

static GT_ATTRIBUTE_RAM_TEXT uint8_t _gt_utf8_to_gb(const uint8_t *src, uint16_t src_len, uint8_t* dst, uint16_t dst_len)
{
    uint32_t out_len = 0;
#if GT_CFG_ENABLE_ZK_FONT == 1
    uint32_t font_code = 0, i = 0;
    uint8_t len = 0, tmp_len = 0, tmp_i = 0;
    for(i = 0; i < src_len;){
        len = _gt_utf8_to_unicode((uint8_t*)&src[i], &font_code);
        if(0 == len){
            return out_len;
        }
        i += len;
        if(font_code <= 0x7F){
            tmp_len = 1;
            goto fill;
        }
#if UNICODETOGBK
        font_code = UnicodeToGBK(font_code);
#endif
        if(0x00 == font_code){
            font_code = 0x3F;
        }
        tmp_len = _gt_gb_code_len_get(font_code);
fill:
        if(out_len + tmp_len > dst_len){
            return out_len;
        }
        for(tmp_i = 0; tmp_i < tmp_len; tmp_i++) {
            dst[out_len + tmp_len -1 - tmp_i] = (uint8_t)((font_code >> (tmp_i * 8))&0xFF);
        }
        out_len += tmp_len;
    }
#endif
    return out_len;
}

static GT_ATTRIBUTE_RAM_TEXT uint16_t _gt_gb_to_utf8(const uint8_t *src, uint16_t src_len, uint8_t* dst, uint16_t dst_len)
{
    uint32_t out_len = 0;
#if GT_CFG_ENABLE_ZK_FONT == 1
    uint32_t font_code = 0, i = 0;
    uint8_t len = 0, tmp_len = 0, tmp_i = 0;

    for(i = 0; i < src_len; ){
        len = _gt_gb_font_one_char_code_len_get(&src[i], &font_code);
        if(0 == len){
            return out_len;
        }
        i += len;
        font_code = (font_code <= 0x7F) ? font_code : GBKToUnicode(font_code);
        if(0x00 == font_code){
            font_code = 0x3F;
        }
        tmp_len = gt_unicode_to_utf8(&dst[out_len], font_code);
        if(out_len + tmp_len > dst_len){
            return out_len;
        }
        out_len += tmp_len;
    }

#endif
    return out_len;
}

static GT_ATTRIBUTE_RAM_TEXT uint16_t _gt_unicode_to_utf8_buf(const uint8_t *src, uint16_t src_len, uint8_t* dst, uint16_t dst_len)
{
    uint32_t out_len = 0;
    uint32_t font_code = 0, i = 0;
    uint8_t tmp_len = 0;

    for(i = 0; i < src_len; i+=2)
    {
        font_code = (src[i] << 8) | (src[i + 1]);
        tmp_len = gt_unicode_to_utf8(&dst[out_len], font_code);
        if(out_len + tmp_len > dst_len){
            return out_len;
        }
        out_len += tmp_len;
    }

    return out_len;
}

#if BIG5TOGBK
static GT_ATTRIBUTE_RAM_TEXT uint16_t _gt_big5_to_gbk(const uint8_t *src, uint16_t src_len, uint8_t* dst, uint16_t dst_len)
{
    uint32_t out_len = 0;
#if GT_CFG_ENABLE_ZK_FONT == 1
    uint32_t font_code = 0, i = 0;
    uint8_t len = 0, tmp_len = 0, tmp_i = 0;

    for(i = 0; i < src_len;){
        if(src[i] <= 0x7F){
            font_code = src[i];
            tmp_len = 1;
            len = 1;
            goto fill;
        }
        font_code = (src[i] << 8) | (src[i + 1]);
        font_code = BIG5ToGBK(font_code);
        tmp_len = 2;
        len = 2;
fill:
        i += len;
        if(0x00 == font_code){
            font_code = 0x3F;
            tmp_len = 1;
        }
        if(out_len + tmp_len > dst_len){
            return out_len;
        }
        for(tmp_i = 0; tmp_i < tmp_len; tmp_i++) {
            dst[out_len + tmp_len -1 - tmp_i] = (uint8_t)((font_code >> (tmp_i * 8))&0xFF);
        }
        out_len += tmp_len;
    }
#endif
    return out_len;
}
#endif

#if SHIFT_JIS_TO_JIS0208
static GT_ATTRIBUTE_RAM_TEXT uint16_t _gt_shiftjis_to_jis0208(const uint8_t *src, uint16_t src_len, uint8_t* dst, uint16_t dst_len)
{
    uint32_t out_len = 0;
#if GT_CFG_ENABLE_ZK_FONT == 1
    uint32_t font_code = 0, i = 0;
    uint8_t len = 0, tmp_len = 0, tmp_i = 0;

    for(i = 0; i < src_len; i+=2){
        font_code = (src[i] << 8) | (src[i + 1]);
        font_code = Shift_JIS_To_JIS0208(font_code);
        tmp_len = 2;
        if(0x00 == font_code){
            font_code = 0x0109;
        }
        if(out_len + tmp_len > dst_len){
            return out_len;
        }
        for(tmp_i = 0; tmp_i < tmp_len; tmp_i++) {
            dst[out_len + tmp_len -1 - tmp_i] = (uint8_t)((font_code >> (tmp_i * 8))&0xFF);
        }
        out_len += tmp_len;
    }
#endif
    return out_len;
}
#endif

uint16_t gt_encoding_convert(const uint8_t *src, uint16_t src_len, uint8_t* dst, uint16_t dst_len, gt_encoding_convert_et enc_cov)
{
    switch (enc_cov)
    {
    case UTF8_2_GB:
        return _gt_utf8_to_gb(src, src_len, dst,dst_len);
    case GB_2_UTF8:
        return _gt_gb_to_utf8(src, src_len, dst,dst_len);
    case UNICODE_2_UTF8:
        return _gt_unicode_to_utf8_buf(src, src_len, dst,dst_len);
#if BIG5TOGBK
    case BIG5_2_GBK:
        return _gt_big5_to_gbk(src, src_len, dst,dst_len);
#endif
#if SHIFT_JIS_TO_JIS0208
    case SJIS_2_JIS0208:
        return _gt_shiftjis_to_jis0208(src, src_len, dst,dst_len);
#endif
    default:
        break;
    }

    return 0;
}

uint8_t gt_encoding_table_one_char(uint8_t *src, uint8_t* dst, gt_encoding_convert_et tab)
{
    uint32_t font_code;
    uint8_t len = 0;

#if GT_CFG_ENABLE_ZK_FONT == 1
    if(UTF8_2_GB == tab)
    {
        // utf8_to_unicode
        len = _gt_utf8_to_unicode(src, &font_code);
        // unicode to gb
#if UNICODETOGBK
        font_code = UnicodeToGBK(font_code);
#endif
        gt_size_t tmp_len = _gt_gb_code_len_get(font_code);
        gt_size_t tmp_i = 0;
        for(tmp_i = 0; tmp_i < tmp_len; tmp_i++) {
            dst[tmp_len -1 - tmp_i] = font_code >> (tmp_i * 8);
        }
    }
    else if(GB_2_UTF8 == tab) {
        len = _gt_gb_font_one_char_code_len_get(src, &font_code);
        // gb to unicode
        font_code =  GBKToUnicode(font_code);
        // unicode to utf8
        gt_unicode_to_utf8(dst, font_code);
    }
#endif
    return len;
}


uint8_t gt_utf8_check_char(uint8_t * utf8)
{
    uint32_t len = strlen((char * )utf8);

    if(len >= 1 && utf8[0] < 0x80){
        return 1;
    }
    if(len >=2 && (utf8[0] >> 5) == 0x06){
        return 2;
    }
    if(len >=3 && (utf8[0] >> 4) == 0x0E){
        return 3;
    }
    if(len >=4 && (utf8[0] >> 3) == 0x1E){
        return 4;
    }
    if(len >=5 && (utf8[0] >> 2) == 0x3E){
        return 5;
    }
    if(len >=6 && (utf8[0] >> 1) == 0x7E){
        return 6;
    }

    return 0;
}

uint8_t gt_gb_check_char(const uint8_t *dst, uint16_t pos, uint32_t* font_code)
{
    if(NULL == dst) return 0;
    uint16_t pos_s = 0, len = 0, idx = 0, str_len = 0;

    str_len = strlen((char * )dst);

    while (idx < str_len) {
        len = _gt_gb_font_one_char_code_len_get(&dst[idx], font_code);

        if(idx == pos) {
            return len;
        }
        if(pos >= pos_s && pos < idx+len) {
            return len;
        }
        pos_s = idx;
        idx+=len;
    }

    return 0;
}

/**
 * @brief Determine whether the encoding is for English (style_en) or Chinese/Japanese/Korean (style_cn)
 *
 * @param unicode unicode
 * @return int 0 - style_cn  else-style_en
 */
gt_font_lan_et gt_font_lan_get(uint32_t unicode, uint8_t encoding)
{
    static GT_ATTRIBUTE_RAM_DATA const _unicode_range_st ranges[] = {
        { 0x4E00, 0x9FA5, FONT_LAN_CN },
        { 0x0080, 0x02FF, FONT_LAN_LATIN },
        { 0x1E00, 0x1EFF, FONT_LAN_LATIN },
        { 0x0370, 0x03FF, FONT_LAN_GREEK },
        { 0x0400, 0x04FF, FONT_LAN_CYRILLIC },
        { 0x0590, 0x05FF, FONT_LAN_HEBREW },
        { 0x0600, 0x06FF, FONT_LAN_ARABIC },
        { 0xFB50, 0xFDFF, FONT_LAN_ARABIC },
        { 0xFE70, 0xFEFF, FONT_LAN_ARABIC },
        { 0x0E00, 0x0E7F, FONT_LAN_THAI },
        { 0x0900, 0x097F, FONT_LAN_HINDI },
        { 0x3040, 0x30FF, FONT_LAN_JAPANESE },
        { 0x31F0, 0x31FF, FONT_LAN_JAPANESE },
        { 0x1100, 0x11FF, FONT_LAN_KOREAN },
        { 0xA960, 0xA97F, FONT_LAN_KOREAN },
        { 0xAC00, 0xD7FF, FONT_LAN_KOREAN },
        { 0x3130, 0x318F, FONT_LAN_KOREAN },
        { 0xF900, 0xFAFF, FONT_LAN_CJK_UNIFIED },
        { 0xFE30, 0xFE4F, FONT_LAN_CJK_UNIFIED },
        { 0x3200, 0x33FF, FONT_LAN_CJK_UNIFIED },
        { 0x3190, 0x319F, FONT_LAN_CJK_UNIFIED },
        { 0xFF00, 0xFFEF, FONT_LAN_CJK_UNIFIED }
    };

    // ascii
    if (unicode < 0x80) {
        return FONT_LAN_ASCII;
    }
    if (GT_ENCODING_GB == encoding) {
        return FONT_LAN_CN;
    }
    else if(GT_ENCODING_SJIS == encoding){
        return FONT_LAN_JAPANESE;
    }

    for (size_t i = 0, cnt = sizeof(ranges) / sizeof(ranges[0]); i < cnt; ++i) {
        if (unicode >= ranges[i].start && unicode <= ranges[i].end) {
            return ranges[i].language;
        }
    }

    return FONT_LAN_CN;
}

bool _gt_font_is_convertor_language(gt_font_lan_et style_lang)
{
#if _GT_FONT_ENABLE_CONVERTOR
    // if (FONT_LAN_HEBREW == style_lang) {
    //     return true;
    // }
    if (FONT_LAN_ARABIC == style_lang) {
        return true;
    }
    if (FONT_LAN_THAI == style_lang) {
        return true;
    }
    if (FONT_LAN_HINDI == style_lang) {
        return true;
    }
#endif
    return false;
}

bool _gt_font_is_convertor_by(uint32_t unicode, uint8_t encoding)
{
#if _GT_FONT_ENABLE_CONVERTOR
    gt_font_lan_et style_lang = gt_font_lan_get(unicode, encoding);

    return _gt_font_is_convertor_language(style_lang);
#else
    return false;
#endif
}

_gt_font_dot_ret_st gt_font_get_dot(gt_font_st * font, uint32_t unicode)
{
    uint16_t font_option = 0, font_lan = FONT_LAN_UNKNOWN, temp = 0;
    _gt_font_dot_ret_st ret = {
        .type = GT_FONT_TYPE_FLAG_NONE,
        .size = font->info.size,
    };
    font_lan =  gt_font_lan_get(unicode, font->info.encoding);
#if GT_CFG_ENABLE_ZK_FONT == 1
    font_option = _get_font_option(font, unicode);
    struct gt_font_func_s * font_func = GT_Get_Font_Func_S(font_option);
    if (!font_func) {
        goto default_font;
    }
    if (!GT_Check_Is_Vec(font_option)) {
        // Dot font
        if(!font_func->func.read_dot_font) {
            return ret;
        }
        temp = font_func->func.read_dot_font(unicode, font_option, font->res);

    } else {
        // Vector font
        if(!font_func->func.read_vec_font) {
            return ret;
        }
        temp = font_func->func.read_vec_font(font->res, font_option, unicode, font->info.size, font->info.gray,
                                            IS_CN_FONT_LAN(font_lan) ? font->info.thick_cn : font->info.thick_en);
    }
    if (0 == temp) {
        return ret;
    }
    ret.type = font_func->type;
    return ret;
#endif

default_font:
    if (FONT_LAN_ASCII == font_lan) {
#if GT_CFG_USE_FONT_16_ASCII == 1
        temp = 32;
        gt_memcpy(font->res, &_ascii_16[ (unicode - 0x20) << 5 ], 32);
        ret.type = GT_FONT_TYPE_FLAG_DOT_MATRIX_NON_WIDTH;
        ret.size = 16;
        font->info.size = 16;
#endif
    }
    return ret;
}


uint8_t gt_font_type_get(unsigned int style)
{
#if GT_CFG_ENABLE_ZK_FONT == 1
    return GT_Get_Font_Type(style);
#else
    return 0;
#endif
}

#if _GT_FONT_ENABLE_CONVERTOR
int gt_font_code_transform(font_convertor_st *convert)
{
unsigned int len = 0;
#if GT_CFG_ENABLE_ZK_FONT == 1
    struct gt_font_func_s* font_func;
    font_func = GT_Get_Font_Func_S(convert->font_option);

    if (NULL == font_func) {
        return len;
    }

    if (GT_Check_Is_Vec(convert->font_option)) {
        convert->read_vec_font = font_func->func.read_vec_font;
    } else {
        convert->read_dot_font = font_func->func.read_dot_font;
    }

    len = GT_Font_Code_Transform(convert);
#endif
    return len;
}

int gt_font_convertor_data_get(font_convertor_st *convert, uint32_t pos)
{
#if GT_CFG_ENABLE_ZK_FONT == 1
    return GT_Get_Font_Convertor_Data(convert, pos);
#else
    return 0;
#endif
}
#endif  /** _GT_FONT_ENABLE_CONVERTOR */

#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))

_gt_font_size_res_st gt_font_get_size_length_by_style(gt_font_info_st * info, uint8_t font_style, uint8_t langue, uint32_t text_len)
{
    _gt_font_size_res_st res = {0};
    if (NULL == info) {
        return res;
    }

    uint8_t s_cn = gt_font_type_get(info->style_cn);
    uint8_t s_en = gt_font_type_get(info->style_en);
    uint8_t s_fl = gt_font_type_get(info->style_fl);

    uint8_t font_type = GT_FONT_TYPE_FLAG_DOT_MATRIX_NON_WIDTH, font_gray = info->gray;

    if( GT_FONT_TYPE_FLAG_VEC == s_cn || GT_FONT_TYPE_FLAG_VEC == s_en || GT_FONT_TYPE_FLAG_VEC == s_fl){
        font_type = GT_FONT_TYPE_FLAG_VEC;
        res.dot_width = (((info->size + 15) >> 4) << 4) * font_gray;
    } else {
        font_gray = GT_MAX((s_cn / 10), GT_MAX((s_en / 10), (s_fl / 10)));
        font_gray = font_gray > 0 ? font_gray : 1;
        res.dot_width = (((info->size + 7) >> 3) << 3);
    }

    if (FONT_LAN_HINDI == langue) {
        res.font_per_size = 48;
        res.font_buff_len = res.font_per_size * (text_len + 1);
    } else {
        res.font_per_size = (res.dot_width * res.dot_width * font_gray) >> 3;
        if(GT_FONT_TYPE_FLAG_DOT_MATRIX_NON_WIDTH == font_type){
            res.font_per_size += 2;
        }
        res.font_buff_len = res.font_per_size * 3;
    }

    if (GT_FONT_TYPE_FLAG_VEC == font_type) {
        res.dot_width = (((((info->size * font_gray + 15) >> 4) << 4) / font_gray) >> 3) << 3;
    }
    return res;
}
#else
_gt_font_size_res_st gt_font_get_size_length_by_style(gt_font_info_st * info, uint8_t langue, uint32_t text_len)
{
    _gt_font_size_res_st res = {0};
    if (NULL == info) {
        return res;
    }

    if(FONT_LAN_HINDI == langue){
        res.dot_width = (((info->size + 7) >> 3) << 3);
        res.font_per_size = 48;
        res.font_buff_len = res.font_per_size * (text_len + 1);
        return res;
    }

    uint8_t font_type = 0, font_gray = 0, tmp = 0;
    for(int i = 0; i < FONT_LAN_MAX_COUNT; i++){
        tmp = gt_font_type_get(gt_font_family_get_option(info->family, i, info->cjk));
        if(GT_FONT_TYPE_FLAG_VEC == tmp){
            font_type = GT_FONT_TYPE_FLAG_VEC;
            tmp = info->gray;
        }
        else{
            if(font_type != GT_FONT_TYPE_FLAG_VEC){
                font_type = GT_MAX(font_type, tmp);
            }
            tmp = (tmp / 10) > 0 ? (tmp / 10) : 1;
        }
        font_gray = GT_MAX(font_gray, tmp);
    }

    res.dot_width = (font_type == GT_FONT_TYPE_FLAG_VEC) ? ((((info->size + 15) >> 4) << 4) * info->gray) : ((((info->size + 7) >> 3) << 3));

    res.font_per_size = (res.dot_width * res.dot_width * font_gray >> 3);
    if(GT_FONT_TYPE_FLAG_DOT_MATRIX_NON_WIDTH == font_type){
        res.font_per_size += 2;
    }
    res.font_buff_len = res.font_per_size * 3;

    if(GT_FONT_TYPE_FLAG_VEC == font_type){
        res.dot_width = (((((info->size * info->gray + 15) >> 4) << 4) / info->gray) >> 3) << 3;
    }
    return res;
}
#endif

uint16_t gt_font_get_longest_line_substring_width(gt_font_info_st * info, const char * const text, uint16_t space)
{
    uint32_t idx = 0, len = strlen(text), unicode = 0;
    uint16_t max_width = 0, cur_line_width = 0;
    gt_font_st temp_font = {
        .info = *info,
        .utf8 = (char * )text,
        .len = len,
        .res = NULL,
    };
    uint8_t count_byte = 0, word_width = 0;

    if (NULL == text) {
        return max_width;
    }

#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(info, 2, 0, 0);
#else
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(&temp_font.info, FONT_LAN_UNKNOWN, 0);
#endif
    temp_font.res = gt_mem_malloc(font_size_res.font_buff_len);
    GT_CHECK_BACK_VAL(temp_font.res, max_width);

    while (idx < len) {
        count_byte = gt_font_one_char_code_len_get((uint8_t*)&text[idx], &unicode, info->encoding);
        idx += count_byte;

        if (0x0A == unicode || 0x00 == unicode) {
            if (cur_line_width > max_width) {
                max_width = cur_line_width;
            }
            cur_line_width = 0;
            continue;
        }
        word_width = gt_font_get_one_word_width(unicode, &temp_font);
        cur_line_width += word_width + space;
    }

    if (cur_line_width > max_width) {
        max_width = cur_line_width;
    }

    gt_mem_free(temp_font.res);
    temp_font.res = NULL;
    return max_width;
}

uint8_t gt_font_get_one_word_width(uint32_t uni_or_gbk, gt_font_st * font)
{
    if (0x20 == uni_or_gbk) {
        return font->info.size >> 1;
    }

    if (gt_symbol_is_valid_range(uni_or_gbk)) {
        return font->info.size;
    }

#if GT_CFG_ENABLE_ZK_FONT == 1
    gt_size_t width = 0;
    uint16_t option = _get_font_option(font, uni_or_gbk);

    if (GT_Check_Is_Vec(option)) {
        uint32_t font_dot_w = (((font->info.size + 15) >> 4) << 4) * font->info.gray;
        font_dot_w = (font_dot_w*font_dot_w) >> 3;
        gt_memset(font->res, 0, font_dot_w);
    }

    width = gt_font_get_font_width_inline(uni_or_gbk, option, font->info.size, font->info.gray, font->res);
    if (0 == width) {
        if(_gt_font_latin_check(uni_or_gbk)) {
            return _gt_font_latin_get_width(uni_or_gbk, font->info.size);
        }
        return font->info.size;
    }
    return width;
#else
    if (_gt_font_latin_check(uni_or_gbk)) {
        return _gt_font_latin_get_width(uni_or_gbk, font->size);
    }
    return font->size;
#endif

}

/**
 * @brief 得到灰度文字的宽度
 *
 * @param data 文字数据
 * @param dot_w 文字宽度    例如：48x48大小的文字 dot_w = 48
 * @param dot_h 文字高度 	例如：48x48大小的文字 dot_h = 48
 * @param gray 取值: 1 / 2 / 4
 * @return int 实际文字宽度
 */
uint16_t gt_font_get_word_width_figure_up(const uint8_t* data, uint16_t dot_w, uint16_t dot_h, uint8_t gray)
{
	uint16_t byte_w, i;
	int j, len;
	uint8_t tmp = 0;

    byte_w = ((dot_w + 7) >> 3) * gray;

	for(j = byte_w-1;j >= 0; j--) {
		tmp = 0;
		for(i = 0; i < dot_h; i++) {
			tmp |= data[i*byte_w + j];
		}
		if(0 != tmp) {
			for(i = 0; i < 8; i++){
				if((tmp >> i)&0x01){
					break;
				}
			}

            len = ((j / gray) * 8) + ((j%gray)*8+(8-i))/gray;
            if (len >= dot_h) {
                continue;
            }

			return len;
		}
	}

	return dot_w;
}

void gt_project_encoding_set(gt_encoding_et charset)
{
    _gt_project_encoding = charset;
}

gt_encoding_et gt_project_encoding_get(void)
{
    return _gt_project_encoding;
}

static GT_ATTRIBUTE_RAM_TEXT uint32_t _gt_font_language_split(const uint8_t *str, uint32_t len, uint8_t encoding, uint8_t *lan)
{
    uint32_t idx = 0 ,tmp, uni_or_gbk;
    idx += gt_font_one_char_code_len_get((uint8_t * )&str[0], &uni_or_gbk, encoding);
    if(idx == 0){
        return 0;
    }
    *lan = gt_font_lan_get(uni_or_gbk, encoding);

    while (idx < len) {
        /* code */
        tmp = gt_font_one_char_code_len_get((uint8_t * )&str[idx], &uni_or_gbk, encoding);
        if(*lan != gt_font_lan_get(uni_or_gbk, encoding)){
            break;
        }
        idx += tmp;
    }

    return idx;
}

static GT_ATTRIBUTE_RAM_TEXT bool _is_punctuation(uint32_t uni_or_gbk) {
    if (uni_or_gbk > 0x1F && uni_or_gbk < 0x41) { return true; }
    // if (uni_or_gbk > 0x39 && uni_or_gbk < 0x41) { return true; }
    if (uni_or_gbk > 0x5A && uni_or_gbk < 0x61) { return true; }
    if (uni_or_gbk > 0x7A && uni_or_gbk < 0x7F) { return true; }
    return false;
}

static GT_ATTRIBUTE_RAM_TEXT uint32_t _gt_font_punctuation_split(const uint8_t *str, uint32_t len, uint8_t encoding)
{
    uint32_t idx = 0, uni_or_gbk = 0;
    uint8_t tmp;
    idx += gt_font_one_char_code_len_get((uint8_t * )&str[idx], &uni_or_gbk, encoding);
    if (_is_punctuation(uni_or_gbk)) {
        return idx;
    }
    while (idx < len) {
        tmp = gt_font_one_char_code_len_get((uint8_t * )&str[idx], &uni_or_gbk, encoding);
        if(_is_punctuation(uni_or_gbk) || '\n' == uni_or_gbk){
            break;
        }
        idx += tmp;
    }

    return idx;
}

static GT_ATTRIBUTE_RAM_TEXT uint32_t _gt_font_cn_in_this_range(const gt_font_st *fonts, uint32_t width, uint32_t space, uint32_t* ret_w)
{
    uint32_t idx = 0, uni_or_gbk = 0, w = 0;
    uint8_t *str = (uint8_t * )fonts->utf8, tmp, tmp_w = 0;

    while (idx < fonts->len) {
        tmp = gt_font_one_char_code_len_get((uint8_t * )&str[idx], &uni_or_gbk, fonts->info.encoding);
        tmp_w = gt_font_get_one_word_width(uni_or_gbk, (gt_font_st*)fonts);

        if (w + tmp_w > width) {
            break;
        }

        w += tmp_w + space;
        idx += tmp;
    }
    *ret_w = w;
    return idx;
}

static GT_ATTRIBUTE_RAM_TEXT uint32_t _gt_font_en_in_this_range(const gt_font_st *fonts, uint32_t width, uint32_t space, uint32_t* ret_w, uint16_t *ol_idx, uint16_t *ol_width)
{
    uint32_t idx = 0, uni_or_gbk = 0, w = 0;
    uint8_t *str = (uint8_t * )fonts->utf8, tmp, tmp_w = 0;
    if (ol_width) { *ol_width = 0; }
    if (ol_idx) { *ol_idx = 0; }

    while (idx < fonts->len) {
        tmp = gt_font_one_char_code_len_get((uint8_t * )&str[idx], &uni_or_gbk, fonts->info.encoding);
        tmp_w = gt_font_get_one_word_width(uni_or_gbk, (gt_font_st*)fonts);

        if (w < width) {
            if (ol_width) { *ol_width = w; }
            if (ol_idx) { *ol_idx = idx; }
        }

        w += tmp_w + space;
        idx += tmp;
    }
    *ret_w = w;
    return idx;
}

#if _GT_FONT_ENABLE_CONVERTOR
static GT_ATTRIBUTE_RAM_TEXT uint32_t _get_convertor_string_width(const gt_font_st *fonts, uint8_t lan, uint32_t space, uint16_t range_w, uint16_t *ol_idx, uint16_t *ol_width)
{
    uint32_t idx = 0, uni_or_gbk = 0, tmp = 0, len = fonts->info.size * fonts->len;
    uint16_t *text = NULL;
    uint8_t *font_buf = NULL;
    text = (uint16_t * )gt_mem_malloc(fonts->len * sizeof(uint16_t));
    if (NULL == text) {
        GT_LOGE(GT_LOG_TAG_GUI, "buf malloc failed, size: %lu", fonts->len * sizeof(uint16_t));
        return len;
    }

    if (NULL != ol_width) {
        *ol_width = 0;
    }
    if (NULL != ol_idx) {
        *ol_idx = 0;
    }

    gt_font_st tmp_font = {
        .info = fonts->info,
        .utf8 = fonts->utf8,
        .len = fonts->len,
        .res = NULL,
    };

    tmp = 0;
    while (idx < tmp_font.len) {
        idx += gt_font_one_char_code_len_get((uint8_t * )&tmp_font.utf8[idx], &uni_or_gbk, tmp_font.info.encoding);
        text[tmp++] = (uint16_t)uni_or_gbk;
    }
    uint16_t text_len = tmp, width;
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
    uint16_t font_option = tmp_font.info.style_fl;
    uint8_t ret_style = gt_font_type_get(tmp_font.info.style_fl);
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(&tmp_font.info, ret_style, lan, text_len);
#else
    uint16_t font_option = gt_font_family_get_option(tmp_font.info.family, lan, tmp_font.info.cjk);
    uint8_t ret_style = gt_font_type_get(font_option);
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(&tmp_font.info, lan, text_len);
#endif

    font_convertor_st convertor = {
        .fontcode = text,
        .code_len = text_len,
        .fontsize = tmp_font.info.size,
        .fontgray = tmp_font.info.gray,
        .font_option = font_option,
        .thick = tmp_font.info.thick_en,
        .data = NULL,
        .data_len = font_size_res.font_buff_len,
        .is_rev = 1,
        .is_vec = ret_style == GT_FONT_TYPE_FLAG_VEC ? 1 : 0,
    };
    if(ret_style != GT_FONT_TYPE_FLAG_VEC){
        convertor.fontgray = (ret_style / 10) ? (ret_style / 10) : 1;
    }

    font_buf = (uint8_t * )gt_mem_malloc(font_size_res.font_buff_len);
    if (NULL == font_buf) {
        GT_LOGE(GT_LOG_TAG_GUI, "buf malloc failed, size: %d", font_size_res.font_buff_len);
        goto _ret_handle;
    }
    gt_memset_0(font_buf, font_size_res.font_buff_len);
    convertor.data = font_buf;

    text_len = gt_font_code_transform(&convertor);

    idx = 0;
    tmp = 0;
    len = 0;
    tmp_font.res = &font_buf[font_size_res.font_buff_len - font_size_res.font_per_size];
    while (idx < text_len) {
        if (FONT_LAN_HINDI == lan) {
            width = gt_font_get_word_width_figure_up(&convertor.data[idx * font_size_res.font_per_size], 16, 24, 1);
            tmp = 1;
        }
        else {
            gt_memset_0(&convertor.data[0], font_size_res.font_buff_len);

            if (0x20 == convertor.fontcode[idx]) {
                width = convertor.fontsize >> 1;
            } else {
                tmp = gt_font_convertor_data_get(&convertor, idx);

                width = gt_font_get_word_width_figure_up(&convertor.data[0], font_size_res.dot_width, convertor.fontsize, convertor.fontgray);
            }
        }

        if (len < range_w && lan != FONT_LAN_ARABIC) {
            if (NULL != ol_width) {
                *ol_width = len;
            }
            if (NULL != ol_idx) {
                *ol_idx = idx*3;
            }
        }

        len += width + space;
        idx += tmp;
    }

_ret_handle:
    if (NULL != font_buf) {
        gt_mem_free(font_buf);
        font_buf = NULL;
    }
    if (NULL !=text) {
        gt_mem_free(text);
        text = NULL;
    }
    return len;
}
#endif  /** _GT_FONT_ENABLE_CONVERTOR */

static GT_ATTRIBUTE_RAM_TEXT uint32_t _gt_font_split(const uint8_t *str, uint32_t len, uint8_t encoding, gt_font_lan_et lan)
{
    uint32_t idx = 0, tmp_idx = 0;
    uint32_t unicode;
    while(idx < len)
    {
        tmp_idx = gt_font_one_char_code_len_get((uint8_t * )&str[idx], &unicode, encoding);
        if (0 == tmp_idx) {
            break;
        }

        if(lan != gt_font_lan_get(unicode, encoding)){
            break;
        }

        if(_is_punctuation(unicode) || '\n' == unicode){
            break;
        }

        idx += tmp_idx;
    }
    return idx ? idx : 1;
}

int32_t gt_font_split_line_str(const gt_font_st * fonts, uint32_t max_w, uint32_t space, uint32_t * width,
                                gt_bidi_st** bidi, uint16_t* bidi_len, uint16_t* bidi_max, uint8_t * overlength,
                                bool is_first_line)
{
    uint32_t idx = 0, tmp_idx = 0, tmp_w = 0;
    uint32_t unicode = 0;
    int32_t len = 0;
    gt_font_lan_et lan = FONT_LAN_UNKNOWN;
    uint16_t ol_w = 0, ol_idx = 0, w_offset = *width;

    gt_font_st tmp_font = {
        .info = fonts->info,
        .utf8 = fonts->utf8,
        .len = len,
        .res = fonts->res,
    };

    if(tmp_font.info.style.reg.italic){
        max_w -= tmp_font.info.size >> 1;
    }

    *width = 0;
    max_w -= w_offset;
    while (idx < fonts->len)
    {
        tmp_idx = gt_font_one_char_code_len_get((uint8_t * )&fonts->utf8[idx], &unicode, fonts->info.encoding);

        if('\n' == unicode){
            return len;
        }

        lan = gt_font_lan_get(unicode, fonts->info.encoding);
        if (tmp_idx == 0) {
            break;
        }

        if (IS_CN_FONT_LAN(lan)) {
            tmp_w = gt_font_get_one_word_width(unicode, (gt_font_st*)fonts);
        }
        else{
            tmp_idx = _gt_font_split((const uint8_t *)&fonts->utf8[idx], fonts->len - idx, fonts->info.encoding, lan);
            tmp_font.utf8 = &fonts->utf8[idx];
            tmp_font.len = tmp_idx;

#if _GT_FONT_ENABLE_CONVERTOR
            if(_gt_font_is_convertor_language(lan)){
                tmp_w = _get_convertor_string_width(&tmp_font, lan, space, max_w - (*width), &ol_idx, &ol_w);
            }
            else {
                _gt_font_en_in_this_range(&tmp_font, max_w - (*width), space, &tmp_w, &ol_idx, &ol_w);
            }
#else
            _gt_font_en_in_this_range(&tmp_font, max_w - (*width), space, &tmp_w, &ol_idx, &ol_w);
#endif
        }

        if((*width) + tmp_w + space > max_w){

            if(0 == idx){

                if(w_offset != 0 && !is_first_line){
                    break;
                }
                if(FONT_LAN_ARABIC == lan){
                    len = tmp_idx;
                    *width = tmp_w;
                    if(overlength) { *overlength = 2; }
                }
                else{
                    len = ol_idx;
                    *width = ol_w;
                }
#if _GT_FONT_ENABLE_CONVERTOR
                if(_gt_font_is_convertor_language(lan)){
                    if(bidi && *bidi){
                        if(*bidi_len >= *bidi_max){
                            *bidi_max = (*bidi_max) + 5;
                            *bidi = (gt_bidi_st*)gt_mem_realloc(*bidi, sizeof(gt_bidi_st) * (*bidi_max));
                            if(NULL == *bidi){
                                GT_LOGE(GT_LOG_TAG_GUI,"bild raalloc failed! size: %d\n" , ((*bidi_max) * sizeof(gt_bidi_st)));
                                return -1;
                            }
                        }
                        if(FONT_LAN_ARABIC == lan){
                            (*bidi)[*bidi_len].idx = idx;
                            (*bidi)[*bidi_len].len = tmp_idx;
                            (*bidi)[*bidi_len].flag = lan;
                            (*bidi_len)++;
                        }
                        else{
                            (*bidi)[*bidi_len].idx = idx;
                            (*bidi)[*bidi_len].len = tmp_idx;
                            (*bidi)[*bidi_len].flag = lan;
                            (*bidi_len)++;
                        }
                    }
                }
#endif
            }
            return len;
        }

#if _GT_FONT_ENABLE_CONVERTOR
        if(_gt_font_is_convertor_language(lan)){
            if(bidi && *bidi){
                if(*bidi_len >= *bidi_max){
                    *bidi_max = (*bidi_max) + 5;
                    *bidi = (gt_bidi_st*)gt_mem_realloc(*bidi, sizeof(gt_bidi_st) * (*bidi_max));
                    if(NULL == *bidi){
                        GT_LOGE(GT_LOG_TAG_GUI,"bild raalloc failed! size: %d\n" , ((*bidi_max) * sizeof(gt_bidi_st)));
                        return -1;
                    }
                }
                (*bidi)[*bidi_len].idx = idx;
                (*bidi)[*bidi_len].len = tmp_idx;
                (*bidi)[*bidi_len].flag = lan;
                (*bidi_len)++;
            }
        }
#endif

        idx += tmp_idx;
        *width += tmp_w + space;
        len += tmp_idx;
    }

    if(overlength) { *overlength = 2; }

    return len ;
}

uint32_t gt_font_split(gt_font_st * fonts, uint32_t width, uint32_t dot_w, uint32_t space, uint32_t *ret_w, uint8_t * lan, uint32_t* lan_len)
{
    uint32_t idx = 0, len = 0;
    uint32_t tmp_len = 0, tmp_w = 0;

    *ret_w = 0;
    *lan_len = 0;
    len = _gt_font_language_split((const uint8_t*)fonts->utf8, fonts->len, fonts->info.encoding, lan);

    if (0 == len) {
        return 0;
    }

    gt_font_st tmp_font = {
        .info = fonts->info,
        .utf8 = fonts->utf8,
        .len = len,
        .res = fonts->res,
    };
    char *text = fonts->utf8;
    uint16_t ol_w = 0, ol_idx = 0;

    if (IS_CN_FONT_LAN(*lan)) {
        len = _gt_font_cn_in_this_range(&tmp_font, width, space, ret_w);
        goto _ret_dat;
    }

    if(tmp_font.info.style.reg.italic){
        dot_w -= tmp_font.info.size >> 1;
    }

    idx = 0;
    *ret_w = 0;
    *lan_len = len;
    while (idx < len) {
        tmp_len = _gt_font_punctuation_split((const uint8_t*)&text[idx], len - idx, fonts->info.encoding);
        tmp_font.utf8 = &text[idx];
        tmp_font.len = tmp_len;

#if _GT_FONT_ENABLE_CONVERTOR
        if(_gt_font_is_convertor_language((gt_font_lan_et)*lan)){
            tmp_w = _get_convertor_string_width(&tmp_font, *lan, space, width - *ret_w, &ol_idx, &ol_w);
        }
        else {
            _gt_font_en_in_this_range(&tmp_font, (width - (*ret_w)), space, &tmp_w, &ol_idx, &ol_w);
        }
#else
        _gt_font_en_in_this_range(&tmp_font, (width - (*ret_w)), space, &tmp_w, &ol_idx, &ol_w);
#endif

        if (tmp_w > width && 0 == ol_idx && 0 == idx) {
#if _GT_FONT_ENABLE_CONVERTOR
            if (_gt_font_is_convertor_language((gt_font_lan_et)*lan)) {
                len = ol_idx;
                *ret_w = ol_w;
            } else {
                *ret_w = tmp_w;
                len = tmp_len;
            }
#else
            *ret_w = tmp_w;
            len = tmp_len;
#endif
            goto _ret_dat;
        }
        else if ((*ret_w + tmp_w >= width) || (0x0A == tmp_font.utf8[0])) {
            if (0 == idx && width == dot_w) {
                len = ol_idx;
                *ret_w = ol_w;
            }
            else if(0 == idx && ol_idx != 0){
                len = ol_idx;
            }
            else {
                len = idx;
            }
            break;
        }
        *ret_w += tmp_w;
        idx += tmp_len;
    }

_ret_dat:
    return len;
}

uint32_t gt_font_split_line_numb(gt_font_info_st* info, gt_font_split_line_st * sp_line, uint32_t * ret_max_w)
{
    uint32_t line_numb = 1;
    const char * text = (const char * )sp_line->text;
    if(!text) return line_numb;

    gt_font_st temp_font = {
        .info = *info,
        .res = NULL,
    };

#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(info, 2, 0, 0);
#else
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(&temp_font.info, FONT_LAN_UNKNOWN, 0);
#endif
    temp_font.res = gt_mem_malloc(font_size_res.font_buff_len);
    GT_CHECK_BACK_VAL(temp_font.res, line_numb);

    uint8_t lan = 0;
    int32_t disp_w = sp_line->start_w - sp_line->indent;
    uint32_t ret_w = 0, lan_len = 0;
    uint32_t idx = 0, len = sp_line->len;
    int32_t idx_step = 0;

    if (0 == len) {
        return line_numb;
    }

    line_numb = 0;
    while (idx < len) {
        if (0x0A == text[idx]) {
            ++idx;
            goto _compute_line;
        }
        temp_font.utf8 = (char*)&text[idx];
        temp_font.len = len - idx;

        if(line_numb != 0){
            disp_w = sp_line->max_w - (0x0A == text[idx - 1] ? sp_line->indent : 0);
        }

        ret_w = sp_line->max_w - disp_w;
        idx_step = gt_font_split_line_str(&temp_font, sp_line->max_w, sp_line->space, &ret_w,
                                        NULL, NULL, NULL, NULL,
                                        line_numb ? 0 : 1);

        if(idx == 0 & idx_step == 0){
            goto _compute_line;
        }
        idx += idx_step;
        disp_w -= ret_w;

    _compute_line:
        ++line_numb;
    }

    if(ret_max_w) *ret_max_w = (line_numb > 1) ? sp_line->max_w : (sp_line->max_w - disp_w);

    gt_mem_free(temp_font.res);
    temp_font.res = NULL;
    return line_numb;
}



static GT_ATTRIBUTE_RAM_TEXT uint32_t _gt_font_lang_and_punctuation_split(const uint8_t *str, uint32_t len, uint8_t encoding, uint8_t *lan, uint8_t is_lan)
{
    uint32_t idx = 0 ,tmp, uni_or_gbk;
    idx += gt_font_one_char_code_len_get((uint8_t * )&str[0], &uni_or_gbk, encoding);
    if (idx == 0) {
        return 0;
    }
    *lan = gt_font_lan_get(uni_or_gbk, encoding);
    if (_is_punctuation(uni_or_gbk)) {
        *lan = is_lan;
    }
    uint8_t tmp_lan;
    while (idx < len) {
        /* code */
        tmp = gt_font_one_char_code_len_get((uint8_t * )&str[idx], &uni_or_gbk, encoding);
        tmp_lan = gt_font_lan_get(uni_or_gbk, encoding);
        if ((*lan != is_lan) && _is_punctuation(uni_or_gbk)) {
            break;
        }
        else if ((!_is_punctuation(uni_or_gbk)) && tmp_lan != *lan) {
            break;
        }
        idx += tmp;
    }
    return idx;
}
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
uint8_t right_to_left_lan_get(uint16_t style)
{
#if _GT_FONT_ENABLE_CONVERTOR
    switch (style) {
        case 71:
        case 72:
        case 73:
        case 141:
        case 169:
        case 170:
        case 194:
        case 195:
        case 196:
        case 197:
        case 198:
        case 199:
            return FONT_LAN_HEBREW;
        case 74:
        case 75:
        case 76:
        case 143:
        case 177:
        case 178:
        case 206:
        case 207:
        case 208:
        case 209:
        case 210:
        case 211:
            return FONT_LAN_ARABIC;
        default:
            break;
    }
#endif

    return 0xFF;
}
#else
uint8_t right_to_left_lan_get(gt_font_st* font)
{
    gt_font_lan_et font_lan = FONT_LAN_UNKNOWN;
    uint8_t* str = (uint8_t*)font->utf8;
    uint16_t idx = 0;
    uint32_t uni_or_gbk = 0;

    while(idx < font->len){
        idx += gt_font_one_char_code_len_get((uint8_t * )&str[idx], &uni_or_gbk, font->info.encoding);
        font_lan = gt_font_lan_get(uni_or_gbk, font->info.encoding);
        if(FONT_LAN_HEBREW == font_lan || FONT_LAN_ARABIC == font_lan){
            return font_lan;
        }
    }
    return 0xFF;
}

#endif

#if _GT_FONT_ENABLE_CONVERTOR
bool gt_right_to_left_handler(const gt_font_st* fonts, uint8_t* ret_text, uint8_t r2l_lan)
{
    uint32_t tmp = 0, uni_or_gbk = 0, len = 0, alb_count = 0;
    int bidi_s = -1, bidi_e = -1, idx = 0;
    uint8_t lan;
    bool ret_flag = false;

    while (idx < fonts->len) {
        if (r2l_lan == FONT_LAN_ARABIC) {
            len = _gt_font_language_split((const uint8_t * )&fonts->utf8[idx], fonts->len-idx, fonts->info.encoding, &lan);
        } else {
            len = _gt_font_lang_and_punctuation_split((const uint8_t * )&fonts->utf8[idx], fonts->len-idx, fonts->info.encoding, &lan, r2l_lan);
        }
        if (r2l_lan == lan) {
            ret_flag = true;
            if (-1 == bidi_s) {
                bidi_s = tmp;
            }
            bidi_e = tmp;
        }
        idx+= len;
        ++tmp;
    }
    if (!ret_flag) {
        return ret_flag;
    }

    uint16_t bidi_len = tmp;
    gt_bidi_st* bidi = (gt_bidi_st*)gt_mem_malloc((bidi_len * sizeof(gt_bidi_st)));
    if (NULL == bidi) {
        GT_LOGE(GT_LOG_TAG_GUI, "bidi malloc err! size = %lu", (bidi_len * sizeof(gt_bidi_st)) );
        return false;
    }
    tmp = 0;
    idx = 0;
    while (idx < fonts->len) {
        if (r2l_lan == FONT_LAN_ARABIC) {
            len = _gt_font_language_split((const uint8_t * )&fonts->utf8[idx], fonts->len-idx, fonts->info.encoding, &lan);
        } else {
            len = _gt_font_lang_and_punctuation_split((const uint8_t * )&fonts->utf8[idx], fonts->len-idx, fonts->info.encoding, &lan, r2l_lan);
        }

        bidi[tmp].idx = idx;
        bidi[tmp].len = len;
        bidi[tmp].flag = (r2l_lan == lan) ? true : false;
        alb_count += (r2l_lan == lan || (1 == len && _is_punctuation(fonts->utf8[idx]))) ? 1 : 0;
        idx+= len;
        tmp++;
    }
    gt_bidi_st tmp_bidi;

    while(bidi_s < bidi_e) {
        if (r2l_lan == FONT_LAN_ARABIC && bidi_len != alb_count) {
            break;
        }
        tmp_bidi = bidi[bidi_s];
        bidi[bidi_s] = bidi[bidi_e];
        bidi[bidi_e] = tmp_bidi;
        ++bidi_s;
        --bidi_e;
    }

    idx = 0;
    for(tmp = 0; tmp < bidi_len; ++tmp) {
        if (false == bidi[tmp].flag || r2l_lan == FONT_LAN_ARABIC) {
            gt_memcpy(&ret_text[idx], &fonts->utf8[bidi[tmp].idx] ,bidi[tmp].len);
            idx += bidi[tmp].len;
            continue;
        }
        bidi_s = bidi[tmp].idx;
        bidi_e = 0;
        while(bidi_e < bidi[tmp].len) {
            len = gt_font_one_char_code_len_get((uint8_t * )&fonts->utf8[bidi_s], &uni_or_gbk, fonts->info.encoding);
            gt_memcpy(&ret_text[idx + (bidi[tmp].len - bidi_e) - len], &fonts->utf8[bidi_s] ,len);
            bidi_e += len;
            bidi_s += len;
        }
        idx += bidi[tmp].len;
    }

    gt_mem_free(bidi);
    bidi = NULL;
    return ret_flag;
}
#endif

#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
bool gt_font_is_illegal_char(uint32_t uni_or_gbk)
{
#if _GT_FONT_APOSTROPHE_PUNCTUATION_MARK_LEGAL
    if ('\'' == uni_or_gbk) { return false; }
#endif
    if (uni_or_gbk < '0') { return true; }
#if _GT_FONT_NUMBER_LEGAL
    if (uni_or_gbk < ':') { return false; }
#endif
    if (uni_or_gbk < 'A') { return true; }
    if (uni_or_gbk < '[') { return false; }
    if (uni_or_gbk < 'a') { return true; }
    if (uni_or_gbk < '{') { return false; }
    if (uni_or_gbk < 0x80) { return true; }
    return false;
}

uint16_t gt_font_get_word_byte_length(char const * const text, uint16_t length, uint8_t encoding)
{
    if (NULL == text) {
        return 0;
    }
    char const * ptr = (char const * )text;
    char const * end_p = (char const * )text + length;
    uint32_t uni_or_gbk = 0;
    uint8_t len = 0;

    gt_font_lan_et remark_sty = FONT_LAN_UNKNOWN;
    gt_font_lan_et cur_sty = FONT_LAN_UNKNOWN;

    while (ptr < end_p) {
        len = gt_font_one_char_code_len_get((uint8_t * )ptr, &uni_or_gbk, encoding);
        if (0 == len) {
            ++ptr;
            continue;
        }
        cur_sty = gt_font_lan_get(uni_or_gbk, encoding);
        if (ptr == text) {
            remark_sty = cur_sty;
        }
        if (remark_sty != cur_sty) {
            break;
        } else if (gt_font_is_illegal_char(uni_or_gbk)) {
            break;
        }
        ptr += len;
    }

    return ptr - text;
}
#endif

/* py input method API interface -------------------------------------*/
gt_py_input_method_st* gt_py_input_method_create(void)
{
#if ((defined(PINYIN_INPUT_METHOD_EN)) && (GT_CFG_ENABLE_ZK_SPELL == 1)&&(GT_CFG_ENABLE_ZK_FONT == 1))
    if(!_gt_py_info){
        _gt_py_info = (py_info_st*) gt_mem_malloc(sizeof(py_info_st));
        if(!_gt_py_info){
            GT_LOGE(GT_LOG_TAG_GUI, "py info malloc err!!! size = %lu", sizeof(py_info_st));
            goto _info_ret;
        }
        gt_memset(_gt_py_info, 0, sizeof(py_info_st));

        _gt_py_info->content = (py_content_st*)gt_mem_malloc(sizeof(py_content_st));
        if(!_gt_py_info->content){
            GT_LOGE(GT_LOG_TAG_GUI, "py info content malloc err!!! size = %lu", sizeof(py_content_st));
            goto _info_ret;
        }
        gt_memset(_gt_py_info->content, 0, sizeof(py_content_st));

        _gt_py_info->py_cache = (unsigned char*)gt_mem_malloc(1218);
        if(!_gt_py_info->py_cache)
        {
            GT_LOGE(GT_LOG_TAG_GUI, "py info py_cache malloc err!!! size = %lu", 1218);
            goto _info_ret;
        }
        gt_memset(_gt_py_info->py_cache, 0, 1218);

        //
        if(0 == gt_pinyin_init(_gt_py_info)){
            GT_LOGE(GT_LOG_TAG_GUI, "py init err!!!");
            goto _info_ret;
        }
        _gt_py_info = _gt_py_info;
    }

    gt_py_input_method_st* py_input_method = (gt_py_input_method_st*) gt_mem_malloc(sizeof(gt_py_input_method_st));
    if(!py_input_method){
        GT_LOGE(GT_LOG_TAG_GUI, "py input method malloc err!!! size = %lu", sizeof(gt_py_input_method_st));
        goto input_met_ret;
    }
    gt_memset(py_input_method, 0, sizeof(gt_py_input_method_st));

    py_input_method->py_info = _gt_py_info;

    return py_input_method;

//
_info_ret:
    if(!_gt_py_info->py_cache){
        gt_mem_free(_gt_py_info->py_cache);
        _gt_py_info->py_cache = NULL;
    }

    if(!_gt_py_info->content){
        gt_mem_free(_gt_py_info->content);
        _gt_py_info->content = NULL;
    }

    if(!_gt_py_info){
        gt_mem_free(_gt_py_info);
        _gt_py_info = NULL;
    }
    return NULL;

//
input_met_ret:
    if(!py_input_method){
        gt_mem_free(py_input_method);
        py_input_method = NULL;
    }

#endif
    return NULL;
}

void gt_py_input_method_push_ascii(gt_py_input_method_st* py_input_method, uint8_t ch)
{
#if ((defined(PINYIN_INPUT_METHOD_EN)) && (GT_CFG_ENABLE_ZK_SPELL == 1)&&(GT_CFG_ENABLE_ZK_FONT == 1))
    if(!py_input_method || py_input_method->ascii_numb >= 6){
        return;
    }

    py_input_method->ascii[py_input_method->ascii_numb++] = ch;
    py_input_method->ascii[py_input_method->ascii_numb] = '\0';
#endif
}

void gt_py_input_method_pop_ascii(gt_py_input_method_st* py_input_method)
{
#if ((defined(PINYIN_INPUT_METHOD_EN)) && (GT_CFG_ENABLE_ZK_SPELL == 1)&&(GT_CFG_ENABLE_ZK_FONT == 1))
    if(py_input_method->ascii_numb <= 0){
        return;
    }
    --py_input_method->ascii_numb;
    py_input_method->ascii[py_input_method->ascii_numb] = '\0';
#endif
}

uint8_t gt_py_input_method_get_ascii_numb(gt_py_input_method_st* py_input_method)
{
    return py_input_method->ascii_numb;
}

int gt_py_input_method_get_chinese(gt_py_input_method_st* py_input_method)
{
#if ((defined(PINYIN_INPUT_METHOD_EN)) && (GT_CFG_ENABLE_ZK_SPELL == 1)&&(GT_CFG_ENABLE_ZK_FONT == 1))
    int ret = gt_pinyin_full_keyboard_get((char * )py_input_method->ascii);
    py_input_method->chinese_numb = ret ? py_input_method->py_info->content->chinese_numb : 0;

    return ret;
#else
    return 0;
#endif
}

void gt_py_input_method_chinese_data_handler(gt_py_input_method_st* py_input_method)
{
#if ((defined(PINYIN_INPUT_METHOD_EN)) && (GT_CFG_ENABLE_ZK_SPELL == 1)&&(GT_CFG_ENABLE_ZK_FONT == 1))
    uint32_t unicode = 0;
    uint8_t len = 0;
    int i = 0;
    gt_memset_0(py_input_method->chinese, GT_PY_MAX_NUMB * GT_PY_MAX_NUMB);
    for(i = 0; i < py_input_method->chinese_numb; i++){
        unicode = GBKToUnicode((((uint32_t)py_input_method->py_info->content->chinese[2*i]) << 8) | py_input_method->py_info->content->chinese[2*i+1]);
        len = gt_unicode_to_utf8(py_input_method->chinese[i], unicode);
        py_input_method->chinese[i][len]='\0';
    }
#endif
}

void gt_py_input_method_last_page(gt_py_input_method_st* py_input_method)
{
#if ((defined(PINYIN_INPUT_METHOD_EN)) && (GT_CFG_ENABLE_ZK_SPELL == 1)&&(GT_CFG_ENABLE_ZK_FONT == 1))
    if(!gt_pinyin_last_page(py_input_method->py_info)){
        return;
    }
    py_input_method->chinese_numb = py_input_method->py_info->content->chinese_numb;
    gt_py_input_method_chinese_data_handler(py_input_method);
#endif
}

void gt_py_input_method_next_page(gt_py_input_method_st* py_input_method)
{
#if ((defined(PINYIN_INPUT_METHOD_EN)) && (GT_CFG_ENABLE_ZK_SPELL == 1)&&(GT_CFG_ENABLE_ZK_FONT == 1))
    if(!gt_pinyin_next_page(py_input_method->py_info)){
        return;
    }
    py_input_method->chinese_numb = py_input_method->py_info->content->chinese_numb;
    gt_py_input_method_chinese_data_handler(py_input_method);
#endif
}

void gt_py_input_method_select_text(gt_py_input_method_st* py_input_method, uint8_t index)
{
#if ((defined(PINYIN_INPUT_METHOD_EN)) && (GT_CFG_ENABLE_ZK_SPELL == 1)&&(GT_CFG_ENABLE_ZK_FONT == 1))
    uint8_t tmp_text[2] = {0};
    gt_pinyin_select_text(py_input_method->py_info, index, tmp_text);
    py_input_method->ascii_numb = py_input_method->py_info->content->ascii_numb;
    py_input_method->chinese_numb = py_input_method->py_info->content->chinese_numb;
#endif

}

void gt_py_input_method_clean(gt_py_input_method_st* py_input_method)
{
#if ((defined(PINYIN_INPUT_METHOD_EN)) && (GT_CFG_ENABLE_ZK_SPELL == 1)&&(GT_CFG_ENABLE_ZK_FONT == 1))
    py_input_method->ascii_numb = 0;
    py_input_method->chinese_numb = 0;
    py_input_method->ascii[py_input_method->ascii_numb] = '\0';
#endif
}

void gt_py_input_method_destroy(gt_py_input_method_st* py_input_method)
{
#if ((defined(PINYIN_INPUT_METHOD_EN)) && (GT_CFG_ENABLE_ZK_SPELL == 1)&&(GT_CFG_ENABLE_ZK_FONT == 1))
    if(py_input_method){
        py_input_method->py_info = NULL;
        gt_mem_free(py_input_method);
        py_input_method = NULL;
    }
#endif
}

#if GT_CFG_USE_FONT_16_ASCII == 1
static GT_ATTRIBUTE_RAM_DATA const uint8_t _ascii_16[] = {
    //
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //!
    0x00,0x00,0x00,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x00,0x00,0x60,0x00,0x60,0x00,0x00,0x00,0x00,0x00,
    //"
    0x00,0x00,0x66,0x00,0x66,0x00,0x66,0x00,0x66,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //#
    0x00,0x00,0x00,0x00,0x1B,0x00,0x1B,0x00,0x1B,0x00,0x7F,0x00,0x7F,0x00,0x36,0x00,0x36,0x00,0x7F,0x00,0x7F,0x00,0x6C,0x00,0x6C,0x00,0x6C,0x00,0x00,0x00,0x00,0x00,
    //$
    0x00,0x00,0x08,0x00,0x1C,0x00,0x3E,0x00,0x6B,0x00,0x68,0x00,0x78,0x00,0x3C,0x00,0x1E,0x00,0x0F,0x00,0x6B,0x00,0x6B,0x00,0x3E,0x00,0x1C,0x00,0x08,0x00,0x00,0x00,
    //%
    0x00,0x00,0x00,0x00,0x78,0x30,0xCC,0x70,0xCC,0x60,0xCC,0xC0,0xCD,0xC0,0x79,0x80,0x03,0x3C,0x03,0x66,0x06,0x66,0x0E,0x66,0x1C,0x66,0x18,0x3C,0x00,0x00,0x00,0x00,
    //&
    0x00,0x00,0x00,0x00,0x1F,0x00,0x3F,0x80,0x31,0x80,0x31,0x80,0x1F,0x00,0x1E,0x00,0x36,0x60,0x67,0x60,0x63,0xE0,0x61,0xC0,0x3F,0xF0,0x1E,0x30,0x00,0x00,0x00,0x00,
    //'
    0x00,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //(
    0x00,0x00,0x18,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x18,0x00,
    //)
    0x00,0x00,0xC0,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0xC0,0x00,
    //*
    0x00,0x00,0x00,0x00,0x18,0x00,0xDB,0x00,0xFF,0x00,0x18,0x00,0x7E,0x00,0x66,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //+
    0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0xFF,0x00,0xFF,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x00,0x60,0x00,0x20,0x00,0x20,0x00,0x40,0x00,
    //-
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x78,0x00,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //.
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x00,0x60,0x00,0x00,0x00,
    ///
    0x00,0x00,0x00,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0x00,0x00,0x00,0x00,
    //0
    0x00,0x00,0x3C,0x00,0x7E,0x00,0xE7,0x00,0xC3,0x00,0xC3,0x00,0xC3,0x00,0xC3,0x00,0xC3,0x00,0xC3,0x00,0xE7,0x00,0x7E,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //1
    0x00,0x00,0x0C,0x00,0x1C,0x00,0x3C,0x00,0x6C,0x00,0x4C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //2
    0x00,0x00,0x3C,0x00,0x7E,0x00,0xE3,0x00,0xC3,0x00,0x03,0x00,0x06,0x00,0x0E,0x00,0x1C,0x00,0x38,0x00,0x60,0x00,0xFF,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //3
    0x00,0x00,0x3E,0x00,0x7F,0x00,0xC3,0x00,0x03,0x00,0x1E,0x00,0x1E,0x00,0x07,0x00,0x03,0x00,0xC3,0x00,0xE7,0x00,0x7E,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //4
    0x00,0x00,0x06,0x00,0x0E,0x00,0x0E,0x00,0x1E,0x00,0x36,0x00,0x36,0x00,0x66,0x00,0xC6,0x00,0xFF,0x00,0xFF,0x00,0x06,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //5
    0x00,0x00,0x7E,0x00,0x7E,0x00,0x60,0x00,0xE0,0x00,0xFC,0x00,0xFE,0x00,0xC7,0x00,0x03,0x00,0xC3,0x00,0xE7,0x00,0x7E,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //6
    0x00,0x00,0x3E,0x00,0x7F,0x00,0x63,0x00,0xC0,0x00,0xDC,0x00,0xFE,0x00,0xE7,0x00,0xC3,0x00,0xC3,0x00,0x63,0x00,0x7E,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //7
    0x00,0x00,0xFF,0x00,0xFF,0x00,0x06,0x00,0x0C,0x00,0x0C,0x00,0x18,0x00,0x18,0x00,0x18,0x00,0x38,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //8
    0x00,0x00,0x3C,0x00,0x7E,0x00,0xC3,0x00,0xC3,0x00,0xC3,0x00,0x7E,0x00,0x7E,0x00,0xC3,0x00,0xC3,0x00,0xC3,0x00,0x7E,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //9
    0x00,0x00,0x3C,0x00,0x7E,0x00,0xC6,0x00,0xC3,0x00,0xC3,0x00,0xE7,0x00,0x7F,0x00,0x3B,0x00,0x03,0x00,0xC6,0x00,0xFE,0x00,0x7C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //:
    0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //;
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x30,0x00,0x10,0x00,0x10,0x00,0x20,0x00,
    //<
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x07,0x00,0x1E,0x00,0x78,0x00,0xE0,0x00,0x78,0x00,0x1E,0x00,0x07,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //=
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //>
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0xE0,0x00,0x78,0x00,0x1E,0x00,0x07,0x00,0x1E,0x00,0x78,0x00,0xE0,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //?
    0x00,0x00,0x00,0x00,0x1E,0x00,0x3F,0x00,0x71,0x80,0x61,0x80,0x03,0x80,0x07,0x00,0x0E,0x00,0x0C,0x00,0x0C,0x00,0x00,0x00,0x0C,0x00,0x0C,0x00,0x00,0x00,0x00,0x00,
    //@
    0x03,0xF0,0x0F,0xFC,0x1C,0x0E,0x39,0xDE,0x37,0xFB,0x76,0x3B,0x6C,0x33,0x6C,0x33,0x6C,0x33,0x6C,0x76,0x6F,0xFC,0x37,0xB8,0x38,0x03,0x1C,0x0E,0x0F,0xFC,0x03,0xF0,
    //A
    0x00,0x00,0x0E,0x00,0x0E,0x00,0x1B,0x00,0x1B,0x00,0x1B,0x00,0x31,0x80,0x31,0x80,0x3F,0x80,0x7F,0xC0,0x60,0xC0,0x60,0xC0,0xC0,0x60,0x00,0x00,0x00,0x00,0x00,0x00,
    //B
    0x00,0x00,0x7F,0x80,0x7F,0xC0,0x60,0xC0,0x60,0xC0,0x60,0xC0,0x7F,0x80,0x7F,0xC0,0x60,0xE0,0x60,0x60,0x60,0x60,0x7F,0xC0,0x7F,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
    //C
    0x00,0x00,0x0F,0x80,0x3F,0xC0,0x30,0xE0,0x60,0x60,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x60,0x30,0xE0,0x3F,0xC0,0x0F,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
    //D
    0x00,0x00,0x7F,0x00,0x7F,0xC0,0x60,0xC0,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0xC0,0x7F,0xC0,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //E
    0x00,0x00,0x7F,0xC0,0x7F,0xC0,0x60,0x00,0x60,0x00,0x60,0x00,0x7F,0xC0,0x7F,0xC0,0x60,0x00,0x60,0x00,0x60,0x00,0x7F,0xC0,0x7F,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,
    //F
    0x00,0x00,0x7F,0x80,0x7F,0x80,0x60,0x00,0x60,0x00,0x60,0x00,0x7F,0x00,0x7F,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //G
    0x00,0x00,0x0F,0x80,0x3F,0xC0,0x30,0xE0,0x60,0x60,0x60,0x00,0x60,0x00,0x63,0xE0,0x63,0xE0,0x60,0x60,0x30,0xE0,0x3F,0xC0,0x0F,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
    //H
    0x00,0x00,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x7F,0xE0,0x7F,0xE0,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x00,0x00,0x00,0x00,0x00,0x00,
    //I
    0x00,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //J
    0x00,0x00,0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x00,0xC3,0x00,0xE7,0x00,0x7E,0x00,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //K
    0x00,0x00,0x60,0x60,0x60,0xC0,0x61,0x80,0x63,0x00,0x66,0x00,0x6F,0x00,0x7B,0x00,0x71,0x80,0x61,0x80,0x60,0xC0,0x60,0xE0,0x60,0x60,0x00,0x00,0x00,0x00,0x00,0x00,
    //L
    0x00,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x7F,0x80,0x7F,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
    //M
    0x00,0x00,0x70,0x70,0x70,0x70,0x78,0xF0,0x78,0xF0,0x68,0xB0,0x6D,0xB0,0x6D,0xB0,0x6D,0xB0,0x67,0x30,0x67,0x30,0x67,0x30,0x62,0x30,0x00,0x00,0x00,0x00,0x00,0x00,
    //N
    0x00,0x00,0x60,0x60,0x70,0x60,0x78,0x60,0x78,0x60,0x6C,0x60,0x66,0x60,0x66,0x60,0x63,0x60,0x61,0xE0,0x61,0xE0,0x60,0xE0,0x60,0x60,0x00,0x00,0x00,0x00,0x00,0x00,
    //O
    0x00,0x00,0x0F,0x00,0x3F,0xC0,0x30,0xC0,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x30,0xC0,0x3F,0xC0,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //P
    0x00,0x00,0x7F,0x00,0x7F,0x80,0x61,0xC0,0x60,0xC0,0x61,0xC0,0x7F,0x80,0x7F,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //Q
    0x00,0x00,0x0F,0x00,0x3F,0xC0,0x30,0xC0,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x66,0x60,0x33,0xC0,0x3F,0xC0,0x0E,0xC0,0x00,0x60,0x00,0x00,0x00,0x00,
    //R
    0x00,0x00,0x7F,0x80,0x7F,0xC0,0x60,0xE0,0x60,0x60,0x60,0xE0,0x7F,0xC0,0x7F,0x00,0x63,0x80,0x61,0xC0,0x60,0xC0,0x60,0xE0,0x60,0x70,0x00,0x00,0x00,0x00,0x00,0x00,
    //S
    0x00,0x00,0x1F,0x00,0x3F,0x80,0x61,0xC0,0x60,0xC0,0x78,0x00,0x3F,0x00,0x0F,0x80,0x01,0xC0,0x60,0xC0,0x71,0xC0,0x3F,0x80,0x1F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //T
    0x00,0x00,0xFF,0xC0,0xFF,0xC0,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //U
    0x00,0x00,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x70,0xE0,0x3F,0xC0,0x1F,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
    //V
    0x00,0x00,0xC0,0x60,0xC0,0x60,0x60,0xC0,0x60,0xC0,0x31,0x80,0x31,0x80,0x31,0x80,0x1B,0x00,0x1B,0x00,0x0E,0x00,0x0E,0x00,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //W
    0x00,0x00,0xC3,0x86,0xE3,0x86,0x63,0x8C,0x66,0xCC,0x66,0xCC,0x36,0xD8,0x36,0xD8,0x36,0xD8,0x1C,0x78,0x1C,0x70,0x1C,0x70,0x1C,0x70,0x00,0x00,0x00,0x00,0x00,0x00,
    //X
    0x00,0x00,0x60,0xC0,0x71,0xC0,0x31,0x80,0x1B,0x00,0x1F,0x00,0x0E,0x00,0x0E,0x00,0x1F,0x00,0x1B,0x00,0x31,0x80,0x71,0xC0,0x60,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,
    //Y
    0x00,0x00,0xC0,0xC0,0xE1,0xC0,0x61,0x80,0x33,0x00,0x33,0x00,0x1E,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //Z
    0x00,0x00,0x7F,0x80,0x7F,0x80,0x03,0x00,0x06,0x00,0x0E,0x00,0x0C,0x00,0x18,0x00,0x38,0x00,0x30,0x00,0x60,0x00,0xFF,0x80,0xFF,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
    //[
    0x00,0x00,0x78,0x00,0x78,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x78,0x00,0x78,0x00,
    //'\'
    0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x00,0x00,
    //]
    0x00,0x00,0xF0,0x00,0xF0,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0xF0,0x00,0xF0,0x00,
    //^
    0x00,0x00,0x18,0x00,0x3C,0x00,0x3C,0x00,0x66,0x00,0x66,0x00,0xC3,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //_
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x80,0xFF,0x80,
    //`
    0x00,0x00,0xC0,0x00,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //a
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3E,0x00,0x7F,0x00,0x63,0x00,0x0F,0x00,0x3F,0x00,0x73,0x00,0x63,0x00,0x7F,0x00,0x3D,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
    //b
    0x00,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x6E,0x00,0x7F,0x00,0x73,0x80,0x61,0x80,0x61,0x80,0x61,0x80,0x73,0x80,0x7F,0x00,0x6E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //c
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1E,0x00,0x3F,0x00,0x73,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x73,0x00,0x3F,0x00,0x1E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //d
    0x00,0x00,0x01,0x80,0x01,0x80,0x01,0x80,0x1D,0x80,0x3F,0x80,0x73,0x80,0x61,0x80,0x61,0x80,0x61,0x80,0x73,0x80,0x3F,0x80,0x1D,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
    //e
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1C,0x00,0x3E,0x00,0x63,0x00,0x7F,0x00,0x7F,0x00,0x60,0x00,0x73,0x00,0x3E,0x00,0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //f
    0x00,0x00,0x38,0x00,0x78,0x00,0x60,0x00,0xF8,0x00,0xF8,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //g
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1D,0x80,0x3F,0x80,0x73,0x80,0x61,0x80,0x61,0x80,0x61,0x80,0x73,0x80,0x3F,0x80,0x1D,0x80,0x61,0x80,0x7F,0x80,0x3F,0x00,
    //h
    0x00,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x6F,0x00,0x7F,0x80,0x71,0x80,0x61,0x80,0x61,0x80,0x61,0x80,0x61,0x80,0x61,0x80,0x61,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
    //i
    0x00,0x00,0x60,0x00,0x60,0x00,0x00,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //j
    0x00,0x00,0x60,0x00,0x60,0x00,0x00,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0xE0,0x00,0xC0,0x00,
    //k
    0x00,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x63,0x00,0x66,0x00,0x6C,0x00,0x7C,0x00,0x7C,0x00,0x76,0x00,0x66,0x00,0x63,0x00,0x63,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //l
    0x00,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //m
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x6E,0x70,0x7F,0xF8,0x73,0x98,0x63,0x18,0x63,0x18,0x63,0x18,0x63,0x18,0x63,0x18,0x63,0x18,0x00,0x00,0x00,0x00,0x00,0x00,
    //n
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x6F,0x00,0x7F,0x80,0x71,0x80,0x61,0x80,0x61,0x80,0x61,0x80,0x61,0x80,0x61,0x80,0x61,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
    //o
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1E,0x00,0x3F,0x00,0x73,0x80,0x61,0x80,0x61,0x80,0x61,0x80,0x73,0x80,0x3F,0x00,0x1E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //p
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x6E,0x00,0x7F,0x00,0x73,0x80,0x61,0x80,0x61,0x80,0x61,0x80,0x73,0x80,0x7F,0x00,0x6E,0x00,0x60,0x00,0x60,0x00,0x60,0x00,
    //q
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1D,0x80,0x3F,0x80,0x73,0x80,0x61,0x80,0x61,0x80,0x61,0x80,0x73,0x80,0x3F,0x80,0x1D,0x80,0x01,0x80,0x01,0x80,0x01,0x80,
    //r
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x6C,0x00,0x7C,0x00,0x70,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //s
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3E,0x00,0x7F,0x00,0x63,0x00,0x78,0x00,0x3E,0x00,0x0F,0x00,0x63,0x00,0x7F,0x00,0x3E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //t
    0x00,0x00,0x20,0x00,0x60,0x00,0x60,0x00,0xF8,0x00,0xF8,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x78,0x00,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //u
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x61,0x80,0x61,0x80,0x61,0x80,0x61,0x80,0x61,0x80,0x61,0x80,0x63,0x80,0x7F,0x80,0x3D,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
    //v
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x63,0x00,0x63,0x00,0x63,0x00,0x36,0x00,0x36,0x00,0x36,0x00,0x1C,0x00,0x1C,0x00,0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //w
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC7,0x18,0xC7,0x18,0x67,0x30,0x6D,0xB0,0x6D,0xB0,0x6D,0xB0,0x38,0xE0,0x38,0xE0,0x38,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,
    //x
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x63,0x00,0x77,0x00,0x36,0x00,0x1C,0x00,0x1C,0x00,0x1C,0x00,0x36,0x00,0x77,0x00,0x63,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //y
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC1,0x80,0xC1,0x80,0x63,0x00,0x63,0x00,0x36,0x00,0x36,0x00,0x3E,0x00,0x1C,0x00,0x1C,0x00,0x18,0x00,0x78,0x00,0x70,0x00,
    //z
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0x00,0x7F,0x00,0x06,0x00,0x0E,0x00,0x1C,0x00,0x38,0x00,0x30,0x00,0x7F,0x00,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    //{
    0x00,0x00,0x1C,0x00,0x3C,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0xE0,0x00,0xE0,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x3C,0x00,0x1C,0x00,
    //|
    0x00,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,0x60,0x00,
    //}
    0x00,0x00,0xE0,0x00,0xF0,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x1C,0x00,0x1C,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0xF0,0x00,0xE0,0x00,
    //~
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x80,0x7F,0x80,0x47,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
#endif
/* end ------------------------------------------------------------------*/
