/**
 * @file gt_serial_command.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-09-26 16:03:09
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_serial_command.h"

#if GT_USE_SERIAL && GT_SERIAL_USE_TIMER_RECV_UNPACK
#include "../../others/gt_log.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static gt_res_t _command_decode_handler_cb(uint8_t const * const buffer, uint16_t len) {
    uint8_t cmd_val = buffer[0];
    // GT_LOG_A("", "command > %x %x", buffer[0], buffer[1]);

    if (GT_SERIAL_COMMAND_WRITE_CMD == cmd_val) {
        // GT_LOG_A("", "command write");
    } else if (GT_SERIAL_COMMAND_READ_CMD == cmd_val) {
        // GT_LOG_A("", "command read");
    }

    return GT_RES_OK;
}


/* global functions / API interface -------------------------------------*/
void gt_serial_command_init(void)
{
    gt_serial_set_handler_cb(_command_decode_handler_cb);
}


/* end ------------------------------------------------------------------*/
#endif  /** GT_USE_SERIAL */
