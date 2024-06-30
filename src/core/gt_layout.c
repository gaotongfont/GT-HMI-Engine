/**
 * @file gt_layout.c
 * @author Feyoung
 * @brief Logical handling of layers
 * @version 0.1
 * @date 2024-03-29 19:53:39
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_layout.h"
#include "stddef.h"
#include "../core/gt_style.h"
#include "../others/gt_log.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_size_t _row_grow_inside(gt_obj_st * target, gt_point_st * offset) {
    gt_obj_st * child = NULL;
    gt_size_t y = 0;
    for (gt_size_t i = 0, cnt = target->cnt_child; i < cnt; ++i) {
        child = target->child[i];
        if (GT_INVISIBLE == gt_obj_get_visible(child)) {
            continue;
        }
        if (gt_obj_get_virtual(child)) {
            offset->x = _row_grow_inside(child, offset);
            continue;
        }
        y = gt_obj_get_y(child);
        if (y < offset->y) {
            y = offset->y;
        } else if (y > offset->y + gt_obj_get_h(target)) {
            y = offset->y;
        }
        gt_obj_set_pos(child, offset->x, y);
        offset->x += gt_obj_get_w(child);
    }

    return offset->x;
}

static gt_size_t _row_grow_inside_invert(gt_obj_st * target, gt_point_st * offset) {
    gt_obj_st * child = NULL;
    gt_size_t y = 0;
    for (gt_size_t i = 0, cnt = target->cnt_child; i < cnt; ++i) {
        child = target->child[i];
        if (GT_INVISIBLE == gt_obj_get_visible(child)) {
            continue;
        }
        if (gt_obj_get_virtual(child)) {
            offset->x = _row_grow_inside_invert(child, offset);
            continue;
        }
        y = gt_obj_get_y(child);
        if (y < offset->y) {
            y = offset->y;
        } else if (y > offset->y + gt_obj_get_h(target)) {
            y = offset->y;
        }
        offset->x -= gt_obj_get_w(child);
        gt_obj_set_pos(child, offset->x, y);
    }

    return offset->x;
}


/* global functions / API interface -------------------------------------*/

void gt_layout_row_grow(gt_obj_st * obj)
{
    GT_CHECK_BACK(obj);
    if (false == obj->row_layout) {
        return ;
    }
    if (0 == obj->cnt_child) {
        return ;
    }
    gt_point_st offset = {
        .x =  obj->area.x,
        .y = obj->area.y
    };

    if (obj->grow_invert) {
        offset.x += gt_obj_get_w(obj);
        offset.x = _row_grow_inside_invert(obj, &offset);
    } else {
        offset.x = _row_grow_inside(obj, &offset);
    }
}


/* end ------------------------------------------------------------------*/
