/**
 * @file gt_event.h
 * @author yongg (your@email.com)
 * @brief 
 * @version 0.1
 * @date 2022-05-11 14:57:40
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_EVENT_TYPE_H_
#define _GT_EVENT_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../gt_conf.h"
#include "gt_event.h"
#include "../others/gt_types.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
struct _gt_obj_s;

typedef enum gt_event_type_e{
    GT_EVENT_TYPE_NONE = 0,

    /** Input device events*/
    GT_EVENT_TYPE_INPUT_CLICKING,
    GT_EVENT_TYPE_INPUT_PROCESS,
    GT_EVENT_TYPE_INPUT_PROCESSED,


    /** Drawing events*/
    GT_EVENT_TYPE_DRAW_START,                /* need to start draw widget */
    GT_EVENT_TYPE_DRAW_END,                  /* draw widget ok */

    /** Special events*/
    GT_EVENT_TYPE_UPDATE_VALUE,              /* value update, need to redraw */
    GT_EVENT_TYPE_SCROLL,

    /** Other events*/
    GT_EVENT_TYPE_CHANGE_CHILD_REMOVE,       /* remove child but not delete child */
    GT_EVENT_TYPE_CHANGE_CHILD_REMOVED,      /* remove child finish */
    GT_EVENT_TYPE_CHANGE_CHILD_DELETE,       /* delete child */
    GT_EVENT_TYPE_CHANGE_CHILD_DELETED,      /* delete child finish */
    GT_EVENT_TYPE_CHANGE_SIZE,               /* size was changed */
    GT_EVENT_TYPE_CHANGE_STYLE,              /* style was changed */


    _GT_EVENT_TYPE_TOTAL,                    /* count event number */
    
    GT_EVENT_TYPE_ALL = 0xFF,
}gt_event_type_et;

typedef struct _gt_event_s{
    struct _gt_obj_s * target;
    gt_event_type_et code;
    void * user_data;
}gt_event_st;

typedef void (* gt_event_cb_t)(gt_event_st * e);

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_obj_add_event_cb(struct _gt_obj_s * obj, gt_event_cb_t event, gt_event_type_et filter, void * user_data);
gt_event_type_et gt_event_get_code(gt_event_st * e);

gt_res_t gt_event_send(struct _gt_obj_s * parent, gt_event_type_et event, void * user_data);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_EVENT_TYPE_H_
