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
#include "gt_mem.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_obj_add_event_cb(gt_obj_st * obj, gt_event_cb_t event, void * user_data){
    GT_LOGV(GT_LOG_TAG_GUI, "obj add event_cb");
    obj->event_attr = gt_mem_realloc(obj->event_attr, (obj->cnt_event+1) * sizeof(gt_obj_event_attr_st) );
    obj->event_attr[obj->cnt_event].cb = event;
    obj->event_attr[obj->cnt_event].user_data = user_data;
    obj->cnt_event++;
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

/* end ------------------------------------------------------------------*/
