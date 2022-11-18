/**
 * @file gt_event.h
 * @author yongg
 * @brief Object implementation for the event handler
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

/**
 * @brief The event type enum definition
 */
typedef enum gt_event_type_e{
    GT_EVENT_TYPE_NONE = 0x0000,

    /* Input device events */
    GT_EVENT_TYPE_INPUT_CLICKING     = 1<<0,        ///< clicking
    GT_EVENT_TYPE_INPUT_PROCESS      = 1<<1,        ///< release,click need handler
    GT_EVENT_TYPE_INPUT_PROCESSED    = 1<<2,        ///< click handler finish
    GT_EVENT_TYPE_INPUT_SCROLL       = 1<<3,
    GT_EVENT_TYPE_INPUT_PROCESS_LOST = 1<<4,        ///< move off and lost handler

    /* update value */
    GT_EVENT_TYPE_UPDATE_VALUE = 1<<5,              ///< value update, need to redraw
    GT_EVENT_TYPE_UPDATE_STYLE = 1<<6,

    /* Drawing events */
    GT_EVENT_TYPE_DRAW_START = 1<<7,                ///< need to start draw widget
    GT_EVENT_TYPE_DRAW_END   = 1<<8,                ///< draw widget ok

    /* Other events */
    GT_EVENT_TYPE_CHANGE_CHILD_REMOVE  = 1<<9,      ///< remove child but not delete child
    GT_EVENT_TYPE_CHANGE_CHILD_REMOVED = 1<<10,     ///< remove child finish
    GT_EVENT_TYPE_CHANGE_CHILD_DELETE  = 1<<11,     ///< delete child
    GT_EVENT_TYPE_CHANGE_CHILD_DELETED = 1<<12,     ///< delete child finish

    _GT_EVENT_TYPE_TOTAL = 12,                      ///< count event number

    GT_EVENT_TYPE_ALL = 0xFFFFFFF,
}gt_event_type_et;

typedef struct _gt_event_s{
    struct _gt_obj_s * target;
    gt_event_type_et code;
    void * user_data;
}gt_event_st;

/**
 * @brief The event handler callback function pointer
 *
 * @param e An object describing the content of the event
 */
typedef void (* gt_event_cb_t)(gt_event_st * e);

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief Adding an event to an object
 *
 * @param obj Which object need to add event callback
 * @param event The callback function
 * @param filter The event type enum @ref gt_event_type_e
 * @param user_data User data [default or unused: NULL]
 */
void gt_obj_add_event_cb(struct _gt_obj_s * obj, gt_event_cb_t event, gt_event_type_et filter, void * user_data);

/**
 * @brief Get the code value for the event
 *
 * @param e The structure of the event
 * @return gt_event_type_et get newly code value @ref gt_event_type_e
 */
gt_event_type_et gt_event_get_code(gt_event_st * e);

/**
 * @brief Send an event code to the object kernel.
 *
 * @param parent Object which want to be notified
 * @param event The event code to send
 * @param user_data User data [default or unused: NULL]
 * @return gt_res_t The result status @ref gt_res_t
 */
gt_res_t gt_event_send(struct _gt_obj_s * parent, gt_event_type_et event, void * user_data);

/**
 * @brief clean the object events
 *
 * @param parent Object which want to change
 */
void gt_event_clr_mask(struct _gt_obj_s * parent);

void _gt_event_init(void);

/**
 * @brief if called event handler it will be locked, otherwise it will unlocked
 *
 * @return true Currently executing event
 * @return false No event is currently being handled
 */
bool gt_event_is_locked(void);

/**
 * @brief An object that is locked when handling an event
 *
 * @return struct gt_obj_s*
 */
struct gt_obj_s * gt_event_get_locked_obj(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_EVENT_TYPE_H_
