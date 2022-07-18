/**
 * @file gt_font.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-06-01 11:22:22
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_font.h"
// #include "gtzk.h"
#include "stdint.h"
#include "stdbool.h"
#include "../others/gt_log.h"
#include "../core/gt_mem.h"
/* private define -------------------------------------------------------*/

#define _GT_ENABLE_GTZK 1

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/
uint8_t latin_16x16_width[] = {
    // l:atin
    // 1.1:0x0020UL<=unicode<=0x007FUL
    0x04,  0x04,  0x08,  0x09,  0x09,  0x10,  0x0d,  0x04,  0x06,  0x05,  0x06,  0x09,  0x04,  0x06,  0x04,  0x05,  0x09,  0x07,  0x09,  0x09,  0x09,  0x09,  0x09,  0x09,  0x09,  0x09,  0x05,  0x05,  0x09,  0x09,  0x09,  0x0a,  0x10,  0x0c,  0x0c,  0x0c,  0x0c,  0x0b,  0x0a,  0x0c,  0x0c,  0x04,  0x09,  0x0c,  0x0a,  0x0d,  0x0c,  0x0c,  0x0b,  0x0c,  0x0d,  0x0b,  0x0b,  0x0c,  0x0c,  0x10,  0x0b,  0x0b,  0x0a,  0x06,  0x05,  0x06,  0x09,  0x0a,  0x04,  0x0a,  0x0a,  0x09,  0x0a,  0x09,  0x06,  0x0a,  0x0a,  0x04,  0x04,  0x09,  0x04,  0x0e,  0x0a,  0x0a,  0x0a,  0x0a,  0x07,  0x09,  0x06,  0x0a,  0x09,  0x0e,  0x09,  0x0a,  0x09,  0x07,  0x04,  0x07,  0x0a,  
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


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

/**
 * @brief get idx in unicode_arr by unicode (for )
 * 
 * @param unicode_arr 
 * @param depth 
 * @param unicode 
 * @return uint32_t 
 */
static uint32_t __get_idx(uint32_t * unicode_arr, uint8_t depth, uint32_t unicode)
{
    uint8_t idx = 0;
    while( idx < depth ){
        if( unicode >= unicode_arr[idx << 2] && unicode <= unicode_arr[(idx << 2) + 1] ){
            idx += unicode - unicode_arr[idx << 2];
            break;
        }else{
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
static bool _gt_font_latin_check(uint32_t unicode)
{
    uint32_t code[] = {
        //start     end
        0x0020UL, 0x007FUL,
        0x00A0UL, 0x017FUL,
        0x01A0UL, 0x01CFUL,
        0x01F0UL, 0x01FFUL,
        0x0210UL, 0x021FUL,
        0x1EA0UL, 0x1EFFUL,
    };
    bool res = false;
    for(int i=0; i<6; i++){
        if( unicode >= code[i<<1] && unicode <= code[(i<<1)+1] ){
            res = true;
        }
    }
    return res;
}



/**
 * @brief get one latin word width
 * 
 * @param unicode latin unicode
 * @param font_size font size
 * @return uint8_t the latin unicode font width
 */
static uint8_t _gt_font_latin_get_width(uint32_t unicode, uint8_t font_size)
{
    uint32_t code[] = {
        //start     end
        0x0020UL, 0x007FUL,
        0x00A0UL, 0x017FUL,
        0x01A0UL, 0x01CFUL,
        0x01F0UL, 0x01FFUL,
        0x0210UL, 0x021FUL,
        0x1EA0UL, 0x1EFFUL,
    };
    uint32_t idx = __get_idx(code, 6, unicode);
    uint8_t width = 0;
    switch( font_size ){
        case 16:
            width = latin_16x16_width[idx];
            break;
        case 24:
            break;
        default:
            break;
    }
    return width;

}


uint8_t utf8_to_unicode(uint8_t * utf8,uint32_t *res)
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

#if _GT_ENABLE_GTZK == 1
extern unsigned long GT_GetChinese(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);
extern unsigned long GT_GetLatin(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);
#endif
// extern unsigned long GT_GetASCII(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);

// extern unsigned long GT_GetJapanese(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);
// extern unsigned long GT_GetKorean(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);
// extern unsigned long GT_GetCyrillic(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);

#define GT_USE_FONT_NUM     2

typedef unsigned long(* read_func_t)(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);

read_func_t read_func_arr[] = {
#if _GT_ENABLE_GTZK == 1
    GT_GetChinese,
    GT_GetLatin,
#endif
    // GT_GetASCII,
    // GT_GetJapanese,
    // GT_GetKorean,
    // GT_GetCyrillic,
	NULL
};


/* global functions / API interface -------------------------------------*/


void gt_font_get_string_dot(gt_font_st * font)
{
    uint32_t idx = 0, unicode, len, idx_func = 0, idx_read = 0, temp = 0;
    uint8_t data[64];
    unsigned long (* read)(unsigned long, unsigned int, unsigned char*);
    GT_LOGV(GT_LOG_TAG_GUI, "text:%s", font->utf8);
    while(idx < font->len) {
        
        len = utf8_to_unicode(&font->utf8[idx], &unicode);
        /* get font data */
        while( idx_func < GT_USE_FONT_NUM ) {
            read = read_func_arr[idx_func];
            if( !read ) {  
                idx_func++;
                continue;
            }
            if( unicode < 0x80 ){
                temp = read(unicode, font->style_en, data);
                if( 0 == temp ){
                    idx_func++;
                    continue;
                }
                temp -= 2;
                gt_memcpy(&font->res[idx_read], &data[2], temp-2);
            }else if( _gt_font_latin_check(unicode) ){
                temp = read(unicode, font->style_en, data);
                if( 0 == temp ){
                    idx_func++;
                    continue;
                }
                temp -= 2;
                gt_memcpy(&font->res[idx_read], &data[2], temp-2);
            }else{
                temp = read(unicode, font->style_cn, &font->res[idx_read]);
            }
            idx_read += temp;
            if(0 != temp) {
                break;
            }
            idx_func++;
        }
        idx_func = 0;
        idx+=len;
    }
}

uint32_t gt_font_count_word(char * utf8, uint32_t len)
{
    uint32_t idx = 0, cnt = 0, unicode;
    while(idx < len){
        idx += utf8_to_unicode((uint8_t *)&utf8[idx], &unicode);
        cnt++;
    }
    return cnt;
}

/**
 * @brief get string all width
 * 
 * @param font string msg
 * @return uint32_t all width
 */
uint32_t gt_font_get_string_width(gt_font_st * font)
{

	uint32_t idx = 0, unicode, width = 0, temp;
    
    while( idx < font->len ){
        temp = utf8_to_unicode(&font->utf8[idx], &unicode);
        idx += temp;
        if( _gt_font_latin_check(unicode) ){
            //the unicode is latin, need get width
           width += _gt_font_latin_get_width( unicode, font->size );
        }else{
            //the unicode is chinese or other
            width += font->size;
        }
    }
    return width;
}


uint8_t gt_font_get_one_word_width(uint32_t unicode, uint8_t font_size){
    if( _gt_font_latin_check(unicode) ){
        return _gt_font_latin_get_width(unicode, font_size);
    }else{
        return font_size;
    }
}

/**
 * @brief get next word width from font_st
 * 
 * @param font font msg
 * @param is_head this can be 0:start from cur, 1:start from head
 * @return uint8_t next word width
 */
uint8_t gt_font_get_next_word_width(gt_font_st * font, uint8_t is_head){
    static uint16_t idx = 0;
    uint16_t i = 0, idx_utf8 = 0;
    uint32_t unicode = 0;
    if(is_head){
        idx = 0;
    }
    while( i <= idx ){
        idx_utf8 += utf8_to_unicode(&font->utf8[idx_utf8], &unicode);
        i++;
    }
    idx++;
    return gt_font_get_one_word_width(unicode, font->size);
}
/* end ------------------------------------------------------------------*/
