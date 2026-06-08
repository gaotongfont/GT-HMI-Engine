/**
 * @file gt_img.h
 * @author yongg
 * @brief image display control
 * @version 0.1
 * @date 2022-07-05 17:36:46
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_IMG_H_
#define _GT_IMG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_IMG
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "../others/gt_color.h"

#if GT_USE_FILE_HEADER
#include "../hal/gt_hal_file_header.h"
#endif

#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
#include "../hal/gt_hal_fs.h"
#endif

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief  create img obj
 *
 * @param parent img's parent obj
 * @return gt_obj_st* img obj ptr
 */
gt_obj_st * gt_img_create(gt_obj_st * parent);

/**
 * @brief set the path of the img
 *
 * @param img img obj
 * @param src path of img
 */
void gt_img_set_src(gt_obj_st * img, char * src);

/**
 * @brief Display the image directly with raw data
 *
 * @param img img object
 * @param raw Raw data of the image
 */
void gt_img_set_raw_data(gt_obj_st * img, gt_color_img_raw_st * raw);

#if GT_USE_FILE_HEADER
/**
 * @brief Display the image by file header information
 *
 * @param img
 * @param fh idx -1[default]: Disabled file header, using img path to open file; >=0: index number, valid value
 *              < 0: invalid value.
 *           package_idx 0[default]: The index of element within item, as the first element
 */
void gt_img_set_by_file_header(gt_obj_st * img, gt_file_header_param_st * fh);

gt_file_header_param_st* gt_img_get_file_header_param(gt_obj_st * img);
#endif

#if GT_USE_DIRECT_ADDR
/**
 * @brief Display the image by direct address
 *
 * @param img
 * @param addr Direct address of the image
 */
void gt_img_set_by_direct_addr(gt_obj_st * img, gt_addr_t addr);

gt_addr_t gt_img_get_direct_addr(gt_obj_st * img);
#endif

#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
/**
 * @brief Display the image by custom size direct address
 *
 * @param img
 * @param dac Custom size direct address of the image
 */
void gt_img_set_by_custom_size_addr(gt_obj_st * img, gt_direct_addr_custom_size_st * dac);

gt_direct_addr_custom_size_st * gt_img_get_custom_size_addr(gt_obj_st * img);
#endif

/**
 * @brief get img src of path
 *
 * @param img img obj
 * @return char* src of img
 */
char * gt_img_get_src(gt_obj_st * img);

/**
 * @brief Gets the width of the image
 *
 * @param img
 * @return uint16_t
 */
uint16_t gt_img_get_width(gt_obj_st * img);

/**
 * @brief Gets the height of the image
 *
 * @param img
 * @return uint16_t
 */
uint16_t gt_img_get_height(gt_obj_st * img);

/**
 * @brief Set the image outline masking effect
 *
 * @param img
 * @param masking
 * @return gt_res_t
 */
gt_res_t gt_img_set_masking(gt_obj_st * img, gt_color_masking_st * masking);

#if GT_USE_IMG_TRANSFORM
/**
 * @brief Set image rotation angle
 *
 * @param img   Image object
 * @param angle Angle in 0.1-degree units (0~3599), e.g. 900 = 90 degrees
 */
void gt_img_set_angle(gt_obj_st * img, uint16_t angle);

/**
 * @brief Set image zoom scale
 *
 * @param img  Image object
 * @param zoom Scale factor: 256 = 1.0x, 128 = 0.5x, 512 = 2.0x
 */
void gt_img_set_zoom(gt_obj_st * img, uint16_t zoom);

/**
 * @brief Set the pivot point for rotation/zoom (relative to image top-left)
 *
 * @param img     Image object
 * @param pivot_x Pivot x in pixels (default: image center)
 * @param pivot_y Pivot y in pixels (default: image center)
 */
void gt_img_set_pivot(gt_obj_st * img, int16_t pivot_x, int16_t pivot_y);
#endif

/**
 * @brief Set the border width of the image
 *
 * @param img   Image object
 * @param width Border width in pixels (0 = no border)
 */
void gt_img_set_border_width(gt_obj_st * img, uint8_t width);

/**
 * @brief Set the border color of the image
 *
 * @param img   Image object
 * @param color Border color
 */
void gt_img_set_border_color(gt_obj_st * img, gt_color_t color);

/**
 * @brief Set the border corner radius of the image
 *
 * @param img    Image object
 * @param radius Corner radius in pixels (0 = sharp corners)
 */
void gt_img_set_border_radius(gt_obj_st * img, uint16_t radius);

/**
 * @brief Enable opa-only tint mode: ignore image RGB data, blend the specified
 *        color using the image alpha channel only.
 *
 * @param img   Image object
 * @param color Tint color to apply
 */
void gt_img_set_opa_color(gt_obj_st * img, gt_color_t color);

/**
 * @brief Disable opa-only tint mode and restore normal image RGB rendering.
 *
 * @param img Image object
 */
void gt_img_clear_opa_color(gt_obj_st * img);

#endif  /** GT_CFG_ENABLE_IMG */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_IMG_H_
