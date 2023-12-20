/**
 * @file gt_group.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-20 15:51:14
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_group.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_event.h"
#include "../core/gt_disp.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_GROUP
#define MY_CLASS    &gt_group_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_group_s {
    uint8_t reserved;
}_gt_group_st;

/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_group_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_group_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

/**
 * @brief obj init group widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
}

/**
 * @brief obj deinit call back
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj) {
}


/**
 * @brief obj event handler call back
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);

    if (GT_EVENT_TYPE_DRAW_START == code) {
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
    }
    else if (GT_EVENT_TYPE_UPDATE_STYLE == code) {
        if (NULL == obj->parent) {
            return;
        }
        gt_area_copy(&obj->area, &obj->parent->area);
    }
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief create a group obj
 *
 * @param parent group's parent element
 * @return gt_obj_st* group obj
 */
gt_obj_st * gt_group_create(gt_obj_st * parent)
{
    if (NULL == parent) {
        return NULL;
    }
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_group_st * style = (_gt_group_st * )obj->style;

    gt_memset(style, 0, sizeof(_gt_group_st));
    gt_obj_set_virtual(obj, true);
    if (obj->parent) {
        gt_area_copy(&obj->area, &obj->parent->area);
    }
    return obj;
}

/* end ------------------------------------------------------------------*/
