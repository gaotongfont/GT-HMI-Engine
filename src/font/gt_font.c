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
#include "../others/gt_log.h"
/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
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

// extern unsigned long GT_GetChinese(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);
// extern unsigned long GT_GetASCII(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);

// extern unsigned long GT_GetJapanese(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);
// extern unsigned long GT_GetKorean(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);
// extern unsigned long GT_GetCyrillic(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);

#define GT_USE_FONT_NUM     2

typedef unsigned long(* read_func_t)(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);

read_func_t read_func_arr[] = {
    // GT_GetChinese,
    // GT_GetASCII,
    // GT_GetJapanese,
    // GT_GetKorean,
    // GT_GetCyrillic,
	NULL
};


/* global functions / API interface -------------------------------------*/

void gt_font_get_text(gt_font_st * font)
{
    uint32_t idx = 0, unicode, len, idx_func = 0, idx_read = 0, temp = 0;

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
                temp = read(unicode, font->style_en, &font->res[idx_read]);
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

uint32_t gt_font_count_word(char * utf8, uint32_t len){
    uint32_t idx = 0, cnt = 0, unicode;
    while(idx < len){
        idx += utf8_to_unicode((uint8_t *)&utf8[idx], &unicode);
        cnt++;
    }
    return cnt;
}

/* end ------------------------------------------------------------------*/
