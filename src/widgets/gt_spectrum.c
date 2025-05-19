/**
 * @file gt_spectrum.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2025-05-16 14:43:34
 * @copyright Copyright (c) 2014-2025, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_spectrum.h"
#if GT_CFG_ENABLE_SPECTRUM
#include "../core/gt_mem.h"
#include "../hal/gt_hal_disp.h"
#include "../others/gt_log.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../core/gt_event.h"
#include "../others/gt_area.h"
#include "../others/gt_anim.h"
#include "../core/gt_timer.h"
/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_SPECTRUM
#define MY_CLASS    &gt_spectrum_class



/* private typedef ------------------------------------------------------*/
typedef struct
{
    uint16_t max;
    uint16_t min;
    uint16_t steps;
}_gt_spectrum_range_st;

typedef struct _gt_spectrum_s{
    gt_obj_st obj;
    _gt_timer_st* timer;

    gt_gradient_color_st gradient;
    _gt_spectrum_range_st range;
    uint16_t numb;
    uint16_t gap;

    gt_spectrum_type_et type;
}_gt_spectrum_st;


/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_spectrum_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_spectrum_st)
};

/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
GT_ATTRIBUTE_RAM_TEXT static uint16_t
_get_item_range_size(_gt_spectrum_st * style){
    return (style->range.max - style->range.min) / style->range.steps;
}

GT_ATTRIBUTE_RAM_TEXT static uint16_t
_get_item_ratio_size(_gt_spectrum_st * style, uint16_t value)
{
    uint16_t size = value / style->numb;
    if (size < style->gap) {
        return 0;
    }

    return size - style->gap;
}

GT_ATTRIBUTE_RAM_TEXT static void
_get_item_param(_gt_spectrum_st * style, uint16_t value, uint16_t * gap, uint16_t* ratio_size, uint16_t* range_size)
{
    *gap = style->gap;
    *ratio_size = _get_item_ratio_size(style, value);
    *range_size = _get_item_range_size(style);
    if(0 == *ratio_size){
        *ratio_size = 1;
        *gap = (*gap > *ratio_size) ? *gap - *ratio_size : 0;
    }
}

GT_ATTRIBUTE_RAM_TEXT static void
_draw_circle_spectrum(gt_obj_st * obj)
{
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;

    gt_attr_arc_st arc_attr = {0};
    arc_attr.color = obj->bgcolor;
    arc_attr.opa = obj->opa;

    uint16_t angle_gap = 0, angle_size = 0, range_size = 0;
    _get_item_param(style, 360, &angle_gap, &angle_size, &range_size);

    uint32_t random = 0;
    gt_area_st item_area = {0};

    for (uint16_t i = 0; i < style->numb; i++)
    {
        arc_attr.start_angle = i * (angle_size + angle_gap);
        arc_attr.end_angle = arc_attr.start_angle + angle_size;

        random = gt_random(0, style->range.steps - 1);
        arc_attr.width = style->range.min + random * range_size;

        if(style->gradient.steps >= 2) {
            arc_attr.color = gt_gradient_color_get(style->gradient, random);
        }

        item_area = gt_area_reduce(obj->area,  style->range.max - arc_attr.width);
        gt_draw_arc(obj->draw_ctx, &arc_attr, &item_area);
    }
}

GT_ATTRIBUTE_RAM_TEXT static void
_draw_rect_spectrum(gt_obj_st * obj)
{
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;

    gt_attr_rect_st rect_attr = {0};
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill   = true;
    rect_attr.bg_opa        = obj->opa;
    rect_attr.fg_color      = obj->bgcolor;
    rect_attr.bg_color      = obj->bgcolor;
    rect_attr.radius        = obj->radius;

    uint16_t rect_gap = 0, rect_size = 0, range_size = 0;
    _get_item_param(style, obj->area.w, &rect_gap, &rect_size, &range_size);

    uint32_t random = 0;
    gt_area_st item_area = {0};

    for (uint16_t i = 0; i < style->numb; i++)
    {
        item_area.x = i * (rect_size + rect_gap) + obj->area.x;
        item_area.w = rect_size;

        random = gt_random(0, style->range.steps - 1);

        item_area.h = style->range.min + random * range_size;
        item_area.y = obj->area.y + style->range.max;
        if(GT_SPECTRUM_TYPE_RECT_UD == style->type) {
            item_area.y -= (item_area.h >> 1);
        }
        else {
            item_area.y -= item_area.h;
        }

        if(style->gradient.steps >= 2) {
            rect_attr.bg_color = gt_gradient_color_get(style->gradient, random);
        }

        draw_bg(obj->draw_ctx, &rect_attr, &item_area);
    }
}

GT_ATTRIBUTE_RAM_TEXT static void
_init_cb(gt_obj_st * obj)
{
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;

    switch (style->type)
    {
    case GT_SPECTRUM_TYPE_CIRCLE:
        _draw_circle_spectrum(obj);
        break;

    case GT_SPECTRUM_TYPE_RECT:
    case GT_SPECTRUM_TYPE_RECT_UD:
        _draw_rect_spectrum(obj);
        break;

    default:
        break;
    }
}

static GT_ATTRIBUTE_RAM_TEXT void
_destroy_timer(_gt_spectrum_st * style) {
    if ( !style->timer ) {
        return ;
    }
    _gt_timer_del(style->timer);
    style->timer = NULL;
}

GT_ATTRIBUTE_RAM_TEXT static void
_deinit_cb(gt_obj_st * obj)
{
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;
    _destroy_timer(style);
}

GT_ATTRIBUTE_RAM_TEXT static void
_event_cb(struct gt_obj_s * obj, gt_event_st * e)
{
    gt_event_type_et code_val = gt_event_get_code(e);
    switch(code_val) {
        case GT_EVENT_TYPE_DRAW_START:
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;
        default:
            break;
    }
}

static GT_ATTRIBUTE_RAM_TEXT void
_auto_play_callback(struct _gt_timer_s * timer)
{
    gt_obj_st * obj = (gt_obj_st * )_gt_timer_get_user_data(timer);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

/* global functions / API interface -------------------------------------*/
GT_ATTRIBUTE_RAM_TEXT gt_obj_st *
gt_spectrum_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    GT_CHECK_BACK_VAL(obj, NULL);

    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;
    obj->radius = 0;
    style->type = GT_SPECTRUM_TYPE_CIRCLE;
    style->gap = 4;
    style->numb = 20;
    style->range.max = 40;
    style->range.min = 10;
    style->range.steps = 4;

    style->gradient.start = gt_color_hex(0xB8BAF9); // #B8BAF9
    style->gradient.end = gt_color_hex(0x748EF6);   // #748EF6
    style->gradient.steps = 4;

    style->timer = _gt_timer_create(_auto_play_callback, 200, (void * )obj);
    _gt_timer_set_paused(style->timer, true);

    return obj;
}

GT_ATTRIBUTE_RAM_TEXT void
gt_spectrum_set_type(gt_obj_st * obj, gt_spectrum_type_et type)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;
    style->type = type;
}

GT_ATTRIBUTE_RAM_TEXT void
gt_spectrum_set_period(gt_obj_st * obj, uint32_t period)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;
    _gt_timer_set_period(style->timer, period);
}

GT_ATTRIBUTE_RAM_TEXT void
gt_spectrum_play(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;
    _gt_timer_set_paused(style->timer, false);
}

GT_ATTRIBUTE_RAM_TEXT void
gt_spectrum_stop(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;
    _gt_timer_set_paused(style->timer, true);
}

GT_ATTRIBUTE_RAM_TEXT void
gt_spectrum_set_range(gt_obj_st * obj, uint16_t min, uint16_t max, uint16_t steps)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;
    if(min > max) {
        style->range.min = max;
        style->range.max = min;
    }
    else {
        style->range.min = min;
        style->range.max = max;
    }
    style->range.steps = steps;
}

GT_ATTRIBUTE_RAM_TEXT void
gt_spectrum_set_numb(gt_obj_st * obj, uint16_t numb, uint16_t gap)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;
    style->numb = numb;
    style->gap = gap;
}

GT_ATTRIBUTE_RAM_TEXT void
gt_spectrum_set_gradient_color(gt_obj_st * obj, gt_color_t start, gt_color_t end, uint16_t steps)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;
    style->gradient.start = start;
    style->gradient.end = end;
    style->gradient.steps = steps;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

/* end of file ----------------------------------------------------------*/
#endif // !GT_CFG_ENABLE_SPECTRUM


