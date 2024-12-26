/**
 * @file gt_serial_cfg.c
 * @author Feyoung
 * @brief cfg file at No.0 file
 * @version 0.1
 * @date 2024-09-20 16:06:42
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_serial_cfg.h"

#if GT_USE_SERIAL_CFG && GT_USE_BIN_CONVERT
#include "../../utils/gt_bin_convert.h"
#include "../../core/gt_fs.h"
#include "../../core/gt_mem.h"
#include "../../others/gt_log.h"
#include "../../utils/gt_serial.h"
#include "./gt_serial_show.h"
#include "./gt_serial_system.h"
#include "./gt_serial_resource.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static gt_serial_cfg_st _serial_cfg = {0};


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_res_t _parse_header(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    uint8_t check[] = { 0x47, 0x54, 0x48, 0x4D, 0x49 };
    if (len != 5) {
        return GT_RES_INV;
    }

    for (uint8_t i = 0; i < len; ++i) {
        if (check[i] != buffer[i]) {
            return GT_RES_INV;
        }
        cfg->header[i] = buffer[i];
    }
    return GT_RES_OK;
}

static gt_res_t _print_header(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    for (uint8_t i = 0; i < len; ++i) {
        buffer[i] = cfg->header[i];
    }
    return GT_RES_OK;
}

static gt_res_t _parse_system_config(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    gt_serial_cfg_system_config_st * sys_cfg = (gt_serial_cfg_system_config_st * )&cfg->system_config;
    uint8_t val = 0;

    val = buffer[0];
    sys_cfg->param_0.crc_checksum = (val >> 7) & 0x01;
    sys_cfg->param_0.beep = (val >> 6) & 0x01;
    sys_cfg->param_0.load_22 = (val >> 5) & 0x01;
    sys_cfg->param_0.touch_auto_upload = (val >> 4) & 0x01;
    sys_cfg->param_0.touch_beep = (val >> 3) & 0x01;
    sys_cfg->param_0.touch_back_light_sleep = (val >> 2) & 0x01;
    sys_cfg->param_0.disp_direct = (val >> 0) & 0x03;

    val = buffer[1];
    sys_cfg->param_1.pwm_output = (val >> 7) & 0x01;
    sys_cfg->param_1.nand_flash_expand = (val >> 6) & 0x01;
    sys_cfg->param_1.nand_flash_format_clean_once = (val >> 5) & 0x01;
    sys_cfg->param_1.nand_flash_size = (val >> 4) & 0x01;
    sys_cfg->param_1.nand_flash_expand_format_clean = (val >> 3) & 0x01;
    sys_cfg->param_1.ad_resolution = (val >> 2) & 0x01;
    /** reserved 1 bit */
    sys_cfg->param_1.sd_download_os_run = (val >> 0) & 0x01;

    sys_cfg->wae_id = buffer[2];
    sys_cfg->icl_id = buffer[3];
    sys_cfg->touch_report_rate = buffer[4];
    uint16_t short_val = gt_convert_parse_u16(&buffer[5]);
    if (0 == short_val) {
        sys_cfg->uart_baud_rate = 115200;
    } else {
        sys_cfg->uart_baud_rate = 3225600 / short_val;
    }

    /* ------------------- init ------------------- */
    gt_serial_set_crc16(sys_cfg->param_0.crc_checksum ? true : false);


    return GT_RES_OK;
}

static gt_res_t _print_system_config(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    gt_serial_cfg_system_config_st * sys_cfg = (gt_serial_cfg_system_config_st * )&cfg->system_config;
    uint8_t val = 0;

    val = (sys_cfg->param_0.crc_checksum << 7) & 0x80;
    val |= (sys_cfg->param_0.beep << 6) & 0x40;
    val |= (sys_cfg->param_0.load_22 << 5) & 0x20;
    val |= (sys_cfg->param_0.touch_auto_upload << 4) & 0x10;
    val |= (sys_cfg->param_0.touch_beep << 3) & 0x08;
    val |= (sys_cfg->param_0.touch_back_light_sleep << 2) & 0x04;
    val |= (sys_cfg->param_0.disp_direct << 0) & 0x03;
    buffer[0] = val;

    val = (sys_cfg->param_1.pwm_output << 7) & 0x80;
    val |= (sys_cfg->param_1.nand_flash_expand << 6) & 0x40;
    val |= (sys_cfg->param_1.nand_flash_format_clean_once << 5) & 0x20;
    val |= (sys_cfg->param_1.nand_flash_size << 4) & 0x10;
    val |= (sys_cfg->param_1.nand_flash_expand_format_clean << 3) & 0x08;
    val |= (sys_cfg->param_1.ad_resolution << 2) & 0x04;
    /** reserved 1 bit */
    val |= (sys_cfg->param_1.sd_download_os_run << 0) & 0x01;
    buffer[1] = val;

    buffer[2] = sys_cfg->wae_id;
    buffer[3] = sys_cfg->icl_id;
    buffer[4] = sys_cfg->touch_report_rate;
    if (0 == sys_cfg->uart_baud_rate) {
        sys_cfg->uart_baud_rate = 115200;
    }
    gt_convert_print_u16(&buffer[5], (uint16_t)(3225600 / sys_cfg->uart_baud_rate));

    return GT_RES_OK;
}

static gt_res_t _parse_backlight_sleep(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    gt_serial_cfg_backlight_sleep_st * bs = (gt_serial_cfg_backlight_sleep_st *)&cfg->backlight_sleep;
    uint8_t tmp[4] = {0};

    bs->brightness = buffer[0];
    bs->brightness_sleep = buffer[1];
    bs->wake_up_time = gt_convert_parse_u16(&buffer[2]);

    /** update var buffer */
    gt_convert_print_u16(tmp, GT_SERIAL_SYS_ADDR_LED_NOW);
    tmp[2] = 0x5a;
    tmp[3] = bs->brightness;
    gt_serial_set_value(tmp, 4);

    return GT_RES_OK;
}

static gt_res_t _print_backlight_sleep(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    gt_serial_cfg_backlight_sleep_st const * bs = (gt_serial_cfg_backlight_sleep_st const *)&cfg->backlight_sleep;

    buffer[0] = bs->brightness;
    buffer[1] = bs->brightness_sleep;
    gt_convert_print_u16(&buffer[2], bs->wake_up_time);

    return GT_RES_OK;
}

static gt_res_t _parse_display(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    gt_serial_cfg_display_st * disp = (gt_serial_cfg_display_st *)&cfg->display;
    uint8_t tmp[6] = {0};

    disp->disp_enabled = gt_convert_parse_u16(&buffer[0]);
    disp->pll_clk_phs = buffer[2];
    disp->pll_clk_div = buffer[3];
    disp->hor_w = buffer[4];
    disp->hor_s = buffer[5];
    disp->hor_d = gt_convert_parse_u16(&buffer[6]);
    disp->hor_e = buffer[8];
    disp->ver_w = buffer[9];
    disp->ver_s = buffer[10];
    disp->ver_d = gt_convert_parse_u16(&buffer[11]);
    disp->ver_e = buffer[13];
    disp->tcon_sel = buffer[14];

    /** update var buffer */
    gt_convert_print_u16(tmp, GT_SERIAL_SYS_ADDR_LCD_HOR);
    gt_convert_print_u16(&tmp[2], disp->hor_d);
    gt_convert_print_u16(&tmp[4], disp->ver_d);
    gt_serial_set_value(tmp, 6);

    return GT_RES_OK;
}

static gt_res_t _print_display(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    gt_serial_cfg_display_st * disp = (gt_serial_cfg_display_st *)&cfg->display;

    gt_convert_print_u16(&buffer[0], disp->disp_enabled);
    buffer[2] = disp->pll_clk_phs;
    buffer[3] = disp->pll_clk_div;
    buffer[4] = disp->hor_w;
    buffer[5] = disp->hor_s;
    gt_convert_print_u16(&buffer[6], disp->hor_d);
    buffer[8] = disp->hor_e;
    buffer[9] = disp->ver_w;
    buffer[10] = disp->ver_s;
    gt_convert_print_u16(&buffer[11], disp->ver_d);
    buffer[13] = disp->ver_e;
    buffer[14] = disp->tcon_sel;

    return GT_RES_OK;
}

static gt_res_t _parse_touch(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    gt_serial_cfg_touch_st * tp = (gt_serial_cfg_touch_st *)&cfg->tp;
    uint8_t val = 0;

    tp->reserved = buffer[0];
    tp->enabled = buffer[1];

    val = buffer[2];
    tp->mode.type = (val >> 4) & 0x0f;
    tp->mode.resistance_calibration = (val >> 3) & 0x01;
    tp->mode.hor_dir = (val >> 2) & 0x01;
    tp->mode.ver_dir = (val >> 1) & 0x01;
    tp->mode.switch_hor_ver = (val >> 0) & 0x01;

    tp->sense = buffer[3];
    tp->freq = buffer[4];

    return GT_RES_OK;
}

static gt_res_t _print_touch(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    gt_serial_cfg_touch_st * tp = (gt_serial_cfg_touch_st *)&cfg->tp;

    buffer[0] = tp->reserved;
    buffer[1] = tp->enabled;

    uint8_t val = 0;
    val = (tp->mode.type << 4) & 0xf0;
    val |= (tp->mode.resistance_calibration << 3) & 0x08;
    val |= (tp->mode.hor_dir << 2) & 0x04;
    val |= (tp->mode.ver_dir << 1) & 0x02;
    val |= (tp->mode.switch_hor_ver << 0) & 0x01;
    buffer[2] = val;

    buffer[3] = tp->sense;
    buffer[4] = tp->freq;

    return GT_RES_OK;
}

static gt_res_t _parse_clock_output(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    gt_serial_cfg_clk_output_st * clk = (gt_serial_cfg_clk_output_st *)&cfg->clk;

    clk->set_en = buffer[0];
    clk->enabled = buffer[1];
    clk->div = buffer[2];

    return GT_RES_OK;
}

static gt_res_t _print_clock_output(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    gt_serial_cfg_clk_output_st * clk = (gt_serial_cfg_clk_output_st *)&cfg->clk;

    buffer[0] = clk->set_en;
    buffer[1] = clk->enabled;
    buffer[2] = clk->div;

    return GT_RES_OK;
}

static gt_res_t _parse_beep(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    gt_serial_cfg_beep_st * beep = (gt_serial_cfg_beep_st *)&cfg->beep;

    beep->set_en = buffer[0];
    beep->freq_div1 = buffer[1];
    beep->freq_div2 = gt_convert_parse_u16(&buffer[2]);
    beep->freq_duty = gt_convert_parse_u16(&buffer[4]);
    beep->time = buffer[6];

    return GT_RES_OK;
}

static gt_res_t _print_beep(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    gt_serial_cfg_beep_st * beep = (gt_serial_cfg_beep_st *)&cfg->beep;

    buffer[0] = beep->set_en;
    buffer[1] = beep->freq_div1;
    gt_convert_print_u16(&buffer[2], beep->freq_div2);
    gt_convert_print_u16(&buffer[4], beep->freq_duty);
    buffer[6] = beep->time;

    return GT_RES_OK;
}

static gt_res_t _parse_init_file(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    gt_serial_cfg_init_file_st * init_file = (gt_serial_cfg_init_file_st *)&cfg->init_file;

    init_file->set_en = buffer[0];
    init_file->file_id = buffer[1];

    return GT_RES_OK;
}

static gt_res_t _print_init_file(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    gt_serial_cfg_init_file_st * init_file = (gt_serial_cfg_init_file_st *)&cfg->init_file;

    buffer[0] = init_file->set_en;
    buffer[1] = init_file->file_id;

    return GT_RES_OK;
}

static gt_res_t _parse_sys_clk(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    gt_serial_cfg_sys_clk_st * sys_clk = (gt_serial_cfg_sys_clk_st *)&cfg->sys_clk;

    sys_clk->set_en = buffer[0];
    sys_clk->adj_set = buffer[1];

    return GT_RES_OK;
}

static gt_res_t _print_sys_clk(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    gt_serial_cfg_sys_clk_st * sys_clk = (gt_serial_cfg_sys_clk_st *)&cfg->sys_clk;

    buffer[0] = sys_clk->set_en;
    buffer[1] = sys_clk->adj_set;

    return GT_RES_OK;
}

static gt_res_t _parse_picture_replace(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    gt_serial_cfg_pic_replace_st * pic_replace = (gt_serial_cfg_pic_replace_st *)&cfg->pic_replace;

    pic_replace->set_en = buffer[0];
    pic_replace->icl_id = buffer[1];

    return GT_RES_OK;
}

static gt_res_t _print_picture_replace(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    gt_serial_cfg_pic_replace_st * pic_replace = (gt_serial_cfg_pic_replace_st *)&cfg->pic_replace;

    buffer[0] = pic_replace->set_en;
    buffer[1] = pic_replace->icl_id;

    return GT_RES_OK;
}

static gt_res_t _parse_expand_param(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    gt_serial_cfg_expand_param_st * expand_param = (gt_serial_cfg_expand_param_st *)&cfg->expand_param;

    expand_param->set_en_start = buffer[0];
    expand_param->set_en = buffer[1];
    expand_param->param_1.period = (buffer[2] >> 7) & 0x01;
    expand_param->param_1.uart1_crc = (buffer[2] >> 6) & 0x01;
    expand_param->param_1.reserved = (buffer[2] >> 0) & 0x3f;
    gt_memcpy(expand_param->reserved, &buffer[3], 9);

    return GT_RES_OK;
}

static gt_res_t _print_expand_param(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    gt_serial_cfg_expand_param_st * expand_param = (gt_serial_cfg_expand_param_st *)&cfg->expand_param;
    uint8_t val = 0;

    buffer[0] = expand_param->set_en_start;
    buffer[1] = expand_param->set_en;
    val = (expand_param->param_1.period << 7) & 0x80;
    val |= (expand_param->param_1.uart1_crc << 6) & 0x40;
    val |= (expand_param->param_1.reserved << 0) & 0x3f;
    buffer[2] = val;
    gt_memcpy(&buffer[3], expand_param->reserved, 9);

    return GT_RES_OK;
}

static gt_res_t _parse_sd(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    gt_serial_cfg_sd_st * sd = (gt_serial_cfg_sd_st *)&cfg->sd;

    sd->set_en = gt_convert_parse_u16(&buffer[0]);
    sd->name_len = buffer[2];
    gt_memcpy(sd->folder_name, &buffer[3], 8);
    gt_memcpy(sd->reserved, &buffer[11], 37);

    return GT_RES_OK;
}

static gt_res_t _print_sd(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    gt_serial_cfg_sd_st * sd = (gt_serial_cfg_sd_st *)&cfg->sd;

    gt_convert_print_u16(&buffer[0], sd->set_en);
    buffer[2] = sd->name_len;
    gt_memcpy(&buffer[3], sd->folder_name, 8);
    gt_memcpy(&buffer[11], sd->reserved, 37);

    return GT_RES_OK;
}

static gt_res_t _parse_backlight_delay(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    gt_serial_cfg_backlight_delay_st * delay = (gt_serial_cfg_backlight_delay_st *)&cfg->backlight_delay;

    delay->set_en = buffer[0];
    delay->delay = buffer[1];

    return GT_RES_OK;
}

static gt_res_t _print_backlight_delay(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    gt_serial_cfg_backlight_delay_st * delay = (gt_serial_cfg_backlight_delay_st *)&cfg->backlight_delay;

    buffer[0] = delay->set_en;
    buffer[1] = delay->delay;

    return GT_RES_OK;
}

static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_bin_item_st format[] = {
    { 5,    _parse_header,          _print_header           },
    { 7,    _parse_system_config,   _print_system_config    },
    { 4,    _parse_backlight_sleep, _print_backlight_sleep  },
    { 15,   _parse_display,         _print_display          },
    { 5,    _parse_touch,           _print_touch            },
    { 3,    _parse_clock_output,    _print_clock_output     },
    { 7,    _parse_beep,            _print_beep             },
    { 2,    _parse_init_file,       _print_init_file        },
    { 2,    _parse_sys_clk,         _print_sys_clk          },
    { 2,    _parse_picture_replace, _print_picture_replace  },
    { 12,   _parse_expand_param,    _print_expand_param     },
    { 48,   _parse_sd,              _print_sd               },
    { 2,    _parse_backlight_delay, _print_backlight_delay  },
    { 14,   (parse_cb_t)NULL,       (print_cb_t)NULL        },
    { 0,    (parse_cb_t)NULL,       (print_cb_t)NULL        },
};

/* global functions / API interface -------------------------------------*/
void gt_serial_cfg_init(void)
{
    uint8_t buffer[128] = {
    #if 0
        0x47, 0x54, 0x48, 0x4D, 0x49,
        // 0x54, 0x35, 0x4C, 0x43, 0x31,
        // 0x20,    // crc disabled
        0xA0,    // crc16 enabled
        0xA4, 0x10, 0x20, 0x28, 0x00, 0x1C, 0x64, 0x32, 0x00, 0x64,
        0x5A, 0xA5, 0x01, 0x04, 0xA0, 0x88, 0x04, 0x00, 0x18, 0x06, 0x1D, 0x02, 0x58, 0x03, 0x00, 0x00,
        0x5A, 0x00, 0x14, 0x00, 0x5A, 0x00, 0x74, 0x5A, 0x6E, 0x0B, 0xB8, 0x00, 0xF0, 0x0A, 0x00, 0x00,
        0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x5A, 0x5A, 0x06, 0x67, 0x74, 0x5F, 0x68, 0x6D, 0x69, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x5A, 0x0A
    #elif 0
        0x47, 0x54, 0x48, 0x4D, 0x49,
        // 0x54, 0x35, 0x4C, 0x43, 0x31,
        0x20, 0x80, 0x10, 0x20, 0x28, 0x01, 0x50, 0x64, 0x32, 0x00, 0x64,
        0x5A, 0xA5, 0x01, 0x04, 0xA0, 0x88, 0x04, 0x00, 0x18, 0x06, 0x1D, 0x02, 0x58, 0x03, 0x00, 0x00,
        0x5A, 0x00, 0x14, 0x00, 0x5A, 0x00, 0xFE, 0x5A, 0x6E, 0x0B, 0xB8, 0x00, 0xF0, 0x0A,
    #else
        0x00
    #endif
    };
    gt_bin_convert_st bin = {
        .tar = &_serial_cfg,
        .buffer = buffer,
        .byte_len = sizeof(buffer) / sizeof(uint8_t),
    };
    gt_bin_res_st ret = {0};

    /** read data from flash */
    uint32_t addr = gt_serial_resource_get_addr_by(0);
    if (GT_SERIAL_RESOURCE_INVALID_ADDR == addr) {
        GT_LOGE(GT_LOG_TAG_SERIAL, "config file invalid address.");
        return;
    }
    gt_fs_read_direct_physical(addr, 128, buffer);

    ret = gt_bin_convert_parse(&bin, format);
    if (GT_RES_OK != ret.res) {
        GT_LOG_A("", "Parse err code: %d, index: %d", ret.res, ret.index);
    }
}

gt_serial_cfg_st * const gt_serial_cfg_get_root(void)
{
    return &_serial_cfg;
}


/* end ------------------------------------------------------------------*/
#endif  /** GT_USE_SERIAL_CFG && GT_USE_BIN_CONVERT */
