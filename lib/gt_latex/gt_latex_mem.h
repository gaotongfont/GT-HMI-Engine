/**
 * @file gt_latex_mem.h
 * @brief GT-Latex 内存管理 / Memory management
 *        默认使用 C 标准库，可通过注册函数替换为自定义分配器
 *        Defaults to C standard library, replaceable via registration
 */

#ifndef _GT_LATEX_MEM_H_
#define _GT_LATEX_MEM_H_

#include "gt_latex_config.h"

#if GT_LATEX_ENABLE

#include <stddef.h>

/* ----------------------------------------------------------------
 * 内存分配函数类型 / Memory allocator function types
 * ---------------------------------------------------------------- */
typedef void *(*gt_latex_malloc_fn)(size_t size);
typedef void  (*gt_latex_free_fn)(void *ptr);
typedef void *(*gt_latex_realloc_fn)(void *ptr, size_t size);

/* ----------------------------------------------------------------
 * 接口声明 / API declarations
 * ---------------------------------------------------------------- */

/**
 * @brief 注册自定义内存分配函数 / Register custom memory allocator
 *        传入 NULL 则恢复为标准库 / Pass NULL to restore standard library
 * @param malloc_fn  自定义 malloc / Custom malloc
 * @param free_fn    自定义 free / Custom free
 * @param realloc_fn 自定义 realloc / Custom realloc
 */
void gt_latex_mem_register(gt_latex_malloc_fn  malloc_fn,
                           gt_latex_free_fn    free_fn,
                           gt_latex_realloc_fn realloc_fn);

/**
 * @brief 分配内存 / Allocate memory
 * @param size 字节数 / Size in bytes
 * @return     内存指针，失败返回 NULL / Pointer, NULL on failure
 */
void *gt_latex_mem_alloc(size_t size);

/**
 * @brief 释放内存 / Free memory
 * @param ptr 内存指针 / Pointer to free
 */
void gt_latex_mem_free(void *ptr);

/**
 * @brief 重新分配内存 / Reallocate memory
 * @param ptr  原内存指针 / Original pointer
 * @param size 新字节数 / New size in bytes
 * @return     新内存指针，失败返回 NULL / New pointer, NULL on failure
 */
void *gt_latex_mem_realloc(void *ptr, size_t size);

#endif /* GT_LATEX_ENABLE */

#endif /* _GT_LATEX_MEM_H_ */
