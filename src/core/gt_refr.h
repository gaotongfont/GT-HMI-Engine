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
void gt_refr_timer(void);



#ifdef __cplusplus
} /*extern "C"*/

#endif

#endif //!_GT_REFR_H_
