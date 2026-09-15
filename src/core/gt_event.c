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
#include "gt_obj_scroll.h"
#include "../others/gt_log.h"
#include "gt_mem.h"
#include "stdlib.h"
#include "gt_timer.h"
#include "gt_disp.h"
#include "stdbool.h"
#include "../others/gt_gc.h"
#include "../others/gt_anim.h"
#include "../hal/gt_hal_indev.h"

/* private define -------------------------------------------------------*/
#ifndef _GT_EVENT_ATTRIBUTE_SRAM_TEXT
    #define _GT_EVENT_ATTRIBUTE_SRAM_TEXT           GT_ATTRIBUTE_SRAM_TEXT
#endif



/* private typedef ------------------------------------------------------*/
/**
 * @brief delay send event
 */
typedef struct {
    gt_event_type_et event;
    void * parms;
}_event_delay_param_st;



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static GT_ATTRIBUTE_RAM_TEXT gt_res_t _gt_event_node_push(gt_event_st * e) {
    gt_event_st * ptr = _GT_GC_GET_ROOT(_gt_event_node_header_ll);
    /* fast path: stack is empty, no duplicate check needed */
    if (NULL == ptr) {
        e->prev = NULL;
        _GT_GC_GET_ROOT(_gt_event_node_header_ll) = e;
        return GT_RES_OK;
    }
    do {
        if (e == ptr) {
            return GT_RES_FAIL;
        }
        if (ptr->code_type == e->code_type && ptr->origin == e->origin) {
            return GT_RES_FAIL;
        }
        ptr = ptr->prev;
    } while (ptr);
    e->prev = _GT_GC_GET_ROOT(_gt_event_node_header_ll);
    _GT_GC_GET_ROOT(_gt_event_node_header_ll) = e;
    return GT_RES_OK;
}

static GT_ATTRIBUTE_RAM_TEXT void _gt_event_node_pop(gt_event_st *e) {
    _GT_GC_GET_ROOT(_gt_event_node_header_ll) = e->prev;
}

static gt_res_t _GT_EVENT_ATTRIBUTE_SRAM_TEXT
_gt_event_calling_user_cb(gt_event_st * e) {
    gt_obj_st * obj = (gt_obj_st *)e->target;
    /* type already validated by caller, access fields directly */
    gt_obj_event_attr_st * event_ptr = obj->event_attr;
    if (NULL == event_ptr) {
        return GT_RES_OK;
    }
    if (obj->disabled) {
        return GT_RES_OK;
    }
    ++obj->event_cb_depth;
    while(event_ptr) {
        gt_obj_event_attr_st * next = event_ptr->next_ptr;  /* cache before cb, in case cb removes events */
        if(event_ptr->user_cb &&
           (event_ptr->filter == GT_EVENT_TYPE_ALL || e->code_type == event_ptr->filter)) {
            e->user_data = event_ptr->user_data;
            event_ptr->user_cb(e);
            /* re-validate: cb may have destroyed the object and freed the event chain */
            if (gt_obj_is_type((struct gt_obj_s *)obj, GT_TYPE_UNKNOWN)) {
                return GT_RES_OK;
            }
        }
        event_ptr = next;
    }
    --obj->event_cb_depth;
    return GT_RES_OK;
}

/**
 * @brief Expand event replace func
 *
 * @param event_ptr
 * @param e
 * @return gt_res_t GT_RES_OK or GT_RES_FAIL
 */
static inline gt_res_t _GT_EVENT_ATTRIBUTE_SRAM_TEXT
_using_replace_core_user_cb(gt_obj_event_attr_st * event_ptr, gt_event_st * e) {
    gt_res_t res = GT_RES_FAIL;
    gt_obj_st * obj = (gt_obj_st *)e->target;
    ++obj->event_cb_depth;
    while(event_ptr) {
        gt_obj_event_attr_st * next = event_ptr->next_ptr;  /* cache before cb, in case cb removes events */
        if(event_ptr->user_cb &&
           (event_ptr->filter == GT_EVENT_TYPE_ALL || e->code_type == event_ptr->filter)) {
            e->user_data = event_ptr->user_data;
            event_ptr->user_cb(e);
            res = GT_RES_OK;
            /* re-validate: cb may have destroyed the object and freed the event chain */
            if (gt_obj_is_type((struct gt_obj_s *)obj, GT_TYPE_UNKNOWN)) {
                return res;
            }
        }
        event_ptr = next;
    }
    --obj->event_cb_depth;
    return res;
}

static inline gt_res_t _GT_EVENT_ATTRIBUTE_SRAM_TEXT
_event_check_scroll_begin(gt_obj_st * obj, gt_event_type_et t) {
    if (GT_EVENT_TYPE_INPUT_SCROLL_START != t) {
        return GT_RES_INV;
    }
    _gt_obj_set_scrolling(obj, true);
    return GT_RES_OK;
}

static inline gt_res_t _GT_EVENT_ATTRIBUTE_SRAM_TEXT
_event_check_scroll_end(gt_obj_st * obj, gt_event_type_et t) {
    if (GT_EVENT_TYPE_INPUT_SCROLL_END != t) {
        return GT_RES_INV;
    }
    _gt_obj_set_scrolling(obj, false);
    return GT_RES_OK;
}

static gt_res_t _GT_EVENT_ATTRIBUTE_SRAM_TEXT
_gt_event_calling_event_cb(gt_event_st * e) {
    gt_obj_st * obj = (gt_obj_st * )e->target;
    /* type already validated by caller, access fields directly */
    const gt_obj_class_st * c = obj->classes;

    if (obj->replace_core_event_attr) {
        gt_res_t rr = _using_replace_core_user_cb(obj->replace_core_event_attr, e);
        /* re-validate: replace cb may have destroyed the object */
        if (gt_obj_is_type((struct gt_obj_s *)obj, GT_TYPE_UNKNOWN)) {
            return GT_RES_OK;
        }
        if (GT_RES_OK == rr) {
            return GT_RES_OK;
        }
    }

    _event_check_scroll_begin(obj, e->code_type);

    /** Default core event func */
    if (c->_event_cb) {
        c->_event_cb(obj, e);
        /* re-validate: core cb may have destroyed the object */
        if (gt_obj_is_type((struct gt_obj_s *)obj, GT_TYPE_UNKNOWN)) {
            return GT_RES_OK;
        }
    }

    _event_check_scroll_end(obj, e->code_type);

    return GT_RES_OK;
}

static gt_res_t _GT_EVENT_ATTRIBUTE_SRAM_TEXT
_gt_event_send_kernel(gt_event_st * e) {
    gt_res_t res = GT_RES_OK;

    while (1) {
        if (gt_obj_is_type(e->target, GT_TYPE_UNKNOWN)) {
            GT_LOGW(GT_LOG_TAG_CHECK, "Unknown object: %p(%d)", e->target, gt_obj_class_get_type(e->target));
            return GT_RES_INV;
        }
        res = _gt_event_calling_event_cb(e);
        if (GT_RES_OK != res) {
            return res;
        }
        /* re-validate: core cb may have destroyed the object */
        if (gt_obj_is_type(e->target, GT_TYPE_UNKNOWN)) {
            return res;
        }
        res = _gt_event_calling_user_cb(e);
        if (GT_RES_OK != res) {
            return res;
        }
        /* re-validate: user cb may have destroyed the object */
        if (gt_obj_is_type(e->target, GT_TYPE_UNKNOWN)) {
            break;
        }
        gt_obj_st * parent_p = gt_obj_get_parent(e->target);
        if (parent_p && ((gt_obj_st *)e->target)->bubble_notify) {
            e->target = parent_p;
        } else {
            break;
        }
    }
    return res;
}

static GT_ATTRIBUTE_RAM_TEXT inline bool _is_input_event(gt_event_type_et event) {
    return (event >= _GT_EVENT_TYPE_INPUT_BEGIN && event < _GT_EVENT_TYPE_INPUT_END);
}

static GT_ATTRIBUTE_RAM_TEXT void _free_event_recursive(gt_obj_event_attr_st * event) {
    while (event) {
        gt_obj_event_attr_st * next = event->next_ptr;
        event->next_ptr = NULL;
        event->user_cb = NULL;
        event->user_data = NULL;
        gt_mem_free(event);
        event = next;
    }
}

static GT_ATTRIBUTE_RAM_TEXT void _anim_to_remove_all_event(struct gt_anim_s * anim) {
    gt_obj_event_attr_st * tar = (gt_obj_event_attr_st * )anim->tar;
    if (NULL == tar) {
        return;
    }
    _free_event_recursive(tar);
    anim->tar = NULL;
}

static inline gt_res_t _GT_EVENT_ATTRIBUTE_SRAM_TEXT
_common_event_send(gt_event_st * e) {
#if GT_USE_SCREEN_ANIM
    if (false == gt_event_is_enabled()) {
        return GT_RES_INV;
    }
#endif
    if (gt_obj_is_type(e->origin, GT_TYPE_UNKNOWN)) { return GT_RES_FAIL; }
    /* also validate target (may differ from origin in global indev path) */
    if (gt_obj_is_type(e->target, GT_TYPE_UNKNOWN)) { return GT_RES_FAIL; }
    if (GT_RES_FAIL == _gt_event_node_push(e)) {
        return GT_RES_INV;
    }
    gt_res_t res = _gt_event_send_kernel(e);
    _gt_event_node_pop(e);
    return res;
}

static bool _GT_EVENT_ATTRIBUTE_SRAM_TEXT
_common_event_has_the_same_cb(gt_obj_event_attr_st * event_ptr, gt_event_cb_t event, gt_event_type_et filter) {
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

static gt_obj_event_attr_st * GT_ATTRIBUTE_RAM_TEXT
_common_add_event_node(gt_obj_event_attr_st ** event_root_p) {
    uint16_t len = sizeof(gt_obj_event_attr_st);
    gt_obj_event_attr_st * new_event = (gt_obj_event_attr_st * )gt_mem_malloc(len);
    GT_CHECK_BACK_VAL(new_event, NULL);
    new_event->next_ptr = NULL;
    new_event->user_cb = NULL;
    new_event->user_data = NULL;
    new_event->filter = GT_EVENT_TYPE_NONE;

    gt_obj_event_attr_st * tmp_ptr = *event_root_p;
    if (tmp_ptr) {
        while(tmp_ptr->next_ptr) { tmp_ptr = tmp_ptr->next_ptr; }
        tmp_ptr->next_ptr = new_event;
    } else {
        *event_root_p = new_event;
    }
    return new_event;
}

static void _event_attr_exec_cb(void * tar, int32_t val) {}

static void _replace_core_event_attr_exec_cb(void * tar, int32_t val) {}

/**
 * @brief Remove event nodes matching filter from a callback linked list.
 *        Safe to call during callback iteration (defers free via animation).
 * @param obj        The object (for event_cb_depth check)
 * @param root_pp    Pointer to the list head pointer
 * @param filter     Event type to remove (GT_EVENT_TYPE_ALL removes all)
 */
static GT_ATTRIBUTE_RAM_TEXT void
_common_remove_event_by_filter(gt_obj_st * obj, gt_obj_event_attr_st ** root_pp, gt_event_type_et filter) {
    if (NULL == obj || NULL == root_pp || NULL == *root_pp) {
        return;
    }
    gt_obj_event_attr_st ** prev_ptr = root_pp;
    gt_obj_event_attr_st * event_ptr = *prev_ptr;

    while (event_ptr) {
        gt_obj_event_attr_st * next = event_ptr->next_ptr;
        if (filter == GT_EVENT_TYPE_ALL || event_ptr->filter == filter) {
            *prev_ptr = next;
            if (obj->event_cb_depth > 0) {
                event_ptr->next_ptr = NULL;
                gt_anim_st anim;
                gt_anim_init(&anim);
                gt_anim_set_time(&anim, 1);
                gt_anim_set_target(&anim, event_ptr);
                gt_anim_set_ready_cb(&anim, _anim_to_remove_all_event);
                gt_anim_set_exec_cb(&anim, _event_attr_exec_cb);
                gt_anim_start(&anim);
            } else {
                event_ptr->next_ptr = NULL;
                event_ptr->user_cb = NULL;
                event_ptr->user_data = NULL;
                gt_mem_free(event_ptr);
            }
        } else {
            prev_ptr = &event_ptr->next_ptr;
        }
        event_ptr = next;
    }
}

static void GT_ATTRIBUTE_RAM_TEXT
_event_delay_exec_cb(void * tar, int32_t val) {
    // NULL
}

static void GT_ATTRIBUTE_RAM_TEXT
_event_delay_send_ready_cb(gt_anim_st * anim) {
    _event_delay_param_st * p = (_event_delay_param_st * )anim->data;
    if (NULL == p) {
        return;
    }
    gt_event_send(anim->tar, p->event, p->parms);
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
    if (_common_event_has_the_same_cb(obj->event_attr, event, filter)) {
        obj->event_attr->user_data = user_data;
        return;
    }
    gt_obj_event_attr_st * new_event = _common_add_event_node(&obj->event_attr);
    GT_CHECK_BACK(new_event);

    new_event->user_cb = event;
    new_event->filter = filter;
    new_event->user_data = user_data;
}

void gt_obj_add_replace_core_event_cb(struct gt_obj_s * obj, gt_event_cb_t event, gt_event_type_et filter, void * user_data)
{
    if (NULL == obj || NULL == event) {
        return;
    }
    if (_common_event_has_the_same_cb(obj->replace_core_event_attr, event, filter)) {
        obj->replace_core_event_attr->user_data = user_data;
        return;
    }
    gt_obj_event_attr_st * new_event = _common_add_event_node(&obj->replace_core_event_attr);
    GT_CHECK_BACK(new_event);

    new_event->user_cb = event;
    new_event->filter = filter;
    new_event->user_data = user_data;
}

void gt_obj_remove_event_cb(struct gt_obj_s * obj, gt_event_type_et filter)
{
    if (NULL == obj) {
        return;
    }
    _common_remove_event_by_filter(obj, &obj->event_attr, filter);
}

void gt_obj_replace_event_cb(struct gt_obj_s * obj, gt_event_cb_t event, gt_event_type_et filter, void * user_data)
{
    if (NULL == obj || NULL == event) {
        return;
    }
    gt_obj_event_attr_st * event_ptr = obj->event_attr;
    while (event_ptr) {
        if (event_ptr->filter == filter) {
            event_ptr->user_cb = event;
            event_ptr->user_data = user_data;
            return;
        }
        event_ptr = event_ptr->next_ptr;
    }
    /* not found, add new */
    gt_obj_add_event_cb(obj, event, filter, user_data);
}

void gt_obj_reset_replace_core_event_cb(struct gt_obj_s * obj, gt_event_type_et filter)
{
    if (NULL == obj) {
        return;
    }
    _common_remove_event_by_filter(obj, &obj->replace_core_event_attr, filter);
}

bool gt_event_has_event_cb(struct gt_obj_s * obj, gt_event_type_et filter)
{
    if (NULL == obj) {
        return false;
    }
    gt_obj_event_attr_st * event_ptr = obj->event_attr;
    if (NULL == event_ptr) {
        return false;
    }
    while (event_ptr) {
        if (event_ptr->filter == GT_EVENT_TYPE_ALL || event_ptr->filter == filter) {
            return true;
        }
        event_ptr = event_ptr->next_ptr;
    }
    return false;
}


bool gt_event_has_the_same_cb(struct gt_obj_s * obj, gt_event_cb_t event, gt_event_type_et filter)
{
    if (NULL == obj) {
        return false;
    }
    return _common_event_has_the_same_cb(obj->event_attr, event, filter);
}

uint16_t gt_obj_get_user_event_count(struct gt_obj_s * obj)
{
    uint16_t cnt = 0;
    if (NULL == obj) {
        return cnt;
    }
    gt_obj_event_attr_st * event_ptr = obj->event_attr;
    while (event_ptr) {
        ++cnt;
        event_ptr = event_ptr->next_ptr;
    }
    return cnt;
}

void gt_obj_remove_all_event_cb(struct gt_obj_s * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    if (NULL == obj->event_attr && NULL == obj->replace_core_event_attr) {
        return;
    }
    gt_anim_st anim;
    gt_anim_init(&anim);
    gt_anim_set_time(&anim, 1);
    gt_anim_set_ready_cb(&anim, _anim_to_remove_all_event);

    if (obj->event_attr) {
        gt_anim_set_target(&anim, obj->event_attr);
        gt_anim_set_exec_cb(&anim, _event_attr_exec_cb);
        gt_anim_start(&anim);
        obj->event_attr = NULL;
    }

    if (obj->replace_core_event_attr) {
        gt_anim_set_target(&anim, obj->replace_core_event_attr);
        gt_anim_set_exec_cb(&anim, _replace_core_event_attr_exec_cb);
        gt_anim_start(&anim);
        obj->replace_core_event_attr = NULL;
    }
}

void gt_obj_remove_all_event_cb_immediately(struct gt_obj_s * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    /* downgrade to safe deferred removal if callback chain is being iterated */
    if (obj->event_cb_depth) {
        gt_obj_remove_all_event_cb(obj);
        return;
    }
    if (obj->event_attr) {
        _free_event_recursive(obj->event_attr);
        obj->event_attr = NULL;
    }
    if (obj->replace_core_event_attr) {
        _free_event_recursive(obj->replace_core_event_attr);
        obj->replace_core_event_attr = NULL;
    }
}

gt_res_t gt_event_send(struct gt_obj_s * parent, gt_event_type_et event, void * parms)
{
    if (NULL == parent) {
        return GT_RES_INV;
    }
    if (gt_obj_is_type(parent, GT_TYPE_UNKNOWN)) {
        GT_LOGD(GT_LOG_TAG_CHECK, "Invalid obj: %p(%d), ev: %d", parent, gt_obj_class_get_type(parent), event);
        return GT_RES_INV;
    }
    gt_event_st e = {
        .target = parent,
        .origin = parent,
        .code_type = event,
        .param = parms,
        .key_val = 0,
    };
    return _common_event_send(&e);
}

gt_res_t gt_event_send_delay(struct gt_obj_s * parent, gt_event_type_et event, void * parms, int32_t delay_ms)
{
    gt_anim_st an = {0};
    _event_delay_param_st param = {
        .event = event,
        .parms = parms,
    };
    gt_anim_init(&an);
    gt_anim_set_target(&an, parent);
    gt_anim_set_data(&an, &param, sizeof(_event_delay_param_st));
    gt_anim_set_time(&an, 0);
    gt_anim_set_time_delay_start(&an, delay_ms);
    gt_anim_set_exec_cb(&an, _event_delay_exec_cb);
    gt_anim_set_ready_cb(&an, _event_delay_send_ready_cb);
    gt_anim_start(&an);

    return GT_RES_OK;
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

    uint16_t cnt = parent->cnt_child;   /* snapshot: child cb may modify the child list */
    if (0 == cnt) { return GT_RES_OK; } /* guard against underflow on unsigned gt_size_t */
    gt_size_t idx = (gt_size_t)cnt - 1;
    for (; idx >= 0; idx--) {
        if (idx >= (gt_size_t)parent->cnt_child) { continue; }  /* child was removed during iteration */
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
    if (NULL == target) {
        return GT_RES_FAIL;
    }
    return gt_event_send(target, event, parms);
}

#if GT_USE_LAYER_TOP
void gt_global_add_event_cb(gt_event_cb_t event, gt_event_type_et filter, void * user_data)
{
    gt_obj_add_event_cb(gt_disp_get_layer_top(), event, filter, user_data);
}

uint16_t gt_global_get_event_count(void)
{
    return gt_obj_get_user_event_count(gt_disp_get_layer_top());
}

bool gt_global_event_has_the_same_cb(gt_event_cb_t event, gt_event_type_et filter)
{
    return gt_event_has_the_same_cb(gt_disp_get_layer_top(), event, filter);
}

void gt_global_remove_all_event_cb(void)
{
    gt_obj_remove_all_event_cb(gt_disp_get_layer_top());
}

void gt_global_remove_all_event_cb_immediately(void)
{
    gt_obj_remove_all_event_cb_immediately(gt_disp_get_layer_top());
}

void gt_global_remove_event_cb(gt_event_type_et filter)
{
    gt_obj_remove_event_cb(gt_disp_get_layer_top(), filter);
}

void gt_global_replace_event_cb(gt_event_cb_t event, gt_event_type_et filter, void * user_data)
{
    gt_obj_replace_event_cb(gt_disp_get_layer_top(), event, filter, user_data);
}

gt_res_t gt_global_event_send(gt_event_type_et event, void * parms)
{
    if (0 == gt_global_get_event_count()) {
        return GT_RES_INV;
    }
    return gt_event_send(gt_disp_get_layer_top(), event, parms);
}

gt_res_t _gt_global_core_indev_event_send(struct gt_obj_s * org, gt_event_type_et event, void * parms)
{
    if (0 == gt_global_get_event_count()) {
        return GT_RES_INV;
    }
    if (gt_obj_is_type(org, GT_TYPE_UNKNOWN)) {
        GT_LOGD(GT_LOG_TAG_CHECK, "Invalid obj: %p(%d), ev: %d", org, gt_obj_class_get_type(org), event);
        return GT_RES_INV;
    }
    gt_indev_param_st * indev = (gt_indev_param_st * )parms;
    gt_event_st e = {
        .target = gt_disp_get_layer_top(),
        .origin = org,
        .code_type = event,
        .param = parms,
        .key_val = indev ? indev->param.keypad_key : 0,
    };
    return _common_event_send(&e);
}
#endif  /** GT_USE_LAYER_TOP */

gt_event_type_et gt_event_get_code(gt_event_st * e)
{
    if (NULL == e) { return GT_EVENT_TYPE_NONE; }
    return e->code_type;
}

bool _gt_obj_in_event_dispatch(struct gt_obj_s * obj)
{
    if (NULL == obj) {
        return false;
    }
    gt_event_st * e = _GT_GC_GET_ROOT(_gt_event_node_header_ll);
    while (e) {
        struct gt_obj_s * t = e->target;
        while (t) {
            if (t == obj) {
                return true;
            }
            t = t->parent;
        }
        t = e->origin;
        while (t) {
            if (t == obj) {
                return true;
            }
            t = t->parent;
        }
        e = e->prev;
    }
    return false;
}

bool _gt_obj_child_in_event_dispatch(struct gt_obj_s * obj)
{
    if (NULL == obj) {
        return false;
    }
    gt_event_st * e = _GT_GC_GET_ROOT(_gt_event_node_header_ll);
    while (e) {
        struct gt_obj_s * t = e->target;
        if (t && t != obj) {
            t = t->parent;
            while (t) {
                if (t == obj) {
                    return true;
                }
                t = t->parent;
            }
        }
        t = e->origin;
        if (t && t != obj) {
            t = t->parent;
            while (t) {
                if (t == obj) {
                    return true;
                }
                t = t->parent;
            }
        }
        e = e->prev;
    }
    return false;
}

/* end ------------------------------------------------------------------*/
