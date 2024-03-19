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
typedef struct _reg_s {
    uint8_t deepest_alive_forever : 1;  /** The flag to indicate whether the deepest item is alive forever */
    uint8_t cycle : 1;                  /** The flag to indicate whether the stack is cyclic */
    uint8_t reserved : 6;
}_reg_st;

typedef struct gt_stack_s {
    _reg_st reg;
    gt_stack_size_t top;    /** The index of the top item */
    gt_stack_size_t bottom; /** The index of the bottom item, such as cycle mode */
    gt_stack_size_t count;  /** The number of items in the stack */
    gt_stack_size_t depth;  /** The maximum number of items the stack can hold */
    uint16_t item_size;     /** The number of bytes the item occupies in memory */
    void ** data;           /** The pointer to the stack */
}gt_stack_st;


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

/**
 * @brief Set the deepest alive forever, only valid for normal stack
 *
 * @param stack
 * @param deepest_alive_forever
 * @return true Set the deepest alive forever successfully
 * @return false Set the deepest alive forever failed
 */
bool gt_stack_set_deepest_alive_forever(gt_stack_st * stack, bool deepest_alive_forever);

bool gt_stack_push(gt_stack_st * stack, void * item);

void * gt_stack_pop(gt_stack_st * stack);

void * gt_stack_peek(gt_stack_st * stack);

bool gt_stack_is_empty(gt_stack_st * stack);

bool gt_stack_is_full(gt_stack_st * stack);

void gt_stack_clear(gt_stack_st * stack);

gt_stack_size_t gt_stack_get_depth(gt_stack_st * stack);

gt_stack_size_t gt_stack_get_count(gt_stack_st * stack);

uint16_t gt_stack_get_item_size(gt_stack_st * stack);

bool gt_stack_is_deepest_alive_forever(gt_stack_st * stack);

bool gt_stack_is_cycle(gt_stack_st * stack);

void gt_stack_destroy(gt_stack_st * stack);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_STACK_H_
