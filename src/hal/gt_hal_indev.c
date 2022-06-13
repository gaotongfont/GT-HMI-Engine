/**
 * @file gt_hal_indev.c
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-05-19 17:35:39
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_hal_indev.h"
#include "../others/gt_log.h"
#include "../core/gt_mem.h"
/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static gt_indev_st * indev;


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_indev_drv_init(gt_indev_drv_st * drv){
    drv->type = GT_INDEV_TYPE_NONE;

    drv->limit_pixel_gesture_hor = GT_CFG_DEFAULT_POINT_GERSTURE_PIXEL_HOR;
    drv->limit_pixel_gesture_ver = GT_CFG_DEFAULT_POINT_GERSTURE_PIXEL_VER;
    drv->limit_pixel_scroll = GT_CFG_DEFAULT_POINT_SCROLL_PIXEL;
    drv->limit_timers_long_press = GT_CFG_DEFAULT_POINT_LONG_PRESS_TIMERS;
}

gt_indev_st * gt_indev_drv_register(gt_indev_drv_st * drv){
    if(indev == NULL){
        indev = gt_mem_malloc(sizeof(gt_indev_st));
    }

    if(drv->read_cb == NULL){
        GT_LOGW(GT_LOG_TAG_GUI, "[error]drv read_cb is null,please check!");
        return NULL;
    }

    GT_LOGV(GT_LOG_TAG_GUI, "register indev type:%d", drv->type);

    indev->drv = drv;
    return indev;
}

gt_indev_st * gt_indev_get_default(){
    return indev;
}

/* end ------------------------------------------------------------------*/
