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
/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/

/**
 * @brief Records the locked state when the event is executed.
 */
static bool _gt_event_is_locked = false;

/**
 * @brief The object records the locked state when the event is executed.
 */
static struct gt_obj_s * _gt_event_obj_locked = NULL;


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static inline void _gt_event_locked(void) {
    _gt_event_is_locked = true;
}

static inline void _gt_event_unlocked(void) {
    _gt_event_is_locked = false;
}

static inline void _gt_event_locked_obj(gt_obj_st * obj) {
    _gt_event_locked();
    _gt_event_obj_locked = (struct gt_obj_s *)obj;
}

static inline void _gt_event_unlocked_obj(void) {
    _gt_event_unlocked();
    _gt_event_obj_locked = NULL;
}

static gt_obj_event_attr_st * gt_obj_get_event_attr(gt_obj_st * obj, uint8_t idx){
    if( 0 == obj->cnt_event ){
        return NULL;
    }
    if( idx > obj->cnt_event ){
        return NULL;
    }
    return &obj->event_attr[idx];
}

static gt_res_t _gt_event_handler_event_mask(gt_obj_st * obj , void * param)
{
    if (!obj->event_mask) {
        return GT_RES_INV;
    }

    if (NULL == obj->class || NULL == obj->class->_event_cb) {
        return GT_RES_OK;
    }

    uint32_t mask_shift = obj->event_mask, event_code = 0;
    uint16_t idx = 0;
    uint16_t count = _GT_EVENT_TYPE_TOTAL_COUNT;
    gt_event_st e;

    e.target = obj;
    e.user_data = obj->user_data;

    if (GT_CFG_DEFAULT_EVENT_MASK_BIT_MAX < _GT_EVENT_TYPE_TOTAL_COUNT) {
        count = GT_CFG_DEFAULT_EVENT_MASK_BIT_MAX;
    }

    _gt_event_locked_obj(obj);

    while (idx < count) {
        event_code =  GT_EVENT_GET_MASK_SHIFT_BIT(idx) & mask_shift;
        if( event_code ){
            e.code = idx;
            e.param = param;
            obj->event_mask &= ~GT_EVENT_GET_MASK_SHIFT_BIT(e.code);
            obj->class->_event_cb(obj, &e);
        }
        idx++;
    }

    _gt_event_unlocked_obj();

    return GT_RES_OK;
}

static void _gt_obj_foreach_event_cb(gt_obj_st * parent){
    _gt_event_handler_event_mask(parent , NULL);

    uint16_t idx_child = 0, count = parent->cnt_child;
    gt_obj_st * obj_temp = NULL;

    while( idx_child < count ){
        obj_temp = parent->child[idx_child];
        if( obj_temp->cnt_child ){
            _gt_obj_foreach_event_cb(obj_temp);
        } else {
            _gt_event_handler_event_mask(obj_temp , NULL);
        }
        idx_child++;
    }
}

/**
 * @brief only printf once time 'no screen'
 */
static GT_ATTRIBUTE_LARGE_RAM_ARRAY bool _is_print_no_screen_flag = 0;

static void _gt_timer_event_cb(struct _gt_timer_s * timer){
    // GT_LOGV(GT_LOG_TAG_GUI, "timer event call back start");

    /* step1: get top layer screen */
    gt_obj_st * scr = gt_disp_get_scr();
    if( NULL == scr ){
        if (_is_print_no_screen_flag) {
            return ;
        }
        GT_LOGW(GT_LOG_TAG_GUI, "No screen loading...");
        _is_print_no_screen_flag = true;
        return;
    }

    /* step2: foreach obj event_mask and process */
    _gt_obj_foreach_event_cb(scr);

    // GT_LOGV(GT_LOG_TAG_GUI, "timer event call back end, %p", scr);
}

static inline bool _is_input_event(gt_event_type_et event) {
    return (event < _GT_EVENT_TYPE_INPUT_BEGIN || event > _GT_EVENT_TYPE_INPUT_END) ? false : true;
}

/* global functions / API interface -------------------------------------*/
void gt_obj_add_event_cb(struct gt_obj_s * obj, gt_event_cb_t event, gt_event_type_et filter, void * user_data){
    GT_LOGV(GT_LOG_TAG_GUI, "start event count:%d", obj->cnt_event);
    if ( NULL == obj->event_attr ) {
        obj->event_attr =  gt_mem_malloc(sizeof(gt_obj_event_attr_st));
        obj->cnt_event = 0;
    } else {
        obj->event_attr = gt_mem_realloc( obj->event_attr, (obj->cnt_event + 1) * sizeof(gt_obj_event_attr_st) );
    }
    obj->event_attr[obj->cnt_event].user_cb = event;
    obj->event_attr[obj->cnt_event].filter = filter;
    obj->event_attr[obj->cnt_event].user_data = user_data;
    ++obj->cnt_event;
    GT_LOGV(GT_LOG_TAG_GUI, "end event count:%d", obj->cnt_event);
}

gt_res_t gt_event_send(struct gt_obj_s * parent, gt_event_type_et event, void * parms){
    if( !parent ){
        GT_LOGD(GT_LOG_TAG_GUI, "parent is null");
        return GT_RES_FAIL;
    }
    // add event to obj----sys event,process draw,del,create...need long time
    if( parent->event_mask & GT_EVENT_GET_MASK_SHIFT_BIT(event) ){
        GT_LOGV(GT_LOG_TAG_GUI, "parent is exist the event code:%d", event);
        return GT_RES_FAIL;
    }
    if (gt_obj_is_disabled(parent) && _is_input_event(event)) {
        return GT_RES_INV;
    }

    parent->event_mask |= GT_EVENT_GET_MASK_SHIFT_BIT(event);
    if (gt_event_is_locked()) {
        return GT_RES_OK;
    }
    if ( !parent->cnt_event ) {
        return GT_RES_OK;
    }

    // foreach parent's event_attr----user event,immediately process the user events
    gt_size_t idx = 0;
    gt_obj_event_attr_st * event_attr;
    gt_event_st e;

    _gt_event_handler_event_mask(parent , parms);

    _gt_event_locked_obj(parent);

    while ( idx < parent->cnt_event ) {
        event_attr = gt_obj_get_event_attr(parent, idx++);
        if ( NULL == event_attr ) {
            continue;
        }
        if ( event == event_attr->filter || GT_EVENT_TYPE_ALL == event_attr->filter ) {
            e.target    = parent;
            e.code      = event;
            e.user_data = event_attr->user_data;
            e.param = parms;
            event_attr->user_cb(&e);
        }
    }

    _gt_event_unlocked_obj();

	return GT_RES_OK;
}

void _gt_event_init(){
    _gt_timer_create(_gt_timer_event_cb, GT_TASK_PERIOD_TIME_EVENT, NULL);
}

void gt_event_clr_all_event(gt_obj_st * parent)
{
    if( parent == NULL ){
        return;
    }
    int idx = 0;
    parent->event_mask = 0;
    while(idx < parent->cnt_child){
        gt_event_clr_all_event( parent->child[idx++] );
    }
}

bool gt_event_is_locked(void)
{
    return _gt_event_is_locked;
}

struct gt_obj_s * gt_event_get_locked_obj(void)
{
    return _gt_event_obj_locked;
}

gt_event_type_et gt_event_get_code(gt_event_st * e) {
    return e->code;
}

/* end ------------------------------------------------------------------*/
