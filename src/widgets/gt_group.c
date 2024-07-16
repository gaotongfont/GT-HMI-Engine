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

#if GT_CFG_ENABLE_GROUP
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
    gt_obj_st obj;
}_gt_group_st;

/* static variables -----------------------------------------------------*/
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_group_class = {
    ._init_cb      = NULL,
    ._deinit_cb    = NULL,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_group_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void _set_infinite_area(gt_obj_st * obj) {
    /** Set an infinite range -32767 ~ 32766 */
    obj->area.x = 1 - ((1 << ((sizeof(gt_size_t) << 3) - 1)));
    obj->area.y = 1 - ((1 << ((sizeof(gt_size_t) << 3) - 1)));
    obj->area.w = ((1 << (sizeof(gt_size_t) << 3))) - 2;
    obj->area.h = ((1 << (sizeof(gt_size_t) << 3))) - 2;
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
        _set_infinite_area(obj);
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
    if (NULL == obj) {
        return obj;
    }

    gt_obj_set_virtual(obj, true);
    _set_infinite_area(obj);

    return obj;
}

gt_obj_st * gt_group_get_active_obj(gt_obj_st * group, gt_obj_type_et type)
{
    if (OBJ_TYPE != gt_obj_class_get_type(group)) {
        return NULL;
    }
    for (uint16_t i = 0, cnt = group->cnt_child; i < cnt; i++) {
        if (type != gt_obj_class_get_type(group->child[i])) {
            continue;
        }
        if (GT_STATE_PRESSED == gt_obj_get_state(group->child[i])) {
            return group->child[i];
        }
    }
    return NULL;
}

bool gt_group_reset_selected_state(gt_obj_st * group, gt_obj_type_et type)
{
    if (OBJ_TYPE != gt_obj_class_get_type(group)) {
        return false;
    }
    gt_obj_st * first_obj = NULL;
    for (uint16_t i = 0, cnt = group->cnt_child; i < cnt; i++) {
        if (type != gt_obj_class_get_type(group->child[i])) {
            continue;
        }
        if (NULL == first_obj) {
            first_obj = group->child[i];
        }
        if (GT_STATE_NONE != gt_obj_get_state(group->child[i])) {
            gt_obj_set_state(group->child[i], GT_STATE_NONE);
            gt_event_send(group->child[i], GT_EVENT_TYPE_DRAW_START, NULL);
        }
    }
    gt_obj_set_state(first_obj, GT_STATE_PRESSED);
    gt_event_send(first_obj, GT_EVENT_TYPE_DRAW_START, NULL);
    return true;
}

#endif  /** GT_CFG_ENABLE_GROUP */
/* end ------------------------------------------------------------------*/
