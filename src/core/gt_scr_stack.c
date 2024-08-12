/**
 * @file gt_scr_stack.c
 * @author Feyoung
 * @brief The screen stack
 * @version 0.1
 * @date 2024-03-20 10:28:28
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_scr_stack.h"
#include "../others/gt_log.h"
#include "../others/gt_gc.h"
#include "../core/gt_mem.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_scr_mapping_table_st * _get_scr_mapping_table(_gt_gc_scr_st * scr_info) {
    return &scr_info->mapping_table;
}

static bool _equal_current_id_cb(void * item, void * target) {
    gt_scr_stack_item_st * it = (gt_scr_stack_item_st * )item;
    gt_scr_stack_item_st * tar_stack = (gt_scr_stack_item_st * )target;
    return it->current_scr_id == tar_stack->current_scr_id;
}

/* global functions / API interface -------------------------------------*/
gt_scr_init_func_cb_t gt_scr_stack_get_init_func(gt_scr_id_t scr_id)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();
    gt_scr_mapping_table_st * mapping = _get_scr_mapping_table(scr_info);

    for (gt_size_t i = 0, cnt = mapping->count; i < cnt; i++) {
        if (scr_id == mapping->scr_list[i].scr_id) {
            return mapping->scr_list[i].init_cb;
        }
    }
    return NULL;
}

bool gt_scr_stack_register_id_list(gt_scr_list_st const * const scr_list, gt_scr_id_t count, gt_stack_size_t stack_depth)
{
    if (NULL == scr_list || 0 == count) {
        return false;
    }
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();
    gt_scr_mapping_table_st * mapping = _get_scr_mapping_table(scr_info);
    mapping->scr_list = scr_list;
    mapping->count = count;

    return gt_scr_stack_set_depth(stack_depth);
}

bool gt_scr_stack_set_home_scr_id(gt_scr_id_t scr_id, bool alive_forever)
{
    if (scr_id < -1) {
        return false;
    }
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();
    scr_info->home_scr.id = scr_id;
    scr_info->home_scr.alive = alive_forever;

    return true;
}

void gt_scr_stack_set_home_scr(gt_obj_st * scr)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();
    scr_info->home_scr.screen = scr;
}

gt_obj_st * gt_scr_stack_get_home_scr(void)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();
    return scr_info->home_scr.screen;
}

gt_scr_id_t gt_scr_stack_get_home_scr_id(void)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();
    if (scr_info->home_scr.id < 0) {
        return -1;
    }
    return scr_info->home_scr.id;
}

bool gt_scr_stack_is_home_scr_alive(void)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();
    return scr_info->home_scr.alive;
}

bool gt_scr_stack_set_depth(gt_stack_size_t stack_depth)
{
    if (stack_depth <= 0) {
        return false;
    }
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();
    if (scr_info->stack) {
        gt_stack_destroy(scr_info->stack);
        scr_info->stack = NULL;
    }
    scr_info->stack = gt_stack_create(stack_depth, sizeof(gt_scr_stack_item_st), false);
    GT_CHECK_BACK_VAL(scr_info->stack, false);

    return true;
}

bool gt_scr_stack_push(gt_scr_stack_item_st * scr)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();

    return gt_stack_push(scr_info->stack, (void * )scr);
}

gt_scr_stack_item_st * gt_scr_stack_pop(gt_stack_size_t count)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();
    gt_scr_stack_item_st * item = NULL;
    gt_obj_st * prev = NULL;
    gt_scr_id_t home_id = gt_scr_stack_get_home_scr_id();

    if (count < 0) {
        return item;
    }

    if (count > gt_stack_get_count(scr_info->stack)) {
        count = gt_stack_get_count(scr_info->stack);
    }

    if (count) {
        /** top stack do not free here, need to free by load scr anim */
        item = (gt_scr_stack_item_st * )gt_stack_pop(scr_info->stack);
        --count;
    }

    for (gt_size_t i = 0; i < count; i++) {
        if (item && item->prev_scr_alive) {
            item->prev_scr_alive->using_sta = false;
            _gt_obj_class_destroy(item->prev_scr_alive);
            item->prev_scr_alive = NULL;
            prev = item->prev_scr_alive;
        }
        item = (gt_scr_stack_item_st * )gt_stack_pop(scr_info->stack);
        if (home_id != item->current_scr_id || false == gt_scr_stack_is_home_scr_alive()) {
            item->current_scr = NULL;
        }
    }

    return item;
}

gt_scr_stack_item_st * gt_scr_stack_peek(void)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();

    return (gt_scr_stack_item_st * )gt_stack_peek(scr_info->stack);
}

gt_obj_st * gt_scr_stack_get_prev_scr(void)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();

    gt_scr_stack_item_st * item = (gt_scr_stack_item_st * )gt_stack_peek(scr_info->stack);
    GT_CHECK_BACK_VAL(item, NULL);
    if (item->prev_scr_id == gt_scr_stack_get_home_scr_id() && gt_scr_stack_is_home_scr_alive()) {
        return gt_scr_stack_get_home_scr();
    }
    return item->prev_scr_alive;
}

bool gt_scr_stack_is_prev(gt_scr_id_t prev_scr_id)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();

    gt_scr_stack_item_st * item = (gt_scr_stack_item_st * )gt_stack_peek(scr_info->stack);
    GT_CHECK_BACK_VAL(item, false);
    return item->prev_scr_id == prev_scr_id;
}

bool gt_scr_stack_is_current(gt_scr_id_t scr_id)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();

    gt_scr_stack_item_st * item = (gt_scr_stack_item_st * )gt_stack_peek(scr_info->stack);
    GT_CHECK_BACK_VAL(item, false);
    return item->current_scr_id == scr_id;
}

gt_scr_id_t gt_scr_stack_get_prev_id(void)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();

    gt_scr_stack_item_st * item = (gt_scr_stack_item_st * )gt_stack_peek(scr_info->stack);
    GT_CHECK_BACK_VAL(item, -1);
    return item->prev_scr_id;
}

gt_scr_id_t gt_scr_stack_get_prev_id_by(gt_stack_size_t step)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();
    gt_scr_stack_item_st * item = (gt_scr_stack_item_st * )gt_stack_peek_by(scr_info->stack, step);

    return item ? item->current_scr_id : -1;
}

gt_obj_st * gt_scr_stack_get_prev_scr_by(gt_stack_size_t step)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();
    gt_scr_stack_item_st * item = (gt_scr_stack_item_st * )gt_stack_peek_by(scr_info->stack, step);
    GT_CHECK_BACK_VAL(item, NULL);
    if (item->current_scr_id == gt_scr_stack_get_home_scr_id() && gt_scr_stack_is_home_scr_alive()) {
        return gt_scr_stack_get_home_scr();
    }
    return item->current_scr;
}

gt_stack_item_st gt_scr_stack_has_before(gt_scr_stack_item_st * scr)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();

    return gt_stack_search_item_from_bottom(scr_info->stack, _equal_current_id_cb, scr);
}

bool gt_scr_stack_is_empty(void)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();

    return gt_stack_is_empty(scr_info->stack);
}

bool gt_scr_stack_is_full(void)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();

    return gt_stack_is_full(scr_info->stack);
}

gt_stack_size_t gt_scr_stack_get_count(void)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();

    return gt_stack_get_count(scr_info->stack);
}

static bool _free_alive_scr_object_cb(gt_stack_item_st item, void * data) {
    gt_scr_stack_item_st * p = (gt_scr_stack_item_st * )item.data;
    _gt_scr_home_st * home = (_gt_scr_home_st * )data;

    if (p->prev_scr_alive == home->screen) {
        if (home->alive) {
            return true;
        }
    }
    GT_CHECK_BACK_VAL(p->prev_scr_alive, true);
    _gt_obj_class_destroy(p->prev_scr_alive);
    p->prev_scr_alive = NULL;
    return true;
}

void gt_scr_stack_clear(void)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();

    // Clear screen object
    gt_stack_iterator(scr_info->stack, _free_alive_scr_object_cb, (void * )&scr_info->home_scr);

    gt_stack_clear(scr_info->stack);
}


/* end ------------------------------------------------------------------*/
