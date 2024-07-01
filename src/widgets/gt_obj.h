/**
 * @file gt_obj.h
 * @author yongg
 * @brief Base object implementation
 * @version 0.1
 * @date 2022-05-11 15:08:01
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
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
    gt_point_st point;      //click point(note: the point is for object inside offset)
    gt_point_st scroll;     //scroll distance, default is 0
}gt_obj_process_attr_st;

typedef enum {
    GT_INVISIBLE = 0,
    GT_VISIBLE = 1,
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

typedef enum {
    GT_OBJ_TRIGGER_MODE_HOLD_ON = 0,
    GT_OBJ_TRIGGER_MODE_SWITCH  = 1,
}gt_obj_trigger_mode_et;


typedef struct gt_obj_s {
    struct gt_obj_s * parent;
    struct gt_obj_s ** child;
    const gt_obj_class_st * class;

    gt_obj_event_attr_st * event_attr;
    struct _gt_draw_ctx_s * draw_ctx;
    gt_obj_process_attr_st process_attr;

    gt_area_st area;                /* base area */

    gt_id_t id;                     /* obj id, register by user, default: [-1] invalid id, normal begin from 0. */
    gt_color_t bgcolor;

    uint16_t cnt_child;
    gt_radius_t radius;
    uint8_t cnt_event;
    uint8_t opa;            /* @ref gt_color.h */
    uint8_t reduce;

    uint32_t state         : 2;     /* obj selected state @ref gt_state_et */
    uint32_t touch_parent  : 1;     /* touch event will be called to parent object */
    uint32_t using         : 1;     /* screen was using or not, not the widgets object. 0:using [Can not be free memory], 1:free */
    uint32_t delate        : 1;     /* The status which be prepare to free the current object 0:no delate, 1:prepare to delate */
    uint32_t focus         : 1;     /* obj focus state 0:no focus, 1:focus */
    uint32_t visible       : 1;     /* obj visible state, @gt_visible_et */
    uint32_t disabled      : 1;     /* obj disabled state, 0:enable, 1:disable */

    uint32_t focus_dis     : 1;     /* obj focus enable , 0:enable, 1:disable*/
    uint32_t fixed         : 1;     /* obj fixed state, 0:unfixed, 1:fixed then search it parent's widget to scroll(Dependent on superclass properties) */
    uint32_t scroll_dir    : 2;     /* @see gt_scroll_dir_et [default: GT_SCROLL_ALL] */
    uint32_t scroll_l_r    : 1;     /* Enabled when GT_SCROLL_HORIZONTAL or GT_SCROLL_ALL @see gt_scroll_dir_et */
    uint32_t scroll_u_d    : 1;     /* Enabled when GT_SCROLL_VERTICAL or GT_SCROLL_ALL @see gt_scroll_dir_et */
    uint32_t scroll_snap_x : 2;     /* Scroll the alignment position @ref gt_scroll_snap_em */
    uint32_t scroll_snap_y : 2;     /* As scroll_snap_x @ref gt_scroll_snap_em */

    uint32_t absorb        : 1;     /* obj absorb state, 0:un-absorb, 1: Adsorption screen maximum boundary */
    uint32_t absorb_dir    : 1;     /* obj absorb direction(It only works when absorb is set to 1), 0: horizontal; 1: vertical */
    uint32_t overflow      : 1;     /* [Inheritable]obj overflow state, 0:un-overflow, 1:overflow (widget can out of screen area size) */
    uint32_t inside        : 1;     /* obj display only limited to parent area, 0:un-inside, 1:inside (widget is inside screen area size) */
    uint32_t virtual       : 1;     /* obj virtual state, 0:un-virtual[default], 1:virtual (widget is virtual logic, not entities) */
    uint32_t septal_line_y : 1;     /* draw obj vertical septal line state, 0:un-septal_line, 1:septal_line */

    uint32_t mask_effect   : 1;     /* Mask effect in the state of object selection: 1: Enabled, 0[default]:Disabled */
    uint32_t trigger_mode  : 1;     /* state toggle trigger mode @ref gt_obj_trigger_mode_et, 0:[default] hold-on; 1: switch */
    uint32_t bubble_notify : 1;     /* The object event will bubble notify to parent, 0: [default] disabled; 1: enabled */
    uint32_t untouchability: 1;     /* The object is untouchable, 0: [default] enabled; 1: disabled, can not be touch */
    uint32_t row_layout    : 1;     /* The child controls are arranged in a row layout */
    uint32_t grow_invert   : 1;     /* The child controls are arranged in a row layout, grow invert */
    uint32_t show_bg       : 1;     /* The object show background, 0: [default] hide; 1: show, bgcolor can be used */
    uint32_t reserved      : 1;
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
 * @brief Get widget parent object
 *
 * @param obj
 * @return gt_obj_st*
 */
gt_obj_st * gt_obj_get_parent(gt_obj_st * obj);

/**
 * @brief
 *
 * @param obj
 * @param to [Warn] Do not set to NULL
 * @return struct gt_obj_s* Parent pointer, if NULL is failed
 */
gt_obj_st * gt_obj_change_parent(gt_obj_st * obj, gt_obj_st * to);

/**
 * @brief Get widget or screen by object id
 *
 * @param widget_id widget or screen id
 * @return gt_obj_st*
 */
gt_obj_st * gt_obj_find_by_id(gt_id_t widget_id);

/**
 * @brief Set widget Id, default: -1 invalid id
 *
 * @param obj
 * @param id Normal begin from 0
 */
void gt_obj_register_id(gt_obj_st * obj, gt_id_t id);

/**
 * @brief Get widget Id
 *
 * @param obj
 * @return gt_id_t -1: invalid id; normal begin from 0
 */
gt_id_t gt_obj_get_id(gt_obj_st * obj);

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
 * @brief If you use the screen stack functionality,
 *      try to avoid using it to free screen objects directly
 *
 * @param obj widgets object
 */
void gt_obj_destroy(gt_obj_st * obj);

/**
 * @brief Set screen background color
 *
 * @param obj
 * @param color
 */
void gt_screen_set_bgcolor(gt_obj_st * obj, gt_color_t color);

/**
 * @brief Get screen background color
 *
 * @param obj
 * @return gt_color_t
 */
gt_color_t gt_screen_get_bgcolor(gt_obj_st * obj);

void gt_obj_set_bgcolor(gt_obj_st * obj, gt_color_t color);

gt_color_t gt_obj_get_bgcolor(gt_obj_st * obj);

gt_size_t gt_obj_get_limit_bottom(gt_obj_st * obj);

gt_size_t gt_obj_get_childs_max_height(gt_obj_st * parent);

/**
 * @brief Set object show background color or not
 *
 * @param obj
 * @param show true: show background; false[default]: hide background
 */
void gt_obj_show_bg(gt_obj_st * obj, bool show);

/**
 * @brief Get object show background color or not
 *
 * @param obj
 * @return true show background
 * @return false hide background
 */
bool gt_obj_is_show_bg(gt_obj_st * obj);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_OBJ_H_
