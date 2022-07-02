/**
 * @file gt_math.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-23 16:42:15
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_math.h"
#include "stdlib.h"

/* private define -------------------------------------------------------*/
#define __GT_ARCH_VAL  0.0174532925    //==PI/180


/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
double gt_sin(double angle){
    return sin( angle * __GT_ARCH_VAL );
}
double gt_cos(double angle){
    return 0;
}
double gt_tan(double angle){
    return 0;
}

double gt_square(double n){
    return pow(n,2);
}

double gt_abs(double n){
    return abs(n);
}

/* end ------------------------------------------------------------------*/
