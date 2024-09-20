/**
 * @file gt_queue.c
 * @author Feyoung
 * @brief cycle queue
 * @version 0.1
 * @date 2024-09-14 16:22:30
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_queue.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "stddef.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static inline uint16_t _head_ptr_inc(gt_queue_st * queue) {
    if (++queue->head >= queue->max_count) {
        queue->head = 0;
    }
    return queue->head;
}

static inline uint16_t _tail_ptr_inc(gt_queue_st * queue) {
    if (++queue->tail >= queue->max_count) {
        queue->tail = 0;
    }
    return queue->tail;
}

static inline uint16_t _item_count_inc(gt_queue_st * queue) {
    if (queue->el_count < queue->max_count) {
        ++queue->el_count;
    }
    return queue->el_count;
}

static inline uint16_t _item_count_dec(gt_queue_st * queue) {
    if (queue->el_count > 0) {
        --queue->el_count;
    }
    return queue->el_count;
}

static gt_res_t _get_item_by(gt_queue_st const * const queue, uint16_t idx, void * res) {
    uint8_t * ptr = (uint8_t * )queue->buffer + idx * queue->instance;
    if (queue->instance > 1) {
        gt_memcpy(res, ptr, queue->instance);
    } else if (1 == queue->instance) {
        *(uint8_t *)res = *ptr;
    } else {
        return GT_RES_FAIL;
    }
    return GT_RES_OK;
}

#if GT_QUEUE_USE_PRE_CHECK_VALID
static inline uint16_t _get_tail_ptr_by(gt_queue_st const * const queue, uint16_t offset) {
    uint16_t idx = queue->tail + offset;
    while (idx >= queue->max_count) {
        idx -= queue->max_count;
    }
    return idx;
}

static gt_res_t _get_value_cb(struct gt_queue_s const * const queue, uint16_t offset, void * res) {
    if (NULL == res) {
        return GT_RES_FAIL;
    }
    if (offset >= queue->el_count) {
        return GT_RES_FAIL;
    }
    uint16_t idx = _get_tail_ptr_by(queue, offset);
    return _get_item_by(queue, idx, res);
}

static uint16_t _get_count_cb(struct gt_queue_s const * const queue) {
    return queue->el_count;
}
#endif

/* global functions / API interface -------------------------------------*/
gt_queue_st * gt_queue_init(uint16_t instance, void * buffer, uint32_t byte_size)
{
    if (0 == instance) {
        return NULL;
    }
    if (NULL == buffer || 0 == byte_size) {
        return NULL;
    }
    if (byte_size < instance) {
        return NULL;
    }
    uint32_t size = sizeof(gt_queue_st);
    gt_queue_st * queue = (gt_queue_st *)gt_mem_malloc(size);
    if (NULL == queue) {
        return NULL;
    }
    gt_memset(queue, 0, size);
    queue->instance = instance;
    queue->buffer = buffer;
    queue->byte_size = byte_size;
    queue->max_count = queue->byte_size / instance;
    return queue;
}

gt_res_t gt_queue_deinit(gt_queue_st * queue)
{
    GT_CHECK_BACK_VAL(queue, GT_RES_FAIL);
    gt_memset(queue, 0, sizeof(gt_queue_st));
    gt_mem_free(queue);
    return GT_RES_OK;
}

bool gt_queue_is_empty(gt_queue_st * queue)
{
    if (NULL == queue) {
        return true;
    }
    if (queue->el_count) {
        return false;
    }
    if (queue->head != queue->tail) {
        return false;
    }
    return true;
}

uint16_t gt_queue_get_count(gt_queue_st * queue)
{
    if (NULL == queue) {
        return 0;
    }
    if (0 == queue->instance) {
        return 0;
    }
    return queue->el_count;
}

gt_res_t gt_queue_clear(gt_queue_st * queue)
{
    if (NULL == queue) {
        return GT_RES_FAIL;
    }
    queue->head = 0;
    queue->tail = 0;
    queue->el_count = 0;
    return GT_RES_OK;
}

gt_res_t gt_queue_push(gt_queue_st * queue, void * data)
{
    if (NULL == queue) {
        return GT_RES_FAIL;
    }
    if (NULL == data) {
        return GT_RES_FAIL;
    }

    uint8_t * ptr = (uint8_t * )queue->buffer + queue->head * queue->instance;
    if (queue->instance > 1) {
        gt_memcpy(ptr, data, queue->instance);
    } else if (1 == queue->instance) {
        *ptr = (uint8_t)(*(uint8_t *)data);
    } else {
        return GT_RES_FAIL;
    }

    if (queue->el_count && queue->head == queue->tail) {
        /** drop the oldest item */
        _tail_ptr_inc(queue);
    }
    _head_ptr_inc(queue);
    _item_count_inc(queue);
    return GT_RES_OK;
}

gt_res_t gt_queue_pop(gt_queue_st * queue, void * res)
{
    if (NULL == res) {
        return GT_RES_FAIL;
    }
    *(uint8_t * )res = 0;
    if (NULL == queue) {
        return GT_RES_FAIL;
    }
    if (queue->el_count < 1) {
        return GT_RES_INV;
    }

    gt_res_t ret = _get_item_by(queue, queue->tail, res);
    if (GT_RES_OK != ret) {
        return ret;
    }
    _tail_ptr_inc(queue);
    _item_count_dec(queue);
    return GT_RES_OK;
}

#if GT_QUEUE_USE_PRE_CHECK_VALID
gt_res_t gt_queue_set_check_valid_cb(gt_queue_st * queue, gt_queue_check_valid_cb_t check_valid_cb)
{
    if (NULL == queue) {
        return GT_RES_FAIL;
    }
    if (NULL == check_valid_cb) {
        return GT_RES_FAIL;
    }
    queue->check_valid_cb = check_valid_cb;
    return GT_RES_OK;
}

gt_queue_check_valid_res_st gt_queue_pop_by_check_valid(gt_queue_st * queue, void * res)
{
    gt_queue_check_valid_res_st ret = {
        .res = GT_RES_FAIL,
        .valid_data_len = 0,
    };
    if (NULL == res) {
        return ret;
    }
    *(uint8_t * )res = 0;
    if (NULL == queue) {
        return ret;
    }
    if (queue->el_count < 1) {
        ret.res = GT_RES_INV;
        return ret;
    }

    uint8_t * ptr = (uint8_t * )queue->buffer + queue->tail * queue->instance;
    gt_queue_check_valid_st check_valid = {
        .queue = queue,
        .get_count = _get_count_cb,
        .get_value = _get_value_cb,
    };
    while (GT_RES_FAIL == ret.res) {
        ret = queue->check_valid_cb(&check_valid);
        if (ret.valid_data_len > queue->max_count) {
            ret.res = GT_RES_FAIL;
            ret.valid_data_len = 0;
        } else if (GT_RES_OK == ret.res) {
            break;
        } else if (GT_RES_INV == ret.res) {
            return ret;
        }
        ptr = (uint8_t * )queue->buffer + _tail_ptr_inc(queue) * queue->instance;
        if (0 == _item_count_dec(queue)) {
            ret.res = GT_RES_FAIL;
            return ret;
        }
    }

    uint8_t * res_ptr = (uint8_t * )res;
    for (uint16_t i = 0; i < ret.valid_data_len; ++i) {
        gt_res_t tmp_ret = _get_item_by(queue, queue->tail, res_ptr);
        if (GT_RES_OK != tmp_ret) {
            ret.res = GT_RES_FAIL;
            return ret;
        }
        _tail_ptr_inc(queue);
        _item_count_dec(queue);
        res_ptr += queue->instance;
    }
    return ret;
}
#endif  /** GT_QUEUE_USE_PRE_CHECK_VALID */



/* end ------------------------------------------------------------------*/
