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
#include "../core/gt_mem.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

/**
 * @brief A free callback to the memory inside the element
 *
 * @param item The item to be freed
 * @return true success
 * @return false failed
 */
static bool _default_free_cb(void * item) {
    gt_mem_free(item);
    return true;
}

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

static void _add_default_callback(_gt_vector_st * vector) {
    if (NULL == vector) {
        return;
    }
    _gt_vector_add_free_item_cb(&vector, _default_free_cb);
    _gt_vector_add_equal_item_cb(&vector, _default_equal_cb);
}

/**
 * @brief Create a vector object
 *
 * @return _gt_vector_st* default object
 */
static inline _gt_vector_st * _gt_vector_create(void) {
    _gt_vector_st * vector = (_gt_vector_st * )gt_mem_malloc(sizeof(_gt_vector_st));
    if (NULL == vector) {
        return NULL;
    }
    gt_memset(vector, 0, sizeof(_gt_vector_st));
    _add_default_callback(vector);
    return vector;
}

/* global functions / API interface -------------------------------------*/

bool _gt_vector_add_free_item_cb(_gt_vector_st ** vector_p, _gt_vector_free_item_cb_t free_item_cb)
{
    if (NULL == vector_p) {
        return false;
    }
    if (NULL == free_item_cb) {
        return false;
    }

    if (NULL == *vector_p) {
        *vector_p = _gt_vector_create();
        if (NULL == *vector_p) {
            return false;
        }
    }

    (*vector_p)->free_item_cb = free_item_cb;
    return true;
}

bool _gt_vector_add_equal_item_cb(_gt_vector_st ** vector_p, _gt_vector_equal_item_cb_t equal_item_cb)
{
    if (NULL == vector_p) {
        return false;
    }
    if (NULL == equal_item_cb) {
        return false;
    }

    if (NULL == *vector_p) {
        *vector_p = _gt_vector_create();
        if (NULL == *vector_p) {
            return false;
        }
    }

    (*vector_p)->equal_item_cb = equal_item_cb;
    return true;
}

bool _gt_vector_add_item(_gt_vector_st ** vector_p, void * item, uint16_t size)
{
    if (NULL == vector_p) {
        return false;
    }
    if (NULL == item) {
        return false;
    }

    if (NULL == *vector_p) {
        *vector_p = _gt_vector_create();
        if (NULL == *vector_p) {
            return false;
        }
    }

    if (NULL == (*vector_p)->list) {
        (*vector_p)->list = (void ** )gt_mem_malloc(sizeof(void *));
        if (NULL == (*vector_p)->list) {
            goto vector_lb;
        }
    } else {
        (*vector_p)->list = (void ** )gt_mem_realloc((*vector_p)->list, sizeof(void * ) * ((*vector_p)->count + 1));
        if (NULL == (*vector_p)->list) {
            goto vector_lb;
        }
    }

    (*vector_p)->list[(*vector_p)->count] = gt_mem_malloc(size);
    if (NULL == (*vector_p)->list[(*vector_p)->count]) {
        goto list_lb;
    }
    gt_memcpy((*vector_p)->list[(*vector_p)->count++], item, size);
    return true;

list_lb:
    (*vector_p)->list = (void ** )gt_mem_realloc((*vector_p)->list, sizeof(void * ) * (*vector_p)->count);
vector_lb:
    gt_mem_free(*vector_p);
    *vector_p = NULL;
    return false;
}

bool _gt_vector_remove_item(_gt_vector_st ** vector_p, void * target)
{
    uint16_t i = 0, count = (*vector_p)->count;
    if (NULL == vector_p || NULL == target) {
        return false;
    }
    if (0 == count) {
        return false;
    }

    for (i = 0; i < count; i++) {
        if (true == (*vector_p)->equal_item_cb((*vector_p)->list[i], target) ) {
            break;
        }
    }
    if (!(*vector_p)->free_item_cb((*vector_p)->list[i])) {
        return false;
    }
    (*vector_p)->list[i] = NULL;
    if (i != count - 1) {
        gt_memmove(&(*vector_p)->list[i], &(*vector_p)->list[i + 1], sizeof(void * ) * (count - i - 1));
    }
    (*vector_p)->list = (void ** )gt_mem_realloc((*vector_p)->list, sizeof(void * ) * (--(*vector_p)->count));
    if ((*vector_p)->index >= (*vector_p)->count) {
        (*vector_p)->index = (*vector_p)->count - 1;
    }
    if (NULL == (*vector_p)->list || 0 == (*vector_p)->count) {
        (*vector_p)->index = 0;
    }

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

bool _gt_vector_free(_gt_vector_st ** vector_p)
{
    if (NULL == vector_p) {
        return false;
    }
    _gt_vector_clear_all_items(*vector_p);

    gt_mem_free(*vector_p);
    *vector_p = NULL;
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
        vector->free_item_cb(vector->list[i]);
        vector->list[i] = NULL;
    }
    gt_mem_free(vector->list);
    vector->list = NULL;
    return true;
}

/* end ------------------------------------------------------------------*/
