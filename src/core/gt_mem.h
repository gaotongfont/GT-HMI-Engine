/**
 * @file gt_mem.h
 * @author yongg
 * @brief Memory / Memory pool management services
 * @version 0.1
 * @date 2022-05-12 10:47:00
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
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
/**
 * @brief init memory allocation
 */
void gt_mem_init(void);

/**
 * @brief destroy the memory pool
 */
void gt_mem_deinit(void);

/**
 * @brief malloc memory
 *
 * @param size Want to malloc memory size [byte]
 * @return void* The buffer pointer which malloc successfully
 */
void * _mem_malloc(size_t size, uint8_t * file_name, size_t line);

/**
 * @brief Realloc memory buffer size
 *
 * @param ptr The newly buffer memory buffer pointer
 * @param size Realloc memory size [byte]
 * @return void* The buffer pointer which malloc successfully
 */
void * _mem_realloc(void * ptr, size_t size, uint8_t * file_name, size_t line);

/**
 * @brief Free memory which malloc by gt_mem_malloc() or gt_mem_realloc().
 *
 * @param ptr The memory buffer address
 */
void _mem_free(void * ptr, uint8_t * file_name, size_t line);

/**
 * @brief Memory data copy from src to dst, which size was given.
 *
 * @param dst destination address, which want to save data buffer
 * @param src source address
 * @param size The buffer size need to be copy [byte]
 * @return void* The destination address
 */
void * gt_memcpy(void *dst, const void *src, size_t size);

/**
 * @brief Memory data copy from src to dst, which size was given.
 *
 * @param dst destination address, which want to save data buffer
 * @param src source address
 * @param size The buffer size need to be copy [byte]
 * @return void*
 */
void * gt_memmove(void * dst, const void * src, size_t size);

/**
 * @brief Memory buffer byte all set to zero.
 *
 * @param ptr The memory buffer address
 * @param size The length of buffer [byte]
 * @return void* The destination address
 */
void * gt_memset_0(void * ptr, size_t size);

/**
 * @brief Memory buffer byte all set to value.
 *
 * @param ptr The memory buffer address
 * @param val The default value of buffer every single byte
 * @param size The length of buffer [byte]
 * @return void* The destination address
 */
void * gt_memset(void * ptr, uint8_t val, size_t size);

/**
 * @brief Compare data between two memory buffers
 *
 * @param dst destination address
 * @param src source address
 * @param size The buffer size need to be copy [byte]
 * @return int 0: Both arrays have the same data
 */
int gt_memcmp(const void * dst, const void * src, size_t size);

void gt_mem_check_used(void);


#define gt_mem_malloc(size) _mem_malloc(size, (uint8_t * )__FILE__, __LINE__)
#define gt_mem_realloc(ptr, size) _mem_realloc(ptr, size, (uint8_t * )__FILE__, __LINE__)
#define gt_mem_free(ptr) _mem_free(ptr, (uint8_t * )__FILE__, __LINE__)

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_MEM_H_
