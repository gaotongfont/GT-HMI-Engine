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
#include "../hal/gt_hal_tick.h"
#include "../others/gt_log.h"
#include "../others/gt_math.h"
#include "gt_disp.h"
#include "gt_obj_pos.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
void gt_indev_handler_point(gt_indev_st * indev)
{
    static uint8_t flag_key_down = 0;
    static uint32_t last_run = 0;
    gt_indev_data_st data_indev;

    gt_obj_st * scr = gt_disp_get_scr();
    gt_point_st point_ret;
    struct _point * point_p = &indev->proc.data.point;
    gt_size_t ver = 0, hor = 0;

    /* call user hard interface */
    indev->drv->read_cb(indev->drv, &data_indev);

    /* process point data */
    indev->proc.state = data_indev.state;

    /* have one touch event */
    if (GT_INDEV_STATE_RELEASED == indev->proc.state) {
        if (!flag_key_down) {
            return;    /* no touch */
        }
        gt_event_type_et event_type = (point_p->obj_act == point_p->obj_last) ? \
            GT_EVENT_TYPE_INPUT_PROCESSED : GT_EVENT_TYPE_INPUT_PROCESS_LOST;

        gt_event_send(point_p->obj_act, event_type, NULL);
        flag_key_down = 0;
        point_p->count_point = 0;
        return;
    }

    /* first key down */
    if (!flag_key_down) {
        flag_key_down = 1;
        last_run = gt_tick_get();
        /* save first pressed point */
        _gt_indev_point_set_value(&point_p->act, data_indev.point.x, data_indev.point.y);

        /* save pressed obj */
        point_p->obj_act = gt_find_clicked_obj_by_point(scr, &data_indev.point);
        point_p->obj_last = point_p->obj_act;
        point_p->obj_scroll = point_p->obj_act;

        /* send clicking event to pressed obj */
        gt_obj_get_click_point_by_phy_point(point_p->obj_act, &data_indev.point,&point_ret);
        _gt_obj_set_process_point(point_p->obj_act, &point_ret);
        gt_event_send(point_p->obj_act, GT_EVENT_TYPE_INPUT_CLICKING, NULL);
        return;
    }

    /* last pressed point */
    _gt_indev_point_set_value(&point_p->last, point_p->newly.x, point_p->newly.y);
    _gt_indev_point_set_value(&point_p->newly, data_indev.point.x, data_indev.point.y);
    ++point_p->count_point;

    point_p->obj_last = gt_find_clicked_obj_by_point(scr, &data_indev.point);

    if (point_p->obj_act == point_p->obj_last) {
        gt_obj_get_click_point_by_phy_point(point_p->obj_act, &point_p->newly,&point_ret);
        _gt_obj_set_process_point(point_p->obj_act, &point_ret);
        if( (gt_tick_get()-last_run) >= indev->drv->limit_timers_long_press ){
            last_run = gt_tick_get();
            gt_event_send(point_p->obj_act, GT_EVENT_TYPE_INPUT_CLICKING, NULL);
        }
    }

    if (_gt_indev_point_equal(&point_p->newly, &point_p->last)) {
        goto long_press_lb;
    }

    /* gesture */
    if (point_p->count_point > 3) {

        hor = point_p->newly.x - point_p->last.x;
        ver = point_p->newly.y - point_p->last.y;

        if (ver < -indev->drv->limit_pixel_gesture_ver) {
            GT_LOGD(GT_LOG_TAG_GUI, "slide up\n");
            point_p->gesture = GT_DIR_UP;
        }
        else if (ver > indev->drv->limit_pixel_gesture_ver) {
            GT_LOGD(GT_LOG_TAG_GUI, "slide down\n");
            point_p->gesture = GT_DIR_DOWN;
        }
        else if (hor < -indev->drv->limit_pixel_gesture_hor) {
            GT_LOGD(GT_LOG_TAG_GUI, "slide left\n");
            point_p->gesture = GT_DIR_LEFT;
        }
        else if (hor > indev->drv->limit_pixel_gesture_hor) {
            GT_LOGD(GT_LOG_TAG_GUI, "slide right\n");
            point_p->gesture = GT_DIR_RIGHT;
        }
        else {
            point_p->gesture = GT_DIR_NONE;
        }

        if( point_p->gesture != GT_DIR_NONE ){
            _gt_indev_point_set_value(&point_p->last, point_p->newly.x, point_p->newly.y);
        }
    }

    /* scroll screen */
    if ((gt_size_t)gt_abs(ver) > indev->drv->limit_pixel_scroll ||
        (gt_size_t)gt_abs(hor) > indev->drv->limit_pixel_scroll) {
        _gt_obj_set_process_scroll_xy(point_p->obj_scroll, hor, ver);
        gt_event_send(point_p->obj_scroll, GT_EVENT_TYPE_INPUT_SCROLL, NULL);
    }
    if( gt_disp_get_scr() == point_p->obj_scroll ){
        _gt_indev_point_set_value(&point_p->newly, data_indev.point.x, data_indev.point.y);
    }
    /* if have gesture,no process click event */
    if(GT_DIR_NONE != point_p->gesture) {
        GT_LOGD(GT_LOG_TAG_GUI, "send scroll code");
        gt_event_send(point_p->obj_last, GT_EVENT_TYPE_INPUT_SCROLL, NULL);
    }

long_press_lb:
    if ( 0 == (point_p->count_point % indev->drv->limit_timers_long_press) ) {
        /* trigger an touch event */
        point_p->count_point = 0;
        GT_LOGD(GT_LOG_TAG_GUI, "continue mode trigger an touch event");
    }
}

void gt_indev_handler_button(gt_indev_st * indev)
{
    static uint8_t flag_key_down = 0;
    static uint32_t last_run = 0;
    gt_indev_data_st data_indev;
    indev->drv->read_cb(indev->drv, &data_indev);

    indev->proc.state = data_indev.state;
    if (indev->proc.state == GT_INDEV_STATE_PRESSED) {
        if (!flag_key_down) {
            flag_key_down = 1;
            last_run = gt_tick_get();
            indev->proc.data.button.id = data_indev.btn_id;
            GT_LOGV(GT_LOG_TAG_GUI, "keydown the key id[%d]", indev->proc.data.button.id);
        }
        indev->proc.data.button.count_keydown++;
    }
    else {
        flag_key_down = 0;
        indev->proc.data.button.count_keydown = 0;
    }

    if (indev->proc.data.button.count_keydown != 0 && (gt_tick_get() - last_run) % indev->drv->limit_timers_long_press == 0) {
        /* trigger an button event:continue mode */
        indev->proc.data.button.count_keydown = 0;
        GT_LOGV(GT_LOG_TAG_GUI, "continue mode trigger an button event, key id[%d]",indev->proc.data.button.id );
    }
}


/* global functions / API interface -------------------------------------*/

void gt_indev_get_point_act(const gt_indev_st * indev, gt_point_st * point)
{
    point->x = indev->proc.data.point.act.x;
    point->y = indev->proc.data.point.act.y;
}

void gt_indev_get_point_newly(const gt_indev_st * indev, gt_point_st * point)
{
    point->x = indev->proc.data.point.newly.x;
    point->y = indev->proc.data.point.newly.y;
}

gt_dir_et gt_indev_get_gesture(const gt_indev_st * indev) {
    return indev->proc.data.point.gesture;
}

void gt_indev_handler()
{
    gt_indev_st * indev;
    uint8_t i, cnt_indev;
    cnt_indev = gt_indev_get_dev_count();
    for(i=0; i<cnt_indev; i++){
        indev = gt_indev_get_dev_by_idx(i);
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
