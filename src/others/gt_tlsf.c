/**
 * @file gt_tlsf.c
 * @author Feyoung
 * @brief TLSF [Two-Level Segregated Fit memory allocator]
 * @version 0.1
 * @date 2022-05-17 10:00:20
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_tlsf.h"

#if GT_MEM_CUSTOM
/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief TLSF memory allocator
 * 
 * @param size 
 * @return void* 
 */
void * gt_tlsf_malloc(size_t size) {
}

/**
 * @brief TLSF memory allocator by per objest size.
 * 
 * @param count 
 * @param per_size 
 * @return void* 
 */
void * gt_tlsf_calloc(size_t count, size_t per_size) {
}

/**
 * @brief TLSF memory reallocator.
 * 
 * @param ptr 
 * @param size 
 * @return void* 
 */
void * gt_tlsf_realloc(void * ptr, size_t size) {
}

/**
 * @brief release unused memory.
 * 
 * @param ptr 
 */
void gt_tlsf_free(void * ptr) {
}

#endif  // !GT_MEM_CUSTOM
/* end ------------------------------------------------------------------*/
