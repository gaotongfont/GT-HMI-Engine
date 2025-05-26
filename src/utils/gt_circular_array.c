/**
 * @file gt_circular_array.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2025-05-26 12:18:12
 * @copyright Copyright (c) 2014-2025, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_circular_array.h"
#include  "../core/gt_mem.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static char* _get_addr(const gt_circular_array_st* array, uint32_t index)
{
    if (!array || index >= array->size) {
        return NULL;
    }
    return (char*)array->data + (index * array->item_size);
}

/* global functions / API interface -------------------------------------*/
gt_circular_array_st* gt_circular_array_create(uint32_t size, uint32_t item_size, bool auto_overflow)
{
    if (size == 0 || item_size == 0) {
        return NULL;
    }

    gt_circular_array_st* array = (gt_circular_array_st*)gt_mem_malloc(sizeof(gt_circular_array_st));
    if (!array) {
        return NULL;
    }

    array->data = gt_mem_malloc(size * item_size);
    if (!array->data) {
        gt_mem_free(array);
        return NULL;
    }
    gt_memset(array->data, 0, size * item_size);

    array->size = size;
    array->count = 0;
    array->head = 0;
    array->tail = 0;
    array->item_size = item_size;
    array->auto_overflow = auto_overflow;

    return array;
}

void gt_circular_array_destroy(gt_circular_array_st* array)
{
    if (array) {
        if (array->data) {
            gt_mem_free(array->data);
            array->data = NULL;
        }
        gt_mem_free(array);
        array = NULL;
    }
}

bool is_circular_array_full(const gt_circular_array_st* array)
{
    return (array->count == array->size);
}

bool is_circular_array_empty(const gt_circular_array_st* array)
{
    return (array->count == 0);
}

bool gt_circular_array_push(gt_circular_array_st* array, const void* item)
{
    if (!array || !item) {
        return false;
    }

    if (is_circular_array_full(array)) {
        if (array->auto_overflow) {
            array->head = (array->head + 1) % array->size; // Overwrite oldest data
        } else {
            return false; // Array is full and auto_overflow is false
        }
    } else {
        array->count++;
    }

    gt_memcpy(_get_addr(array, array->tail), item, array->item_size);
    array->tail = (array->tail + 1) % array->size;

    return true;
}

bool gt_circular_array_push_head(gt_circular_array_st* array, const void* item)
{
    if (!array || !item) {
        return false;
    }

    if (is_circular_array_full(array)) {
        if (array->auto_overflow) {
            array->tail = (array->tail + 1) % array->size;
        } else {
            return false;
        }
    } else {
        array->count++;
    }

    array->head = (array->head - 1 + array->size) % array->size;

    gt_memcpy(_get_addr(array, array->head), item, array->item_size);

    return true;
}

bool gt_circular_array_pop(gt_circular_array_st* array, void* item)
{
    if (!array || !item) {
        return false;
    }

    if (is_circular_array_empty(array)) {
        return false; // Array is empty
    }

    gt_memcpy(item, _get_addr(array, array->head), array->item_size);

    array->head = (array->head + 1) % array->size;
    array->count--;

    return true;
}

bool gt_circular_array_get(const gt_circular_array_st* array, uint32_t index, void* item)
{
    if (!array || !item || index >= array->count) {
        return false;
    }

    uint32_t real_index = (array->head + index) % array->size;

    gt_memcpy(item, _get_addr(array, real_index), array->item_size);

    return true;
}

bool gt_circular_array_set(gt_circular_array_st* array, uint32_t index, const void* item)
{
    if (!array || !item || index >= array->count) {
        return false;
    }

    uint32_t real_index = (array->head + index) % array->size;

    gt_memcpy(_get_addr(array, real_index), item, array->item_size);

    return true;
}

void gt_circular_array_set_full(gt_circular_array_st* array)
{
    if (!array) {
        return;
    }
    array->count = array->size;
    array->head = 0;
    array->tail = 0;
}

void gt_circular_array_clear(gt_circular_array_st* array)
{
    if (!array) {
        return;
    }
    array->count = 0;
    array->head = 0;
    array->tail = 0;
    gt_memset(array->data, 0, array->size * array->item_size);
}

/* end of file ----------------------------------------------------------*/


