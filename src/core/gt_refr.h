/**
 * @file gt_refr.h
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-08-12 10:05:04
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_REFR_H_
#define _GT_REFR_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_draw.h"


/* define ---------------------------------------------------------------*/
#ifndef GT_REFR_AREA_ALIGN_HOR
    /**
     * @brief Refresh area resize to alignment horizontal, x or width will be
     *      aligned to the multiple of GT_REFR_AREA_ALIGN_HOR_PIXEL
     */
    #define GT_REFR_AREA_ALIGN_HOR          0
#endif

#if GT_REFR_AREA_ALIGN_HOR
    /**
     * @brief The pixel value for horizontal alignment
     *      such as: x: 3, w: 8, GT_REFR_AREA_ALIGN_HOR_PIXEL: 4
     *      result: x will be 0, w will be 12 = (3 + 4 + 8) / 4 * 4.
     */
    #define GT_REFR_AREA_ALIGN_HOR_PIXEL    0
#endif

#ifndef GT_REFR_AREA_ALIGN_VER
    /**
     * @brief Refresh area resize to alignment vertical, y or height will be
     *     aligned to the multiple of GT_REFR_AREA_ALIGN_VER_PIXEL
     */
    #define GT_REFR_AREA_ALIGN_VER          0
#endif

#if GT_REFR_AREA_ALIGN_VER
    /**
     * @brief The pixel value for vertical alignment
     *      such as: y: 3, h: 8, GT_REFR_AREA_ALIGN_VER_PIXEL: 4
     *      result: y will be 0, h will be 12 = (3 + 4 + 8) / 4 * 4.
     */
    #define GT_REFR_AREA_ALIGN_VER_PIXEL    0
#endif


/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief Align a refresh area to the configured pixel boundaries in-place.
 *        Expands x/y toward the lower boundary and w/h toward the upper
 *        boundary so both edges are multiples of the alignment pixel value.
 *        No-op when both GT_REFR_AREA_ALIGN_HOR and GT_REFR_AREA_ALIGN_VER
 *        are 0 (the function body is empty and optimized away).
 *
 * @param area Area to align (modified in-place)
 */
static inline void gt_refr_area_align(gt_area_st * area) {
#if GT_REFR_AREA_ALIGN_HOR || GT_REFR_AREA_ALIGN_VER
    uint16_t align_val = 0;
#if GT_REFR_AREA_ALIGN_HOR
    align_val = (uint16_t)gt_abs(area->x % GT_REFR_AREA_ALIGN_HOR_PIXEL);
    if (align_val) {
        area->x -= align_val;
        area->w += align_val;
    }
    /* w must also be aligned up independently, regardless of x alignment */
    align_val = area->w % GT_REFR_AREA_ALIGN_HOR_PIXEL;
    if (align_val) {
        area->w += GT_REFR_AREA_ALIGN_HOR_PIXEL - align_val;
    }
#endif
#if GT_REFR_AREA_ALIGN_VER
    align_val = (uint16_t)gt_abs(area->y % GT_REFR_AREA_ALIGN_VER_PIXEL);
    if (align_val) {
        area->y -= align_val;
        area->h += align_val;
    }
    /* h must also be aligned up independently, regardless of y alignment */
    align_val = area->h % GT_REFR_AREA_ALIGN_VER_PIXEL;
    if (align_val) {
        area->h += GT_REFR_AREA_ALIGN_VER_PIXEL - align_val;
    }
#endif
    (void)align_val;
#else
    (void)area;
#endif
}

void gt_refr_timer(void);

/**
 * @brief Force flush the screen
 *      [Warn] Thread Unsafe, should load screen init before flush
 */
void gt_refr_force_flush(void);

/**
 * @brief Force flush the screen
 *      [Warn] Thread Safe, should load screen init before flush
 * @param timeout
 * @return true ok
 */
gt_res_t gt_refr_force_flush_thread_safe(uint32_t timeout);

/**
 * @brief Set refresh enabled or not
 *      [WARN] Dangerous behavior!
 *
 * @param enabled [default: 1]
 */
void gt_refr_set_enabled(bool enabled);

/**
 * @brief ignore the refresh area check and redraw
 *
 * @param disp NULL: Using default
 * @return true [default]
 * @return false
 */
bool gt_refr_is_enabled(gt_disp_st * disp);


#ifdef __cplusplus
} /*extern "C"*/

#endif

#endif //!_GT_REFR_H_
