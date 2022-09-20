/**
 * @file gt_indev.c
 * @author yongg
 * @brief input device handler
 * @version 0.1
 * @date 2022-06-06 14:06:09
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

 /* include --------------------------------------------------------------*/
#include "gt_indev.h"
#include "../hal/gt_hal_indev.h"
#include "../hal/gt_hal_disp.h"
#include "../others/gt_log.h"
#include "gt_disp.h"
#include "gt_obj_pos.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
void gt_indev_handler_point(gt_indev_st * indev)
{
    static uint8_t flag_key_down = 0; //cnt_key_down_continue = 0;
    gt_indev_data_st data_indev;

    gt_obj_st * scr = gt_disp_get_scr();
    gt_point_st point_ret;

    /* call user hard interface */
    indev->drv->read_cb(indev->drv, &data_indev);

    /* process point data */
    indev->proc.state = data_indev.state;

    /* have one touch event */
    if (indev->proc.state == GT_INDEV_STATE_PRESSED) {
        /* first key down */
        if (!flag_key_down) {
            flag_key_down = 1;
            /* save first pressed point */
            indev->proc.data.point.point_act.x = data_indev.point.x;
            indev->proc.data.point.point_act.y = data_indev.point.y;

            /* save pressed point for scroll process */
            indev->proc.data.point.point_temp.x = data_indev.point.x;
            indev->proc.data.point.point_temp.y = data_indev.point.y;

            /* save pressed obj */
            indev->proc.data.point.obj_act = gt_find_clicked_obj_by_point(scr, &data_indev.point);
            indev->proc.data.point.obj_scroll = indev->proc.data.point.obj_act;

            /* send clicking event to pressed obj */
            // _gt_obj_set_process_point(indev->proc.data.point.obj_act, &indev->proc.data.point.point_act);
            gt_obj_get_click_point_by_phy_point(indev->proc.data.point.obj_act, &data_indev.point,&point_ret);
            _gt_obj_set_process_point(indev->proc.data.point.obj_act, &point_ret);
            gt_event_send(indev->proc.data.point.obj_act, GT_EVENT_TYPE_INPUT_CLICKING, NULL);
            return;
        }else {
            /* last pressed point */
            indev->proc.data.point.point_last.x = data_indev.point.x;
            indev->proc.data.point.point_last.y = data_indev.point.y;
        }

        indev->proc.data.point.obj_last = gt_find_clicked_obj_by_point(scr, &data_indev.point);

        if( indev->proc.data.point.obj_act != indev->proc.data.point.obj_last ){
            gt_event_send(indev->proc.data.point.obj_act, GT_EVENT_TYPE_INPUT_PROCESS, NULL);
            indev->proc.data.point.obj_act = indev->proc.data.point.obj_last;
        }else{
            gt_obj_get_click_point_by_phy_point(indev->proc.data.point.obj_act, &indev->proc.data.point.point_last,&point_ret);
            _gt_obj_set_process_point(indev->proc.data.point.obj_act, &point_ret);
            if( indev->proc.data.point.count_point > indev->drv->limit_timers_long_press ){
                gt_event_send(indev->proc.data.point.obj_act, GT_EVENT_TYPE_INPUT_CLICKING, NULL);
            }
        }
        indev->proc.data.point.count_point++;
    }
    /* no touch */
    else {
        if( flag_key_down ){
            if( indev->proc.data.point.obj_last ){
                gt_event_send( indev->proc.data.point.obj_last, GT_EVENT_TYPE_INPUT_PROCESS, NULL);
            }
        }
        flag_key_down = 0;
        indev->proc.data.point.count_point = 0;

        return ;
    }

    /* gesture */
    int ver = 0, hor = 0;
    if (indev->proc.data.point.count_point > 3) {

        ver = indev->proc.data.point.point_last.y - indev->proc.data.point.point_temp.y;
        hor = indev->proc.data.point.point_last.x - indev->proc.data.point.point_temp.x;

        if (ver < -indev->drv->limit_pixel_gesture_ver) {
            GT_LOGD(GT_LOG_TAG_GUI, "slide up\n");
            indev->proc.data.point.gesture = GT_DIR_UP;
        }
        else if (ver > indev->drv->limit_pixel_gesture_ver) {
            GT_LOGD(GT_LOG_TAG_GUI, "slide down\n");
            indev->proc.data.point.gesture = GT_DIR_DOWN;
        }
        else if (hor < -indev->drv->limit_pixel_gesture_hor) {
            GT_LOGD(GT_LOG_TAG_GUI, "slide left\n");
            indev->proc.data.point.gesture = GT_DIR_LEFT;
        }
        else if (hor > indev->drv->limit_pixel_gesture_hor) {
            GT_LOGD(GT_LOG_TAG_GUI, "slide right\n");
            indev->proc.data.point.gesture = GT_DIR_RIGHT;
        }
        else {
            indev->proc.data.point.gesture = GT_DIR_NONE;
        }

        if( indev->proc.data.point.gesture != GT_DIR_NONE ){
            indev->proc.data.point.point_temp.x = indev->proc.data.point.point_last.x;
            indev->proc.data.point.point_temp.y = indev->proc.data.point.point_last.y;
        }

    }

    /* scroll screen */
    if( gt_disp_get_scr() == indev->proc.data.point.obj_scroll ){
        // if( ver > indev->drv->limit_pixel_scroll || ver < -indev->drv->limit_pixel_scroll ){
            // gt_disp_scroll_area_act(GT_DIR_VER, ver/-2);
        // }else if( hor > indev->drv->limit_pixel_scroll || hor < -indev->drv->limit_pixel_scroll ) {
        //     gt_disp_scroll_area_act(GT_DIR_HOR, hor/-2);
        // }
        indev->proc.data.point.point_last.x = data_indev.point.x;
        indev->proc.data.point.point_last.y = data_indev.point.y;
        return;
    }else{
        if( ver > indev->drv->limit_pixel_scroll || ver < -indev->drv->limit_pixel_scroll ){
            _gt_obj_set_process_scroll_xy(indev->proc.data.point.obj_scroll, 0, ver);
            gt_event_send(indev->proc.data.point.obj_scroll, GT_EVENT_TYPE_INPUT_SCROLL, NULL);
        }else if( hor > indev->drv->limit_pixel_scroll || hor < -indev->drv->limit_pixel_scroll ) {
            _gt_obj_set_process_scroll_xy(indev->proc.data.point.obj_scroll, hor, 0);
            gt_event_send(indev->proc.data.point.obj_scroll, GT_EVENT_TYPE_INPUT_SCROLL, NULL);
        }
        return;
    }

    /* if have gesture,no process click event */
    if( indev->proc.data.point.gesture != GT_DIR_NONE ){

        GT_LOGD(GT_LOG_TAG_GUI, "send scroll code");
        gt_event_send(indev->proc.data.point.obj_last, GT_EVENT_TYPE_INPUT_SCROLL, NULL);
        return;
    }

    if (indev->proc.data.point.count_point != 0 && indev->proc.data.point.count_point % indev->drv->limit_timers_long_press == 0) {
        /* trigger an touch event */
        indev->proc.data.point.count_point = 0;
        GT_LOGD(GT_LOG_TAG_GUI, "continue mode trigger an touch event");
    }
}

void gt_indev_handler_button(gt_indev_st * indev)
{
    static uint8_t flag_key_down = 0;

    gt_indev_data_st data_indev;
    indev->drv->read_cb(indev->drv, &data_indev);

    indev->proc.state = data_indev.state;
    if (indev->proc.state == GT_INDEV_STATE_PRESSED) {
        if (!flag_key_down) {
            flag_key_down = 1;
            indev->proc.data.button.id = data_indev.btn_id;
            GT_LOGV(GT_LOG_TAG_GUI, "keydown the key id[%d]", indev->proc.data.button.id);
        }
        indev->proc.data.button.count_keydown++;
    }
    else {
        flag_key_down = 0;
        indev->proc.data.button.count_keydown = 0;
    }

    if (indev->proc.data.button.count_keydown != 0 && indev->proc.data.button.count_keydown % indev->drv->limit_timers_long_press == 0) {
        /* trigger an button event:continue mode */
        indev->proc.data.button.count_keydown = 0;
        GT_LOGV(GT_LOG_TAG_GUI, "continue mode trigger an button event, key id[%d]",indev->proc.data.button.id );
    }
}


/* global functions / API interface -------------------------------------*/

void gt_indev_get_point_act(const gt_indev_st * indev, gt_point_st * point)
{
    point->x = indev->proc.data.point.point_act.x;
    point->y = indev->proc.data.point.point_act.y;
}

void gt_indev_get_point_last(const gt_indev_st * indev, gt_point_st * point)
{
    point->x = indev->proc.data.point.point_last.x;
    point->y = indev->proc.data.point.point_last.y;
    printf("1111:x:%d,y:%d\n", point->x, point->y);
}

gt_dir_et gt_indev_get_gesture(const gt_indev_st * indev) {
    return indev->proc.data.point.gesture;
}

void gt_indev_handler()
{
    gt_indev_st * indev;
    uint8_t i, cnt_indev;
    cnt_indev = gt_indev_get_dev_num();
    for(i=0; i<cnt_indev; i++){
        indev = gt_indev_get_dev_by_nub(i);
        switch(indev->drv->type){
            case GT_INDEV_TYPE_POINTER:
                gt_indev_handler_point(indev);
                break;
            case GT_INDEV_TYPE_BUTTON:
                gt_indev_handler_button(indev);
                break;
            case GT_INDEV_TYPE_KEYPAD:
                break;
            default:
                break;
        }
    }
}

/* end ------------------------------------------------------------------*/
