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
#include "stddef.h"
#include "../gt_conf.h"
#include "string.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_mem_init(void);
void gt_mem_deinit(void);

void * gt_mem_malloc(size_t size);
void * gt_mem_realloc(void * ptr, size_t size);
void gt_mem_free(void * ptr);

void * gt_memcpy(void *dst, const void *src, size_t size);
void * gt_memset_0(void * ptr, size_t size);
void * gt_memset(void * ptr, uint8_t val, size_t size);
int gt_memcmp(const void * dst, const void * src, size_t size);

void gt_mem_check_used(void);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_MEM_H_
