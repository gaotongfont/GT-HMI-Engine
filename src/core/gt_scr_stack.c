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

#ifndef _GT_SCR_STACK_TRACE_LOG
    /**
     * @brief print all stack trace log
     *      [default]: 0
     */
    #define _GT_SCR_STACK_TRACE_LOG             0
#endif


/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
#if _GT_SCR_STACK_TRACE_LOG
static GT_ATTRIBUTE_RAM_TEXT bool
_print_stack_item_info_cb(gt_stack_item_st item, void * data) {
    gt_scr_stack_item_st * p = (gt_scr_stack_item_st * )item.data;
    if (0 == item.index_from_top) {
        return true;
    }
    GT_LOGD(GT_LOG_TAG_STACK,
        "stack info:\n"
        "  top idx: %d\n"
        "  scr  id: 0x%4x(%4d)  %p\n"
        "  time: %d, delay: %d, type: %d",
        item.index_from_top,
        p->current_scr_id, p->current_scr_id, p->current_scr,
        p->time, p->delay, p->anim_type);

    return true;
}
#endif

static GT_ATTRIBUTE_RAM_TEXT gt_scr_mapping_table_st * _get_scr_mapping_table(_gt_gc_scr_st * scr_info) {
    return &scr_info->mapping_table;
}

static GT_ATTRIBUTE_RAM_TEXT bool _equal_current_id_cb(void * item, void * target) {
    gt_scr_stack_item_st * it = (gt_scr_stack_item_st * )item;
    gt_scr_stack_item_st * tar_stack = (gt_scr_stack_item_st * )target;
    return it->current_scr_id == tar_stack->current_scr_id;
}

static GT_ATTRIBUTE_RAM_TEXT bool
_free_alive_scr_object_cb(gt_stack_item_st item, void * data) {
    gt_obj_st const * const active_scr = gt_disp_get_scr();
    gt_scr_stack_item_st * p = (gt_scr_stack_item_st * )item.data;
    _gt_scr_home_st const * const home = (_gt_scr_home_st * )data;

    if (NULL == p->current_scr) {
        return true;
    }
    if (active_scr == p->current_scr) {
        return true;
    }
    if (home->screen == p->current_scr) {
        if (home->alive) {
            return true;
        }
    }
    _gt_obj_class_destroy(p->current_scr);
    p->current_scr = NULL;
    return true;
}

static GT_ATTRIBUTE_RAM_TEXT bool
_remove_scr_pointer_it_cb(gt_stack_item_st item, void * data) {
    gt_scr_stack_item_st * p = (gt_scr_stack_item_st * )item.data;
    gt_obj_st const * const remove_scr = (gt_obj_st const * const)data;
    if (NULL == remove_scr) { return true; }

    if (remove_scr == p->current_scr) {
        p->current_scr = NULL;
    }
    return true;
}

/**
 * @brief Try to release pop remark pointer and list space.
 *
 * @param scr_info
 * @param active_scr The current screen object
 * @return gt_res_t GT_ERS_OK: Have empty space to record screen ptr;
 *                  GT_RES_FAIL: Have no enough space to record.
 */
static gt_res_t GT_ATTRIBUTE_RAM_TEXT
_release_all_pop_remark_except_active_scr(_gt_gc_scr_st * scr_info, gt_obj_st const * const active_scr) {
    gt_obj_st * pop_scr = NULL;
    gt_obj_st * temp_scr = NULL;
    gt_stack_size_t i = scr_info->pop_remark.cnt - 1;
    gt_scr_id_t stack_idx = 0, stack_count = gt_scr_stack_get_count();
    bool is_alive = false;

    while (i >= 0) {
        is_alive = false;
        pop_scr = scr_info->pop_remark.pop_scr_list[i];
        for (stack_idx = 0; stack_idx < stack_count; stack_idx++) {
            temp_scr = gt_scr_stack_get_prev_scr_by(stack_idx);
            if (pop_scr == temp_scr) {
                is_alive = true;
                break;
            }
        }
        if (false == is_alive) {
            if (pop_scr == active_scr) {
                --i;
                continue;
            }
            gt_obj_destroy_immediately(pop_scr);
            scr_info->pop_remark.pop_scr_list[i] = NULL;
            if (i + 1 < scr_info->pop_remark.cnt) {
                gt_memmove(&scr_info->pop_remark.pop_scr_list[i],
                    &scr_info->pop_remark.pop_scr_list[i + 1],
                    sizeof(gt_obj_st *) * (scr_info->pop_remark.cnt - i - 1));
            }
            --scr_info->pop_remark.cnt;
        }
        --i;
    }
    return scr_info->pop_remark.cnt + 1 < scr_info->stack->depth ? GT_RES_OK : GT_RES_FAIL;
}

/**
 * @brief
 *
 * @param scr_info
 * @param pop_item Just pop_item->current_scr vaild.
 * @return gt_res_t GT_ERS_OK: Have empty space to record screen ptr;
 *                  GT_RES_FAIL: Have no enough space to record.
 */
static gt_res_t GT_ATTRIBUTE_RAM_TEXT
_gt_scr_stack_add_pop_remark(_gt_gc_scr_st * scr_info, gt_scr_stack_item_st * pop_item) {
    gt_obj_st * active_scr = gt_disp_get_scr();
    if (NULL == scr_info) {
        return GT_RES_INV;
    }
    if (scr_info->pop_remark.cnt >= scr_info->stack->depth) {
        /** Free unused screen */
        if (GT_RES_FAIL == _release_all_pop_remark_except_active_scr(scr_info, active_scr)) {
            GT_LOGE(GT_LOG_TAG_GUI, "Release all pop screen failed!");
            return GT_RES_FAIL;
        }
    }
    if (NULL == pop_item->current_scr) {
        return GT_RES_OK;
    }
#if GT_USE_LAYER_TOP
    gt_obj_st const * const top_scr = gt_disp_get_layer_top();
    if (top_scr && top_scr == pop_item->current_scr) {
        /** Do not free layer top memory. */
        return GT_RES_OK;
    }
#endif
    for (int16_t i = 0, cnt = scr_info->pop_remark.cnt; i < cnt; i++) {
        if (pop_item->current_scr == scr_info->pop_remark.pop_scr_list[i]) {
            /** has before */
            return GT_RES_OK;
        }
    }
    scr_info->pop_remark.pop_scr_list[scr_info->pop_remark.cnt] = pop_item->current_scr;
    ++scr_info->pop_remark.cnt;
    return GT_RES_OK;
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

gt_scr_id_t gt_scr_stack_get_current_id(void)
{
    gt_scr_stack_item_st * item = gt_scr_stack_peek();
    if (NULL == item) {
        return -1;
    }
    return item->current_scr_id;
}

bool gt_scr_stack_is_home_scr_alive(void)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();
    return scr_info->home_scr.alive;
}

bool _gt_scr_stack_has_pop_before(gt_obj_st const * const scr)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();
    if (NULL == scr) {
        return false;
    }
    if (NULL == scr_info) {
        return false;
    }
    for (int16_t i = 0, cnt = scr_info->pop_remark.cnt; i < cnt; i++) {
        if (scr == scr_info->pop_remark.pop_scr_list[i]) {
            return true;
        }
    }
    return false;
}

gt_res_t _gt_scr_stack_remove_pop_scr(gt_obj_st const * const scr)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();
    if (NULL == scr) {
        return GT_RES_FAIL;
    }
    if (NULL == scr_info) {
        return GT_RES_FAIL;
    }
    for (int16_t i = 0, cnt = scr_info->pop_remark.cnt; i < cnt; i++) {
        if (scr != scr_info->pop_remark.pop_scr_list[i]) {
            continue;
        }
        if (i + 1 < scr_info->pop_remark.cnt) {
            gt_memmove(&scr_info->pop_remark.pop_scr_list[i], &scr_info->pop_remark.pop_scr_list[i + 1],
                sizeof(gt_obj_st *) * (scr_info->pop_remark.cnt - i - 1));
        }
        --scr_info->pop_remark.cnt;
        break;
    }

    gt_stack_iterator(scr_info->stack, _remove_scr_pointer_it_cb, (void *)scr);
    if (scr == scr_info->home_scr.screen) {
        scr_info->home_scr.screen = NULL;
    }
    return GT_RES_OK;
}

gt_res_t _gt_scr_stack_clean_unused_pop_scrs(gt_obj_st const * const active_scr)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();
    return _release_all_pop_remark_except_active_scr(scr_info, active_scr);
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
    if (NULL == scr_info->pop_remark.pop_scr_list) {
        scr_info->pop_remark.pop_scr_list = gt_mem_malloc(sizeof(gt_obj_st *) * stack_depth);
    } else {
        scr_info->pop_remark.pop_scr_list = gt_mem_realloc(scr_info->pop_remark.pop_scr_list,
            sizeof(gt_obj_st *) * stack_depth);
    }
    if (NULL == scr_info->pop_remark.pop_scr_list) {
        GT_LOGE(GT_LOG_TAG_GUI, "Pop screen list malloc memory failed!");
        gt_stack_destroy(scr_info->stack);
        scr_info->stack = NULL;
        return false;
    }
    return true;
}

bool gt_scr_stack_push(gt_scr_stack_item_st * scr)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();

#if _GT_SCR_STACK_TRACE_LOG
    GT_LOGD("", "scr before: %p", scr->current_scr);
    bool is_ok = gt_stack_push(scr_info->stack, (void * )scr);
    GT_LOGD("", "scr: %p", scr->current_scr);
    if (is_ok) {
        gt_stack_iterator(scr_info->stack, _print_stack_item_info_cb, NULL);
    }
    return is_ok;
#else
    return gt_stack_push(scr_info->stack, (void * )scr);
#endif
}

gt_scr_stack_item_st * gt_scr_stack_pop(gt_stack_size_t count)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();
    gt_scr_stack_item_st * item = NULL;
    gt_scr_id_t home_id = gt_scr_stack_get_home_scr_id();
    bool is_home_alive = gt_scr_stack_is_home_scr_alive();

    if (count < 0) {
        return item;
    }
    if (count > gt_stack_get_count(scr_info->stack)) {
        count = gt_stack_get_count(scr_info->stack);
    }

    if (count) {
        /** top stack do not free here, need to free by load scr anim */
        item = (gt_scr_stack_item_st * )gt_stack_pop(scr_info->stack);
        if (home_id != item->current_scr_id || false == is_home_alive) {
            _gt_scr_stack_add_pop_remark(scr_info, item);
        }
    }
    while (--count > 0) {
        item = (gt_scr_stack_item_st * )gt_stack_pop(scr_info->stack);
        if (count > 0 && item && item->current_scr) {
            /** Free middle screen, keep top and reload one */
            if (home_id != item->current_scr_id || false == is_home_alive) {
                item->current_scr->using_sta = false;
                _gt_obj_class_destroy(item->current_scr);
                item->current_scr = NULL;
            }
        }
    }
#if _GT_SCR_STACK_TRACE_LOG
    gt_stack_iterator(scr_info->stack, _print_stack_item_info_cb, NULL);
#endif
    return item;
}

gt_scr_stack_item_st * gt_scr_stack_peek(void)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();
    if (NULL == scr_info) {
        return NULL;
    }
    return (gt_scr_stack_item_st * )gt_stack_peek(scr_info->stack);
}

gt_scr_stack_item_st * gt_scr_stack_peek_by(gt_stack_size_t step)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();
    if (NULL == scr_info) {
        return NULL;
    }
    return (gt_scr_stack_item_st * )gt_stack_peek_by(scr_info->stack, step);
}

gt_obj_st * gt_scr_stack_get_prev_scr(void)
{
    return gt_scr_stack_get_prev_scr_by(1);
}

bool gt_scr_stack_is_prev(gt_scr_id_t prev_scr_id)
{
    return gt_scr_stack_get_prev_id_by(1) == prev_scr_id;
}

bool gt_scr_stack_is_current(gt_scr_id_t scr_id)
{
    return gt_scr_stack_get_prev_id_by(0) == scr_id;
}

gt_scr_id_t gt_scr_stack_get_prev_id(void)
{
    return gt_scr_stack_get_prev_id_by(1);
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

void gt_scr_stack_clear(void)
{
    _gt_gc_scr_st * scr_info = _gt_gc_get_scr_info();

    // Clear screen object
    gt_stack_iterator(scr_info->stack, _free_alive_scr_object_cb, (void * )&scr_info->home_scr);

    gt_stack_clear(scr_info->stack);
}


/* end ------------------------------------------------------------------*/
