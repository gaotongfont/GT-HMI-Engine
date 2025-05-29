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
#include "../others/gt_types.h"


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

#define GT_CLAMP(min, val, max) (GT_MAX(min, (GT_MIN(val, max))))


#define GT_MATH_BEZIER_VAL_SHIFT        10  /* (1 << 10) = 1024 */
#define GT_MATH_BEZIER_VAL_RESOLUTION   (1 << GT_MATH_BEZIER_VAL_SHIFT)
#define GT_MATH_BEZIER_VAL_FLOAT(_val)  (uint32_t)((_val) * GT_MATH_BEZIER_VAL_RESOLUTION)

#define GT_MATH_CUBIC_NEWTON_ITER       8   /** Cubic Newton iterations */
#define GT_MATH_CUBIC_BITS              10  /** 10 or 14 bits recommended, int64_t calculation is used for >14bit precision */

#define GT_MATH_TRIGO_SIN_MAX 32767
#define GT_MATH_TRIGO_SHIFT 15 /**<  >> GT_MATH_TRIGO_SHIFT to normalize*/


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

/**
 * @brief line Ax + By + C = 0
 */
typedef struct {
    float a;
    float b;
    float c;
}gt_math_line_st;

/**
 * @brief Circle structure
 */
typedef struct {
    gt_point_f_st center; ///< Center point coordinates
    double radius;        ///< Circle radius
}gt_math_circle_st;

typedef struct {
    int32_t min_val;
    int32_t max_val;
    int32_t step;
    int32_t cur_val;
    int8_t dir;
}gt_generator_st;

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

#if _USE_TRIGON_FLOAT
double gt_sin(double angle);
double gt_cos(double angle);
double gt_atan2(double x, double y);
#else
int16_t gt_sin(int16_t angle);
int16_t gt_cos(int16_t angle);
uint16_t gt_atan2(int x, int y);
#endif

double gt_square(double n);

int gt_abs(int n);

/**
 * @brief get the round value, such as:
 *        1.2 -> 1
 *        1.7 -> 2
 * @param val
 * @return long
 */
long gt_round(double val);

uint32_t gt_per_255(uint8_t n);

/**
 * @brief set random seed
 *
 * @param seed
 */
void gt_srandom(uint32_t seed);
/**
 * @brief get random value
 *
 * @param min minimum value
 * @param max maximum value
 * @return int32_t random value, min <= value <= max
 */
int32_t gt_random(int32_t min, int32_t max);

int32_t gt_sawtooth_generator_next(gt_generator_st *gen);
int32_t gt_triangle_generator_next(gt_generator_st *gen);

/**
 * @brief binary search
 *
 * @param arr   array
 * @param size  array size
 * @param target target value
 * @return long index of target value, -1 if not found
 */
long gt_binary_search_from_u16(const uint16_t *arr, uint32_t size, uint32_t target);
long gt_binary_search_from_u32(const uint32_t *arr, uint32_t size, uint32_t target);


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

/**
 * @brief
 *
 * @param bezier
 * @return uint32_t
 */
uint32_t gt_math_calc_bezier(gt_math_bezier_st const * const bezier);

/**
 * @brief
 *
 * @param x
 * @param x1 Control point 1
 * @param y1
 * @param x2 Control point 2
 * @param y2
 * @return int32_t y value
 */
int32_t gt_cubic_bezier(int32_t x, int32_t x1, int32_t y1, int32_t x2, int32_t y2);

/**
 * @brief Get line Ax + By + C = 0
 *
 * @param line Output
 * @param p1
 * @param p2
 * @return gt_res_t
 */
gt_res_t gt_math_line_get(gt_math_line_st * line,
    gt_point_f_st const * const p1, gt_point_f_st const * const p2);

/**
 * @brief Get perpendicular line base on two points of line
 *
 * @param line Output
 * @param perpendicular_point The perpendicular line
 *          passing through the coordinate point
 * @param p1
 * @param p2
 * @return gt_res_t
 */
gt_res_t gt_math_line_get_perpendicular(gt_math_line_st *line,
    gt_point_f_st const * const perpendicular_point,
    gt_point_f_st const * const p1, gt_point_f_st const * const p2);

/**
 * @brief Calculate intersection points between line and circle
 * @param p1 First intersection point (output)
 * @param p2 Second intersection point (output)
 * @param line Pointer to line structure
 * @param circle Pointer to circle structure
 * @return gt_res_t
 */
gt_res_t gt_math_line_circle_intersect(gt_point_f_st * ret_p1, gt_point_f_st* ret_p2,
    const gt_math_line_st * line, const gt_math_circle_st * circle);

/**
 * @brief
 *
 * @param t
 * @param p0
 * @param p1
 * @param p2
 * @param p3
 * @return gt_point_f_st
 */
gt_point_f_st gt_math_catmullrom(float t, gt_point_f_st const * p0, gt_point_f_st const * p1, gt_point_f_st const * p2, gt_point_f_st const * p3);

static inline uint32_t gt_math_bezier3(gt_math_bezier_st const * const bezier) {
    return gt_cubic_bezier(bezier->t, 341, bezier->p1, 683, bezier->p3);
}

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_MATH_H_
