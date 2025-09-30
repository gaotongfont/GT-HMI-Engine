/**
 * @file gt_fifo.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2025-07-18
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_fifo.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"



/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_fifo_st * gt_fifo_init(gt_fifo_param_st const * const param)
{
    if (NULL == param || param->size <= 0) {
        return NULL;
    }
    gt_size_t len = sizeof(gt_fifo_st);
    gt_fifo_st * ret_p = gt_mem_extern_malloc(len);
    if (NULL == ret_p) {
        return NULL;
    }
    gt_memset(ret_p, 0, len);
    ret_p->size = param->size;
    if (NULL == param->buffer) {
        ret_p->buffer = gt_mem_extern_malloc(ret_p->size);
        if (NULL == ret_p->buffer) {
            goto fail_lb;
        }
        ret_p->_is_malloc_inside = true;
    } else {
        ret_p->buffer = param->buffer;
    }
    ret_p->_is_init = true;
    return ret_p;

fail_lb:
    gt_mem_extern_free(ret_p);
    ret_p = NULL;
    return NULL;
}

void gt_fifo_free(gt_fifo_st * fifo)
{
    if (NULL == fifo) {
        return ;
    }
    if (fifo->_is_malloc_inside && fifo->buffer) {
        gt_mem_extern_free(fifo->buffer);
    }
    gt_memset(fifo, 0, sizeof(gt_fifo_st));
    gt_mem_extern_free(fifo);
}

int32_t gt_fifo_avail(gt_fifo_st * fifo)
{
    if (NULL == fifo) {
        return 0;
    }
    if (fifo->_is_full) {
        return 0;
    }
    if (fifo->head >= fifo->tail) {
        return fifo->size - fifo->head + fifo->tail;
    }
    return fifo->tail - fifo->head;
}

int32_t gt_fifo_used(gt_fifo_st * fifo)
{
    if (NULL == fifo) {
        return -1;
    }
    if (fifo->head == fifo->tail && !fifo->_is_full) {
        return 0;
    }

    if (fifo->head >= fifo->tail) {
        return fifo->head - fifo->tail;
    }
    return fifo->size - fifo->tail + fifo->head;
}

int32_t gt_fifo_get_size(gt_fifo_st * fifo)
{
    if (NULL == fifo) {
        return -1;
    }
    return fifo->size;
}

bool gt_fifo_is_empty(gt_fifo_st * fifo)
{
    if (NULL == fifo) {
        return false;
    }
    return (fifo->head == fifo->tail) && !fifo->_is_full;
}

void gt_fifo_reset(gt_fifo_st * fifo)
{
    if (NULL == fifo) {
        return;
    }
    fifo->head = 0;
    fifo->tail = 0;
    fifo->_is_full = false;
}

bool gt_fifo_push(gt_fifo_st * fifo, uint8_t data)
{
    if (NULL == fifo) {
        return false;
    }
    if (fifo->_is_full) {
        return false;
    }
    if (0 == gt_fifo_used(fifo)) {
        gt_fifo_reset(fifo);
    }

    fifo->buffer[fifo->head] = data;
    fifo->head = (fifo->head + 1) % fifo->size;

    if (fifo->head == fifo->tail) {
        fifo->_is_full = true;
    }

    return true;
}

int32_t gt_fifo_push_multi(gt_fifo_st * fifo, const uint8_t * data, int32_t length)
{
    int32_t avail = gt_fifo_avail(fifo);
    if (0 == avail) {
        return 0;
    }
    if (avail < length) {
        length = avail;
    }
    if (0 == length) {
        return 0;
    }
    if (0 == gt_fifo_used(fifo)) {
        gt_fifo_reset(fifo);
    }

    int32_t contig_avail = fifo->size - fifo->head;

    if (contig_avail >= length) {
        gt_memcpy(&fifo->buffer[fifo->head], data, length);
    } else {
        gt_memcpy(&fifo->buffer[fifo->head], data, contig_avail);
        gt_memcpy(fifo->buffer, data + contig_avail, length - contig_avail);
    }

    fifo->head = (fifo->head + length) % fifo->size;

    if (length == avail) {
        fifo->_is_full = true;
    }

    return length;
}

bool gt_fifo_pop(gt_fifo_st * fifo, uint8_t * data)
{
    if (gt_fifo_is_empty(fifo)) {
        return false;
    }

    *data = fifo->buffer[fifo->tail];
    fifo->tail = (fifo->tail + 1) % fifo->size;
    fifo->_is_full = false;

    return true;
}

int32_t gt_fifo_pop_multi(gt_fifo_st * fifo, uint8_t * data, int32_t length)
{
    int32_t used = gt_fifo_used(fifo);
    if (-1 == used) {
        return -1;
    }
    if (used < length) {
        length = used;
    }
    if (length == 0) {
        return 0;
    }

    unsigned int contig_used = fifo->size - fifo->tail;

    if (contig_used >= length) {
        gt_memcpy(data, &fifo->buffer[fifo->tail], length);
    } else {
        gt_memcpy(data, &fifo->buffer[fifo->tail], contig_used);
        gt_memcpy(data + contig_used, fifo->buffer, length - contig_used);
    }

    fifo->tail = (fifo->tail + length) % fifo->size;
    fifo->_is_full = false;

    return length;
}

bool gt_fifo_peek(gt_fifo_st * fifo, uint8_t * data, int32_t offset)
{
    if (offset >= gt_fifo_used(fifo)) {
        return false;
    }

    int32_t pos = (fifo->tail + offset) % fifo->size;
    *data = fifo->buffer[pos];

    return true;
}

/* end ------------------------------------------------------------------*/
