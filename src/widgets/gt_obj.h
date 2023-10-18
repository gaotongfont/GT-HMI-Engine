/**
 * @file gt_obj.h
 * @author yongg
 * @brief Base object implementation
 * @version 0.1
 * @date 2022-05-11 15:08:01
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_OBJ_H_
#define _GT_OBJ_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../others/gt_types.h"
#include "../others/gt_color.h"
#include "gt_obj_class.h"
#include "../core/gt_event.h"
#include "stdbool.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/

typedef struct _gt_point_s {
    gt_size_t x;
    gt_size_t y;
}gt_point_st;

typedef struct _gt_obj_event_attr_s {
    gt_event_cb_t user_cb;
    void * user_data;
    gt_event_type_et filter;
}gt_obj_event_attr_st;

/**
 * @brief save all indev/disp/others dev data
 */
typedef struct _gt_obj_process_attr_s {
    gt_point_st point;      //click point(note: the point is for obj)
    gt_point_st scroll;     //scroll distance
}gt_obj_process_attr_st;

typedef enum {
    GT_VISIBLE = 0,
    GT_INVISIBLE = 1,
}gt_visible_et;

typedef enum {
    GT_ENABLED = 0,
    GT_DISABLED = 1,
}gt_disabled_et;

typedef enum {
    GT_SCROLL_DISABLE    = 0,
    GT_SCROLL_HORIZONTAL = 1,
    GT_SCROLL_VERTICAL   = 2,
    GT_SCROLL_ALL        = 3,
}gt_scroll_dir_et;

typedef enum {
    GT_SCROLL_SNAP_NONE = 0,
    GT_SCROLL_SNAP_START,
    GT_SCROLL_SNAP_END,
}gt_scroll_snap_em;



typedef struct gt_obj_s {
    struct gt_obj_s * parent;
    struct gt_obj_s ** child;
    const gt_obj_class_st * class;
#if GT_USE_USER_DATA
    void * user_data;                   /* Notice: must be manual free memory, when memory is not used it! */
#endif
    gt_obj_event_attr_st * event_attr;
    struct _gt_draw_ctx_t * draw_ctx;
    gt_obj_process_attr_st process_attr;

    style_widget_t style;

    gt_area_st area;                    /* base area */

    uint32_t event_mask;                /* send event code add this mask */

    uint16_t cnt_child;
    uint8_t cnt_event;
    uint8_t opa;            /* @ref gt_color.h */

    uint8_t state         : 3;    /* obj state @ref gt_state_et */
    uint8_t using         : 1;    /* obj using state 0:using [Can not be free memory], 1:free */
    uint8_t delate        : 1;    /* Prepare to free the current object 0:no delate, 1:delate */
    uint8_t focus         : 1;    /* obj focus state 0:no focus, 1:focus */
    uint8_t visible       : 1;    /* obj visible state, @gt_visible_e */
    uint8_t disabled      : 1;    /* obj disabled state, 0:enable, 1:disable */

    uint8_t focus_dis     : 1;    /* obj focus enable , 0:enable, 1:disable*/
    uint8_t fixed         : 1;    /* obj fixed state, 0:unfixed, 1:fixed(Dependent on superclass properties) */
    uint8_t scroll_dir    : 2;    /* @see gt_scroll_em (Bit Set) [default: GT_SCROLL_ALL] */
    uint8_t scroll_snap_x : 2;    /* Scroll the alignment position @see gt_scroll_snap_em */
    uint8_t scroll_snap_y : 2;    /* As scroll_snap_x */

    uint8_t absorb        : 1;    /* obj absorb state, 0:un-absorb, 1: Adsorption screen maximum boundary */
    uint8_t absorb_dir    : 1;    /* obj absorb direction(It only works when absorb is set to 1), 0: horizontal; 1: vertical */
    uint8_t overflow      : 1;    /* [Inheritable]obj overflow state, 0:un-overflow, 1:overflow (widget can out of screen area size) */
    uint8_t reserved      : 5;

}gt_obj_st;


/* private typedef  */



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief create obj, recommend the obj will as screen obj
 *
 * @param parent obj's parent obj
 * @return gt_obj_st* obj ptr
 */
gt_obj_st *gt_obj_create(gt_obj_st *parent);

/**
 * @brief
 *
 * @param obj
 * @param to
 * @return bool true: success, false: failed
 */
bool gt_obj_change_parent(gt_obj_st * obj, gt_obj_st * to);

/**
 * @brief
 *
 * @param obj
 * @param parent
 * @return true
 * @return false
 */
bool gt_obj_is_child(gt_obj_st * obj, gt_obj_st * parent);

/**
 * @brief
 *
 * @param obj
 */
void gt_obj_destroy(gt_obj_st * obj);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_OBJ_H_
