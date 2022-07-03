/**
 * @file gt_math.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-23 16:42:15
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_conf.h"
#include "gt_math.h"
#include "stdlib.h"

/* private define -------------------------------------------------------*/

#ifdef _USE_FLOAT
#define __GT_ARCH_VAL  0.0174532925    //==PI/180
#else
static const GT_ATTRIBUTE_LARGE_RAM_ARRAY int16_t _gt_sin_90_table[] = {
    0,     572,   1144,  1715,  2286,  2856,  3425,  3993,  4560,  5126,  5690,  6252,  6813,  7371,  7927,  8481,
    9032,  9580,  10126, 10668, 11207, 11743, 12275, 12803, 13328, 13848, 14364, 14876, 15383, 15886, 16383, 16876,
    17364, 17846, 18323, 18794, 19260, 19720, 20173, 20621, 21062, 21497, 21925, 22347, 22762, 23170, 23571, 23964,
    24351, 24730, 25101, 25465, 25821, 26169, 26509, 26841, 27165, 27481, 27788, 28087, 28377, 28659, 28932, 29196,
    29451, 29697, 29934, 30162, 30381, 30591, 30791, 30982, 31163, 31335, 31498, 31650, 31794, 31927, 32051, 32165,
    32269, 32364, 32448, 32523, 32587, 32642, 32687, 32722, 32747, 32762, 32767
};
#endif


/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
#if _USE_TRIGON_FLOAT
double gt_sin(double angle)
{
    return sin( angle * __GT_ARCH_VAL );
}

double gt_cos(double angle)
{
    return 0;
}

#else
/**
 * @brief return with sin of an angle, such as:
 *  sin[90] = 32767, sin[-90] = -32767
 *
 * @param angle degrees of angle
 * @return int16_t get value 1 degrees of 32767
 */
int16_t gt_sin(int16_t angle)
{
    int16_t ret = 0;

    angle = angle % 360;

    if (angle < 0) {
        angle += 360;
    }

    if (angle < 90) {
        ret = _gt_sin_90_table[angle];
    }
    else if (angle < 180) {
        angle = 180 - angle;
        ret = _gt_sin_90_table[angle];
    }
    else if (angle < 270) {
        angle -= 180;
        ret = -_gt_sin_90_table[angle];
    }
    else {
        /* >= 270 */
        angle = 360 - angle;
        ret = -_gt_sin_90_table[angle];
    }

    return ret;
}

/**
 * @brief return with cos of an angle, such as:
 *  sin[0] = 32767, sin[180] = -32767
 *
 * @param angle degrees of angle
 * @return int16_t get value 1 degrees of 32767
 */
int16_t gt_cos(int16_t angle)
{
    return gt_sin(angle + 90);
}

#endif

double gt_square(double n)
{
    return pow(n,2);
}

double gt_abs(double n)
{
    return abs(n);
}

/* end ------------------------------------------------------------------*/
