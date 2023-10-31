/**
 * @file gt_view_pager.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2023-10-07 14:39:17
 * @copyright Copyright (c) 2014-2023, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_view_pager.h"
#include "../core/gt_mem.h"
#include "../others/gt_types.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../others/gt_area.h"
#include "../others/gt_log.h"


#if GT_CFG_ENABLE_VIEW_PAGER
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
    struct _reg_s reg;
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
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj->style;
    uint8_t idx = style->reg.index;
    uint8_t count = style->reg.count;
    uint16_t total_width = count * _TIP_BAR_POINT_SELECT_WIDTH;
    uint8_t i = 0;
    gt_area_st area_tip = obj->area;

    area_tip.y += area_tip.h - _TIP_BAR_OFFSET_Y;
    area_tip.h = _TIP_BAR_POINT_HEIGHT;

    rect_attr->bg_opa = GT_OPA_50;

    area_tip.x = ((obj->parent->area.w - total_width) >> 1) + _TIP_BAR_POINT_UNSELECT_OFFSET_X;
    for (i = 0; i < count; i++) {
        area_tip.w = _TIP_BAR_POINT_HEIGHT;
        draw_bg(obj->draw_ctx, rect_attr, &area_tip);
        area_tip.x += _TIP_BAR_POINT_SELECT_WIDTH;
    }

    area_tip.x = ((obj->parent->area.w - total_width) >> 1);
    area_tip.x += -obj->process_attr.scroll.x * _TIP_BAR_POINT_SELECT_WIDTH / obj->parent->area.w;
    area_tip.w = _TIP_BAR_POINT_HEIGHT << 1;
    rect_attr->bg_opa = GT_OPA_100;
    draw_bg(obj->draw_ctx, rect_attr, &area_tip);
}

static void _init_cb(gt_obj_st * obj) {
    gt_attr_rect_st rect_attr;
    gt_area_st area_bg = obj->area;
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj->style;
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

static void _deinit_cb(gt_obj_st * obj) {

}

static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj->style;

    switch (e->code) {
    case GT_EVENT_TYPE_DRAW_START: {
        gt_disp_invalid_area(obj);
        break;
    }
    case GT_EVENT_TYPE_DRAW_END: {
        break;
    }
    case GT_EVENT_TYPE_CHANGE_CHILD_ADD: {
        gt_obj_child_set_prop(obj, GT_OBJ_PROP_TYPE_FIXED, true);
        break;
    }
    case GT_EVENT_TYPE_INPUT_SCROLL_START: {
        break;
    }
    case GT_EVENT_TYPE_INPUT_SCROLL: {
        gt_obj_move_to(obj, obj->process_attr.scroll.x, obj->area.y);
        break;
    }
    case GT_EVENT_TYPE_INPUT_SCROLL_END: {
        style->reg.index = gt_abs(obj->process_attr.scroll.x) / obj->parent->area.w;
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

static void _update_max_width(gt_obj_st * obj, uint8_t fragment_count) {
    obj->area.w = gt_disp_get_res_hor(NULL) * fragment_count;
}

/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_view_pager_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj->style;
    gt_memset(style, 0, sizeof(_gt_view_pager_st));
    obj->area.x = 0;
    obj->area.y = 0;
    obj->area.w = gt_disp_get_res_hor(NULL);
    obj->area.h = gt_disp_get_res_ver(NULL);
    obj->opa = GT_OPA_30;
    obj->absorb = true;

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
    if (NULL == obj) {
        return 0;
    }
    if (OBJ_TYPE != obj->class->type) {
        return 0;
    }
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj->style;
    if (_is_over_max_page(count)) {
        count = _MAX_VIEW_PAGER_COUNT;
    }
    style->reg.count = count;
    obj->area.w = gt_disp_get_res_hor(NULL) * count;

    return style->reg.count;
}

int8_t gt_view_pager_add_fragment(gt_obj_st * obj)
{
    if (NULL == obj) {
        return 0;
    }
    if (OBJ_TYPE != obj->class->type) {
        return 0;
    }
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj->style;
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
    if (NULL == view_pager) {
        return;
    }
    if (OBJ_TYPE != view_pager->class->type) {
        return;
    }
    _gt_view_pager_st * style = (_gt_view_pager_st * )view_pager->style;
    if (_is_over_max_page(fragment_idx)) {
        return;
    }
    if (false == gt_obj_is_child(child, view_pager)) {
        gt_obj_change_parent(child, view_pager);
        gt_obj_set_overflow(child, true);
        gt_obj_child_set_prop(child, GT_OBJ_PROP_TYPE_FIXED, true);
    }
    uint16_t scr_width = gt_disp_get_res_hor(NULL);

    child->area.x = (child->area.x % scr_width) + fragment_idx * scr_width;
}

void gt_view_pager_set_glass(gt_obj_st * obj, bool enabled)
{
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj->style;
    style->reg.glass = enabled;
}

bool gt_view_pager_get_glass(gt_obj_st * obj)
{
    return ((_gt_view_pager_st * )obj->style)->reg.glass;
}

void gt_view_pager_set_glass_color(gt_obj_st * obj, gt_color_t color)
{
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj->style;
    style->color_glass = color;
}

gt_color_t gt_view_pager_get_glass_color(gt_obj_st * obj)
{
    return ((_gt_view_pager_st * )obj->style)->color_glass;
}

/* end ------------------------------------------------------------------*/
#endif /** GT_CFG_ENABLE_VIEW_PAGER */