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
#include "../utils/gt_circular_array.h"
/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_SPECTRUM
#define MY_CLASS    &gt_spectrum_class



/* private typedef ------------------------------------------------------*/
typedef struct
{
    gt_circular_array_st * value; // Circular array for current values
    gt_circular_array_st * aim;   // Circular array for target values
    gt_anim_st anim;
    uint16_t anim_count;
    uint16_t height;
    uint16_t steps;
    uint8_t gap;
    uint8_t reflect :       1;
    uint8_t not_random :    1;
    uint8_t reserved :      6;
}_gt_spectrum_range_st;

typedef struct {
    uint16_t numb;
    uint16_t gap;
    _gt_spectrum_range_st range;
}_gt_spectrum_item_st;

typedef struct{
    _gt_vector_st* gradient;
    uint16_t max_steps;
    uint16_t move_dist;
    uint8_t move_gap;
    uint8_t move_dir :     1;
    uint8_t move_enable :  1;
    uint8_t type :         4;
    uint8_t reserved :     2;
}_gt_spectrum_color_st;


typedef struct _gt_spectrum_s{
    gt_obj_st base;
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
_get_item_range_size(_gt_spectrum_st * style, uint16_t value, uint16_t* gap){
    uint16_t size = value / style->item.range.steps;
    if(!gap) {
        return size;
    }

    *gap = style->item.range.gap;
    if(size <= style->item.range.gap){
        size = 1;
        *gap = (style->item.range.gap > size) ? style->item.range.gap - size : 0;
        return size;
    }

    return size - style->item.range.gap;
}

GT_ATTRIBUTE_RAM_TEXT static uint16_t
_get_item_ratio_size(_gt_spectrum_st * style, uint16_t value, uint16_t* gap)
{
    uint16_t size = value / (style->item.numb);
    *gap = style->item.gap;
    if(size <= style->item.gap){
        size = 1;
        *gap = (*gap > size) ? *gap - size : 0;
        return size;
    }

    return size - style->item.gap;
}



GT_ATTRIBUTE_RAM_TEXT static uint16_t
_get_item_value(_gt_spectrum_st * style, uint16_t numb_step)
{
    uint16_t value = 0;
    gt_circular_array_get(style->item.range.value, numb_step, (void*)&value);
    return value;
}

GT_ATTRIBUTE_RAM_TEXT static void
_set_item_value(_gt_spectrum_st * style, uint16_t numb_step, uint16_t value)
{
    gt_circular_array_set(style->item.range.value, numb_step, (const void*)&value);
}

GT_ATTRIBUTE_RAM_TEXT static uint16_t
_get_item_aim(_gt_spectrum_st * style, uint16_t numb_step)
{
    uint16_t value = 0;
    gt_circular_array_get(style->item.range.aim, numb_step, (void*)&value);
    return value;
}

GT_ATTRIBUTE_RAM_TEXT static void
_set_item_aim(_gt_spectrum_st * style, uint16_t numb_step, uint16_t value)
{
    gt_circular_array_set(style->item.range.aim, numb_step, (const void*)&value);
}


GT_ATTRIBUTE_RAM_TEXT static uint16_t
_get_item_range_step(_gt_spectrum_st * style, uint16_t numb_step)
{
    return _get_item_value(style, numb_step) % style->item.range.steps;
}


GT_ATTRIBUTE_RAM_TEXT static uint16_t
_get_gradient_color_move_step(_gt_spectrum_st * style, uint16_t step)
{
    uint32_t move_step = step;
    if(style->color.move_enable) {
        if(style->color.move_dir) {
            move_step = (uint32_t)(step + style->color.move_dist) % style->color.max_steps;
        } else {
            move_step = (uint32_t)(style->color.max_steps + step - style->color.move_dist) % style->color.max_steps;
        }
    }
    return (uint16_t)move_step;
}

GT_ATTRIBUTE_RAM_TEXT static void
_gt_gradient_color_move_dist_inc(_gt_spectrum_st * style)
{
    style->color.move_dist = (style->color.move_dist + style->color.move_gap) % style->color.max_steps;
}


GT_ATTRIBUTE_RAM_TEXT static void
_get_item_gradient_color(_gt_spectrum_st * style, gt_color_t* color, uint16_t range_step, uint16_t numb_step)
{
    if(NULL == style->color.gradient) {return ;}

    gt_size_t count = _gt_vector_get_count(style->color.gradient);
    if(count <= 0) {return ;}

    uint32_t step = 0;

    if(GT_SPECTRUM_COLOR_TYPE_NUMB == style->color.type) {
        step = _get_gradient_color_move_step(style, numb_step);
    }
    else{
        step = _get_gradient_color_move_step(style, range_step);
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
    angle_size = _get_item_ratio_size(style, 360, &angle_gap);
    range_size = _get_item_range_size(style, style->item.range.height, NULL);

    uint16_t range_step = 0;
    gt_area_st item_area = {0};

    for (uint16_t i = 0; i < style->item.numb; ++i)
    {
        arc_attr.start_angle = i * (angle_size + angle_gap);
        arc_attr.end_angle = arc_attr.start_angle + angle_size;

        range_step = _get_item_range_step(style , i);
        arc_attr.width = (range_step + 1) * range_size;

        if(style->color.gradient) {
            _get_item_gradient_color(style, &arc_attr.color, range_step, i);
        }

        item_area = gt_area_reduce(obj->area,  style->item.range.height - arc_attr.width);
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
    rect_attr.limit_area    = &obj->area;

    uint16_t rect_gap = 0, rect_size = 0, range_size = 0;
    rect_size = _get_item_ratio_size(style, obj->area.w, &rect_gap);
    range_size = _get_item_range_size(style, obj->area.h, NULL);

    uint16_t range_step = 0;
    gt_area_st item_area = {0};
    item_area.w = rect_size;
    item_area.x = obj->area.x + ((obj->area.w - ((rect_size + rect_gap) * style->item.numb) + rect_gap) >> 1);
    for (uint16_t i = 0; i < style->item.numb; ++i)
    {
        range_step = _get_item_range_step(style , i);

        item_area.h = (range_step + 1) * range_size;
        item_area.y = obj->area.y;
        if (GT_SPECTRUM_TYPE_RECT == style->type) {
            item_area.y += (obj->area.h - item_area.h);
        }
        else if(GT_SPECTRUM_TYPE_RECT_UD == style->type) {
            item_area.y += ((obj->area.h - item_area.h) >> 1);
        }

        if(style->color.gradient) {
            _get_item_gradient_color(style, &rect_attr.bg_color, range_step, i);
        }

        draw_bg(obj->draw_ctx, &rect_attr, &item_area);

        item_area.x += rect_size + rect_gap;
    }
}

GT_ATTRIBUTE_RAM_TEXT static void
_draw_stair_spectrum(gt_obj_st * obj)
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

    uint16_t range_gap = style->item.range.gap;
    uint16_t rect_gap = 0, rect_size = 0, range_size = 0;
    rect_size = _get_item_ratio_size(style, obj->area.w, &rect_gap);

    if(style->item.range.reflect) {
        range_size = _get_item_range_size(style, obj->area.h >> 1, &range_gap);
    }
    else {
        range_size = _get_item_range_size(style, obj->area.h, &range_gap);
    }

    uint16_t range_step = 0;
    gt_area_st item_area = {0};
    item_area.x = obj->area.x + ((obj->area.w - ((rect_size + rect_gap) * style->item.numb) + rect_gap) >> 1);

    item_area.w = rect_size;
    item_area.h = range_size;

    for (uint16_t i = 0; i < style->item.numb; ++i)
    {
        range_step = _get_item_range_step(style , i);

        if(style->color.gradient) {
            _get_item_gradient_color(style, &rect_attr.bg_color, range_step, i);
        }

        if(style->item.range.reflect) {
            item_area.y = obj->area.y + (obj->area.h >> 1) - range_size - range_gap;
        }
        else {
            item_area.y = obj->area.y + obj->area.h - range_size;
        }

        for(uint16_t n = 0; n < (range_step + 1); ++n) {
            draw_bg(obj->draw_ctx, &rect_attr, &item_area);
            item_area.y -= (range_size + range_gap);
        }

        if( style->item.range.reflect )
        {
            item_area.y = obj->area.y + ((obj->area.h) >> 1) + range_gap;

            gt_gradient_color_st gradient = {0};
            gradient.start = rect_attr.bg_color;
            gradient.end = obj->bgcolor;
            gradient.steps = style->item.range.steps << 1;

            for(uint16_t n = 0; n < (range_step + 1); ++n) {
                rect_attr.bg_color = gt_gradient_color_get(gradient, n + (style->item.range.steps));
                draw_bg(obj->draw_ctx, &rect_attr, &item_area);
                item_area.y += (range_size + range_gap);
            }
        }
        item_area.x += rect_size + rect_gap;
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

    switch (style->type)
    {
    case GT_SPECTRUM_TYPE_CIRCLE:
        _draw_circle_spectrum(obj);
        break;

    case GT_SPECTRUM_TYPE_RECT:
    case GT_SPECTRUM_TYPE_RECT_UD:
        _draw_rect_spectrum(obj);
        break;
    case GT_SPECTRUM_TYPE_STAIR:
        _draw_stair_spectrum(obj);
        break;

    default:
        break;
    }

    //
    _gt_gradient_color_move_dist_inc(style);

    // focus
    draw_focus(obj);
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
    if (NULL != style->item.range.value) {
        gt_circular_array_destroy(style->item.range.value);
        style->item.range.value = NULL;
    }

    if (NULL != style->item.range.aim) {
        gt_circular_array_destroy(style->item.range.aim);
        style->item.range.aim = NULL;
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
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;
    uint16_t max_value = 0, random = 0, value = 0;
    for(uint16_t i  = 0; i < style->item.numb; ++i) {
        random = gt_random(0, style->item.range.steps-1);
        value = _get_item_value(style, i);
        if(random > value) {
            value = random - value;
        }
        else {
            value = value - random;
        }
        _set_item_aim(style, i, random);

        max_value = GT_MAX(max_value, value);
    }

    style->item.range.anim_count = 0;
    gt_anim_set_time(&style->item.range.anim, _gt_timer_get_period(style->timer));
    gt_anim_set_value(&style->item.range.anim, 0, max_value);
    gt_anim_start(&style->item.range.anim);
}

static GT_ATTRIBUTE_RAM_TEXT void _item_anim_ready_cb(struct gt_anim_s * anim) {

}

static GT_ATTRIBUTE_RAM_TEXT void _item_anim_exec_cb(void * obj, int32_t count) {

    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;
    uint16_t value = 0, aim = 0, numb = 0;

    numb = count -  style->item.range.anim_count;
    style->item.range.anim_count = count;

    for(uint16_t i = 0; i < style->item.numb; ++i) {
        value = _get_item_value(style, i);
        aim = _get_item_aim(style, i);

        if(value < aim) {
            value += numb;
            if(value >= aim) {
                value = aim;
            }
        }
        else if(value > aim) {
            if(value <= numb){
                value = 0;
            }
            else{
                value -= numb;
            }

            if(value <= aim) {
                value = aim;
            }
        }

        _set_item_value(style, i, value);
    }
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
    gt_obj_type_et type = gt_obj_class_get_type(obj);
    if (GT_TYPE_UNKNOWN == type) {
        return NULL;
    }

    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;
    obj->radius = 0;
    style->type = GT_SPECTRUM_TYPE_CIRCLE;
    style->item.gap = 4;
    style->item.numb = 20;
    style->item.range.height = 40;
    style->item.range.steps = 4;

    style->color.gradient = _gt_vector_create(_gradient_free_cb, _gradient_equal_cb);
    if(!style->color.gradient) {
        GT_LOGW(GT_LOG_TAG_GUI,"create gradient vector failed!!!");
    }

    style->timer = _gt_timer_create(_auto_play_callback, 200, (void * )obj);
    _gt_timer_set_paused(style->timer, true);

    gt_anim_init(&style->item.range.anim);
    gt_anim_set_target(&style->item.range.anim, obj);
    gt_anim_set_start_cb(&style->item.range.anim, NULL);
    gt_anim_set_ready_cb(&style->item.range.anim, _item_anim_ready_cb);
    gt_anim_set_exec_cb(&style->item.range.anim, _item_anim_exec_cb);

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
    if(style->item.range.not_random) {
        _gt_timer_set_paused(style->timer, true);
        return ;
    }
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

    if(0 == numb) {
        if(style->item.range.value) {
            gt_circular_array_destroy(style->item.range.value);
        }
        if(style->item.range.aim) {
            gt_circular_array_destroy(style->item.range.aim);
        }
    }
    else if(numb != style->item.numb) {
        if(style->item.range.value) {
            gt_circular_array_destroy(style->item.range.value);
        }

        style->item.range.value = gt_circular_array_create(numb, sizeof(uint16_t), true);

        if(!style->item.range.value) {
            GT_LOGE(GT_LOG_TAG_GUI,"malloc item value failed!!!");
            style->item.numb = 0;
            return ;
        }

        if(style->item.range.aim) {
            gt_circular_array_destroy(style->item.range.aim);
        }

        style->item.range.aim = gt_circular_array_create(numb, sizeof(uint16_t), true);

        if(!style->item.range.aim) {
            GT_LOGE(GT_LOG_TAG_GUI,"malloc item aim failed!!!");
            gt_circular_array_destroy(style->item.range.value);
            style->item.numb = 0;
            return ;
        }

        gt_circular_array_clear(style->item.range.value);
        gt_circular_array_clear(style->item.range.aim);
        //
        gt_circular_array_set_full(style->item.range.value);
        gt_circular_array_set_full(style->item.range.aim);
    }

    style->item.numb = numb;
    style->item.gap = gap;
}

GT_ATTRIBUTE_RAM_TEXT void
gt_spectrum_set_item_range(gt_obj_st * obj, uint16_t height, uint16_t steps)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;
    style->item.range.height = height;
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
gt_spectrum_set_item_range_not_random(gt_obj_st * obj, bool not_random)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;
    style->item.range.not_random = not_random;
}

GT_ATTRIBUTE_RAM_TEXT void
gt_spectrum_push_item_range_value(gt_obj_st * obj, uint16_t value)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;

    if(!style->item.range.value) {
        GT_LOGE(GT_LOG_TAG_GUI,"spectrum item range value is NULL!!!");
        return ;
    }

    if(!style->item.range.not_random) {
        return ;
    }

    gt_circular_array_push_head(style->item.range.value, (const void *)&value);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
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
gt_spectrum_set_color_move(gt_obj_st* obj, bool enable, bool dir, uint8_t gap)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_spectrum_st * style = (_gt_spectrum_st *)obj;
    style->color.move_enable = enable;
    style->color.move_dir = dir;
    style->color.move_gap = gap;
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
    style->color.max_steps += steps;
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
    style->color.max_steps = 0;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}


/* end of file ----------------------------------------------------------*/
#endif // !GT_CFG_ENABLE_SPECTRUM


