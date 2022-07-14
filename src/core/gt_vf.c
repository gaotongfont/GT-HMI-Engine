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

void gt_vf_read_img(const char * path, uint8_t * res){
    gt_vf_st * vf_dev = gt_vf_get_default();
    gt_vfp_st * vfp = vf_dev->drv->open_cb(vf_dev->drv, (char *)path);
    vf_dev->drv->read_cb(vf_dev->drv, vfp, res, vfp->msg.pic.w * vfp->msg.pic.h * sizeof(gt_color_t) );
}

void gt_vf_read_img_offset(const char * path, uint8_t * res, uint32_t offset, uint32_t len){
    gt_vf_st * vf_dev = gt_vf_get_default();
    gt_vfp_st * vfp = vf_dev->drv->open_cb(vf_dev->drv, (char *)path);
    vf_dev->drv->seek_cb(vf_dev->drv, vfp, offset, GT_VF_SEEK_SET);
    vf_dev->drv->read_cb(vf_dev->drv, vfp, res, len);
}



/* end ------------------------------------------------------------------*/
