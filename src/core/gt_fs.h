/**
 * @file gt_fs.h
 * @author yongg
 * @brief file system implementation for hal layer devices.
 * @version 0.1
 * @date 2022-07-05 19:15:04
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
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

const char * gt_fs_get_ext(const char * path);

/**
 * @brief get source file type @ref gt_fs_type_et
 *
 * @param src the path or file name
 * @return gt_fs_res_et the result status
 */
gt_fs_type_et gt_fs_get_src_type(const void * src);

/**
 * @brief Try to identify the file and driver and open the file
 *
 * @param path The full path of file, see port file lists
 * @param mode The file operation mode
 * @return gt_fs_res_et the result status
 */
gt_fs_fp_st * gt_fs_open(const char * path, gt_fs_mode_et mode);

#if GT_USE_FILE_HEADER
/**
 * @brief open file by file header control
 *
 * @param fh_param The index of header list item and the index of
 *              header item inside count, package_index Enabled when
 *              _GT_FILE_HEADER_IMG_PACKAGE is enabled,
 * @param mode The file operation mode
 * @return gt_fs_res_et the result status
 */
gt_fs_fp_st * gt_fs_fh_open(gt_file_header_param_st const * const fh_param, gt_fs_mode_et mode);
#endif

#if GT_USE_DIRECT_ADDR
/**
 * @brief Open file by direct address
 *
 * @param addr
 * @param mode
 * @return gt_fs_fp_st*
 */
gt_fs_fp_st * gt_fs_direct_addr_open(gt_addr_t addr, gt_fs_mode_et mode);
#endif

#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
/**
 * @brief Open file by custom size direct address
 *
 * @param dac
 * @param mode
 * @return gt_fs_fp_st*
 */
gt_fs_fp_st * gt_fs_custom_size_addr_open(gt_direct_addr_custom_size_st * dac, gt_fs_mode_et mode);
#endif

/**
 * @brief Read file contents from file descriptor
 *
 * @param out output data buffer
 * @param size need to read bytes count from file
 * @param ret_len The number of bytes actually read
 * @return gt_fs_res_et the result status
 */
gt_fs_res_et gt_fs_read(gt_fs_fp_st * fp, uint8_t * out, uint32_t size, uint32_t * ret_len);

/**
 * @brief Set the cursor for the file
 *
 * @param offset Offset from the current cursor position
 * @param whence The position of the cursor relative to the file
 * @return gt_fs_res_et The result status
 */
gt_fs_res_et gt_fs_seek(gt_fs_fp_st * fp, uint32_t offset, gt_fs_whence_et whence);

/**
 * @brief Get the cursor position from file content
 *
 * @param pos The absolute offset in the file
 * @return gt_fs_res_et The result status
 */
gt_fs_res_et gt_fs_tell(gt_fs_fp_st * fp, uint32_t * pos);

/**
 * @brief The content are ready to be written to the file
 *
 * @param buffer The data need to be written
 * @param len The data bytes length
 * @param ret_len The number of bytes successfully written to the file
 * @return gt_fs_res_et The result status
 */
gt_fs_res_et gt_fs_write(gt_fs_fp_st * fp, uint8_t * buffer, uint32_t len, uint32_t * ret_len);

/**
 * @brief Gets data of the specified length in bytes from the start of the
 *      file content.
 *      Must be called after gt_fs_open().
 *
 * @param res The buffer save data from file content
 * @param offset The offset in the file content
 * @param len The number bytes of buffer length, which need to be read
 * @return gt_fs_res_et The result status
 */
gt_fs_res_et gt_fs_read_img_offset(gt_fs_fp_st * fp, uint8_t * res, uint32_t offset, uint32_t len);

/**
 * @brief Get the image width and height, by the full path of file name.
 *
 * @param path The full path of file name
 * @param w Get image width value
 * @param h Get image height value
 * @return gt_fs_res_et The result status
 */
gt_fs_res_et gt_fs_read_img_wh(const char * path, uint16_t * w, uint16_t * h);

#if GT_USE_FILE_HEADER
/**
 * @brief Get the image width and height, by the file header of file item.
 *
 * @param fh [file header] idx: The index of header list item and the package index of
 *              header item inside count
 * @param w Get image width value
 * @param h Get image height value
 * @return gt_fs_res_et The result status
 */
gt_fs_res_et gt_fs_fh_read_img_wh(gt_file_header_param_st * fh, uint16_t * w, uint16_t * h);
#endif

#if GT_USE_DIRECT_ADDR
/**
 * @brief Get the image width and height, by direct addr of file item.
 *
 * @param addr The address of the image data
 * @param w Get image width value
 * @param h Get image height value
 * @return gt_fs_res_et The result status
 */
gt_fs_res_et gt_fs_direct_addr_read_img_wh(gt_addr_t addr, uint16_t * w, uint16_t * h);
#endif

/**
 * @brief Directly read the physical data from vf(HMI-chip, flash, etc) the address
 *
 * @param addr HMI-chip or Flash address
 * @param len The number of bytes to read
 * @param data The buffer to save the data
 * @return uint32_t The number of bytes read back
 */
uint32_t gt_fs_read_direct_physical(gt_addr_t addr, uint32_t len, uint8_t * data);

/**
 * @brief Close the file operation handler, which need to
 *      be called after gt_fs_open().
 */
void gt_fs_close(gt_fs_fp_st * fp);

#if GT_USE_FS_NAME_BY_INDEX
/**
 * @brief Get the file name by index of the file list
 *
 * @param file_type
 * @param index_of_list
 * @return char const* const The file full fname
 */
char const * const gt_fs_get_name_by_index(gt_fs_type_et file_type, uint16_t index_of_list);
#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_FS_H_
