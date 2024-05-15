/**
 * @file gt_hal_file_header.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-05-10 14:33:48
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_HAL_FILE_HEADER_H_
#define _GT_HAL_FILE_HEADER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"
#include "../gt_conf.h"

#if GT_USE_FILE_HEADER
#include "../others/gt_types.h"



/* define ---------------------------------------------------------------*/

#ifndef _GT_FILE_HEADER_USE_RAND_INDEX
    /**
     * @brief Use random index to search file header item.
     *      If disabled, the search will be based on the dfault order.
     *      default: 0
     */
    #define _GT_FILE_HEADER_USE_RAND_INDEX      0
#endif

#ifndef _GT_FILE_HEADER_IMG_PACKAGE
    /**
     * @brief The image package size.
     *      default: 0
     */
    #define _GT_FILE_HEADER_IMG_PACKAGE         01
#endif

#ifndef _GT_FILE_HEADER_IMG_SAME_SIZE
    /**
     * @brief The image using the same size.
     *      default: 0
     */
    #define _GT_FILE_HEADER_IMG_SAME_SIZE      0
#endif

/* typedef --------------------------------------------------------------*/

/**
 * @brief Must be gt_file_header_param_init() before using.
 */
typedef struct gt_file_header_param_s {
    /** -1[defalut]: Disabled, using path to open img file; otherwise open file by file header */
    gt_file_header_idx_t idx;

#if _GT_FILE_HEADER_IMG_PACKAGE
    /** 0[defalut]: The first element of item */
    gt_file_header_idx_t package_idx;
#endif
}gt_file_header_param_st;

/**
 * @brief The file header
 */
typedef struct gt_file_header_s {
    uint32_t address;

#if _GT_FILE_HEADER_USE_RAND_INDEX
    gt_file_header_idx_t index;
#endif
#if _GT_FILE_HEADER_IMG_PACKAGE
    uint16_t package_count;
#endif

#if !_GT_FILE_HEADER_IMG_SAME_SIZE
    uint16_t width;
    uint16_t height;
    uint8_t alpha;
    uint8_t color_depth;
#endif
    uint16_t reserved;
}gt_file_header_st;


typedef struct _gt_file_header_ctl_s {
    gt_file_header_st const * header;
    uint16_t count;

#if _GT_FILE_HEADER_IMG_SAME_SIZE
    uint32_t img_size;
#endif
}_gt_file_header_ctl_st;

/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief Get sizeof(gt_file_header_st)
 *
 * @return uint16_t The size of gt_file_header_st
 */
uint16_t gt_file_header_get_instance_size(void);

#if _GT_FILE_HEADER_IMG_SAME_SIZE
/**
 * @brief
 *
 * @param header [WARN] The object cannot be deallocated, and freeing memory causes an exception
 * @param count The count of header item
 * @param img_size The image data byte size
 */
void gt_file_header_init(gt_file_header_st const * header_list, uint16_t count, uint32_t img_size);
#else
/**
 * @brief
 *
 * @param header [WARN] The object cannot be deallocated, and freeing memory causes an exception
 * @param count The count of header item
 */
void gt_file_header_init(gt_file_header_st const * header_list, uint16_t count);
#endif

/**
 * @brief Get the count of file header
 *
 * @return uint16_t
 */
uint16_t gt_file_header_get_count(void);

/**
 * @brief Init gt_file_header_param_st
 *
 * @param param
 */
void gt_file_header_param_init(gt_file_header_param_st * param);

/**
 * @brief
 *
 * @param index
 * @return gt_file_header_st const*
 */
gt_file_header_st const * gt_file_header_get(gt_file_header_idx_t index);

/**
 * @brief Get image data byte size
 *
 * @param item NULL: When _GT_FILE_HEADER_IMG_SAME_SIZE
 * @return uint32_t
 */
uint32_t gt_file_header_get_img_size(gt_file_header_st const * item);

#if _GT_FILE_HEADER_IMG_PACKAGE
/**
 * @brief
 *
 * @param item
 * @param image_idx The index of item inside the package count, begin from 0,
 *      which equal to item begin address.
 * @return uint32_t
 */
uint32_t gt_file_header_get_img_offset_by(gt_file_header_st const * item, gt_file_header_idx_t image_idx);
#endif

#endif  /** GT_USE_FILE_HEADER */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_HAL_FILE_HEADER_H_
