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
    gt_color_t bgcolor;

    gt_area_st area;                    /* base area */

    uint32_t event_mask;                /* send event code add this mask */

    uint16_t cnt_child;
    uint8_t cnt_event;
    uint8_t opa;            /* @ref gt_color.h */

    uint32_t state         : 3;    /* obj selected state @ref gt_state_et */
    uint32_t using         : 1;    /* obj using state 0:using [Can not be free memory], 1:free */
    uint32_t delate        : 1;    /* Prepare to free the current object 0:no delate, 1:delate */
    uint32_t focus         : 1;    /* obj focus state 0:no focus, 1:focus */
    uint32_t visible       : 1;    /* obj visible state, @gt_visible_e */
    uint32_t disabled      : 1;    /* obj disabled state, 0:enable, 1:disable */

    uint32_t focus_dis     : 1;    /* obj focus enable , 0:enable, 1:disable*/
    uint32_t fixed         : 1;    /* obj fixed state, 0:unfixed, 1:fixed then search it parent's widget to scroll(Dependent on superclass properties) */
    uint32_t scroll_dir    : 2;    /* @see gt_scroll_dir_et [default: GT_SCROLL_ALL] */
    uint32_t scroll_l_r    : 1;    /* Enabled when GT_SCROLL_HORIZONTAL or GT_SCROLL_ALL @see gt_scroll_dir_et */
    uint32_t scroll_u_d    : 1;    /* Enabled when GT_SCROLL_VERTICAL or GT_SCROLL_ALL @see gt_scroll_dir_et */
    uint32_t scroll_snap_x : 2;    /* Scroll the alignment position @see gt_scroll_snap_em */
    uint32_t scroll_snap_y : 2;    /* As scroll_snap_x */

    uint32_t absorb        : 1;    /* obj absorb state, 0:un-absorb, 1: Adsorption screen maximum boundary */
    uint32_t absorb_dir    : 1;    /* obj absorb direction(It only works when absorb is set to 1), 0: horizontal; 1: vertical */
    uint32_t overflow      : 1;    /* [Inheritable]obj overflow state, 0:un-overflow, 1:overflow (widget can out of screen area size) */
    uint32_t inside        : 1;    /* obj display only limited to parent area, 0:un-inside, 1:inside (widget is inside screen area size) */
    uint32_t virtual       : 1;    /* obj virtual state, 0:un-virtual[default], 1:virtual (widget is virtual logic, not entities) */
    uint32_t reserved      : 9;

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
 * @return struct gt_obj_s* Parent pointer, if NULL is failed
 */
gt_obj_st * gt_obj_change_parent(gt_obj_st * obj, gt_obj_st * to);

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
