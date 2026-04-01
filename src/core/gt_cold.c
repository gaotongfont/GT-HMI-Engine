/**
 * @file gt_cold.c
 * @brief Cold data management implementation
 *
 * Lazy initialization - slabs are only created when first needed.
 *
 * @version 0.1
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_cold.h"
#include "gt_mem.h"
#include "../others/gt_log.h"
#include <string.h>

/* private define -------------------------------------------------------*/
#define GT_COLD_TAG   "COLD"

/* private typedef ------------------------------------------------------*/

/* static variables -----------------------------------------------------*/

/**
 * @brief Global cold data slabs - lazily initialized
 */
static gt_slab_st g_scroll_slab;
static gt_slab_st g_event_slab;
static bool g_scroll_slab_inited = false;
static bool g_event_slab_inited = false;

/* static functions -----------------------------------------------------*/

/**
 * @brief Get or create scroll slab lazily
 */
static gt_slab_st *_get_scroll_slab(void) {
    if (!g_scroll_slab_inited) {
        if (!gt_slab_init(&g_scroll_slab, sizeof(gt_scroll_cold_st),
                          GT_SCROLL_COLD_PAGE_OBJECTS, "scroll_cold")) {
            GT_LOGE(GT_COLD_TAG, "Failed to create scroll slab");
            return NULL;
        }
        g_scroll_slab_inited = true;
    }
    return &g_scroll_slab;
}

/**
 * @brief Get or create event slab lazily
 */
static gt_slab_st *_get_event_slab(void) {
    if (!g_event_slab_inited) {
        if (!gt_slab_init(&g_event_slab, sizeof(gt_event_list_st),
                          GT_EVENT_LIST_PAGE_OBJECTS, "event_list")) {
            GT_LOGE(GT_COLD_TAG, "Failed to create event slab");
            return NULL;
        }
        g_event_slab_inited = true;
    }
    return &g_event_slab;
}

/* global functions / API interface -------------------------------------*/

gt_scroll_cold_st *gt_cold_scroll_alloc(struct gt_obj_s *owner) {
    gt_slab_st *slab = _get_scroll_slab();
    if (slab == NULL) {
        return NULL;
    }

    gt_scroll_cold_st *cold = (gt_scroll_cold_st *)gt_slab_alloc(slab);
    if (cold) {
        memset(cold, 0, sizeof(gt_scroll_cold_st));
    }
    return cold;
}

void gt_cold_scroll_free(gt_scroll_cold_st *cold) {
    if (cold == NULL || !g_scroll_slab_inited) {
        return;
    }
    gt_slab_free(&g_scroll_slab, cold);
}

gt_event_list_st *gt_cold_event_alloc(struct gt_obj_s *owner) {
    gt_slab_st *slab = _get_event_slab();
    if (slab == NULL) {
        return NULL;
    }

    gt_event_list_st *node = (gt_event_list_st *)gt_slab_alloc(slab);
    if (node) {
        memset(node, 0, sizeof(gt_event_list_st));
    }
    return node;
}

void gt_cold_event_free(gt_event_list_st *node) {
    if (node == NULL || !g_event_slab_inited) {
        return;
    }
    gt_slab_free(&g_event_slab, node);
}

void gt_cold_get_stats(uint8_t *scroll_usage, uint8_t *event_usage) {
    if (scroll_usage) {
        *scroll_usage = g_scroll_slab_inited ? gt_slab_usage(&g_scroll_slab) : 0;
    }
    if (event_usage) {
        *event_usage = g_event_slab_inited ? gt_slab_usage(&g_event_slab) : 0;
    }
}
