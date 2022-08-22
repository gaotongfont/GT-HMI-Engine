/**
 * @file gt_math.h
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-23 16:42:24
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
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

/* typedef --------------------------------------------------------------*/



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

double gt_abs(double n);




#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_MATH_H_
