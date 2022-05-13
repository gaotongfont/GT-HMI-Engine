/**
 * @file gt_mem.h
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-05-12 10:47:00
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_MEM_H_
#define _GT_MEM_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/



/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void * gt_mem_malloc(uint32_t size);
void gt_mem_free(void * ptr);

void gt_memcpy(void *dst, void *src, uint32_t size);
void gt_memset_0(void * ptr, uint32_t size);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_MEM_H_
