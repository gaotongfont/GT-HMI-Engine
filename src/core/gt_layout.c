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
#include "../hal/gt_hal_disp.h"
#include "./gt_disp.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/
typedef struct {
    bool (* update_align_pos_p)(gt_obj_st *, gt_area_st *, gt_size_t, gt_layout_align_items_t);
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
    return (GT_LAYOUT_FLEX_DIR_ROW == dir || GT_LAYOUT_FLEX_DIR_ROW_REVERSE == dir);
}

static GT_ATTRIBUTE_RAM_TEXT bool _update_flex_align_pos_row(gt_obj_st * child, gt_area_st * pos, gt_size_t second_dir_height, gt_layout_align_items_t align) {
    gt_size_t nx, ny;
    if (GT_LAYOUT_ALIGN_ITEMS_START == align) {
        nx = pos->x + pos->w; ny = pos->y;
    } else if (GT_LAYOUT_ALIGN_ITEMS_END == align) {
        nx = pos->x + pos->w; ny = pos->y + second_dir_height - child->area.h;
    } else {
        nx = pos->x + pos->w; ny = ((second_dir_height - child->area.h) >> 1) + pos->y;
    }
    if (nx == child->area.x && ny == child->area.y) { return false; }
    gt_obj_set_pos(child, nx, ny);
    return true;
}

static GT_ATTRIBUTE_RAM_TEXT bool _update_flex_align_pos_column(gt_obj_st * child, gt_area_st * pos, gt_size_t second_dir_height, gt_layout_align_items_t align) {
    gt_size_t nx, ny;
    if (GT_LAYOUT_ALIGN_ITEMS_START == align) {
        nx = pos->x; ny = pos->y + pos->h;
    } else if (GT_LAYOUT_ALIGN_ITEMS_END == align) {
        nx = pos->x + second_dir_height - child->area.w; ny = pos->y + pos->h;
    } else {
        nx = ((second_dir_height - child->area.w) >> 1) + pos->x; ny = pos->y + pos->h;
    }
    if (nx == child->area.x && ny == child->area.y) { return false; }
    gt_obj_set_pos(child, nx, ny);
    return true;
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

static GT_ATTRIBUTE_RAM_TEXT inline bool _is_justify_content_space_mode(gt_layout_justify_content_t jc) {
    return (GT_LAYOUT_JUSTIFY_CONTENT_SPACE_AROUND == jc ||
            GT_LAYOUT_JUSTIFY_CONTENT_SPACE_BETWEEN == jc ||
            GT_LAYOUT_JUSTIFY_CONTENT_SPACE_EVENLY == jc);
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

static GT_ATTRIBUTE_RAM_TEXT bool _sort_order_by_grow_dir(gt_obj_st * parent, gt_area_st * pos, bool is_row) {
    _update_align_cb_st cb_st = _get_update_align_pos_cb(is_row);
    gt_size_t cnt = parent->cnt_child;
    gt_size_t second_dir_height = is_row ? parent->area.h : parent->area.w;
    gt_layout_flex_direction_t dir = parent->container.flex_direction;
    gt_layout_align_items_t align = parent->container.align_items;
    gt_size_t gap = parent->container.gap;
    gt_obj_st * child = NULL;
    gt_size_t i;
    bool changed = false;

    if (GT_LAYOUT_FLEX_DIR_ROW == dir || GT_LAYOUT_FLEX_DIR_COLUMN == dir) {
        for (i = 0; i < cnt; ++i) {
            child = parent->child[i];
            if (cb_st.update_align_pos_p(child, pos, second_dir_height, align)) { changed = true; }
            cb_st.update_max_size_p(child, pos, gap);
        }
    } else {
        for (i = cnt - 1; i >= 0; --i) {
            child = parent->child[i];
            if (cb_st.update_align_pos_p(child, pos, second_dir_height, align)) { changed = true; }
            cb_st.update_max_size_p(child, pos, gap);
        }
    }
    return changed;
}

static GT_ATTRIBUTE_RAM_TEXT gt_area_st _childs_sort_by_flex_direction(gt_obj_st * parent, bool * changed_out) {
    gt_size_t half_radius = parent->radius >> 1;
    gt_area_st pos = {
        .x = parent->area.x + half_radius,
        .y = parent->area.y + half_radius,
        .w = 0, .h = 0
    };  /** w/h temp to calc offset */
    gt_layout_justify_content_t jc = parent->container.justify_content;
    bool is_row = _is_flex_row_dir(parent->container.flex_direction);

    if (_is_justify_content_space_mode(jc)) {
        _content_space_around_st sa_ret = _calc_space_mode_gap_and_offset(parent, is_row, jc);
        parent->container.gap = sa_ret.gap;
        if (is_row) {
            pos.w += sa_ret.offset;
        } else {
            pos.h += sa_ret.offset;
        }
    } else if (GT_LAYOUT_ALIGN_ITEMS_END == parent->container.align_items) {
        if (is_row) {
            pos.y = parent->area.y - half_radius;
        } else {
            pos.x = parent->area.x - half_radius;
        }
    }
    bool changed = _sort_order_by_grow_dir(parent, &pos, is_row);
    if (changed_out) { *changed_out = changed; }
    if (!_is_justify_content_space_mode(jc)) {
        if (is_row) {
            if (pos.w > parent->area.w) {
                if (GT_LAYOUT_JUSTIFY_CONTENT_END == jc) {
                    parent->area.x += parent->area.w - pos.w;
                } else if (GT_LAYOUT_JUSTIFY_CONTENT_CENTER == jc) {
                    parent->area.x += (parent->area.w - pos.w) >> 1;
                }
                parent->area.w = pos.w;
            }
            if (GT_LAYOUT_SHRINK_DISABLE == parent->container.shrink && pos.h > parent->area.h) {
                parent->area.h = pos.h;
            }
        } else {
            if (pos.h > parent->area.h) {
                if (GT_LAYOUT_JUSTIFY_CONTENT_END == jc) {
                    parent->area.y += parent->area.h - pos.h;
                } else if (GT_LAYOUT_JUSTIFY_CONTENT_CENTER == jc) {
                    parent->area.y += (parent->area.h - pos.h) >> 1;
                }
                parent->area.h = pos.h;
            }
            if (GT_LAYOUT_SHRINK_DISABLE == parent->container.shrink && pos.w > parent->area.w) {
                parent->area.w = pos.w;
            }
        }
        if (GT_LAYOUT_JUSTIFY_CONTENT_END == jc) {
            if (is_row) {
                pos.x += half_radius;
            } else {
                pos.y += half_radius;
            }
        }
    }
    return pos;
}

/**
 * @brief 处理 END 和 CENTER 主轴对齐偏移
 *
 * @param parent 父对象
 * @param content_pos 所有子元素的整体位置
 */
static bool GT_ATTRIBUTE_RAM_TEXT
_childs_adjust_justify_content(gt_obj_st * parent, gt_area_st * content_pos) {
    gt_layout_justify_content_t jc = parent->container.justify_content;

    if (GT_LAYOUT_JUSTIFY_CONTENT_END != jc && GT_LAYOUT_JUSTIFY_CONTENT_CENTER != jc) {
        return false;
    }

    gt_point_st diff = {0};
    if (_is_flex_row_dir(parent->container.flex_direction)) {
        diff.x = parent->area.x + parent->area.w - content_pos->w - content_pos->x;
    } else {
        diff.y = parent->area.y + parent->area.h - content_pos->h - content_pos->y;
    }
    if (GT_LAYOUT_JUSTIFY_CONTENT_CENTER == jc) {
        if (parent->area.w > content_pos->w) { diff.x >>= 1; }
        if (parent->area.h > content_pos->h) { diff.y >>= 1; }
    }
    if (0 == diff.x && 0 == diff.y) { return false; }
    _gt_obj_move_child_by(parent, diff.x, diff.y);
    return true;
}

static GT_ATTRIBUTE_RAM_TEXT void _adjust_flex_content(gt_obj_st * target) {
    if (!gt_layout_is_type(target, GT_LAYOUT_TYPE_FLEX)) {
        return;
    }
    if (0 == target->cnt_child) {
        return;
    }
    _gt_disp_refr_suspend();
    bool changed = false;
    gt_area_st pos = _childs_sort_by_flex_direction(target, &changed);
    if (_childs_adjust_justify_content(target, &pos)) { changed = true; }
    _gt_disp_refr_resume();
    if (changed) {
        _gt_disp_refr_append_area(&target->area);
    }
}
#endif

/* global functions / API interface -------------------------------------*/

void gt_layout_row_grow(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
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
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return;
    }
    GT_CHECK_BACK(container);

    obj->container = *container;
    if (0 == obj->cnt_child) {
        return;
    }
    gt_layout_update_core(obj);
}

gt_res_t gt_layout_update_core(gt_obj_st * obj)
{
    if (NULL == obj) { return GT_RES_FAIL; }

    /* re-entry guard: if layout batch is in progress, defer to flush_dirty */
    gt_disp_st * disp = gt_disp_get_default();
    if (disp && disp->refr.suspended) {
        gt_layout_mark_dirty(obj);
        if (obj->parent) { gt_layout_mark_dirty(obj->parent); }
        return GT_RES_OK;
    }

    if (GT_LAYOUT_TYPE_FIXED == obj->container.layout_type) {
        if (NULL == obj->parent || GT_LAYOUT_TYPE_FIXED == obj->parent->container.layout_type) {
            return GT_RES_INV;
        }
    }

    if (obj->classes && obj->classes->_init_cb) {
        struct _gt_draw_ctx_s tmp_draw_ctx = {
            .parent_area = obj->inside ? &obj->parent->area : NULL,
            .buf_area = obj->area,
        };
        obj->draw_ctx = &tmp_draw_ctx;
        obj->classes->_init_cb(obj);
        obj->draw_ctx = NULL;
    }

    _adjust_flex_content(obj);
    if (obj->parent) {
        _adjust_flex_content(obj->parent);
    }
    return GT_RES_OK;
}

enum gt_layout_type_e gt_layout_get_type(gt_obj_st * obj)
{
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return GT_LAYOUT_TYPE_FIXED;
    }
    return obj->container.layout_type;
}

bool gt_layout_is_type(gt_obj_st * obj, enum gt_layout_type_e type)
{
    if (NULL == obj) { return false; }
    if (type >= _GT_LAYOUT_TYPE_TOTAL) {
        return false;
    }
    return (type == obj->container.layout_type);
}

static GT_ATTRIBUTE_RAM_TEXT void _flush_dirty_recursive(gt_obj_st * obj) {
    if (NULL == obj) { return; }
    if (obj->layout_dirty) {
        obj->layout_dirty = 0;      /* clear before calc, prevent re-entry */
        _adjust_flex_content(obj);
    }
    for (uint16_t i = 0; i < obj->cnt_child; i++) {
        _flush_dirty_recursive(obj->child[i]);
    }
}

void gt_layout_mark_dirty(gt_obj_st * obj)
{
    if (NULL == obj) { return; }
    if (!gt_layout_is_type(obj, GT_LAYOUT_TYPE_FLEX)) { return; }
    obj->layout_dirty = 1;
}

void gt_layout_flush_dirty(void)
{
    gt_disp_st * disp = gt_disp_get_default();
    if (NULL == disp || NULL == disp->scr_act) { return; }
    _flush_dirty_recursive(disp->scr_act);
}

void gt_layout_set_type(gt_obj_st * obj, enum gt_layout_type_e type)
{
    if (NULL == obj) { return; }
    if (GT_TYPE_UNKNOWN == gt_obj_class_get_type(obj)) { return; }
    if ((gt_layout_type_t)type == obj->container.layout_type) { return; }

    if (GT_LAYOUT_TYPE_FLEX == type) {
        /* FIXED → FLEX：保存子元素相对坐标 */
        for (uint16_t i = 0; i < obj->cnt_child; i++) {
            gt_obj_st * child = obj->child[i];
            child->fixed_pos.x = child->area.x - obj->area.x;
            child->fixed_pos.y = child->area.y - obj->area.y;
            child->fixed_pos_saved = 1;
        }
        obj->container.layout_type = GT_LAYOUT_TYPE_FLEX;
        _adjust_flex_content(obj);
    } else {
        /* FLEX → FIXED：先改类型，再恢复坐标（避免 mark_dirty 再触发 flex） */
        obj->container.layout_type = GT_LAYOUT_TYPE_FIXED;
        _gt_disp_refr_suspend();
        for (uint16_t i = 0; i < obj->cnt_child; i++) {
            gt_obj_st * child = obj->child[i];
            if (child->fixed_pos_saved) {
                gt_obj_set_pos(child,
                    obj->area.x + child->fixed_pos.x,
                    obj->area.y + child->fixed_pos.y);
                child->fixed_pos_saved = 0;
            }
        }
        _gt_disp_refr_resume();
        _gt_disp_refr_append_area(&obj->area);
    }
}
#endif

/* end ------------------------------------------------------------------*/
