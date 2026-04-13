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
#include "../others/gt_anim.h"
#include "../core/gt_style.h"


/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_VIEW_PAGER
#define MY_CLASS    &_gt_view_pager_class

/** Stores the bit-width of the maximum number of pages */
#define _VIEW_PAGER_COUNT_BIT_WIDTH     7

/** The maximum number of pages to store */
#define _MAX_VIEW_PAGER_COUNT           (1 << _VIEW_PAGER_COUNT_BIT_WIDTH)

/** tip area height */
#ifndef _TIP_BAR_HEIGHT
    /**
     * @brief The height of the tip bar
     */
    #define _TIP_BAR_HEIGHT             24
#endif
#ifndef _TIP_BAR_POINT_HEIGHT
    /**
     * @brief The height of the tip bar point
     */
    #define _TIP_BAR_POINT_HEIGHT       8
#endif

/**
 * effect span: +++ is selected.
 * (+++)( + )( + )
 * ( + )(+++)( + )
 * ( + )( + )(+++)
 */

/** The unselect point span size*/
#define _TIP_BAR_POINT_UNSELECT_WIDTH       _TIP_BAR_POINT_HEIGHT
#define _TIP_BAR_POINT_UNSELECT_OFFSET_X    (_TIP_BAR_POINT_UNSELECT_WIDTH >> 1)
#define _TIP_BAR_POINT_PER_ITEM_WIDTH       (_TIP_BAR_POINT_UNSELECT_WIDTH << 1)

#ifndef _TIP_BAR_POINT_SELECT_WIDTH
    /**
     * @brief The width of the selected point width
     */
    #define _TIP_BAR_POINT_SELECT_WIDTH     (_TIP_BAR_POINT_HEIGHT << 1)
#endif

#define _TIP_BAR_OFFSET_Y               (_TIP_BAR_HEIGHT - _TIP_BAR_POINT_HEIGHT)

/* private typedef ------------------------------------------------------*/
struct _reg_s {
    uint16_t count      : _VIEW_PAGER_COUNT_BIT_WIDTH;
    uint16_t index      : _VIEW_PAGER_COUNT_BIT_WIDTH;
    uint16_t glass      : 1;     /** display background glass */
    uint16_t dir        : 1;     /** 0: horizontal, 1: vertical */
    uint16_t tip_pos    : 1;     /** 0: default(bottom/right), 1: opposite(top/left) */
    uint16_t bouncing   : 1;     /** 1: bounce-back animation in progress */
};

typedef struct _gt_view_pager_s {
    gt_obj_st base;
    struct _reg_s reg;
    uint16_t page_w;            /** visible window width (one page width) */
    uint16_t page_h;            /** visible window height (one page height) */
    gt_obj_st * content;        /** inner scrollable content layer */
    gt_color_t color_glass;
}_gt_view_pager_st;



/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st _gt_view_pager_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = (_gt_deinit_cb_t)NULL,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_view_pager_st)
};


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

/** Get the scroll offset from the content layer */
static GT_ATTRIBUTE_RAM_TEXT inline gt_size_t _get_scroll_x(_gt_view_pager_st * style) {
    return style->content ? style->content->process_attr.scroll.x : 0;
}
static GT_ATTRIBUTE_RAM_TEXT inline gt_size_t _get_scroll_y(_gt_view_pager_st * style) {
    return style->content ? style->content->process_attr.scroll.y : 0;
}

static GT_ATTRIBUTE_RAM_TEXT void _draw_tip_bar(gt_obj_st * obj, gt_attr_rect_st * rect_attr) {
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj;
    uint8_t count = style->reg.count;
    uint16_t total_width = count * _TIP_BAR_POINT_PER_ITEM_WIDTH;
    uint8_t i = 0;
    gt_area_st area_tip = obj->area;
    bool is_ver = (bool)style->reg.dir;

    rect_attr->bg_opa = GT_OPA_50;

    if (!is_ver) {
        if (style->reg.tip_pos) {
            /** top: place dots near the top edge */
            area_tip.y += (_TIP_BAR_POINT_HEIGHT >> 1);
        } else {
            /** bottom (default) */
            area_tip.y += area_tip.h - _TIP_BAR_OFFSET_Y;
        }
        area_tip.h = _TIP_BAR_POINT_HEIGHT;
        area_tip.x = obj->area.x + ((style->page_w - total_width) >> 1) + _TIP_BAR_POINT_UNSELECT_OFFSET_X;
        for (i = 0; i < count; i++) {
            area_tip.w = _TIP_BAR_POINT_HEIGHT;
            draw_bg(obj->draw_ctx, rect_attr, &area_tip);
            area_tip.x += _TIP_BAR_POINT_PER_ITEM_WIDTH;
        }
        area_tip.w = _TIP_BAR_POINT_SELECT_WIDTH;
        area_tip.x = obj->area.x + ((style->page_w - total_width) >> 1);
        if (style->page_w) {
            gt_size_t sx = _get_scroll_x(style);
            gt_size_t sx_min = -(gt_size_t)(style->reg.count - 1) * style->page_w;
            if (sx > 0) { sx = 0; }
            if (sx < sx_min) { sx = sx_min; }
            area_tip.x += -(sx * _TIP_BAR_POINT_PER_ITEM_WIDTH / style->page_w) +
                ((_TIP_BAR_POINT_PER_ITEM_WIDTH - area_tip.w) >> 1);
        }
        rect_attr->bg_opa = GT_OPA_100;
        draw_bg(obj->draw_ctx, rect_attr, &area_tip);
    } else {
        uint16_t total_h = count * _TIP_BAR_POINT_PER_ITEM_WIDTH;
        area_tip.w = _TIP_BAR_POINT_HEIGHT;
        if (style->reg.tip_pos) {
            /** left: place dots near the left edge */
            area_tip.x = obj->area.x + (_TIP_BAR_POINT_HEIGHT >> 1);
        } else {
            /** right (default) */
            area_tip.x = obj->area.x + style->page_w - _TIP_BAR_OFFSET_Y;
        }
        area_tip.y = obj->area.y + ((style->page_h - total_h) >> 1) + _TIP_BAR_POINT_UNSELECT_OFFSET_X;
        for (i = 0; i < count; i++) {
            area_tip.h = _TIP_BAR_POINT_HEIGHT;
            draw_bg(obj->draw_ctx, rect_attr, &area_tip);
            area_tip.y += _TIP_BAR_POINT_PER_ITEM_WIDTH;
        }
        area_tip.h = _TIP_BAR_POINT_SELECT_WIDTH;
        area_tip.y = obj->area.y + ((style->page_h - total_h) >> 1);
        if (style->page_h) {
            gt_size_t sy = _get_scroll_y(style);
            gt_size_t sy_min = -(gt_size_t)(style->reg.count - 1) * style->page_h;
            if (sy > 0) { sy = 0; }
            if (sy < sy_min) { sy = sy_min; }
            area_tip.y += -(sy * _TIP_BAR_POINT_PER_ITEM_WIDTH / style->page_h) +
                ((_TIP_BAR_POINT_PER_ITEM_WIDTH - area_tip.h) >> 1);
        }
        rect_attr->bg_opa = GT_OPA_100;
        draw_bg(obj->draw_ctx, rect_attr, &area_tip);
    }
}

static void _init_cb(gt_obj_st * obj) {
    gt_attr_rect_st rect_attr;
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj;
    if (0 == style->reg.count) {
        return;
    }
    bool is_ver = (bool)style->reg.dir;

    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill   = true;
    rect_attr.radius        = 10;
    rect_attr.bg_opa        = obj->opa;
    rect_attr.border_width  = 0;
    rect_attr.bg_color      = style->color_glass;

    if (style->reg.glass) {
        gt_area_st area_bg = obj->area;
        if (!is_ver) {
            uint16_t space = (style->page_w << 1) / 100;
            area_bg.x += (space >> 1);
            area_bg.w = style->page_w - space;
            area_bg.h = style->page_h - _TIP_BAR_HEIGHT;
            if (style->reg.tip_pos) {
                /** tip bar at top: shift glass down */
                area_bg.y += _TIP_BAR_HEIGHT;
            }
            draw_bg(obj->draw_ctx, &rect_attr, &area_bg);
        } else {
            uint16_t space = (style->page_h << 1) / 100;
            area_bg.y += (space >> 1);
            area_bg.h = style->page_h - space;
            area_bg.w = style->page_w - _TIP_BAR_HEIGHT;
            if (style->reg.tip_pos) {
                /** tip bar at left: shift glass right */
                area_bg.x += _TIP_BAR_HEIGHT;
            }
            draw_bg(obj->draw_ctx, &rect_attr, &area_bg);
        }
    }
    _draw_tip_bar(obj, &rect_attr);
    draw_focus(obj);
}

/**
 * @brief A filter which can be slide object child type
 */
static GT_ATTRIBUTE_RAM_TEXT inline bool _ignore_view_pager_slider_control(gt_obj_st * obj) {
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_SLIDER == type) { return true; }
    else if (GT_TYPE_ROLLER == type) { return true; }
    else if (GT_TYPE_LISTVIEW == type) { return true; }
    else if (GT_TYPE_KEYPAD == type) { return true; }
    return false;
}

static GT_ATTRIBUTE_RAM_TEXT bool _unfixed_slider_widgets(gt_obj_st * self, bool is_root) {
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
        if (is_slider) { is_cur_slider = true; }
    }
    if ((is_cur_slider && false == is_root) || _ignore_view_pager_slider_control(self)) {
        if (gt_obj_get_fixed(self)) { gt_obj_set_fixed(self, false); }
    }
    return is_cur_slider;
}

static GT_ATTRIBUTE_RAM_TEXT inline void _init_child_prop(gt_obj_st * content) {
    gt_obj_child_set_prop(content, GT_OBJ_PROP_TYPE_FIXED, true);
    gt_obj_child_set_prop(content, GT_OBJ_PROP_TYPE_INSIDE, true);
    _unfixed_slider_widgets(content, true);
}

/** Resize the content layer to fit count pages */
static GT_ATTRIBUTE_RAM_TEXT inline void _content_resize(_gt_view_pager_st * style) {
    gt_obj_st * c = style->content;
    if (NULL == c) { return; }
    if (style->reg.dir) {
        c->area.w = style->page_w;
        c->area.h = style->page_h * (style->reg.count ? style->reg.count : 1);
    } else {
        c->area.w = style->page_w * (style->reg.count ? style->reg.count : 1);
        c->area.h = style->page_h;
    }
}

/** Damping factor denominator for over-scroll bounce: larger = more resistance */
#define _BOUNCE_DAMP    2

static GT_ATTRIBUTE_RAM_TEXT inline void _scrolling_handler(gt_obj_st * obj) {
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj;
    gt_obj_st * c = style->content;
    if (NULL == c) { return; }
    bool is_ver = (bool)style->reg.dir;
    uint16_t page_size = is_ver ? style->page_h : style->page_w;

    if (page_size && style->reg.count > 1) {
        gt_size_t scroll = is_ver ? c->process_attr.scroll.y : c->process_attr.scroll.x;
        gt_size_t total_min = -(gt_size_t)(style->reg.count - 1) * page_size;
        if (gt_obj_is_scrolling(c) && !style->reg.bouncing) {
            /** User dragging: apply damped over-scroll at boundaries, write back */
            if (scroll < total_min) {
                gt_size_t over = scroll - total_min;
                scroll = total_min + over / _BOUNCE_DAMP;
            } else if (scroll > 0) {
                scroll = scroll / _BOUNCE_DAMP;
            }
        } else {
            /** Animation: hard clamp */
            if (scroll < total_min) { scroll = total_min; }
            if (scroll > 0) { scroll = 0; }
        }
        if (is_ver) { c->process_attr.scroll.y = scroll; }
        else        { c->process_attr.scroll.x = scroll; }
    }

    gt_size_t display_scroll = is_ver ? c->process_attr.scroll.y : c->process_attr.scroll.x;
    if (is_ver) {
        gt_obj_set_pos(c, obj->area.x, obj->area.y + display_scroll);
    } else {
        gt_obj_set_pos(c, obj->area.x + display_scroll, obj->area.y);
    }
    /** redraw the view_pager itself to update tip bar */
    gt_disp_invalid_area(obj);
}

static GT_ATTRIBUTE_RAM_TEXT void _statical_scroll_calc_new_page(gt_obj_st * obj) {
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj;
    gt_obj_st * c = style->content;
    if (NULL == c) { return; }
    uint16_t prev_idx = style->reg.index;
    bool is_ver = (bool)style->reg.dir;
    gt_size_t scroll      = is_ver ? c->process_attr.scroll.y : c->process_attr.scroll.x;
    gt_size_t scroll_prev = is_ver ? gt_obj_get_scroll_prev_y(c) : gt_obj_get_scroll_prev_x(c);
    uint16_t page_size    = is_ver ? style->page_h : style->page_w;

    if (scroll == scroll_prev) { return; }
    if (scroll_prev < scroll) {
        if (style->reg.index) { --style->reg.index; }
        else { style->reg.index = style->reg.count - 1; }
    } else {
        if (style->reg.index + 1 < style->reg.count) { ++style->reg.index; }
        else { style->reg.index = 0; }
    }
    gt_size_t new_scroll = -(gt_size_t)style->reg.index * page_size;
    if (is_ver) {
        c->process_attr.scroll.y = new_scroll;
        gt_obj_set_scroll_prev(c, gt_obj_get_scroll_prev_x(c), new_scroll);
    } else {
        c->process_attr.scroll.x = new_scroll;
        gt_obj_set_scroll_prev(c, new_scroll, gt_obj_get_scroll_prev_y(c));
    }
    if (prev_idx == style->reg.index) { return; }
    if (is_ver) { gt_obj_set_pos(c, obj->area.x, obj->area.y + c->process_attr.scroll.y); }
    else        { gt_obj_set_pos(c, obj->area.x + c->process_attr.scroll.x, obj->area.y); }
    gt_disp_invalid_area(obj);
}

static GT_ATTRIBUTE_RAM_TEXT inline void _scroll_end_handler(gt_obj_st * obj) {
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj;
    gt_obj_st * c = style->content;
    if (NULL == c) { return; }
    bool is_ver = (bool)style->reg.dir;
    uint16_t page_size = is_ver ? style->page_h : style->page_w;

    /** Clear bounce flag when any scroll animation ends */
    bool was_bouncing = (bool)style->reg.bouncing;
    style->reg.bouncing = 0;

    if (c->ignore_throw) {
        _statical_scroll_calc_new_page(obj);
        return;
    }
    uint16_t prev_idx = style->reg.index;
    gt_size_t scroll = is_ver ? c->process_attr.scroll.y : c->process_attr.scroll.x;
    gt_size_t total_min = -(gt_size_t)(style->reg.count - 1) * page_size;

    /** Bounce back if over-scrolled past boundaries */
    if (scroll > 0) {
        if (was_bouncing) {
            /** Already bounced once: force-align to boundary, no further bounce */
            if (is_ver) { c->process_attr.scroll.y = 0; gt_obj_set_pos(c, obj->area.x, obj->area.y); }
            else        { c->process_attr.scroll.x = 0; gt_obj_set_pos(c, obj->area.x, obj->area.y); }
            gt_disp_invalid_area(obj);
            return;
        }
        /** Over-scrolled past first page: animate back to page 0 */
        style->reg.index = 0;
        style->reg.bouncing = 1;
        gt_size_t delta = -scroll;
        if (is_ver) { gt_obj_scroll_to_y_tast(c, delta, GT_ANIM_ON); }
        else        { gt_obj_scroll_to_x_fast(c, delta, GT_ANIM_ON); }
        return;
    }
    if (scroll < total_min) {
        if (was_bouncing) {
            /** Already bounced once: force-align to boundary, no further bounce */
            if (is_ver) { c->process_attr.scroll.y = total_min; gt_obj_set_pos(c, obj->area.x, obj->area.y + total_min); }
            else        { c->process_attr.scroll.x = total_min; gt_obj_set_pos(c, obj->area.x + total_min, obj->area.y); }
            gt_disp_invalid_area(obj);
            return;
        }
        /** Over-scrolled past last page: animate back to last page */
        style->reg.index = style->reg.count - 1;
        style->reg.bouncing = 1;
        gt_size_t delta = total_min - scroll;
        if (is_ver) { gt_obj_scroll_to_y_tast(c, delta, GT_ANIM_ON); }
        else        { gt_obj_scroll_to_x_fast(c, delta, GT_ANIM_ON); }
        return;
    }
    gt_size_t offset = gt_abs(scroll) - prev_idx * page_size;
    uint16_t half = page_size >> 1;

    if (offset > 0 && (uint16_t)offset >= half) {
        if (prev_idx + 1 < style->reg.count) { style->reg.index = prev_idx + 1; }
    } else if (offset < 0 && (uint16_t)gt_abs(offset) >= half) {
        if (prev_idx > 0) { style->reg.index = prev_idx - 1; }
    }
    gt_view_pager_scroll_to_fragment(obj, style->reg.index);
}

static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj;
    gt_event_type_et code_val = gt_event_get_code(e);

    switch (code_val) {
    case GT_EVENT_TYPE_DRAW_START: {
        gt_disp_invalid_area(obj);
        break;
    }
    case GT_EVENT_TYPE_CHANGE_CHILD_ADD: {
        if (style->content) {
            _init_child_prop(style->content);
        }
        break;
    }
    default:
        break;
    }
}

/** Event callback forwarded from the content layer to the view_pager */
static void _content_event_cb(gt_event_st * e) {
    gt_obj_st * content = (gt_obj_st *)e->target;
    gt_obj_st * vp = content->parent;
    if (NULL == vp) { return; }
    _gt_view_pager_st * style = (_gt_view_pager_st * )vp;
    gt_event_type_et code_val = gt_event_get_code(e);
    bool is_ver = (bool)style->reg.dir;

    switch (code_val) {
    case GT_EVENT_TYPE_INPUT_SCROLL_START: {
        break;
    }
    case GT_EVENT_TYPE_INPUT_SCROLL: {
        _scrolling_handler(vp);
        break;
    }
    case GT_EVENT_TYPE_INPUT_SCROLL_END: {
        _scroll_end_handler(vp);
        break;
    }
    case GT_EVENT_TYPE_INPUT_PRESS_LOST:
    case GT_EVENT_TYPE_INPUT_RELEASED: {
        if (gt_obj_is_scrolling(content)) { break; }
        uint16_t page_size = is_ver ? style->page_h : style->page_w;
        gt_size_t scroll = is_ver ? content->process_attr.scroll.y : content->process_attr.scroll.x;
        if (scroll % page_size) {
            /** Snap to nearest page based on current scroll position */
            uint16_t nearest = (uint16_t)(gt_abs(scroll) + (page_size >> 1)) / page_size;
            if (nearest >= style->reg.count) { nearest = style->reg.count - 1; }
            gt_view_pager_scroll_to_fragment(vp, nearest);
        }
        break;
    }
    case GT_EVENT_TYPE_INPUT_SCROLL_UP:
    case GT_EVENT_TYPE_INPUT_SCROLL_LEFT: {
        gt_view_pager_scroll_to_fragment(vp, style->reg.index - 1);
        break;
    }
    case GT_EVENT_TYPE_INPUT_SCROLL_DOWN:
    case GT_EVENT_TYPE_INPUT_SCROLL_RIGHT: {
        gt_view_pager_scroll_to_fragment(vp, style->reg.index + 1);
        break;
    }
    default:
        break;
    }
}

static GT_ATTRIBUTE_RAM_TEXT inline bool _is_over_max_page(uint8_t val) {
    return ((val + 1) < _MAX_VIEW_PAGER_COUNT) ? false : true;
}

static GT_ATTRIBUTE_RAM_TEXT bool _child_is_group(gt_obj_st * obj) {
    return (GT_TYPE_GROUP == obj->classes->type) ? true : false;
}

static GT_ATTRIBUTE_RAM_TEXT void _child_set_group_area(gt_obj_st const * const view_pager, gt_obj_st * group) {
    gt_area_copy(&group->area, &view_pager->area);
}

/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_view_pager_create(gt_obj_st * parent)
{
    if (gt_obj_is_type(parent, GT_TYPE_DIALOG)) {
        GT_LOGE(GT_LOG_TAG_CHECK, "view pager can not be child of dialog");
        return NULL;
    }
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) { return obj; }
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj;

    uint16_t pw = parent ? parent->area.w : gt_disp_get_res_hor(NULL);
    uint16_t ph = parent ? parent->area.h : gt_disp_get_res_ver(NULL);
    style->page_w = pw;
    style->page_h = ph;

    /** view_pager itself = visible window, fixed size, no scroll */
    obj->area.x = 0;
    obj->area.y = 0;
    obj->area.w = pw;
    obj->area.h = ph;
    obj->opa    = GT_OPA_30;
    obj->fixed  = false;
    gt_obj_set_overflow(obj, false);
    gt_obj_set_scroll_dir(obj, GT_SCROLL_DISABLE);

    style->reg.glass = true;
    style->color_glass = gt_color_make(0xff, 0xff, 0xff);

    /** content layer: scrollable, clipped to view_pager area via inside=true.
     *  Keep at absolute (0,0) so snap arithmetic (x / page_w) works correctly. */
    gt_obj_st * content = gt_obj_create(obj);
    content->area.x = 0;
    content->area.y = 0;
    content->area.w = pw;
    content->area.h = ph;
    content->fixed  = false;
    gt_obj_set_overflow(content, true);
    gt_obj_set_inside(content, true);
    gt_obj_set_scroll_dir(content, GT_SCROLL_HORIZONTAL);
    gt_obj_set_scroll_snap_x(content, GT_SCROLL_SNAP_START);
    content->absorb     = true;
    content->absorb_dir = 0;
    gt_obj_add_event_cb(content, _content_event_cb, GT_EVENT_TYPE_INPUT_SCROLL_START, NULL);
    gt_obj_add_event_cb(content, _content_event_cb, GT_EVENT_TYPE_INPUT_SCROLL, NULL);
    gt_obj_add_event_cb(content, _content_event_cb, GT_EVENT_TYPE_INPUT_SCROLL_END, NULL);
    gt_obj_add_event_cb(content, _content_event_cb, GT_EVENT_TYPE_INPUT_RELEASED, NULL);
    gt_obj_add_event_cb(content, _content_event_cb, GT_EVENT_TYPE_INPUT_PRESS_LOST, NULL);
    gt_obj_add_event_cb(content, _content_event_cb, GT_EVENT_TYPE_INPUT_SCROLL_UP, NULL);
    gt_obj_add_event_cb(content, _content_event_cb, GT_EVENT_TYPE_INPUT_SCROLL_DOWN, NULL);
    gt_obj_add_event_cb(content, _content_event_cb, GT_EVENT_TYPE_INPUT_SCROLL_LEFT, NULL);
    gt_obj_add_event_cb(content, _content_event_cb, GT_EVENT_TYPE_INPUT_SCROLL_RIGHT, NULL);
    style->content = content;

    return obj;
}

void gt_view_pager_scroll_to_fragment(gt_obj_st * obj, gt_size_t index)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) { return; }
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj;
    if (index < 0 || index >= style->reg.count) { return; }
    gt_obj_st * c = style->content;
    if (NULL == c) { return; }
    bool is_ver = (bool)style->reg.dir;
    gt_anim_enable_et anim = c->ignore_throw ? GT_ANIM_OFF : GT_ANIM_ON;
    style->reg.index = (uint16_t)index;
    gt_size_t delta = is_ver
        ? (gt_abs(c->process_attr.scroll.y) - (gt_size_t)index * style->page_h)
        : (gt_abs(c->process_attr.scroll.x) - (gt_size_t)index * style->page_w);
    /** delta=0 means already aligned: skip scroll call to avoid stalling is_scrolling state */
    if (0 == delta) { return; }
    if (is_ver) {
        gt_obj_scroll_to_y_tast(c, delta, anim);
    } else {
        gt_obj_scroll_to_x_fast(c, delta, anim);
    }
}

void gt_view_pager_set_scroll_effect(gt_obj_st * obj, gt_view_pager_scroll_effect_et effect)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) { return; }
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj;
    gt_obj_st * c = style->content;
    if (NULL == c) { return; }
    c->ignore_throw    = false;
    c->finger_unfollow = false;
    if (GT_VIEW_PAGER_SCROLL_EFFECT_STATIC == effect) {
        c->ignore_throw    = true;
        c->finger_unfollow = true;
    } else if (GT_VIEW_PAGER_SCROLL_EFFECT_NO_FINGER_FOLLOW == effect) {
        c->finger_unfollow = true;
    }
}

int8_t gt_view_pager_set_fragment_count(gt_obj_st * obj, uint8_t count)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) { return 0; }
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj;
    if (_is_over_max_page(count)) { count = _MAX_VIEW_PAGER_COUNT; }
    style->reg.count = count;
    _content_resize(style);
    return style->reg.count;
}

uint8_t gt_view_pager_get_cur_page(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) { return 0; }
    return ((_gt_view_pager_st * )obj)->reg.index;
}

int8_t gt_view_pager_add_fragment(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) { return 0; }
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj;
    if (_is_over_max_page(style->reg.count)) { return -1; }
    ++style->reg.count;
    _content_resize(style);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
    return style->reg.count;
}

void gt_view_pager_fragment_add_widget(gt_obj_st * view_pager, uint8_t fragment_idx, gt_obj_st * child)
{
    if (false == gt_obj_is_type(view_pager, OBJ_TYPE)) { return; }
    if (_is_over_max_page(fragment_idx)) { return; }
    _gt_view_pager_st * style = (_gt_view_pager_st * )view_pager;
    gt_obj_st * content = style->content;
    if (NULL == content) { return; }

    /** move child into content layer if not already there */
    if (false == gt_obj_is_child(child, content)) {
        gt_obj_st * old_parent = gt_obj_change_parent(child, content);
        _gt_obj_class_inherent_attr_from_parent(child, old_parent);
        _init_child_prop(content);
    }
    if (_child_is_group(child)) {
        _child_set_group_area(view_pager, child);
    }
    /** position child within the content layer.
     *  child->area.x/y is the in-page offset relative to vp origin.
     *  content tracks vp position, so absolute = vp.origin + in-page offset + fragment * page_size */
    if (style->reg.dir) {
        gt_obj_set_pos(child, view_pager->area.x + child->area.x,
                       view_pager->area.y + child->area.y + (uint16_t)fragment_idx * style->page_h);
    } else {
        gt_obj_set_pos(child, view_pager->area.x + child->area.x + (uint16_t)fragment_idx * style->page_w,
                       view_pager->area.y + child->area.y);
    }
}

gt_size_t gt_view_pager_get_widget_belong_fragment(gt_obj_st * view_pager, gt_obj_st * child)
{
    if (false == gt_obj_is_type(view_pager, OBJ_TYPE)) { return -1; }
    _gt_view_pager_st * style = (_gt_view_pager_st * )view_pager;
    gt_obj_st * content = style->content;
    if (NULL == content) { return -1; }
    if (false == gt_obj_is_child(child, content)) { return -1; }
    /** Compensate for scroll offset so the result is stable regardless of current scroll position */
    if (style->reg.dir) {
        gt_size_t scroll = content->process_attr.scroll.y;
        return (child->area.y - view_pager->area.y - scroll) / style->page_h;
    }
    gt_size_t scroll = content->process_attr.scroll.x;
    return (child->area.x - view_pager->area.x - scroll) / style->page_w;
}

void gt_view_pager_set_glass(gt_obj_st * obj, bool enabled)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) { return; }
    ((_gt_view_pager_st * )obj)->reg.glass = enabled;
}

bool gt_view_pager_get_glass(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) { return false; }
    return ((_gt_view_pager_st * )obj)->reg.glass;
}

void gt_view_pager_set_glass_color(gt_obj_st * obj, gt_color_t color)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) { return; }
    ((_gt_view_pager_st * )obj)->color_glass = color;
}

gt_color_t gt_view_pager_get_glass_color(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) { return gt_color_black(); }
    return ((_gt_view_pager_st * )obj)->color_glass;
}

void gt_view_pager_set_window_size(gt_obj_st * obj, uint16_t w, uint16_t h)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) { return; }
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj;
    style->page_w = w;
    style->page_h = h;
    /** view_pager itself stays at visible window size */
    obj->area.w = w;
    obj->area.h = h;
    /** content layer resizes to total pages */
    _content_resize(style);
}

void gt_view_pager_set_direction(gt_obj_st * obj, gt_view_pager_dir_et dir)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) { return; }
    _gt_view_pager_st * style = (_gt_view_pager_st * )obj;
    gt_obj_st * c = style->content;
    style->reg.dir = (uint16_t)dir;
    if (GT_VIEW_PAGER_DIR_VERTICAL == dir) {
        if (c) {
            gt_obj_set_scroll_dir(c, GT_SCROLL_VERTICAL);
            gt_obj_set_scroll_snap_x(c, GT_SCROLL_SNAP_NONE);
            gt_obj_set_scroll_snap_y(c, GT_SCROLL_SNAP_START);
            c->absorb_dir = 1;
        }
    } else {
        if (c) {
            gt_obj_set_scroll_dir(c, GT_SCROLL_HORIZONTAL);
            gt_obj_set_scroll_snap_x(c, GT_SCROLL_SNAP_START);
            gt_obj_set_scroll_snap_y(c, GT_SCROLL_SNAP_NONE);
            c->absorb_dir = 0;
        }
    }
    _content_resize(style);
}

gt_view_pager_dir_et gt_view_pager_get_direction(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) { return GT_VIEW_PAGER_DIR_HORIZONTAL; }
    return (gt_view_pager_dir_et)((_gt_view_pager_st * )obj)->reg.dir;
}

void gt_view_pager_set_tip_bar_pos(gt_obj_st * obj, gt_view_pager_tip_bar_pos_et pos)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) { return; }
    ((_gt_view_pager_st * )obj)->reg.tip_pos = (uint16_t)pos;
}

/* end ------------------------------------------------------------------*/
#endif /** GT_CFG_ENABLE_VIEW_PAGER */
