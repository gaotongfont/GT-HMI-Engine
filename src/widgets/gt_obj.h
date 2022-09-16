/**
 * @file gt_obj.h
 * @author yongg
 * @brief
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
typedef struct gt_line_s{
    gt_size_t xs;
    gt_size_t ys;
    gt_size_t xe;
    gt_size_t ye;
}gt_line_st;

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
}gt_visible_e;

typedef enum{
    GT_UNCACHED,
    GT_CACHED,
}gt_cache_e;

typedef enum{
    GT_CREATE_TYPE_FULL,
    GT_CREATE_TYPE_PART,
}gt_create_type_e;

struct _gt_draw_ctx_t;
typedef struct _gt_obj_s {
    struct _gt_obj_s * parent;
    struct _gt_obj_s ** child;
    const gt_obj_class_st * class;
#if GT_USE_USER_DATA
    void * user_data;
#endif
    gt_obj_event_attr_st * event_attr;
    struct _gt_draw_ctx_t * draw_ctx;
    gt_obj_process_attr_st process_attr;

    uint32_t event_mask;                /* send event code add this mask */

    uint16_t cnt_child;
    uint8_t cnt_event;
    uint8_t opa_bg;

    gt_area_st area;                    /* base area */

    style_widget_t style;

    uint8_t visible;  /* obj visible state, @gt_visible_e */
} gt_obj_st;


/* private typedef  */
typedef struct _gt_style_obj_s
{
    uint8_t selected;
}_gt_style_obj_st;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st *gt_obj_create(gt_obj_st *parent);
void gt_obj_del(gt_obj_st * obj);


void gt_init_area(gt_area_st * area, gt_size_t x, gt_size_t y, gt_size_t w, gt_size_t h);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_OBJ_H_
