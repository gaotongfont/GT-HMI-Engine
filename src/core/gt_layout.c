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
static GT_ATTRIBUTE_RAM_TEXT gt_size_t _row_grow_inside(gt_obj_st * target, gt_point_st * offset) {
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

static GT_ATTRIBUTE_RAM_TEXT gt_size_t _row_grow_inside_invert(gt_obj_st * target, gt_point_st * offset) {
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

#if GT_USE_WIDGET_LAYOUT
static GT_ATTRIBUTE_RAM_TEXT inline bool _is_flex_row_dir(gt_layout_flex_direction_t dir) {
    return (GT_LAYOUT_FLEX_DIR_ROW == dir || GT_LAYOUT_FLEX_DIR_ROW_REVERSE ==  dir) ? true : false;
}

static GT_ATTRIBUTE_RAM_TEXT void _update_flex_align_pos_row(gt_obj_st * child, gt_area_st * pos, gt_size_t second_dir_height, gt_layout_align_items_t align) {
    if (GT_LAYOUT_ALIGN_ITEMS_START == align) {
        gt_obj_set_pos(child, pos->x + pos->w, pos->y);
    } else if (GT_LAYOUT_ALIGN_ITEMS_END == align) {
        gt_obj_set_pos(child, pos->x + pos->w, pos->y + second_dir_height - child->area.h);
    } else if (GT_LAYOUT_ALIGN_ITEMS_CENTER == align) {
        gt_obj_set_pos(child, pos->x + pos->w, ((second_dir_height - child->area.h) >> 1) + pos->y);
    }
}

static GT_ATTRIBUTE_RAM_TEXT void _update_flex_align_pos_column(gt_obj_st * child, gt_area_st * pos, gt_size_t second_dir_height, gt_layout_align_items_t align) {
    if (GT_LAYOUT_ALIGN_ITEMS_START == align) {
        gt_obj_set_pos(child, pos->x, pos->y + pos->h);
    } else if (GT_LAYOUT_ALIGN_ITEMS_END == align) {
        gt_obj_set_pos(child, pos->x + second_dir_height - child->area.w, pos->y + pos->h);
    } else if (GT_LAYOUT_ALIGN_ITEMS_CENTER == align) {
        gt_obj_set_pos(child, ((second_dir_height - child->area.w) >> 1) + pos->x, pos->y + pos->h);
    }
}

static GT_ATTRIBUTE_RAM_TEXT void _update_flex_area_max_size_row(gt_obj_st * child, gt_area_st * pos, gt_size_t gap) {
    pos->w += child->area.w + gap;
    if (child->area.h > pos->h) {
        pos->h = child->area.h;
    }
}

static GT_ATTRIBUTE_RAM_TEXT void _update_flex_area_max_size_column(gt_obj_st * child, gt_area_st * pos, gt_size_t gap) {
    pos->h += child->area.h + gap;
    if (child->area.w > pos->w) {
        pos->w = child->area.w;
    }
}

static GT_ATTRIBUTE_RAM_TEXT _update_align_cb_st _get_update_align_pos_cb(bool is_row) {
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

static GT_ATTRIBUTE_RAM_TEXT bool _is_justify_content_space_mode(gt_layout_justify_content_t jc) {
    return (GT_LAYOUT_JUSTIFY_CONTENT_SPACE_AROUND == jc ||
            GT_LAYOUT_JUSTIFY_CONTENT_SPACE_BETWEEN == jc ||
            GT_LAYOUT_JUSTIFY_CONTENT_SPACE_EVENLY == jc) ? true : false;
}

static GT_ATTRIBUTE_RAM_TEXT _content_space_around_st _common_calc_space_gap_and_offset(
    uint16_t obj_size, uint16_t gap_count, uint16_t parent_radius,
    uint16_t total_size, gt_layout_justify_content_t jc) {
    _content_space_around_st ret = {0};

    obj_size -= parent_radius;
    if (total_size == obj_size) {
        return ret;
    }
    if (total_size > obj_size) {
        ret.offset = (obj_size - total_size) >> 1;
        return ret;
    }
    if (GT_LAYOUT_JUSTIFY_CONTENT_SPACE_BETWEEN == jc && gap_count > 1) {
        ret.gap = (obj_size - total_size) / (gap_count - 1);
        return ret;
    } else if (GT_LAYOUT_JUSTIFY_CONTENT_SPACE_EVENLY == jc) {
        ret.gap = (obj_size - total_size) / (gap_count + 1);
        ret.offset = ret.gap;
        return ret;
    }
    ret.gap = (obj_size - total_size) / gap_count;
    ret.offset = ret.gap >> 1;
    return ret;
}

static GT_ATTRIBUTE_RAM_TEXT _content_space_around_st _calc_space_mode_gap_and_offset(
    gt_obj_st * parent, bool is_row_dir, gt_layout_justify_content_t jc) {
    uint16_t total_size = 0;

    if (is_row_dir) {
        for (gt_size_t i = 0; i < parent->cnt_child; ++i) {
            total_size += parent->child[i]->area.w;
        }
        return _common_calc_space_gap_and_offset(parent->area.w, parent->cnt_child, parent->radius, total_size, jc);
    }
    /** column dir */
    for (gt_size_t i = 0; i < parent->cnt_child; ++i) {
        total_size += parent->child[i]->area.h;
    }
    return _common_calc_space_gap_and_offset(parent->area.h, parent->cnt_child, parent->radius, total_size, jc);
}

static GT_ATTRIBUTE_RAM_TEXT void _sort_order_by_grow_dir(gt_obj_st * parent, gt_area_st * pos, uint8_t is_row) {
    _update_align_cb_st cb_st = _get_update_align_pos_cb(is_row);
    gt_obj_st * child = NULL;
    gt_size_t i = 0, cnt = parent->cnt_child;
    gt_size_t second_dir_height = is_row ? parent->area.h : parent->area.w;
    gt_layout_flex_direction_t dir = parent->container.flex_direction;
    gt_layout_align_items_t align = parent->container.align_items;

    if (GT_LAYOUT_FLEX_DIR_ROW == dir || GT_LAYOUT_FLEX_DIR_COLUMN == dir) {
        for (i = 0; i < cnt; ++i) {
            child = parent->child[i];
            /** align second dir */
            cb_st.update_align_pos_p(child, pos, second_dir_height, align);
            cb_st.update_max_size_p(child, pos, parent->container.gap);
        }
    } else {
        for (i = cnt - 1; i >= 0; --i) {
            child = parent->child[i];
            /** align second dir */
            cb_st.update_align_pos_p(child, pos, second_dir_height, align);
            cb_st.update_max_size_p(child, pos, parent->container.gap);
        }
    }
}

static GT_ATTRIBUTE_RAM_TEXT gt_area_st _childs_sort_by_flex_direction(gt_obj_st * parent) {
    gt_area_st pos = {
        .x = parent->area.x + (parent->radius >> 1),
        .y = parent->area.y + (parent->radius >> 1),
        .w = 0, .h = 0
    };  /** w/h temp to calc offset */
    gt_layout_justify_content_t jc = parent->container.justify_content;
    uint8_t is_row = _is_flex_row_dir(parent->container.flex_direction);

    if (_is_justify_content_space_mode(jc)) {
        /** pre calc real total width or height value */
        _content_space_around_st sa_ret = _calc_space_mode_gap_and_offset(parent, is_row, jc);
        parent->container.gap = sa_ret.gap;
        if (is_row) {
            pos.w += sa_ret.offset;
        } else {
            pos.h += sa_ret.offset;
        }
    } else if (GT_LAYOUT_ALIGN_ITEMS_END == parent->container.align_items) {
        if (is_row) {
            pos.y = parent->area.y - (parent->radius >> 1);
        } else {
            pos.x = parent->area.x - (parent->radius >> 1);
        }
    }
    _sort_order_by_grow_dir(parent, &pos, is_row);

    if (false == _is_justify_content_space_mode(jc)) {
        /** Expand parent disp area size */
        if (is_row) {
            if (pos.w > parent->area.w) {
                if (GT_LAYOUT_JUSTIFY_CONTENT_END == jc) {
                    parent->area.x = parent->area.x + parent->area.w - pos.w;
                } else if (GT_LAYOUT_JUSTIFY_CONTENT_CENTER == jc) {
                    parent->area.x = parent->area.x + ((parent->area.w - pos.w) >> 1);
                }
                /** Main axis dir to grow size */
                parent->area.w = pos.w;
            }
            if (GT_LAYOUT_SHRINK_DISABLE == parent->container.shrink && pos.h > parent->area.h) {
                /** Second axis dir to self-adjust size */
                parent->area.h = pos.h;
            }
        } else {
            if (pos.h > parent->area.h) {
                if (GT_LAYOUT_JUSTIFY_CONTENT_END == jc) {
                    parent->area.y = parent->area.y + parent->area.h - pos.h;
                } else if (GT_LAYOUT_JUSTIFY_CONTENT_CENTER == jc) {
                    parent->area.y = parent->area.y + ((parent->area.h - pos.h) >> 1);
                }
                /** Main axis dir to grow size */
                parent->area.h = pos.h;
            }
            if (GT_LAYOUT_SHRINK_DISABLE == parent->container.shrink && pos.w > parent->area.w) {
                /** Second axis dir to self-adjust size */
                parent->area.w = pos.w;
            }
        }
        if (GT_LAYOUT_JUSTIFY_CONTENT_END == jc) {
            if (is_row) {
                pos.x += (parent->radius >> 1);
            } else {
                pos.y += (parent->radius >> 1);
            }
        }
    }
    return pos;
}

/**
 * @brief Handle END and CENTER justify content
 *
 * @param parent
 * @param content_pos Overall position of all elements
 * @return gt_area_st
 */
static GT_ATTRIBUTE_RAM_TEXT gt_area_st _childs_adjust_justify_content(gt_obj_st * parent, gt_area_st * content_pos) {
    gt_layout_justify_content_t jc = parent->container.justify_content;
    gt_point_st diff = {0};

    if (GT_LAYOUT_JUSTIFY_CONTENT_END != jc && GT_LAYOUT_JUSTIFY_CONTENT_CENTER != jc) {
        return *content_pos;
    }

    // justify content end pos
    if (_is_flex_row_dir(parent->container.flex_direction)) {
        diff.x = parent->area.x + parent->area.w - content_pos->w - content_pos->x;
    } else {
        diff.y = parent->area.y + parent->area.h - content_pos->h - content_pos->y;
    }
    if (GT_LAYOUT_JUSTIFY_CONTENT_CENTER == jc) {
        if (parent->area.w > content_pos->w) { diff.x >>= 1; }
        if (parent->area.h > content_pos->h) { diff.y >>= 1; }
    }
    _gt_obj_move_child_by(parent, diff.x, diff.y);

    return *content_pos;
}

static GT_ATTRIBUTE_RAM_TEXT void _adjust_flex_content(gt_obj_st * target) {
    if (false == gt_layout_is_type(target, GT_LAYOUT_TYPE_FLEX)) {
        return ;
    }
    if (0 == target->cnt_child) {
        return ;
    }
    gt_area_st pos = _childs_sort_by_flex_direction(target);
    _childs_adjust_justify_content(target, &pos);
}
#endif

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

#if GT_USE_WIDGET_LAYOUT
void gt_layout_init(gt_obj_st * obj, gt_obj_container_st const * const container)
{
    GT_CHECK_BACK(obj);
    GT_CHECK_BACK(container);

    obj->container = *container;
    if (0 == obj->cnt_child) {
        return;
    }
    gt_layout_update_core(obj);
}

gt_res_t gt_layout_update_core(gt_obj_st * obj)
{
    GT_CHECK_BACK_VAL(obj, GT_RES_FAIL);

    gt_layout_type_t type = obj->container.layout_type;
    if (obj->parent && obj->parent->cnt_child) {
        if (GT_LAYOUT_TYPE_FIXED == type &&
            GT_LAYOUT_TYPE_FIXED == obj->parent->container.layout_type) {
            return GT_RES_INV;
        }
    } else if (GT_LAYOUT_TYPE_FIXED == type) {
        return GT_RES_INV;
    }

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
    if (NULL == obj) { return false; }
    if (type >= _GT_LAYOUT_TYPE_TOTAL) {
        return false;
    }
    return (type == obj->container.layout_type) ? true : false;
}
#endif

/* end ------------------------------------------------------------------*/
