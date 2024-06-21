/**
 * @file gt_txt.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-08-26 14:55:59
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_txt.h"
#include "../core/gt_mem.h"
#include "../font/gt_font.h"
#include "gt_log.h"
/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/


static char gt_txt_string_del(char * str, uint8_t encoding, uint32_t del_txt_pos) {
    uint32_t len = 0, len_str = str ? strlen(str) : 0;
    int32_t idx = del_txt_pos;

    len = gt_txt_check_char_numb(str, encoding, &idx);

    if(!len){
        return 0 ;
    }

    if(len_str > del_txt_pos){
        gt_memcpy( &str[idx] , &str[idx+len], len_str - (idx+len) ) ;
    }
    gt_memset(&str[len_str - len], 0, len);
    len_str = strlen(str);
    str = (char * )gt_mem_realloc(str, len_str + 1);
    str[len_str] = '\0';

    return len;

}
/* global functions / API interface -------------------------------------*/

uint8_t gt_txt_check_char_numb(char * dst, uint8_t encoding, int32_t* pos)
{
    uint32_t len = 0;
    int32_t *idx = pos;
    while ((*idx) >= 0) {
        if(GT_ENCODING_UTF8 == encoding) {
            len = gt_utf8_check_char((uint8_t * )&dst[*idx]);
        }
        else if(GT_ENCODING_GB == encoding) {
            len = gt_gb_check_char((uint8_t * )dst , *pos , NULL);
            *pos -= (len-1);
            return len;
        }
        if(!len) {
            --(*idx);
            continue;
        }
        break;
    }

    return len;
}


/**
 * @brief insert src to the pos position in dst
 *
 * @param dst dst string
 * @param pos position
 * @param src need insert src string
 */
char * gt_txt_ins(char * dst, uint32_t pos, char * src)
{
    if( !dst || !src ){
        return dst;
    }
    uint32_t len_dst = strlen(dst);
    uint32_t len_src = strlen(src);
    uint32_t len_all = len_dst + len_src;
    if( pos > len_dst ){
        return dst;
    }
    dst = (char * )gt_mem_realloc(dst, len_all + 1);
    /*move chars*/
    if (len_dst > pos) {
        // gt_memcpy(&dst[ pos + len_src ], &dst[pos], len_dst - pos);
        gt_memmove(&dst[ pos + len_src ], &dst[pos], len_dst - pos);
    }
    gt_memmove(&dst[pos], src, len_src);
    // gt_memcpy(&dst[pos], src, len_src);
    dst[len_all] = '\0';

    return dst;
}

/**
 * @brief cut out dst string from pos_start to pos_end
 *
 * @param dst dst string
 * @param pos_start position start
 * @param pos_end position end
 */
char gt_txt_cut(char * dst, uint8_t encoding, uint32_t pos_start, uint32_t pos_end)
{
    uint32_t len_dst = dst ? strlen(dst) : 0;
    uint32_t ps,pe;

    if (!len_dst) {
        return 0;
    }

    if( pos_start > pos_end ){
        pe = pos_start;
        ps = pos_end;
    }else{
        ps = pos_start;
        pe = pos_end;
    }
    if( ps > len_dst ){
        return 0;
    }
    if( pe > len_dst ){
        pe = len_dst;
    }

#if 0
    if (len_dst > pe) {
        gt_memcpy(&dst[ps], &dst[pe], len_dst - pe);
    }
    gt_memset(&dst[len_dst - (pe-ps)], 0, pe-ps);
    len_dst = strlen(dst);
    dst = (char * )gt_mem_realloc(dst, len_dst + 1);
    dst[len_dst] = '\0';
    return 1;
#else
    return gt_txt_string_del(dst, encoding, ps);
#endif
}
/* end ------------------------------------------------------------------*/
