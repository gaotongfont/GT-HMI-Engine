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
    return target == item ? true : false;
}

/* global functions / API interface -------------------------------------*/

_gt_vector_st * _gt_vector_create(_gt_vector_free_item_cb_t free_cb, _gt_vector_equal_item_cb_t equal_item_cb)
{
    _gt_vector_st * vector = (_gt_vector_st * )gt_mem_malloc(sizeof(_gt_vector_st));
    if (NULL == vector) {
        return NULL;
    }
    gt_memset(vector, 0, sizeof(_gt_vector_st));
    _GT_INIT_LIST_HEAD(&vector->list_head);
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

bool _gt_vector_add_item(_gt_vector_st * vector_p, void * item)
{
    if (NULL == vector_p) {
        return false;
    }
    if (NULL == item) {
        return false;
    }
    _gt_vector_item_st * new_item = (_gt_vector_item_st * )gt_mem_malloc(sizeof(_gt_vector_item_st));
    if (NULL == new_item) {
        GT_CHECK_PRINT(new_item);
        return false;
    }
    _GT_INIT_LIST_HEAD(&new_item->node);
    new_item->val = item;
    _gt_list_add_tail(&new_item->node, &vector_p->list_head);
    ++vector_p->count;
    return true;
}

bool _gt_vector_replace_item(_gt_vector_st * vector_p, uint16_t index, void * item)
{
    if (NULL == item) {
        return false;
    }
    if (NULL == vector_p) {
        return false;
    }
    if (index >= vector_p->count) {
        return false;
    }
    _gt_vector_item_st * item_p = NULL;
    _gt_list_for_each_entry(item_p, &vector_p->list_head, _gt_vector_item_st, node) {
        if (index-- == 0) {
            if (vector_p->free_item_cb) {
                vector_p->free_item_cb(item_p->val);
            }
            item_p->val = item;
            return true;
        }
    }
    return false;
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
    _gt_vector_item_st * item_p = NULL;
    _gt_vector_item_st * backup_p = NULL;
    void * val = NULL;
    _gt_list_for_each_entry_safe(item_p, backup_p, &vector_p->list_head, _gt_vector_item_st, node) {
        if (false == vector_p->equal_item_cb(item_p->val, target)) {
            continue;
        }
        _gt_list_del(&item_p->node);
        val = item_p->val;
        gt_mem_free(item_p);
        if (vector_p->free_item_cb) {
            vector_p->free_item_cb(val);
        }
        val = NULL;
        --vector_p->count;
        is_find = true;
    }
    return is_find;
}

void * _gt_vector_turn_next(_gt_vector_st * vector)
{
    if (NULL == vector) {
        return NULL;
    }
    if (0 == vector->count) {
        return NULL;
    }
    if (++vector->index >= vector->count) {
        vector->index = 0;
    }
    _gt_vector_item_st * item_p = NULL;
    uint16_t i = 0;
    _gt_list_for_each_entry(item_p, &vector->list_head, _gt_vector_item_st, node) {
        if (i++ == vector->index) {
            return item_p->val;
        }
    }
    return NULL;
}

gt_size_t _gt_vector_get_count(_gt_vector_st const * vector)
{
    if (NULL == vector) {
        return -1;
    }
    return (gt_size_t)vector->count;
}

gt_size_t _gt_vector_get_index(_gt_vector_st const * vector)
{
    if (NULL == vector) {
        return -1;
    }
    if (0 == vector->count) {
        return -1;
    }
    return (gt_size_t)vector->index;
}

void * _gt_vector_get_item(_gt_vector_st const * vector, uint16_t index)
{
    if (NULL == vector) {
        return NULL;
    }
    if (0 == vector->count) {
        return NULL;
    }
    if (index >= vector->count) {
        return NULL;
    }
    _gt_vector_item_st * item_p = NULL;
    uint16_t i = 0;
    _gt_list_for_each_entry(item_p, &vector->list_head, _gt_vector_item_st, node) {
        if (i++ == index) {
            return item_p->val;
        }
    }
    return NULL;
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

    _gt_vector_item_st * item_p = NULL;
    _gt_vector_item_st * backup_p = NULL;
    void * val = NULL;

   _gt_list_for_each_entry_safe_reverse(item_p, backup_p, &vector->list_head, _gt_vector_item_st, node) {
        _gt_list_del(&item_p->node);
        val = item_p->val;
        gt_mem_free(item_p);
        if (vector->free_item_cb) {
            vector->free_item_cb(val);
        }
        val = NULL;
        --vector->count;
    }
    return true;
}

/* end ------------------------------------------------------------------*/
