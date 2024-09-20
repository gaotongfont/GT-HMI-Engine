/**
 * @file gt_queue.h
 * @author Feyoung
 * @brief cycle queue
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
#include "../gt_conf.h"
#include "../others/gt_types.h"



/* define ---------------------------------------------------------------*/
#ifndef GT_QUEUE_USE_PRE_CHECK_VALID
    /**
     * @brief Use pre-check data by callback function, such as: check uart data is valid
     *      [default: 0]
     */
    #define GT_QUEUE_USE_PRE_CHECK_VALID    0
#endif



/* typedef --------------------------------------------------------------*/

#if GT_QUEUE_USE_PRE_CHECK_VALID
/**
 * @brief Describe
 */
struct gt_queue_s;

/**
 * @brief Get the valid data by index / offset
 * @param queue The object of gt_queue_check_valid_cb_t callback function param
 *          @ref gt_queue_check_valid_st -> queue
 * @param index 0 <= index < get_count_cb_t() - 1
 * @param res The data buffer to store the valid data
 */
typedef gt_res_t ( * get_value_cb_t)(struct gt_queue_s const * const, uint16_t, void * );

/**
 * @brief Get the number of valid elements in the queue
 * @param queue The object of gt_queue_check_valid_cb_t callback function param
 *          @ref gt_queue_check_valid_st -> queue
 */
typedef uint16_t ( * get_count_cb_t)(struct gt_queue_s const * const);

typedef struct gt_queue_check_valid_s {
    struct gt_queue_s const * const queue;
    get_count_cb_t get_count;
    get_value_cb_t get_value;
}gt_queue_check_valid_st;

/**
 * @brief
 * @ref gt_res_t    GT_RES_OK: success;
 *                  GT_RES_FAIL: failed;
 *                  GT_RES_INV: invalid, need to waiting for more data byte recv to pre-check
 * @ref valid_data_len  valid data byte length
 */
typedef struct gt_queue_check_valid_res_s {
    gt_res_t res;
    uint16_t valid_data_len;
}gt_queue_check_valid_res_st;

/**
 * @brief The callback function to pre-check the validity of the data
 * @param index The tail index of the queue
 * @param valid_count queue valid data count
 * @returns gt_res_t        GT_RES_OK: success;
 *                          GT_RES_FAIL: failed;
 *                          GT_RES_INV: invalid, need to waiting for more data byte recv to pre-check
 *          valid_data_len  valid data byte length
 */
typedef gt_queue_check_valid_res_st ( * gt_queue_check_valid_cb_t)(struct gt_queue_check_valid_s const * const);
#endif

typedef struct gt_queue_s {
    uint16_t head;          /** index of push position offset */
    uint16_t tail;          /** index of pop position offset */
    uint16_t el_count;      /** element_count */
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
 * @return gt_queue_check_valid_res_st:
 *          @ref gt_res_t           GT_RES_OK: success;
 *                                  GT_RES_FAIL: failed;
 *                                  GT_RES_INV: invalid, need to waiting for more data byte recv to pre-check
 *          @ref valid_data_len     valid data byte length
 */
gt_queue_check_valid_res_st gt_queue_pop_by_check_valid(gt_queue_st * queue, void * res);
#endif  /** GT_QUEUE_USE_PRE_CHECK_VALID */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_QUEUE_H_
