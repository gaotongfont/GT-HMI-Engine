/**
 * @file gt_hal_vf.h
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-06-20 17:58:44
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_HAL_VF_H_
#define _GT_HAL_VF_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"
#include "stdio.h"
#include "../others/gt_types.h"
#include "gt_hal_fs.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/

/**
 * @brief virt file system file message
 */
typedef struct _gt_vfs_s {
    char * name;
    gt_size_t w;
    gt_size_t h;
    uint32_t addr;
    uint32_t size;
}gt_vfs_st;

/**
 * @brief virt file system device
 */
typedef struct _gt_vf_s {
    gt_fs_drv_st * drv;
    uint32_t addr_max;
    uint32_t addr_start;
}gt_vf_st;

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief init virt file system
 *
 * @param vfs gt_vfs_st arr,pic files messages
 */
void gt_vf_init(gt_vfs_st * vfs);

/**
 * @brief register virt file system drv
 *
 * @param drv gt_fs_drv_st device, user need only set _gt_vf_rw
 */
void gt_vf_drv_register(gt_fs_drv_st * drv);

/**
 * @brief get default virt file system dev
 *
 * @return gt_vf_st *
 */
gt_fs_drv_st * gt_vf_get_drv(void);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_VIRT_FILE_H_
