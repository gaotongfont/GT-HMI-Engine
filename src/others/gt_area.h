/**
 * @file gt_area.h
 * @author yongg
 * @brief display area interface
 * @version 0.1
 * @date 2022-08-11 09:34:53
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
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

bool gt_area_is_intersect_screen(const gt_area_st * screen, const gt_area_st * area_fore);

/**
 * @brief calc two areas intersect, the result is begin from area_fore offset value.
 *      such as:
 *          1. screen is (0, 0, 800, 480), area_fore is (10, 10, 100, 100), result: (0, 0, 100, 100)
 *          2. screen is (0, 0, 800, 480), area_fore is (0, -20, 100, 100), result: (0, 20, 100, 80)
 *      if two areas not intersect, the result is (0, 0, 0, 0)
 *
 * @param screen background area
 * @param area_fore foreground area
 * @param area_res area intersect point and size
 * @return true two areas intersect,
 *      false two areas not intersect
 */
bool gt_area_intersect_screen(const gt_area_st * screen, const gt_area_st * area_fore, gt_area_st * area_res);

/**
 * @brief The intersection of two regions: area1 and area2,
 *     the result is stored in area_res
 *
 * @param area1
 * @param area2
 * @param area_res
 * @return true Have intersection
 * @return false No intersection
 */
bool gt_area_cover_screen(gt_area_st const * const area1, gt_area_st const * const area2, gt_area_st * area_res);

/**
 * @brief The inclusion relation of two regions
 *
 * @param a1_p The pointer of area
 * @param a2_p The another pointer of area
 * @return true
 * @return false
 */
bool gt_area_is_on(gt_area_st * a1_p, gt_area_st * a2_p);
void gt_area_join(gt_area_st * res, const gt_area_st * const src, const gt_area_st * const dst);

bool gt_area_check_legal(gt_area_st * area);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_AREA_H_
