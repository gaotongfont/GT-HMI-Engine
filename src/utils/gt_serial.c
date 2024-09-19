/**
 * @file gt_serial.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-09-18 19:37:34
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "stddef.h"
#include "./gt_serial.h"

#if GT_USE_SERIAL
#include "./gt_queue.h"
#include "../others/gt_log.h"
#include "../core/gt_mem.h"
#include "../others/gt_crc.h"


/* private define -------------------------------------------------------*/
#define _SERIAL_CRC16_BYTE_LEN   2



/* private typedef ------------------------------------------------------*/
typedef struct _serial_reg_s {
    uint8_t crc16 : 1;
}_serial_reg_st;


typedef struct gt_serial_s {
    gt_queue_st * master;   /** HMI as master */
    gt_queue_st * client;   /** HMI as client */

    _serial_reg_st reg;

    uint16_t headers_len;
    uint8_t headers[];
}gt_serial_st;


/* static variables -----------------------------------------------------*/
static GT_ATTRIBUTE_LARGE_RAM_ARRAY uint8_t _serial_master_cache[GT_SERIAL_MASTER_CACHE_SIZE] = {0};

static GT_ATTRIBUTE_LARGE_RAM_ARRAY uint8_t _serial_client_cache[GT_SERIAL_CLIENT_CACHE_SIZE] = {0};

static gt_serial_st _serial = {
    .master = NULL,
    .client = NULL,
    .reg = {
        .crc16 = false,
    },
    .headers_len = 0,
    .headers = GT_SERIAL_HEADER_BYTE,
};

/* macros ---------------------------------------------------------------*/

static inline uint16_t _serial_checksum_switch_byte(uint16_t value) {
    return ((value & 0xff) << 8) | (value >> 8);
}

static inline uint16_t _serial_get_crc16_byte_length(void) {
    return _serial.reg.crc16 ? _SERIAL_CRC16_BYTE_LEN : 0;
}

/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
/**
 * @brief Check package data is valid or not
 *
 * @param cv param
 * @return gt_queue_check_valid_res_st:
 * @returns gt_res_t        GT_RES_OK: success;
 *                          GT_RES_FAIL: failed;
 *                          GT_RES_INV: invalid, need to waiting for more data byte recv to pre-check
 *          valid_data_len  valid data byte length
 */
static gt_queue_check_valid_res_st _serial_queue_check_valid_cb(gt_queue_check_valid_st const * const cv) {
    gt_queue_check_valid_res_st ret = {
        .res = GT_RES_FAIL,
        .valid_data_len = 0,
    };
	uint16_t count = cv->get_count(cv->queue);
    uint16_t i = 0, idx = 0;
	uint8_t data = 0;

    /** check valid headers content */
    for (i = 0; i < _serial.headers_len; ++i) {
        cv->get_value(cv->queue, idx++, &data);
        if (_serial.headers[i] != data) {
            return ret;
        }
    }

    /** check valid byte data has recv done */
    for (i = 0; i < GT_SERIAL_WIDTH_BYTE_LENGTH; ++i) {
        ret.valid_data_len <<= 8;
        cv->get_value(cv->queue, idx++, &data);
        ret.valid_data_len |= data;
    }

    ret.valid_data_len += _serial.headers_len + GT_SERIAL_WIDTH_BYTE_LENGTH;
    if (count < ret.valid_data_len) {
        /** Wait for the complete data */
        ret.res = GT_RES_INV;
        return ret;
    }
    /** check crc16 */
#if GT_ENABLED_CRC16
    if (_serial.reg.crc16) {
        uint16_t checksum = GT_CRC16_START_MOD_BUS;
        uint16_t target = 0;
        uint16_t end = ret.valid_data_len - _serial_get_crc16_byte_length();

        for (i = _serial.headers_len + GT_SERIAL_WIDTH_BYTE_LENGTH; i < end; ++i) {
            cv->get_value(cv->queue, i, &data);
            checksum = gt_update_crc16(checksum, data);
        }
        checksum = _serial_checksum_switch_byte(checksum);

        for (i = end; i < ret.valid_data_len; ++i) {
            target <<= 8;
            cv->get_value(cv->queue, i, &data);
            target |= data;
        }

        if (checksum != target) {
            GT_LOGW(GT_LOG_TAG_SERIAL, "Failed crc16 calc: %x, target: %x", checksum, target);
            return ret;
        }
    }
#endif
    ret.res = GT_RES_OK;
    return ret;
}

static gt_res_t _serial_common_send(gt_queue_st * queue, uint8_t const * const data, uint16_t len) {
    if (NULL == data || 0 == len) {
        return GT_RES_FAIL;
    }
    uint8_t * p = (uint8_t *)data;
    gt_res_t ret = GT_RES_OK;

    for (uint16_t i = 0; i < len; ++i) {
        ret = gt_queue_push(queue, p++);
        if (GT_RES_OK != ret) {
            break;
        }
    }
    return ret;
}

/**
 * @brief Unpack the valid data from the queue,
 *      such as: [0x5A, 0xA5, 0x83, 0x00, 0x10, 0x04, 0x25, 0xA3],
 *      result:              [0x83, 0x00, 0x10, 0x04, 0x00]
 *
 * @param ret_p
 * @param res_buffer
 * @return uint16_t
 */
static uint16_t _serial_common_unpack_valid_data(gt_queue_check_valid_res_st * ret_p, uint8_t * res_buffer) {
#if GT_SERIAL_GET_ONLY_VALID_DATA
    uint16_t i = 0;
    uint16_t offset = _serial.headers_len + GT_SERIAL_WIDTH_BYTE_LENGTH;

    ret_p->valid_data_len -= offset + _serial_get_crc16_byte_length();
    for (i = 0; i < ret_p->valid_data_len; ++i) {
        res_buffer[i] = res_buffer[offset + i];
    }
    res_buffer[i] = 0;
#endif
    return ret_p->valid_data_len;
}

#if GT_SERIAL_GET_ONLY_VALID_DATA
static gt_res_t _serial_common_push_integer(gt_queue_st * queue, uint32_t value, uint16_t byte_len) {
    uint16_t i = 0;
    uint8_t length_arr[4] = {0};
    gt_res_t ret = GT_RES_OK;
    if (0 == byte_len) {
        return ret;
    }

    for (i = 0; i < byte_len; ++i) {
        length_arr[i] = (value >> (8 * (byte_len - i - 1))) & 0xff;
    }
    ret = _serial_common_send(queue, length_arr, byte_len);
    if (GT_RES_OK != ret) {
        return ret;
    }
    return ret;
}

/**
 * @brief Send real valid data and auto calc crc16 value and pack the data,
 *      such as:            [0x83, 0x00, 0x10, 0x04],
 *      result: [0x5A, 0xA5, 0x83, 0x00, 0x10, 0x04, 0x25, 0xA3]
 *
 * @param queue
 * @param data
 * @param len data length
 * @return gt_res_t
 */
static gt_res_t _serial_common_send_pack(gt_queue_st * queue, uint8_t const * const data, uint16_t len) {
    uint16_t i = 0, width_len = GT_SERIAL_WIDTH_BYTE_LENGTH;
    uint32_t checksum = 0;
    gt_res_t ret = GT_RES_OK;

    /** push headers */
    ret = _serial_common_send(queue, _serial.headers, _serial.headers_len);
    if (GT_RES_OK != ret) {
        return ret;
    }

    /** push length */
    uint32_t value = len + _serial_get_crc16_byte_length();
    ret = _serial_common_push_integer(queue, value, width_len);
    if (GT_RES_OK != ret) {
        return ret;
    }

    /** push data and calc checksum value */
    uint8_t * p = (uint8_t *)data;
#if GT_ENABLED_CRC16
    checksum = GT_CRC16_START_MOD_BUS;
#endif
    for (i = 0; i < len; ++i) {
        ret = gt_queue_push(queue, p);
        if (GT_RES_OK != ret) {
            return ret;
        }
#if GT_ENABLED_CRC16
        checksum = gt_update_crc16(checksum, *p++);
#endif
    }
#if GT_ENABLED_CRC16
    checksum = _serial_checksum_switch_byte(checksum);
#endif

    /** push checksum value */
    return _serial_common_push_integer(queue, checksum, _serial_get_crc16_byte_length());
}
#endif

/* global functions / API interface -------------------------------------*/
void gt_serial_init(void)
{
    _serial.master = gt_queue_init(sizeof(uint8_t), _serial_master_cache, GT_SERIAL_MASTER_CACHE_SIZE);
    _serial.client = gt_queue_init(sizeof(uint8_t), _serial_client_cache, GT_SERIAL_CLIENT_CACHE_SIZE);
    _serial.headers_len = strlen(_serial.headers);

    gt_queue_set_check_valid_cb(_serial.master, _serial_queue_check_valid_cb);
    gt_queue_set_check_valid_cb(_serial.client, _serial_queue_check_valid_cb);

    GT_LOG_A(GT_LOG_TAG_SERIAL, "Serial header byte len: %d", _serial.headers_len);
    GT_LOG_ARR(_serial.headers, 0, _serial.headers_len);
}

void gt_serial_deinit(void)
{
    if (_serial.master) {
        gt_queue_deinit(_serial.master);
        _serial.master = NULL;
    }
    if (_serial.client) {
        gt_queue_deinit(_serial.client);
        _serial.client = NULL;
    }
}

void gt_serial_set_crc16(bool enable)
{
    _serial.reg.crc16 = enable;
}

gt_res_t gt_serial_master_send(uint8_t const * const data, uint16_t len)
{
    return _serial_common_send(_serial.master, data, len);
}

gt_res_t gt_serial_master_send_packet(uint8_t const * const data, uint16_t len)
{
#if GT_SERIAL_GET_ONLY_VALID_DATA
    return _serial_common_send_pack(_serial.master, data, len);
#else
    return _serial_common_send(_serial.master, data, len);
#endif
}

uint16_t gt_serial_master_recv(uint8_t * res_buffer)
{
	gt_queue_check_valid_res_st ret = gt_queue_pop_by_check_valid(_serial.master, res_buffer);
    if (GT_RES_INV == ret.res) {
        return 0;
    } else if (GT_RES_FAIL == ret.res) {
        GT_LOGW(GT_LOG_TAG_SERIAL, "Master queue pop failed!");
        return 0;
    }
    return _serial_common_unpack_valid_data(&ret, res_buffer);
}

gt_res_t gt_serial_client_send(uint8_t const * const data, uint16_t len)
{
    return _serial_common_send(_serial.client, data, len);
}

gt_res_t gt_serial_client_send_packet(uint8_t const * const data, uint16_t len)
{
#if GT_SERIAL_GET_ONLY_VALID_DATA
    return _serial_common_send_pack(_serial.client, data, len);
#else
    return _serial_common_send(_serial.client, data, len);
#endif
}

uint16_t gt_serial_client_recv(uint8_t * res_buffer)
{
	gt_queue_check_valid_res_st ret = gt_queue_pop_by_check_valid(_serial.client, res_buffer);
    if (GT_RES_INV == ret.res) {
        return 0;
    } else if (GT_RES_FAIL == ret.res) {
        GT_LOGW(GT_LOG_TAG_SERIAL, "Client queue pop failed!");
        return 0;
    }
    return _serial_common_unpack_valid_data(&ret, res_buffer);
}

/* end ------------------------------------------------------------------*/
#endif  /** GT_USE_SERIAL */
