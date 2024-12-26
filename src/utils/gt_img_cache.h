/**
 * @file gt_img_cache.h
 * @author Feyoung
 * @brief The image cache function, which can be used to cache the image data.
 * @version 0.1
 * @date 2024-10-17 10:27:10
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_IMG_CACHE_H_
#define _GT_IMG_CACHE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../gt_conf.h"

#if GT_USE_IMG_CACHE
#include "../others/gt_types.h"
#include "../others/gt_color.h"
#include "../hal/gt_hal_file_header.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief Get the raw meta data from the image cache, which cache buffer is provided by the user;
 *          The cache buffer size must be enough to store the image data.
 *          byte_len >= (sizeof(gt_color_t) + sizeof(gt_opa_t)) * width * height;
 *
 * @param src The path of the image
 * @param cache_buffer The cache buffer provided by the user
 * @param byte_len The size of the cache buffer
 * @return gt_color_img_raw_st using the result to set widget image data, such as: gt_img_set_raw_data(), and so on.
 *         If the return raw.buffer value is invalid, the image data is not cached.
 */
gt_color_img_raw_st gt_img_cache_get_raw(char * src, uint8_t * cache_buffer, uint32_t byte_len);

#if GT_USE_FILE_HEADER
/**
 * @brief Get the raw meta data from the image cache, which cache buffer is provided by the user;
 *          The cache buffer size must be enough to store the image data.
 *          byte_len >= (sizeof(gt_color_t) + sizeof(gt_opa_t)) * width * height;
 *
 * @param fh Using the file header information to get the image data
 * @param cache_buffer The cache buffer provided by the user
 * @param byte_len The size of the cache buffer
 * @return gt_color_img_raw_st using the result to set widget image data, such as: gt_img_set_raw_data(), and so on.
 *         If the return raw.buffer value is invalid, the image data is not cached.
 */
gt_color_img_raw_st gt_img_cache_fh_get_raw(gt_file_header_param_st * fh, uint8_t * cache_buffer, uint32_t byte_len);
#endif

#if GT_USE_DIRECT_ADDR
/**
 * @brief Get the raw meta data from the image cache, which cache buffer is provided by the user;
 *          The cache buffer size must be enough to store the image data.
 *          byte_len >= (sizeof(gt_color_t) + sizeof(gt_opa_t)) * width * height;
 *
 * @param addr The address of the image data
 * @param cache_buffer The cache buffer provided by the user
 * @param byte_len The size of the cache buffer
 * @return gt_color_img_raw_st using the result to set widget image data, such as: gt_img_set_raw_data(), and so on.
 *         If the return raw.buffer value is invalid, the image data is not cached.
 */
gt_color_img_raw_st gt_img_cache_addr_get_raw(gt_addr_t addr, uint8_t * cache_buffer, uint32_t byte_len);
#endif


#endif  /** GT_USE_IMG_CACHE */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_IMG_CACHE_H_
