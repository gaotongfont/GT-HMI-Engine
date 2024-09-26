/**
 * @file gt_serial_command.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-09-26 16:03:02
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SERIAL_COMMAND_H_
#define _GT_SERIAL_COMMAND_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../utils/gt_serial.h"

#if GT_USE_SERIAL && GT_SERIAL_USE_TIMER_RECV_UNPACK


/* define ---------------------------------------------------------------*/
#ifndef GT_SERIAL_COMMAND_WRITE_CMD
    /* serial command write cmd */
    #define GT_SERIAL_COMMAND_WRITE_CMD  0x82
#endif

#ifndef GT_SERIAL_COMMAND_READ_CMD
    /* serial command read cmd */
    #define GT_SERIAL_COMMAND_READ_CMD   0x83
#endif

#ifndef GT_SERIAL_COMMAND_PASS_BACK_CMD
    /* serial command pass back cmd */
    #define GT_SERIAL_COMMAND_PASS_BACK_CMD  0x83
#endif

/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief serial command decode
 */
void gt_serial_command_init(void);


#endif  /** GT_USE_SERIAL */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SERIAL_COMMAND_H_
