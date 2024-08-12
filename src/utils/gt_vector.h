/**
 * @file gt_vector.h
 * @author Feyoung
 * @brief widget extension properties vector
 * @version 0.1
 * @date 2023-05-23 19:59:57
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_VECTOR_H_
#define _GT_VECTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../others/gt_types.h"
#include "../others/gt_list.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/

/**
 * @brief Tell Engine how to free the item in vector
 * @param item The item to be freed
 * @returns true: success, false: failed
 */
typedef bool ( * _gt_vector_free_item_cb_t)(void * item);

/**
 * @brief Tell Engine how to search the target item in vector list
 * @param item The item that in order from the list
 * @param target The target content to be searched
 * @returns -1: not found, >=0: found, and the index of the item
 */
typedef bool ( * _gt_vector_equal_item_cb_t)(void * item, void * target);

typedef struct _gt_vector_s _iter_dsc_st;

/**
 * @brief Has next item in vector iterator
 * @param vct_dsc The dsc object from @see _gt_vector_iterator_st
 */
typedef bool ( * has_next_cb)(_iter_dsc_st * vct_dsc);

/**
 * @brief Get the next item from vector iterator
 * @param vct_dsc The dsc object from @see _gt_vector_iterator_st
 */
typedef void * ( * next_cb)(_iter_dsc_st * vct_dsc);

/**
 * @brief Get the index of the item from vector iterator, when "iter.next()" is
 *       called, the index will be increased.
 * @param vct_dsc The dsc object from @see _gt_vector_iterator_st
 */
typedef gt_size_t ( * index_cb)(_iter_dsc_st * vct_dsc);

typedef struct {
    struct _gt_list_head node;
    void * val;
}_gt_vector_item_st;

typedef struct _gt_vector_iterator_s {
    has_next_cb has_next;
    next_cb next;
    index_cb index;
    _iter_dsc_st * dsc_t;
}_gt_vector_iterator_st;

typedef struct _gt_vector_iterator_ctl_s {
    _gt_vector_item_st * item_p;
    gt_size_t idx;
}_gt_vector_iterator_ctl_st;

typedef struct _gt_vector_s {
    struct _gt_list_head list_head;

    _gt_vector_free_item_cb_t free_item_cb;
    _gt_vector_equal_item_cb_t equal_item_cb;

    gt_size_t count;
    gt_size_t index;

    _gt_vector_iterator_ctl_st * iter_ctl;
}_gt_vector_st;



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief Create a vector object
 *
 * @param free_cb To free memory inside the project
 * @param equal_item_cb To compare the same item
 * @return _gt_vector_st* default object
 */
_gt_vector_st * _gt_vector_create(_gt_vector_free_item_cb_t free_cb, _gt_vector_equal_item_cb_t equal_item_cb);

/**
 * @brief A callback to free custom content memory
 *
 * @param vector_p
 * @param free_item_cb
 * @return true
 * @return false
 */
bool _gt_vector_add_free_item_cb(_gt_vector_st * vector_p, _gt_vector_free_item_cb_t free_item_cb);

/**
 * @brief Find a way to compare the same item
 *
 * @param vector_p
 * @param equal_item_cb
 * @return true
 * @return false
 */
bool _gt_vector_add_equal_item_cb(_gt_vector_st * vector_p, _gt_vector_equal_item_cb_t equal_item_cb);

/**
 * @brief Add a item to vector
 *
 * @param vector_p
 * @param item The point of item, the item must be malloced, free by free_cb()
 * @return true
 * @return false
 */
bool _gt_vector_add_item(_gt_vector_st * vector_p, void * item);

/**
 * @brief Replace a item in vector
 *
 * @param vector_p
 * @param index The index of the item to be replaced
 * @param item The point of item, the item must be malloced, free by free_cb()
 * @return true
 * @return false
 */
bool _gt_vector_replace_item(_gt_vector_st * vector_p, uint16_t index, void * item);

/**
 * @brief Remove a item from vector
 *
 * @param vector_p
 * @param target
 * @return true
 * @return false
 */
bool _gt_vector_remove_item(_gt_vector_st * vector_p, void * target);

/**
 * @brief Get the prev item from vector, index will be decreased
 *
 * @param vector
 * @return void*
 */
void * _gt_vector_turn_prev(_gt_vector_st * vector);

/**
 * @brief Get next the item from vector, index will be increased
 *
 * @param vector
 * @return void*
 */
void * _gt_vector_turn_next(_gt_vector_st * vector);

/**
 * @brief Get the count of items in vector
 *
 * @param vector
 * @return gt_size_t -1: failed to get the count, >=0: the count of items
 */
gt_size_t _gt_vector_get_count(_gt_vector_st const * vector);

/**
 * @brief Set the index of the item in vector
 *
 * @param vector
 * @param index -1: last one; >=0: index number
 * @return gt_size_t -1: failed to set the index, >=0: the index of the item
 */
gt_size_t _gt_vector_set_index(_gt_vector_st * vector, gt_size_t index);

/**
 * @brief Get current the item index from vector
 *
 * @param vector
 * @return gt_size_t -1: failed to find the item, >=0: the index of the item
 */
gt_size_t _gt_vector_get_index(_gt_vector_st const * vector);

/**
 * @brief Get the item from vector
 *
 * @param vector
 * @param index The index of the item, -1: failed; >=0: the index of the item
 * @return void*
 */
void * _gt_vector_get_item(_gt_vector_st const * vector, gt_size_t index);

/**
 * @brief Is the current index is the tail one
 *
 * @param vector
 * @return true The current index is the tail one
 * @return false
 */
bool _gt_vector_is_tail_index_now(_gt_vector_st const * vector);

/**
 * @brief Free the whole vector
 *
 * @param vector_p
 * @return true
 * @return false
 */
bool _gt_vector_free(_gt_vector_st * vector_p);

/**
 * @brief Get the iterator of the vector, detail @see _gt_vector_iterator_st:
 *      use:    _gt_vector_iterator_st iter = _gt_vector_get_iterator(vector);
 *              while (iter.has_next(iter.dsc_t)) {
 *                  void * item = iter.next(iter.dsc_t);
 *                  // do something with item
 *              }
 *
 * @param vector
 * @return _gt_vector_iterator_st
 */
_gt_vector_iterator_st _gt_vector_get_iterator(_gt_vector_st * vector);

/**
 * @brief Just clear the vector list, but not free the vector
 *
 * @param vector
 * @return true
 * @return false
 */
bool _gt_vector_clear_all_items(_gt_vector_st * vector);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_VECTOR_H_
