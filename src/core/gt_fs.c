/**
 * @file gt_fs.c
 * @author yongg
 * @brief file system implementation for hal layer devices.
 * @version 0.1
 * @date 2022-07-05 19:15:15
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_fs.h"
#include "../others/gt_color.h"
#include "../others/gt_log.h"
#include "ctype.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/

typedef struct _gt_vf_type_label_s {
    gt_fs_type_et type;
    gt_fs_label_et label;
}_gt_vf_type_label_st;

typedef struct _gt_fs_dsc_s {
    gt_fs_drv_st * drv;
    gt_fs_fp_st * fp;
}_gt_fs_dsc_st;

/* static variables -----------------------------------------------------*/

/**
 * @brief default drive label
 */
static const _gt_vf_type_label_st _drive_label[] = {
    { GT_FS_TYPE_SD,    GT_FS_LABEL_SD    },
    { GT_FS_TYPE_ARRAY, GT_FS_LABEL_ARRAY },
    { GT_FS_TYPE_FLASH, GT_FS_LABEL_FLASH },
};

/**
 * @brief recoding now file descriptions
 */
static GT_ATTRIBUTE_LARGE_RAM_ARRAY _gt_fs_dsc_st _self;

static uint8_t file_occupied_count = 0;

/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

/**
 * @brief get file system type
 *
 * @param index
 * @return gt_fs_type_et
 */
static inline gt_fs_type_et _gt_fs_get_type(gt_size_t index)
{
    return _drive_label[index].type;
}

/**
 * @brief get file system label
 *
 * @param index
 * @return gt_fs_label_et
 */
static inline gt_fs_label_et _gt_fs_get_label(gt_size_t index)
{
    return _drive_label[index].label;
}

static inline _gt_fs_dsc_st * _gt_fs_get_dsc(void)
{
    return &_self;
}

static inline void _gt_fs_set_drv(gt_fs_drv_st * drv)
{
    _self.drv = drv;
}

static inline void _gt_fs_set_fp(gt_fs_fp_st * fp)
{
    _self.fp = fp;
}

static inline gt_fs_drv_st * _gt_fs_get_drv(void)
{
    return _self.drv;
}

static inline void _gt_fs_free_dsc(void)
{
    _self.drv = NULL;
    _self.fp = NULL;
}

static inline void _gt_fs_file_occupied_increase(void)
{
    ++file_occupied_count;
}

static inline void _gt_fs_file_occupied_decrease(void)
{
    if (!file_occupied_count) {
        return ;
    }
    --file_occupied_count;
}

static inline uint8_t _gt_fs_get_file_occupied_count(void)
{
    return file_occupied_count;
}

/**
 * @brief get file system driver, according to letters
 *          @ref _drive_label array
 *
 * @param path full path
 * @return gt_fs_drv_st* the driver of file system
 */
static gt_fs_drv_st * gt_fs_get_default_drv(char const * path)
{
    gt_fs_type_et type = gt_fs_get_src_type(path);
    gt_fs_drv_st * drv = NULL;

    switch (type)
    {
    case GT_FS_TYPE_ARRAY: {
       drv = gt_src_get_drv();
        break;
    }
    case GT_FS_TYPE_FLASH: {
        drv = gt_vf_get_drv();
        break;
    }
    case GT_FS_TYPE_SD: {
        break;
    }
    default:
        break;
    }
    return drv;
}


/* global functions / API interface -------------------------------------*/

gt_fs_type_et gt_fs_get_src_type(const void * src)
{
    gt_fs_type_et ret = GT_FS_TYPE_UNKNOWN;
    char drive_label = tolower(*(uint8_t * )src);
    gt_size_t len = sizeof(_drive_label) / sizeof(_gt_vf_type_label_st);

    while (len--) {
        if (_gt_fs_get_label(len) == drive_label) {
            ret = _gt_fs_get_type(len);
            break;
        }
    }

    return ret;
}

gt_fs_fp_st * gt_fs_get_fp(void)
{
    return _self.fp;
}

gt_fs_fp_st * gt_fs_open(const char * path, gt_fs_mode_et mode)
{
    gt_fs_drv_st * drv = gt_fs_get_default_drv(path);

    if (!drv) {
        return NULL;
    }
    gt_fs_fp_st * fp = drv->open_cb(drv, (char *)path, mode);

    _gt_fs_file_occupied_increase();

    _gt_fs_set_drv(drv);
    _gt_fs_set_fp(fp);

    return fp;
}

gt_fs_res_et gt_fs_read(uint8_t * out, uint32_t size, uint32_t * ret_len)
{
    gt_fs_drv_st * drv = _gt_fs_get_drv();
    gt_fs_fp_st * fp = gt_fs_get_fp();

    return drv->read_cb(drv, fp, out, size, ret_len);
}

gt_fs_res_et gt_fs_seek(uint32_t offset, gt_fs_whence_et whence)
{
    gt_fs_drv_st * drv = _gt_fs_get_drv();
    gt_fs_fp_st * fp = gt_fs_get_fp();

    return drv->seek_cb(drv, fp, offset, whence);
}

gt_fs_res_et gt_fs_tell(uint32_t * pos)
{
    gt_fs_drv_st * drv = _gt_fs_get_drv();
    gt_fs_fp_st * fp = gt_fs_get_fp();

    return drv->tell_cb(drv, fp, pos);
}

gt_fs_res_et gt_fs_write(uint8_t * buffer, uint32_t len, uint32_t * ret_len)
{
    gt_fs_drv_st * drv = _gt_fs_get_drv();
    gt_fs_fp_st * fp = gt_fs_get_fp();

    return drv->write_cb(drv, fp, buffer, len, ret_len);
}

gt_fs_res_et gt_fs_read_img_offset(uint8_t * res, uint32_t offset, uint32_t len)
{
    gt_fs_drv_st * drv = _gt_fs_get_drv();
    gt_fs_fp_st * fp = gt_fs_get_fp();
    uint32_t ret_len = 0;

    if (fp == NULL) {
        return GT_RES_FAIL;
    }
    drv->seek_cb(drv, fp, offset, GT_FS_SEEK_SET);
    drv->read_cb(drv, fp, res, len, &ret_len);

    return GT_RES_OK;
}

gt_fs_res_et gt_fs_read_img_wh(const char * path, uint16_t * w, uint16_t * h)
{
    if( !path ){
        return GT_FS_RES_FAIL;
    }
    gt_fs_fp_st * fp = gt_fs_open(path, GT_FS_MODE_RD);

    if (!fp) {
        return GT_RES_FAIL;
    }
    *w = fp->msg.pic.w;
    *h = fp->msg.pic.h;

    gt_fs_close();

    return GT_RES_OK;
}

void gt_fs_close(void)
{
    _gt_fs_dsc_st * dsc = _gt_fs_get_dsc();
    dsc->drv->close_cb(dsc->drv, dsc->fp);

    _gt_fs_free_dsc();
}

/* end ------------------------------------------------------------------*/
