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
#include "../others/gt_log.h"

#if GT_USE_MEM_ILLEGAL_POINTER_HANDLE
#include "../hal/gt_hal_tick.h"
#endif

#include GT_MEM_CUSTOM_INCLUDE


/* private define -------------------------------------------------------*/
#ifndef USE_MEM_LOG_PRINT
    /**
     * @brief printf memory log information by console
     */
    #define USE_MEM_LOG_PRINT           0
#endif

#ifndef USE_MEM_BIG_UNIT_REMARK
    /**
     * @brief Keep track of large memory calls
     */
    #define USE_MEM_BIG_UNIT_REMARK     0
#endif

#if USE_MEM_BIG_UNIT_REMARK
    /** Memory that exceeds this limit needs to be tracked  */
    #define _BIG_UNIT_REMARK_SIZE       2048
#endif

#ifndef USE_MEM_TRACE_DEBUG_BY_FILE
    /**
     * @brief [TRACE] Memory trace debug, using to debug memory leak
     */
    #define USE_MEM_TRACE_DEBUG_BY_FILE 0
#endif

#ifndef USE_MEM_MAX_REMARK
    /**
     * @brief Display max memory remark address
     */
    #define USE_MEM_MAX_REMARK          0
#endif

#ifndef USE_MEM_REAL_USED_REMARK
    /**
     * @brief Remark the real used memory and block count
     *      default: 0
     */
    #define USE_MEM_REAL_USED_REMARK    0
#endif

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

typedef struct {
    MEM_UNIT addr_start;
    MEM_UNIT addr_end;

#if GT_USE_MEM_ILLEGAL_POINTER_HANDLE
    gt_illegal_pointer_hdl_cb illegal_pointer_cb;
#endif

#if USE_MEM_REAL_USED_REMARK
    MEM_UNIT used;
    MEM_UNIT blocks;        /** current using block count */
#endif

#if USE_MEM_MAX_REMARK
    MEM_UNIT max_addr;
    MEM_UNIT max_count;
#endif

#if GT_DISP_STACK_ABORT_LOADING_WHILE_NOT_ENOUGH_MEM
    uint32_t failed_count;
#endif
}_mem_heap_info_st;

/* static variables -----------------------------------------------------*/

#if GT_MEM_CUSTOM
#if GT_MEM_CUSTOM_POINTER
static GT_ATTRIBUTE_LARGE_RAM_ARRAY MEM_UNIT * _mem_pool_int = NULL;
#else
static GT_ATTRIBUTE_LARGE_RAM_ARRAY MEM_UNIT _mem_pool_int[GT_MEM_SIZE / sizeof(MEM_UNIT)];
#endif  /** GT_MEM_CUSTOM_POINTER */

static GT_ATTRIBUTE_RAM_DATA gt_tlsf_t _tlsf = NULL;
static GT_ATTRIBUTE_RAM_DATA _mem_heap_info_st _heap_info = {0};

#else
/**
 * @brief 映射底层函数接口
 */
static GT_ATTRIBUTE_RAM_DATA gt_mem_hooks_st _self = {
    .malloc_hooks   = _DEFAULT_MALLOC_FUNC,
    .realloc_hooks  = _DEFAULT_REALLOC_FUNC,
    .free_hooks     = _DEFAULT_FREE_FUNC,
    .block_size_hooks = _DEFAULT_BLOCK_SIZE_FUNC,
};
#endif

/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/


#if USE_MEM_TRACE_DEBUG_BY_FILE || USE_MEM_MAX_REMARK || USE_MEM_BIG_UNIT_REMARK || USE_MEM_LOG_PRINT || USE_MEM_REAL_USED_REMARK
static GT_ATTRIBUTE_RAM_TEXT size_t _gt_block_size(void * ptr) {
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
static GT_ATTRIBUTE_RAM_TEXT inline void *_gt_malloc_hooks(size_t size) {
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
static GT_ATTRIBUTE_RAM_TEXT inline void *_gt_realloc_hooks(void * old_ptr, size_t size) {
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
static GT_ATTRIBUTE_RAM_TEXT inline void _gt_free_hooks(void * ptr) {
#if GT_MEM_CUSTOM
    gt_tlsf_free(_tlsf, ptr);
#else
    if (_self.free_hooks) {
        _self.free_hooks(ptr);
    }
#endif
}
#if GT_USE_MEM_ILLEGAL_POINTER_HANDLE
static inline bool GT_ATTRIBUTE_RAM_TEXT
_mem_illegal_pointer_hdl(char * name, void * ptr) {
    if (NULL == _heap_info.illegal_pointer_cb) {
        return false;
    }
    if (NULL == ptr) {
        return false;
    }
    volatile uintptr_t start = _heap_info.addr_start;
    volatile uintptr_t end = _heap_info.addr_end;
    if ((uintptr_t)ptr >= start && (uintptr_t)ptr <= end) {
        return false;
    }
    if (start == end) {
        return false;
    }
    if (0 == start && 0 == end) {
        return false;
    }
    GT_LOGE(GT_LOG_TAG_MEM, "[%s] event get illegal ptr: %p", name, ptr);
    gt_sleep_ms(10);
    _heap_info.illegal_pointer_cb();
    return true;
}
#endif

/* global functions / API interface -------------------------------------*/
#if GT_USE_MEM_ILLEGAL_POINTER_HANDLE
void gt_mem_register_illegal_pointer_event(gt_illegal_pointer_hdl_cb cb)
{
    _heap_info.illegal_pointer_cb = cb;
}
#endif

void gt_mem_init(void)
{
#if GT_MEM_CUSTOM
#if GT_MEM_CUSTOM_POINTER
    if (NULL == _mem_pool_int) {
        _mem_pool_int = (MEM_UNIT * )gt_mem_extern_malloc(sizeof(MEM_UNIT) * GT_MEM_SIZE);
        if (NULL == _mem_pool_int) {
            GT_LOGE(GT_LOG_TAG_MEM, "custom malloc failed");
            return;
        }
    }
#endif
    _tlsf = gt_tlsf_create_with_pool((void *)_mem_pool_int, GT_MEM_SIZE);
    _heap_info.addr_start = (uintptr_t)_mem_pool_int;
    _heap_info.addr_end = (uintptr_t)_mem_pool_int + GT_MEM_SIZE;
#if GT_DISP_STACK_ABORT_LOADING_WHILE_NOT_ENOUGH_MEM
    _heap_info.failed_count = 0;
#endif
#if GT_BOOTING_INFO_MSG
	GT_LOG_A(GT_LOG_TAG_MEM, "Custom memory pool addr: %lx -> %lx, size: 0x%x(%d)", (uintptr_t)_heap_info.addr_start, (uintptr_t)_heap_info.addr_end, GT_MEM_SIZE, GT_MEM_SIZE);
#else
	GT_LOGD(GT_LOG_TAG_MEM, "\t--- addr: %lx -> %lx, size: 0x%x(%d) ---", (uintptr_t)_heap_info.addr_start, (uintptr_t)_heap_info.addr_end, GT_MEM_SIZE, GT_MEM_SIZE);
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
    fprintf(fp, "\t--- addr: %x -> %x, size: 0x%x(%d) ---\n", (uintptr_t)_heap_info.addr_start, (uintptr_t)_heap_info.addr_end, GT_MEM_SIZE, GT_MEM_SIZE);
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
#if GT_MEM_CUSTOM_POINTER
    if (_mem_pool_int) {
        gt_mem_extern_free(_mem_pool_int);
        _mem_pool_int = NULL;
    }
#endif
#endif  /** GT_MEM_CUSTOM */
}

#if GT_MEM_CUSTOM_POINTER
void gt_mem_set_pool_pointer(char * pool_pointer)
{
    _mem_pool_int = (MEM_UNIT *)pool_pointer;
}
#endif

void * _mem_malloc(size_t size, char const * file_name, char const * func_name, size_t line)
{
    void * ret = _gt_malloc_hooks(size);

#if USE_MEM_TRACE_DEBUG_BY_FILE || USE_MEM_MAX_REMARK || USE_MEM_BIG_UNIT_REMARK || USE_MEM_LOG_PRINT || USE_MEM_REAL_USED_REMARK
    size_t block_size = _gt_block_size(ret);
#endif

#if GT_DISP_STACK_ABORT_LOADING_WHILE_NOT_ENOUGH_MEM
    if (NULL == ret) { ++_heap_info.failed_count; }
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
    if ((uintptr_t)ret > _heap_info.max_addr) {
        _heap_info.max_addr = (uintptr_t)ret;
        GT_LOG_A(GT_LOG_TAG_MEM, "[%s%s:%d] >>>>>> %d max mem addr: %p   %d [%d] <<<<<<",
                    file_name, func_name, line, ++_heap_info.max_count, _heap_info.max_addr, size, block_size);
    }
#endif

#if USE_MEM_REAL_USED_REMARK
    if (ret) {
        _heap_info.used += block_size;
        ++_heap_info.blocks;
    }
#endif

#if GT_USE_MEM_ILLEGAL_POINTER_HANDLE
    if (_mem_illegal_pointer_hdl("malloc", ret)) {
        return NULL;
    }
#endif

    return ret;
}

void * _mem_calloc(size_t num, size_t size, char const * file_name, char const * func_name, size_t line)
{
    uint32_t len = num * size;
    void * ptr = _mem_malloc(len, file_name, func_name, line);
    if (NULL == ptr) {
        return ptr;
    }
    gt_memset(ptr, 0, len);

#if GT_USE_MEM_ILLEGAL_POINTER_HANDLE
    if (_mem_illegal_pointer_hdl("calloc", ptr)) {
        return NULL;
    }
#endif

    return ptr;
}

void * _mem_realloc(void * ptr, size_t size, char const * file_name, char const * func_name, size_t line)
{
#if USE_MEM_TRACE_DEBUG_BY_FILE || USE_MEM_MAX_REMARK || USE_MEM_BIG_UNIT_REMARK || USE_MEM_LOG_PRINT || USE_MEM_REAL_USED_REMARK
    size_t old_size = _gt_block_size(ptr);
#endif

    void * ret = _gt_realloc_hooks(ptr, size);

#if USE_MEM_TRACE_DEBUG_BY_FILE || USE_MEM_MAX_REMARK || USE_MEM_BIG_UNIT_REMARK || USE_MEM_LOG_PRINT || USE_MEM_REAL_USED_REMARK
    size_t block_size = _gt_block_size(ret);
#endif

#if GT_DISP_STACK_ABORT_LOADING_WHILE_NOT_ENOUGH_MEM
    if (NULL == ret) { ++_heap_info.failed_count; }
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
    if ((uintptr_t)ret > _heap_info.max_addr) {
        _heap_info.max_addr = (uintptr_t)ret;
        GT_LOG_A(GT_LOG_TAG_MEM, "[%s%s:%d] >>>>>> %d R max mem addr: %p   %d [%d -> %d] <<<<<<",
                    file_name, func_name, line, ++_heap_info.max_count, _heap_info.max_addr, size, old_size, block_size);
    }
#endif

#if USE_MEM_REAL_USED_REMARK
    if (NULL == ret && ptr) {
        --_heap_info.blocks;
    } else if (NULL == ptr && ret) {
        ++_heap_info.blocks;
    }
    _heap_info.used += block_size - old_size;
#endif

#if GT_USE_MEM_ILLEGAL_POINTER_HANDLE
    if (_mem_illegal_pointer_hdl("realloc", ptr)) {
        return NULL;
    }
#endif

    return ret;
}

void _mem_free(void * ptr, char const * file_name, char const * func_name, size_t line)
{
#if GT_USE_MEM_ILLEGAL_POINTER_HANDLE
    if (_mem_illegal_pointer_hdl("free", ptr)) {
        return;
    }
#endif
#if USE_MEM_TRACE_DEBUG_BY_FILE || USE_MEM_MAX_REMARK || USE_MEM_BIG_UNIT_REMARK || USE_MEM_LOG_PRINT || USE_MEM_REAL_USED_REMARK
    size_t block_size = _gt_block_size(ptr);
#endif

#if USE_MEM_BIG_UNIT_REMARK
    if (block_size >= _BIG_UNIT_REMARK_SIZE) {
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
#if USE_MEM_REAL_USED_REMARK
    if (ptr) {
        _heap_info.used -= block_size;
        --_heap_info.blocks;
    }
#endif
    _gt_free_hooks(ptr);
}

#ifdef _GT_PORT_SIMULATOR_ENVS
void gt_mem_print_info(void)
{
#if USE_MEM_MAX_REMARK
	GT_LOG_A(GT_LOG_TAG_MEM, "\t--- addr: %p -> %p, size: 0x%x(%d) max addr: %p, used: 0x%x(%.3f KB), blocks/max: %d / %d ---",
                _heap_info.addr_start, _heap_info.addr_end, GT_MEM_SIZE, GT_MEM_SIZE, _heap_info.max_addr,
                _heap_info.max_addr - _heap_info.addr_start, (_heap_info.max_addr - _heap_info.addr_start) * 1.0 / 1024,
#if USE_MEM_REAL_USED_REMARK
                _heap_info.blocks,
#else
                0,
#endif
                _heap_info.max_count);
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

#if GT_DISP_STACK_ABORT_LOADING_WHILE_NOT_ENOUGH_MEM
void gt_mem_reset_failed_count(void)
{
    _heap_info.failed_count = 0;
}

bool gt_mem_has_failed(void)
{
    return _heap_info.failed_count ? true : false;
}
#endif

void gt_mem_check_used(void)
{
#if GT_MEM_CUSTOM
    float total_size = (float)(_heap_info.addr_end - _heap_info.addr_start);
#if USE_MEM_REAL_USED_REMARK
    static MEM_UNIT prev_blocks = 0, prev_used = 0;
    if (prev_blocks != _heap_info.blocks || prev_used != _heap_info.used) {
        GT_LOGI(GT_LOG_TAG_MEM, "blks: %d, used: %.2f%% [%d / %.0f(%.2f kB) Byte]",
            _heap_info.blocks, _heap_info.used * 100.0 / total_size, _heap_info.used, total_size, total_size / 1024);
        prev_blocks = _heap_info.blocks;
        prev_used = _heap_info.used;
    }
#else
    char * addr_now = gt_mem_malloc(1024);
    if( !addr_now ){
        GT_LOGI(GT_LOG_TAG_MEM, "mem is less 1024byte");
        return;
    }
    gt_mem_free(addr_now);
    float used = (float)((uintptr_t)addr_now - _heap_info.addr_start) / total_size;
    static float prev_used = 0;
    if (prev_used != used) {
        GT_LOGI(GT_LOG_TAG_MEM, "used: %.2f%%",used * 100);
        prev_used = used;
    }
    addr_now = NULL;
#endif  /** USE_MEM_REAL_USED_REMARK */
#endif  /** GT_MEM_CUSTOM */
}

void gt_mem_heap_get_space(gt_mem_info_st * mem_info)
{
    if (NULL == mem_info) {
        return;
    }
    mem_info->start = 0;
    mem_info->end = 0;
    mem_info->current = 0;
    mem_info->used = 0;
    mem_info->blocks = 0;

#if GT_MEM_CUSTOM
    char * addr_now = gt_mem_malloc(1024);
    if (!addr_now) {
        GT_LOGI(GT_LOG_TAG_MEM, "Get heap get space failed");
        return;
    }
    gt_mem_free(addr_now);

    mem_info->start = (MEM_UNIT)_heap_info.addr_start;
    mem_info->end = (MEM_UNIT)_heap_info.addr_end;
    mem_info->current = (MEM_UNIT)addr_now;

#if USE_MEM_REAL_USED_REMARK
    mem_info->used = _heap_info.used;
    mem_info->blocks = _heap_info.blocks;
#endif  /** USE_MEM_REAL_USED_REMARK */

#endif
    addr_now = NULL;
}
/* end ------------------------------------------------------------------*/
