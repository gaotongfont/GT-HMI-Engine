/**
 * @file gt_hal_src.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-08-19 15:25:35
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_hal_src.h"
#include "stddef.h"
#include "../gt_conf.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"


/* private define -------------------------------------------------------*/
#if GT_USE_MODE_SRC


/* private typedef ------------------------------------------------------*/

typedef struct {
    gt_fs_drv_st drv;
    const gt_src_st * sys;    // source icon file list
    gt_size_t sys_count;
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

        if (++idx > dev->sys_count) {
            break;
        }
    }

    if (!is_find) {
        GT_LOGV(GT_LOG_TAG_GUI, "Can not find this file: %s", name);
        return NULL;
    }

    gt_fs_fp_st * fp = _gt_hal_fp_init();
    GT_CHECK_BACK_VAL(fp, NULL);
    fp->start = 0;
    fp->end = dev->sys[idx].size;
    fp->type = GT_FS_TYPE_ARRAY;
    fp->mode = mode;
    fp->pos = fp->start;
    fp->buffer = (void * )dev->sys[idx].img;

    fp->msg.pic.w = dev->sys[idx].w;
    fp->msg.pic.h = dev->sys[idx].h;
    fp->msg.pic.is_alpha = dev->sys[idx].is_alpha;
    fp->drv = &dev->drv;

    return (void * )fp;
}

static void _close_cb(struct _gt_fs_drv_s * drv, void * fp)
{
    drv->seek_cb(drv, fp, 0, GT_FS_SEEK_SET);
    gt_mem_free(fp);
}

static gt_fs_res_et _read_cb(struct _gt_fs_drv_s * drv, void * fp, uint8_t * data, uint32_t len, uint32_t * ret_len)
{
    gt_fs_res_et ret = GT_FS_RES_OK;
    _gt_src_dev_st * dev = _get_dev();
    gt_fs_fp_st * file_p = (gt_fs_fp_st * )fp;
    const uint8_t *buffer = (const uint8_t * )file_p->buffer;

    gt_memcpy(data, &buffer[file_p->pos], len);
    drv->seek_cb(drv, file_p, len, GT_FS_SEEK_CUR);
    *ret_len = len;

    return ret;
}

static gt_fs_res_et _seek_cb(struct _gt_fs_drv_s * drv, void * fp, uint32_t pos, gt_fs_whence_et whence)
{
    gt_fs_fp_st * file_p = (gt_fs_fp_st * )fp;

    if (!file_p) {
        return GT_FS_RES_FAIL;
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

    return GT_FS_RES_OK;
}

static gt_fs_res_et _tell_cb(struct _gt_fs_drv_s * drv, void * fp, uint32_t * pos)
{
    gt_fs_fp_st * file_p = (gt_fs_fp_st * )fp;

    *pos = file_p->pos - file_p->start;

    return GT_FS_RES_OK;
}

#if GT_USE_FS_NAME_BY_INDEX
static char const * const _get_name_by_cb(uint16_t index_of_list) {
    uint16_t idx = 0;
    char * ret_name = NULL;
    _gt_src_dev_st * dev = _get_dev();

    while (NULL != dev->sys[idx].name) {
        if (index_of_list == idx) {
            ret_name = dev->sys[idx].name;
            break;
        }
        ++idx;
    }
    return ret_name;
}
#endif

static void _gt_src_drv_register(gt_fs_drv_st * drv)
{
    drv->letter       = GT_FS_LABEL_ARRAY;

#if GT_USE_FILE_HEADER
    drv->fh_open_cb   = NULL;
#endif
    drv->open_cb      = _open_cb;
    drv->close_cb     = _close_cb;
    drv->read_cb      = _read_cb;
    drv->seek_cb      = _seek_cb;
    drv->tell_cb      = _tell_cb;
    drv->write_cb     = NULL;
#if GT_USE_FOLDER_SYSTEM
    drv->dir_open_cb  = NULL;
    drv->dir_read_cb  = NULL;
    drv->dir_close_cb = NULL;
#endif
#if GT_USE_FS_NAME_BY_INDEX
    drv->get_name_by_cb = _get_name_by_cb;
#endif
}

#endif  /** GT_USE_MODE_SRC */
/* global functions / API interface -------------------------------------*/

void gt_src_init(const gt_src_st * const src_sys, uint32_t sys_count)
{
#if GT_USE_MODE_SRC
    if (!src_sys) {
        GT_LOGW(GT_LOG_TAG_DATA, "src array is NULL, init failed.");
        return ;
    }

    if (!sys_count) {
        GT_LOGW(GT_LOG_TAG_DATA, "src array length is 0, init failed.");
        return ;
    }

    if (!_self) {
        _self = (_gt_src_dev_st * )gt_mem_malloc(sizeof(_gt_src_dev_st));
        GT_CHECK_BACK(_self);
        gt_memset_0(_self, sizeof(_gt_src_dev_st));
    }

    _self->sys = (const gt_src_st * )src_sys;
    _self->sys_count = sys_count;

    _gt_src_drv_register(&_self->drv);
#endif
}

#if GT_USE_MODE_SRC
gt_fs_drv_st * gt_src_get_drv(void)
{
    return _get_drv();
}
#endif


/* end ------------------------------------------------------------------*/
