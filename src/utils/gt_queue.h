/**
 * @file gt_queue.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-09-14 15:48:02
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_QUEUE_H_
#define _GT_QUEUE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../others/gt_types.h"



/* define ---------------------------------------------------------------*/
#ifndef GT_QUEUE_USE_PRE_CHECK_VALID
    /**
     * @brief Use pre-check data by callback function
     *      [default: 0]
     */
    #define GT_QUEUE_USE_PRE_CHECK_VALID    01
#endif



/* typedef --------------------------------------------------------------*/

#if GT_QUEUE_USE_PRE_CHECK_VALID
/**
 * @brief The callback function to pre-check the validity of the data
 * @param ptr The queue cache buffer pointer
 * @param valid_byte_len The number of bytes to be processed
 * @return gt_res_t GT_RES_OK: success;
 *                  GT_RES_FAIL: failed;
 *                  GT_RES_INV: invalid, need to waiting for more data byte recv to pre-check
 */
typedef gt_res_t ( * gt_queue_check_valid_cb_t)(uint8_t const * const, uint16_t);
#endif

typedef struct gt_queue_s {
    uint16_t head;          /** index of push position offset */
    uint16_t tail;          /** index of pop position offset */
    uint16_t num_byte;      /** number of bytes = element_count * instance */
    uint16_t instance;      /** element instance byte size */

    void * buffer;          /** queue cache buffer */
    uint32_t byte_size;     /** queue cache buffer byte length */
    uint16_t max_count;     /** max count = byte_size / instance */

#if GT_QUEUE_USE_PRE_CHECK_VALID
    /**
     * @brief Use to pre-check data is valid.
     */
    gt_queue_check_valid_cb_t check_valid_cb;
#endif
}gt_queue_st;


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief Init queue
 *
 * @param instance The element byte size, must be greater than 0
 * @param buffer Extra array buffer to cache queue data
 * @param byte_size Extra array buffer byte length
 *                  max count = byte_size / instance
 * @return gt_queue_st * [Warn] Must be use @ref gt_queue_deinit() to free memory
 */
gt_queue_st * gt_queue_init(uint16_t instance, void * buffer, uint32_t byte_size);

/**
 * @brief Must be use to free memory
 *
 * @param queue
 * @return gt_res_t
 */
gt_res_t gt_queue_deinit(gt_queue_st * queue);

bool gt_queue_is_empty(gt_queue_st * queue);

/**
 * @brief Get the number of elements in the queue
 *
 * @param queue
 * @return uint16_t The number of elements in the queue
 */
uint16_t gt_queue_get_count(gt_queue_st * queue);

/**
 * @brief
 *
 * @param queue
 * @return gt_res_t
 */
gt_res_t gt_queue_clear(gt_queue_st * queue);

/**
 * @brief Push data into queue
 *
 * @param queue
 * @param data The data want to push into queue, must be instance byte size as @ref gt_queue_init()
 * @return gt_res_t GT_RES_OK: success; GT_RES_FAIL: failed;
 */
gt_res_t gt_queue_push(gt_queue_st * queue, void * data);

/**
 * @brief Pop data from queue
 *
 * @param queue
 * @param res
 * @return gt_res_t GT_RES_OK: success; GT_RES_FAIL: failed; GT_RES_INV: invalid;
 */
gt_res_t gt_queue_pop(gt_queue_st * queue, void * res);


#if GT_QUEUE_USE_PRE_CHECK_VALID
/**
 * @brief Set the callback function to pre-check the validity of the data
 *
 * @param queue
 * @param check_valid_cb The callback function
 * @return gt_res_t GT_RES_OK: success;
 *                  GT_RES_FAIL: failed;
 */
gt_res_t gt_queue_set_check_valid_cb(gt_queue_st * queue, gt_queue_check_valid_cb_t check_valid_cb);

/**
 * @brief Pop the valid data by include pre-check callback function checked
 *
 * @param queue
 * @param res The data buffer to store the valid data
 * @return uint16_t valid data byte length, 0: no valid data
 */
uint16_t gt_queue_pop_by_check_valid(gt_queue_st * queue, void * res);

#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_QUEUE_H_
