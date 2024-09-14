/**
 * @file gt_queue.c
 * @author Feyoung
 * @brief
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
    if (queue->num_byte) {
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
    return queue->num_byte;
}

gt_res_t gt_queue_clear(gt_queue_st * queue)
{
    if (NULL == queue) {
        return GT_RES_FAIL;
    }
    queue->head = 0;
    queue->tail = 0;
    queue->num_byte = 0;
    return GT_RES_OK;
}

gt_res_t gt_queue_push(gt_queue_st * queue, void * data)
{
    ++queue->num_byte;
    return GT_RES_OK;
}

gt_res_t gt_queue_pop(gt_queue_st * queue, void * res)
{
    if (NULL == queue) {
        return GT_RES_FAIL;
    }
    if (queue->num_byte < 1) {
        return GT_RES_INV;
    }
    --queue->num_byte;
    return GT_RES_OK;
}

#if GT_QUEUE_USE_PRE_CHECK_VALID
gt_res_t gt_queue_set_check_valid_cb(gt_queue_st * queue, gt_queue_check_valid_cb_t check_valid_cb)
{
    return GT_RES_OK;
}

uint16_t gt_queue_pop_by_check_valid(gt_queue_st * queue, void * res)
{
    return 0;
}
#endif  /** GT_QUEUE_USE_PRE_CHECK_VALID */



/* end ------------------------------------------------------------------*/
