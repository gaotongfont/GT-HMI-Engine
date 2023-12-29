/**
 * @file gt_hal_indev.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-19 17:35:39
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_hal_indev.h"
#include "../others/gt_log.h"
#include "../core/gt_mem.h"
#include "../core/gt_timer.h"

#include "../core/gt_indev.h"

/* private define -------------------------------------------------------*/
#define _CNT_INDEV_MAX  5


/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static gt_indev_st * indev;
static gt_indev_st * indev_act[_CNT_INDEV_MAX];
static uint8_t cnt_indev = 0;


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_indev_drv_init(gt_indev_drv_st * drv)
{

    gt_memset(drv, 0, sizeof(gt_indev_drv_st));

    drv->type = GT_INDEV_TYPE_NONE;

    drv->limit_timers_long_press = GT_CFG_DEFAULT_POINT_LONG_PRESS_TIMERS;
    drv->limit_pixel_gesture_hor = GT_CFG_DEFAULT_POINT_GERSTURE_PIXEL_HOR;
    drv->limit_pixel_gesture_ver = GT_CFG_DEFAULT_POINT_GERSTURE_PIXEL_VER;
    drv->limit_pixel_scroll      = GT_CFG_DEFAULT_POINT_SCROLL_PIXEL;
    drv->limit_scroll_throw      = GT_CFG_DEFAULT_POINT_SCROLL_THROW;
}

gt_indev_st * gt_indev_drv_register(gt_indev_drv_st * drv)
{
    if (!drv) {
        GT_LOGW(GT_LOG_TAG_GUI, "[error]drv is null,please check!");
        return NULL;
    }

    if(drv->read_cb == NULL){
        GT_LOGW(GT_LOG_TAG_GUI, "[error]drv read_cb is null,please check!");
        return NULL;
    }

    if(indev_act[cnt_indev] == NULL){
        indev_act[cnt_indev] = gt_mem_malloc(sizeof(gt_indev_st));
        if (!indev_act[cnt_indev]) {
            return NULL;
        }
        gt_memset_0(indev_act[cnt_indev], sizeof(gt_indev_st));
    }

    GT_LOGV(GT_LOG_TAG_GUI, "addr:%p", indev_act[cnt_indev]);
    GT_LOGV(GT_LOG_TAG_GUI, "register indev type:%d", drv->type);

    indev_act[cnt_indev]->drv = drv;

    if( cnt_indev == 0 ){
        _gt_timer_create(gt_indev_handler, GT_TASK_PERIOD_TIME_INDEV, (void *)indev_act);
    }
    cnt_indev++;

    return indev_act[cnt_indev];
}

void gt_indev_set_default(gt_indev_st * _indev)
{
    indev = _indev;
}

gt_indev_st * gt_indev_get_default()
{
    return indev;
}

gt_indev_st * gt_indev_get_dev_by_idx(uint8_t idx)
{
    if( idx >= _CNT_INDEV_MAX ){
        GT_LOGW(GT_LOG_TAG_GUI, "idx is over number max of indev");
        return NULL;
    }
    return indev_act[idx];
}

uint8_t gt_indev_get_dev_count(void)
{
    return cnt_indev;
}



/* end ------------------------------------------------------------------*/
