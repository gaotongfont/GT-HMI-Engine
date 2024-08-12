/**
 * @file gt_stack.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-03-19 15:09:48
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_stack.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"


/* private define -------------------------------------------------------*/

/**
 * @brief The dummy size for cycle stack
 */
#define _CYCLE_STACK_DUMMY_SIZE     1

/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static inline void * _get_offset_by_top(gt_stack_st * stack, gt_stack_size_t top_ptr) {
    return (void * )((uint8_t * )stack->data + top_ptr * stack->item_size);
}

static inline gt_stack_size_t _get_item_top_by_index(gt_stack_st * stack, gt_stack_size_t index) {
    ++index;
    if (index > stack->count) {
        index = stack->count;
    }
    if (index > stack->top) {
        return stack->depth + stack->top + 1 - index;
    }
    return stack->top ? (stack->top - index) : stack->depth;
}

static inline bool _cycle_item_push(gt_stack_st * stack, void * item) {
    uint8_t * ptr = _get_offset_by_top(stack, stack->top);
    gt_memcpy(ptr, item, stack->item_size);

    gt_stack_size_t depth = stack->depth + _CYCLE_STACK_DUMMY_SIZE;
    gt_stack_size_t real_top = (stack->top + 1) % depth;
    if (real_top == stack->bottom) {
        stack->bottom = (stack->bottom + 1) % depth;
    }
    stack->top = real_top;
    if (stack->count < stack->depth) {
        ++stack->count;
    }
    return true;
}

static inline bool _item_push(gt_stack_st * stack, void * item) {
    if (gt_stack_is_full(stack)) {
        return false;
    }

    uint8_t * ptr = _get_offset_by_top(stack, stack->top);

    gt_memcpy(ptr, item, stack->item_size);

    ++stack->top;
    ++stack->count;
    return true;
}

static inline void * _item_pop(gt_stack_st * stack) {
    if (gt_stack_is_empty(stack)) {
        return NULL;
    }

    gt_stack_size_t real_top = _get_item_top_by_index(stack, 0);
    void * ptr = _get_offset_by_top(stack, real_top);

    stack->top = real_top;
    --stack->count;
    return ptr;
}

/* global functions / API interface -------------------------------------*/
gt_stack_st * gt_stack_create(gt_stack_size_t depth, uint16_t item_size, bool is_cycle_stack)
{
    if (depth <= 0 || 0 == item_size) {
        return NULL;
    }
    uint16_t size_byte = sizeof(gt_stack_st);
    uint16_t cycle_dummy_size = is_cycle_stack ? _CYCLE_STACK_DUMMY_SIZE : 0;
    gt_stack_st * ret = (gt_stack_st * )gt_mem_malloc(size_byte);
    if (NULL == ret) {
        return NULL;
    }
    gt_memset(ret, 0, size_byte);

    size_byte = (depth + cycle_dummy_size) * item_size;
    ret->data = (void **)gt_mem_malloc(size_byte);
    if (NULL == ret->data) {
        goto fail_lb;
    }
    gt_memset(ret->data, 0, size_byte);

    ret->depth = depth;
    ret->item_size = item_size;
    ret->reg.cycle = is_cycle_stack;
    return ret;

fail_lb:
    ret->depth = 0;
    ret->item_size = 0;
    ret->reg.cycle = false;
    gt_mem_free(ret);
    return NULL;
}

bool gt_stack_push(gt_stack_st * stack, void * item)
{
    if (NULL == stack || NULL == item) {
        return false;
    }

    if (stack->reg.cycle) {
        return _cycle_item_push(stack, item);
    }
    return _item_push(stack, item);
}

void * gt_stack_pop(gt_stack_st * stack)
{
    if (NULL == stack) { return NULL; }

    return _item_pop(stack);
}

void * gt_stack_peek(gt_stack_st * stack)
{
    return gt_stack_peek_by(stack, 0);
}

void * gt_stack_peek_by(gt_stack_st * stack, gt_stack_size_t index)
{
    if (NULL == stack) { return NULL; }
    if (gt_stack_is_empty(stack)) {
        return NULL;
    }
    gt_stack_size_t real_top = _get_item_top_by_index(stack, index);

    return _get_offset_by_top(stack, real_top);
}

bool gt_stack_is_empty(gt_stack_st * stack)
{
    if (NULL == stack) {
        return false;
    }
    return stack->count == 0 ? true : false;
}

bool gt_stack_is_full(gt_stack_st * stack)
{
    if (NULL == stack) {
        return false;
    }
    return stack->count == stack->depth ? true : false;
}

void gt_stack_clear(gt_stack_st * stack)
{
    if (NULL == stack) { return ; }

    stack->top = 0;
    stack->bottom = 0;
    stack->count = 0;
    gt_memset(stack->data, 0, stack->item_size * stack->depth);
}

gt_stack_size_t gt_stack_get_depth(gt_stack_st * stack)
{
    return stack ? stack->depth : 0;
}

gt_stack_size_t gt_stack_get_count(gt_stack_st * stack)
{
    return stack ? stack->count : 0;
}

uint16_t gt_stack_get_item_size(gt_stack_st * stack)
{
    return stack ? stack->item_size : 0;
}

bool gt_stack_is_cycle(gt_stack_st * stack)
{
    return stack ? stack->reg.cycle : false;
}

gt_stack_item_st gt_stack_search_item_from_top(gt_stack_st * stack, gt_stack_equal_cb_t equal_cb, void * target)
{
    gt_stack_item_st ret = {
        .index_from_top = -1,
        .data = NULL
    };
    if (NULL == stack || NULL == equal_cb || NULL == target) {
        return ret;
    }

    void * ptr = NULL;
    gt_stack_size_t top = stack->top;
    for (gt_stack_size_t i = 0; i < stack->count; ++i) {
        ptr = _get_offset_by_top(stack, top);
        if (equal_cb(ptr, target)) {
            ret.index_from_top = i;
            ret.data = ptr;
            break;
        }
        top = top ? (top - 1) : stack->depth;
    }
    return ret;
}

gt_stack_item_st gt_stack_search_item_from_bottom(gt_stack_st * stack, gt_stack_equal_cb_t equal_cb, void * target)
{
    gt_stack_item_st ret = {
        .index_from_top = -1,
        .data = NULL
    };
    if (NULL == stack || NULL == equal_cb || NULL == target) {
        return ret;
    }

    void * ptr = NULL;
    gt_stack_size_t bottom = stack->bottom;
    for (gt_stack_size_t i = 0; i < stack->count; ++i) {
        ptr = _get_offset_by_top(stack, bottom);
        if (equal_cb(ptr, target)) {
            ret.index_from_top = stack->count - i;
            ret.data = ptr;
            break;
        }
        bottom = (bottom + 1) % stack->depth;
    }
    return ret;
}

bool gt_stack_iterator(gt_stack_st * stack, gt_stack_iterator_cb_t it_cb, void * data)
{
    if (NULL == stack || NULL == it_cb) {
        return false;
    }
    gt_stack_item_st ptr = { 0 };
    gt_stack_size_t top = stack->top;

    for (gt_stack_size_t i = 0, cnt = stack->count; i < cnt; ++i) {
        ptr.index_from_top = i;
        ptr.data = _get_offset_by_top(stack, top);
        if (false == it_cb(ptr, data)) {
            return false;
        }
        top = top ? (top - 1) : stack->depth;
    }

    return true;
}

void gt_stack_destroy(gt_stack_st * stack)
{
    if (NULL == stack) { return ; }

    if (stack->data) {
        gt_mem_free(stack->data);
        stack->data = NULL;
    }

    gt_mem_free(stack);
    stack = NULL;
}

/* end ------------------------------------------------------------------*/
