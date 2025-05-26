/**
 * @file gt_circular_array.h
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2025-05-26 12:18:39
 * @copyright Copyright (c) 2014-2025, Company Genitop. Co., Ltd.
 */
#ifndef _GT_CIRCULAR_ARRAY_H_
#define _GT_CIRCULAR_ARRAY_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct gt_circular_array_s
{
    void* data;             // Pointer to the data array
    uint32_t size;          // Size of the array
    uint32_t count;         // Number of items currently in the array
    uint32_t head;          // index of the first element
    uint32_t tail;          // Index of the next item to be added
    uint32_t item_size;     // Size of each item in the array
    bool auto_overflow;     // If true, when the array is full, the oldest data will be overwritten by new data
} gt_circular_array_st;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief Create a new circular array
 *
 * @param size item count of the array
 * @param item_size Size of each item in the array
 * @param auto_overflow If true, when the array is full, the oldest data will be overwritten by new data
 * @return gt_circular_array_st* Pointer to the created circular array, or NULL if creation failed
 */
gt_circular_array_st* gt_circular_array_create(uint32_t size, uint32_t item_size, bool auto_overflow);

/**
 * @brief Destroy a circular array
 *
 * @param array Pointer to the circular array to be destroyed
 */
void gt_circular_array_destroy(gt_circular_array_st* array);

/**
 * @brief Add an item to the circular array
 *
 * @param array Pointer to the circular array
 * @return true if the item was successfully added, false otherwise
 * @return false if the array is full and auto_overflow is false
 */
bool is_circular_array_full(const gt_circular_array_st* array);

/**
 * @brief Check if the circular array is empty
 *
 * @param array Pointer to the circular array
 * @return true if the circular array is empty, false otherwise
 * @return false if the circular array is not empty
 */
bool is_circular_array_empty(const gt_circular_array_st* array);

/**
 * @brief Get the item at the specified index in the circular array
 *
 * @param array Pointer to the circular array
 * @param item Pointer to the item to be retrieved
 * @return true if the item was successfully retrieved, false otherwise
 * @return false if the index is out of range
 */
bool gt_circular_array_push(gt_circular_array_st* array, const void* item);

/**
 * @brief Add an item to the head of the circular array
 *
 * @param array Pointer to the circular array
 * @param item Pointer to the item to be added
 * @return true if the item was successfully added, false otherwise
 * @return false if the array is full and auto_overflow is false
 */
bool gt_circular_array_push_head(gt_circular_array_st* array, const void* item);

/**
 * @brief Remove and return the oldest item from the circular array
 *
 * @param array Pointer to the circular array
 * @param item Pointer to the item to be removed
 * @return true if the item was successfully removed, false otherwise
 * @return false if the circular array is empty
 */
bool gt_circular_array_pop(gt_circular_array_st* array, void* item);

/**
 * @brief Get the item at the specified index in the circular array
 *
 * @param array Pointer to the circular array
 * @param index Index of the item to be retrieved
 * @param item Pointer to the item to be retrieved
 * @return true if the item was successfully retrieved, false otherwise
 * @return false if the index is out of range
 */
bool gt_circular_array_get(const gt_circular_array_st* array, uint32_t index, void* item);

/**
 * @brief Set the item at the specified index in the circular array
 *
 * @param array Pointer to the circular array
 * @param index Index of the item to be set
 * @param item Pointer to the item to be set
 * @return true if the item was successfully set, false otherwise
 * @return false if the index is out of range
 */
bool gt_circular_array_set(gt_circular_array_st* array, uint32_t index, const void* item);

/**
 * @brief Set the circular array to be full
 *
 * @param array Pointer to the circular array
 */
void gt_circular_array_set_full(gt_circular_array_st* array);

/**
 * @brief Clear the circular array
 *
 * @param array Pointer to the circular array to be cleared
 */
void gt_circular_array_clear(gt_circular_array_st* array);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_CIRCULAR_ARRAY_H_

/* end of file ----------------------------------------------------------*/


