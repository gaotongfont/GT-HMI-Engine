/**
 * @file gt_mem.c
 * @author yongg
 * @brief Memory / Memory pool management services
 * @version 0.1
 * @date 2022-05-12 10:46:48
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_mem.h"
#include "stdint.h"
#include "string.h"
#include "../gt_conf.h"
#include "../others/gt_log.h"
#include GT_MEM_CUSTOM_INCLUDE


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/



/* private define -------------------------------------------------------*/

#ifdef GT_ARCH_64
    #define MEM_UNIT        uint64_t
    #define ALIGN_MASK      0x07
#else
    #define MEM_UNIT        uint32_t
    #define ALIGN_MASK      0x03
#endif

#if GT_MEM_CUSTOM
    #define _DEFAULT_MALLOC_FUNC    NULL
    #define _DEFAULT_REALLOC_FUNC   NULL
    #define _DEFAULT_FREE_FUNC      NULL
#else
    #ifdef GT_MEM_CUSTOM_MALLOC
        #define _DEFAULT_MALLOC_FUNC        GT_MEM_CUSTOM_MALLOC
    #else
        #define _DEFAULT_MALLOC_FUNC        malloc
    #endif

    #ifdef GT_MEM_CUSTOM_REALLOC
        #define _DEFAULT_REALLOC_FUNC       GT_MEM_CUSTOM_REALLOC
    #else
        #define _DEFAULT_REALLOC_FUNC       realloc
    #endif

    #ifdef GT_MEM_CUSTOM_FREE
        #define _DEFAULT_FREE_FUNC          GT_MEM_CUSTOM_FREE
    #else
        #define _DEFAULT_FREE_FUNC          free
    #endif
#endif



/* private typedef ------------------------------------------------------*/

/**
 * @brief 指针
 */
typedef struct gt_mem_hooks_s {
    void * ( * malloc_hooks)(size_t size);
    void * ( * realloc_hooks)(void * ptr, size_t size);
    void   ( * free_hooks)(void *ptr);
}gt_mem_hooks_st;

/* static variables -----------------------------------------------------*/

#if GT_MEM_CUSTOM
static GT_ATTRIBUTE_LARGE_RAM_ARRAY MEM_UNIT _mem_pool_int[GT_MEM_SIZE / sizeof(MEM_UNIT)];

static gt_tlsf_t _tlsf;

static MEM_UNIT _addr_start = 0, _addr_end = 0;
#else
/**
 * @brief 映射底层函数接口
 */
static gt_mem_hooks_st _self = {
    .malloc_hooks   = _DEFAULT_MALLOC_FUNC,
    .realloc_hooks  = _DEFAULT_REALLOC_FUNC,
    .free_hooks     = _DEFAULT_FREE_FUNC,
};
#endif

/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/


/**
 * @brief 申请内存
 *
 * @param size 内存大小, 总的字节数
 * @return void* 返回内存地址
 */
static void *_gt_malloc_hooks(size_t size)
{
#if GT_MEM_CUSTOM
    void * ptr = gt_tlsf_malloc(_tlsf, size);
#else
    void * ptr = _self.malloc_hooks(size);
#endif

#ifdef GT_LOGV
    GT_LOGV(GT_LOG_TAG_MEM, "\t--- malloc addr: 0x%p, size: 0x%x ---", ptr, size);
#endif
    return ptr;
}

/**
 * @brief 申请内存
 *
 * @param old_ptr
 * @param size
 * @return void*
 */
static void *_gt_realloc_hooks(void * old_ptr, size_t size)
{
#if GT_MEM_CUSTOM
    void * ptr = gt_tlsf_realloc(_tlsf, old_ptr, size);
#else
    void * ptr = _self.realloc_hooks(old_ptr, size);
#endif

#ifdef GT_LOGV
    GT_LOGV(GT_LOG_TAG_MEM, "\t--- realloc addr: 0x%p -> 0x%p, size: 0x%x ---", old_ptr, ptr, size);
#endif
    return ptr;
}

/**
 * @brief 释放内存
 *
 * @param ptr
 */
static void _gt_free_hooks(void * ptr)
{
#ifdef GT_LOGV
    GT_LOGV(GT_LOG_TAG_MEM, "\t--- free addr: 0x%p ---", ptr);
#endif

#if GT_MEM_CUSTOM
    gt_tlsf_free(_tlsf, ptr);
#else
    _self.free_hooks(ptr);
#endif
}


/* global functions / API interface -------------------------------------*/

void gt_mem_init(void)
{
#if GT_MEM_CUSTOM
    _tlsf = gt_tlsf_create_with_pool((void *)_mem_pool_int, GT_MEM_SIZE);
	GT_LOGE(GT_LOG_TAG_MEM, "\t--- addr: 0x%p -> 0x%x, size: 0x%x(%d) ---", _mem_pool_int, (MEM_UNIT)_mem_pool_int + GT_MEM_SIZE, GT_MEM_SIZE, GT_MEM_SIZE);
    _addr_start = (MEM_UNIT)_mem_pool_int;
    _addr_end = (MEM_UNIT)_mem_pool_int + GT_MEM_SIZE;
#endif
}

void gt_mem_deinit(void)
{
#if GT_MEM_CUSTOM
    gt_tlsf_destroy(_tlsf);
#endif
}

void * gt_mem_malloc(size_t size)
{
    return _gt_malloc_hooks(size);
}

void * gt_mem_realloc(void * ptr, size_t size)
{
    return _gt_realloc_hooks(ptr, size);
}

void gt_mem_free(void * ptr)
{
    _gt_free_hooks(ptr);
}

void * gt_memcpy(void * dst, const void * src, size_t size)
{
    return memcpy(dst, src, size);
}

void * gt_memset_0(void * ptr, size_t size)
{
    return memset(ptr, 0x00, size);
}

void * gt_memset(void * ptr, uint8_t val, size_t size)
{
    return memset(ptr, val, size);
}

int gt_memcmp(const void * dst, const void * src, size_t size)
{
    return memcmp(dst, src, size);
}

void gt_mem_check_used(void){
#if GT_MEM_CUSTOM
    char * addr_now = gt_mem_malloc(1024);
    if( !addr_now ){
        GT_LOGI(GT_LOG_TAG_MEM, "mem is less 1024byte");
        return;
    }
    float used = (float)((MEM_UNIT)addr_now - _addr_start) / (float)( _addr_end - _addr_start );
    gt_mem_free(addr_now);
    GT_LOGI(GT_LOG_TAG_MEM, "used:%.2f%%",used*100);
    printf("used:%.2f%%\n",used*100);
#endif
}
/* end ------------------------------------------------------------------*/
