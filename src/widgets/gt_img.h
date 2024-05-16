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

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct gt_img_raw_s {
    gt_color_t const * buffer;    /** color channel, necessary */
    gt_opa_t   const * opa;       /** alpha channel, unnecessary */
    uint16_t   width;             /** width of the image */
    uint16_t   height;            /** height of the image */
}gt_img_raw_st;



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
void gt_img_set_raw_data(gt_obj_st * img, gt_img_raw_st * raw);

#if GT_USE_FILE_HEADER
/**
 * @brief Display the image by file header information
 *
 * @param img
 * @param fh idx -1[defalut]: Disabled file header, using img path to open file; >=0: index number, valid value
 *              < 0: invalid value.
 *           package_idx 0[defalut]: The index of element within item, as the first element
 */
void gt_img_set_by_file_header(gt_obj_st * img, gt_file_header_param_st * fh);
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


#endif  /** GT_CFG_ENABLE_IMG */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_IMG_H_
