/**
 * @file gt_crc.h
 * @author Feyoung
 * @brief CRC8 CRC16 CRC32 calculation
 * @version 0.1
 * @date 2024-09-11 13:56:33
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_CRC_H_
#define _GT_CRC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../gt_conf.h"

#if GT_USE_CRC
#include "stddef.h"
#include "./gt_types.h"


/* define ---------------------------------------------------------------*/
#ifndef GT_ENABLED_CRC8
    /**
     * @brief crc8 enable flag
     *  [default: 0]
     */
    #define GT_ENABLED_CRC8         0
#endif

#ifndef GT_ENABLED_CRC16
    /**
     * @brief crc16 enable flag
     *  [default: 1]
     */
    #define GT_ENABLED_CRC16        01
#endif

#ifndef GT_ENABLED_CRC32
    /**
     * @brief crc32 enable flag
     *  [default: 0]
     */
    #define GT_ENABLED_CRC32        0
#endif


#if GT_ENABLED_CRC8
    #define GT_CRC8_POLY            0x07
    #define GT_CRC8_START           0x00
#endif

#if GT_ENABLED_CRC16
    #define GT_CRC16_POLY           0xA001
    #define GT_CRC16_START          0x0000
    #define GT_CRC16_START_MOD_BUS  0xFFFF
#endif

#if GT_ENABLED_CRC32
    #define GT_CRC32_POLY       0xEDB88320ul
    #define GT_CRC_START_32     0xFFFFFFFFul
#endif

/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
#if GT_ENABLED_CRC8
/**
 * @brief calculates the 8 bit wide CRC of an input string of a
 *      given length.*
 * @param input_str
 * @param num_bytes
 * @return uint8_t
 */
uint8_t gt_crc8(const uint8_t * input_str, size_t num_bytes);

/**
 * @brief Given a data byte and the previous value of the CRC value, the function
 *      gt_update_crc8() calculates and returns the new actual CRC value of the data
 *      coming in.
 * @param crc The previous value of the CRC value
 * @param val The new data byte
 * @return uint8_t new crc value
 */
uint8_t gt_update_crc8(uint8_t crc, uint8_t val);
#endif  /** GT_ENABLED_CRC8 */


#if GT_ENABLED_CRC16
/**
 * @brief Calculates the 16 bits CRC16/IBM in one pass for a byte
 *      string of which the beginning has been passed to the function. The number of
 *      bytes to check is also a parameter. The number of the bytes in the string is
 *      limited by the constant SIZE_MAX.
 * @param input_str
 * @param num_bytes
 * @return uint16_t
 */
uint16_t gt_crc16(const uint8_t * input_str, size_t num_bytes);

/**
 * @brief Calculates the 16 bits Modbus CRC in one pass for a byte string
 *      of which the beginning has been passed to the function. The
 *      number of bytes to check is also a parameter.
 * @param input_str
 * @param num_bytes
 * @return uint16_t
 */
uint16_t gt_crc16_mod_bus(const uint8_t * input_str, size_t num_bytes);

/**
 * @brief Calculates a new CRC-16 value based on the previous
 *      value of the CRC and the next byte of data to be checked.
 * @param crc The previous value of the CRC value
 * @param c The next byte of the data to be checked
 * @return uint16_t
 */
uint16_t gt_update_crc16(uint16_t crc, uint8_t c);
#endif  /** GT_ENABLED_CRC16 */

#if GT_ENABLED_CRC32
/**
 * @brief Calculates in one pass the common 32 bit CRC value for
 *      a byte string that is passed to the function together with a parameter
 *      indicating the length.
 * @param input_str
 * @param num_bytes
 * @return uint32_t
 */
uint32_t gt_crc32(const uint8_t * input_str, size_t num_bytes);

/**
 * @brief Calculates a new CRC-32 value based on the
 *      previous value of the CRC and the next byte of the data to be checked.
 * @param crc The previous value of the CRC value
 * @param c The next byte of the data to be checked
 * @return uint32_t
 */
uint32_t gt_update_crc32(uint32_t crc, uint8_t c);
#endif  /** GT_ENABLED_CRC32 */


#endif  /** GT_USE_SRC */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_CRC_H_
