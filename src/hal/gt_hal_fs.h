/**
 * @file gt_hal_fs.h
 * @author Feyoung
 * @brief file system implementation
 * @version 0.1
 * @date 2022-08-22 14:31:34
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_HAL_FS_H_
#define _GT_HAL_FS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../gt_conf.h"
#include "../others/gt_types.h"

#if GT_USE_FILE_HEADER
#include "../hal/gt_hal_file_header.h"
#endif


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/

typedef uint32_t ( * rw_cb_t)(uint8_t * data_write, uint32_t len_write, uint8_t * data_read, uint32_t len_read);

/**
 * @brief virt file drv operate result
 */
typedef enum {
    GT_FS_RES_FAIL = -1,    ///< failed to open file
    GT_FS_RES_OK,           ///< file was successfully opened
    GT_FS_RES_HW_ERR,       ///< file system hardware errors
    GT_FS_RES_NOT_EX,       ///< file system not exists
    GT_FS_RES_DEINIT,       ///< file system deinit
    GT_FS_RES_BUSY,         ///< file operation is busy
    GT_FS_RES_READY,        ///< file system ready
    GT_FS_RES_NULL,         ///< file is null
}gt_fs_res_et;

/**
 * @brief virtual file system write or read mode.
 */
typedef enum {
    GT_FS_MODE_RD  = 0,     ///< file system read only mode
    GT_FS_MODE_WD  = 1,     ///< file system write mode
    GT_FS_MODE_WR  = 2,     ///< file system read and write mode
}gt_fs_mode_et;

/**
 * @brief file system type enum.
 */
typedef enum gt_fs_type_e {
    GT_FS_TYPE_UNKNOWN = -1,
    GT_FS_TYPE_FLASH   = 0,
    GT_FS_TYPE_ARRAY   = 1,
    GT_FS_TYPE_SD      = 2,
}gt_fs_type_et;

/**
 * @brief The drive letter of the hard disk
 */
typedef enum gt_fs_label_e {
    GT_FS_LABEL_FLASH = 'f',
    GT_FS_LABEL_ARRAY = '.',
    GT_FS_LABEL_SD    = 's',
}gt_fs_label_et;

/**
 * @brief seek set pos type
 */
typedef enum {
    GT_FS_SEEK_SET = 0x00,      ///< Set the position from absolutely (from the start of file)
    GT_FS_SEEK_CUR = 0x01,      ///< Set the position from the current position
    GT_FS_SEEK_END = 0x02,      ///< Set the position from the end of the file
}gt_fs_whence_et;

/**
 * @brief file system directory info structure
 */
typedef struct _gt_fs_dir_s {
    char * dir_d;               ///< directory name
    struct _gt_fs_drv_s * drv;  ///< driver
}gt_fs_dir_st;

/**
 * @brief virt file system drv
 */
typedef struct _gt_fs_drv_s {
    gt_fs_label_et letter;

    /**
     * @brief write or read data from flash
     * ! [user set: spi read write func]
     *
     * param data_write the data to be written
     * param len_write the length of the data to be written
     * param data_read the data to be read from flash
     * param len_read the length need to read from flash
     *
     * return uint32_t The number of bytes data read back preferentially,
     *      otherwise the number of bytes written successfully.
     */
    rw_cb_t rw_cb;

    /* do not change */
#if GT_USE_FILE_HEADER
    void *( * fh_open_cb)(struct _gt_fs_drv_s * drv, gt_file_header_param_st const * const fh_param, gt_fs_mode_et mode);
#endif
    void *( * open_cb)(struct _gt_fs_drv_s * drv, char * name, gt_fs_mode_et mode);
    void ( * close_cb)(struct _gt_fs_drv_s * drv, void * fp);
    gt_fs_res_et ( * read_cb )(struct _gt_fs_drv_s * drv, void * fp, uint8_t * data, uint32_t len, uint32_t * ret_len);
    gt_fs_res_et ( * write_cb)(struct _gt_fs_drv_s * drv, void * fp, uint8_t * data, uint32_t len, uint32_t * ret_len);
    gt_fs_res_et ( * seek_cb )(struct _gt_fs_drv_s * drv, void * fp, uint32_t pos, gt_fs_whence_et whence);
    gt_fs_res_et ( * tell_cb )(struct _gt_fs_drv_s * drv, void * fp, uint32_t * pos);

#if GT_USE_FOLDER_SYSTEM
    gt_fs_res_et ( * dir_open_cb )(struct _gt_fs_drv_s * drv, char * name);
    gt_fs_res_et ( * dir_read_cb )(struct _gt_fs_drv_s * drv, struct _gt_fs_dir_s * dir_p);
    gt_fs_res_et ( * dir_close_cb)(struct _gt_fs_dir_s * dir_p);
#endif

#if GT_USE_FS_NAME_BY_INDEX
    char const * const ( * get_name_by_cb)(uint16_t index_of_list);
#endif
}gt_fs_drv_st;

/**
 * @brief virt file drv open_cb return type
 */
typedef struct _gt_fs_fp_s {
    uint32_t start;
    uint32_t end;
    uint32_t pos;

    gt_fs_type_et type;     ///< file type flash or array,this can be @ref gt_fs_type_et
    gt_fs_mode_et mode;     ///< file mode

    union {
        struct {
            gt_size_t w;
            gt_size_t h;
            uint8_t is_alpha;
        }pic;
        struct {
            uint32_t encoding;
        }txt;
    }msg;
    void * file_fp;     ///< unused  FILE * fp; for  SD Card
    gt_fs_drv_st * drv;
    void * buffer;
}gt_fs_fp_st;



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

gt_fs_fp_st * _gt_hal_fp_init(void);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_HAL_FS_H_
