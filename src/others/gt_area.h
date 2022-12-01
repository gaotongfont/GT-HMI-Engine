/**
 * @file gt_area.h
 * @author yongg
 * @brief display area interface
 * @version 0.1
 * @date 2022-08-11 09:34:53
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_AREA_H_
#define _GT_AREA_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"
#include "stdbool.h"
#include "gt_types.h"
#include "gt_math.h"


/* define ---------------------------------------------------------------*/
#define AREA_X1(area)   (area->x)
#define AREA_X2(area)   (area->x + area->w)
#define AREA_Y1(area)   (area->y)
#define AREA_Y2(area)   (area->y + area->h)
#define AREA_W(area)   (area->w)
#define AREA_H(area)   (area->h)


/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief calc two areas intersect
 *
 * @param area_back background area
 * @param area_fore foreground area
 * @param area_res area intersect point and size
 * @return true two areas intersect,
 *      false two areas not intersect
 */
bool gt_area_intersect_screen(const gt_area_st * area_back, const gt_area_st * area_fore, gt_area_st * area_res);

bool gt_area_is_on(gt_area_st * a1_p, gt_area_st * a2_p);
void gt_area_join(gt_area_st * res, const gt_area_st * const src, const gt_area_st * const dst);

void gt_area_printf(gt_area_st area);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_AREA_H_
