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
    GT_EVENT_TYPE_NONE = 0x0000,

    /** Input device events*/
    GT_EVENT_TYPE_INPUT_CLICKING    = 1<<0,     /* clicking */
    GT_EVENT_TYPE_INPUT_PROCESS     = 1<<1,     /* click need handler */
    GT_EVENT_TYPE_INPUT_PROCESSED   = 1<<2,     /* click handler finish */
    GT_EVENT_TYPE_INPUT_SCROLL      = 1<<3,

    /* update value */
    GT_EVENT_TYPE_UPDATE_VALUE      = 1<<4,             /* value update, need to redraw */
    GT_EVENT_TYPE_UPDATE_STYLE      = 1<<5,

    /** Drawing events*/
    GT_EVENT_TYPE_DRAW_START        = 1<<6,             /* need to start draw widget */
    GT_EVENT_TYPE_DRAW_END          = 1<<7,             /* draw widget ok */

    /** Other events*/
    GT_EVENT_TYPE_CHANGE_CHILD_REMOVE   = 1<<8,         /* remove child but not delete child */
    GT_EVENT_TYPE_CHANGE_CHILD_REMOVED  = 1<<9,         /* remove child finish */
    GT_EVENT_TYPE_CHANGE_CHILD_DELETE   = 1<<10,        /* delete child */
    GT_EVENT_TYPE_CHANGE_CHILD_DELETED  = 1<<11,        /* delete child finish */

    _GT_EVENT_TYPE_TOTAL = 11,                          /* count event number */

    GT_EVENT_TYPE_ALL = 0xFFFFFFF,
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

void _gt_event_init(void);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_EVENT_TYPE_H_
