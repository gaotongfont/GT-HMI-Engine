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
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "stdbool.h"
#include "string.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/

static gt_vf_st * vfdev;
static gt_vfs_st * _vfs;
static gt_vfp_st * _vfp;
static gt_vf_res_e _state;


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
void (*_gt_vf_read)(uint8_t * data_write, uint32_t len_write, uint8_t * data_read, uint32_t len_read);

static void * _open_cb(struct _gt_vf_drv_s * drv, char * name){
    uint32_t idx = 0;
    bool is_suc = false;

    /* check drv state */
    if( _state == GT_VF_RES_DEINIT ){
        GT_LOGV(GT_LOG_TAG_GUI, "vf drv is deinit");
        return NULL;
    }

    /* find this file by name  */
    while( NULL != _vfs[idx].name ){
        if( strcmp( _vfs[idx].name, name ) == 0 ){
            is_suc = true;
            break;
        }
        idx++;
    }

    if( is_suc == false ){
        GT_LOGV(GT_LOG_TAG_GUI, "can not find this file:%s", name);
        return NULL;
    }

    /* set vf ptr */
    if( !_vfp ){
        _vfp = gt_mem_malloc( sizeof(gt_vfp_st) );
    }

    if( _vfs[idx].addr > vfdev->addr_max ){
        GT_LOGW(GT_LOG_TAG_GUI, "out of GT_VF_FLASH_SIZE");
    }

    _vfp->start = _vfs[idx].addr;
    _vfp->end = _vfs[idx].addr + _vfs[idx].size;
    _vfp->pos = 0;
    _vfp->msg.pic.w = _vfs[idx].w;
    _vfp->msg.pic.h = _vfs[idx].h;
    return _vfp;
}

static void _close_cb(struct _gt_vf_drv_s * drv, void * fp){
    /* code */
    drv->seek_cb(drv, fp, 0, GT_VF_SEEK_SET);
}
 
static gt_vf_res_e _read_cb(struct _gt_vf_drv_s * drv, void * fp, uint8_t * data, uint32_t len){
    gt_vfp_st * _vfp = fp;

    if( _state != GT_VF_RES_READY ){
        GT_LOGV(GT_LOG_TAG_GUI, "drv is busy");
        return GT_VF_RES_BUSY;
    }

    if( !drv ){
        GT_LOGW(GT_LOG_TAG_GUI, "drv is null,err code[%d]", GT_VF_RES_HW_ERR);
        return GT_VF_RES_HW_ERR;
    }

    if( !_vfp ){
        GT_LOGW(GT_LOG_TAG_GUI, "fp is null");
        return GT_VF_RES_NULL;
    }

    /* set state busy */
    _state = GT_VF_RES_BUSY;

    /* start read */
    uint32_t addr = _vfp->start + _vfp->pos;
    uint8_t data_write[4] = {0x03, addr>>16, addr>>8, addr>>0 };
    drv->rw_cb(data_write, 4, data, len);
    drv->seek_cb(drv, _vfp, len, GT_VF_SEEK_CUR);

    /* set state ready */
    _state = GT_VF_RES_READY;

    return GT_VF_RES_OK;
}

static gt_vf_res_e _write_cb(struct _gt_vf_drv_s * drv, void * fp, uint8_t * data, uint32_t len){
    gt_vfp_st * _vfp = fp;


    if( !drv ){
        GT_LOGW(GT_LOG_TAG_GUI, "drv is null,err code[%d]", GT_VF_RES_HW_ERR);
        return GT_VF_RES_HW_ERR;
    }

    if( _state != GT_VF_RES_READY ){
        GT_LOGV(GT_LOG_TAG_GUI, "drv is busy");
        return GT_VF_RES_BUSY;
    }

    if( !_vfp ){
        GT_LOGW(GT_LOG_TAG_GUI, "fp is null");
        return GT_VF_RES_NULL;
    }

    /* set state busy */
    _state = GT_VF_RES_BUSY;

    /* start read */
    // uint32_t addr = _vfp->start + _vfp->pos + vfdev->addr_start;
    // uint8_t data_write[4] = {0x02, addr>>16, addr>>8, addr>>0 };
    // drv->rw_cb(data_write, 4, data, 0);

    /* write enable */
    // 0x06
    /*  */
    /* write start */

    /* write disable */

    /* set state ready */
    _state = GT_VF_RES_READY;

    return GT_VF_RES_OK;
}

static gt_vf_res_e _seek_cb(struct _gt_vf_drv_s * drv, void * fp, uint32_t pos, gt_vf_whence_e whence){
    gt_vfp_st * _fp = fp;

    if( !_fp ){
        GT_LOGW(GT_LOG_TAG_GUI, "fp  is null");
        return GT_VF_RES_NULL;
    }

    switch (whence)
    {
        case GT_VF_SEEK_SET:
            _fp->pos = pos;
            break;
        case GT_VF_SEEK_CUR:
            _fp->pos = _fp->pos + pos;
            break;
        case GT_VF_SEEK_END:
            _fp->pos = _fp->end - pos;
            break;
        default:
            break;
    }

    return GT_VF_RES_OK;
}

static gt_vf_res_e _tell_cb(struct _gt_vf_drv_s * drv, void * fp, uint32_t * pos){
    gt_vfp_st * _fp = fp;
    if( !_fp ){
        GT_LOGW(GT_LOG_TAG_GUI, "fp is null");
        return GT_VF_RES_NULL;
    }
    *pos = _fp->pos;
    return GT_VF_RES_OK;
}


/* global functions / API interface -------------------------------------*/
void gt_vf_init(gt_vfs_st * vfs){
    _vfs = vfs;

    if( !vfdev ){
        vfdev = gt_mem_malloc( sizeof(gt_vf_st) );
    }

    /* set vfdev msg */
    vfdev->addr_max = GT_VF_FLASH_START + GT_VF_FLASH_SIZE;
    vfdev->addr_start = GT_VF_FLASH_START;

    if( !vfdev->drv ){
        _state = GT_VF_RES_DEINIT;
    }else{
        _state = GT_VF_RES_READY;
    }
}

void gt_vf_drv_register(gt_vf_drv_st * drv){

    drv->open_cb = _open_cb;
    drv->close_cb = _close_cb;
    drv->read_cb = _read_cb;
    drv->write_cb = _write_cb;
    drv->seek_cb = _seek_cb;
    drv->tell_cb = _tell_cb;
    vfdev->drv = drv;
    if( !_vfs ){
        _state = GT_VF_RES_DEINIT;
    }else{
        _state = GT_VF_RES_READY;
    }
}


gt_vf_st * gt_vf_get_default(){
    if( !vfdev ){
        GT_LOGW(GT_LOG_TAG_GUI, "vfdev is not init");
    }
    return vfdev;
}

/* end ------------------------------------------------------------------*/
