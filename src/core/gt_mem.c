/**
 * @file gt_mem.c
 * @author yongg
 * @brief Memory / Memory pool management services
 * @version 0.1
 * @date 2022-05-12 10:46:48
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "stdint.h"
#include "string.h"
#include "gt_mem.h"
#include "../gt_conf.h"
#include "../others/gt_log.h"

#include GT_MEM_CUSTOM_INCLUDE


/* private define -------------------------------------------------------*/
/**
 * @brief printf memory log information by console
 */
#define USE_MEM_LOG_PRINT               0

/**
 * @brief Keep track of large memory calls
 */
#define USE_MEM_BIG_UNIT_REMARK         0
#if USE_MEM_BIG_UNIT_REMARK
    /** Memory that exceeds this limit needs to be tracked  */
    #define _BIG_UNIT_REMARK_SIZE       2048
#endif

/**
 * @brief [TRACE] Memory trace debug, using to debug memory leak
 *
 */
#define USE_MEM_TRACE_DEBUG_BY_FILE     0

/**
 * @brief Display max memory remark address
 */
#define USE_MEM_MAX_REMARK              0

#if USE_MEM_TRACE_DEBUG_BY_FILE
#include "stdio.h"
#include "unistd.h"
#include "sys/stat.h"

#define MEM_TRACE_RUNTIMES_PATH "./runtimes"
#define MEM_TRACE_PATH          "./runtimes/mem"
#define MEM_TRACE_FAILED_PATH   "./runtimes/mem/failed.log"
#define MEM_TRACE_LOG_PATH      "./runtimes/mem/success.log"
#endif

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
    #define _DEFAULT_MALLOC_FUNC        NULL
    #define _DEFAULT_REALLOC_FUNC       NULL
    #define _DEFAULT_FREE_FUNC          NULL
    #define _DEFAULT_BLOCK_SIZE_FUNC    NULL
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

    #ifdef GT_MEM_CUSTOM_BLOCK_SIZE
        #define _DEFAULT_BLOCK_SIZE_FUNC    GT_MEM_CUSTOM_BLOCK_SIZE
    #else
        #define _DEFAULT_BLOCK_SIZE_FUNC    NULL
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
    size_t ( * block_size_hooks)(void * ptr);
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
    .block_size_hooks = _DEFAULT_BLOCK_SIZE_FUNC,
};
#endif

#if USE_MEM_MAX_REMARK
static MEM_UNIT _max_addr = 0;
static MEM_UNIT _malloc_count = 0;
#endif

/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/


#if USE_MEM_TRACE_DEBUG_BY_FILE || USE_MEM_MAX_REMARK || USE_MEM_BIG_UNIT_REMARK || USE_MEM_LOG_PRINT
static size_t _gt_block_size(void * ptr) {
    size_t size = 0;

#if GT_MEM_CUSTOM
    size = gt_tlsf_block_size(ptr);
#else
    if (_self.block_size_hooks) {
        size = _self.block_size_hooks(ptr);
    }
#endif

    return size;
}
#endif

/**
 * @brief 申请内存
 *
 * @param size 内存大小, 总的字节数
 * @return void* 返回内存地址
 */
static inline void *_gt_malloc_hooks(size_t size) {
    void * ptr = NULL;
#if GT_MEM_CUSTOM
    ptr = gt_tlsf_malloc(_tlsf, size);
#else
    if (_self.malloc_hooks) {
        ptr = _self.malloc_hooks(size);
    }
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
static inline void *_gt_realloc_hooks(void * old_ptr, size_t size) {
    void * ptr = NULL;
#if GT_MEM_CUSTOM
    ptr = gt_tlsf_realloc(_tlsf, old_ptr, size);
#else
    if (_self.realloc_hooks) {
        ptr = _self.realloc_hooks(old_ptr, size);
    }
#endif
    return ptr;
}

/**
 * @brief 释放内存
 *
 * @param ptr
 */
static inline void _gt_free_hooks(void * ptr) {
#if GT_MEM_CUSTOM
    gt_tlsf_free(_tlsf, ptr);
#else
    if (_self.free_hooks) {
        _self.free_hooks(ptr);
    }
#endif
}

/* global functions / API interface -------------------------------------*/
void gt_mem_init(void)
{
#if GT_MEM_CUSTOM
    _tlsf = gt_tlsf_create_with_pool((void *)_mem_pool_int, GT_MEM_SIZE);
    _addr_start = (uintptr_t)_mem_pool_int;
    _addr_end = (uintptr_t)_mem_pool_int + GT_MEM_SIZE;
#if GT_BOOTING_INFO_MSG
	GT_LOG_A(GT_LOG_TAG_MEM, "Custom memory pool addr: %lx -> %lx, size: 0x%x(%d)", (uintptr_t)_addr_start, (uintptr_t)_addr_end, GT_MEM_SIZE, GT_MEM_SIZE);
#else
	GT_LOGD(GT_LOG_TAG_MEM, "\t--- addr: %lx -> %lx, size: 0x%x(%d) ---", (uintptr_t)_addr_start, (uintptr_t)_addr_end, GT_MEM_SIZE, GT_MEM_SIZE);
#endif

#if USE_MEM_TRACE_DEBUG_BY_FILE
    uint8_t buffer[128] = {0};
    FILE * fp = NULL;
    struct stat info;
    if (stat(MEM_TRACE_PATH, &info)) {
        mkdir(MEM_TRACE_RUNTIMES_PATH);
        mkdir(MEM_TRACE_PATH);
    }
    fp = fopen(MEM_TRACE_LOG_PATH, "a");
    fprintf(fp, "\t--- addr: %x -> %x, size: 0x%x(%d) ---\n", (uintptr_t)_addr_start, (uintptr_t)_addr_end, GT_MEM_SIZE, GT_MEM_SIZE);
    fflush(fp);
    fclose(fp);
    fp = NULL;
#endif

#endif
}

void gt_mem_deinit(void)
{
#if GT_MEM_CUSTOM
    gt_tlsf_destroy(_tlsf);
#endif
}

void * _mem_malloc(size_t size, char const * file_name, char const * func_name, size_t line)
{
    void * ret = _gt_malloc_hooks(size);

#if USE_MEM_TRACE_DEBUG_BY_FILE || USE_MEM_MAX_REMARK || USE_MEM_BIG_UNIT_REMARK || USE_MEM_LOG_PRINT
    size_t block_size = _gt_block_size(ret);
#endif

#if USE_MEM_LOG_PRINT
    GT_LOG_A(GT_LOG_TAG_MEM, "[%s%s:%d] \t--- malloc addr: %p, size: %d real: %d ---",
        file_name, func_name, line, ret, size, block_size);
#endif

#if USE_MEM_BIG_UNIT_REMARK
    if (size >= _BIG_UNIT_REMARK_SIZE) {
        GT_LOG_A(GT_LOG_TAG_MEM, "[%s%s:%d] ### + %p big unit: 0x%x(%d) real: %d ###",
                    file_name, func_name, line, ret, size, size, block_size);
    }
#endif

#if USE_MEM_TRACE_DEBUG_BY_FILE
    uint8_t buffer[128] = {0};
    sprintf((char *)buffer, "%s/%p_%d.mem\0", MEM_TRACE_PATH, ret, block_size);
    FILE * fp = fopen(buffer, "w");
    if (NULL == fp) {
        GT_LOGE(GT_LOG_TAG_MEM, "fopen failed: %s", buffer);
        return ret;
    }

    fprintf(fp, "[+]%s%s:%d   %p   want: 0x%x(%d)   real: 0x%x(%d)\n",
        file_name, func_name, line, ret, size, size, block_size, block_size);
    fflush(fp);
    fclose(fp);
#endif

#if USE_MEM_MAX_REMARK
    if ((uintptr_t)ret > _max_addr) {
        _max_addr = (uintptr_t)ret;
        GT_LOG_A(GT_LOG_TAG_MEM, "[%s%s:%d] >>>>>> %d max mem addr: %p   %d [%d] <<<<<<",
                    file_name, func_name, line, ++_malloc_count, _max_addr, size, block_size);
    }
#endif
    return ret;
}

void * _mem_realloc(void * ptr, size_t size, char const * file_name, char const * func_name, size_t line)
{
#if USE_MEM_TRACE_DEBUG_BY_FILE || USE_MEM_MAX_REMARK || USE_MEM_BIG_UNIT_REMARK || USE_MEM_LOG_PRINT
    size_t old_size = _gt_block_size(ptr);
#endif

    void * ret = _gt_realloc_hooks(ptr, size);

#if USE_MEM_TRACE_DEBUG_BY_FILE || USE_MEM_MAX_REMARK || USE_MEM_BIG_UNIT_REMARK || USE_MEM_LOG_PRINT
    size_t block_size = _gt_block_size(ret);
#endif

#if USE_MEM_LOG_PRINT
    GT_LOG_A(GT_LOG_TAG_MEM, "[%s%s:%d] \t--- realloc addr: %p -> %p, size: %d real: %d -> %d ---",
        file_name, func_name, line, ptr, ret, size, old_size, block_size);
#endif

#if USE_MEM_BIG_UNIT_REMARK
    if (size >= _BIG_UNIT_REMARK_SIZE) {
        GT_LOG_A(GT_LOG_TAG_MEM, "[%s%s:%d] ### ++ %p -> %p big unit: %d %d -> %d ###",
                    file_name, func_name, line, ptr, ret, size, old_size, block_size);
    }
#endif

#if USE_MEM_TRACE_DEBUG_BY_FILE
    uint8_t buffer[128] = {0};
    FILE * fp = NULL;

    // 删除之前内存记录
    sprintf((char *)buffer, "%s/%p_%d.mem\0", MEM_TRACE_PATH, ptr, old_size);
    if (ptr != ret
#if USE_MEM_TRACE_DEBUG_BY_FILE || USE_MEM_MAX_REMARK || USE_MEM_BIG_UNIT_REMARK || USE_MEM_LOG_PRINT
    || old_size != block_size
#endif
    ) {
        if (unlink(buffer) < 0) {
            fp = fopen(MEM_TRACE_FAILED_PATH, "a");
            fprintf(fp, "[++]%s%s:%d   addr: %p -> %p,   want: 0x%x(%d)   real: 0x%x(%d) -> 0x%x(%d)\n",
                file_name, func_name, line, ptr, ret, size, size, old_size, old_size, block_size, block_size);
            fflush(fp);
            fclose(fp);
            fp = NULL;
        }
    }

    sprintf((char *)buffer, "%s/%p_%d.mem\0", MEM_TRACE_PATH, ret, block_size);
    fp = fopen(buffer, "w");
    if (NULL == fp) {
        GT_LOGE(GT_LOG_TAG_MEM, "fopen failed: %s", buffer);
        return ret;
    }

    fprintf(fp, "[++]%s%s:%d   %p -> %p   want: 0x%x(%d),  real: 0x%x(%d) -> 0x%x(%d)\n",
        file_name, func_name, line, ptr, ret, size, size, old_size, old_size, block_size, block_size);
    fflush(fp);
    fclose(fp);
#endif

#if USE_MEM_MAX_REMARK
    if ((uintptr_t)ret > _max_addr) {
        _max_addr = (uintptr_t)ret;
        GT_LOG_A(GT_LOG_TAG_MEM, "[%s%s:%d] >>>>>> %d R max mem addr: %p   %d [%d -> %d] <<<<<<",
                    file_name, func_name, line, ++_malloc_count, _max_addr, size, old_size, block_size);
    }
#endif
    return ret;
}

void _mem_free(void * ptr, char const * file_name, char const * func_name, size_t line)
{
#if USE_MEM_TRACE_DEBUG_BY_FILE || USE_MEM_MAX_REMARK || USE_MEM_BIG_UNIT_REMARK || USE_MEM_LOG_PRINT
    size_t block_size = _gt_block_size(ptr);
#endif

#if USE_MEM_BIG_UNIT_REMARK
    if (size >= _BIG_UNIT_REMARK_SIZE) {
        GT_LOG_A(GT_LOG_TAG_MEM, "[%s%s:%d] ### - %p big unit: 0x%x(%d) ###",
                    file_name, func_name, line, ptr, block_size, block_size);
    }
#endif

#if USE_MEM_LOG_PRINT
    GT_LOG_A(GT_LOG_TAG_MEM, "[%s%s:%d] \t--- free addr: %p, size: 0x%x ---", file_name, func_name, line, ptr, block_size);
#endif

#if USE_MEM_TRACE_DEBUG_BY_FILE
    uint8_t buffer[128] = {0};
    FILE * fp = NULL;

    // 删除之前内存记录
    sprintf((char *)buffer, "%s/%p_%d.mem\0", MEM_TRACE_PATH, ptr, block_size);
    if (unlink(buffer) < 0) {
        fp = fopen(MEM_TRACE_FAILED_PATH, "a");
        fprintf(fp, "[-]%s%s:%d    addr: %p    real size: 0x%x(%d)\n",
            file_name, func_name, line, ptr, block_size, block_size);
        fflush(fp);
        fclose(fp);
        fp = NULL;
    } else {
        fp = fopen(MEM_TRACE_LOG_PATH, "a");
        fprintf(fp, "[o]%s%s:%d    addr: %p    real size: 0x%x(%d)\n",
            file_name, func_name, line, ptr, block_size, block_size);
        fflush(fp);
        fclose(fp);
        fp = NULL;
    }
#endif
    _gt_free_hooks(ptr);
}

#ifdef _GT_PORT_SIMULATOR_ENVS
void gt_mem_print_info(void)
{
#if USE_MEM_MAX_REMARK
	GT_LOG_A(GT_LOG_TAG_MEM, "\t--- addr: %p -> %p, size: 0x%x(%d) max addr: %p, used: 0x%x(%.3f KB), cnt: %d ---",
                _addr_start, _addr_end, GT_MEM_SIZE, GT_MEM_SIZE, _max_addr,
                _max_addr - _addr_start, (_max_addr - _addr_start) * 1.0 / 1024, _malloc_count);
#endif
}
#endif

void * gt_memcpy(void * dst, const void * src, size_t size)
{
    if (NULL == dst || NULL == src) {
        return NULL;
    }
    if (0 == size) {
        return dst;
    }
    return memcpy(dst, src, size);
}

void * gt_memmove(void * dst, const void * src, size_t size)
{
    return memmove(dst, src, size);
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

void gt_mem_check_used(void)
{
#if GT_MEM_CUSTOM
    char * addr_now = gt_mem_malloc(1024);
    if( !addr_now ){
        GT_LOGI(GT_LOG_TAG_MEM, "mem is less 1024byte");
        return;
    }
    float used = (float)((uintptr_t)addr_now - _addr_start) / (float)( _addr_end - _addr_start );
    gt_mem_free(addr_now);
    GT_LOGI(GT_LOG_TAG_MEM, "used:%.2f%%",used*100);
    printf("used:%.2f%%\n",used*100);
#endif
}
/* end ------------------------------------------------------------------*/
