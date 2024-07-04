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

static inline void _free_item_obj(_gt_vector_st * vector, _gt_vector_item_st * item_p) {
    void * val = item_p->val;

    _gt_list_del(&item_p->node);
    /** free vector list item */
    gt_mem_free(item_p);

    if (NULL == vector->free_item_cb) {
        return;
    }
    vector->free_item_cb(val);
}

static bool _gt_vector_iterator_has_next(_iter_dsc_st * vct_dsc) {
    GT_CHECK_BACK_VAL(vct_dsc, false);
    GT_CHECK_BACK_VAL(vct_dsc->iter_ctl, false);
    return vct_dsc->iter_ctl->idx < vct_dsc->count ? true : false;
}

static void * _gt_vector_iterator_next(_iter_dsc_st * vct_dsc) {
    GT_CHECK_BACK_VAL(vct_dsc, false);
    GT_CHECK_BACK_VAL(vct_dsc->iter_ctl, false);

    if (vct_dsc->iter_ctl->item_p) {
        vct_dsc->iter_ctl->item_p = (_gt_vector_item_st * )_gt_list_next_entry(vct_dsc->iter_ctl->item_p, _gt_vector_item_st, node);
    } else {
        vct_dsc->iter_ctl->item_p = (_gt_vector_item_st * )_gt_list_first_entry(&vct_dsc->list_head, _gt_vector_item_st, node);
    }
    ++vct_dsc->iter_ctl->idx;
    return vct_dsc->iter_ctl->item_p->val;
}

static gt_size_t _gt_vector_iterator_index(_iter_dsc_st * vct_dsc) {
    GT_CHECK_BACK_VAL(vct_dsc, -1);
    GT_CHECK_BACK_VAL(vct_dsc->iter_ctl, -1);
    return vct_dsc->iter_ctl->idx;
}

/* global functions / API interface -------------------------------------*/

_gt_vector_st * _gt_vector_create(_gt_vector_free_item_cb_t free_cb, _gt_vector_equal_item_cb_t equal_item_cb)
{
    _gt_vector_st * vector = (_gt_vector_st * )gt_mem_malloc(sizeof(_gt_vector_st));
    GT_CHECK_BACK_VAL(vector, NULL);

    gt_memset(vector, 0, sizeof(_gt_vector_st));
    _GT_INIT_LIST_HEAD(&vector->list_head);
    _gt_vector_add_free_item_cb(vector, free_cb);
    _gt_vector_add_equal_item_cb(vector, equal_item_cb ? equal_item_cb : _default_equal_cb);
    return vector;
}

bool _gt_vector_add_free_item_cb(_gt_vector_st * vector_p, _gt_vector_free_item_cb_t free_item_cb)
{
    GT_CHECK_BACK_VAL(vector_p, false);
    GT_CHECK_BACK_VAL(free_item_cb, false);

    vector_p->free_item_cb = free_item_cb;
    return true;
}

bool _gt_vector_add_equal_item_cb(_gt_vector_st * vector_p, _gt_vector_equal_item_cb_t equal_item_cb)
{
    GT_CHECK_BACK_VAL(vector_p, false);
    GT_CHECK_BACK_VAL(equal_item_cb, false);

    vector_p->equal_item_cb = equal_item_cb;
    return true;
}

bool _gt_vector_add_item(_gt_vector_st * vector_p, void * item)
{
    GT_CHECK_BACK_VAL(vector_p, false);
    GT_CHECK_BACK_VAL(item, false);
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
    GT_CHECK_BACK_VAL(vector_p, false);
    GT_CHECK_BACK_VAL(item, false);
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
    GT_CHECK_BACK_VAL(vector_p, false);
    GT_CHECK_BACK_VAL(target, false);
    GT_CHECK_BACK_VAL(count, false);
    GT_CHECK_BACK_VAL(vector_p->equal_item_cb, false);

    _gt_vector_item_st * item_p = NULL;
    _gt_vector_item_st * backup_p = NULL;
    _gt_list_for_each_entry_safe(item_p, backup_p, &vector_p->list_head, _gt_vector_item_st, node) {
        if (false == vector_p->equal_item_cb(item_p->val, target)) {
            continue;
        }
        _free_item_obj(vector_p, item_p);
        --vector_p->count;
        is_find = true;
    }
    if (vector_p->index + 1 > vector_p->count) {
        vector_p->index = vector_p->count - 1;
    }
    return is_find;
}

void * _gt_vector_turn_prev(_gt_vector_st * vector)
{
    GT_CHECK_BACK_VAL(vector, NULL);
    GT_CHECK_BACK_VAL(vector->count, NULL);
    if (vector->index == 0) {
        vector->index = vector->count;
    }
    if (--vector->index < 0) {
        vector->index = vector->count - 1;
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

void * _gt_vector_turn_next(_gt_vector_st * vector)
{
    GT_CHECK_BACK_VAL(vector, NULL);
    GT_CHECK_BACK_VAL(vector->count, NULL);
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
    GT_CHECK_BACK_VAL(vector, -1);
    return vector->count;
}

gt_size_t _gt_vector_set_index(_gt_vector_st * vector, gt_size_t index)
{
    GT_CHECK_BACK_VAL(vector, -1);
    GT_CHECK_BACK_VAL(vector->count, -1);
    if (index < -1) {
        return -1;
    }
    if (-1 == index) {
        /** Set to last one item index */
        vector->index = (gt_size_t)(vector->count - 1);
        return vector->index;
    }
    /** Normal direct */
    if (index + 1 > vector->count) {
        return -1;
    }
    vector->index = index;
    return vector->index;
}

gt_size_t _gt_vector_get_index(_gt_vector_st const * vector)
{
    GT_CHECK_BACK_VAL(vector, -1);
    GT_CHECK_BACK_VAL(vector->count, -1);

    return vector->index;
}

void * _gt_vector_get_item(_gt_vector_st const * vector, gt_size_t index)
{
    GT_CHECK_BACK_VAL(vector, NULL);
    GT_CHECK_BACK_VAL(vector->count, NULL);
    if (index < 0) {
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

bool _gt_vector_is_tail_index_now(_gt_vector_st const * vector)
{
    GT_CHECK_BACK_VAL(vector, false);
    return (vector->index + 1) >= vector->count ? true : false;
}

bool _gt_vector_free(_gt_vector_st * vector_p)
{
    GT_CHECK_BACK_VAL(vector_p, false);

    _gt_vector_clear_all_items(vector_p);

    gt_mem_free(vector_p);
    return true;
}

_gt_vector_iterator_st _gt_vector_get_iterator(_gt_vector_st * vector)
{
    _gt_vector_iterator_st ret_iter = {
        .has_next = _gt_vector_iterator_has_next,
        .next = _gt_vector_iterator_next,
        .index = _gt_vector_iterator_index,
    };
    uint16_t instance = sizeof(_gt_vector_iterator_ctl_st);
    GT_CHECK_BACK_VAL(vector, ret_iter);
    if (NULL == vector->iter_ctl) {
        vector->iter_ctl = (_gt_vector_iterator_ctl_st * )gt_mem_malloc(instance);
        GT_CHECK_BACK_VAL(vector->iter_ctl, ret_iter);
    }
    gt_memset(vector->iter_ctl, 0, instance);
    ret_iter.dsc_t = vector;
    return ret_iter;
}

bool _gt_vector_clear_all_items(_gt_vector_st * vector)
{
    GT_CHECK_BACK_VAL(vector, false);

    if (vector->iter_ctl) {
        gt_mem_free(vector->iter_ctl);
        vector->iter_ctl = NULL;
    }

    _gt_vector_item_st * item_p = NULL;
    _gt_vector_item_st * backup_p = NULL;
   _gt_list_for_each_entry_safe_reverse(item_p, backup_p, &vector->list_head, _gt_vector_item_st, node) {
        _free_item_obj(vector, item_p);
    }
    vector->index = 0;
    vector->count = 0;
    return true;
}

/* end ------------------------------------------------------------------*/
