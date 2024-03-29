/**
 * @file gt_obj.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-11 15:03:47
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "../core/gt_obj_pos.h"
#include "../core/gt_timer.h"
#include "../core/gt_disp.h"
#include "../core/gt_mem.h"
#include "../core/gt_style.h"
#include "../core/gt_disp.h"
#include "../core/gt_obj_scroll.h"
#include "../hal/gt_hal_disp.h"
#include "../others/gt_log.h"


/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_OBJ
#define MY_CLASS    &gt_obj_class

#define OBJ_TYPE_SCREEN     GT_TYPE_SCREEN
#define MY_CLASS_SCREEN     &gt_screen_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_style_obj_s {
    gt_color_t bgcolor;
}_gt_style_obj_st;

/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(gt_obj_st * obj, struct _gt_event_s * e);

static const gt_obj_class_st gt_obj_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_style_obj_st)
};

static const gt_obj_class_st gt_screen_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = GT_TYPE_SCREEN,
    .size_style    = sizeof(_gt_style_obj_st)
};

/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj) {
}

static void _deinit_cb(gt_obj_st * obj) {
}

static void _event_cb(gt_obj_st * obj, struct _gt_event_s * e)
{
    gt_event_type_et code = gt_event_get_code(e);
    gt_area_abs_st * max_area_p = gt_disp_get_area_max();

    switch(code) {
        case GT_EVENT_TYPE_DRAW_START: {
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;
        }
        case GT_EVENT_TYPE_UPDATE_STYLE: {
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            gt_disp_invalid_area(obj);
            break;
        }
        case GT_EVENT_TYPE_INPUT_SCROLL: {
            /** temp */
            if (-obj->process_attr.scroll.x < max_area_p->left) {
                obj->process_attr.scroll.x = -max_area_p->left;
            }
            else if (-obj->process_attr.scroll.x + obj->area.w > max_area_p->right) {
                obj->process_attr.scroll.x = -max_area_p->right + obj->area.w;
            }
            if (-obj->process_attr.scroll.y < max_area_p->top) {
                obj->process_attr.scroll.y = -max_area_p->top;
            }else if (-obj->process_attr.scroll.y + obj->area.h > max_area_p->bottom) {
                obj->process_attr.scroll.y = -max_area_p->bottom + obj->area.h;
            }
            gt_disp_scroll_area_act(-obj->process_attr.scroll.x, -obj->process_attr.scroll.y);
            break;
        }
        case GT_EVENT_TYPE_INPUT_RELEASED: {
            /* click event finish */
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;
        }
        case GT_EVENT_TYPE_INPUT_SCROLL_UP: {
            /** scroll 3/4 screen */
            gt_obj_scroll_to_y(obj, (gt_disp_get_res_ver(NULL) * 3) >> 2, GT_ANIM_ON);
            break;
        }
        case GT_EVENT_TYPE_INPUT_SCROLL_DOWN: {
            gt_obj_scroll_to_y(obj, (-gt_disp_get_res_ver(NULL) * 3) >> 2, GT_ANIM_ON);
            break;
        }
        case GT_EVENT_TYPE_INPUT_SCROLL_LEFT: {
            gt_obj_scroll_to_x(obj, (gt_disp_get_res_hor(NULL) * 3) >> 2, GT_ANIM_ON);
            break;
        }
        case GT_EVENT_TYPE_INPUT_SCROLL_RIGHT: {
            gt_obj_scroll_to_x(obj, (-gt_disp_get_res_hor(NULL) * 3) >> 2, GT_ANIM_ON);
        }
        default:
            break;
    }
}

/**
 * @brief destroy the object by core timer handler,
 * Free the memory of the object and its children.
 *
 * @param timer
 */
void _gt_obj_destroy_handler_cb(struct _gt_timer_s * timer)
{
    gt_obj_st * obj = (gt_obj_st * )_gt_timer_get_user_data(timer);

    if (gt_event_is_locked() && gt_event_get_locked_obj() == obj) {
        GT_LOGW(GT_LOG_TAG_GUI, "Disables the removal of the control itself in the callback.");
        _gt_timer_set_repeat_count(timer, 1);   // retry to destroy object
        return ;
    }

    if (obj->using) {
        GT_LOGW("obj", "Try to delete an object that is in use.");
        _gt_timer_set_repeat_count(timer, 1);   // retry to destroy object
        return;
    }

    // release children memory and remove itself from parent children array
#if GT_USE_DISPLAY_PREF_DESTROY
    uint32_t start = gt_tick_get();
#endif

    obj->delate = 0;
    _gt_obj_class_destroy(obj);
    _gt_timer_set_user_data(timer, NULL);

#if GT_USE_DISPLAY_PREF_DESTROY
    GT_LOGI(GT_LOG_TAG_MEM, "destroy obj: %p, used: %d ms", obj, gt_tick_get() - start);
#endif
}

/* global functions / API interface -------------------------------------*/



gt_obj_st * gt_obj_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(parent ? MY_CLASS : MY_CLASS_SCREEN, parent);
    obj->bgcolor = gt_color_hex(0xFFFFFF);
    return obj;
}

gt_obj_st * gt_obj_change_parent(gt_obj_st * obj, gt_obj_st * to)
{
    return _gt_obj_class_change_parent(obj, to);
}

bool gt_obj_is_child(gt_obj_st * obj, gt_obj_st * parent)
{
    if (NULL == parent || NULL == obj) {
        return false;
    }
    uint16_t i, cnt = parent->cnt_child;
    gt_obj_st * child = NULL;
    for (i = 0; i < cnt; i++) {
        child = parent->child[i];
        if (child == obj) {
            return true;
        }
        if (true == gt_obj_is_child(obj, child)) {
            return true;
        }
    }
    return false;
}

void gt_obj_destroy(gt_obj_st * obj)
{
    // GT_LOGV(GT_LOG_TAG_GUI, "Want to destroy obj: 0x%p", obj);
    if (!obj) { return ; }
    obj->delate = 1;

    // create a temp timer once task to destroy the object
    _gt_timer_st * tmp_timer = _gt_timer_create(_gt_obj_destroy_handler_cb, GT_TASK_PERIOD_TIME_DESTROY, obj);
    _gt_timer_set_repeat_count(tmp_timer, 1);
}

void gt_screen_set_bgcolor(gt_obj_st * obj, gt_color_t color)
{
    if( GT_TYPE_SCREEN != gt_obj_class_get_type(obj) ) {
        return;
    }
    obj->bgcolor = color;
}

gt_color_t gt_screen_get_bgcolor(gt_obj_st * obj)
{
    if( GT_TYPE_SCREEN != gt_obj_class_get_type(obj) ) {
        return gt_color_make(0, 0, 0);
    }
    return obj->bgcolor;
}

/* end ------------------------------------------------------------------*/
