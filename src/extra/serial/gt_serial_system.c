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
#include "./gt_serial_show.h"
#include "./gt_serial_cfg.h"
#include "../../core/gt_indev.h"
#include "../../widgets/gt_img.h"
#include "../../core/gt_disp.h"
#include "../../core/gt_style.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/

typedef struct {
    uint32_t addr;
    gt_serial_pack_buffer_st unpack;    /** need to unpack data */
    gt_serial_pack_buffer_st pack;      /** want to pack up data */
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
static gt_res_t _write_buffer_direct(_serial_sys_pack_st * const pack_p) {
    gt_var_value_st var_value = {
        .buffer = pack_p->unpack.buffer,
        .len = pack_p->unpack.len,
    };
    if (gt_serial_var_buffer_set(&var_value)) {
        return GT_RES_FAIL;
    }
    return GT_RES_OK;
}

/**
 * @brief format: 0x00f0 0x01
 *                 addr + len
 *
 * @param pack_p
 * @return uint16_t
 */
static uint16_t _read_buffer_direct(_serial_sys_pack_st * const pack_p) {
    uint16_t len = 0, need_read = 0;

    need_read = pack_p->unpack.buffer[pack_p->unpack.len - 1] << 1; // short 0x01 -> 2 byte
    gt_memcpy(&pack_p->pack.buffer[len], pack_p->unpack.buffer, pack_p->unpack.len);
    len += pack_p->unpack.len;

    gt_serial_var_buffer_get(&pack_p->unpack, &pack_p->pack.buffer[len], need_read);
    pack_p->pack.len = len + need_read;

    return pack_p->pack.len;
    return 0;
}

static gt_res_t _write_system_reset(_serial_sys_pack_st * const pack_p) {
    uint8_t reset[] = { 0x55, 0xaa, 0x5a, 0xa5 };
    for (uint8_t i = 0; i < 4; ++i) {
        if (reset[i] != pack_p->unpack.buffer[i]) {
            return GT_RES_FAIL;
        }
    }
    /** reset system */

    return GT_RES_OK;
}

static gt_res_t _write_update_cmd(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

/**
 * @brief
 *      write: 5AA5 0B 82 0008 A5 000002 1002 0002, value addr 0x1002 2 byte write to 0x000002
 *      read:  5AA5 0B 82 0008 5A 000002 2000 0002, value addr 0x2000 2 byte read from 0x000002
 * @param pack_p
 * @return gt_res_t
 */
static gt_res_t _write_nor_flash(_serial_sys_pack_st * const pack_p) {
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
static uint16_t _read_ver(_serial_sys_pack_st * const pack_p) {
    return _read_buffer_direct(pack_p);
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
static gt_res_t _write_rtc(_serial_sys_pack_st * const pack_p) {
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
static uint16_t _read_rtc(_serial_sys_pack_st * const pack_p) {
    return _read_buffer_direct(pack_p);
}

static uint16_t _read_pic_now(_serial_sys_pack_st * const pack_p) {
    return _read_buffer_direct(pack_p);
}

/**
 * @brief 0x0000: idle; 0x0001: busy, handle 13.bin / 14.bin
 *
 * @param pack_p
 * @return uint16_t
 */
static uint16_t _read_gui_status(_serial_sys_pack_st * const pack_p) {
    return _read_buffer_direct(pack_p);
}

// uint16_t _read_tp_status(_serial_sys_pack_st * const pack_p) {
//     return 0;
// }

static uint16_t _read_led_now(_serial_sys_pack_st * const pack_p) {
    return _read_buffer_direct(pack_p);
}

static uint16_t _read_ad0_ad7(_serial_sys_pack_st * const pack_p) {
    return 0;
}

static uint16_t _read_lcd_hor(_serial_sys_pack_st * const pack_p) {
    return _read_buffer_direct(pack_p);
}

static uint16_t _read_lcd_ver(_serial_sys_pack_st * const pack_p) {
    return _read_buffer_direct(pack_p);
}

static gt_res_t _write_system_config(_serial_sys_pack_st * const pack_p) {
    bool is_load = 0x5A == pack_p->unpack.buffer[0] ? true : false;
    if (!is_load) {
        return GT_RES_OK;
    }
    gt_serial_cfg_st * const cfg = gt_serial_cfg_get_root();
    uint8_t d0 = pack_p->unpack.buffer[5];
    cfg->system_config.param_0.touch_auto_upload = (d0 >> 5) & 0x01;
    cfg->system_config.param_0.touch_beep = (d0 >> 4) & 0x01;
    cfg->system_config.param_0.touch_back_light_sleep = (d0 >> 3) & 0x01;
    cfg->system_config.param_0.disp_direct = d0 & 0x03;
    return _write_buffer_direct(pack_p);
}

static uint16_t _read_system_config(_serial_sys_pack_st * const pack_p) {
    return _read_buffer_direct(pack_p);
}

static gt_res_t _write_led_config(_serial_sys_pack_st * const pack_p) {
    gt_serial_cfg_st * const cfg = gt_serial_cfg_get_root();
    cfg->backlight_sleep.brightness = pack_p->unpack.buffer[0];
    cfg->backlight_sleep.brightness_sleep = pack_p->unpack.buffer[1];
    cfg->backlight_sleep.wake_up_time = gt_convert_parse_u16(&pack_p->unpack.buffer[2]);
    return _write_buffer_direct(pack_p);
}

static uint16_t _read_led_config(_serial_sys_pack_st * const pack_p) {
    return _read_buffer_direct(pack_p);
}

/**
 * @brief 0x5a01 0x0001
 *
 * @param pack_p
 * @return gt_res_t
 */
static gt_res_t _write_pic_set(_serial_sys_pack_st * const pack_p) {
    bool is_load = 0x5A == pack_p->unpack.buffer[0] ? true : false;
    if (!is_load) {
        return GT_RES_OK;
    }
    uint16_t idx = gt_convert_parse_u16(&pack_p->unpack.buffer[2]);
    gt_serial_load_page(idx);
    pack_p->unpack.buffer[0] = 0x00;
    return _write_buffer_direct(pack_p);
}

static uint16_t _read_pic_set(_serial_sys_pack_st * const pack_p) {
    return _read_buffer_direct(pack_p);
}

static gt_res_t _write_pwm0_set(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

static uint16_t _read_pwm0_set(_serial_sys_pack_st * const pack_p) {
    return 0;
}

// static gt_res_t _write_pwm1_set(_serial_sys_pack_st * const pack_p) {
//     return GT_RES_OK;
// }

// static uint16_t _read_pwm1_set(_serial_sys_pack_st * const pack_p) {
//     return 0;
// }

static gt_res_t _write_pwm0_out(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

static uint16_t _read_pwm0_out(_serial_sys_pack_st * const pack_p) {
    return 0;
}

static gt_res_t _write_pwm1_out(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

static uint16_t _read_pwm1_out(_serial_sys_pack_st * const pack_p) {
    return 0;
}

static gt_res_t _write_rtc_set(_serial_sys_pack_st * const pack_p) {
    if (0x5A != pack_p->unpack.buffer[0]) {
        return GT_RES_OK;
    }
    if (0xA5 != pack_p->unpack.buffer[1]) {
        return GT_RES_OK;
    }
    /** Y-M-D h-m-s */

    return _write_buffer_direct(pack_p);
}

static gt_res_t _write_wae(_serial_sys_pack_st * const pack_p) {
    return GT_RES_OK;
}

static uint16_t _read_wae(_serial_sys_pack_st * const pack_p) {
    return 0;
}

/**
 * @brief  0x5a +
 *          (read flash data)
 *          1. 0x01(read) 0x00(font id) 0x000000(font addr) 0x0000(var_addr even number) 0x0000(var length even number) 0x0000
 *          (write 32kB data to flash)
 *          2. 0x02(write) 0x0000(32kB data addr/pre 16MB flash) 0x0000(save addr) 0x0000(delay  1ms) 0x0000
 *          (copy font data)
 *          3. 0x03(copy) 0x0000(copy from id / 256kB) 0x0000(copy to) 0x0000(copy times) 0x000000
 *          (write 4kB  to expand nand flash)
 *          4. 0x04(write) 0x000000(to 4kB data addr) 0x0000(from vp buffer) 0x00(4kB count) 0x0000
 *          (nand  32bit crc checksum)
 *          5. 0x05(crc) 0x0000 0000 (low 12bit is 0, 4kB align) 0x0000(count <= 0x8000) 0x00000000(crc value)
 *          (nand flash copy)
 *          6. 0x06(copy) 0x0000 0000 (low 12bit is 0, 4kB align) 0x0000(target id <= 0x00ff) 0x0000(count) 0x0000
 * @param pack_p
 * @return gt_res_t
 */
static gt_res_t _write_expand_128mbit_flash(_serial_sys_pack_st * const pack_p) {
    if (0x5A != pack_p->unpack.buffer[0]) {
        return GT_RES_OK;
    }
    gt_serial_cfg_st * const cfg = gt_serial_cfg_get_root();
    cfg->expand_flash_hd.is_running = true;
    cfg->expand_flash_hd.handler_type = pack_p->unpack.buffer[1];
    switch (cfg->expand_flash_hd.handler_type)
    {
        case 0x01: {
            cfg->expand_flash_hd.font_id = pack_p->unpack.buffer[2];
            cfg->expand_flash_hd.addr_or_id = gt_convert_parse_by_len(&pack_p->unpack.buffer[3], 3);
            cfg->expand_flash_hd.var_addr = gt_convert_parse_u16(&pack_p->unpack.buffer[6]);
            cfg->expand_flash_hd.count_block = gt_convert_parse_u16(&pack_p->unpack.buffer[8]);
            break;
        }
        case 0x02: {
            cfg->expand_flash_hd.addr_or_id = gt_convert_parse_u16(&pack_p->unpack.buffer[2]);
            cfg->expand_flash_hd.var_addr = gt_convert_parse_u16(&pack_p->unpack.buffer[4]);
            cfg->expand_flash_hd.delay_to_next = gt_convert_parse_u16(&pack_p->unpack.buffer[6]);
            break;
        }
        case 0x03: {
            cfg->expand_flash_hd.font_id = gt_convert_parse_u16(&pack_p->unpack.buffer[2]);
            cfg->expand_flash_hd.addr_or_id = gt_convert_parse_u16(&pack_p->unpack.buffer[4]);
            cfg->expand_flash_hd.count_block = gt_convert_parse_u16(&pack_p->unpack.buffer[6]);
            break;
        }
        case 0x04: {
            cfg->expand_flash_hd.addr_or_id = gt_convert_parse_u32(&pack_p->unpack.buffer[2]);
            cfg->expand_flash_hd.var_addr = gt_convert_parse_u16(&pack_p->unpack.buffer[6]);
            cfg->expand_flash_hd.count_block = pack_p->unpack.buffer[8];
            break;
        }
        case 0x05: {
            cfg->expand_flash_hd.addr_or_id = gt_convert_parse_u32(&pack_p->unpack.buffer[2]);
            cfg->expand_flash_hd.count_block = gt_convert_parse_u16(&pack_p->unpack.buffer[6]);
            cfg->expand_flash_hd.crc32_checksum = gt_convert_parse_u32(&pack_p->unpack.buffer[8]);
            break;
        }
        case 0x06: {
            cfg->expand_flash_hd.addr_or_id = gt_convert_parse_u32(&pack_p->unpack.buffer[2]);
            cfg->expand_flash_hd.font_id = gt_convert_parse_u16(&pack_p->unpack.buffer[6]);
            cfg->expand_flash_hd.count_block = gt_convert_parse_u16(&pack_p->unpack.buffer[8]);
            break;
        }
        default:
            break;
    }
    return _write_buffer_direct(pack_p);
}

static uint16_t _read_expand_128mbit_flash(_serial_sys_pack_st * const pack_p) {
    return _read_buffer_direct(pack_p);
}

static gt_res_t _write_tp_cmd_visit(_serial_sys_pack_st * const pack_p) {
    if (0x5A != pack_p->unpack.buffer[0]) {
        return GT_RES_FAIL;
    }
    if (0xA5 != pack_p->unpack.buffer[1]) {
        return GT_RES_FAIL;
    }
    uint16_t page_id = gt_convert_parse_u16(&pack_p->unpack.buffer[2]);
    uint8_t widget_id = pack_p->unpack.buffer[4];
    uint8_t cmd_code = pack_p->unpack.buffer[5];
    /** 0x0000: disabled current tp cmd;
     *  0x0001: enabled;
     *  0x0002: read cmd to addr;
     *  0x0003: replace cmd by addr data;
     */
    uint16_t mode_val_or_addr = gt_convert_parse_u16(&pack_p->unpack.buffer[6]);

    // TODO handle widget touch cmd


    /** clear after done */
    pack_p->unpack.buffer[0] = 0x00;
    pack_p->unpack.buffer[1] = 0x00;
    return _write_buffer_direct(pack_p);
}

static gt_res_t _write_tp_simulate(_serial_sys_pack_st * const pack_p) {
    if (0x5A != pack_p->unpack.buffer[0]) {
        return GT_RES_OK;
    }
    if (0xA5 != pack_p->unpack.buffer[1]) {
        return GT_RES_OK;
    }
    /**
     * 1: press down; 2; press up; 3: pressing; 4: click(down + up)
     * must be have 2 after 1 or 3.
     */
    uint16_t press_mode = gt_convert_parse_u16(&pack_p->unpack.buffer[2]);
    gt_size_t x = gt_convert_parse_u16(&pack_p->unpack.buffer[4]);
    gt_size_t y = gt_convert_parse_u16(&pack_p->unpack.buffer[6]);
    gt_indev_state_et state = GT_INDEV_STATE_INVALID;

    if (1 == press_mode || 3 == press_mode) {
        gt_indev_simulate_handler(GT_INDEV_STATE_PRESSED, x, y);
    } else if (2 == press_mode) {
        gt_indev_simulate_handler(GT_INDEV_STATE_RELEASED, x, y);
    } else if (4 == press_mode) {
        gt_indev_simulate_handler(GT_INDEV_STATE_PRESSED, x, y);
        gt_indev_simulate_handler(GT_INDEV_STATE_RELEASED, x, y);
    }

    /** clear after done */
    pack_p->unpack.buffer[0] = 0x00;
    pack_p->unpack.buffer[1] = 0x00;
    return _write_buffer_direct(pack_p);
}

static gt_res_t _write_pointer_icon_overlay(_serial_sys_pack_st * const pack_p) {
    if (0x5A != pack_p->unpack.buffer[0]) {
        return GT_RES_OK;
    }
    uint8_t icl_id = pack_p->unpack.buffer[1];
    uint16_t icon_id = gt_convert_parse_u16(&pack_p->unpack.buffer[2]);
    gt_size_t x = gt_convert_parse_u16(&pack_p->unpack.buffer[4]);
    gt_size_t y = gt_convert_parse_u16(&pack_p->unpack.buffer[6]);

    gt_obj_st * pointer_icon = gt_obj_find_by_id(GT_POINTER_ICON_ID);
    if (NULL == pointer_icon) {
        pointer_icon = gt_img_create(gt_disp_get_layer_top());
        if (NULL == pointer_icon) {
            return GT_RES_FAIL;
        }
        gt_obj_register_id(pointer_icon, GT_POINTER_ICON_ID);
    }
#if GT_USE_FILE_HEADER
    // gt_file_header_param_st fh = {0};
    // gt_img_set_by_file_header(pointer_icon, &fh);
#endif
    gt_obj_set_pos(pointer_icon, x, y);

    return _write_buffer_direct(pack_p);
}

// static gt_res_t _write_wae_switch(_serial_sys_pack_st * const pack_p) {
    // return _write_buffer_direct(pack_p);
// }

static gt_res_t _write_icl_switch(_serial_sys_pack_st * const pack_p) {
    return _write_buffer_direct(pack_p);
}

static gt_res_t _write_crc_checksum(_serial_sys_pack_st * const pack_p) {
    if (0x5A != pack_p->unpack.buffer[0]) {
        return GT_RES_OK;
    }
    uint8_t store_type = pack_p->unpack.buffer[1];
    /** 0x00 font; 0x02 OS; 0x03 lib */
    if (0x00 == store_type) {
        uint8_t font_id = pack_p->unpack.buffer[2];
        uint8_t block_4k_count = pack_p->unpack.buffer[3];
    } else if (0x02 == store_type) {
        /** from 0x1000 start, size = 0x0001 ~ 0x7000 */
    } else if (0x03 == store_type) {
        uint16_t lib_id = gt_convert_parse_u16(&pack_p->unpack.buffer[2]);
    }
    /* TODO send back crc */

    return _write_buffer_direct(pack_p);
}

static uint16_t _read_crc_checksum(_serial_sys_pack_st * const pack_p) {
    return _read_buffer_direct(pack_p);
}

// static uint16_t _read_area_single_bitmap_export(_serial_sys_pack_st * const pack_p) {
//     return 0;
// }

// static uint16_t _read_page_overlay_switch(_serial_sys_pack_st * const pack_p) {
//     return 0;
// }

static gt_res_t _write_music_stream(_serial_sys_pack_st * const pack_p) {
    if (0x5A != pack_p->unpack.buffer[0]) {
        return GT_RES_OK;
    }
    uint8_t play_mode = pack_p->unpack.buffer[1];   /* 0x00 stop(clear buffer); 0x01 stop(no clear); 0x02 play */
    uint16_t var_addr = gt_convert_parse_u16(&pack_p->unpack.buffer[4]);
    uint16_t var_len = gt_convert_parse_u16(&pack_p->unpack.buffer[6]);     /** such as: 0x8000 */

    /* TODO copy wav to var_addr area */

    return _write_buffer_direct(pack_p);
}

static gt_res_t _write_tp_draw_window(_serial_sys_pack_st * const pack_p) {
    /** v45 deprecated */
    if (0x5A != pack_p->unpack.buffer[0]) {
        return GT_RES_OK;
    }
    uint8_t handle_mode = pack_p->unpack.buffer[1]; /* 0x00 normal; 0x01 init */
    uint8_t param_reg = pack_p->unpack.buffer[2];   /* draw buffer handler: 00b: close; 01b: reset init(normal) */
    uint8_t line_thick = pack_p->unpack.buffer[4];  /* 0x01 ~ 0x0F */
    uint8_t red = pack_p->unpack.buffer[5];
    uint8_t green = pack_p->unpack.buffer[6];
    uint8_t blue = pack_p->unpack.buffer[7];
    uint16_t x = gt_convert_parse_u16(&pack_p->unpack.buffer[8]);
    uint16_t y = gt_convert_parse_u16(&pack_p->unpack.buffer[10]);
    uint16_t w = gt_convert_parse_u16(&pack_p->unpack.buffer[12]);
    uint16_t h = gt_convert_parse_u16(&pack_p->unpack.buffer[14]);

    /* 128KB window double addr 0x00 8000 ~ 0x00 FFFF max 208 * 208 */

    return _write_buffer_direct(pack_p);
}

// static gt_res_t _write_gui_stop_enabled(_serial_sys_pack_st * const pack_p) {
//     return _write_buffer_direct(pack_p);
// }

// static gt_res_t _write_uart_download(_serial_sys_pack_st * const pack_p) {
//     return _write_buffer_direct(pack_p);
// }

// static gt_res_t _write_fsk(_serial_sys_pack_st * const pack_p) {
//     return _write_buffer_direct(pack_p);
// }

// static uint16_t _read_fsk(_serial_sys_pack_st * const pack_p) {
//    return _read_buffer_direct(pack_p);
// }

static gt_res_t _write_dynamic_curve(_serial_sys_pack_st * const pack_p) {
    return _write_buffer_direct(pack_p);
}

static uint16_t _read_dynamic_curve(_serial_sys_pack_st * const pack_p) {
    return _read_buffer_direct(pack_p);
}

// static gt_res_t _write_dynamic_curve_channel1(_serial_sys_pack_st * const pack_p) {
//     return _write_buffer_direct(pack_p);
// }

// static gt_res_t _write_dynamic_curve_channel2(_serial_sys_pack_st * const pack_p) {
//     return _write_buffer_direct(pack_p);
// }

// static gt_res_t _write_dynamic_curve_channel3(_serial_sys_pack_st * const pack_p) {
//     return _write_buffer_direct(pack_p);
// }

// static gt_res_t _write_dynamic_curve_channel4(_serial_sys_pack_st * const pack_p) {
//     return _write_buffer_direct(pack_p);
// }

// static gt_res_t _write_dynamic_curve_channel5(_serial_sys_pack_st * const pack_p) {
//     return _write_buffer_direct(pack_p);
// }

// static gt_res_t _write_dynamic_curve_channel6(_serial_sys_pack_st * const pack_p) {
//     return _write_buffer_direct(pack_p);
// }

// static gt_res_t _write_dynamic_curve_channel7(_serial_sys_pack_st * const pack_p) {
//     return _write_buffer_direct(pack_p);
// }

// static gt_res_t _write_dynamic_curve_channel8(_serial_sys_pack_st * const pack_p) {
//     return _write_buffer_direct(pack_p);
// }

static gt_res_t _write_wifi_connect(_serial_sys_pack_st * const pack_p) {
    return _write_buffer_direct(pack_p);
}

static uint16_t _read_wifi_connect(_serial_sys_pack_st * const pack_p) {
    return _read_buffer_direct(pack_p);
}

static gt_res_t _write_multi_media(_serial_sys_pack_st * const pack_p) {
    return _write_buffer_direct(pack_p);
}

static uint16_t _read_multi_media(_serial_sys_pack_st * const pack_p) {
    return _read_buffer_direct(pack_p);
}

static gt_res_t _write_expand_store(_serial_sys_pack_st * const pack_p) {
    return _write_buffer_direct(pack_p);
}

static uint16_t _read_expand_store(_serial_sys_pack_st * const pack_p) {
    return _read_buffer_direct(pack_p);
}

static uint16_t _read_value_change(_serial_sys_pack_st * const pack_p) {
    return _read_buffer_direct(pack_p);
}

static GT_ATTRIBUTE_LARGE_RAM_ARRAY const _serial_system_st _system_list[] = {
    // { 0x00, 4, (_system_write_cb_t)NULL, (_system_read_cb_t)NULL },
    { GT_SERIAL_SYS_ADDR_RESET, 2, _write_system_reset, (_system_read_cb_t)NULL },
    { GT_SERIAL_SYS_ADDR_UPDATE_CMD, 2, _write_update_cmd, (_system_read_cb_t)NULL },
    { GT_SERIAL_SYS_ADDR_NOR_FLASH, 4, _write_nor_flash, (_system_read_cb_t)NULL },
    // { 0x0c, 2, _write_uart2, (_system_read_cb_t)NULL },
    { GT_SERIAL_SYS_ADDR_VER, 2, (_system_write_cb_t)NULL, _read_ver },
    { GT_SERIAL_SYS_ADDR_RTC, 4, _write_rtc, _read_rtc },
    { GT_SERIAL_SYS_ADDR_PIC_NOW, 1, (_system_write_cb_t)NULL, _read_pic_now },
    { GT_SERIAL_SYS_ADDR_GUI_STATUS, 1, (_system_write_cb_t)NULL, _read_gui_status },
    // { 0x16, 4, (_system_write_cb_t)NULL, _read_tp_status },  // deprecated
    // { 0x1a, 23, (_system_write_cb_t)NULL, (_system_read_cb_t)NULL },
    { GT_SERIAL_SYS_ADDR_LED_NOW, 1, (_system_write_cb_t)NULL, _read_led_now },
    { GT_SERIAL_SYS_ADDR_AD0_7, 8, (_system_write_cb_t)NULL, _read_ad0_ad7 },
    // { 0x3a, 64, (_system_write_cb_t)NULL, (_system_read_cb_t)NULL },
    { GT_SERIAL_SYS_ADDR_LCD_HOR, 1, (_system_write_cb_t)NULL, _read_lcd_hor },
    { GT_SERIAL_SYS_ADDR_LCD_VER, 1, (_system_write_cb_t)NULL, _read_lcd_ver },
    // { 0x7c, 4, (_system_write_cb_t)NULL, (_system_read_cb_t)NULL },
    { GT_SERIAL_SYS_ADDR_SYS_CONFIG, 2, _write_system_config, _read_system_config },
    { GT_SERIAL_SYS_ADDR_LED_CONFIG, 2, _write_led_config, _read_led_config },
    { GT_SERIAL_SYS_ADDR_PIC_SET, 2, _write_pic_set, _read_pic_set },
    { GT_SERIAL_SYS_ADDR_PWM0_SET, 2, _write_pwm0_set, _read_pwm0_set },
    // { 0x88, 2, _write_pwm1_set, _read_pwm1_set },    // deprecated
    // { 0x8a, 10, (_system_write_cb_t)NULL, (_system_read_cb_t)NULL },
    { GT_SERIAL_SYS_ADDR_PWM0_OUT, 1, _write_pwm0_out, _read_pwm0_out },
    { GT_SERIAL_SYS_ADDR_PWM1_OUT, 1, _write_pwm1_out, _read_pwm1_out },
    // { 0x94, 9, (_system_write_cb_t)NULL, (_system_read_cb_t)NULL },
    { GT_SERIAL_SYS_ADDR_RTC_SET, 4, _write_rtc_set, (_system_read_cb_t)NULL },
    { GT_SERIAL_SYS_ADDR_READ_WAE, 4, _write_wae, _read_wae },
    // { 0xa4, 6, (_system_write_cb_t)NULL, (_system_read_cb_t)NULL },
    { GT_SERIAL_SYS_ADDR_EXP_FLASH, 6, _write_expand_128mbit_flash, _read_expand_128mbit_flash },
    { GT_SERIAL_SYS_ADDR_TP_CMD_VISIT, 36, _write_tp_cmd_visit, (_system_read_cb_t)NULL },
    { GT_SERIAL_SYS_ADDR_TP_SIMULATE, 4, _write_tp_simulate, (_system_read_cb_t)NULL },
    { GT_SERIAL_SYS_ADDR_PTR_ICON_OVERLAY, 4, _write_pointer_icon_overlay, (_system_read_cb_t)NULL },
    // { 0xdc, 2, _write_wae_switch, (_system_read_cb_t)NULL }, // deprecated
    { GT_SERIAL_SYS_ADDR_ICL_SWITCH, 2, _write_icl_switch, (_system_read_cb_t)NULL },
    // { 0xdc, 4, (_system_write_cb_t)NULL, (_system_read_cb_t)NULL },
    { GT_SERIAL_SYS_ADDR_CRC_CHECKSUM, 2, _write_crc_checksum, _read_crc_checksum },
    // { 0xe2, 6, (_system_write_cb_t)NULL, _read_area_single_bitmap_export },  // deprecated
    // { 0xe8, 2, (_system_write_cb_t)NULL, _read_page_overlay_switch },        // deprecated
    // { 0xea, 6, (_system_write_cb_t)NULL, (_system_read_cb_t)NULL },
    { GT_SERIAL_SYS_ADDR_MUSIC_STREAM, 4, _write_music_stream, (_system_read_cb_t)NULL },
    { GT_SERIAL_SYS_ADDR_TP_DRAW_WINDOW, 8, _write_tp_draw_window, (_system_read_cb_t)NULL },
    // deprecated
    // { 0xfc, 2, _write_gui_stop_enabled, (_system_read_cb_t)NULL },
    // { 0xfe, 2, _write_uart_download, (_system_read_cb_t)NULL },
    // { 0x100, 512, _write_fsk, _read_fsk },
    { GT_SERIAL_SYS_ADDR_DYNAMIC_CURVE, 128, _write_dynamic_curve, _read_dynamic_curve },
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
    { GT_SERIAL_SYS_ADDR_WIFI_CONNECT, 256, _write_wifi_connect, _read_wifi_connect },
    { GT_SERIAL_SYS_ADDR_MULTI_MEDIA, 192, _write_multi_media, _read_multi_media },
    { GT_SERIAL_SYS_ADDR_EXPAND_STORE, 64, _write_expand_store, _read_expand_store },
    // { 0x600, 2404, (_system_write_cb_t)NULL, (_system_read_cb_t)NULL },
    { GT_SERIAL_SYS_ADDR_VALUE_CHANGE, 2, (_system_write_cb_t)NULL, _read_value_change },
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

uint16_t gt_serial_system_get_value_by_addr(uint8_t * const res, uint16_t addr, uint8_t short_len)
{
    uint8_t tmp[3] = {0}, cnt = 0;
    cnt = gt_convert_print_u16(tmp, addr);
    tmp[cnt++] = short_len;
    return gt_serial_system_get_value(res, tmp, cnt);
}


/* end ------------------------------------------------------------------*/
#endif /** GT_USE_SERIAL && GT_USE_SERIAL_SYSTEM */
