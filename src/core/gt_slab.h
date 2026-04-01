/**
 * @file gt_slab.h
 * @brief Dynamic Slab memory allocator for object pooling
 *
 * Slab allocator provides O(1) allocation/deallocation by managing
 * objects from pre-allocated memory pages. When a page is full, it
 * dynamically allocates additional pages.
 *
 * Features:
 * - O(1) alloc/free (amortized)
 * - Dynamic growth - no fixed capacity limit
 * - Memory from system heap (via gt_mem)
 * - No fragmentation
 *
 * @note This avoids heap fragmentation which is critical for
 *       resource-constrained embedded systems.
 */
#ifndef _GT_SLAB_H_
#define _GT_SLAB_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../others/gt_types.h"

/* define ---------------------------------------------------------------*/

/**
 * @brief Default alignment for slab objects
 */
#define GT_SLAB_ALIGNMENT   8

/**
 * @brief Default page size (in objects)
 *        A page of 64 objects = 1KB for 16-byte objects
 */
#define GT_SLAB_PAGE_OBJECTS    64

/**
 * @brief Default number of pages to pre-allocate
 */
#define GT_SLAB_DEFAULT_PAGES   1

/**
 * @brief Maximum pages per slab (limits max capacity to 64*256 = 16384 objects)
 */
#define GT_SLAB_MAX_PAGES       256

/* macros ---------------------------------------------------------------*/

/**
 * @brief Align size to specified alignment
 */
#define GT_SLAB_ALIGN_SIZE(size)  (((size) + (GT_SLAB_ALIGNMENT - 1)) & ~(GT_SLAB_ALIGNMENT - 1))

/* typedef --------------------------------------------------------------*/

/**
 * @brief Memory page descriptor
 *
 * Each page contains a fixed number of objects.
 */
typedef struct gt_slab_page_s {
    uint8_t *memory;               /**< Page memory start */
    uint16_t obj_count;           /**< Number of objects in this page */
    uint16_t free_count;           /**< Free objects in this page */
    uint16_t *free_indices;        /**< Stack of free object indices */
    uint16_t free_top;             /**< Stack top */
    struct gt_slab_page_s *next;   /**< Next page in list */
} gt_slab_page_st;

/**
 * @brief Dynamic Slab allocator instance
 *
 * Manages dynamically growing set of memory pages containing objects
 * of the same size. Uses a free-stack per page for O(1) alloc/free.
 */
typedef struct gt_slab_s {
    uint16_t obj_size;             /**< Aligned object size in bytes */
    uint16_t page_objects;         /**< Objects per page */
    const char *name;              /**< Debug name for this slab */

    gt_slab_page_st *pages;        /**< List of memory pages */
    uint16_t page_count;           /**< Number of pages allocated */
    uint16_t total_capacity;       /**< Total objects capacity */
    uint16_t total_free;           /**< Total free objects */

#if GT_CFG_ENABLE_ASSERT
    bool initialized;              /**< Whether this slab is initialized */
#endif
} gt_slab_st;

/* global functions / API interface -------------------------------------*/

/**
 * @brief Initialize a slab allocator
 *
 * Called internally or explicitly. First allocation will auto-initialize
 * if not yet done.
 *
 * @param slab          Pointer to slab structure
 * @param obj_size      Size of each object (will be aligned)
 * @param page_objects  Number of objects per page
 * @param name          Debug name (can be NULL)
 * @return true         Success
 * @return false        Failed
 */
bool gt_slab_init(gt_slab_st *slab, uint16_t obj_size, uint16_t page_objects, const char *name);

/**
 * @brief Deinitialize a slab allocator
 *
 * Frees all memory pages.
 *
 * @param slab      Pointer to slab structure
 */
void gt_slab_deinit(gt_slab_st *slab);

/**
 * @brief Allocate an object from slab
 *
 * O(1) amortized - may trigger page allocation if current pages are full.
 *
 * @param slab      Pointer to slab structure
 * @return void*    Pointer to allocated object, or NULL if failed
 */
void *gt_slab_alloc(gt_slab_st *slab);

/**
 * @brief Free an object back to slab
 *
 * O(1) - pushes the object index back to the page's free stack.
 *
 * @param slab      Pointer to slab structure
 * @param obj       Object pointer to free
 * @return true     Success
 * @return false    Failed (object not from this slab)
 */
bool gt_slab_free(gt_slab_st *slab, void *obj);

/**
 * @brief Check if slab has any free objects
 *
 * @param slab      Pointer to slab structure
 * @return true     Has free objects available
 * @return false    No free objects (would need to allocate new page)
 */
static inline bool gt_slab_has_free(const gt_slab_st *slab) {
    return slab->total_free > 0;
}

/**
 * @brief Get total capacity (for debugging)
 *
 * @param slab      Pointer to slab structure
 * @return uint16_t Total number of objects that can be stored
 */
static inline uint16_t gt_slab_capacity(const gt_slab_st *slab) {
    return slab->total_capacity;
}

/**
 * @brief Get current usage (for debugging)
 *
 * @param slab      Pointer to slab structure
 * @return uint16_t Number of allocated objects
 */
static inline uint16_t gt_slab_used(const gt_slab_st *slab) {
    return slab->total_capacity - slab->total_free;
}

/**
 * @brief Get utilization percentage
 *
 * @param slab      Pointer to slab structure
 * @return uint8_t  Usage percentage (0-100)
 */
static inline uint8_t gt_slab_usage(const gt_slab_st *slab) {
    if (slab->total_capacity == 0) return 0;
    return (uint8_t)((slab->total_capacity - slab->total_free) * 100 / slab->total_capacity);
}

/**
 * @brief Get number of pages (for debugging)
 *
 * @param slab      Pointer to slab structure
 * @return uint16_t Number of memory pages allocated
 */
static inline uint16_t gt_slab_page_count(const gt_slab_st *slab) {
    return slab->page_count;
}

#ifdef __cplusplus
}
#endif

#endif /* _GT_SLAB_H_ */
