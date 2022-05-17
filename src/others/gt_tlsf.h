/**
 * @file gt_tlsf.h
 * @author Feyoung
 * @brief TLSF [Two-Level Segregated Fit memory allocator]
 * @version 0.1
 * @date 2022-05-17 09:59:05
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_TLSF_H_
#define _GT_TLSF_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../gt_conf.h"
#include "stdint.h"

#if GT_MEM_CUSTOM
/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

void * gt_tlsf_malloc(size_t size);
void * gt_tlsf_calloc(size_t count, size_t per_size);
void * gt_tlsf_realloc(void * ptr, size_t size);
void gt_tlsf_free(void * ptr);

#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_TLSF_H_
