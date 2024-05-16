/**
 * @file gt_view_pager.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2023-10-07 14:39:17
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_view_pager.h"

#if GT_CFG_ENABLE_VIEW_PAGER
#include "../core/gt_mem.h"
#include "../others/gt_types.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../others/gt_area.h"
#include "../others/gt_log.h"
#include "../core/gt_obj_pos.h"
#include "../core/gt_obj_scroll.h"


/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_VIEW_PAGER
#define MY_CLASS    &_gt_view_pager_class

/** Stores the bit-width of the maximum number of pages */
#define _VIEW_PAGER_COUNT_BIT_WIDTH     4

/** The maximum number of pages to store */
#define _MAX_VIEW_PAGER_COUNT           (1 << _VIEW_PAGER_COUNT_BIT_WIDTH)

/** tip area height */
#define _TIP_BAR_HEIGHT                 24
#define _TIP_BAR_POINT_HEIGHT           8

/**
 * effect
 * (+++)( + )( + )
 * ( + )(+++)( + )
 * ( + )( + )(+++)
 */

/** The unselect point */
#define _TIP_BAR_POINT_UNSELECT_WIDTH       _TIP_BAR_POINT_HEIGHT
#define _TIP_BAR_POINT_UNSELECT_OFFSET_X    (_TIP_BAR_POINT_UNSELECT_WIDTH >> 1)
#define _TIP_BAR_POINT_SELECT_WIDTH         (_TIP_BAR_POINT_UNSELECT_WIDTH << 1)

#define _TIP_BAR_OFFSET_Y               (_TIP_BAR_HEIGHT - _TIP_BAR_POINT_HEIGHT)

/* private typedef ------------------------------------------------------*/
struct _reg_s {
    uint16_t count : _VIEW_PAGER_COUNT_BIT_WIDTH;
    uint16_t index : _VIEW_PAGER_COUNT_BIT_WIDTH;
    uint16_t glass : 1;     /** display background glass */
    uint16_t reserved : 7;
};

typedef struct _gt_view_pager_s {
    gt_obj_st obj;
    struct _reg_s reg;
    uint16_t width;
    gt_color_t color_glass;
}_gt_view_pager_st;



/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st _gt_view_pager_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_view_pager_st)
};


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void _draw_tip_bar(gt_obj_st * obj, gt_attr_rect_st * rect_attr) {
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj;
    uint8_t count = style->reg.count;
    uint16_t total_width = count * _TIP_BAR_POINT_SELECT_WIDTH;
    uint8_t i = 0;
    gt_area_st area_tip = obj->area;

    area_tip.y += area_tip.h - _TIP_BAR_OFFSET_Y;
    area_tip.h = _TIP_BAR_POINT_HEIGHT;

    rect_attr->bg_opa = GT_OPA_50;

    area_tip.x = ((style->width - total_width) >> 1) + _TIP_BAR_POINT_UNSELECT_OFFSET_X;
    for (i = 0; i < count; i++) {
        area_tip.w = _TIP_BAR_POINT_HEIGHT;
        draw_bg(obj->draw_ctx, rect_attr, &area_tip);
        area_tip.x += _TIP_BAR_POINT_SELECT_WIDTH;
    }

    area_tip.x = ((style->width - total_width) >> 1);
    if (style->width) {
        area_tip.x += -obj->process_attr.scroll.x * _TIP_BAR_POINT_SELECT_WIDTH / style->width;
    }
    area_tip.w = _TIP_BAR_POINT_HEIGHT << 1;
    rect_attr->bg_opa = GT_OPA_100;
    draw_bg(obj->draw_ctx, rect_attr, &area_tip);
}

static void _init_cb(gt_obj_st * obj) {
    gt_attr_rect_st rect_attr;
    gt_area_st area_bg = obj->area;
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj;
    uint16_t i = 0, width = area_bg.w / style->reg.count;
    uint16_t space = (width << 1) / 100;    /** 2% of screen width */

    area_bg.x += (space >> 1);
    area_bg.w = width - space;
    area_bg.h -= _TIP_BAR_HEIGHT;

    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill   = true;
    rect_attr.radius        = 10;
    rect_attr.bg_opa        = obj->opa;
    rect_attr.border_width  = 0;
    rect_attr.bg_color      = style->color_glass;
    if (style->reg.glass) {
        for (i = 0; i < style->reg.count; i++) {
            draw_bg(obj->draw_ctx, &rect_attr, &area_bg);
            area_bg.x += width;
        }
    }

    _draw_tip_bar(obj, &rect_attr);
}

/**
 * @brief A filter which can be slide object child type
 *
 * @param obj
 * @return true Target object can slide or disable view pager scroll
 * @return false
 */
static inline bool _ignore_view_pager_slider_control(gt_obj_st * obj) {
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_SLIDER == type) {
        return true;
    }
    else if (GT_TYPE_ROLLER == type) {
        return true;
    }
    else if (GT_TYPE_LISTVIEW == type) {
        return true;
    }
    else if (GT_TYPE_KEYPAD == type) {
        return true;
    }
    return false;
}

/**
 * @brief Set child unfixed property which can be slide itself or disable view pager scroll
 *
 * @param parent The target children
 * @param is_root root must be view pager widget
 * @returns true: childs has slider control
 *          false: childs has no slider control
 */
static bool _unfixed_slider_widgets(gt_obj_st * self, bool is_root) {
    gt_obj_st * child = NULL;
    uint8_t is_slider = false, is_cur_slider = false;

    if (false == is_root && 0 == self->cnt_child) {
        is_slider = _ignore_view_pager_slider_control(self);
        if (is_slider && gt_obj_get_fixed(self)) {
            gt_obj_set_fixed(self, false);
        }
        return is_slider;
    }

    for (gt_size_t i = 0, cnt = self->cnt_child; i < cnt; i++) {
        child = self->child[i];
        is_slider = _unfixed_slider_widgets(child, false);
        if (is_slider) {
            is_cur_slider = true;
        }
    }
    if ((is_cur_slider && false == is_root) || _ignore_view_pager_slider_control(self)) {
        /** Child controls can either slide or slide themselves */
        if (gt_obj_get_fixed(self)) {
            gt_obj_set_fixed(self, false);
        }
    }
    return is_cur_slider;
}

static inline void _init_child_prop(gt_obj_st * obj) {
    gt_obj_child_set_prop(obj, GT_OBJ_PROP_TYPE_FIXED, true);
    gt_obj_child_set_prop(obj, GT_OBJ_PROP_TYPE_INSIDE, true);

    _unfixed_slider_widgets(obj, true);
}

static void _deinit_cb(gt_obj_st * obj) {

}

static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj;
    gt_size_t value = 0;

    switch (e->code) {
    case GT_EVENT_TYPE_DRAW_START: {
        gt_disp_invalid_area(obj);
        break;
    }
    case GT_EVENT_TYPE_CHANGE_CHILD_ADD: {
        // NOTE change to last child when be reset prop
        _init_child_prop(obj);
        break;
    }
    case GT_EVENT_TYPE_INPUT_SCROLL: {
        gt_obj_set_pos(obj, obj->process_attr.scroll.x, obj->area.y);
        break;
    }
    case GT_EVENT_TYPE_INPUT_SCROLL_END: {
        style->reg.index = gt_abs(obj->process_attr.scroll.x) / style->width;
        break;
    }
    case GT_EVENT_TYPE_INPUT_SCROLL_UP:
    case GT_EVENT_TYPE_INPUT_SCROLL_LEFT: {
        value = gt_abs(obj->process_attr.scroll.x) / style->width;
        if (value) {
            gt_obj_scroll_to_x(obj, gt_abs(obj->process_attr.scroll.x) - (value - 1) * style->width, GT_ANIM_ON);
        }
        break;
    }
    case GT_EVENT_TYPE_INPUT_SCROLL_DOWN:
    case GT_EVENT_TYPE_INPUT_SCROLL_RIGHT: {
        value = (gt_abs(obj->process_attr.scroll.x) + style->width - 1) / style->width;
        if ((value + 1) < style->reg.count) {
            gt_obj_scroll_to_x(obj, gt_abs(obj->process_attr.scroll.x) - ((value + 1) * style->width), GT_ANIM_ON);
        }
        break;
    }
    default:
        break;
    }
}

/**
 * @brief Is exceeded the maximum number of pages
 *
 * @param val
 */
static inline bool _is_over_max_page(uint8_t val) {
    return ((val + 1) < _MAX_VIEW_PAGER_COUNT) ? false : true;
}

static bool _child_is_group(gt_obj_st * obj) {
    return (GT_TYPE_GROUP == obj->class->type) ? true : false;
}

static void _child_set_group_area(gt_obj_st const * const view_pager, gt_obj_st * group) {
    gt_area_copy(&group->area, &view_pager->area);
}

/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_view_pager_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj;
    style->width = gt_disp_get_res_hor(NULL);
    obj->area.x = 0;
    obj->area.y = 0;
    obj->area.w = style->width;
    obj->area.h = gt_disp_get_res_ver(NULL);
    obj->opa = GT_OPA_30;
    obj->absorb = true;
    obj->fixed = false;

    style->reg.glass = true;
    style->color_glass = gt_color_make(0xff, 0xff, 0xff);

    gt_obj_set_overflow(obj, true);
    gt_obj_set_scroll_dir(obj, GT_SCROLL_HORIZONTAL);
    gt_obj_set_scroll_snap_x(obj, GT_SCROLL_SNAP_START);

    /** tip point can not overflow */
    return obj;
}

int8_t gt_view_pager_set_fragment_count(gt_obj_st * obj, uint8_t count)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return 0;
    }
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj;
    if (_is_over_max_page(count)) {
        count = _MAX_VIEW_PAGER_COUNT;
    }
    style->reg.count = count;
    obj->area.w = gt_disp_get_res_hor(NULL) * count;

    return style->reg.count;
}

int8_t gt_view_pager_add_fragment(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return 0;
    }
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj;
    if (_is_over_max_page(style->reg.count)) {
        return -1;
    }
    ++style->reg.count;
    obj->area.w = gt_disp_get_res_hor(NULL) * style->reg.count;

    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);

    return style->reg.count;
}

void gt_view_pager_fragment_add_widget(gt_obj_st * view_pager, uint8_t fragment_idx, gt_obj_st * child)
{
    if (false == gt_obj_is_type(view_pager, OBJ_TYPE)) {
        return;
    }
    if (_is_over_max_page(fragment_idx)) {
        return;
    }
    if (false == gt_obj_is_child(child, view_pager)) {
        gt_obj_st * parent = gt_obj_change_parent(child, view_pager);
        _gt_obj_class_inherent_attr_from_parent(child, parent);
        _init_child_prop(parent);
    }
    if (_child_is_group(child)) {
        _child_set_group_area(view_pager, child);
    }
    uint16_t scr_width = gt_disp_get_res_hor(NULL);
    uint16_t new_pos = (child->area.x % scr_width) + fragment_idx * scr_width;

    gt_obj_set_pos(child, new_pos, child->area.y);
}

void gt_view_pager_set_glass(gt_obj_st * obj, bool enabled)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj;
    style->reg.glass = enabled;
}

bool gt_view_pager_get_glass(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return false;
    }
    return ((_gt_view_pager_st * )obj)->reg.glass;
}

void gt_view_pager_set_glass_color(gt_obj_st * obj, gt_color_t color)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj;
    style->color_glass = color;
}

gt_color_t gt_view_pager_get_glass_color(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return gt_color_black();
    }
    return ((_gt_view_pager_st * )obj)->color_glass;
}

/* end ------------------------------------------------------------------*/
#endif /** GT_CFG_ENABLE_VIEW_PAGER */
