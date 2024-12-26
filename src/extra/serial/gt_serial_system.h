/**
 * @file gt_serial_system.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-09-27 15:35:59
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SERIAL_SYSTEM_H_
#define _GT_SERIAL_SYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../utils/gt_serial.h"

#if GT_USE_SERIAL && GT_USE_BIN_CONVERT
#include "../../others/gt_types.h"



/* define ---------------------------------------------------------------*/

#define GT_SERIAL_SYS_ADDR_RESET            0x04
#define GT_SERIAL_SYS_ADDR_UPDATE_CMD       0x06
#define GT_SERIAL_SYS_ADDR_NOR_FLASH        0x08
#define GT_SERIAL_SYS_ADDR_VER              0x0E
#define GT_SERIAL_SYS_ADDR_RTC              0x10
#define GT_SERIAL_SYS_ADDR_PIC_NOW          0x14
#define GT_SERIAL_SYS_ADDR_GUI_STATUS       0x15
#define GT_SERIAL_SYS_ADDR_LED_NOW          0x31
#define GT_SERIAL_SYS_ADDR_AD0_7            0x32
#define GT_SERIAL_SYS_ADDR_LCD_HOR          0x7A
#define GT_SERIAL_SYS_ADDR_LCD_VER          0x7B
#define GT_SERIAL_SYS_ADDR_SYS_CONFIG       0x80
#define GT_SERIAL_SYS_ADDR_LED_CONFIG       0x82
#define GT_SERIAL_SYS_ADDR_PIC_SET          0x84
#define GT_SERIAL_SYS_ADDR_PWM0_SET         0x86
#define GT_SERIAL_SYS_ADDR_PWM0_OUT         0x92
#define GT_SERIAL_SYS_ADDR_PWM1_OUT         0x93
#define GT_SERIAL_SYS_ADDR_RTC_SET          0x9C
#define GT_SERIAL_SYS_ADDR_READ_WAE         0xA0
#define GT_SERIAL_SYS_ADDR_EXP_FLASH        0xAA
#define GT_SERIAL_SYS_ADDR_TP_CMD_VISIT     0xB0
#define GT_SERIAL_SYS_ADDR_TP_SIMULATE      0xD4
#define GT_SERIAL_SYS_ADDR_PTR_ICON_OVERLAY 0xD8
#define GT_SERIAL_SYS_ADDR_ICL_SWITCH       0xDE
#define GT_SERIAL_SYS_ADDR_CRC_CHECKSUM     0xE0
#define GT_SERIAL_SYS_ADDR_MUSIC_STREAM     0xF0
#define GT_SERIAL_SYS_ADDR_TP_DRAW_WINDOW   0xF4
#define GT_SERIAL_SYS_ADDR_DYNAMIC_CURVE    0x300
#define GT_SERIAL_SYS_ADDR_WIFI_CONNECT     0x400
#define GT_SERIAL_SYS_ADDR_MULTI_MEDIA      0x500
#define GT_SERIAL_SYS_ADDR_EXPAND_STORE     0x5C0
#define GT_SERIAL_SYS_ADDR_VALUE_CHANGE     0xF00


#ifndef GT_POINTER_ICON_ID
    #define GT_POINTER_ICON_ID              (-100)
#endif


/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief
 *
 * @param buffer 0x0000(address) + 0x00 ... (data)
 * @param len
 * @return gt_res_t
 */
gt_res_t gt_serial_system_set_value(uint8_t const * const buffer, uint32_t len);

/**
 * @brief
 *
 * @param res The buffer to store the read value
 * @param buffer  0x0000(address) + 0x00(len)
 * @param len buffer length
 * @return uint16_t valid read byte length
 */
uint16_t gt_serial_system_get_value(uint8_t * const res, uint8_t const * const buffer, uint32_t len);

/**
 * @brief
 *
 * @param res The buffer to store the read value
 * @param addr  0x0000
 * @param short_len 0x00 short or word length
 * @return uint16_t valid read byte length
 */
uint16_t gt_serial_system_get_value_by_addr(uint8_t * const res, uint16_t addr, uint8_t short_len);

#endif  /** GT_USE_SERIAL */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SERIAL_SYSTEM_H_
