/**
 * @file gt_scroll.h
 * @brief Scroll-related cold data definitions
 *
 * This file contains the scroll cold data structure that is
 * separated from the main object to improve cache efficiency.
 *
 * @version 0.1
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SCROLL_H_
#define _GT_SCROLL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../others/gt_types.h"

/* define ---------------------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* typedef --------------------------------------------------------------*/

/**
 * @brief Scroll-related cold data structure
 *
 * Contains scroll state that is not accessed on every frame.
 * Allocated from dedicated slab when scroll feature is used.
 *
 * This structure is 16 bytes aligned for efficient slab allocation.
 *
 * Size: 16 bytes
 */
typedef struct _gt_scroll_cold_s {
    gt_point_st scroll_prev;        /**< Previous scroll position for delta calculation */
    uint8_t is_scrolling : 1;      /**< Scroll animation in progress */
    uint8_t _reserved : 7;         /**< Padding for alignment */
    uint8_t _pad[5];                /**< Explicit padding to 16 bytes */
} gt_scroll_cold_st;

/* global functions / API interface -------------------------------------*/

/* No public API - this file only defines the structure */
/* Allocation/deallocation is handled by gt_cold module */

#ifdef __cplusplus
}
#endif

#endif /* _GT_SCROLL_H_ */
