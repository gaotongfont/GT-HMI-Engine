/**
 * @file gt_mem.c
 * @author yongg
 * @brief 
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

/* private define -------------------------------------------------------*/

#if GT_MEM_CUSTOM
    #define _DEFAULT_MALLOC_FUNC    gt_tlsf_malloc
    #define _DEFAULT_CALLOC_FUNC    gt_tlsf_calloc
    #define _DEFAULT_REALLOC_FUNC   gt_tlsf_realloc
    #define _DEFAULT_FREE_FUNC      gt_tlsf_free
#else
    #include GT_MEM_CUSTOM_INCLUDE

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

    #ifdef GT_MEM_CUSTOM_CALLOC
        #define _DEFAULT_CALLOC_FUNC        GT_MEM_CUSTOM_CALLOC
    #else
        #define _DEFAULT_CALLOC_FUNC        calloc
    #endif

    #ifdef GT_MEM_CUSTOM_FREE
        #define _DEFAULT_FREE_FUNC          GT_MEM_CUSTOM_FREE
    #else
        #define _DEFAULT_FREE_FUNC          free
    #endif
#endif



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/
/**
 * @brief 映射底层函数接口
 *  [!]禁止使用`realloc`方法, 分配内存.
 */
static gt_mem_hooks_st _self = {
    .malloc_hooks   = _DEFAULT_MALLOC_FUNC,
    .calloc_hooks   = _DEFAULT_CALLOC_FUNC,
    .realloc_hooks  = _DEFAULT_REALLOC_FUNC,
    .free_hooks     = _DEFAULT_FREE_FUNC,
};

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
    void * ptr = _self.malloc_hooks(size);
#ifdef GT_LOGD
    GT_LOGD(GT_LOG_TAG_MEM, "\t--- malloc addr: 0x%p, size: 0x%x ---", ptr, size);
#endif
    return ptr;
}

/**
 * @brief 申请内存
 * 
 * @param count 
 * @param per_size 
 * @return void* 
 */
static void *_gt_calloc_hooks(size_t count, size_t per_size)
{
    void * ptr = _self.calloc_hooks(count, per_size);
#ifdef GT_LOGD
    GT_LOGD(GT_LOG_TAG_MEM, "\t--- calloc addr: 0x%p, size: 0x%x ---", ptr, count * per_size);
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
    void * ptr = _self.realloc_hooks(old_ptr, size);
#ifdef GT_LOGD
    GT_LOGD(GT_LOG_TAG_MEM, "\t--- realloc addr: 0x%p -> 0x%p, size: 0x%x ---", old_ptr, ptr, size);
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
#ifdef GT_LOGD
    GT_LOGD(GT_LOG_TAG_MEM, "\t--- free addr: 0x%p ---", ptr);
#endif
    _self.free_hooks(ptr);
}


/* global functions / API interface -------------------------------------*/
/**
 * @brief 设置
 * 
 * @param hooks 需要重映射的函数接口
 * @return uint8_t 0: 正常退出
 */
uint8_t gt_mem_init(struct gt_mem_hooks_s *hooks)
{
    if (!hooks) {
        return 1;
    }

    _self.malloc_hooks = hooks->malloc_hooks ? hooks->malloc_hooks : _DEFAULT_MALLOC_FUNC;
    _self.calloc_hooks = hooks->calloc_hooks ? hooks->calloc_hooks : _DEFAULT_CALLOC_FUNC;
    _self.free_hooks   = hooks->free_hooks   ? hooks->free_hooks   : _DEFAULT_FREE_FUNC;

    return 0;
}

void * gt_mem_malloc(size_t size){
    return _gt_malloc_hooks(size);
}

void * gt_mem_calloc(size_t count, size_t per_size)
{
    return _gt_calloc_hooks(count, per_size);
}

void * gt_mem_realloc(void * ptr, size_t size){
    return _gt_realloc_hooks(ptr, size);
}

void gt_mem_free(void * ptr){
    _gt_free_hooks(ptr);
}

void gt_memcpy(void * dst, void * src, size_t size){
    memcpy(dst, src, size);
}

void gt_memset_0(void * ptr, size_t size){
    memset(ptr, 0x00, size);
}

/* end ------------------------------------------------------------------*/
