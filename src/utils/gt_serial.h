/**
 * @file gt_serial.h
 * @author Feyoung
 * @brief Serial communication module, want to use this module,
 *      please check defined is enabled:
 *          @ref GT_USE_CRC
 *          @ref GT_ENABLED_CRC16
 *          @ref GT_QUEUE_USE_PRE_CHECK_VALID
 *          @ref GT_USE_SERIAL
 *      buffer size @ref GT_SERIAL_MASTER_CACHE_SIZE, @ref GT_SERIAL_CLIENT_CACHE_SIZE;
 *      unpack store buffer size @ref GT_SERIAL_UNPACK_CACHE_SIZE;
 *  Total RAM cost: GT_SERIAL_MASTER_CACHE_SIZE + GT_SERIAL_CLIENT_CACHE_SIZE + GT_SERIAL_UNPACK_CACHE_SIZE.
 *
 *  Crc16 enable set by @ref gt_serial_set_crc16(), Default: disabled;
 *
 *  serial headers byte content @ref GT_SERIAL_HEADER_BYTE;
 *  serial width byte length @ref GT_SERIAL_WIDTH_BYTE_LENGTH;
 *  serial valid auto pack by @ref gt_serial_client_send_packet, @ref gt_serial_client_send_packet()
 *  when enabled @ref GT_SERIAL_USE_AUTO_PACK;
 *
 *  HMI as master, send data to ohter device by @ref gt_serial_master_send();
 *  HMI as master, receive one byte data from other device by @ref gt_serial_master_recv(),
 *      get a complete packet of data(include: headers, length, crc16),
 *      by disabled @ref GT_SERIAL_GET_ONLY_VALID_DATA
 *
 *  HMI as client, ohter device send data to HMI by @ref gt_serial_client_send();
 *  HMI as client, receive one byte data from other device by @ref gt_serial_client_recv(),
 *      get a complete packet of data(include: headers, length, crc16),
 *      by disabled @ref GT_SERIAL_GET_ONLY_VALID_DATA
 *
 * @version 0.1
 * @date 2024-09-18 19:34:39
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SERIAL_H_
#define _GT_SERIAL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../gt_conf.h"

#if GT_USE_SERIAL
#include "stdbool.h"
#include "../others/gt_types.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
/**
 * @brief Serial protocol data buffer handler callback function
 * @param data_buffer a full serial protocol data buffer
 * @param data_len data byte length
 * @return gt_res_t GT_RES_OK: success; GT_RES_FAIL: failed;
 */
typedef gt_res_t ( * gt_serial_handler_cb_t)(uint8_t const * const, uint16_t);


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief Initialize the serial communication module, master and client
 *      buffer size @ref GT_SERIAL_MASTER_CACHE_SIZE, @ref GT_SERIAL_CLIENT_CACHE_SIZE;
 *      crc16 enable set by @ref gt_serial_set_crc16()
 */
void gt_serial_init(void);

/**
 * @brief Deinitialize the serial communication module
 */
void gt_serial_deinit(void);

#if GT_SERIAL_USE_TIMER_RECV_UNPACK
/**
 * @brief Set the serial protocol data buffer handler callback function,
 *      The user parses the serial port content in the callback function
 *
 * @param cb The callback function
 */
void gt_serial_set_handler_cb(gt_serial_handler_cb_t cb);
#endif

/**
 * @brief crc16 enable set by mod-bus mode, The last two bytes
 *      of the packet are the checksum data
 *
 * @param enable true: enable, false: disable
 */
void gt_serial_set_crc16(bool enable);

/**
 * @brief HMI as master, send data to ohter device
 *
 * @param data data buffer
 * @param len data length
 * @return gt_res_t GT_RES_OK: success; GT_RES_FAIL: failed;
 */
gt_res_t gt_serial_master_send(uint8_t const * const data, uint16_t len);

/**
 * @brief HMI as master, when GT_SERIAL_GET_ONLY_VALID_DATA is set 1,
 *      send real valid data and auto calc crc16 value and pack the data,
 *      such as:                  [0x83, 0x00, 0x10, 0x04]
 *      result: [0x5A, 0xA5, 0x06, 0x83, 0x00, 0x10, 0x04, 0x25, 0xA3];
 *      otherwise, send data directly.
 *
 * @param data data buffer
 * @param len data length
 * @return gt_res_t GT_RES_OK: success; GT_RES_FAIL: failed;
 */
gt_res_t gt_serial_master_send_packet(uint8_t const * const data, uint16_t len);

/**
 * @brief HMI as master, receive one byte data from other device,
 *      Get a complete packet of data, by @ref GT_QUEUE_USE_PRE_CHECK_VALID
 *      [default: 0x5A, 0xA5, 0xWH, 0xWL, data0 ... dataN, crc16_1, crc16_2 ],
 *      when GT_SERIAL_GET_ONLY_VALID_DATA is set to 1, the data will be [data0 ... dataN]
 *
 * @param res_buffer An array large enough to store bytes of data
 * @return uint16_t Valid data byte length
 */
uint16_t gt_serial_master_recv(uint8_t * res_buffer);

/**
 * @brief HMI as client, ohter device send data to HMI
 *
 * @param data data buffer
 * @param len data length
 * @return gt_res_t GT_RES_OK: success; GT_RES_FAIL: failed;
 */
gt_res_t gt_serial_client_send(uint8_t const * const data, uint16_t len);

/**
 * @brief HMI as client, when GT_SERIAL_GET_ONLY_VALID_DATA is set 1,
 *      send real valid data and auto calc crc16 value and pack the data,
 *      such as:                  [0x83, 0x00, 0x10, 0x04]
 *      result: [0x5A, 0xA5, 0x06, 0x83, 0x00, 0x10, 0x04, 0x25, 0xA3];
 *      otherwise, send data directly.
 *
 * @param data data buffer
 * @param len data length
 * @return gt_res_t GT_RES_OK: success; GT_RES_FAIL: failed;
 */
gt_res_t gt_serial_client_send_packet(uint8_t const * const data, uint16_t len);

/**
 * @brief HMI as client, receive one byte data from other device,
 *      Get a complete packet of data, by @ref GT_QUEUE_USE_PRE_CHECK_VALID
 *      [default: 0x5A, 0xA5, 0xWH, 0xWL, data0 ... dataN, crc16_1, crc16_2 ],
 *      when GT_SERIAL_GET_ONLY_VALID_DATA is set to 1, the data will be [data0 ... dataN]
 *
 * @param res_buffer An array large enough to store bytes of data
 * @return uint16_t Valid data byte length
 */
uint16_t gt_serial_client_recv(uint8_t * res_buffer);


#endif  /** GT_USE_SERIAL */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SERIAL_H_
