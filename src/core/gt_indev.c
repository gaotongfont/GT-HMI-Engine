/**
 * @file gt_indev.c
 * @author yongg
 * @brief input device handler
 * @version 0.1
 * @date 2022-06-06 14:06:09
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

 /* include --------------------------------------------------------------*/
#include "gt_indev.h"
#include "stdbool.h"
#include "../hal/gt_hal_indev.h"
#include "../hal/gt_hal_disp.h"
#include "../hal/gt_hal_tick.h"
#include "../others/gt_log.h"
#include "../others/gt_math.h"
#include "gt_disp.h"
#include "gt_obj_pos.h"
#include "gt_obj_scroll.h"
#include "gt_style.h"
#include "../widgets/gt_dialog.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/




/* static variables -----------------------------------------------------*/

static gt_indev_param_st _indev_params = {
    .disabled = 0,
};

/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static inline bool _is_scroll_vertical(gt_dir_et dir) {
    return (GT_DIR_UP == dir || GT_DIR_DOWN == dir) ? true : false;
}

static gt_size_t _scroll_predict(gt_indev_st * indev, gt_size_t v)
{
    gt_size_t result = 0;
    if (NULL == indev) {
        return result;
    }

    gt_size_t limit_scroll_throw = indev->drv->limit_scroll_throw;
    uint32_t duration = gt_abs(gt_tick_get() - indev->proc.timestamp_start);
    while(v) {
        result += v;
        v = (100 - limit_scroll_throw) * v / 100;
    }

    if (0 == duration) {
        return result;
    }

    return (gt_size_t)(1000.0 * result / duration);
}

static gt_obj_st * _search_not_fixed_by_parent_recursive(gt_obj_st * obj) {
    gt_obj_st * ptr = obj;
    while(ptr) {
        if (false == ptr->fixed) {
            break;
        }
        ptr = ptr->parent;
    }
    return ptr;
}

static gt_dir_et _get_scroll_dir(gt_indev_st * indev) {
    struct _point * point_p = &indev->proc.data.point;
    gt_dir_et ret = (gt_dir_et)point_p->gesture;
    if (ret) {
        /** Locked the first direction */
        return ret;
    }

    if (gt_abs(point_p->scroll_sum.x) > gt_abs(point_p->scroll_sum.y)) {
        if (point_p->scroll_sum.x > indev->drv->limit_pixel_scroll) {
            ret = GT_DIR_RIGHT;
        } else if (point_p->scroll_sum.x < -indev->drv->limit_pixel_scroll) {
            ret = GT_DIR_LEFT;
        }
        point_p->scroll_throw.y = 0;
    } else if (point_p->scroll_sum.x || point_p->scroll_sum.y) {
        if (point_p->scroll_sum.y > indev->drv->limit_pixel_scroll) {
            ret = GT_DIR_DOWN;
        } else if (point_p->scroll_sum.y < -indev->drv->limit_pixel_scroll) {
            ret = GT_DIR_UP;
        }
        point_p->scroll_throw.x = 0;
    } else {
        ret = GT_DIR_NONE;
    }

    return ret;
}

static bool _is_indev_home_gesture(gt_indev_st * indev, gt_event_type_et* event)
{
    struct _point * point_p = &indev->proc.data.point;
    point_p->gesture = _get_scroll_dir(indev);

    if((uint16_t)GT_DIR_NONE == point_p->gesture) {
        return false;
    }

    if(GT_DIR_DOWN == point_p->gesture){
        // top
        if(GT_CFG_DEFAULT_AREA_HOME_GESTURE_TOP >= point_p->act.y){
            if(event) *event = GT_EVENT_TYPE_INPUT_HOME_GESTURE_TOP;
            return true;
        }
    }
    else if(GT_DIR_UP == point_p->gesture){
        // bottom
        if(GT_CFG_DEFAULT_AREA_HOME_GESTURE_BOTTOM >= GT_SCREEN_HEIGHT - point_p->act.y){
            if(event) *event = GT_EVENT_TYPE_INPUT_HOME_GESTURE_BOTTOM;
            return true;
        }
    }
    else if(GT_DIR_LEFT == point_p->gesture){
        // right
        if(GT_CFG_DEFAULT_AREA_HOME_GESTURE_RIGHT >= GT_SCREEN_WIDTH - point_p->act.x){
            if(event) *event = GT_EVENT_TYPE_INPUT_HOME_GESTURE_RIGHT;
            return true;
        }
    }
    else if(GT_DIR_RIGHT == point_p->gesture){
        // left
        if(GT_CFG_DEFAULT_AREA_HOME_GESTURE_LEFT >= point_p->act.x){
            if(event) *event = GT_EVENT_TYPE_INPUT_HOME_GESTURE_LEFT;
            return true;
        }
    }

    return false;
}

static bool _indev_send_home_gesture_event(gt_indev_st * indev)
{
    if((gt_tick_get() - indev->proc.timestamp_start) > GT_CFG_DEFAULT_POINT_HOME_GESTURE_TIME){
        return false;
    }

    gt_obj_st * scr = gt_disp_get_scr();

    gt_event_type_et e = GT_EVENT_TYPE_NONE;
    if(_is_indev_home_gesture(indev, &e)){
        gt_event_send(scr, e, NULL);
        return true;
    }

    return false;
}

/**
 * @brief Touch gestures make the act of throwing
 *
 * @param indev
 */
static void _indev_scroll_throw_handler(gt_indev_st * indev) {
    struct _point * point_p = &indev->proc.data.point;
    point_p->gesture = _get_scroll_dir(indev);

    if((uint16_t)GT_DIR_NONE == point_p->gesture) {
        return ;
    }

    //
    if(_indev_send_home_gesture_event(indev)){
        return ;
    }

    gt_obj_st * obj_scroll = point_p->obj_scroll;
    gt_size_t dx = _scroll_predict(indev, point_p->scroll_throw.x);
    gt_size_t dy = _scroll_predict(indev, point_p->scroll_throw.y);
    uint16_t w_parent = obj_scroll->parent->area.w;
    uint16_t h_parent = obj_scroll->parent->area.h;
    gt_size_t left = 0, right = GT_SCREEN_WIDTH;
    gt_size_t top = 0, bottom = GT_SCREEN_HEIGHT;

    if (obj_scroll->absorb) {
        if (false == obj_scroll->absorb_dir) {
            /** hor */
            gt_size_t x = obj_scroll->area.x + dx;  /** target x position */
            // outside the left
            if (x > left) {
                dx = -obj_scroll->area.x;
            }
            // outside the right
            else if (x + obj_scroll->area.w < right) {
                dx = right - obj_scroll->area.w - obj_scroll->area.x;
            }
            // inside
            else if (obj_scroll->scroll_snap_x) {
                if (0 == dx) {
                    // start algin
                    dx = (x / w_parent) * w_parent - x;
                    if (dx % w_parent  > (w_parent >> 1)) {
                        dx -= w_parent;
                    }
                }
                else if ((uint16_t)GT_DIR_LEFT == point_p->gesture) {
                    dx = ((x - w_parent) / w_parent) * w_parent - obj_scroll->area.x;
                } else if ((uint16_t)GT_DIR_RIGHT == point_p->gesture) {
                    dx = (x / w_parent) * w_parent - obj_scroll->area.x;
                }
            }

        } else {
            /** ver */
            gt_size_t y = obj_scroll->area.y + dy;
            // top
            if (y > top) {
                dy = -obj_scroll->area.y;
            }
            // bottom
            else if (y + obj_scroll->area.h < bottom) {
                dy = bottom - obj_scroll->area.h - obj_scroll->area.y;
            }
            else if (obj_scroll->scroll_snap_y) {
                if (0 == dy) {
                    // start algin
                    dy = (y / obj_scroll->parent->area.y) * obj_scroll->parent->area.y - y;
                    if (dy > (obj_scroll->parent->area.y >> 1)) {
                        dy -= obj_scroll->parent->area.y;
                    }
                }
                else if ((uint16_t)GT_DIR_UP == point_p->gesture) {
                    dy = ((y - h_parent) / h_parent) * h_parent - obj_scroll->area.y;
                } else if ((uint16_t)GT_DIR_DOWN == point_p->gesture) {
                    dy = (y / h_parent) * h_parent - obj_scroll->area.y;
                }
            }
        }
    }
    gt_obj_scroll_to(obj_scroll, dx, dy, GT_ANIM_ON);
}

/**
 * @brief Scroll when pressing
 *
 * @param indev
 */
static void _indev_scroll_handler(gt_indev_st * indev) {
    struct _point * point_p = &indev->proc.data.point;

    point_p->scroll_sum.x += point_p->scroll_diff.x;
    point_p->scroll_sum.y += point_p->scroll_diff.y;
    point_p->gesture = _get_scroll_dir(indev);

    if((uint16_t)GT_DIR_NONE == point_p->gesture) {
        return;
    }
    gt_size_t dx = 0;
    gt_size_t dy = 0;
    point_p->obj_scroll = point_p->obj_target;

    if(_is_indev_home_gesture(indev, NULL)){
        return ;
    }

    if (_is_scroll_vertical((gt_dir_et)point_p->gesture)) {
        dy = point_p->scroll_diff.y;
        point_p->scroll_throw.x = 0;
        gt_obj_set_scroll_ud(point_p->obj_scroll, GT_DIR_DOWN == point_p->gesture ? true : false);
    } else {
        dx = point_p->scroll_diff.x;
        point_p->scroll_throw.y = 0;
        gt_obj_set_scroll_lr(point_p->obj_scroll, GT_DIR_RIGHT == point_p->gesture ? true : false);
    }
    gt_obj_scroll_to(point_p->obj_scroll, dx, dy, GT_ANIM_OFF);
}

static inline void _send_released_event(struct _point * point_p) {
    gt_event_type_et event_type = GT_EVENT_TYPE_INPUT_RELEASED;

    if (point_p->obj_act != point_p->obj_target) {
        event_type = GT_EVENT_TYPE_INPUT_PRESS_LOST;
    }

    if (gt_obj_is_untouchability(point_p->obj_target)) {
        return;
    }

    gt_event_send(point_p->obj_target, event_type, NULL);

    if (point_p->obj_origin != point_p->obj_target) {
        gt_event_send(point_p->obj_origin, GT_EVENT_TYPE_INPUT_PRESS_LOST, NULL);
    }
}

static void _indev_released_handle(gt_indev_st * indev) {
    struct _point * point_p = &indev->proc.data.point;
    if (NULL == point_p->obj_target) {
        return;
    }

    _send_released_event(point_p);
    point_p->obj_act = NULL;

    if (point_p->obj_scroll) {
        _indev_scroll_throw_handler(indev);
    }
    point_p->obj_target = NULL;
    point_p->obj_origin = NULL;
}

/**
 * @brief Objects outside the control cannot be activated
 *
 * @return true can not be activated
 * @return false can be activated
 */
static bool _obj_outside_can_not_be_activated(void) {
    if (gt_dialog_has_showing()) {
        return true;
    }
    return false;
}

/**
 * @brief Search the active object from all layers
 *
 * @param point physical point
 * @param new_action true: new action, false: continue action
 * @return gt_obj_st* NULL or active object
 */
static gt_obj_st * _search_active_obj_from_all_layers(gt_point_st * point, bool new_action) {
    gt_obj_st * scr = gt_disp_get_scr();
    gt_obj_st * ret = NULL;
#if GT_USE_LAYER_TOP
    gt_obj_st * layer_top = gt_disp_get_layer_top();

    if (layer_top->cnt_child) {
        ret = gt_find_clicked_obj_by_point(layer_top, point);
        if (true == new_action && NULL == ret) {
            /** clicked dialog, etc. widget's outside area, hided top widgets */
            _gt_disp_hided_layer_top_widgets(layer_top);
            if (_obj_outside_can_not_be_activated()) {
                return ret;
            }
        }
    }
#endif
    if (NULL == ret) {
        /** Layer top have no widget find out, search it within screen */
        ret = gt_find_clicked_obj_by_point(scr, point);
    }
    return ret;
}

static void _indev_pressed_handle(gt_indev_st * indev) {
    gt_point_st point_ret;
    struct _point * point_p = &indev->proc.data.point;

    if (NULL == point_p->obj_target) {
        indev->proc.timestamp_start = gt_tick_get();
        indev->proc.timestamp_long_press = indev->proc.timestamp_start;

        _gt_indev_point_set_value(&point_p->scroll_sum, 0, 0);
        _gt_indev_point_set_value(&point_p->scroll_diff, 0, 0);
        /* save pressed obj */
        point_p->gesture = GT_DIR_NONE;
        point_p->obj_act = _search_active_obj_from_all_layers(&point_p->newly, true);
        if (NULL == point_p->obj_act) {
            return;
        }
        if (gt_obj_is_untouchability(point_p->obj_act)) {
            return;
        }
        point_p->obj_target = point_p->obj_act;
        point_p->obj_origin = point_p->obj_target;

        /* send clicking event to pressed obj */
        gt_obj_get_click_point_by_phy_point(point_p->obj_act, &point_p->newly, &point_ret);
        _gt_indev_point_set_value(&point_p->act, point_p->newly.x, point_p->newly.y);
        _gt_indev_point_set_value(&point_p->last, point_p->newly.x, point_p->newly.y);
        _gt_obj_set_process_point(point_p->obj_act, &point_ret);
        gt_event_send(point_p->obj_act, GT_EVENT_TYPE_INPUT_PRESSED, NULL);
    }

    if (point_p->obj_target && point_p->obj_scroll) {
        /** Reset scroll throw data, prepare for next throw. */
        point_p->scroll_throw.x = 0;
        point_p->scroll_throw.y = 0;
    }
    // GT_LOGD(">", "%p (%d, %d) (%d, %d)", point_p->obj_target, point_p->newly.x, point_p->newly.y, point_p->newly.x - point_p->last.x, point_p->newly.y - point_p->last.y);

    point_p->obj_act = _search_active_obj_from_all_layers(&point_p->newly, false);

    if (point_p->obj_act == point_p->obj_target) {
        /** Hold for a long time to trigger multiple times */
        gt_obj_get_click_point_by_phy_point(point_p->obj_target, &point_p->newly, &point_ret);
        _gt_obj_set_process_point(point_p->obj_target, &point_ret);
        if( (gt_tick_get() - indev->proc.timestamp_long_press) >= indev->drv->limit_timers_long_press ){
            indev->proc.timestamp_long_press = gt_tick_get();
            gt_event_send(point_p->obj_target, GT_EVENT_TYPE_INPUT_PRESSING, NULL);
        }
    }
    if (point_p->obj_target->fixed && point_p->obj_scroll) {
        /** if widget fixed, then search it parent's widget to scroll */
        if (point_p->obj_scroll == point_p->obj_target) {
            point_p->obj_target = _search_not_fixed_by_parent_recursive(point_p->obj_target);
        }
    }

    point_p->scroll_diff.x = point_p->newly.x - point_p->last.x;
    point_p->scroll_diff.y = point_p->newly.y - point_p->last.y;

    point_p->scroll_throw.x = (point_p->scroll_throw.x << 2) >> 3;
    point_p->scroll_throw.y = (point_p->scroll_throw.y << 2) >> 3;

    point_p->scroll_throw.x += (point_p->scroll_diff.x << 2) >> 3;
    point_p->scroll_throw.y += (point_p->scroll_diff.y << 2) >> 3;

    if (NULL == point_p->obj_target) {
        return;
    }
    _indev_scroll_handler(indev);
}

static void _gt_indev_handler_point(gt_indev_st * indev)
{
    struct _point * point_p = &indev->proc.data.point;
    gt_indev_data_st data_indev = {
        .point.x = point_p->newly.x,
        .point.y = point_p->newly.y,
        .state = GT_INDEV_STATE_RELEASED,
    };

    /* call user hard interface */
    indev->drv->read_cb(indev->drv, &data_indev);

    if (data_indev.point.x > GT_SCREEN_WIDTH || data_indev.point.y > GT_SCREEN_HEIGHT) {
        // GT_LOGW(GT_LOG_TAG_TP, "point out of range: x:%d, y:%d", data_indev.point.x, data_indev.point.y);
        return;
    }

    /* process point data */
    indev->proc.state = data_indev.state;

    _indev_params.param.point.x = data_indev.point.x;
    _indev_params.param.point.y = data_indev.point.y;
    _gt_indev_point_set_value(&point_p->newly, data_indev.point.x, data_indev.point.y);

    /* have one touch event */
    if (GT_INDEV_STATE_RELEASED == indev->proc.state) {
        _indev_released_handle(indev);
    } else {
        _indev_pressed_handle(indev);
    }

    _gt_indev_point_set_value(&point_p->last, point_p->newly.x, point_p->newly.y);
}

static void _gt_indev_handler_button(gt_indev_st * indev)
{
    static uint8_t flag_key_down = 0;
    static uint32_t last_run = 0;
    gt_indev_data_st data_indev = {
        .btn_id = indev->proc.data.button.id,
        .state = GT_INDEV_STATE_RELEASED,
    };
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

static void _gt_indev_handler_keypad(gt_indev_st* indev)
{
    gt_indev_data_st data_indev = {
        .key = GT_KEY_NONE,
        .state = GT_INDEV_STATE_RELEASED,
    };
    static uint32_t last_key = GT_KEY_NONE;
    gt_obj_st * scr = gt_disp_get_scr();

    indev->drv->read_cb(indev->drv, &data_indev);
    indev->proc.state = data_indev.state;
    indev->proc.data.keypad.obj_target = gt_find_clicked_obj_by_focus(scr);

    if(GT_INDEV_STATE_RELEASED == indev->proc.state){
        if(indev->proc.data.keypad.count_keydown != 0 && GT_KEY_ENTER == indev->proc.data.keypad.key){
            _indev_params.param.keypad_key = GT_KEY_ENTER;
            if (false == gt_obj_is_untouchability(indev->proc.data.keypad.obj_target)) {
                gt_event_send(indev->proc.data.keypad.obj_target, GT_EVENT_TYPE_INPUT_RELEASED, &_indev_params);
            }
        }
        indev->proc.data.keypad.count_keydown = 0;
        last_key = GT_KEY_NONE;
        return ;
    }
    // 新的按键
    if(GT_KEY_NONE == last_key || last_key != data_indev.key)
    {
        indev->proc.data.keypad.key = data_indev.key;
        last_key = data_indev.key;
        indev->proc.data.keypad.count_keydown = 0;
        if(GT_KEY_NEXT == indev->proc.data.keypad.key){
            gt_obj_next_focus_change(indev->proc.data.keypad.obj_target);
            _indev_params.param.keypad_key = GT_KEY_NEXT;
            gt_event_send(indev->proc.data.keypad.obj_target, GT_EVENT_TYPE_INPUT_FOCUSED, &_indev_params);
        }
        else if(GT_KEY_PREV == indev->proc.data.keypad.key){
            gt_obj_prev_focus_change(indev->proc.data.keypad.obj_target);
            _indev_params.param.keypad_key = GT_KEY_PREV;
            gt_event_send(indev->proc.data.keypad.obj_target, GT_EVENT_TYPE_INPUT_FOCUSED, &_indev_params);
        }
        else{
            _indev_params.param.keypad_key = indev->proc.data.keypad.key;
            gt_event_send(indev->proc.data.keypad.obj_target, GT_EVENT_TYPE_INPUT_KEY, &_indev_params);
            if (GT_KEY_ENTER == indev->proc.data.keypad.key &&
                false == gt_obj_is_untouchability(indev->proc.data.keypad.obj_target)){
                gt_event_send(indev->proc.data.keypad.obj_target, GT_EVENT_TYPE_INPUT_PRESSED, &_indev_params);
            }
        }
        // GT_LOGD(GT_LOG_TAG_GUI, "keypad the key = %#X  count_keydown = %d obj type = %d", indev->proc.data.keypad.key, indev->proc.data.keypad.count_keydown, indev->proc.data.keypad.obj_target->class->type);
        return ;
    }
    // 长按
    indev->proc.data.keypad.count_keydown++;
    if(indev->proc.data.keypad.count_keydown >= (GT_CFG_DEFAULT_POINT_LONG_PRESS_TIMERS / GT_TASK_PERIOD_TIME_EVENT)){
        if(GT_KEY_NEXT == indev->proc.data.keypad.key){
            gt_obj_next_focus_change(indev->proc.data.keypad.obj_target);
            _indev_params.param.keypad_key = GT_KEY_NEXT;
            gt_event_send(indev->proc.data.keypad.obj_target, GT_EVENT_TYPE_INPUT_FOCUSED, &_indev_params);
        }
        else if(GT_KEY_PREV == indev->proc.data.keypad.key){
            gt_obj_prev_focus_change(indev->proc.data.keypad.obj_target);
            _indev_params.param.keypad_key = GT_KEY_PREV;
            gt_event_send(indev->proc.data.keypad.obj_target, GT_EVENT_TYPE_INPUT_FOCUSED, &_indev_params);
        }
        else{
            _indev_params.param.keypad_key = indev->proc.data.keypad.key;
            gt_event_send(indev->proc.data.keypad.obj_target, GT_EVENT_TYPE_INPUT_KEY, &_indev_params);
            if(0 == indev->proc.data.keypad.count_keydown % (GT_CFG_DEFAULT_POINT_LONG_PRESS_TIMERS / GT_TASK_PERIOD_TIME_EVENT)){
                if (GT_KEY_ENTER == indev->proc.data.keypad.key &&
                    false == gt_obj_is_untouchability(indev->proc.data.keypad.obj_target)){
                    gt_event_send(indev->proc.data.keypad.obj_target, GT_EVENT_TYPE_INPUT_PRESSING, &_indev_params);
                }
            }
        }
        // GT_LOGD(GT_LOG_TAG_GUI, "long, key = %#X  count_keydown = %d", indev->proc.data.keypad.key, indev->proc.data.keypad.count_keydown);
    }
}

/* global functions / API interface -------------------------------------*/
uint32_t gt_indev_get_key(void)
{
    return _indev_params.param.keypad_key;
}

gt_point_st gt_indev_get_point(void)
{
    return _indev_params.param.point;
}

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
    return (gt_dir_et)indev->proc.data.point.gesture;
}

void gt_indev_handler(struct _gt_timer_s * timer)
{
    gt_indev_st * indev;
    uint8_t i, cnt_indev;

    if (gt_indev_is_disabled()) {
        return;
    }

    cnt_indev = gt_indev_get_dev_count();
    for(i=0; i<cnt_indev; i++){
        indev = gt_indev_get_dev_by_idx(i);
        switch(indev->drv->type){
            case GT_INDEV_TYPE_POINTER:
                _gt_indev_handler_point(indev);
                break;
            case GT_INDEV_TYPE_BUTTON:
                _gt_indev_handler_button(indev);
                break;
            case GT_INDEV_TYPE_KEYPAD:
                _gt_indev_handler_keypad(indev);
                break;
            default:
                break;
        }
    }
}

void gt_indev_set_disabled(bool disabled)
{
    _indev_params.disabled = disabled;
}

bool gt_indev_is_disabled(void)
{
    return _indev_params.disabled;
}

bool _gt_indev_remove_want_delate_target(gt_obj_st * target)
{
    gt_indev_st * indev = NULL;
    struct _point * point_p = &indev->proc.data.point;
    uint8_t i, cnt_indev = gt_indev_get_dev_count();
    bool ret = false;

    for (i = 0; i < cnt_indev; i++) {
        indev = gt_indev_get_dev_by_idx(i);
        point_p = &indev->proc.data.point;

        if (target != point_p->obj_target) {
            continue;
        }
        if (false == target->delate) {
            continue;
        }
        point_p->obj_target = NULL;
        ret = true;
    }
    return ret;
}

/* end ------------------------------------------------------------------*/
