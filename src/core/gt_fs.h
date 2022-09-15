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
 * @param src the path or file name
 * @return gt_fs_res_et the result status
 */
gt_fs_type_et gt_fs_get_src_type(const void * src);

/**
 * @brief Get the virtual file operation pointer
 *
 * @return gt_fs_fp_st* The virtual file pointer
 */
gt_fs_fp_st * gt_fs_get_fp(void);

/**
 * @brief Try to identify the file and driver and open the file
 *
 * @param path The full path of file , see port file lists
 * @param mode The file operation mode
 * @return gt_fs_res_et the result status
 */
gt_fs_fp_st * gt_fs_open(const char * path, gt_fs_mode_et mode);

/**
 * @brief Read file contents from file descriptor
 *
 * @param out output data buffer
 * @param size need to read bytes count from file
 * @param ret_len The number of bytes actually read
 * @return gt_fs_res_et the result status
 */
gt_fs_res_et gt_fs_read(uint8_t * out, uint32_t size, uint32_t * ret_len);

/**
 * @brief Set the cursor for the file
 *
 * @param offset Offset from the current cursor position
 * @param whence The position of the cursor relative to the file
 * @return gt_fs_res_et The result status
 */
gt_fs_res_et gt_fs_seek(uint32_t offset, gt_fs_whence_et whence);

/**
 * @brief Get the cursor position from file content
 *
 * @param pos The absolute offset in the file
 * @return gt_fs_res_et The result status
 */
gt_fs_res_et gt_fs_tell(uint32_t * pos);

/**
 * @brief The content are ready to be written to the file
 *
 * @param buffer The data need to be written
 * @param len The data bytes length
 * @param ret_len The number of bytes successfully written to the file
 * @return gt_fs_res_et The result status
 */
gt_fs_res_et gt_fs_write(uint8_t * buffer, uint32_t len, uint32_t * ret_len);

gt_fs_res_et gt_fs_read_img(uint8_t * res);
gt_fs_res_et gt_fs_read_img_offset(uint8_t * res, uint32_t offset, uint32_t len);
gt_fs_res_et gt_fs_read_img_wh(const char * path, uint16_t * w, uint16_t * h);

void gt_fs_close(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_FS_H_
