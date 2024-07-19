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
#include "./gt_style.h"
#include "../others/gt_log.h"
#include "./gt_draw.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/
typedef struct {
    void (* update_align_pos_p)(gt_obj_st *, gt_area_st *, gt_size_t, gt_layout_align_items_t);
    void (* update_max_size_p)(gt_obj_st *, gt_area_st *, gt_size_t);
}_update_align_cb_st;

typedef struct {
    uint16_t offset;
    uint16_t gap;
}_content_space_around_st;




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

static inline bool _is_flex_row_dir(gt_layout_flex_direction_t dir) {
    return (GT_LAYOUT_FLEX_DIR_ROW == dir || GT_LAYOUT_FLEX_DIR_ROW_REVERSE ==  dir) ? true : false;
}

static void _update_flex_align_pos_row(gt_obj_st * child, gt_area_st * pos, gt_size_t second_dir_height, gt_layout_align_items_t align) {
    if (GT_LAYOUT_ALIGN_ITEMS_START == align) {
        gt_obj_set_pos(child, pos->x + pos->w, pos->y);
    } else if (GT_LAYOUT_ALIGN_ITEMS_END == align) {
        gt_obj_set_pos(child, pos->x + pos->w, pos->y + second_dir_height - child->area.h);
    } else if (GT_LAYOUT_ALIGN_ITEMS_CENTER == align) {
        gt_obj_set_pos(child, pos->x + pos->w, ((second_dir_height - child->area.h) >> 1) + pos->y);
    }
}

static void _update_flex_align_pos_column(gt_obj_st * child, gt_area_st * pos, gt_size_t second_dir_height, gt_layout_align_items_t align) {
    if (GT_LAYOUT_ALIGN_ITEMS_START == align) {
        gt_obj_set_pos(child, pos->x, pos->y + pos->h);
    } else if (GT_LAYOUT_ALIGN_ITEMS_END == align) {
        gt_obj_set_pos(child, pos->x + second_dir_height - child->area.w, pos->y + pos->h);
    } else if (GT_LAYOUT_ALIGN_ITEMS_CENTER == align) {
        gt_obj_set_pos(child, ((second_dir_height - child->area.w) >> 1) + pos->x, pos->y + pos->h);
    }
}

static void _update_flex_area_max_size_row(gt_obj_st * child, gt_area_st * pos, gt_size_t gap) {
    pos->w += child->area.w + gap;
    if (child->area.h > pos->h) {
        pos->h = child->area.h;
    }
}

static void _update_flex_area_max_size_column(gt_obj_st * child, gt_area_st * pos, gt_size_t gap) {
    pos->h += child->area.h + gap;
    if (child->area.w > pos->w) {
        pos->w = child->area.w;
    }
}

static _update_align_cb_st _get_update_align_pos_cb(bool is_row) {
    _update_align_cb_st ret = { 0 };
    if (is_row) {
        ret.update_align_pos_p = _update_flex_align_pos_row;
        ret.update_max_size_p = _update_flex_area_max_size_row;
    } else {
        ret.update_align_pos_p = _update_flex_align_pos_column;
        ret.update_max_size_p = _update_flex_area_max_size_column;
    }
    return ret;
}

static _content_space_around_st _space_around_align(gt_obj_st * parent, bool is_row_dir) {
    uint16_t total_size = 0;
    _content_space_around_st ret = {0};

    if (is_row_dir) {
        for (gt_size_t i = 0; i < parent->cnt_child; ++i) {
            GT_LOG_A("", "%d. %d", i, parent->child[i]->area.w);
            total_size += parent->child[i]->area.w;
        }
        GT_LOG_A("", "total: %d", total_size);
        if (total_size == parent->area.w) {
            return ret;
        }
        if (total_size > parent->area.w) {
            ret.offset = (parent->area.w - total_size) >> 1;
            return ret;
        }
        ret.gap = ((parent->area.w - total_size) / (parent->cnt_child + 1));
        ret.offset = ret.gap >> 1;
        return ret;
    }
    /** column dir */
    for (gt_size_t i = 0; i < parent->cnt_child; ++i) {
        total_size += parent->child[i]->area.h;
    }
    if (total_size == parent->area.h) {
        return ret;
    }
    if (total_size > parent->area.h) {
        ret.offset = (parent->area.h - total_size) >> 1;
        return ret;
    }
    ret.gap = ((parent->area.h - total_size) / (parent->cnt_child + 1));
    ret.offset = parent->container.gap >> 1;
    return ret;
}

static gt_area_st _childs_sort_by_flex_direction(gt_obj_st * parent) {
    gt_obj_st * child = NULL;
    gt_area_st pos = { .x = parent->area.x, .y = parent->area.y, .w = 0, .h = 0 };  /** w/h temp to calc offset */
    gt_size_t i = 0, cnt = parent->cnt_child;
    gt_gap_t gap = parent->container.gap;
    gt_layout_flex_direction_t dir = parent->container.flex_direction;
    gt_layout_align_items_t align = parent->container.align_items;

    uint8_t is_row = _is_flex_row_dir(dir);
    _update_align_cb_st cb_st = _get_update_align_pos_cb(is_row);
    gt_size_t second_dir_height = is_row ? parent->area.h : parent->area.w;
    _content_space_around_st sa_ret = {0};

    if (GT_LAYOUT_JUSTIFY_CONTENT_SPACE_AROUND == parent->container.justify_content) {
        /** pre calc real total width or height value */
        sa_ret = _space_around_align(parent, _is_flex_row_dir(parent->container.flex_direction));
        parent->container.gap = sa_ret.gap;
    }

    if (GT_LAYOUT_FLEX_DIR_ROW == dir || GT_LAYOUT_FLEX_DIR_COLUMN == dir) {
        pos.w += sa_ret.offset;
        for (i = 0; i < cnt; ++i) {
            child = parent->child[i];
            /** align second dir */
            cb_st.update_align_pos_p(child, &pos, second_dir_height, align);
            cb_st.update_max_size_p(child, &pos, gap);
        }
    } else {
        pos.h += sa_ret.offset;
        for (i = cnt - 1; i >= 0; --i) {
            child = parent->child[i];
            /** align second dir */
            cb_st.update_align_pos_p(child, &pos, second_dir_height, align);
            cb_st.update_max_size_p(child, &pos, gap);
        }
    }
    if (is_row) {
        if (pos.w > parent->area.w) { parent->area.w = pos.w; }
    } else {
        if (pos.h > parent->area.h) { parent->area.h = pos.h; }
    }
    return pos;
}

/**
 * @brief
 *
 * @param parent
 * @param content_pos
 * @return gt_area_st
 */
static gt_area_st _childs_adjust_justify_content(gt_obj_st * parent, gt_area_st * content_pos) {
    gt_layout_justify_content_t jc = parent->container.justify_content;
    gt_point_st diff = {0};

    if (GT_LAYOUT_JUSTIFY_CONTENT_START == jc) {
        return *content_pos;
    }
    if (GT_LAYOUT_JUSTIFY_CONTENT_SPACE_AROUND == jc) {
    //     return _space_around_align(parent, content_pos, _is_flex_row_dir(parent->container.flex_direction));
        return *content_pos;
    }

    // justify content end pos
    if (_is_flex_row_dir(parent->container.flex_direction)) {
        diff.x = (parent->area.x + parent->area.w - content_pos->w) - content_pos->x;
    } else {
        diff.y = (parent->area.y + parent->area.h - content_pos->h) - content_pos->y;
    }
    if (GT_LAYOUT_JUSTIFY_CONTENT_CENTER == jc) {
        diff.x >>= 1;
        diff.y >>= 1;
    }
    _gt_obj_move_child_by(parent, diff.x, diff.y);

    return *content_pos;
}

static void _adjust_flex_content(gt_obj_st * target) {
    if (false == gt_layout_is_type(target, GT_LAYOUT_TYPE_FLEX)) {
        return ;
    }
    gt_area_st pos = _childs_sort_by_flex_direction(target);
    _childs_adjust_justify_content(target, &pos);
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

void gt_layout_init(gt_obj_st * obj, gt_obj_container_st const * const container)
{
    GT_CHECK_BACK(obj);
    GT_CHECK_BACK(container);

    obj->container = *container;
    if (0 == obj->cnt_child) {
        return;
    }
    gt_layout_type_t type = obj->container.layout_type;
    if (GT_LAYOUT_TYPE_FIXED == type) {
        return;
    } else if (GT_LAYOUT_TYPE_FLEX == type) {
        gt_layout_update_core(obj);
    }
}

gt_res_t gt_layout_update_core(gt_obj_st * obj)
{
    GT_CHECK_BACK_VAL(obj, GT_RES_FAIL);

    if (obj->classes->_init_cb) {
        struct _gt_draw_ctx_s tmp_draw_ctx = {
            .parent_area = obj->inside ? &obj->parent->area : NULL,
            .buf_area = obj->area,
        };
        obj->draw_ctx = &tmp_draw_ctx;
        obj->classes->_init_cb(obj);
        obj->draw_ctx = NULL;
    }

    _adjust_flex_content(obj);
    _adjust_flex_content(obj->parent);
    return GT_RES_OK;
}

enum gt_layout_type_e gt_layout_get_type(gt_obj_st * obj)
{
    GT_CHECK_BACK_VAL(obj, GT_LAYOUT_TYPE_FIXED);
    return obj->container.layout_type;
}

bool gt_layout_is_type(gt_obj_st * obj, enum gt_layout_type_e type)
{
    GT_CHECK_BACK_VAL(obj, false);
    if (type >= _GT_LAYOUT_TYPE_TOTAL) {
        return false;
    }
    return (type == obj->container.layout_type) ? true : false;
}

/* end ------------------------------------------------------------------*/
