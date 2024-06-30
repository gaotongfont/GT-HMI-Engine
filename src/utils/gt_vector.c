/**
 * @file gt_vector.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2023-05-23 19:59:36
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_vector.h"
#include "../others/gt_log.h"
#include "../core/gt_mem.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

/**
 * @brief String comparison is used by default
 *
 * @param item The item that in order from the list
 * @param target The target content to be searched
 * @return true find it out
 * @return false not find it out
 */
static bool _default_equal_cb(void * item, void * target) {
    return strcmp(item, target) ? false : true;
}

/* global functions / API interface -------------------------------------*/

_gt_vector_st * _gt_vector_create(_gt_vector_free_item_cb_t free_cb, _gt_vector_equal_item_cb_t equal_item_cb)
{
    _gt_vector_st * vector = (_gt_vector_st * )gt_mem_malloc(sizeof(_gt_vector_st));
    if (NULL == vector) {
        return NULL;
    }
    gt_memset(vector, 0, sizeof(_gt_vector_st));
    _gt_vector_add_free_item_cb(vector, free_cb);
    _gt_vector_add_equal_item_cb(vector, equal_item_cb ? equal_item_cb : _default_equal_cb);
    return vector;
}

bool _gt_vector_add_free_item_cb(_gt_vector_st * vector_p, _gt_vector_free_item_cb_t free_item_cb)
{
    if (NULL == vector_p) {
        return false;
    }
    if (NULL == free_item_cb) {
        return false;
    }

    vector_p->free_item_cb = free_item_cb;
    return true;
}

bool _gt_vector_add_equal_item_cb(_gt_vector_st * vector_p, _gt_vector_equal_item_cb_t equal_item_cb)
{
    if (NULL == vector_p) {
        return false;
    }
    if (NULL == equal_item_cb) {
        return false;
    }

    vector_p->equal_item_cb = equal_item_cb;
    return true;
}

bool _gt_vector_add_item(_gt_vector_st * vector_p, void * item, uint16_t size)
{
    if (NULL == vector_p) {
        return false;
    }
    if (NULL == item) {
        return false;
    }
    if (NULL == vector_p->list) {
        vector_p->index = 0;
        vector_p->count = 0;
        vector_p->list = (void ** )gt_mem_malloc(sizeof(void *));
        if (NULL == vector_p->list) {
            return false;
        }
    } else {
        void ** ptr = (void ** )gt_mem_realloc(vector_p->list, sizeof(void * ) * (vector_p->count + 1));
        if (NULL == ptr) {
            goto list_lb;
        }
        vector_p->list = ptr;
    }
    vector_p->list[vector_p->count] = gt_mem_malloc(size);
    if (NULL == vector_p->list[vector_p->count]) {
        goto list_lb;
    }
    gt_memcpy(vector_p->list[vector_p->count++], item, size);
    // GT_LOG_A("+", "add %p, idx: %d/%d", vector_p->list[vector_p->count - 1], vector_p->count - 1, vector_p->count);
    return true;

list_lb:
    vector_p->list = (void ** )gt_mem_realloc(vector_p->list, sizeof(void * ) * vector_p->count);
    return false;
}

bool _gt_vector_replace_item(_gt_vector_st * vector_p, uint16_t index, void * item, uint16_t size)
{
    if (NULL == item) {
        return false;
    }
    if (NULL == vector_p) {
        return false;
    }
    if (NULL == vector_p->list) {
        return _gt_vector_add_item(vector_p, item, size);
    }
    if (index >= vector_p->count) {
        return false;
    }
    if (vector_p->free_item_cb) {
        vector_p->free_item_cb(vector_p->list[index]);
    }
    gt_memcpy(vector_p->list[index], item, size);
    return true;
}

bool _gt_vector_remove_item(_gt_vector_st * vector_p, void * target)
{
    uint16_t i = 0, count = vector_p->count;
    uint8_t is_find = false;
    if (NULL == vector_p || NULL == target) {
        return false;
    }
    if (0 == count) {
        return false;
    }
    if (NULL == vector_p->equal_item_cb) {
        return false;
    }
    for (i = 0; i < count; i++) {
        if (true == vector_p->equal_item_cb(vector_p->list[i], target)) {
            is_find = true;
            break;
        }
    }
    if (false == is_find) {
        return false;
    }
    if (vector_p->free_item_cb) {
        if (!vector_p->free_item_cb(vector_p->list[i])) {
            return false;
        }
    }
    vector_p->list[i] = NULL;
    if (i < --count) {
        gt_memmove(&vector_p->list[i], &vector_p->list[i + 1], sizeof(void * ) * (count - i));
    }
    vector_p->list[count] = NULL;
    vector_p->list = (void ** )gt_mem_realloc(vector_p->list, sizeof(void * ) * (--vector_p->count));
    if (vector_p->index >= vector_p->count) {
        vector_p->index = vector_p->count - 1;
    }
    if (NULL == vector_p->list || 0 == vector_p->count) {
        vector_p->index = 0;
    }
    // GT_LOG_A("-", "remove %p, idx: %d/%d", vector_p->list, vector_p->index, vector_p->count);
    return true;
}

void * _gt_vector_turn_next(_gt_vector_st * vector)
{
    if (NULL == vector) {
        return NULL;
    }
    if (NULL == vector->list || 0 == vector->count) {
        return NULL;
    }
    if (++vector->index >= vector->count) {
        vector->index = 0;
    }
    return vector->list[vector->index];
}

gt_size_t _gt_vector_get_count(_gt_vector_st const * vector)
{
    if (NULL == vector) {
        return -1;
    }
    if (NULL == vector->list) {
        return -1;
    }
    return (gt_size_t)vector->count;
}

gt_size_t _gt_vector_get_index(_gt_vector_st const * vector)
{
    if (NULL == vector) {
        return -1;
    }
    if (NULL == vector->list || 0 == vector->count) {
        return -1;
    }
    return (gt_size_t)vector->index;
}

void * _gt_vector_get_item(_gt_vector_st const * vector, uint16_t index)
{
    if (NULL == vector) {
        return NULL;
    }
    if (NULL == vector->list || 0 == vector->count) {
        return NULL;
    }

    if (index >= vector->count) {
        return NULL;
    }
    return vector->list[index];
}

bool _gt_vector_free(_gt_vector_st * vector_p)
{
    if (NULL == vector_p) {
        return false;
    }
    _gt_vector_clear_all_items(vector_p);

    gt_mem_free(vector_p);
    return true;
}

bool _gt_vector_clear_all_items(_gt_vector_st * vector)
{
    if (NULL == vector) {
        return false;
    }

    gt_size_t i = vector->count - 1;
    for (; i >= 0; i--) {
        if (!vector->list[i]) {
            continue;
        }
        if (vector->free_item_cb) {
            vector->free_item_cb(vector->list[i]);
        }
        gt_mem_free(vector->list[i]);
        // GT_LOG_A("-", "clear %p, idx: %d/%d", vector->list[i], i, vector->count - 1);
        vector->list[i] = NULL;
        --vector->count;
    }
    gt_mem_free(vector->list);
    vector->list = NULL;
    return true;
}

/* end ------------------------------------------------------------------*/
