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

typedef struct _gt_point_s
{
    gt_size_t x;
    gt_size_t y;
}gt_point_st;

typedef struct _gt_obj_event_attr_s{
    gt_event_cb_t cb;
    void * user_data;
    gt_event_type_et filter;
}gt_obj_event_attr_st;

/**
 * @brief save all indev/disp/others dev data
 */
typedef struct _gt_obj_process_attr_s{
    gt_point_st point;      //click point(note: the point is for obj)
    gt_size_t x_scroll;     //scroll x distance
    gt_size_t y_scroll;     //scroll y distance
}gt_obj_process_attr_st;

typedef enum{
    GT_VISIBLE,
    GT_INVISIBLE,
}gt_visible_et;


struct _gt_draw_ctx_t;
typedef struct _gt_obj_s {
    struct _gt_obj_s * parent;
    struct _gt_obj_s ** child;
    const gt_obj_class_st * class;
#if GT_USE_USER_DATA
    void * user_data;                   /* Notice: must be manual free memory, when memory is not used it! */
#endif
    gt_obj_event_attr_st * event_attr;
    struct _gt_draw_ctx_t * draw_ctx;
    gt_obj_process_attr_st process_attr;

    uint32_t event_mask;                /* send event code add this mask */

    uint16_t cnt_child;
    uint8_t cnt_event;
    uint8_t opa;

    gt_area_st area;                    /* base area */

    style_widget_t style;

    gt_state_et state;  /* obj state */

    uint8_t     visible:1;  /* obj visible state, @gt_visible_e */
    uint8_t     focus:1;    /* obj focus state 0:no focus, 1:focus */
} gt_obj_st;


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


void gt_obj_destroy(gt_obj_st * obj);




#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_OBJ_H_
