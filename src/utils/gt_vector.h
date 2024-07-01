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

typedef struct _gt_vector_s {
    _gt_vector_free_item_cb_t free_item_cb;
    _gt_vector_equal_item_cb_t equal_item_cb;

    void ** list;
    uint8_t count;
    uint8_t index;
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
 * @param instance_size
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
 * @param item
 * @param size
 * @return true
 * @return false
 */
bool _gt_vector_add_item(_gt_vector_st * vector_p, void * item, uint16_t size);

/**
 * @brief Replace a item in vector
 *
 * @param vector_p
 * @param index The index of the item to be replaced
 * @param item The new item
 * @param size The size of the new item
 * @return true
 * @return false
 */
bool _gt_vector_replace_item(_gt_vector_st * vector_p, uint16_t index, void * item, uint16_t size);

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
 * @brief Get the item from vector
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
 * @param index
 * @return void*
 */
void * _gt_vector_get_item(_gt_vector_st const * vector, uint16_t index);

/**
 * @brief Free the whole vector
 *
 * @param vector_p
 * @return true
 * @return false
 */
bool _gt_vector_free(_gt_vector_st * vector_p);

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
