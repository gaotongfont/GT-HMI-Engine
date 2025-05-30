/**
 * @file gt_spectrum_line.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2025-05-28 15:39:19
 * @copyright Copyright (c) 2014-2025, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_spectrum_line.h"
#if GT_CFG_ENABLE_SPECTRUM_LINE
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
#define OBJ_TYPE    GT_TYPE_SPECTRUM_LINE
#define MY_CLASS    &gt_spectrum_line_class



/* private typedef ------------------------------------------------------*/
typedef struct {
    _gt_vector_st* sp_point;
    gt_line_st line;
}_sp_line_st;

typedef struct _gt_spectrum_line_s{
    gt_obj_st obj;
    _gt_vector_st* sp_line;
    _gt_timer_st* timer;
    get_generator_next get_gen_next;

}_gt_spectrum_line_st;


/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_spectrum_line_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_spectrum_line_st)
};


/* macros ---------------------------------------------------------------*/


/* static functions -----------------------------------------------------*/
GT_ATTRIBUTE_RAM_TEXT static void
_draw_line(gt_obj_st * obj, _sp_line_st * sp_line)
{
    _gt_spectrum_line_st * style = (_gt_spectrum_line_st*)obj;

    gt_size_t count = _gt_vector_get_count(sp_line->sp_point);
    if(count < 2) {
        return ;
    }

    gt_size_t i = 0;
    gt_attr_curve_st bz_attr = { .line = sp_line->line};
    bz_attr.line.opa = obj->opa;

    gt_sp_point_st * point = (gt_sp_point_st*)_gt_vector_get_item(sp_line->sp_point, 0);
    bz_attr.p1.x = point->point.x;
    bz_attr.p1.y = style->get_gen_next(&point->gen);

    bz_attr.p2 = bz_attr.p1;

    point = (gt_sp_point_st*)_gt_vector_get_item(sp_line->sp_point, 1);
    bz_attr.p3.x = point->point.x;
    bz_attr.p3.y = style->get_gen_next(&point->gen);

    for(i = 0; i + 1 < count; ++i) {
        bz_attr.p0 = bz_attr.p1;
        bz_attr.p1 = bz_attr.p2;
        bz_attr.p2 = bz_attr.p3;

        if(i + 2 < count) {
            point = (gt_sp_point_st*)_gt_vector_get_item(sp_line->sp_point, i + 2);
            bz_attr.p3.x = point->point.x;
            bz_attr.p3.y = style->get_gen_next(&point->gen);
        }
        gt_draw_catmullrom(obj->draw_ctx, &bz_attr, &obj->area);
    }
}


GT_ATTRIBUTE_RAM_TEXT static void
_init_cb(gt_obj_st * obj)
{

    _gt_spectrum_line_st * style = (_gt_spectrum_line_st*)obj;

    gt_size_t line_count = _gt_vector_get_count(style->sp_line);
    if(line_count < 1) {
        return ;
    }

    gt_attr_curve_st bz_attr = { 0 };
    bz_attr.line.opa = obj->opa;
    bz_attr.line.color = gt_color_red();
    bz_attr.line.width = 1;
    bz_attr.line.brush = GT_BRUSH_TYPE_ROUND;
    bz_attr.line.type = GT_GRAPHS_TYPE_CURVE;

    for(gt_size_t line_index = 0; line_index < line_count; ++line_index)
    {
        _sp_line_st * sp_line = (_sp_line_st*)_gt_vector_get_item(style->sp_line, line_index);
        _draw_line( obj, sp_line);
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

GT_ATTRIBUTE_RAM_TEXT static void
_free_line_point(_sp_line_st * sp_line)
{
    if(!sp_line) {
        return ;
    }

    if(sp_line->sp_point) {
        _gt_vector_free(sp_line->sp_point);
        sp_line->sp_point = NULL;
    }
}

GT_ATTRIBUTE_RAM_TEXT static void
_free_line_vector(_gt_spectrum_line_st * style)
{
    if(style->sp_line) {
        return ;
    }

    uint16_t count = _gt_vector_get_count(style->sp_line);
    for(uint16_t i = 0; i < count; ++i) {
        _sp_line_st * item = (_sp_line_st*)_gt_vector_get_item(style->sp_line, i);
        if(item) {
            _free_line_point(item);
        }
    }
    _gt_vector_free(style->sp_line);
    style->sp_line = NULL;
}


static GT_ATTRIBUTE_RAM_TEXT void
_destroy_timer(_gt_spectrum_line_st * style) {
    if ( !style->timer ) {
        return ;
    }
    _gt_timer_del(style->timer);
    style->timer = NULL;
}

GT_ATTRIBUTE_RAM_TEXT static void
_deinit_cb(gt_obj_st * obj)
{
    _gt_spectrum_line_st * style = (_gt_spectrum_line_st*)obj;

    _destroy_timer(style);

    _free_line_vector(style);
}

static GT_ATTRIBUTE_RAM_TEXT bool _vector_free_cb(void * item) {
    if (NULL == item) {
        return false;
    }
    gt_mem_free(item);
    return true;
}

static GT_ATTRIBUTE_RAM_TEXT bool _vector_equal_cb(void * item, void * target) {
    return strcmp(item, target) ? false : true;
}


static GT_ATTRIBUTE_RAM_TEXT void
_auto_play_callback(struct _gt_timer_s * timer)
{
    gt_obj_st * obj = (gt_obj_st * )_gt_timer_get_user_data(timer);
	gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

/* global functions / API interface -------------------------------------*/
GT_ATTRIBUTE_RAM_TEXT gt_obj_st *
gt_spectrum_line_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    GT_CHECK_BACK_VAL(obj, NULL);

    _gt_spectrum_line_st * style = (_gt_spectrum_line_st*)obj;

    style->get_gen_next = gt_triangle_generator_next;

    style->sp_line = _gt_vector_create(_vector_free_cb, _vector_equal_cb);
    GT_CHECK_BACK_VAL(style->sp_line, NULL);

    style->timer = _gt_timer_create(_auto_play_callback, 200, (void * )obj);
    GT_CHECK_BACK_VAL(style->timer, NULL);

    _gt_timer_set_paused(style->timer, true);

    return obj;
}

GT_ATTRIBUTE_RAM_TEXT gt_size_t
gt_spectrum_line_add(gt_obj_st* obj, uint8_t width, gt_color_t color)
{
    _gt_spectrum_line_st * style = (_gt_spectrum_line_st*)obj;

    if(NULL == style->sp_line) {
        return -1;
    }

    _sp_line_st * item  = (_sp_line_st*)gt_mem_malloc(sizeof(_sp_line_st));
    if(NULL == item) {
        GT_LOGE(GT_LOG_TAG_GUI, "malloc line failed!!! size: %d", sizeof(_sp_line_st));
        return -1;
    }

    item->sp_point = _gt_vector_create(_vector_free_cb, _vector_equal_cb);
    if(NULL == item->sp_point) {
        gt_mem_free(item);
        GT_LOGE(GT_LOG_TAG_GUI, "malloc line point vector failed!!!");
        return -1;
    }

    item->line.brush = GT_BRUSH_TYPE_ROUND;
    item->line.width = width;
    item->line.color = color;
    item->line.type = GT_GRAPHS_TYPE_CURVE;

    _gt_vector_add_item(style->sp_line, (void*)item);

    return _gt_vector_get_count(style->sp_line) - 1;
}

GT_ATTRIBUTE_RAM_TEXT gt_size_t
gt_spectrum_line_add_point(gt_obj_st* obj, uint16_t line_index ,gt_sp_point_st sp_ping)
{

    _gt_spectrum_line_st * style = (_gt_spectrum_line_st*)obj;

    if(NULL == style->sp_line) {
        return -1;
    }

    _sp_line_st * item  = (_sp_line_st*)_gt_vector_get_item(style->sp_line, line_index);
    if(NULL == item) {
        GT_LOGE(GT_LOG_TAG_GUI, "line index %d not exist!!!", line_index);
        return -1;
    }

    if(NULL == item->sp_point) {
        return -1;
    }

    gt_sp_point_st * point = (gt_sp_point_st*)gt_mem_malloc(sizeof(gt_sp_point_st));
    if(NULL == point) {
        GT_LOGE(GT_LOG_TAG_GUI, "malloc sp point failed!!! size: %d", sizeof(gt_sp_point_st));
        return -1;
    }

    *point = sp_ping;
    _gt_vector_add_item(item->sp_point, (void*)point);

    return _gt_vector_get_count(item->sp_point) - 1;
}

GT_ATTRIBUTE_RAM_TEXT void
gt_spectrum_line_move_copy(gt_obj_st* obj, uint16_t count, gt_color_t start, gt_color_t end)
{
    if(count < 2) {
        return ;
    }

    _gt_spectrum_line_st * style = (_gt_spectrum_line_st*)obj;

    gt_size_t line_count = _gt_vector_get_count(style->sp_line);
    if(line_count < 1) {
        GT_LOGW(GT_LOG_TAG_GUI, "sp line count is %d", line_count);
        return ;
    }

    gt_gradient_color_st gradient = {
        .start = start,
        .end = end,
        .steps = count,
    };

    _sp_line_st * sp_line_1 = (_sp_line_st*)_gt_vector_get_item(style->sp_line, 0);
    gt_size_t point_count = _gt_vector_get_count(sp_line_1->sp_point);

    if(point_count < 2) {
        return ;
    }

    sp_line_1->line.color = gt_gradient_color_get(gradient, 0);
    gt_size_t i = 0;
    for( i = line_count; i < count; i++) {
        gt_spectrum_line_add(obj, sp_line_1->line.width, gt_color_black());
    }

    line_count = _gt_vector_get_count(style->sp_line);
    for(i = 1; i < line_count; i++) {
        _sp_line_st * sp_line = (_sp_line_st*)_gt_vector_get_item(style->sp_line, i);
        if(!sp_line) {
            continue;

        }
        sp_line->line.color = gt_gradient_color_get(gradient, i);
        _gt_vector_clear_all_items(sp_line->sp_point);



        for(gt_size_t j = 0; j < point_count; j++) {
            gt_sp_point_st * point = (gt_sp_point_st*)_gt_vector_get_item(sp_line_1->sp_point, j);

            gt_sp_point_st new_point = {
                .gen = point->gen,
                .point.x = point->point.x,
                .point.y = style->get_gen_next(&point->gen),
            };
            gt_spectrum_line_add_point(obj, i, new_point);
        }

    }

    for(gt_size_t j = 0; j < point_count; j++) {
        gt_sp_point_st * point = (gt_sp_point_st*)_gt_vector_get_item(sp_line_1->sp_point, j);
        point->gen.cur_val = point->point.y;
    }

    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

GT_ATTRIBUTE_RAM_TEXT void
gt_spectrum_line_set_period(gt_obj_st * obj, uint32_t period)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_spectrum_line_st * style = (_gt_spectrum_line_st *)obj;
    _gt_timer_set_period(style->timer, period);
}

GT_ATTRIBUTE_RAM_TEXT void
gt_spectrum_line_play(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_spectrum_line_st * style = (_gt_spectrum_line_st *)obj;
    _gt_timer_set_paused(style->timer, false);
}

GT_ATTRIBUTE_RAM_TEXT void
gt_spectrum_line_stop(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_spectrum_line_st * style = (_gt_spectrum_line_st *)obj;
    _gt_timer_set_paused(style->timer, true);
}

GT_ATTRIBUTE_RAM_TEXT void
gt_spectrum_line_set_gen_next_func(gt_obj_st * obj, get_generator_next func)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    if(NULL == func) {
        return ;
    }
    _gt_spectrum_line_st * style = (_gt_spectrum_line_st *)obj;
    style->get_gen_next = func;
}
/* end of file ----------------------------------------------------------*/
#endif // !GT_CFG_ENABLE_SPECTRUM_LINE

