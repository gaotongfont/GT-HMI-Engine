/**
 * @file gt_slab.c
 * @brief Dynamic Slab memory allocator implementation
 *
 * @version 0.1
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_slab.h"
#include "gt_mem.h"
#include "../others/gt_log.h"
#include <string.h>

/* private define -------------------------------------------------------*/
#define GT_SLAB_TAG   "SLAB"

/* static functions -----------------------------------------------------*/

/**
 * @brief Allocate a new page and add it to the slab
 */
static bool _slab_add_page(gt_slab_st *slab) {
    if (slab->page_count >= GT_SLAB_MAX_PAGES) {
        GT_LOGW(GT_SLAB_TAG, "Slab '%s' reached max pages (%d)",
                slab->name ? slab->name : "unknown", GT_SLAB_MAX_PAGES);
        return false;
    }

    // Allocate page descriptor
    gt_slab_page_st *page = (gt_slab_page_st *)gt_mem_malloc(sizeof(gt_slab_page_st));
    if (page == NULL) {
        GT_LOGW(GT_SLAB_TAG, "Failed to allocate page descriptor for slab '%s'",
                slab->name ? slab->name : "unknown");
        return false;
    }

    // Calculate memory needed for this page
    uint32_t page_size = (uint32_t)slab->obj_size * slab->page_objects;
    uint32_t total_size = page_size + (uint32_t)sizeof(uint16_t) * slab->page_objects;

    // Allocate page memory
    page->memory = (uint8_t *)gt_mem_malloc(total_size);
    if (page->memory == NULL) {
        GT_LOGW(GT_SLAB_TAG, "Failed to allocate %lu bytes for slab '%s' page",
                (unsigned long)page_size, slab->name ? slab->name : "unknown");
        gt_mem_free(page);
        return false;
    }

    // Initialize page
    page->obj_count = slab->page_objects;
    page->free_count = slab->page_objects;
    page->free_top = slab->page_objects;    /* stack top starts at end */
    page->free_indices = (uint16_t *)(page->memory + page_size);
    page->next = NULL;

    // Initialize free stack: push all indices so free_top decrements on alloc
    for (uint16_t i = 0; i < slab->page_objects; i++) {
        page->free_indices[i] = i;
    }

    // Add to slab's page list
    page->next = slab->pages;
    slab->pages = page;
    slab->page_count++;
    slab->total_capacity += slab->page_objects;
    slab->total_free += slab->page_objects;

    GT_LOGI(GT_SLAB_TAG, "Slab '%s' added page %d: %d objects, total capacity: %d",
            slab->name ? slab->name : "unknown", slab->page_count,
            slab->page_objects, slab->total_capacity);

    return true;
}

/**
 * @brief Find which page an object belongs to
 */
static gt_slab_page_st *_slab_find_page(gt_slab_st *slab, void *obj) {
    uint8_t *obj_ptr = (uint8_t *)obj;
    gt_slab_page_st *page = slab->pages;

    while (page) {
        uint8_t *page_start = page->memory;
        uint8_t *page_end = page_start + (uint32_t)slab->obj_size * page->obj_count;

        if (obj_ptr >= page_start && obj_ptr < page_end) {
            return page;
        }
        page = page->next;
    }

    return NULL;
}

/* global functions / API interface -------------------------------------*/

bool gt_slab_init(gt_slab_st *slab, uint16_t obj_size, uint16_t page_objects, const char *name) {
    if (slab == NULL || obj_size == 0) {
        GT_LOGW(GT_SLAB_TAG, "Invalid parameters: slab=%p, obj_size=%d",
                slab, obj_size);
        return false;
    }

    // Use defaults if not specified
    if (page_objects == 0) {
        page_objects = GT_SLAB_PAGE_OBJECTS;
    }

    // Align object size
    uint16_t aligned_size = GT_SLAB_ALIGN_SIZE(obj_size);

    // Initialize slab structure
    memset(slab, 0, sizeof(gt_slab_st));
    slab->obj_size = aligned_size;
    slab->page_objects = page_objects;
    slab->name = name;
    slab->pages = NULL;
    slab->page_count = 0;
    slab->total_capacity = 0;
    slab->total_free = 0;

#if GT_CFG_ENABLE_ASSERT
    slab->initialized = true;
#endif

    // Allocate initial page
    if (!_slab_add_page(slab)) {
        memset(slab, 0, sizeof(gt_slab_st));
        return false;
    }

    GT_LOGI(GT_SLAB_TAG, "Slab '%s' initialized: obj_size=%d, page_objects=%d",
            name ? name : "unknown", aligned_size, page_objects);

    return true;
}

void gt_slab_deinit(gt_slab_st *slab) {
    if (slab == NULL) {
        return;
    }

    GT_LOGI(GT_SLAB_TAG, "Slab '%s' deinitialized: pages=%d, peak usage=%d%%",
            slab->name ? slab->name : "unknown",
            slab->page_count, gt_slab_usage(slab));

    // Free all pages
    gt_slab_page_st *page = slab->pages;
    while (page) {
        gt_slab_page_st *next = page->next;
        if (page->memory) {
            gt_mem_free(page->memory);
        }
        gt_mem_free(page);
        page = next;
    }

    memset(slab, 0, sizeof(gt_slab_st));

#if GT_CFG_ENABLE_ASSERT
    slab->initialized = false;
#endif
}

void *gt_slab_alloc(gt_slab_st *slab) {
#if GT_CFG_ENABLE_ASSERT
    if (!slab->initialized) {
        GT_LOGW(GT_SLAB_TAG, "Alloc from uninitialized slab");
        return NULL;
    }
#endif

    // Try to find a page with free objects
    gt_slab_page_st *page = slab->pages;
    while (page && page->free_count == 0) {
        page = page->next;
    }

    // If no page has free objects, try to add a new page
    if (page == NULL) {
        if (!_slab_add_page(slab)) {
            GT_LOGW(GT_SLAB_TAG, "Slab '%s' allocation failed: out of memory",
                    slab->name ? slab->name : "unknown");
            return NULL;
        }
        page = slab->pages;
    }

    // Pop from free stack: decrement first, then read
    --page->free_top;
    uint16_t idx = page->free_indices[page->free_top];
    page->free_indices[page->free_top] = 0xFFFF;    /* invalidate to prevent false double-free detection */
    page->free_count--;
    slab->total_free--;

    // Calculate object address
    void *obj = page->memory + (uint32_t)idx * slab->obj_size;

    return obj;
}

bool gt_slab_free(gt_slab_st *slab, void *obj) {
#if GT_CFG_ENABLE_ASSERT
    if (!slab->initialized) {
        GT_LOGW(GT_SLAB_TAG, "Free to uninitialized slab");
        return false;
    }
#endif

    if (obj == NULL) {
        return false;
    }

    // Find which page this object belongs to
    gt_slab_page_st *page = _slab_find_page(slab, obj);
    if (page == NULL) {
        GT_LOGW(GT_SLAB_TAG, "Object %p not in slab '%s'",
                obj, slab->name ? slab->name : "unknown");
        return false;
    }

    // Calculate object index within page
    uint8_t *page_mem_start = page->memory;
    uint16_t idx = (uint16_t)((uint8_t *)obj - page_mem_start) / slab->obj_size;

    // Check if already in free list (double free)
    for (uint16_t i = page->free_top; i < page->obj_count; i++) {
        if (page->free_indices[i] == idx) {
            GT_LOGW(GT_SLAB_TAG, "Double free detected at index %d in slab '%s'",
                    idx, slab->name ? slab->name : "unknown");
            return false;
        }
    }

    // Push to free stack: write then increment
    page->free_indices[page->free_top++] = idx;
    page->free_count++;
    slab->total_free++;

    return true;
}
