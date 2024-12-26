/**
 * @file gt_mask.h
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-11-18 15:03:06
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */
#ifndef _GT_MASK_H_
#define _GT_MASK_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_graph_base.h"



/* define ---------------------------------------------------------------*/
#define _GT_MASK_MAX_NUM         (16)
#define _GT_MASK_CIRCLE_MAX_NUM  (4)


/* typedef --------------------------------------------------------------*/
typedef enum{
    GT_MASK_RES_TRANSP = 0,
    GT_MASK_RES_FULL_COVER,
    GT_MASK_RES_CHANGE,
    GT_MASK_RES_UNKNOWN,
}gt_mask_res_et;


typedef enum {
    GT_MASK_TYPE_RADIUS = 0,
    GT_MASK_TYPE_LINE,
    GT_MASK_TYPE_ANGLE,
}gt_mask_type_et;

typedef enum {
    GT_MASK_LINE_SIDE_LEFT = 0,
    GT_MASK_LINE_SIDE_RIGHT,
    GT_MASK_LINE_SIDE_TOP,
    GT_MASK_LINE_SIDE_BOTTOM,
}gt_mask_line_side_et;

typedef gt_mask_res_et (*gt_mask_get_cb_t)(gt_opa_t* , gt_size_t, gt_size_t, uint16_t, void*);

typedef struct {
    gt_mask_get_cb_t mask_get_cb;
    gt_mask_type_et type;
}_gt_mask_dsc_st;

typedef struct {
    uint8_t *buf;
    gt_opa_t *opa;
    uint16_t *x_start_on_y;
    uint16_t *opa_start_on_y;
    int32_t count;
    int32_t used_cnt;
    uint16_t radius;
}_gt_mask_radius_circle_st;

typedef struct {
    _gt_mask_dsc_st dsc;
    _gt_mask_radius_circle_st* circle;
    gt_area_st area;
    uint16_t radius;
    uint8_t outer;
}gt_mask_radius_st;

typedef struct {
    _gt_mask_dsc_st dsc;
    gt_point_st p1;
    gt_point_st p2;
    gt_point_st origo;
    int32_t xy_steep;
    int32_t yx_steep;
    int32_t steep;
    int32_t spx;
    uint8_t side : 2;
    uint8_t flat : 1;
    uint8_t inv : 1;
}gt_mask_line_st;

typedef struct {
    _gt_mask_dsc_st dsc;
    gt_point_st vertex_p;
    gt_size_t start_angle;
    gt_size_t end_angle;
    gt_mask_line_st start_line;
    gt_mask_line_st end_line;
    uint16_t delta_deg;
}gt_mask_angle_st;

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief add mask to list
 *
 * @param p_mask mask param
 * @return int16_t list index; -1: failed
 */
int16_t gt_mask_add(void* p_mask);
/**
 * @brief get mask result from list
 *
 * @param mask_buf
 * @param x
 * @param y
 * @param len
 * @return gt_mask_res_et
 */
gt_mask_res_et gt_mask_get(gt_opa_t* mask_buf, gt_size_t x, gt_size_t y, uint16_t len);

/**
 * @brief remove mask from list
 *
 * @param idx
 */
void gt_mask_remove_idx(int16_t idx);
/**
 * @brief remove all mask from list
 *
 */
void gt_mask_remove_all(void);

/**
 * @brief free mask
 *
 * @param p_mask
 */
void gt_mask_free(void* p_mask);

/**
 * @brief check if mask is valid from list
 *
 * @param coords
 * @return true
 * @return false
 */
bool gt_mask_check(const gt_area_st * coords);

/**
 * @brief init mask radius
 *
 * @param p_mask
 * @param coords
 * @param radius
 * @param outer
 */
void gt_mask_radius_init(gt_mask_radius_st* p_mask, const gt_area_st* coords, uint16_t radius, bool outer);
/**
 * @brief init mask line
 *
 * @param p_mask
 * @param p1x
 * @param p1y
 * @param p2x
 * @param p2y
 * @param side
 */
void gt_mask_line_points_init(gt_mask_line_st* p_mask, gt_size_t p1x, gt_size_t p1y, gt_size_t p2x, gt_size_t p2y, gt_mask_line_side_et side);
/**
 * @brief init mask line
 *
 * @param p_mask
 * @param p1x
 * @param py
 * @param angle
 * @param side
 */
void gt_mask_line_angle_init(gt_mask_line_st* p_mask, gt_size_t p1x, gt_size_t py, int16_t angle, gt_mask_line_side_et side);
/**
 * @brief init mask angle
 *
 * @param p_mask
 * @param vertex_x
 * @param vertex_y
 * @param start_angle
 * @param end_angle
 */
void gt_mask_angle_init(gt_mask_angle_st* p_mask, gt_size_t vertex_x, gt_size_t vertex_y, gt_size_t start_angle, gt_size_t end_angle);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_MASK_H_

/* end of file ----------------------------------------------------------*/


