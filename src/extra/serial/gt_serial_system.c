/**
 * @file gt_serial_system.c
 * @author Feyoung
 * @brief system value api interface
 *      length unit: short(2byte)
 * @version 0.1
 * @date 2024-09-27 15:36:55
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_serial_system.h"

#if GT_USE_SERIAL && GT_USE_BIN_CONVERT
#include "./gt_serial.h"
#include "./gt_serial_var.h"
#include "../utils/gt_bin_convert.h"
#include "../../core/gt_mem.h"
#include "../../others/gt_log.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/

typedef struct {
    uint32_t addr;
    gt_serial_pack_buffer_st unpack;
    gt_serial_pack_buffer_st pack;
}_serial_sys_pack_st;

typedef gt_res_t ( * _system_write_cb_t)(_serial_sys_pack_st * const);

typedef uint16_t (*_system_read_cb_t)(_serial_sys_pack_st * const);

typedef struct {
    uint16_t addr;
    uint16_t len;
    _system_write_cb_t sys_write_cb;
    _system_read_cb_t sys_read_cb;
}_serial_system_st;

/* static variables -----------------------------------------------------*/
uint16_t _read_buffer_direct(_serial_sys_pack_st * const pack_p) {

    // pack_p->unpack.buffer[0] = GT_SERIAL_COMMAND_PASS_BACK_CMD;
    // gt_memcpy(&pack_p->unpack.buffer[1], buffer, );
    // gt_serial_master_send_packet();
    return 0;
}

gt_res_t _write_system_reset(_serial_sys_pack_st * const pack_p) {
    uint8_t reset[] = { 0x55, 0xaa, 0x5a, 0xa5 };
    for (uint8_t i = 0; i < 4; ++i) {
        // if (reset[i] != buffer[i]) {
        if (reset[i] != pack_p->unpack.buffer[i]) {
            return GT_RES_FAIL;
        }
    }
    /** reset system */

    return GT_RES_OK;
}

gt_res_t _write_update_cmd(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

/**
 * @brief
 *      write: 5AA5 0B 82 0008 A5 000002 1002 0002, value addr 0x1002 2 byte write to 0x000002
 *      read:  5AA5 0B 82 0008 5A 000002 2000 0002, value addr 0x2000 2 byte read from 0x000002
 * @param pack_p
 * @return gt_res_t
 */
gt_res_t _write_nor_flash(_serial_sys_pack_st * const pack_p) {
    uint8_t type = pack_p->unpack.buffer[0];
    uint32_t addr = gt_convert_parse_by_len(&pack_p->unpack.buffer[1], 3);
    if (0x5a == type) {
        /** read */

        return GT_RES_OK;
    }
    if (0xa5 != type) {
        return GT_RES_FAIL;
    }
    /** write */

    return GT_RES_OK;
}

// gt_res_t _write_uart2(_serial_sys_pack_st * const pack_p) {
//     return GT_RES_OK;
// }

/**
 * @brief 0x0f 1 byte version number
 *
 * @param pack_p
 * @param len
 * @return uint16_t
 */
uint16_t _read_ver(_serial_sys_pack_st * const pack_p) {
    GT_LOG_A("", "addr: %p, %d", pack_p->addr, pack_p->unpack.len);
    GT_LOG_ARR(pack_p->unpack.buffer, 0, pack_p->unpack.len);
    gt_serial_var_buffer_get(&pack_p->unpack, pack_p->pack.buffer, pack_p->unpack.len);

    pack_p->pack.len = pack_p->unpack.len;

    GT_LOG_A("", "_read_ver %d", pack_p->unpack.len);
    GT_LOG_ARR(pack_p->pack.buffer, 0, pack_p->pack.len);
    return 0;
}

/**
 * @brief
 *      写：5A A5 0B 82 0010 13 0A 01 00 0B 0C 0D 00
 *      文本RTC显示2019-10-01 11:12:13 SUN ，
 *      年系统自动补齐20；星期为英文显示，系统会自动换算。
 *
 * @param pack_p
 * @return gt_res_t
 */
gt_res_t _write_rtc(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

/**
 * @brief
 *      读：5A A5 04 83 00 10 04
 *      应答：5A A5 0C 83 00 10 04 13 0A 01 00 0B 0C 0D 00
 *
 * @param pack_p
 * @return uint16_t
 */
uint16_t _read_rtc(_serial_sys_pack_st * const pack_p) {
    // gt_serial_pack_buffer_st unpack = gt_serial_get_temp_pack_buffer();
    // if (len > unpack.len) {
    //     return GT_RES_FAIL;
    // }
    // unpack.buffer[0] = GT_SERIAL_COMMAND_PASS_BACK_CMD;
    // gt_memcpy(&unpack.buffer[1], buffer, );
    // gt_serial_master_send_packet();
    return 0;
}

uint16_t _read_pic_now(_serial_sys_pack_st * const pack_p) {

    return 0;
}

uint16_t _read_gui_status(_serial_sys_pack_st * const pack_p) {
    return 0;
}

// uint16_t _read_tp_status(_serial_sys_pack_st * const pack_p) {
//     return 0;
// }

uint16_t _read_led_now(_serial_sys_pack_st * const pack_p) {
    return 0;
}

uint16_t _read_ad0_ad7(_serial_sys_pack_st * const pack_p) {
    return 0;
}

uint16_t _read_lcd_hor(_serial_sys_pack_st * const pack_p) {
    return 0;
}

uint16_t _read_lcd_ver(_serial_sys_pack_st * const pack_p) {
    return 0;
}

gt_res_t _write_system_config(_serial_sys_pack_st * const pack_p) {
    GT_LOG_A("", "_write_system_config");
    return GT_RES_OK;
}

uint16_t _read_system_config(_serial_sys_pack_st * const pack_p) {
    GT_LOG_A("", "_read_system_config");

    return 0;
}

gt_res_t _write_led_config(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

uint16_t _read_led_config(_serial_sys_pack_st * const pack_p) {
    return 0;
}

gt_res_t _write_pic_set(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

uint16_t _read_pic_set(_serial_sys_pack_st * const pack_p) {
    return 0;
}

gt_res_t _write_pwm0_set(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

uint16_t _read_pwm0_set(_serial_sys_pack_st * const pack_p) {
    return 0;
}

// gt_res_t _write_pwm1_set(_serial_sys_pack_st * const pack_p) {
//     return GT_RES_OK;
// }

// uint16_t _read_pwm1_set(_serial_sys_pack_st * const pack_p) {
//     return 0;
// }

gt_res_t _write_pwm0_out(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

uint16_t _read_pwm0_out(_serial_sys_pack_st * const pack_p) {
    return 0;
}

gt_res_t _write_pwm1_out(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

uint16_t _read_pwm1_out(_serial_sys_pack_st * const pack_p) {
    return 0;
}

gt_res_t _write_rtc_set(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

uint16_t _read_rtc_set(_serial_sys_pack_st * const pack_p) {
    return 0;
}

gt_res_t _write_wae(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

uint16_t _read_wae(_serial_sys_pack_st * const pack_p) {
    return 0;
}

gt_res_t _write_expand_128mbit_flash(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

uint16_t _read_expand_128mbit_flash(_serial_sys_pack_st * const pack_p) {
    return 0;
}

gt_res_t _write_tp_cmd_visit(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

gt_res_t _write_tp_simulate(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

gt_res_t _write_pointer_icon_overlay(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

// gt_res_t _write_wae_switch(_serial_sys_pack_st * const pack_p) {
//     return GT_RES_OK;
// }

gt_res_t _write_icl_switch(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

gt_res_t _write_crc_checksum(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

uint16_t _read_crc_checksum(_serial_sys_pack_st * const pack_p) {
    return 0;
}

// uint16_t _read_area_single_bitmap_export(_serial_sys_pack_st * const pack_p) {
//     return 0;
// }

// uint16_t _read_page_overlay_switch(_serial_sys_pack_st * const pack_p) {
//     return 0;
// }

gt_res_t _write_music_stream(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

gt_res_t _write_tp_draw_window(_serial_sys_pack_st * const pack_p) {
    /** deprecated */
    return GT_RES_OK;
}

// gt_res_t _write_gui_stop_enabled(_serial_sys_pack_st * const pack_p) {
//     return GT_RES_OK;
// }

// gt_res_t _write_uart_download(_serial_sys_pack_st * const pack_p) {
//     return GT_RES_OK;
// }

// gt_res_t _write_fsk(_serial_sys_pack_st * const pack_p) {
//     return GT_RES_OK;
// }

// uint16_t _read_fsk(_serial_sys_pack_st * const pack_p) {
//     return 0;
// }

gt_res_t _write_dynamic_curve(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

uint16_t _read_dynamic_curve(_serial_sys_pack_st * const pack_p) {
    return 0;
}

// gt_res_t _write_dynamic_curve_channel1(_serial_sys_pack_st * const pack_p) {
//     return GT_RES_OK;
// }

// gt_res_t _write_dynamic_curve_channel2(_serial_sys_pack_st * const pack_p) {
//     return GT_RES_OK;
// }

// gt_res_t _write_dynamic_curve_channel3(_serial_sys_pack_st * const pack_p) {
//     return GT_RES_OK;
// }

// gt_res_t _write_dynamic_curve_channel4(_serial_sys_pack_st * const pack_p) {
//     return GT_RES_OK;
// }

// gt_res_t _write_dynamic_curve_channel5(_serial_sys_pack_st * const pack_p) {
//     return GT_RES_OK;
// }

// gt_res_t _write_dynamic_curve_channel6(_serial_sys_pack_st * const pack_p) {
//     return GT_RES_OK;
// }

// gt_res_t _write_dynamic_curve_channel7(_serial_sys_pack_st * const pack_p) {
//     return GT_RES_OK;
// }

// gt_res_t _write_dynamic_curve_channel8(_serial_sys_pack_st * const pack_p) {
//     return GT_RES_OK;
// }

gt_res_t _write_wifi_connect(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

uint16_t _read_wifi_connect(_serial_sys_pack_st * const pack_p) {
    return 0;
}

gt_res_t _write_multi_media(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

uint16_t _read_multi_media(_serial_sys_pack_st * const pack_p) {
    return 0;
}

gt_res_t _write_expand_store(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

uint16_t _read_expand_store(_serial_sys_pack_st * const pack_p) {
    return 0;
}

uint16_t _read_value_change(_serial_sys_pack_st * const pack_p) {
    return 0;
}

static GT_ATTRIBUTE_LARGE_RAM_ARRAY const _serial_system_st _system_list[] = {
    // { 0x00, 4, (_system_write_cb_t)NULL, (_system_read_cb_t)NULL },
    { 0x04, 2, _write_system_reset, (_system_read_cb_t)NULL },
    { 0x06, 2, _write_update_cmd, (_system_read_cb_t)NULL },
    { 0x08, 4, _write_nor_flash, (_system_read_cb_t)NULL },
    // { 0x0c, 2, _write_uart2, (_system_read_cb_t)NULL },
    { 0x0e, 2, (_system_write_cb_t)NULL, _read_ver },
    { 0x10, 4, _write_rtc, _read_rtc },
    { 0x14, 1, (_system_write_cb_t)NULL, _read_pic_now },
    { 0x15, 1, (_system_write_cb_t)NULL, _read_gui_status },
    // { 0x16, 4, (_system_write_cb_t)NULL, _read_tp_status },  // deprecated
    // { 0x1a, 23, (_system_write_cb_t)NULL, (_system_read_cb_t)NULL },
    { 0x31, 1, (_system_write_cb_t)NULL, _read_led_now },
    { 0x32, 8, (_system_write_cb_t)NULL, _read_ad0_ad7 },
    // { 0x3a, 64, (_system_write_cb_t)NULL, (_system_read_cb_t)NULL },
    { 0x7a, 1, (_system_write_cb_t)NULL, _read_lcd_hor },
    { 0x7b, 1, (_system_write_cb_t)NULL, _read_lcd_ver },
    // { 0x7c, 4, (_system_write_cb_t)NULL, (_system_read_cb_t)NULL },
    { 0x80, 2, _write_system_config, _read_system_config },
    { 0x82, 2, _write_led_config, _read_led_config },
    { 0x84, 2, _write_pic_set, _read_pic_set },
    { 0x86, 2, _write_pwm0_set, _read_pwm0_set },
    // { 0x88, 2, _write_pwm1_set, _read_pwm1_set },    // deprecated
    // { 0x8a, 10, (_system_write_cb_t)NULL, (_system_read_cb_t)NULL },
    { 0x92, 1, _write_pwm0_out, _read_pwm0_out },
    { 0x93, 1, _write_pwm1_out, _read_pwm1_out },
    // { 0x94, 9, (_system_write_cb_t)NULL, (_system_read_cb_t)NULL },
    { 0x9c, 4, _write_rtc_set, _read_rtc_set },
    { 0xa0, 4, _write_wae, _read_wae },
    // { 0xa4, 6, (_system_write_cb_t)NULL, (_system_read_cb_t)NULL },
    { 0xaa, 6, _write_expand_128mbit_flash, _read_expand_128mbit_flash },
    { 0xb0, 36, _write_tp_cmd_visit, (_system_read_cb_t)NULL },
    { 0xd4, 4, _write_tp_simulate, (_system_read_cb_t)NULL },
    { 0xd8, 4, _write_pointer_icon_overlay, (_system_read_cb_t)NULL },
    // { 0xdc, 2, _write_wae_switch, (_system_read_cb_t)NULL }, // deprecated
    { 0xde, 2, _write_icl_switch, (_system_read_cb_t)NULL },
    // { 0xdc, 4, (_system_write_cb_t)NULL, (_system_read_cb_t)NULL },
    { 0xe0, 2, _write_crc_checksum, _read_crc_checksum },
    // { 0xe2, 6, (_system_write_cb_t)NULL, _read_area_single_bitmap_export },  // deprecated
    // { 0xe8, 2, (_system_write_cb_t)NULL, _read_page_overlay_switch },        // deprecated
    // { 0xea, 6, (_system_write_cb_t)NULL, (_system_read_cb_t)NULL },
    { 0xf0, 4, _write_music_stream, (_system_read_cb_t)NULL },
    { 0xf4, 8, _write_tp_draw_window, (_system_read_cb_t)NULL },
    // deprecated
    // { 0xfc, 2, _write_gui_stop_enabled, (_system_read_cb_t)NULL },
    // { 0xfe, 2, _write_uart_download, (_system_read_cb_t)NULL },
    // { 0x100, 512, _write_fsk, _read_fsk },
    { 0x300, 128, _write_dynamic_curve, _read_dynamic_curve },
    // deprecated
    // { 0x380, 2, _write_dynamic_curve_channel1, (_system_read_cb_t)NULL },
    // { 0x382, 2, _write_dynamic_curve_channel2, (_system_read_cb_t)NULL },
    // { 0x384, 2, _write_dynamic_curve_channel3, (_system_read_cb_t)NULL },
    // { 0x386, 2, _write_dynamic_curve_channel4, (_system_read_cb_t)NULL },
    // { 0x388, 2, _write_dynamic_curve_channel5, (_system_read_cb_t)NULL },
    // { 0x38a, 2, _write_dynamic_curve_channel6, (_system_read_cb_t)NULL },
    // { 0x38c, 2, _write_dynamic_curve_channel7, (_system_read_cb_t)NULL },
    // { 0x38e, 2, _write_dynamic_curve_channel8, (_system_read_cb_t)NULL },
    // // { 0x390, 112, (_system_write_cb_t)NULL, (_system_read_cb_t)NULL },
    { 0x400, 256, _write_wifi_connect, _read_wifi_connect },
    { 0x500, 192, _write_multi_media, _read_multi_media },
    { 0x5c0, 64, _write_expand_store, _read_expand_store },
    // { 0x600, 2404, (_system_write_cb_t)NULL, (_system_read_cb_t)NULL },
    { 0xf00, 2, (_system_write_cb_t)NULL, _read_value_change },
    // { 0xf02, 254, (_system_write_cb_t)NULL, (_system_read_cb_t)NULL },
};


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_res_t gt_serial_system_set_value(uint8_t const * const buffer, uint32_t len)
{
    uint16_t offset = 0;
    uint16_t i,  count = sizeof(_system_list) / sizeof(_serial_system_st);
    _serial_sys_pack_st pack_info = {
        .addr = gt_convert_parse_u16(buffer),
        .unpack = {
            .buffer = (uint8_t * )buffer + GT_VAR_REG_LEN,
            .len = len - GT_VAR_REG_LEN
        },
    };
    _serial_system_st const * const sys = (_serial_system_st const * const)_system_list;
    gt_res_t ret = GT_RES_OK;

    for (i = 0; i < count; ++i) {
        if (pack_info.addr < sys[i].addr || pack_info.addr >= sys[i].addr + sys[i].len) {
            continue;
        }
        if (sys[i].sys_write_cb) {
            ret = sys[i].sys_write_cb(&pack_info);
            if (GT_RES_FAIL == ret) {
                GT_LOGE(GT_LOG_TAG_SERIAL, "write system failed! addr: 0x%X", pack_info.addr);
            }
        }
        break;
    }
    return ret;
}

uint16_t gt_serial_system_get_value(uint8_t * const res, uint8_t const * const buffer, uint32_t len)
{
    uint16_t offset = 0;
    uint16_t i,  count = sizeof(_system_list) / sizeof(_serial_system_st);
    _serial_sys_pack_st pack_info = {
        .addr = gt_convert_parse_u16(buffer),
        .unpack = {
            .buffer = (uint8_t * )buffer,
            .len = len
        },
        .pack = {
            .buffer = res,
            .len = 0,
        }
    };
    _serial_system_st const * const sys = (_serial_system_st const * const)_system_list;
    uint16_t byte_size = 0;

    for (i = 0; i < count; ++i) {
        if (pack_info.addr < sys[i].addr || pack_info.addr >= sys[i].addr + sys[i].len) {
            continue;
        }
        if (sys[i].sys_read_cb) {
            byte_size = sys[i].sys_read_cb(&pack_info);
        }
        break;
    }
    return byte_size;
}


/* end ------------------------------------------------------------------*/
#endif /** GT_USE_SERIAL && GT_USE_SERIAL_SYSTEM */
