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
#include "./gt_serial_show.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static gt_res_t _command_decode_handler_cb(uint8_t const * const buffer, uint16_t len) {
    gt_serial_pack_buffer_st pack = gt_serial_get_temp_pack_buffer();
    uint8_t cmd_val = buffer[0];

    if (GT_SERIAL_COMMAND_READ_CMD == cmd_val) {
        pack.buffer[0] = GT_SERIAL_COMMAND_PASS_BACK_CMD;
        pack.len = 1;
        pack.len += gt_serial_get_value(&pack.buffer[1], (uint8_t * )&buffer[1], len - 1);
        if (pack.len < 2) {
            return GT_RES_INV;
        }
        return gt_serial_master_send_packet(pack.buffer, pack.len);
    }
    if (GT_SERIAL_COMMAND_WRITE_CMD == cmd_val) {
        gt_serial_set_value((uint8_t * )&buffer[1], len - 1);
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
