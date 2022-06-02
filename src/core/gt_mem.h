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
#include "stdint.h"
#include "../others/gt_types.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/

/**
 * @brief 指针
 */
typedef struct gt_mem_hooks_s {
    void * ( * malloc_hooks)(size_t size);
    void * ( * calloc_hooks)(size_t count, size_t per_size);
    void * ( * realloc_hooks)(void * ptr, size_t size);
    void   ( * free_hooks)(void *ptr);
}gt_mem_hooks_st;

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief 初始化内存
 * 
 * @param hooks 
 * @return uint8_t 
 */
uint8_t gt_mem_init(struct gt_mem_hooks_s *hooks);

void * gt_mem_malloc(size_t size);
void * gt_mem_calloc(size_t count, size_t per_size);
void * gt_mem_realloc(void * ptr, size_t size);
void gt_mem_free(void * ptr);

void gt_memcpy(void *dst, void *src, size_t size);
void gt_memset_0(void * ptr, size_t size);
void gt_memset(void * ptr, uint8_t val, size_t size);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_MEM_H_
