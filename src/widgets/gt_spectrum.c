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
#include "../utils/gt_vector.h"
/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_SPECTRUM
#define MY_CLASS    &gt_spectrum_class



/* private typedef ------------------------------------------------------*/
typedef struct
{
    uint16_t max;
    uint16_t min;
    uint16_t steps;
    uint8_t gap;
    uint8_t reflect : 1;
    uint8_t reserved : 7;
}_gt_spectrum_range_st;

typedef struct {
    _gt_spectrum_range_st range;
    uint16_t numb;
    uint16_t gap;
}_gt_spectrum_item_st;

typedef struct{
    _gt_vector_st* gradient;
    uint16_t move_dist;
    uint8_t move_step;
    uint8_t move_dir :     1;
    uint8_t move_enable :  1;
    uint8_t type :         4;
    uint8_t reserved :     2;
}_gt_spectrum_color_st;


typedef struct _gt_spectrum_s{
    gt_obj_st obj;
    _gt_timer_st* timer;
    _gt_spectrum_item_st item;
    _gt_spectrum_color_st color;
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
    return (style->item.range.max - style->item.range.min) / style->item.range.steps;
}

GT_ATTRIBUTE_RAM_TEXT static uint16_t
_get_item_ratio_size(_gt_spectrum_st * style, uint16_t value)
{
    uint16_t size = value / style->item.numb;
    if (size < style->item.gap) {
        return 0;
    }

    return size - style->item.gap;
}

GT_ATTRIBUTE_RAM_TEXT static void
_get_item_param(_gt_spectrum_st * style, uint16_t value, uint16_t * gap, uint16_t* ratio_size, uint16_t* range_size)
{
    *gap = style->item.gap;
    *ratio_size = _get_item_ratio_size(style, value);
    *range_size = _get_item_range_size(style);
    if(0 == *ratio_size){
        *ratio_size = 1;
        *gap = (*gap > *ratio_size) ? *gap - *ratio_size : 0;
    }
}

GT_ATTRIBUTE_RAM_TEXT static uint16_t
_get_item_range_step(_gt_spectrum_st * style, uint16_t numb_step)
{
    return gt_random(0, style->item.range.steps - 1);
}

GT_ATTRIBUTE_RAM_TEXT static void
_get_item_gradient_color(_gt_spectrum_st * style, gt_color_t* color, uint16_t range_step, uint16_t numb_step)
{
    if(NULL == style->color.gradient) {return ;}

    gt_size_t count = _gt_vector_get_count(style->color.gradient);
    if(count <= 0) {return ;}

    uint32_t step = 0;

    if(GT_SPECTRUM_COLOR_TYPE_NUMB == style->color.type) {
        step = numb_step;
        if(style->color.move_enable) {
            if(style->color.move_dir) {
                step = (step + style->color.move_dist) % style->item.numb;
            } else {
                step = (style->item.numb + step - style->color.move_dist) % style->item.numb;
            }
        }
    }
    else{
        step = range_step;
        if(style->color.move_enable) {
            if(style->color.move_dir) {
                step = (step + style->color.move_dist) % style->item.range.steps;
            } else {
                step = (style->item.range.steps + step - style->color.move_dist) % style->item.range.steps;
            }
        }
    }
    gt_gradient_color_get_by_vector(color, style->color.gradient, step);
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

    uint16_t range_step = 0;
    gt_area_st item_area = {0};

    for (uint16_t i = 0; i < style->item.numb; ++i)
    {
        arc_attr.start_angle = i * (angle_size + angle_gap);
        arc_attr.end_angle = arc_attr.start_angle + angle_size;

        range_step = _get_item_range_step(style , i);
        arc_attr.width = style->item.range.min + range_step * range_size;

        if(style->color.gradient) {
            _get_item_gradient_color(style, &arc_attr.color, range_step, i);
        }

        item_area = gt_area_reduce(obj->area,  style->item.range.max - arc_attr.width);
        gt_draw_arc(obj->draw_ctx, &arc_attr, &item_area);
    }
}

GT_ATTRIBUTE_RAM_TEXT static void
_draw_stair_range(gt_obj_st* obj, gt_attr_rect_st* rect_attr, gt_area_st* item_area, uint16_t range_step)
{
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;

    uint16_t range_gap = style->item.range.gap;
    uint16_t h = (style->item.range.max) / style->item.range.steps;
    if(h < range_gap) {
        h = 1;
        range_gap = (range_gap > h) ? range_gap - h : 0;
    }
    else{
        h -= range_gap;
    }

    gt_area_st stair_area = {0};
    gt_area_copy(&stair_area, item_area);
    stair_area.h = h;
    stair_area.y = obj->area.y + style->item.range.max - h;

    for(uint16_t i = 0; i < (range_step + 1); ++i) {
        draw_bg(obj->draw_ctx, rect_attr, &stair_area);
        stair_area.y -= (h + range_gap);
    }

    if( !style->item.range.reflect ) { return ; }

    gt_gradient_color_st gradient = {0};
    gradient.start = rect_attr->bg_color;
    gradient.end = obj->bgcolor;
    gradient.steps = style->item.range.steps << 1;
    stair_area.y = obj->area.y + + style->item.range.max + (range_gap << 1);

    for(uint16_t i = 0; i < (range_step + 1); ++i) {

        rect_attr->bg_color = gt_gradient_color_get(gradient, i + (style->item.range.steps));
        draw_bg(obj->draw_ctx, rect_attr, &stair_area);
        stair_area.y += (h + range_gap);
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
    rect_attr.limit_area    = &obj->area;

    uint16_t rect_gap = 0, rect_size = 0, range_size = 0;
    _get_item_param(style, obj->area.w, &rect_gap, &rect_size, &range_size);

    uint16_t range_step = 0;
    gt_area_st item_area = {0};

    for (uint16_t i = 0; i < style->item.numb; ++i)
    {
        item_area.x = i * (rect_size + rect_gap) + obj->area.x;
        item_area.w = rect_size;

        range_step = _get_item_range_step(style , i);

        item_area.h = style->item.range.min + range_step * range_size;
        item_area.y = obj->area.y + style->item.range.max;
        if (GT_SPECTRUM_TYPE_RECT == style->type) {
            item_area.y -= item_area.h;
        }
        else if(GT_SPECTRUM_TYPE_RECT_UD == style->type) {
            item_area.y -= (item_area.h >> 1);
        }

        if(style->color.gradient) {
            _get_item_gradient_color(style, &rect_attr.bg_color, range_step, i);
        }

        if(GT_SPECTRUM_TYPE_STAIR == style->type) {
            _draw_stair_range(obj, &rect_attr, &item_area, range_step);
        }
        else {
            draw_bg(obj->draw_ctx, &rect_attr, &item_area);
        }
    }
}


GT_ATTRIBUTE_RAM_TEXT static void
_init_cb(gt_obj_st * obj)
{
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;

    if(obj->show_bg) {
        gt_attr_rect_st rect_attr = {0};
        gt_graph_init_rect_attr(&rect_attr);
        rect_attr.bg_color = obj->bgcolor;
        rect_attr.bg_opa = obj->opa;
        rect_attr.reg.is_fill = true;
        draw_bg(obj->draw_ctx, &rect_attr, &obj->area);
    }

    //
    style->color.move_dist += style->color.move_step;

    switch (style->type)
    {
    case GT_SPECTRUM_TYPE_CIRCLE:
        _draw_circle_spectrum(obj);
        break;

    case GT_SPECTRUM_TYPE_RECT:
    case GT_SPECTRUM_TYPE_RECT_UD:
    case GT_SPECTRUM_TYPE_STAIR:
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

    if (NULL != style->color.gradient) {
        _gt_vector_free(style->color.gradient);
        style->color.gradient = NULL;
    }

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

static GT_ATTRIBUTE_RAM_TEXT bool _gradient_free_cb(void * item) {
    if (NULL == item) {
        return false;
    }
    gt_mem_free(item);
    return true;
}

static GT_ATTRIBUTE_RAM_TEXT bool _gradient_equal_cb(void * item, void * target) {
    return strcmp(item, target) ? false : true;
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
    style->item.gap = 4;
    style->item.numb = 20;
    style->item.range.max = 40;
    style->item.range.min = 10;
    style->item.range.steps = 4;

    style->color.gradient = _gt_vector_create(_gradient_free_cb, _gradient_equal_cb);
    if(!style->color.gradient) {
        GT_LOGW(GT_LOG_TAG_GUI,"create gradient vector failed!!!");
    }

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
gt_spectrum_set_item_numb(gt_obj_st * obj, uint16_t numb, uint16_t gap)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;
    style->item.numb = numb;
    style->item.gap = gap;
}

GT_ATTRIBUTE_RAM_TEXT void
gt_spectrum_set_item_range(gt_obj_st * obj, uint16_t min, uint16_t max, uint16_t steps)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;
    if(min > max) {
        style->item.range.min = max;
        style->item.range.max = min;
    }
    else {
        style->item.range.min = min;
        style->item.range.max = max;
    }
    style->item.range.steps = steps;
}

GT_ATTRIBUTE_RAM_TEXT void
gt_spectrum_set_item_range_reflect(gt_obj_st * obj, bool reflect)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;
    style->item.range.reflect = reflect;
}

GT_ATTRIBUTE_RAM_TEXT void
gt_spectrum_set_item_range_gap(gt_obj_st * obj, uint16_t gap)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;
    style->item.range.gap = gap;
}

GT_ATTRIBUTE_RAM_TEXT void
gt_spectrum_set_color_type(gt_obj_st * obj, gt_spectrum_color_type_et type)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;
    style->color.type = type;
}

GT_ATTRIBUTE_RAM_TEXT void
gt_spectrum_set_color_move(gt_obj_st* obj, bool enable, bool dir, uint8_t step)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;
    style->color.move_enable = enable;
    style->color.move_dir = dir;
    style->color.move_step = step;
}

GT_ATTRIBUTE_RAM_TEXT void
gt_spectrum_add_gradient_color(gt_obj_st * obj, gt_color_t start, gt_color_t end, uint16_t steps)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;

    if(!style->color.gradient) {
        style->color.gradient = _gt_vector_create(_gradient_free_cb, _gradient_equal_cb);
        if(!style->color.gradient) {
            GT_LOGW(GT_LOG_TAG_GUI,"create gradient vector failed!!!");
            return;
        }
    }

    gt_gradient_color_st* gradient = gt_mem_malloc(sizeof(gt_gradient_color_st));
    if(!gradient) {
        GT_LOGW(GT_LOG_TAG_GUI,"create gradient color failed!!!");
        return;
    }

    gradient->start = start;
    gradient->end = end;
    gradient->steps = steps;

    _gt_vector_add_item(style->color.gradient, (void *)gradient);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

GT_ATTRIBUTE_RAM_TEXT void
gt_spectrum_clear_gradient_color(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;

    if(!style->color.gradient) {
        return ;
    }

    _gt_vector_clear_all_items(style->color.gradient);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

/* end of file ----------------------------------------------------------*/
#endif // !GT_CFG_ENABLE_SPECTRUM


