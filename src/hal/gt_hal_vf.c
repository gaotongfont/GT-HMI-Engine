/**
 * @file gt_hal_vf.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-06-20 17:58:48
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_hal_vf.h"
#include "gt_hal_fs.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "../others/gt_types.h"
#include "stdbool.h"
#include "string.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/

/**
 * @brief virt file system device
 */
typedef struct _gt_vf_s {
    gt_fs_drv_st drv;     ///< file descriptor  driver
    uint32_t addr_max;      ///< flash max address range
    uint32_t addr_start;    ///< resource begin address in flash
}gt_vf_st;



/* static variables -----------------------------------------------------*/

static gt_vf_st * _vf_dev = NULL;
static gt_vfs_st const * _vfs = NULL;
static gt_fs_res_et _state = GT_FS_RES_FAIL;


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _gt_vf_set_fp_by_list(gt_fs_fp_st * fp, gt_vfs_st const * const item)
{
    fp->start = item->addr;
    fp->end = item->addr + item->size;
    fp->pos = fp->start;
    fp->msg.pic.w = item->w;
    fp->msg.pic.h = item->h;
    fp->msg.pic.is_alpha = item->is_alpha;
}

static void * _open_cb(struct _gt_fs_drv_s * drv, char * name, gt_fs_mode_et mode)
{
    uint16_t idx = 0;
    bool is_find = false;

    /* check drv state */
    if( _state == GT_FS_RES_DEINIT ){
        GT_LOGV(GT_LOG_TAG_GUI, "vf drv is deinit");
        return NULL;
    }

    /* find this file by name  */
    while( NULL != _vfs[idx].name ){
        if( strcmp( _vfs[idx].name, name ) == 0 ){
            is_find = true;
            break;
        }
        ++idx;
    }

    if( is_find == false ){
        GT_LOGV(GT_LOG_TAG_GUI, "can not find this file:%s", name);
        return NULL;
    }

    /* set vf ptr */
    gt_fs_fp_st * vfp = _gt_hal_fp_init();

    if(  _vfs[idx].addr > _vf_dev->addr_max ){
        GT_LOGW(GT_LOG_TAG_GUI, "out of GT_VF_FLASH_SIZE");
    }

    _gt_vf_set_fp_by_list(vfp, &_vfs[idx]);
    vfp->drv = gt_vf_get_drv();

    vfp->mode = mode;
    return vfp;
}

static void _close_cb(struct _gt_fs_drv_s * drv, void * fp)
{
    drv->seek_cb(drv, fp, 0, GT_FS_SEEK_SET);
    ((gt_fs_fp_st *)fp)->drv = NULL;
    gt_mem_free(fp);
}

static gt_fs_res_et _read_cb(struct _gt_fs_drv_s * drv, void * fp, uint8_t * data, uint32_t len, uint32_t * ret_len)
{
    gt_fs_fp_st * vfp = (gt_fs_fp_st * )fp;

    if( _state != GT_FS_RES_READY ){
        GT_LOGV(GT_LOG_TAG_GUI, "drv is busy");
        return GT_FS_RES_BUSY;
    }

    if( !drv ){
        GT_LOGW(GT_LOG_TAG_GUI, "drv is null,err code[%d]", GT_FS_RES_HW_ERR);
        return GT_FS_RES_HW_ERR;
    }

    if( !vfp ){
        GT_LOGW(GT_LOG_TAG_GUI, "fp is null");
        return GT_FS_RES_NULL;
    }
    uint32_t addr = vfp->pos + _vf_dev->addr_start;
    uint8_t data_write[4] = {0x03, (uint8_t)(addr>>16), (uint8_t)(addr>>8), (uint8_t)addr };

    /* set state busy */
    _state = GT_FS_RES_BUSY;

    /* start read */
    *ret_len = drv->rw_cb(data_write, 4, data, len);
    drv->seek_cb(drv, vfp, len, GT_FS_SEEK_CUR);

    /* set state ready */
    _state = GT_FS_RES_READY;

    return GT_FS_RES_OK;
}

static gt_fs_res_et _write_cb(struct _gt_fs_drv_s * drv, void * fp, uint8_t * data, uint32_t len, uint32_t * ret_len)
{
    gt_fs_fp_st * vfp = (gt_fs_fp_st * )fp;

    if( !drv ){
        GT_LOGW(GT_LOG_TAG_GUI, "drv is null,err code[%d]", GT_FS_RES_HW_ERR);
        return GT_FS_RES_HW_ERR;
    }

    if( _state != GT_FS_RES_READY ){
        GT_LOGV(GT_LOG_TAG_GUI, "drv is busy");
        return GT_FS_RES_BUSY;
    }

    if( !vfp ){
        GT_LOGW(GT_LOG_TAG_GUI, "fp is null");
        return GT_FS_RES_NULL;
    }

    /* set state busy */
    _state = GT_FS_RES_BUSY;

    /* start read */
    // uint32_t addr = vfp->start + vfp->pos + _vf_dev->addr_start;
    // uint8_t data_write[4] = {0x02, addr>>16, addr>>8, addr>>0 };
    // drv->rw_cb(data_write, 4, data, 0);

    /* write enable */
    // 0x06
    /*  */
    /* write start */

    /* write disable */

    /* set state ready */
    _state = GT_FS_RES_READY;

    *ret_len = len;

    return GT_FS_RES_OK;
}

static gt_fs_res_et _seek_cb(struct _gt_fs_drv_s * drv, void * fp, uint32_t pos, gt_fs_whence_et whence)
{
    gt_fs_fp_st * _fp = (gt_fs_fp_st * )fp;

    if( !_fp ){
        GT_LOGW(GT_LOG_TAG_GUI, "fp  is null");
        return GT_FS_RES_NULL;
    }

    switch (whence)
    {
        case GT_FS_SEEK_SET:
            _fp->pos = _fp->start + pos;
            break;
        case GT_FS_SEEK_CUR:
            _fp->pos += pos;
            break;
        case GT_FS_SEEK_END:
            _fp->pos = _fp->end - pos;
            break;
        default:
            break;
    }

    return GT_FS_RES_OK;
}

static gt_fs_res_et _tell_cb(struct _gt_fs_drv_s * drv, void * fp, uint32_t * pos)
{
    gt_fs_fp_st * _fp = (gt_fs_fp_st * )fp;
    if( !_fp ){
        GT_LOGW(GT_LOG_TAG_GUI, "fp is null");
        return GT_FS_RES_NULL;
    }
    *pos = _fp->pos - _fp->start;
    return GT_FS_RES_OK;
}

static void _gt_vf_drv_init(gt_fs_drv_st * drv)
{
    drv->letter       = GT_FS_LABEL_FLASH;

    drv->open_cb      = _open_cb;
    drv->close_cb     = _close_cb;
    drv->read_cb      = _read_cb;
    drv->write_cb     = _write_cb;
    drv->seek_cb      = _seek_cb;
    drv->tell_cb      = _tell_cb;
    drv->dir_open_cb  = NULL;
    drv->dir_read_cb  = NULL;
    drv->dir_close_cb = NULL;
}


/* global functions / API interface -------------------------------------*/
void gt_vf_init(const gt_vfs_st * vfs)
{
    _vfs = vfs;

    if( !_vf_dev ){
        _vf_dev = gt_mem_malloc( sizeof(gt_vf_st) );
    }

    /* set _vf_dev msg */
    _vf_dev->addr_max = GT_VF_FLASH_START + GT_VF_FLASH_SIZE;
    _vf_dev->addr_start = GT_VF_FLASH_START;

    _gt_vf_drv_init(&_vf_dev->drv);

    if( !_vf_dev->drv.rw_cb ){
        _state = GT_FS_RES_DEINIT;
    }else{
        _state = GT_FS_RES_READY;
    }
}

void gt_vf_drv_register(rw_cb_t rw_cb)
{
    gt_fs_drv_st * drv = gt_vf_get_drv();
    drv->rw_cb = rw_cb;

    if( !drv->rw_cb ){
        _state = GT_FS_RES_DEINIT;
    }else{
        _state = GT_FS_RES_READY;
    }
}

gt_fs_drv_st * gt_vf_get_drv(void)
{
    if (!_vf_dev) {
        GT_LOGW(GT_LOG_TAG_GUI, "_vf_dev is not init");
        return NULL;
    }

    return &_vf_dev->drv;
}

/* end ------------------------------------------------------------------*/
