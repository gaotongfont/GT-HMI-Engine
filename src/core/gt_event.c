/**
 * @file gt_event.c
 * @author Feyoung
 * @brief Object implementation for the event handler
 * @version 0.2
 * @date 2024-7-4
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_conf.h"
#include "gt_event.h"
#include "./gt_style.h"
#include "../widgets/gt_obj.h"
#include "../others/gt_log.h"
#include "gt_mem.h"
#include "stdlib.h"
#include "gt_timer.h"
#include "gt_disp.h"
#include "stdbool.h"
#include "../others/gt_gc.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_res_t _gt_event_node_push(gt_event_st * e) {
    gt_event_st * ptr = _GT_GC_GET_ROOT(_gt_event_node_header_ll);
    while (ptr) {
        if (e == ptr) {
            return GT_RES_FAIL;
        }
        if (ptr->code == e->code && ptr->origin == e->origin) {
            return GT_RES_FAIL;
        }
        ptr = ptr->prev;
    }
    e->prev = _GT_GC_GET_ROOT(_gt_event_node_header_ll);
    _GT_GC_GET_ROOT(_gt_event_node_header_ll) = e;
    return GT_RES_OK;
}

static void _gt_event_node_pop(gt_event_st *e) {
    _GT_GC_GET_ROOT(_gt_event_node_header_ll) = e->prev;
}

static gt_res_t _gt_event_calling_user_cb(gt_event_st * e) {
    gt_obj_event_attr_st * event_ptr = e->target->event_attr;
    if (NULL == event_ptr) {
        return GT_RES_OK;
    }
    while(event_ptr) {
        if(event_ptr->user_cb && e->code == event_ptr->filter) {
            e->user_data = event_ptr->user_data;
            event_ptr->user_cb(e);
        }
        event_ptr = event_ptr->next_ptr;
    }
    return GT_RES_OK;
}

static gt_res_t _gt_event_calling_event_cb(gt_event_st * e) {
    gt_obj_st * obj = (gt_obj_st * )e->target;
    GT_CHECK_BACK_VAL(obj, GT_RES_FAIL);

    const gt_obj_class_st * c = obj->classes;
    GT_CHECK_BACK_VAL(c, GT_RES_FAIL);
    GT_CHECK_BACK_VAL(c->_event_cb, GT_RES_FAIL);

    c->_event_cb(obj, e);
    return GT_RES_OK;
}

static gt_res_t _gt_event_send_kernel(gt_event_st * e) {
    gt_res_t res = GT_RES_OK;

    res = _gt_event_calling_event_cb(e);
    if (GT_RES_OK != res) {
        return res;
    }

    res = _gt_event_calling_user_cb(e);
    if (GT_RES_OK != res) {
        return res;
    }

    gt_obj_st * parent_p = gt_obj_get_parent(e->target);
    if (parent_p && gt_obj_is_bubble_notify(e->target)) {
        e->target = parent_p;
        res = _gt_event_send_kernel(e);
        if (GT_RES_OK != res) {
            return res;
        }
    }

    return res;
}

static inline bool _is_input_event(gt_event_type_et event) {
    return (event < _GT_EVENT_TYPE_INPUT_BEGIN || event > _GT_EVENT_TYPE_INPUT_END) ? false : true;
}

static bool _has_the_same_cb(struct gt_obj_s * obj, gt_event_cb_t event, gt_event_type_et filter) {
    gt_obj_event_attr_st * event_ptr = obj->event_attr;
    if (NULL == event_ptr) {
        return false;
    }
    while (event_ptr) {
        if (event_ptr->user_cb == event && event_ptr->filter == filter) {
            return true;
        }
        event_ptr = event_ptr->next_ptr;
    }
    return false;
}

static void _free_event_recursive(gt_obj_event_attr_st * event) {
    if (event->next_ptr) {
        _free_event_recursive(event->next_ptr);
    }
    event->next_ptr = NULL;
    event->user_cb = NULL;
    gt_mem_free(event);
}

/* global functions / API interface -------------------------------------*/
#if GT_USE_SCREEN_ANIM
void gt_event_set_enabled(bool enabled)
{
    struct _gt_gc_event_s * event_c = (struct _gt_gc_event_s * )&_GT_GC_GET_ROOT(event_ctl);
    GT_CHECK_BACK(event_c);
    event_c->enabled = enabled;
}

bool gt_event_is_enabled(void)
{
    struct _gt_gc_event_s * event_c = (struct _gt_gc_event_s * )&_GT_GC_GET_ROOT(event_ctl);
    GT_CHECK_BACK_VAL(event_c, false);
    return event_c->enabled;
}
#endif  /** GT_USE_SCREEN_ANIM */

void gt_obj_add_event_cb(struct gt_obj_s * obj, gt_event_cb_t event, gt_event_type_et filter, void * user_data)
{
    if (NULL == obj || NULL == event) {
        return;
    }
    if (_has_the_same_cb(obj, event, filter)) {
        return ;
    }
    uint16_t len = sizeof(gt_obj_event_attr_st);
    gt_obj_event_attr_st * tmp_ptr = obj->event_attr;
    gt_obj_event_attr_st * new_event = (gt_obj_event_attr_st * )gt_mem_malloc(len);
    GT_CHECK_BACK(new_event);
    new_event->next_ptr = NULL;
    new_event->user_cb = event;
    new_event->filter = filter;
    new_event->user_data = user_data;

    if (tmp_ptr) {
        while(tmp_ptr->next_ptr) { tmp_ptr = tmp_ptr->next_ptr; }
        tmp_ptr->next_ptr = new_event;
    } else {
        obj->event_attr = new_event;
    }
}

void gt_obj_remove_all_event_cb(struct gt_obj_s * obj)
{
    GT_CHECK_BACK(obj);
    if (NULL == obj->event_attr) {
        return;
    }
    _free_event_recursive(obj->event_attr);
    obj->event_attr = NULL;
}

gt_res_t gt_event_send(struct gt_obj_s * parent, gt_event_type_et event, void * parms)
{
#if GT_USE_SCREEN_ANIM
    if (false == gt_event_is_enabled()) {
        return GT_RES_INV;
    }
#endif
    GT_CHECK_BACK_VAL(parent, GT_RES_FAIL);
    gt_event_st e = {
        .target = parent,
        .origin = parent,
        .code = event,
        .param = parms,
    };

    if (GT_RES_FAIL == _gt_event_node_push(&e)) {
        return GT_RES_INV;
    }

    gt_res_t res = _gt_event_send_kernel(&e);

    _gt_event_node_pop(&e);

    return res;
}

gt_res_t gt_event_send_to_childs(struct gt_obj_s * parent, gt_event_type_et event, void * parms)
{
#if GT_USE_SCREEN_ANIM
    if (false == gt_event_is_enabled()) {
        return GT_RES_INV;
    }
#endif
    gt_res_t res = GT_RES_OK;
    GT_CHECK_BACK_VAL(parent, GT_RES_FAIL);

    gt_size_t idx = parent->cnt_child - 1;
    for (; idx >= 0; idx--) {
        res = gt_event_send(parent->child[idx], event, parms);
        if (GT_RES_OK != res) {
            break;
        }
    }
    return res;
}

gt_res_t gt_event_send_by_id(gt_id_t widget_id, gt_event_type_et event, void * parms)
{
#if GT_USE_SCREEN_ANIM
    if (false == gt_event_is_enabled()) {
        return GT_RES_INV;
    }
#endif
    if (-1 == widget_id) {
        return GT_RES_FAIL;
    }
    gt_obj_st * target = gt_obj_find_by_id(widget_id);

    return gt_event_send(target, event, parms);
}

gt_event_type_et gt_event_get_code(gt_event_st * e)
{
    return e->code;
}

/* end ------------------------------------------------------------------*/
