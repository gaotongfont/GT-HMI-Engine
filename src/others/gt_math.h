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



/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

double gt_sin(double angle);
double gt_cos(double angle);
double gt_tan(double angle);

double gt_square(double n);

double gt_abs(double n);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_MATH_H_
