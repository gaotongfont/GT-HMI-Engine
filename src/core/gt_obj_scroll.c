/**
 * @file gt_obj_scroll.c
 * @author yongg
 * @brief Scroll bar animation
 * @version 0.1
 * @date 2022-08-31 14:25:17
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_obj_scroll.h"
#include "gt_timer.h"
#include "../hal/gt_hal_indev.h"
#include "../hal/gt_hal_disp.h"
#include "../others/gt_anim.h"
#include "./gt_style.h"
#include "../others/gt_log.h"
#include "./gt_disp.h"

/* private define -------------------------------------------------------*/
#define _SCROLL_ANIM_TIME_MIN    200    /*ms*/
#define _SCROLL_ANIM_TIME_MAX    400    /*ms*/


/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static GT_ATTRIBUTE_RAM_TEXT void _scroll_by_raw(gt_obj_st * obj, int32_t x, int32_t y) {
    _gt_obj_set_process_scroll_xy(obj, x, y);
    gt_event_send(obj, GT_EVENT_TYPE_INPUT_SCROLL, NULL);
}

static GT_ATTRIBUTE_RAM_TEXT void _scroll_by_x(void * obj, int32_t x) {
    _gt_obj_set_process_scroll_xy((gt_obj_st * )obj, x - ((gt_obj_st * )obj)->process_attr.scroll.x, 0);
    gt_event_send((gt_obj_st * )obj, GT_EVENT_TYPE_INPUT_SCROLL, NULL);
}

static GT_ATTRIBUTE_RAM_TEXT void _scroll_by_y(void * obj, int32_t y) {
    _gt_obj_set_process_scroll_xy((gt_obj_st * )obj, 0, y - ((gt_obj_st * )obj)->process_attr.scroll.y);
    gt_event_send((gt_obj_st * )obj, GT_EVENT_TYPE_INPUT_SCROLL, NULL);
}

static GT_ATTRIBUTE_RAM_TEXT void _scroll_ready_cb(struct gt_anim_s * anim) {
    gt_event_send((gt_obj_st * )anim->tar, GT_EVENT_TYPE_INPUT_SCROLL_END, NULL);
}

/* global functions / API interface -------------------------------------*/
void gt_obj_scroll_to(struct gt_obj_s * obj, gt_size_t dx, gt_size_t dy, gt_anim_enable_et en)
{
    if (gt_obj_is_type(obj, GT_TYPE_UNKNOWN)) {
        GT_LOGE(GT_LOG_TAG_CHECK, "Unvaild obj type: %p(%d)", obj, gt_obj_class_get_type(obj));
        return;
    }
    if (0 == dx && 0 == dy) {
        return;
    }

    if (GT_ANIM_ON == en) {
        gt_anim_st anim;
        gt_anim_init(&anim);
        gt_anim_set_target(&anim, obj);
        gt_anim_set_ready_cb(&anim, _scroll_ready_cb);
        gt_anim_set_path_type(&anim, GT_ANIM_PATH_TYPE_EASE_OUT);

        if (dx) {
            int32_t d = gt_anim_speed_to_time(gt_disp_get_res_hor(NULL) >> 1, 0, dx);
            if (d < _SCROLL_ANIM_TIME_MIN) d = _SCROLL_ANIM_TIME_MIN;
            if (d > _SCROLL_ANIM_TIME_MAX) d = _SCROLL_ANIM_TIME_MAX;
#if GT_USE_WIDGET_DYNAMIC_FRAME_RATE
            if (GT_OBJ_FRAME_RATE_LOW == obj->frame_rate) {
                d >>= 2;
            } else if (GT_OBJ_FRAME_RATE_MIDDLE == obj->frame_rate) {
                d >>= 1;
            }
#endif
            gt_anim_set_time(&anim, d);
            gt_anim_set_value(&anim, obj->process_attr.scroll.x, obj->process_attr.scroll.x + dx);
            gt_anim_set_exec_cb(&anim, _scroll_by_x);
            gt_event_send(obj, GT_EVENT_TYPE_INPUT_SCROLL_START, NULL);
            gt_anim_start(&anim);
        }

        if (dy) {
            int32_t d = gt_anim_speed_to_time(gt_disp_get_res_ver(NULL) >> 1, 0, dy);
            if (d < _SCROLL_ANIM_TIME_MIN) d = _SCROLL_ANIM_TIME_MIN;
            if (d > _SCROLL_ANIM_TIME_MAX) d = _SCROLL_ANIM_TIME_MAX;
#if GT_USE_WIDGET_DYNAMIC_FRAME_RATE
            if (GT_OBJ_FRAME_RATE_LOW == obj->frame_rate) {
                d >>= 2;
            } else if (GT_OBJ_FRAME_RATE_MIDDLE == obj->frame_rate) {
                d >>= 1;
            }
#endif
            gt_anim_set_time(&anim, d);
            gt_anim_set_value(&anim, obj->process_attr.scroll.y, obj->process_attr.scroll.y + dy);
            gt_anim_set_exec_cb(&anim, _scroll_by_y);
            gt_event_send(obj, GT_EVENT_TYPE_INPUT_SCROLL_START, NULL);
            gt_anim_start(&anim);
        }
        return;
    }

    gt_anim_del(obj, _scroll_by_x);
    gt_anim_del(obj, _scroll_by_y);

    _scroll_by_raw(obj, dx, dy);
}

void gt_obj_scroll_to_x(struct gt_obj_s * obj, gt_size_t x, gt_anim_enable_et en)
{
    gt_obj_scroll_to(obj, x, 0, en);
}


void gt_obj_scroll_to_y(struct gt_obj_s * obj, gt_size_t y, gt_anim_enable_et en)
{
    gt_obj_scroll_to(obj, 0, y, en);
}

gt_size_t gt_obj_scroll_get_x(gt_obj_st * obj)
{
    if (gt_obj_is_type(obj, GT_TYPE_UNKNOWN)) {
        return 0;
    }
    return obj->process_attr.scroll.x;
}

gt_size_t gt_obj_scroll_get_y(gt_obj_st * obj)
{
    if (gt_obj_is_type(obj, GT_TYPE_UNKNOWN)) {
        return 0;
    }
    return obj->process_attr.scroll.y;
}

void _gt_obj_scroll_internal(gt_obj_st * obj, gt_size_t offset_x, gt_size_t offset_y)
{
    gt_point_st * scroll = &obj->process_attr.scroll;
    gt_point_st * prev = &obj->process_attr.scroll_prev;
    gt_padding_st padding = gt_obj_get_limit_padding(obj);
    padding.right += offset_x;
    padding.bottom += offset_y;

    gt_size_t diff_y = scroll->y - prev->y;
    gt_size_t diff_x = scroll->x - prev->x;

    if(scroll->y > 0) {
        scroll->y = 0;
    }
    else if(padding.top >= obj->area.y && padding.bottom <= (obj->area.y + obj->area.h)) {
        scroll->y = (0 == prev->y) ? 0 : padding.top - obj->area.y;
    }
    else if((padding.bottom + diff_y) < obj->area.y + obj->area.h ||
            (((padding.bottom + diff_y) == obj->area.y + obj->area.h) && (scroll->y < prev->y))) {

        scroll->y = prev->y + ((obj->area.y + obj->area.h - padding.bottom));
    }

    if(scroll->x > 0) {
        scroll->x = 0;
    }
    else if(padding.left >= obj->area.x && padding.right <= (obj->area.x + obj->area.w)) {
        scroll->x = (0 == prev->x) ? 0 : padding.left - obj->area.x;
    }
    else if((padding.right + diff_x) < obj->area.x + obj->area.w ||
            (((padding.right + diff_x) == obj->area.x + obj->area.w) && (scroll->x < prev->x))) {

        scroll->x = prev->x + ((obj->area.x + obj->area.w - padding.right));
    }

    if(scroll->x != prev->x || scroll->y != prev->y) {
        _gt_obj_move_child_by(obj, scroll->x - prev->x, scroll->y - prev->y);
        gt_disp_invalid_area(obj);

        if(scroll->x != prev->x) {
            prev->x = scroll->x;
        }
        if(scroll->y != prev->y) {
            prev->y = scroll->y;
        }
    }
}


/**
 * @brief set obj process point data
 *
 * @param obj obj
 * @param point point ptr
 */
void _gt_obj_set_process_point(gt_obj_st * obj, gt_point_st * point)
{
    if (gt_obj_is_type(obj, GT_TYPE_UNKNOWN)) {
        return ;
    }
    obj->process_attr.point.x = point->x;
    obj->process_attr.point.y = point->y;
}

/**
 * @brief set obj process scroll xy
 *
 * @param obj
 * @param x_scroll
 * @param y_scroll
 */
void _gt_obj_set_process_scroll_xy(gt_obj_st * obj, gt_size_t x_scroll, gt_size_t y_scroll)
{
    if (gt_obj_is_type(obj, GT_TYPE_UNKNOWN)) {
        return ;
    }
    obj->process_attr.scroll.x += x_scroll;
    obj->process_attr.scroll.y += y_scroll;
}

/* end ------------------------------------------------------------------*/
