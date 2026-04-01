/**
 * @file gt_cold.h
 * @brief Cold data management for GT-HMI-Engine objects
 *
 * Cold data refers to object properties that are accessed infrequently
 * compared to hot data (area, position, visible state, etc.).
 *
 * By separating hot and cold data:
 * 1. Hot data fits in CPU cache lines better
 * 2. Cold data can be allocated lazily or from separate slabs
 * 3. Non-scrolling objects don't pay for scroll-related memory
 *
 * @version 0.1
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_COLD_H_
#define _GT_COLD_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_slab.h"
#include "gt_scroll.h"
#include "../core/gt_event.h"

/* forward declarations */
struct gt_obj_s;  /* Uses gt_obj_st from gt_obj.h */

/* define ---------------------------------------------------------------*/

/**
 * @brief Default objects per page for scroll cold data slab
 *        Adjust based on expected max scrolling widgets per screen
 */
#define GT_SCROLL_COLD_PAGE_OBJECTS   64

/**
 * @brief Default objects per page for event list slab
 *        Adjust based on expected max event callbacks per screen
 */
#define GT_EVENT_LIST_PAGE_OBJECTS    128

/* macros ---------------------------------------------------------------*/

/* typedef --------------------------------------------------------------*/

/**
 * @brief Event callback list node
 *
 * Linked list node for event callbacks.
 * Allocated from slab when event is registered.
 *
 * Size: 24 bytes (aligned)
 */
typedef struct gt_event_list_s {
    struct gt_event_list_s *next;  /**< Next node in list */
    gt_event_cb_t user_cb;          /**< Callback function */
    void *user_data;                /**< User data passed to callback */
    gt_event_type_et filter;       /**< Event type filter */
} gt_event_list_st;

/**
 * @brief Global slabs for cold data
 *
 * These slabs are initialized once and manage all cold data
 * allocations across all screens.
 */
typedef struct gt_cold_slabs_s {
    gt_slab_st scroll;              /**< Slab for scroll cold data */
    gt_slab_st events;              /**< Slab for event list nodes */
} gt_cold_slabs_st;

/* global functions / API interface -------------------------------------*/

/**
 * @brief Allocate scroll cold data for an object
 *
 * Lazy allocation - slab is created on first call.
 *
 * @param owner     Owner object (for debugging)
 * @return gt_scroll_cold_st*  Pointer to allocated cold data, or NULL if failed
 */
gt_scroll_cold_st *gt_cold_scroll_alloc(struct gt_obj_s * owner);

/**
 * @brief Free scroll cold data
 *
 * @param cold      Scroll cold data to free
 */
void gt_cold_scroll_free(gt_scroll_cold_st *cold);

/**
 * @brief Allocate event list node
 *
 * Lazy allocation - slab is created on first call.
 *
 * @param owner     Owner object (for debugging)
 * @return gt_event_list_st*  Pointer to allocated node, or NULL if failed
 */
gt_event_list_st *gt_cold_event_alloc(struct gt_obj_s *owner);

/**
 * @brief Free event list node
 *
 * @param node      Event node to free
 */
void gt_cold_event_free(gt_event_list_st *node);

/**
 * @brief Get cold data slab statistics
 *
 * @param scroll_usage     Output: scroll slab usage percentage (can be NULL)
 * @param event_usage      Output: event slab usage percentage (can be NULL)
 */
void gt_cold_get_stats(uint8_t *scroll_usage, uint8_t *event_usage);

#ifdef __cplusplus
}
#endif

#endif /* _GT_COLD_H_ */
