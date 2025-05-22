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
static const GT_ATTRIBUTE_LARGE_RAM_ARRAY uint32_t _gt_tab_per_255[] = {
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
static volatile uint32_t _random_seed = 0x13579BDF;


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

double gt_atan2(double x, double y)
{
    return atan2(x, y);
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

uint16_t gt_atan2(int x, int y)
{
    unsigned char negflag;
    unsigned char tempdegree;
    unsigned char comp;
    unsigned int degree;     // this will hold the result
    unsigned int ux;
    unsigned int uy;

    // Save the sign flags then remove signs and get XY as unsigned ints
    negflag = 0;
    if(x < 0) {
        negflag += 0x01;    // x flag bit
        x = (0 - x);        // is now +
    }
    ux = x;                // copy to unsigned var before multiply
    if(y < 0) {
        negflag += 0x02;    // y flag bit
        y = (0 - y);        // is now +
    }
    uy = y;                // copy to unsigned var before multiply

    // 1. Calc the scaled "degrees"
    if(ux > uy) {
        degree = (uy * 45) / ux;   // degree result will be 0-45 range
        negflag += 0x10;    // octant flag bit
    }
    else {
        degree = (ux * 45) / uy;   // degree result will be 0-45 range
    }

    // 2. Compensate for the 4 degree error curve
    comp = 0;
    tempdegree = degree;    // use an unsigned char for speed!
    if(tempdegree > 22) {    // if top half of range
        if(tempdegree <= 44) comp++;
        if(tempdegree <= 41) comp++;
        if(tempdegree <= 37) comp++;
        if(tempdegree <= 32) comp++;  // max is 4 degrees compensated
    }
    else {   // else is lower half of range
        if(tempdegree >= 2) comp++;
        if(tempdegree >= 6) comp++;
        if(tempdegree >= 10) comp++;
        if(tempdegree >= 15) comp++;  // max is 4 degrees compensated
    }
    degree += comp;   // degree is now accurate to +/- 1 degree!

    // Invert degree if it was X>Y octant, makes 0-45 into 90-45
    if(negflag & 0x10) degree = (90 - degree);

    // 3. Degree is now 0-90 range for this quadrant,
    // need to invert it for whichever quadrant it was in
    if(negflag & 0x02) { // if -Y
        if(negflag & 0x01)   // if -Y -X
            degree = (180 + degree);
        else        // else is -Y +X
            degree = (180 - degree);
    }
    else {   // else is +Y
        if(negflag & 0x01)   // if +Y -X
            degree = (360 - degree);
    }
    return degree;
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

long gt_round(double val)
{
    return (long)(val + 0.5);
}

uint32_t gt_per_255(uint8_t n)
{
    return _gt_tab_per_255[n];
}

static int32_t do_cubic_bezier(int32_t t, int32_t a, int32_t b, int32_t c) {
    /*a * t^3 + b * t^2 + c * t*/
#if GT_MATH_CUBIC_BITS > 14
    int64_t ret;
#else
    int32_t ret;
#endif

    ret = a;
    ret = (ret * t) >> GT_MATH_CUBIC_BITS;
    ret = ((ret + b) * t) >> GT_MATH_CUBIC_BITS;
    ret = ((ret + c) * t) >> GT_MATH_CUBIC_BITS;
    return ret;
}

int32_t gt_cubic_bezier(int32_t x, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    int32_t ax, bx, cx, ay, by, cy;
    int32_t tl, tr, t;  /*t in cubic-bezier function, used for bisection */
    int32_t xs;  /*x sampled on curve */
#if GT_MATH_CUBIC_BITS > 14
    int64_t d; /*slope value at specified t*/
#else
    int32_t d;
#endif

    if(x == 0 || x == GT_MATH_BEZIER_VAL_RESOLUTION) return x;

    /* input is always GT_MATH_BEZIER_VAL_SHIFT bit precision */

#if GT_MATH_CUBIC_BITS != GT_MATH_BEZIER_VAL_SHIFT
    x <<= GT_MATH_CUBIC_BITS - GT_MATH_BEZIER_VAL_SHIFT;
    x1 <<= GT_MATH_CUBIC_BITS - GT_MATH_BEZIER_VAL_SHIFT;
    x2 <<= GT_MATH_CUBIC_BITS - GT_MATH_BEZIER_VAL_SHIFT;
    y1 <<= GT_MATH_CUBIC_BITS - GT_MATH_BEZIER_VAL_SHIFT;
    y2 <<= GT_MATH_CUBIC_BITS - GT_MATH_BEZIER_VAL_SHIFT;
#endif

    cx = 3 * x1;
    bx = 3 * (x2 - x1) - cx;
    ax = (1L << GT_MATH_CUBIC_BITS) - cx - bx;

    cy = 3 * y1;
    by = 3 * (y2 - y1) - cy;
    ay = (1L << GT_MATH_CUBIC_BITS)  - cy - by;

    /*Try Newton's method firstly */
    t = x; /*Make a guess*/
    for(int i = 0; i < GT_MATH_CUBIC_NEWTON_ITER; i++) {
        /*Check if x on curve at t matches input x*/
        xs = do_cubic_bezier(t, ax, bx, cx) - x;
        if(gt_abs(xs) <= 1) goto found;

        /* get slop at t, d = 3 * ax * t^2 + 2 * bx + t + cx */
        d = ax; /* use 64bit operation if needed. */
        d = (3 * d * t) >> GT_MATH_CUBIC_BITS;
        d = ((d + 2 * bx) * t) >> GT_MATH_CUBIC_BITS;
        d += cx;

        if(gt_abs(d) <= 1) break;

        d = ((int64_t)xs * (1L << GT_MATH_CUBIC_BITS)) / d;
        if(d == 0) break;  /*Reached precision limits*/
        t -= d;
    }

    /*Fallback to bisection method for reliability*/
    tl = 0, tr = 1L << GT_MATH_CUBIC_BITS, t = x;

    if(t < tl) {
        t = tl;
        goto found;
    }

    if(t > tr) {
        t = tr;
        goto found;
    }

    while(tl < tr) {
        xs = do_cubic_bezier(t, ax, bx, cx);
        if(gt_abs(xs - x) <= 1) goto found;
        x > xs ? (tl = t) : (tr = t);
        t = (tr - tl) / 2 + tl;
        if(t == tl) break;
    }

    /*Failed to find suitable t for given x, return a value anyway.*/
found:
    /*Return y at t*/
#if GT_MATH_CUBIC_BITS != GT_MATH_BEZIER_VAL_SHIFT
    return do_cubic_bezier(t, ay, by, cy) >> (GT_MATH_CUBIC_BITS - GT_MATH_BEZIER_VAL_SHIFT);
#else
    return do_cubic_bezier(t, ay, by, cy);
#endif
}

gt_res_t
gt_math_line_get(gt_math_line_st * line,
    gt_point_f_st const * const p1, gt_point_f_st const * const p2)
{
    /* 提取两点坐标 */
    const float x1 = p1->x;
    const float y1 = p1->y;
    const float x2 = p2->x;
    const float y2 = p2->y;

    /* 计算方向向量分量 */
    const float dx = x2 - x1;
    const float dy = y2 - y1;

    /* 检查两点是否重合 */
    if (fabsf(dx) < 1e-6f && fabsf(dy) < 1e-6f) {
        return GT_RES_INV;  // 无效输入，两点重合
    }

    /* 计算一般式系数 */
    line->a = dy;          // A = y2 - y1
    line->b = -dx;         // B = x1 - x2
    line->c = dx*y1 - dy*x1;  // C = x2y1 - x1y2

    /* 优化系数符号（可选）*/
    if (line->a < 0 || (line->a == 0 && line->b < 0)) {
        line->a = -line->a;
        line->b = -line->b;
        line->c = -line->c;
    }

    return GT_RES_OK;
}

gt_res_t
gt_math_line_get_perpendicular(gt_math_line_st *line,
    gt_point_f_st const * const perpendicular_point,
    gt_point_f_st const * const p1, gt_point_f_st const * const p2)
{
    /* 计算原直线方向向量 */
    const float dx = p2->x - p1->x;
    const float dy = p2->y - p1->y;

    /* 检查原直线有效性 */
    if (fabsf(dx) < 1e-6f && fabsf(dy) < 1e-6f) {
        return GT_RES_INV; // 两点重合，无法构成直线
    }

    /* 垂线系数：A=dx, B=dy */
    float A = dx;
    float B = dy;
    float C = -(A * perpendicular_point->x + B * perpendicular_point->y);

    /* 符号规范化（保证A≥0，且当A=0时B≥0）*/
    if (A < 0.0f || (A == 0.0f && B < 0.0f)) {
        A = -A;
        B = -B;
        C = -C;
    }

    /* 存储结果 */
    line->a = A;
    line->b = B;
    line->c = C;

    return GT_RES_OK;
}

gt_res_t
gt_math_line_circle_intersect(
    gt_point_f_st * ret_p1, gt_point_f_st * ret_p2,
    const gt_math_line_st * line, const gt_math_circle_st * circle)
{
    /* 从直线结构体获取参数 */
    float A = line->a;
    float B = line->b;
    float C = line->c;

    /* 从圆结构体获取参数 */
    float cx = circle->center.x;
    float cy = circle->center.y;
    float r = circle->radius;

    /* 检查直线有效性（A和B不能同时为0）*/
    if (fabsf(A) < 1e-6f && fabsf(B) < 1e-6f) {
        return GT_RES_INV; // 无效直线
    }

    /* 计算圆心到直线的距离 */
    float numerator = A * cx + B * cy + C;
    float denom_sq = A * A + B * B;
    float denom = sqrtf(denom_sq);
    float d = fabsf(numerator) / denom;

    /* 无交点 */
    if (d > r + 1e-6f) { // 考虑浮点误差
        return GT_RES_FAIL;
    }

    /* 计算垂足点（直线离圆心最近的点）*/
    float factor = numerator / denom_sq;
    float x0 = cx - A * factor;
    float y0 = cy - B * factor;

    /* 相切情况（填充两个相同点）*/
    if (fabsf(d - r) < 1e-6f) {
        ret_p1->x = ret_p2->x = x0;
        ret_p1->y = ret_p2->y = y0;
        return GT_RES_OK;
    }

    /* 两个交点情况 */
    float t = sqrtf(r*r - d*d) / denom;

    /* 沿直线方向向量的偏移量 */
    float dx = B * t;
    float dy = -A * t;

    /* 计算两个交点 */
    ret_p1->x = x0 + dx;  // 第一交点
    ret_p1->y = y0 + dy;
    ret_p2->x = x0 - dx;  // 第二交点
    ret_p2->y = y0 - dy;

    return GT_RES_OK;
}

gt_point_f_st gt_math_catmullrom(float t, gt_point_f_st const * p0, gt_point_f_st const * p1, gt_point_f_st const * p2, gt_point_f_st const * p3)
{
    gt_point_f_st result;
    float t2 = t * t;
    float t3 = t2 * t;

    result.x = 0.5 * (2 * p1->x +
                (p2->x - p0->x) * t +
                (2 * p0->x - 5 * p1->x + 4 * p2->x - p3->x) * t2 +
                (-p0->x + 3 * p1->x - 3 * p2->x + p3->x) * t3);

    result.y = 0.5 * (2 * p1->y +
                (p2->y - p0->y) * t +
                (2 * p0->y - 5 * p1->y + 4 * p2->y - p3->y) * t2 +
                (-p0->y + 3 * p1->y - 3 * p2->y + p3->y) * t3);

    return result;
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

uint32_t gt_math_calc_bezier(gt_math_bezier_st const * const bezier)
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

void gt_srandom(uint32_t seed)
{
    _random_seed = seed;
}

uint32_t gt_random(uint32_t min, uint32_t max)
{
    if (min > max) {
        uint32_t tmp = min;
        min = max;
        max = tmp;
    }

    uint32_t x = _random_seed;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    _random_seed = x;

    const uint64_t range = (uint64_t)max - min + 1;
    const uint64_t rand_val = ((uint64_t)x * range) >> 32;

    return min + (uint32_t)rand_val;
}


// 二分查找函数
GT_ATTRIBUTE_RAM_TEXT long
gt_binary_search_from_u16(const uint16_t *arr, uint32_t size, uint32_t target)
{
    if (!arr || !size) {
        return -1;  // 如果数组为空或大小为 0，直接返回 -1
    }

    uint32_t left = 0;
    uint32_t right = size - 1;

    // 检查目标值是否在数组范围内
    if (target < arr[left] || target > arr[right]) {
        return -1;
    }

    while (left <= right) {
        uint32_t mid = left + ((right - left) >> 1);  // 计算中间位置，避免溢出

        if (arr[mid] == target) {
            return mid;  // 找到目标元素，返回索引
        } else if (arr[mid] < target) {
            left = mid + 1;  // 目标在右半部分，调整左边界
        } else {
            right = mid - 1;  // 目标在左半部分，调整右边界
        }
    }

    return -1;  // 未找到目标元素，返回 -1
}

GT_ATTRIBUTE_RAM_TEXT long
gt_binary_search_from_u32(const uint32_t *arr, uint32_t size, uint32_t target)
{
    if (!arr || !size) {
        return -1;  // 如果数组为空或大小为 0，直接返回 -1
    }

    uint32_t left = 0;
    uint32_t right = size - 1;

    // 检查目标值是否在数组范围内
    if (target < arr[left] || target > arr[right]) {
        return -1;
    }

    while (left <= right) {
        uint32_t mid = left + ((right - left) >> 1);  // 计算中间位置，避免溢出

        if (arr[mid] == target) {
            return mid;  // 找到目标元素，返回索引
        } else if (arr[mid] < target) {
            left = mid + 1;  // 目标在右半部分，调整左边界
        } else {
            right = mid - 1;  // 目标在左半部分，调整右边界
        }
    }

    return -1;  // 未找到目标元素，返回 -1
}
/* end ------------------------------------------------------------------*/
