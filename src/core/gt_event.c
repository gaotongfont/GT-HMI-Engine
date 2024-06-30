/**
 * @file gt_event.c
 * @author yongg
 * @brief Object implementation for the event handler
 * @version 0.1
 * @date 2022-05-11 15:02:33
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
static void _gt_event_node_push(gt_event_st * e) {
    e->prev = _GT_GC_GET_ROOT(_gt_event_node_header_ll);
    _GT_GC_GET_ROOT(_gt_event_node_header_ll) = e;
}

static void _gt_event_node_pop(gt_event_st *e) {
    _GT_GC_GET_ROOT(_gt_event_node_header_ll) = e->prev;
}

static gt_res_t _gt_event_calling_user_cb(gt_event_st * e) {
    gt_obj_event_attr_st * event_list = e->target->event_attr;

    for (gt_size_t i = 0, count = e->target->cnt_event; i < count; i++) {
        if (NULL == event_list[i].user_cb) {
            continue;
        }
        if (e->code != event_list[i].filter) {
            continue;
        }
        e->user_data = event_list[i].user_data;
        event_list[i].user_cb(e);
    }

    return GT_RES_OK;
}

static gt_res_t _gt_event_calling_event_cb(gt_event_st * e) {
    gt_obj_st * obj = e->target;
    GT_CHECK_BACK_VAL(obj, GT_RES_FAIL);

    const gt_obj_class_st * class = obj->class;
    GT_CHECK_BACK_VAL(class, GT_RES_FAIL);
    GT_CHECK_BACK_VAL(class->_event_cb, GT_RES_FAIL);

    class->_event_cb(obj, e);
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
    gt_size_t i = obj->cnt_event - 1;
    for (; i >= 0; --i) {
        if (obj->event_attr[i].user_cb == event && obj->event_attr[i].filter == filter) {
            return true;
        }
    }
    return false;
}

/* global functions / API interface -------------------------------------*/
void gt_obj_add_event_cb(struct gt_obj_s * obj, gt_event_cb_t event, gt_event_type_et filter, void * user_data)
{
    if (NULL == obj || NULL == event) {
        return;
    }
    if (_has_the_same_cb(obj, event, filter)) {
        return ;
    }
    if ( NULL == obj->event_attr ) {
        obj->event_attr =  gt_mem_malloc(sizeof(gt_obj_event_attr_st));
        obj->cnt_event = 0;
    } else {
        obj->event_attr = gt_mem_realloc( obj->event_attr, (obj->cnt_event + 1) * sizeof(gt_obj_event_attr_st) );
    }
    GT_CHECK_BACK(obj->event_attr);
    obj->event_attr[obj->cnt_event].user_cb = event;
    obj->event_attr[obj->cnt_event].filter = filter;
    obj->event_attr[obj->cnt_event].user_data = user_data;
    ++obj->cnt_event;
}

gt_res_t gt_event_send(struct gt_obj_s * parent, gt_event_type_et event, void * parms)
{
    GT_CHECK_BACK_VAL(parent, GT_RES_FAIL);
    gt_event_st e;
    e.target = parent;
    e.origin = parent;
    e.code = event;
    e.param = parms;

    _gt_event_node_push(&e);

    gt_res_t res = _gt_event_send_kernel(&e);

    _gt_event_node_pop(&e);

    return res;
}

gt_res_t gt_event_send_to_childs(struct gt_obj_s * parent, gt_event_type_et event, void * parms)
{
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
