/**
 * @file gt_txt.c
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-08-26 14:55:59
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_txt.h"
#include "../core/gt_mem.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief insert src to the pos position in dst
 * 
 * @param dst dst string
 * @param pos position
 * @param src need insert src string
 */
void gt_txt_ins(char * dst, uint32_t pos, char * src)
{
    if( !dst || !src ){
        return;
    }
    uint32_t len_dst = strlen(dst);
    uint32_t len_src = strlen(src);
    uint32_t len_all = len_dst + len_src;
    if( pos > len_dst ){
        return;
    }
    dst = gt_mem_realloc(dst, len_all + 1);
    /*move chars*/
    gt_memcpy(&dst[ pos + len_src ], &dst[pos], len_dst - pos);
    gt_memcpy(&dst[pos], src, len_src);
    dst[len_all] = '\0';
}

/**
 * @brief cut out dst string from pos_start to pos_end
 * 
 * @param dst dst string
 * @param pos_start position start
 * @param pos_end position end
 */
void gt_txt_cut(char * dst, uint32_t pos_start, uint32_t pos_end)
{
    uint32_t len_dst = strlen(dst);
    uint32_t ps,pe;
    
    if( pos_start > pos_end ){
        pe = pos_start;
        ps = pos_end;
    }else{
        ps = pos_start;
        pe = pos_end;
    }
    if( ps > len_dst ){
        return;
    }
    if( pe > len_dst ){
        pe = len_dst;
    }
    gt_memcpy(&dst[ps], &dst[pe], len_dst - pe);
    gt_memset(&dst[len_dst - (pe-ps)], 0, pe-ps);
    len_dst = strlen(dst);
    dst = gt_mem_realloc(dst, len_dst+1);
    dst[len_dst] = '\0';
}
/* end ------------------------------------------------------------------*/
