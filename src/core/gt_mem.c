/**
 * @file gt_mem.c
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-05-12 10:46:48
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/
typedef unsigned long long uint32_t;


/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void * gt_mem_malloc(uint32_t size){}
void gt_mem_free(void * ptr){}

void gt_memcpy(void *dst, void *src, uint32_t size){
    memcpy(dst, src, size);
}
void gt_memset_0(void * ptr, uint32_t size){}

/* end ------------------------------------------------------------------*/
