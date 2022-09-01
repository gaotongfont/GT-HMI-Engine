/**
 * @file gt_hal_src.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-08-19 15:25:35
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_hal_src.h"
#include "stddef.h"
#include "../gt_conf.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/

typedef struct {
    gt_fs_drv_st drv;
    const gt_src_st * sys;    // source icon file list
    const uint8_t * data;     // source data buffer
    uint32_t data_len;
}_gt_src_dev_st;


/* static variables -----------------------------------------------------*/

static _gt_src_dev_st * _self = NULL;

/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static inline _gt_src_dev_st * _get_dev(void)
{
    return _self;
}

static inline gt_fs_drv_st * _get_drv(void)
{
    return _self ? &_self->drv : NULL;
}

/* -------------------------------------------- */

static void * _open_cb(struct _gt_fs_drv_s * drv, char * name, gt_fs_mode_et mode)
{
    uint16_t idx = 0;
    bool is_find = false;
    uint32_t addr = 0;
    _gt_src_dev_st * dev = _get_dev();

    GT_UNUSED(drv);

    if (!name) {
        return NULL;
    }

    while (NULL != dev->sys[idx].name) {
        if (!strcmp(name, dev->sys[idx].name)) {
            is_find = true;
            break;
        }
        ++idx;
        addr += dev->sys[idx].size;
    }

    if (!is_find) {
        GT_LOGV(GT_LOG_TAG_GUI, "Can not find this file: %s", name);
        return NULL;
    }

    if (addr > dev->data_len) {
        return NULL;
    }

    gt_fs_fp_st * fp = _gt_hal_fp_init();
    fp->start = addr;
    fp->end = fp->start + dev->sys[idx].size;
    fp->type = GT_FS_TYPE_ARRAY;
    fp->mode = mode;
    fp->pos = fp->start;

    return (void * )fp;
}

static void _close_cb(struct _gt_fs_drv_s * drv, void * fp)
{
    drv->seek_cb(drv, fp, 0, GT_FS_SEEK_SET);
}

static gt_fs_res_et _read_cb(struct _gt_fs_drv_s * drv, void * fp, uint8_t * data, uint32_t len, uint32_t * ret_len)
{
    gt_fs_res_et ret = GT_RES_OK;
    _gt_src_dev_st * dev = _get_dev();
    gt_fs_fp_st * file_p = (gt_fs_fp_st * )fp;

    gt_memcpy(data, &dev->data[file_p->pos], len);
    drv->seek_cb(drv, file_p, len, GT_FS_SEEK_CUR);
    *ret_len = len;

    return ret;
}

static gt_fs_res_et _seek_cb(struct _gt_fs_drv_s * drv, void * fp, uint32_t pos, gt_fs_whence_et whence)
{
    gt_fs_fp_st * file_p = (gt_fs_fp_st * )fp;

    if (!file_p) {
        return GT_RES_FAIL;
    }

    switch (whence) {
    case GT_FS_SEEK_SET: {
        file_p->pos = file_p->start + pos;
        break;
    }
    case GT_FS_SEEK_CUR: {
        file_p->pos += pos;
        break;
    }
    case GT_FS_SEEK_END: {
        file_p->pos = file_p->end - pos;
        break;
    }
    default:
        break;
    }

    return GT_RES_OK;
}

static gt_fs_res_et _tell_cb(struct _gt_fs_drv_s * drv, void * fp, uint32_t * pos)
{
    *pos = ((gt_fs_fp_st * )fp)->pos;

    return GT_RES_OK;
}

static void _gt_src_drv_register(gt_fs_drv_st * drv)
{
    drv->letter       = GT_FS_LABEL_ARRAY;

    drv->open_cb      = _open_cb;
    drv->close_cb     = _close_cb;
    drv->read_cb      = _read_cb;
    drv->seek_cb      = _seek_cb;
    drv->tell_cb      = _tell_cb;
    drv->write_cb     = NULL;
    drv->dir_open_cb  = NULL;
    drv->dir_read_cb  = NULL;
    drv->dir_close_cb = NULL;
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief Initializes the array mode file management system
 *
 * @param src_sys The list of resources system.
 * @param data The buffer which contains the resources details data.
 * @param data_len The length of the buffer.
 */
void gt_src_init(const gt_src_st * const src_sys, const uint8_t * const data, uint32_t data_len)
{
    if (!data) {
        GT_LOGW(GT_LOG_TAG_DATA, "src array is NULL, init failed.");
        return ;
    }

    if (!data_len) {
        GT_LOGW(GT_LOG_TAG_DATA, "src array length is 0, init failed.");
        return ;
    }

    if (!_self) {
        _self = (_gt_src_dev_st * )gt_mem_malloc(sizeof(_gt_src_dev_st));
        if (!_self) {
            GT_LOGW(GT_LOG_TAG_MEM, "mem malloc failed.");
            return ;
        }
        gt_memset_0(_self, sizeof(_gt_src_dev_st));
    }

    _self->sys = (const gt_src_st * )src_sys;
    _self->data = (const uint8_t * )data;
    _self->data_len = data_len;

    _gt_src_drv_register(&_self->drv);
}


gt_fs_drv_st * gt_src_get_drv(void)
{
    return _get_drv();
}


/* end ------------------------------------------------------------------*/
