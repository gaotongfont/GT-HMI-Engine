/**
 * @file gt_draw_blend_with_rgb565.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-06-04 21:31:27
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_DRAW_BLEND_WITH_RGB565_H_
#define _GT_DRAW_BLEND_WITH_RGB565_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "./gt_draw_blend.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_draw_blend_with_rgb565_opacity(_gt_draw_blend_fill_cache_st const * const fc);

void gt_draw_blend_with_rgb565_no_opacity(_gt_draw_blend_fill_cache_st const * const fc);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_DRAW_BLEND_WITH_RGB565_H_
