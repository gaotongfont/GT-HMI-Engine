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
#include "stddef.h"



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
    #define _GT_FILE_HEADER_IMG_PACKAGE         0
#endif

#ifndef _GT_FILE_HEADER_IMG_SAME_SIZE
    /**
     * @brief The image using the same size.
     *      default: 0
     */
    #define _GT_FILE_HEADER_IMG_SAME_SIZE       0
#endif

/**
 * @brief init invalid index
 */
#define GT_FILE_HEADER_INVALID_IDX              -1

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
 * @brief Image size: [(sizeof(gt_color_t) + alpha ? sizeof(gt_opa_t) : 0) * width * height],
 *      The size of image data in bytes
 *      distribution: [color data][color data] ... [color data] * width * height
 *                    [alpha data][alpha data] ... [alpha data] * width * height
 */
typedef struct gt_file_header_color_info_s {
    uint16_t width;
    uint16_t height;
    uint8_t alpha;
}gt_file_header_color_info_st;

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
    gt_file_header_color_info_st info;
#endif

    uint16_t reserved;
}gt_file_header_st;


typedef struct _gt_file_header_ctl_s {
    gt_file_header_st const * header;
    uint16_t count;

#if _GT_FILE_HEADER_IMG_SAME_SIZE
    gt_file_header_color_info_st info;
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
 * @param info Image info
 */
void gt_file_header_init(gt_file_header_st const * header_list, uint16_t count, gt_file_header_color_info_st * info);
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
 * @param fh file header param
 */
void gt_file_header_param_init(gt_file_header_param_st * fh);

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
 * @param item
 * @return uint32_t
 */
uint32_t gt_file_header_get_img_size(gt_file_header_st const * item);

/**
 * @brief
 *
 * @param item
 * @param fh File header param The index of item inside the package count, begin from 0,
 *      which equal to item begin address.
 *      Enabled when _GT_FILE_HEADER_IMG_PACKAGE is enabled.
 * @return uint32_t
 */
uint32_t gt_file_header_get_img_offset_by(gt_file_header_st const * item, gt_file_header_param_st const * const fh);

/**
 * @brief Check param is valid, otherwise return NULL
 *
 * @param param The param to check
 * @return gt_file_header_param_st * result object to draw image
 */
gt_file_header_param_st * gt_file_header_param_check_valid(gt_file_header_param_st * param);

#endif  /** GT_USE_FILE_HEADER */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_HAL_FILE_HEADER_H_
