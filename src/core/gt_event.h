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


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef enum gt_event_type_e{
    GT_EVENT_TYPE_ALL = 0,

    /** Input device events*/
    GT_EVENT_TYPE_PRESSED,             /**< The object has been pressed*/
    GT_EVENT_TYPE_PRESSING,            /**< The object is being pressed (called continuously while pressing)*/
    GT_EVENT_TYPE_PRESS_LOST,          /**< The object is still being pressed but slid cursor/finger off of the object */
    GT_EVENT_TYPE_SHORT_CLICKED,       /**< The object was pressed for a short period of time, then released it. Not called if scrolled.*/
    GT_EVENT_TYPE_LONG_PRESSED,        /**< Object has been pressed for at least `long_press_time`.  Not called if scrolled.*/
    GT_EVENT_TYPE_LONG_PRESSED_REPEAT, /**< Called after `long_press_time` in every `long_press_repeat_time` ms.  Not called if scrolled.*/
    GT_EVENT_TYPE_CLICKED,             /**< Called on release if not scrolled (regardless to long press)*/
    GT_EVENT_TYPE_RELEASED,            /**< Called in every cases when the object has been released*/
    GT_EVENT_TYPE_SCROLL_BEGIN,        /**< Scrolling begins*/
    GT_EVENT_TYPE_SCROLL_END,          /**< Scrolling ends*/
    GT_EVENT_TYPE_SCROLL,              /**< Scrolling*/
    GT_EVENT_TYPE_GESTURE,             /**< A gesture is detected. Get the gesture with `GT_indev_get_gesture_dir(GT_indev_get_act());` */
    GT_EVENT_TYPE_KEY,                 /**< A key is sent to the object. Get the key with `GT_indev_get_key(GT_indev_get_act());`*/
    GT_EVENT_TYPE_FOCUSED,             /**< The object is focused*/
    GT_EVENT_TYPE_DEFOCUSED,           /**< The object is defocused*/
    GT_EVENT_TYPE_LEAVE,               /**< The object is defocused but still selected*/
    GT_EVENT_TYPE_HIT_TEST,            /**< Perform advanced hit-testing*/

    /** Drawing events*/
    GT_EVENT_TYPE_COVER_CHECK,        /**< Check if the object fully covers an area. The event parameter is `GT_cover_check_info_t *`.*/
    GT_EVENT_TYPE_REFR_EXT_DRAW_SIZE, /**< Get the required extra draw area around the object (e.g. for shadow). The event parameter is `GT_coord_t *` to store the size.*/
    GT_EVENT_TYPE_DRAW_MAIN_BEGIN,    /**< Starting the main drawing phase*/
    GT_EVENT_TYPE_DRAW_MAIN,          /**< Perform the main drawing*/
    GT_EVENT_TYPE_DRAW_MAIN_END,      /**< Finishing the main drawing phase*/
    GT_EVENT_TYPE_DRAW_POST_BEGIN,    /**< Starting the post draw phase (when all children are drawn)*/
    GT_EVENT_TYPE_DRAW_POST,          /**< Perform the post draw phase (when all children are drawn)*/
    GT_EVENT_TYPE_DRAW_POST_END,      /**< Finishing the post draw phase (when all children are drawn)*/
    GT_EVENT_TYPE_DRAW_PART_BEGIN,    /**< Starting to draw a part. The event parameter is `GT_obj_draw_dsc_t *`. */
    GT_EVENT_TYPE_DRAW_PART_END,      /**< Finishing to draw a part. The event parameter is `GT_obj_draw_dsc_t *`. */

    /** Special events*/
    GT_EVENT_TYPE_VALUE_CHANGED,       /**< The object's value has changed (i.e. slider moved)*/
    GT_EVENT_TYPE_INSERT,              /**< A text is inserted to the object. The event data is `char *` being inserted.*/
    GT_EVENT_TYPE_REFRESH,             /**< Notify the object to refresh something on it (for the user)*/
    GT_EVENT_TYPE_READY,               /**< A process has finished*/
    GT_EVENT_TYPE_CANCEL,              /**< A process has been cancelled */

    /** Other events*/
    GT_EVENT_TYPE_DELETE,              /**< Object is being deleted*/
    GT_EVENT_TYPE_CHILD_CHANGED,       /**< Child was removed, added, or its size, position were changed */
    GT_EVENT_TYPE_CHILD_CREATED,       /**< Child was created, always bubbles up to all parents*/
    GT_EVENT_TYPE_CHILD_DELETED,       /**< Child was deleted, always bubbles up to all parents*/
    GT_EVENT_TYPE_SCREEN_UNLOAD_START, /**< A screen unload started, fired immediately when scr_load is called*/
    GT_EVENT_TYPE_SCREEN_LOAD_START,   /**< A screen load started, fired when the screen change delay is expired*/
    GT_EVENT_TYPE_SCREEN_LOADED,       /**< A screen was loaded*/
    GT_EVENT_TYPE_SCREEN_UNLOADED,     /**< A screen was unloaded*/
    GT_EVENT_TYPE_SIZE_CHANGED,        /**< Object coordinates/size have changed*/
    GT_EVENT_TYPE_STYLE_CHANGED,       /**< Object's style has changed*/
    GT_EVENT_TYPE_LAYOUT_CHANGED,      /**< The children position has changed due to a layout recalculation*/
    GT_EVENT_TYPE_GET_SELF_SIZE,       /**< Get the internal size of a widget*/

    _GT_EVENT_TYPE_LAST,               /** Number of default events*/


    GT_EVENT_TYPE_PREPROCESS = 0x80,   /** This is a flag that can be set with an event so it's processed
                                      before the class default event processing */
}gt_event_type_et;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_EVENT_TYPE_H_
