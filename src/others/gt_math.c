/**
 * @file gt_math.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-23 16:42:15
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_conf.h"
#include "gt_math.h"
#include "stdlib.h"

/* private define -------------------------------------------------------*/

#define _GT_MATH_GET_BEZIER_SHIFT_VALUE(_value)     ((_value) >> GT_MATH_BEZIER_VAL_SHIFT)

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

#if 1
static const uint32_t _gt_tab_per_255[] = {
    0, 129, 257, 386, 514, 643, 771, 900, 1028, 1157, 1285, 1414, 1542, 1671, 1799, 1928, 2056,
    2185, 2313, 2442, 2570, 2699, 2827, 2956, 3084, 3213, 3341, 3470, 3598, 3727, 3855, 3984, 4112,
    4241, 4369, 4498, 4626, 4755, 4883, 5012, 5140, 5269, 5397, 5526, 5654, 5783, 5911, 6040, 6168,
    6297, 6425, 6554, 6682, 6811, 6939, 7068, 7196, 7325, 7453, 7582, 7710, 7839, 7967, 8096, 8224,
    8353, 8481, 8610, 8738, 8867, 8995, 9124, 9252, 9381, 9509, 9638, 9766, 9895, 10023, 10152, 10280,
    10409, 10537, 10666, 10794, 10923, 11051, 11180, 11308, 11437, 11565, 11694, 11822, 11951, 12079, 12208, 12336,
    12465, 12593, 12722, 12850, 12979, 13107, 13236, 13364, 13493, 13621, 13750, 13878, 14007, 14135, 14264, 14392,
    14521, 14649, 14778, 14906, 15035, 15163, 15292, 15420, 15549, 15677, 15806, 15934, 16063, 16191, 16320, 16448,
    16577, 16705, 16834, 16962, 17091, 17219, 17348, 17476, 17605, 17733, 17862, 17990, 18119, 18247, 18376, 18504,
    18633, 18761, 18890, 19018, 19147, 19275, 19404, 19532, 19661, 19789, 19918, 20046, 20175, 20303, 20432, 20560,
    20689, 20817, 20946, 21074, 21203, 21331, 21460, 21588, 21717, 21845, 21974, 22102, 22231, 22359, 22488, 22616,
    22745, 22873, 23002, 23130, 23259, 23387, 23516, 23644, 23773, 23901, 24030, 24158, 24287, 24415, 24544, 24672,
    24801, 24929, 25058, 25186, 25315, 25443, 25572, 25700, 25829, 25957, 26086, 26214, 26343, 26471, 26600, 26728,
    26857, 26985, 27114, 27242, 27371, 27499, 27628, 27756, 27885, 28013, 28142, 28270, 28399, 28527, 28656, 28784,
    28913, 29041, 29170, 29298, 29427, 29555, 29684, 29812, 29941, 30069, 30198, 30326, 30455, 30583, 30712, 30840,
    30969, 31097, 31226, 31354, 31483, 31611, 31740, 31868, 31997, 32125, 32254, 32382, 32511, 32639, 32768,
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

int gt_abs(int n)
{
    return abs(n);
}

uint32_t gt_per_255(uint8_t n)
{
    return _gt_tab_per_255[n];
}

int32_t gt_map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max)
{
    if (in_max >= in_min) {
        if (x >= in_max) return out_max;
        if (x <= in_min) return out_min;
    }
    if (in_max <= in_min) {
        if (x >= in_min) return out_min;
        if (x <= in_max) return out_max;
    }

    int32_t in_delta  = in_max - in_min;
    int32_t out_delta = out_max - out_min;

    return ( ((x - in_min) * out_delta) / in_delta ) + out_min;
}

uint32_t gt_bezier3(gt_math_bezier_st const * const bezier)
{
    uint32_t t_res  = GT_MATH_BEZIER_VAL_RESOLUTION - bezier->t;
    uint32_t t_res2 = _GT_MATH_GET_BEZIER_SHIFT_VALUE(t_res * t_res);
    uint32_t t_res3 = _GT_MATH_GET_BEZIER_SHIFT_VALUE(t_res2 * t_res);
    uint32_t t2     = _GT_MATH_GET_BEZIER_SHIFT_VALUE(bezier->t * bezier->t);
    uint32_t t3     = _GT_MATH_GET_BEZIER_SHIFT_VALUE(t2 * bezier->t);

    uint32_t v1 = _GT_MATH_GET_BEZIER_SHIFT_VALUE(bezier->p0 * t_res3);
    uint32_t v2 = _GT_MATH_GET_BEZIER_SHIFT_VALUE(_GT_MATH_GET_BEZIER_SHIFT_VALUE(3 * bezier->p1 * t_res2));
    uint32_t v3 = _GT_MATH_GET_BEZIER_SHIFT_VALUE(_GT_MATH_GET_BEZIER_SHIFT_VALUE(3 * bezier->p2 * t2 * t_res));
    uint32_t v4 = _GT_MATH_GET_BEZIER_SHIFT_VALUE(bezier->p3 * t3);

    return v1 + v2 + v3 + v4;
}
/* end ------------------------------------------------------------------*/
