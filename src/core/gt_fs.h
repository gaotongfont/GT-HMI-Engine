/**
 * @file gt_fs.h
 * @author yongg
 * @brief file system implementation for hal layer devices.
 * @version 0.1
 * @date 2022-07-05 19:15:04
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_FS_H_
#define _GT_FS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"
#include "../hal/gt_hal_fs.h"
#include "../hal/gt_hal_vf.h"
#include "../hal/gt_hal_src.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief get source file type @ref gt_fs_type_et
 *
 * @param src
 * @return gt_fs_type_et
 */
gt_fs_type_et gt_fs_get_src_type(const void * src);

gt_fs_fp_st * gt_fs_get_fp(void);

gt_fs_fp_st * gt_fs_open(const char * path, gt_fs_mode_et mode);

gt_fs_res_et gt_fs_read(uint8_t * out, uint32_t size, uint32_t * ret_len);

gt_fs_res_et gt_fs_seek(uint32_t offset, gt_fs_whence_et whence);

gt_fs_res_et gt_fs_tell(uint32_t * pos);

gt_fs_res_et gt_fs_write(uint8_t * buffer, uint32_t len, uint32_t * ret_len);

gt_fs_res_et gt_fs_read_img(uint8_t * res);
gt_fs_res_et gt_fs_read_img_offset(uint8_t * res, uint32_t offset, uint32_t len);
gt_fs_res_et gt_fs_read_img_wh(const char * path, uint16_t * w, uint16_t * h);

void gt_fs_close(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_FS_H_
