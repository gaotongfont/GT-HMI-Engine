/**
 * @file gt_event.c
 * @author yongg
 * @brief Object implementation for the event handler
 * @version 0.1
 * @date 2022-05-11 15:02:33
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_conf.h"
#include "gt_event.h"
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



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_obj_event_attr_st * gt_obj_get_event_attr(gt_obj_st * obj, uint8_t idx){
    if( 0 == obj->cnt_event ){
        return NULL;
    }
    if( idx > obj->cnt_event ){
        return NULL;
    }
    return &obj->event_attr[idx];
}


/* global functions / API interface -------------------------------------*/
void gt_obj_add_event_cb(gt_obj_st * obj, gt_event_cb_t event, gt_event_type_et filter, void * user_data){
    GT_LOGD(GT_LOG_TAG_GUI, "start add event cb:%d", obj->cnt_event);
    if( NULL == obj->event_attr ){
        obj->event_attr =  gt_mem_malloc(sizeof(gt_obj_event_attr_st));
        obj->event_attr[0].cb = event;
        obj->event_attr[0].user_data = user_data;
        obj->event_attr[0].filter = filter;
        obj->cnt_event = 1;
    }
    else{
        obj->event_attr = gt_mem_realloc( obj->event_attr, (obj->cnt_event+1) * sizeof(gt_obj_event_attr_st) );
        obj->event_attr[obj->cnt_event].cb = event;
        obj->event_attr[obj->cnt_event].user_data = user_data;
        obj->event_attr[obj->cnt_event].filter = filter;
        obj->cnt_event++;
    }
    GT_LOGD(GT_LOG_TAG_GUI, "end add event cb:%d", obj->cnt_event);
}

gt_event_type_et gt_event_get_code(gt_event_st * e){
    return e->code;
}

/* 1th version--unused */
gt_res_t gt_event_send_0(gt_obj_st * parent, gt_event_type_et event, void * user_data){
    gt_event_st *e, _e;
    int idx_child = 0, idx_event = 0;
    gt_event_type_et filter = event;

    _e.code = event;
    _e.target = parent;
    _e.user_data = user_data;
    e = &_e;

    while(idx_child < parent->cnt_child){
        gt_event_send(parent->child[idx_child], event, user_data);
        parent->child[idx_child]->class->_event_cb(parent->child[idx_child], e);
        idx_child++;
    }
    while(idx_event < parent->cnt_event ){
        if(filter == GT_EVENT_TYPE_ALL || filter == parent->event_attr[idx_event].filter ){
            parent->event_attr[idx_event].cb(e);
        }
        idx_event++;
    }
		return GT_RES_OK;
}

/* 2th version--use */
gt_res_t gt_event_send(gt_obj_st * parent, gt_event_type_et event, void * parms){
    if( !parent ){
        GT_LOGD(GT_LOG_TAG_GUI, "parent is null");
        return GT_RES_FAIL;
    }
    // add event to obj----sys event,process draw,del,create...need long time
    if( parent->event_mask&event ){
        GT_LOGD(GT_LOG_TAG_GUI, "parent is exist the event code:%d", event);
        return GT_RES_FAIL;
    }
    parent->event_mask |= event;

    // foreach parent's event_attr----user event,immediately process the user events
    int idx = 0;
    gt_obj_event_attr_st * event_attr;
    gt_event_st e;
    while( idx < parent->cnt_event ){
        event_attr = gt_obj_get_event_attr(parent, idx);//&parent->event_attr[idx];
        if( NULL == event_attr ){
            idx++;
            continue;
        }
        if(event_attr->filter == GT_EVENT_TYPE_ALL || event_attr->filter == event ) {
            e.target    = parent;
            e.code      = event;
            e.user_data = event_attr->user_data;
            event_attr->cb(&e);
        }
        idx++;
    }
	return GT_RES_OK;
}

void _gt_event_handler_event_mask(gt_obj_st * obj){
    int idx = 0;
    uint32_t event_code;
    gt_event_st e;
    while(idx < GT_CFG_DEFAULT_EVENT_MASK_BIT_MAX){
        event_code = obj->event_mask & (1<<idx);
        if( event_code ){
            e.target = obj;
            e.code = (gt_event_type_et)event_code;
            e.user_data = NULL;
            obj->class->_event_cb(obj, &e);
            obj->event_mask &= ~(1<<idx);
        }
        idx++;
    }
}


static void _gt_obj_foreach_event_cb(gt_obj_st * parent){
    _gt_event_handler_event_mask(parent);

    int idx_child = 0;
    gt_obj_st * obj_temp = NULL;
    while( idx_child < parent->cnt_child ){
		obj_temp = parent->child[idx_child];

        if( 0 != obj_temp->cnt_child ){
            _gt_obj_foreach_event_cb(obj_temp);
        }
        _gt_event_handler_event_mask( obj_temp );
        idx_child++;
    }
}
/**
 * @brief only printf once time 'no screen'
 */
static GT_ATTRIBUTE_LARGE_RAM_ARRAY bool _is_print_no_screen_flag = 0;

void _gt_timer_event_cb(struct _gt_timer_s * timer){
    // GT_LOGV(GT_LOG_TAG_GUI, "timer event cb start");

    /* step1: get top layer screen */
    gt_obj_st * scr = gt_disp_get_scr();
    if( NULL == scr ){
        if (_is_print_no_screen_flag) {
            return ;
        }
        GT_LOGW(GT_LOG_TAG_GUI, "no screen");
        _is_print_no_screen_flag = true;
        return;
    }

    /* step2: foreach obj event_mask and process */
    _gt_obj_foreach_event_cb(scr);

    // GT_LOGV(GT_LOG_TAG_GUI, "timer event cb end");
}

void _gt_event_init(){
    _gt_timer_create(_gt_timer_event_cb, GT_TASK_PERIOD_TIME_EVENT, NULL);
}

void gt_event_clr_mask(gt_obj_st * parent)
{
    int idx = 0;
    parent->event_mask = 0;
    while(idx < parent->cnt_child){
        gt_event_clr_mask( parent->child[idx++] );
    }
}


/* end ------------------------------------------------------------------*/
