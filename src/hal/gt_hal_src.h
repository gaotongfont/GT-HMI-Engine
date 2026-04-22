/**
 * @file gt_hal_src.h
 * @author Feyoung
 * @brief Hal layer source file system implementation, arrays form management.
 * @version 0.1
 * @date 2022-08-19 15:25:28
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_HAL_SRC_H_
#define _GT_HAL_SRC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../gt_conf.h"
#include "../others/gt_types.h"
#include "./gt_hal_fs.h"



/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/

/**
 * @brief material management
 */
typedef struct gt_src_s {
    char * name;    ///< path or name
    gt_size_t w;    ///< width
    gt_size_t h;    ///< height
    uint8_t const * const img;  ///< The pointer of image data
    uint32_t size;  ///< image data count of byte.
    uint8_t is_alpha : 1;   ///< 1: has alpha channel
    uint8_t color_format : 5;   ///< pixel format @ref gt_img_color_format_et: 0=auto(RGB565), 2=RGB565, 3=RGB888, 4=ARGB8888
    uint8_t reserved : 2;
}gt_src_st;


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief The resource is initialized to the working mode of an array
 *
 * @param src_sys Resource File System Management
 * @param sys_count The length of the sys array.
 */
void gt_src_init(const gt_src_st * const src_sys, uint32_t sys_count);

/**
 * @brief Get the driver from the resource mode.
 *
 * @return gt_fs_drv_st* The driver
 */
gt_fs_drv_st * gt_src_get_drv(void);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_HAL_SRC_H_
