/**
 * @file gt_vf.c
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-07-05 19:15:15
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_vf.h"
#include "../hal/gt_hal_vf.h"
#include "../others/gt_color.h"
#include "../others/gt_log.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

gt_res_t gt_vf_read_img(const char * path, uint8_t * res){
    gt_vf_st * vf_dev = gt_vf_get_default();
    gt_vfp_st * vfp = vf_dev->drv->open_cb(vf_dev->drv, (char *)path);
    if( vfp == NULL){
        return GT_RES_FAIL;
    }
    vf_dev->drv->read_cb(vf_dev->drv, vfp, res, vfp->msg.pic.w * vfp->msg.pic.h * sizeof(gt_color_t) );
}

gt_res_t gt_vf_read_img_offset(const char * path, uint8_t * res, uint32_t offset, uint32_t len){
    gt_vf_st * vf_dev = gt_vf_get_default();
    gt_vfp_st * vfp = vf_dev->drv->open_cb(vf_dev->drv, (char *)path);
    if( vfp == NULL){
        return GT_RES_FAIL;
    }
    vf_dev->drv->seek_cb(vf_dev->drv, vfp, offset, GT_VF_SEEK_SET);
    vf_dev->drv->read_cb(vf_dev->drv, vfp, res, len);
}

gt_res_t gt_vf_read_img_wh(const char * path, uint16_t * w, uint16_t * h){
    gt_vf_st * vf_dev = gt_vf_get_default();
    gt_vfp_st * vfp = vf_dev->drv->open_cb(vf_dev->drv, (char *)path);
    if( vfp == NULL){
        return GT_RES_FAIL;
    }
    *w = vfp->msg.pic.w;
    *h = vfp->msg.pic.h;
}

/* end ------------------------------------------------------------------*/
