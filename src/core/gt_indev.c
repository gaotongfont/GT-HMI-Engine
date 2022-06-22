/**
 * @file gt_indev.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-06-06 14:06:09
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

 /* include --------------------------------------------------------------*/
#include "gt_indev.h"
#include "../hal/gt_hal_indev.h"
#include "../hal/gt_hal_disp.h"
#include "../others/gt_log.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
void  gt_indev_handler_point(gt_indev_st * indev) {
    static uint8_t flag_key_down = 0, cnt_key_down_continue = 0;

    gt_indev_data_st data_indev;
    indev->drv->read_cb(indev->drv, &data_indev);

    indev->proc.state = data_indev.state;
    if (indev->proc.state == GT_INDEV_STATE_PRESSED) {
        if (!flag_key_down) {
            flag_key_down = 1;
            indev->proc.data.point.point_act.x = data_indev.point.x;
            indev->proc.data.point.point_act.y = data_indev.point.y;
        }
        else {
            indev->proc.data.point.point_last.x = data_indev.point.x;
            indev->proc.data.point.point_last.y = data_indev.point.y;
        }
        indev->proc.data.point.count_point++;
    }
    else {
        flag_key_down = 0;
        indev->proc.data.point.count_point = 0;
    }

    /* find touch obj */
//    gt_obj_st * obj = gt_;

    int ver = 0, hor = 0;
    if (indev->proc.data.point.count_point > 3) {
        ver = indev->proc.data.point.point_last.y - indev->proc.data.point.point_act.y;
        hor = indev->proc.data.point.point_last.x - indev->proc.data.point.point_act.x;

        if (ver < -indev->drv->limit_pixel_gesture_ver) {
            GT_LOGV(GT_LOG_TAG_GUI, "slide up\n");
            indev->proc.data.point.gesture = GT_DIR_UP;
        }
        else if (ver > indev->drv->limit_pixel_gesture_ver) {
            GT_LOGV(GT_LOG_TAG_GUI, "slide down\n");
            indev->proc.data.point.gesture = GT_DIR_DOWN;
        }
        else if (hor < -indev->drv->limit_pixel_gesture_hor) {
            GT_LOGV(GT_LOG_TAG_GUI, "slide left\n");
            indev->proc.data.point.gesture = GT_DIR_LEFT;
        }
        else if (hor > indev->drv->limit_pixel_gesture_hor) {
            GT_LOGV(GT_LOG_TAG_GUI, "slide right\n");
            indev->proc.data.point.gesture = GT_DIR_RIGHT;
        }
        else {
            indev->proc.data.point.gesture = GT_DIR_NONE;
        }
    }

    if (indev->proc.data.point.count_point != 0 && indev->proc.data.point.count_point % indev->drv->limit_timers_long_press == 0) {
        /* trigger an touch event */
        indev->proc.data.point.count_point = 0;
        GT_LOGV(GT_LOG_TAG_GUI, "trigger an event\n");
    }
}


/* global functions / API interface -------------------------------------*/


void gt_indev_get_point(const gt_indev_st * indev, gt_point_st * point) {
    point->x = indev->proc.data.point.point_act.x;
    point->y = indev->proc.data.point.point_act.y;
}

gt_dir_et gt_indev_get_gesture(const gt_indev_st * indev) {
    return indev->proc.data.point.gesture;
}



/* end ------------------------------------------------------------------*/
