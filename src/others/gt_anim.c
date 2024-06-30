/**
 * @file gt_anim.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-06-21 17:59:29
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_anim.h"
#include "../core/gt_timer.h"
#include "../core/gt_mem.h"
#include "../core/gt_style.h"
#include "../hal/gt_hal_tick.h"
#include "../others/gt_gc.h"
#include "../others/gt_log.h"
#include "../others/gt_types.h"
#include "../others/gt_math.h"
#include "../widgets/gt_obj.h"
#include "../core/gt_obj_pos.h"


/* private define -------------------------------------------------------*/
#define _GT_ANIM_PATH_RESOLUTION    1024
#define _GT_ANIM_PATH_SHIFT         10


/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static _gt_timer_st * _anim_timer = NULL;
static uint32_t _time_last_run    = 0;
static bool _is_list_change       = false;
static bool _is_run_same_time     = false;
static gt_anim_st * _anim_act     = NULL;

/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static inline void _gt_anim_set_act(gt_anim_st * anim) {
    _anim_act = anim;
}

static inline gt_anim_st * _gt_anim_get_act(void) {
    return _anim_act;
}

static inline void _gt_anim_clr_act(void) {
    _anim_act = NULL;
}

static inline bool _gt_anim_free_data(gt_anim_st * anim) {
    if (!anim || !anim->data) {
        return false;
    }
    gt_mem_free(anim->data);
    anim->data = NULL;
    return true;
}

static inline _gt_timer_st * _gt_anim_get_timer(void) {
    return _anim_timer;
}

static inline void _gt_anim_set_timer(struct _gt_timer_s * timer) {
    _anim_timer = timer;
}

static void _gt_anim_change_list(void)
{
    _is_list_change = true;
    if (_gt_gc_is_ll_empty(&_GT_GC_GET_ROOT(_gt_anim_ll))) {
        _gt_timer_set_paused(_gt_anim_get_timer(), true);
    } else {
        _gt_timer_set_paused(_gt_anim_get_timer(), false);
    }
}

static void _gt_anim_free_task(gt_anim_st * anim) {
    _gt_list_del(&anim->list);
    _gt_anim_change_list();
    _gt_anim_free_data(anim);
    gt_mem_free(anim);
}

static int32_t _gt_anim_path_linear(const struct gt_anim_s * anim) {
    int32_t step = gt_map(anim->time_act, 0, anim->time, 0, _GT_ANIM_PATH_RESOLUTION);
    int32_t value = (anim->value_end - anim->value_start) * step;

    value >>= _GT_ANIM_PATH_SHIFT;
    value += anim->value_start;

    return value;
}

static inline int32_t _gt_anim_calc_path_by_bezier(const struct gt_anim_s * anim, uint32_t p0, uint32_t p1, uint32_t p2, uint32_t p3) {
    gt_math_bezier_st bezier = {
        .t = gt_map(anim->time_act, 0, anim->time, 0, _GT_ANIM_PATH_RESOLUTION),
        .p0 = p0, .p1 = p1, .p2 = p2, .p3 = p3
    };
    int32_t step = gt_bezier3(&bezier);

    int32_t value = (anim->value_end - anim->value_start) * step;
    value >>= GT_MATH_BEZIER_VAL_SHIFT;
    value += anim->value_start;

    return value;
}

static int32_t _gt_anim_path_ease_in(const struct gt_anim_s * anim) {
    return _gt_anim_calc_path_by_bezier(anim, 0, 50, 100, GT_MATH_BEZIER_VAL_RESOLUTION);
}

static int32_t _gt_anim_path_ease_out(const struct gt_anim_s * anim) {
    return _gt_anim_calc_path_by_bezier(anim, 0, 0, 600, GT_MATH_BEZIER_VAL_RESOLUTION);
}

static int32_t _gt_anim_path_ease_in_out(const struct gt_anim_s * anim) {
    return _gt_anim_calc_path_by_bezier(anim, 0, 50, 950, GT_MATH_BEZIER_VAL_RESOLUTION);
}

static int32_t _gt_anim_path_overshoot(const struct gt_anim_s * anim) {
    return _gt_anim_calc_path_by_bezier(anim, 0, 1000, 1300, GT_MATH_BEZIER_VAL_RESOLUTION);
}

static int32_t _gt_anim_path_bounce(const struct gt_anim_s * anim) {
    int32_t t = gt_map(anim->time_act, 0, anim->time, 0, _GT_ANIM_PATH_RESOLUTION);
    int32_t diff = anim->value_end - anim->value_start;

    /*3 bounces has 5 parts: 3 down and 2 up. One part is t / 5 long*/
    if (t < 408) {
        /* go down */
        t = (t * 2500) >> _GT_ANIM_PATH_SHIFT;
    }
    else if (t < 614) { /* [408, 614) */
        t -= 408;
        t *= 5;
        t = _GT_ANIM_PATH_RESOLUTION - t;
        diff /= 20;
    }
    else if (t < 819) { /* [614, 819) */
        /* fall back */
        t -= 614;
        t *= 5;     /* [0..1024] range */
        diff /= 20;
    }
    else if (t < 921) { /* [819, 921) */
        t -= 819;
        t *= 10;
        t = _GT_ANIM_PATH_RESOLUTION - t;
        diff /= 40;
    }
    else if (t < _GT_ANIM_PATH_RESOLUTION) {   /* [921, _GT_ANIM_PATH_RESOLUTION) */
        t -= 921;
        t *= 10;
        diff /= 40;
    }

    if (t > _GT_ANIM_PATH_RESOLUTION) { t = _GT_ANIM_PATH_RESOLUTION; }
    if (t < 0) { t = 0; }
    gt_math_bezier_st bezier = {
        .t = t,
        .p0 = GT_MATH_BEZIER_VAL_RESOLUTION,
        .p1 = 800,
        .p2 = 500,
        .p3 = 0
    };
    int32_t step = gt_bezier3(&bezier);

    int32_t value = step * diff;
    value >>= GT_MATH_BEZIER_VAL_SHIFT;

    return (anim->value_end - value);
}

static int32_t _gt_anim_path_step(const struct gt_anim_s * anim) {
    if (anim->time_act < anim->time) {
        return anim->value_start;
    }
    return anim->value_end;
}

static void _gt_anim_ready_handler(gt_anim_st * anim) {
    if (anim->repeat_count && !anim->invert && GT_ANIM_REPEAT_INFINITE != anim->repeat_count) {
        --anim->repeat_count;
    }

    if (!anim->repeat_count && (anim->invert || !anim->playback_time)) {
        if (anim->ready_cb) { anim->ready_cb(anim); }
        if (anim->deleted_cb) { anim->deleted_cb(anim); }
        _gt_anim_free_task(anim);
        return ;
    }

    /* playback handler */
    anim->time_act = -(int32_t)anim->repeat_delay;
    if (!anim->playback_time) {
        return ;
    }
    if (!anim->invert) {
        // FIXME
        anim->time_act = -(int32_t)anim->playback_delay;

        anim->invert = anim->invert ? 0 : 1;

        /* swap start and end value, and swap anim time */
        int32_t tmp = anim->value_start;
        anim->value_start = anim->value_end;
        anim->value_end   = tmp;

        tmp = anim->playback_time;
        anim->playback_time = anim->time;
        anim->time = tmp;
    }
}

static void _gt_anim_task_handler(struct _gt_timer_s * timer)
{
    GT_UNUSED(timer);
    if (_gt_gc_is_ll_empty(&_GT_GC_GET_ROOT(_gt_anim_ll))) {
        return ;
    }

    int32_t    new_value    = 0;
    uint32_t   time_gone    = gt_tick_elapse(_time_last_run);
    gt_anim_st * ptr        = NULL;
    gt_anim_st * backup_ptr = NULL;

    _is_run_same_time = _is_run_same_time ? false : true;
refresh_lb:
    _gt_list_for_each_entry_safe(ptr, backup_ptr, &_GT_GC_GET_ROOT(_gt_anim_ll), gt_anim_st, list) {
        _is_list_change = false;

        if (ptr->run_already == _is_run_same_time) {
            continue;
        }
        ptr->run_already = _is_run_same_time ? 1 : 0;

        if (ptr->paused) {
            ptr->tick_create = gt_tick_elapse(ptr->tick_create);
            continue;
        }
        if (ptr->time_delay_start > 0) {
            if (gt_tick_elapse(ptr->tick_create) < ptr->time_delay_start) {
                continue;
            }
            ptr->time_delay_start = 0;
        }

        _gt_anim_set_act(ptr);

        if (ptr->time_act <= 0 && ptr->start_cb) {
            ptr->start_cb(ptr);
        }

        ptr->time_act += time_gone;
        if (ptr->time_act < 0) {
            continue;
        }
        if (ptr->time_act > ptr->time) {
            ptr->time_act = ptr->time;
        }
        new_value = ptr->_path_cb(ptr);
        if (new_value != ptr->value_current) {
            ptr->value_current = new_value;
            if (ptr->exec_cb) {
                ptr->exec_cb(ptr->tar, ptr->value_current);
            }
        }
        if (ptr->time_act >= ptr->time) {
            _gt_anim_ready_handler(ptr);
        }

        if (_is_list_change) {
            break;
        }
    }

    if (_is_list_change) {
        _is_list_change = false;
        goto refresh_lb;
    }
    _gt_anim_clr_act();
    _time_last_run = gt_tick_get();
}

static void _default_exec_cb(void * obj, int32_t value) {
    gt_anim_st * anim = _gt_anim_get_act();
    gt_point_st * point = (gt_point_st * )gt_anim_get_data();

    gt_obj_set_pos((gt_obj_st * )obj,
        ((point[1].x - point[0].x) * anim->value_current / anim->value_end) + point[0].x,
        ((point[1].y - point[0].y) * anim->value_current / anim->value_end) + point[0].y);
}

/* global functions / API interface -------------------------------------*/

uint32_t gt_anim_speed_to_time(uint32_t speed, int32_t start, int32_t end)
{
    uint32_t d    = gt_abs(start - end);
    uint32_t time = (d * 1000) / speed;

    return time ? time : 1;
}

void * gt_anim_get_data(void)
{
    gt_anim_st * anim = _gt_anim_get_act();
    return anim->data;
}

void gt_anim_param_init(gt_anim_param_st * param)
{
    param->dst.x = 0;
    param->dst.y = 0;
    param->time_delay_start = 0;
    param->time_anim_exec = 500;
    param->type = GT_ANIM_PATH_TYPE_LINEAR;
}

void gt_anim_pos_move(gt_obj_st * obj, gt_anim_param_st * param)
{
    gt_anim_st anim;
    gt_anim_init(&anim);
    gt_anim_set_target(&anim, obj);
    gt_anim_set_value(&anim, 0, 100);
    gt_anim_set_path_type(&anim, param->type);
    gt_anim_set_time(&anim, param->time_anim_exec);
    gt_anim_set_time_delay_start(&anim, param->time_delay_start);

    gt_anim_set_exec_cb(&anim, _default_exec_cb);

    gt_point_st * point = gt_mem_malloc(sizeof(gt_point_st) << 1);
    point[0].x = obj->area.x;
    point[0].y = obj->area.y;
    point[1].x = param->dst.x;
    point[1].y = param->dst.y;
    gt_anim_set_data(&anim, (void * )point, sizeof(gt_point_st) << 1);
    gt_anim_start(&anim);
}

void _gt_anim_core_init(void)
{
    _GT_INIT_LIST_HEAD(&_GT_GC_GET_ROOT(_gt_anim_ll));

    _gt_anim_set_timer(_gt_timer_create(_gt_anim_task_handler, GT_TASK_PERIOD_TIME_ANIM, NULL));
    _gt_anim_change_list();
    _is_list_change = false;
}

void gt_anim_init(gt_anim_st * anim)
{
    if (!anim) { return ; }

    gt_memset_0(anim, sizeof(gt_anim_st));
    _GT_INIT_LIST_HEAD(&anim->list);
    anim->time_delay_start = 0;
    anim->time = 500;
    anim->time_act = 0;
    anim->value_start = 0;
    anim->value_end = 100;
    anim->_path_cb = _gt_anim_path_linear;
    anim->data = NULL;
}

gt_anim_st * gt_anim_start(const gt_anim_st * anim)
{
    if (!anim) { return NULL; }

    gt_anim_st * ptr        = NULL;
    gt_anim_st * backup_ptr = NULL;
    gt_anim_st * new_obj = (gt_anim_st * )gt_mem_malloc(sizeof(gt_anim_st));
    GT_CHECK_BACK_VAL(new_obj, NULL);

    if (_gt_gc_is_ll_empty(&_GT_GC_GET_ROOT(_gt_anim_ll))) {
        _time_last_run = gt_tick_get();
    }

    gt_memcpy(new_obj, anim, sizeof(gt_anim_st));
    new_obj->tick_create = gt_tick_get();

    if (new_obj->list.next != &new_obj->list || new_obj->list.prev != &new_obj->list) {
        _GT_INIT_LIST_HEAD(&new_obj->list);
    }

    // remove before the same exec callback
    _gt_list_for_each_entry_safe(ptr, backup_ptr, &_GT_GC_GET_ROOT(_gt_anim_ll), gt_anim_st, list) {
        if (new_obj->tar != ptr->tar) {
            continue;
        }
        if (new_obj->exec_cb != ptr->exec_cb) {
            continue;
        }
        if (ptr->deleted_cb) { ptr->deleted_cb(ptr); }
        _gt_anim_free_task(ptr);
    }

    _gt_list_add(&new_obj->list, &_GT_GC_GET_ROOT(_gt_anim_ll));
    _gt_anim_change_list();

    return new_obj;
}

void gt_anim_restart(gt_anim_st * anim)
{
    if (!anim) { return ; }
    anim->tick_create = gt_tick_get();
}

bool gt_anim_del(void const * const target, gt_anim_exec_cb_t exec_cb)
{
    bool ret = false;
    gt_anim_st * ptr        = NULL;
    gt_anim_st * backup_ptr = NULL;

    if (_gt_gc_is_ll_empty(&_GT_GC_GET_ROOT(_gt_anim_ll))) {
        return ret;
    }

    _gt_list_for_each_entry_safe(ptr, backup_ptr, &_GT_GC_GET_ROOT(_gt_anim_ll), gt_anim_st, list) {
        if ((ptr->tar == target || NULL == ptr->tar) && (ptr->exec_cb == exec_cb || NULL == ptr->exec_cb)) {
            if (ptr->deleted_cb) { ptr->deleted_cb(ptr); }
            _gt_anim_free_task(ptr);
            ret = true;
        }
    }

    return ret;
}

bool gt_anim_del_by(gt_anim_st * anim)
{
    bool ret = false;
    gt_anim_st * ptr        = NULL;
    gt_anim_st * backup_ptr = NULL;

    if (_gt_gc_is_ll_empty(&_GT_GC_GET_ROOT(_gt_anim_ll))) {
        return ret;
    }

    _gt_list_for_each_entry_safe(ptr, backup_ptr, &_GT_GC_GET_ROOT(_gt_anim_ll), gt_anim_st, list) {
        if (ptr == anim) {
            if (ptr->deleted_cb) { ptr->deleted_cb(ptr); }
            _gt_anim_free_task(ptr);
            ret = true;
        }
    }

    return ret;
}

void gt_anim_del_all(void)
{
    bool ret = false;
    gt_anim_st * ptr        = NULL;
    gt_anim_st * backup_ptr = NULL;

    if (_gt_gc_is_ll_empty(&_GT_GC_GET_ROOT(_gt_anim_ll))) {
        return ;
    }

    _gt_list_for_each_entry_safe(ptr, backup_ptr, &_GT_GC_GET_ROOT(_gt_anim_ll), gt_anim_st, list) {
        if (ptr) {
            if (ptr->deleted_cb) { ptr->deleted_cb(ptr); }
            _gt_anim_free_task(ptr);
            ret = true;
        }
    }

    if (ret) {
        _gt_anim_change_list();
    }
}

void gt_anim_set_path_type(gt_anim_st * anim, gt_anim_path_type_em type)
{
    switch(type) {
        case GT_ANIM_PATH_TYPE_EASE_IN: {
            anim->_path_cb = _gt_anim_path_ease_in;
            break;
        }
        case GT_ANIM_PATH_TYPE_EASE_OUT: {
            anim->_path_cb = _gt_anim_path_ease_out;
            break;
        }
        case GT_ANIM_PATH_TYPE_EASE_IN_OUT: {
            anim->_path_cb = _gt_anim_path_ease_in_out;
            break;
        }
        case GT_ANIM_PATH_TYPE_OVERSHOOT: {
            anim->_path_cb = _gt_anim_path_overshoot;
            break;
        }
        case GT_ANIM_PATH_TYPE_BOUNCE: {
            anim->_path_cb = _gt_anim_path_bounce;
            break;
        }
        case GT_ANIM_PATH_TYPE_STEP: {
            anim->_path_cb = _gt_anim_path_step;
        }
        case GT_ANIM_PATH_TYPE_LINEAR:
        default: {
            anim->_path_cb = _gt_anim_path_linear;
            break;
        }
    }
}


/* end ------------------------------------------------------------------*/
