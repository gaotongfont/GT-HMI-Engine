/**
 * @file gt_event.c
 * @author yongg
 * @brief
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


gt_res_t gt_event_send(gt_obj_st * parent, gt_event_type_et event, void * user_data){
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
}


/* end ------------------------------------------------------------------*/
