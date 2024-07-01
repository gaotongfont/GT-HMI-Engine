/**
 * @file gt_math.h
 * @author yongg
 * @brief Math library
 * @version 0.1
 * @date 2022-05-23 16:42:24
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_MATH_H_
#define _GT_MATH_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "math.h"
#include "stdint.h"


/* define ---------------------------------------------------------------*/

/**
 * @brief use default float calculate trigonometric value,
 *  otherwise calculate trigonometric value by sin table value.
 * ! [internal use only]
 */
#define _USE_TRIGON_FLOAT 0

/**
 * @brief calculate the angle return from the true value, such as:
 *  30 * sin(30) = 15 ==>
 *      14 = GT_MATH_GET_TRIGON_VAL(30 * gt_sin(30))    [approximate value]
 */
#define GT_MATH_GET_TRIGON_VAL(_value) (_value >> 15)


#define GT_UDIV255(x) (((x) * 0x8081U) >> 0x17)


#define GT_MIN(a,b) ((a) > (b) ? (b) : (a))
#define GT_MAX(a,b) ((a) > (b) ? (a) : (b))

#define GT_MATH_BEZIER_VAL_SHIFT        10  /* (1 << 10) = 1024 */
#define GT_MATH_BEZIER_VAL_RESOLUTION   (1 << GT_MATH_BEZIER_VAL_SHIFT)

/* typedef --------------------------------------------------------------*/
/**
 * @brief Bezier curve struct
 */
typedef struct gt_math_bezier_s {
    uint32_t t;
    uint32_t p0;
    uint32_t p1;
    uint32_t p2;
    uint32_t p3;
}gt_math_bezier_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

#if _USE_TRIGON_FLOAT
double gt_sin(double angle);
double gt_cos(double angle);
#else
int16_t gt_sin(int16_t angle);
int16_t gt_cos(int16_t angle);
#endif

double gt_square(double n);

int gt_abs(int n);

uint32_t gt_per_255(uint8_t n);

/**
 * @brief get the value of range [in_min, in_max] map onto [out_min, out_max]
 *
 * @param x current value of range [in_min, in_max]
 * @param in_min the input range min value
 * @param in_max the input range max value
 * @param out_min the output range min value which want to get value from it
 * @param out_max the output range max value which want to get value from it
 * @return int32_t the actual value between [out_min and out_max]
 */
int32_t gt_map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max);

uint32_t gt_bezier3(gt_math_bezier_st const * const bezier);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_MATH_H_
