/**
 * @file gt_stack.h
 * @author Feyoung
 * @brief The stack / cycle stack module
 * @version 0.1
 * @date 2024-03-19 15:09:39
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_STACK_H_
#define _GT_STACK_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../others/gt_types.h"



/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/

typedef struct _stack_reg_s {
    uint8_t cycle : 1;                  /** The flag to indicate whether the stack is cyclic */
    uint8_t reserved : 7;
}_stack_reg_st;

typedef struct gt_stack_s {
    _stack_reg_st reg;
    gt_stack_size_t top;    /** The index of the top item */
    gt_stack_size_t bottom; /** The index of the bottom item, such as cycle mode */
    gt_stack_size_t count;  /** The number of items in the stack */
    gt_stack_size_t depth;  /** The maximum number of items the stack can hold */
    uint16_t item_size;     /** The number of bytes the item occupies in memory */
    void ** data;           /** The pointer to the stack */
}gt_stack_st;

typedef struct gt_stack_ret_s {
    gt_stack_size_t index_from_top;    /** The index of the item from the top */
    void * data;
}gt_stack_item_st;

/**
 * @brief Compare the item with the data
 * @param item The pointer to the item
 * @param target The object data which is used to compare with the item
 * @return  true The item is equal to the data
 *          false The item is not equal to the data
 */
typedef bool ( * gt_stack_equal_cb_t)(void * , void *);

/**
 * @brief The iterator callback function
 * @param item The item
 * @param data The object data or user data
 */
typedef bool ( * gt_stack_iterator_cb_t)(gt_stack_item_st , void * );

/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief Normal stack
 *
 * @param depth The maximum number of items the stack can hold
 * @param item_size The number of bytes the item occupies in memory
 * @return gt_stack_st*
 */
gt_stack_st * gt_stack_create(gt_stack_size_t depth, uint16_t item_size);

/**
 * @brief Cycle stack
 *
 * @param depth The maximum number of items the stack can hold
 * @param item_size The number of bytes the item occupies in memory
 * @return gt_stack_st*
 */
gt_stack_st * gt_stack_cycle_create(gt_stack_size_t depth, uint16_t item_size);

bool gt_stack_push(gt_stack_st * stack, void * item);

void * gt_stack_pop(gt_stack_st * stack);

void * gt_stack_peek(gt_stack_st * stack);

void * gt_stack_peek_by(gt_stack_st * stack, gt_stack_size_t index);

bool gt_stack_is_empty(gt_stack_st * stack);

bool gt_stack_is_full(gt_stack_st * stack);

void gt_stack_clear(gt_stack_st * stack);

gt_stack_size_t gt_stack_get_depth(gt_stack_st * stack);

gt_stack_size_t gt_stack_get_count(gt_stack_st * stack);

uint16_t gt_stack_get_item_size(gt_stack_st * stack);

bool gt_stack_is_cycle(gt_stack_st * stack);

/**
 * @brief Search the item from the stack top
 *
 * @param stack
 * @param equal_cb
 * @param target
 * @return gt_stack_item_st ret.data is NULL or ret.index_from_top is -1, if not found
 */
gt_stack_item_st gt_stack_search_item_from_top(gt_stack_st * stack, gt_stack_equal_cb_t equal_cb, void * target);

/**
 * @brief Search the item from the stack bottom
 *
 * @param stack
 * @param equal_cb
 * @param target
 * @return gt_stack_item_st ret.data is NULL or ret.index_from_top is -1, if not found
 */
gt_stack_item_st gt_stack_search_item_from_bottom(gt_stack_st * stack, gt_stack_equal_cb_t equal_cb, void * target);

bool gt_stack_iterator(gt_stack_st * stack, gt_stack_iterator_cb_t it_cb, void * data);

void gt_stack_destroy(gt_stack_st * stack);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_STACK_H_
