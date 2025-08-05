/**
 * @file gt_fifo.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2025-07-18
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_FIFO_H_
#define _GT_FIFO_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../others/gt_types.h"


/* define ---------------------------------------------------------------*/




/* typedef --------------------------------------------------------------*/
typedef struct gt_fifo_s {
    uint8_t * buffer;
    int32_t size;
    int32_t head;
    int32_t tail;
    bool _is_full : 1;
    bool _is_malloc_inside : 1;
    bool _is_init : 1;
}gt_fifo_st;



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief
 * @param fifo If buffer is NULL, it will be malloced by inside, _is_malloc_inside
 *          will be set to true, free by inside; otherwise, it will not malloc inside.
 *          Using static buffer by user.
 * @param size
 */
gt_res_t gt_fifo_init(gt_fifo_st * fifo, int32_t size);

void gt_fifo_free(gt_fifo_st * fifo);

int32_t gt_fifo_avail(gt_fifo_st * fifo);

/**
 * @return length
 */
int32_t gt_fifo_used(gt_fifo_st * fifo);

int32_t gt_fifo_get_size(gt_fifo_st * fifo);

bool gt_fifo_is_empty(gt_fifo_st * fifo);

void gt_fifo_reset(gt_fifo_st * fifo);

bool gt_fifo_push(gt_fifo_st * fifo, uint8_t data);

int32_t gt_fifo_push_multi(gt_fifo_st * fifo, const uint8_t * data, int32_t length);

bool gt_fifo_pop(gt_fifo_st * fifo, uint8_t * data);

int32_t gt_fifo_pop_multi(gt_fifo_st * fifo, uint8_t * data, int32_t length);

bool gt_fifo_peek(gt_fifo_st * fifo, uint8_t * data, int32_t offset);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_FIFO_H_
