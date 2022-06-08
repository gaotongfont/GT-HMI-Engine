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

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void  gt_indev_handler_point(const gt_indev_st * indev){
    static uint8_t flag_key_down = 0, cnt_key_down_continue = 0;
    
    gt_indev_data_st data_indev;
    indev->drv->read_cb(indev->drv, &data_indev);
    
//    indev->proc.state = data_indev.state;
    if( indev->proc.state == GT_INDEV_STATE_PRESSED ){
        flag_key_down = 1;
//        indev->proc.data.point.count_point++;
    }else{
        flag_key_down = 0;        
    }

    /* find touch obj */
//    gt_obj_st * obj = gt_;

    if( indev->proc.data.point.count_point%indev->cnt_repeat == 0 ){
        /* trigger an event */
        
    }
}


/* global functions / API interface -------------------------------------*/


void gt_indev_get_point(const gt_indev_st * indev, gt_point_st * point){
    point->x = indev->proc.data.point.point_act.x;
    point->y = indev->proc.data.point.point_act.y;
}

gt_dir_et gt_indev_get_gesture(const gt_indev_st * indev){
    return indev->proc.data.point.gesture;
}



/* end ------------------------------------------------------------------*/
