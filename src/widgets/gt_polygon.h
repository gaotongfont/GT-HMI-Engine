/**
 * @file gt_polygon.h
 * @author Feyoung
 * @brief Polygon widget: arbitrary convex/concave polygon with fill and stroke.
 * @version 0.1
 * @date 2026-05-19
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_POLYGON_H_
#define _GT_POLYGON_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_POLYGON
#include "gt_obj.h"
#include "gt_obj_class.h"

/* define ---------------------------------------------------------------*/

/* typedef --------------------------------------------------------------*/

/* macros ---------------------------------------------------------------*/

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a polygon widget.
 *
 * @param parent Parent object
 * @return gt_obj_st* Polygon object pointer
 */
gt_obj_st * gt_polygon_create(gt_obj_st * parent);

/**
 * @brief Set polygon vertices. Copies points into internal buffer and
 *        automatically updates the bounding box. Triggers a redraw.
 *
 * @param obj    Polygon object
 * @param points Vertex array (screen absolute coordinates)
 * @param count  Number of vertices
 */
void gt_polygon_set_points(gt_obj_st * obj, gt_point_st const * points, uint16_t count);

/**
 * @brief Append a single vertex. Reallocates internal buffer if needed.
 *        Triggers bounding box recalculation and redraw.
 *
 * @param obj Polygon object
 * @param x   Vertex X (screen absolute)
 * @param y   Vertex Y (screen absolute)
 */
void gt_polygon_add_point(gt_obj_st * obj, int16_t x, int16_t y);

/**
 * @brief Remove all vertices and free the internal buffer.
 *
 * @param obj Polygon object
 */
void gt_polygon_clear_points(gt_obj_st * obj);

/**
 * @brief Set fill color.
 *
 * @param obj   Polygon object
 * @param color Fill color
 */
void gt_polygon_set_fill_color(gt_obj_st * obj, gt_color_t color);

/**
 * @brief Set fill opacity.
 *
 * @param obj Polygon object
 * @param opa Opacity (GT_OPA_TRANSP = no fill, GT_OPA_COVER = opaque)
 */
void gt_polygon_set_fill_opa(gt_obj_st * obj, gt_opa_t opa);

/**
 * @brief Set stroke color.
 *
 * @param obj   Polygon object
 * @param color Stroke color
 */
void gt_polygon_set_line_color(gt_obj_st * obj, gt_color_t color);

/**
 * @brief Set stroke width. 0 = no stroke.
 *
 * @param obj   Polygon object
 * @param width Stroke width in pixels
 */
void gt_polygon_set_line_width(gt_obj_st * obj, uint8_t width);

/**
 * @brief Set stroke opacity.
 *
 * @param obj Polygon object
 * @param opa Stroke opacity
 */
void gt_polygon_set_line_opa(gt_obj_st * obj, gt_opa_t opa);

/**
 * @brief Get the number of vertices.
 *
 * @param obj Polygon object
 * @return uint16_t Vertex count
 */
uint16_t gt_polygon_get_count(gt_obj_st * obj);

/**
 * @brief Get a read-only pointer to the vertex array.
 *
 * @param obj Polygon object
 * @return gt_point_st const* Vertex array, or NULL if none
 */
gt_point_st const * gt_polygon_get_points(gt_obj_st * obj);

#endif  /** GT_CFG_ENABLE_POLYGON */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //!_GT_POLYGON_H_
