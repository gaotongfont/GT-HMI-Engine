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
#include "../others/gt_log.h"
#include "../others/gt_types.h"

/* define ---------------------------------------------------------------*/
#ifndef GT_MEM_USE_MUTEX
    /**
     * @brief Mult thread mode suggest enabled
     *      default: 0
     */
    #define GT_MEM_USE_MUTEX                        0
#endif

#ifndef GT_MEM_USE_CANARY
    /**
     * @brief Enable a backend-agnostic safe-heap guard. Every allocation gets a
     *      magic header so double-free / use-after-free / buffer overflow can be
     *      detected deterministically at free time.
     *      It wraps the malloc/free hooks, therefore it works over BOTH the TLSF
     *      pool (GT_MEM_CUSTOM=1) and external allocators (GT_MEM_CUSTOM=0).
     *      0: disabled (zero overhead, zero pointer offset)
     *      1: enabled (adds a small header per allocation)
     *      [default: 0]
     */
    #define GT_MEM_USE_CANARY                       0
#endif

#ifndef GT_MEM_QUARANTINE_MAX
    /**
     * @brief [GT_MEM_USE_CANARY only] Number of freed blocks parked in the
     *      quarantine ring before the oldest one is returned to the allocator.
     *      Delaying the actual free keeps the address valid (but poisoned) so
     *      a use-after-free / double-free right after a flush is caught by the
     *      canary check instead of silently landing on a reused block.
     *      It bounds the detector's extra peak memory to this many blocks.
     *      Ignored (zero cost) when GT_MEM_USE_CANARY is 0.
     *      [default: 32]
     */
    #define GT_MEM_QUARANTINE_MAX                    32
#endif

#ifndef GT_MEM_CANARY_POISON_MAX
    /**
     * @brief [GT_MEM_USE_CANARY only] Cap on how many payload bytes are
     *      poisoned at free time. Large blocks (image buffers, font tables)
     *      would otherwise cost an O(size) memset per free; poisoning the first
     *      N bytes is enough to expose the overwhelming majority of
     *      use-after-free reads, which hit the leading object-header fields.
     *      Keeps free O(1) regardless of block size.
     *      Ignored (zero cost) when GT_MEM_USE_CANARY is 0.
     *      [default: 64]
     */
    #define GT_MEM_CANARY_POISON_MAX                 64
#endif

#ifndef GT_MEM_CANARY_QUARANTINE_MUTEX
    /**
     * @brief [GT_MEM_USE_CANARY only] Protect the quarantine ring against
     *      concurrent push (gt_mem_reclaim) from multiple threads. Reuses the
     *      same mutex registered via gt_mem_register_mutex(), so it only takes
     *      effect together with GT_MEM_USE_MUTEX = 1; when GT_MEM_USE_MUTEX is
     *      0 it silently degrades to an unlocked ring (single-threaded build).
     *      The lock never spans the underlying free hooks: the oldest block is
     *      released outside the critical section, so it can not nest with the
     *      allocator mutex and deadlock.
     *      Ignored (zero cost) when GT_MEM_USE_CANARY is 0.
     *      [default: 0]
     */
    #define GT_MEM_CANARY_QUARANTINE_MUTEX           0
#endif

#ifndef GT_USE_MEM_ILLEGAL_POINTER_HANDLE
    /**
     * @brief Register illegal memory pointer handle callback
     *      default: 0
     */
    #define GT_USE_MEM_ILLEGAL_POINTER_HANDLE       0
#endif

#ifndef _GT_MEM_MALLOC_POOL_POINTER
    /**
     * @brief Extern malloc func api
     */
    #define _GT_MEM_MALLOC_POOL_POINTER             malloc
#endif


#ifdef GT_ARCH_64
    #define MEM_UNIT        uint64_t
    #define ALIGN_MASK      0x07
#else
    #define MEM_UNIT        uint32_t
    #define ALIGN_MASK      0x03
#endif



/* typedef --------------------------------------------------------------*/
#if GT_USE_MEM_ILLEGAL_POINTER_HANDLE
typedef void ( * gt_illegal_pointer_hdl_cb)(void);
#endif

/**
 * @brief
 *
 * @param start Get the start address of the memory pool
 * @param end Get the end address of the memory pool
 * @param current Get the current address of the memory pool
 * @param used Get the used memory of the memory pool
 * @param blocks Get the using blocks of the memory pool
 */
typedef struct gt_mem_info_s {
    MEM_UNIT start;
    MEM_UNIT end;
    MEM_UNIT current;

    MEM_UNIT used;          /** current used memory when enabled @ref USE_MEM_REAL_USED_REMARK */
    MEM_UNIT blocks;        /** current using block count when enabled @ref USE_MEM_REAL_USED_REMARK */
}gt_mem_info_st;


#if GT_MEM_USE_MUTEX
typedef gt_res_t ( * gt_mem_mutex_lock_t)(void * mutex, uint32_t wait_ms);
typedef gt_res_t ( * gt_mem_mutex_unlock_t)(void * mutex);

typedef struct gt_mem_mutex_s {
    gt_mem_mutex_lock_t lock_cb;
    gt_mem_mutex_unlock_t unlock_cb;

    void * mtx_hdl;
    uint32_t wait_ms;
}gt_mem_mutex_st;
#endif

/* macros ---------------------------------------------------------------*/
static inline bool gt_is_same_string(const char * str1, const char * str2) {
    return (0 == strcmp(str1, str2));
}

static inline bool gt_is_same_memory(const void * mem1, const void * mem2, size_t size) {
    return (0 == memcmp(mem1, mem2, size));
}


/* global functions / API interface -------------------------------------*/
#if GT_MEM_USE_MUTEX
void gt_mem_register_mutex(gt_mem_mutex_st const * const mutex_params);
gt_mem_mutex_st * gt_mem_get_mutex_hdl(void);
#endif

#if GT_USE_MEM_ILLEGAL_POINTER_HANDLE
/**
 * @brief Register illegal pointger handle event, which want to reboot or do someting else.
 *
 * @param cb default: NULL
 */
void gt_mem_register_illegal_pointer_event(gt_illegal_pointer_hdl_cb cb);
#endif

/**
 * @brief init memory allocation
 */
void gt_mem_init(void);

/**
 * @brief destroy the memory pool
 */
void gt_mem_deinit(void);

#if GT_MEM_CUSTOM_POINTER
/**
 * @brief Set the memory pool pointer, The memory size is defined
 *      by GT_MEM_SIZE and must be aligned with MEM_UNIT bytes
 *      [Warning: This function must be called before gt_init()]
 *
 * @param pool_pointer The memory size is defined by GT_MEM_SIZE
 *      and must be aligned with MEM_UNIT bytes
 */
void gt_mem_set_pool_pointer(char * pool_pointer);
#endif
/**
 * @brief malloc memory
 *
 * @param size Want to malloc memory size [byte]
 * @return void* The buffer pointer which malloc successfully
 */
void * _mem_malloc(size_t size, char const * file_name, char const * func_name, size_t line);

/**
 * @brief calloc memory and init memory data to zero.
 *      Base on malloc() and memset().
 *
 * @param num number of elements
 * @param size size of each element
 * @param file_name
 * @param func_name
 * @param line
 * @return void*
 */
void * _mem_calloc(size_t num, size_t size, char const * file_name, char const * func_name, size_t line);

/**
 * @brief Realloc memory buffer size
 *
 * @param ptr The newly buffer memory buffer pointer
 * @param size Realloc memory size [byte]
 * @return void* The buffer pointer which malloc successfully
 */
void * _mem_realloc(void * ptr, size_t size, char const * file_name, char const * func_name, size_t line);

/**
 * @brief Free memory which malloc by gt_mem_malloc() or gt_mem_realloc().
 *
 * @param ptr The memory buffer address
 */
void _mem_free(void * ptr, char const * file_name, char const * func_name, size_t line);

#ifdef _GT_PORT_SIMULATOR_ENVS
void gt_mem_print_info(void);
#endif

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

#if GT_DISP_STACK_ABORT_LOADING_WHILE_NOT_ENOUGH_MEM
/**
 * @brief Reset the count number of memory malloc / realloc failures
 */
void gt_mem_reset_failed_count(void);

/**
 * @brief Check whether the memory malloc / realloc has failed
 *
 * @return true The memory malloc / realloc has failed
 * @return false normal memory malloc / realloc
 */
bool gt_mem_has_failed(void);
#endif

void gt_mem_check_used(void);

/**
 * @brief Get the memory pool address space
 *
 * @param mem_info Get the memory pool information
 */
void gt_mem_heap_get_space(gt_mem_info_st * mem_info);

/**
 * @brief Mark a block as deferred (quarantined) so its memory stays valid until
 *      gt_mem_reclaim() releases it. A gt_mem_free() on a deferred block is
 *      treated as a double-free. No-op when GT_MEM_USE_CANARY is 0.
 *
 * @param ptr payload pointer previously returned by gt_mem_malloc/realloc/calloc
 */
void gt_mem_defer(void * ptr);

/**
 * @brief Release a block previously marked deferred by gt_mem_defer().
 *      Equivalent to gt_mem_free() when GT_MEM_USE_CANARY is 0.
 *
 * @param ptr payload pointer previously passed to gt_mem_defer()
 */
void gt_mem_reclaim(void * ptr);

#ifndef gt_mem_malloc
    #define gt_mem_malloc(size)         _mem_malloc(size, _GT_LOG_PRINT_FILE, __func__, __LINE__)
#endif
#ifndef gt_mem_calloc
    #define gt_mem_calloc(num, size)    _mem_calloc(num, size, _GT_LOG_PRINT_FILE, __func__, __LINE__)
#endif
#ifndef gt_mem_realloc
    #define gt_mem_realloc(ptr, size)   _mem_realloc(ptr, size, _GT_LOG_PRINT_FILE, __func__, __LINE__)
#endif
#ifndef gt_mem_free
    #define gt_mem_free(ptr)            _mem_free(ptr, _GT_LOG_PRINT_FILE, __func__, __LINE__)
#endif

/**
 * @brief Third-party memory allocation interface, such as: src/extra/.
 *      Maybe you want to use psram memory pool.
 *      [Warning]: malloc/calloc/realloc/free must be used in pairs,
 *          free() must be the same properly as malloc/calloc/realloc!!!
 *          DO NOT MIXED with OTHER memory allocation interface!!!
 */
#ifndef gt_mem_extern_malloc
    /**
     * @brief Default using gt_mem_malloc()
     */
    #define gt_mem_extern_malloc(size)          gt_mem_malloc(size)
#endif
#ifndef gt_mem_extern_calloc
    /**
     * @brief Default using gt_mem_calloc()
     */
    #define gt_mem_extern_calloc(num, size)     gt_mem_calloc(num, size)
#endif
#ifndef gt_mem_extern_realloc
    /**
     * @brief Default using gt_mem_realloc()
     */
    #define gt_mem_extern_realloc(ptr, size)    gt_mem_realloc(ptr, size)
#endif
#ifndef gt_mem_extern_free
    /**
     * @brief Default using gt_mem_free()
     */
    #define gt_mem_extern_free(ptr)             gt_mem_free(ptr)
#endif


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_MEM_H_
