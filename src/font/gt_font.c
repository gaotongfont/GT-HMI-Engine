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
// extern unsigned long GT_GetJapanese(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);
// extern unsigned long GT_GetKorean(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);
// extern unsigned long GT_GetCyrillic(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);

#define GT_USE_FONT_NUM     2

void * read_func_arr[] = {
	NULL
    // GT_GetChinese,
    // GT_GetASCII,
    // GT_GetJapanese,
    // GT_GetKorean,
    // GT_GetCyrillic,
};


/* global functions / API interface -------------------------------------*/

void gt_font_get_text(gt_font_st * font)
{
    uint32_t idx = 0, unicode, len, fidx = 0, ridx = 0, temp = 0;
    unsigned long (* read)(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer);
    GT_LOGV(GT_LOG_TAG_GUI, "text:%s", font->utf8);
    while(idx < font->len) {
        len = utf8_to_unicode(&font->utf8[idx], &unicode);
        /* get font data */
        while( fidx < GT_USE_FONT_NUM ) {
            read = read_func_arr[fidx];
            if( !read ) { 
                fidx++;
                continue;
            }
            if( unicode < 0x80 ){
                temp = read(unicode, font->style_en, &font->res[ridx]);
            }else{
                temp = read(unicode, font->style_cn, &font->res[ridx]);
            }
            ridx += temp;
            if(0 != temp) {
                break;
            }
            fidx++;
        }
        fidx = 0;
        idx+=len;
    }
}


/* end ------------------------------------------------------------------*/
