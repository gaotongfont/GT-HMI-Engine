/**
 * @file gt_hal_vf.h
 * @author yongg
 * @brief flash resource management implementation
 * @version 0.1
 * @date 2022-06-20 17:58:44
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_HAL_VF_H_
#define _GT_HAL_VF_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"
#include "../gt_conf.h"
#include "../others/gt_types.h"
#include "gt_hal_fs.h"


/* define ---------------------------------------------------------------*/


/* typedef --------------------------------------------------------------*/

/**
 * @brief virt file system file message
 */
typedef struct _gt_vfs_s {
    char * name;    ///< path or file name
    gt_size_t w;    ///< width
    gt_size_t h;    ///< height
    uint32_t addr;  ///< address which resource data save
    uint32_t size;  ///< the resource size
    uint8_t is_alpha;   ///< is not alpha channel
}gt_vfs_st;

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
#if GT_USE_MODE_FLASH

/**
 * @brief init virt file system
 *
 * @param vfs gt_vfs_st arr,pic files messages
 */
void gt_vf_init(const gt_vfs_st * vfs);

/**
 * @brief register virt file system drv
 *
 * @param drv gt_fs_drv_st device, user need only set _gt_vf_rw
 */
void gt_vf_drv_register(rw_cb_t rw_cb);

/**
 * @brief get default virt file system dev
 *
 * @return gt_vf_st *
 */
gt_fs_drv_st * gt_vf_get_drv(void);

#endif  /** GT_USE_MODE_FLASH */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_VIRT_FILE_H_
