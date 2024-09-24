/**
 * @file gt_serial_cfg.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-09-20 16:06:42
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_serial_cfg.h"

#if GT_USE_SERIAL_CFG && GT_USE_BIN_CONVERT
#include "../../utils/gt_bin_convert.h"
#include "../../others/gt_log.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/
typedef struct {
    uint8_t crc_checksum : 1;
    uint8_t beep : 1;                       /** 0: beep; 1: music */
    uint8_t load_22 : 1;
    uint8_t touch_auto_upload : 1;
    uint8_t touch_beep : 1;
    uint8_t touch_back_light_sleep : 1;
    uint8_t disp_direct : 2;
}_serial_param_0_st;

typedef struct {
    uint8_t pwm_output : 1;
    uint8_t nand_flash_expand : 1;
    uint8_t nand_flash_format_clean_once : 1;       /** Reset to 0 after clean */
    uint8_t nand_flash_size : 1;                    /** 0: 1G bits; 1: 4G bits */
    uint8_t nand_flash_expand_format_clean : 1;
    uint8_t ad_resolution : 2;                      /** 0: 12bit; 1: 16bit */
    uint8_t reserved : 1;
    uint8_t sd_download_os_run : 1;
}_serial_param_1_st;

typedef struct {
    _serial_param_0_st param_0;
    _serial_param_1_st param_1;
    uint8_t wae_id;
    uint8_t icl_id;
    uint8_t touch_report_rate;          /** Range 0x01-0xff; [default 0x28, rate: 400Hz/val] */
    uint8_t uart_baud_rate[2];          /** rate = 3225600 / val; 115200bps = 0x001c, max = 0x3FF */
}_serial_system_config_st;

typedef struct {
    uint8_t brightness;                 /** 0x00-0x64: 1% */
    uint8_t brightness_sleep;           /** 0x00-0x64: 1% */
    uint8_t wake_up_time[2];            /** 0x0001-0xffff: 10ms */
}_serial_backlight_sleep_st;

typedef struct {
    uint8_t disp_enabled[2];            /** 0x5aa5 valid */
    uint8_t pll_clk_phs;                /** 0x00: falling edge; 0x01: rising edge */
    uint8_t pll_clk_div;
    uint8_t hor_w;
    uint8_t hor_s;
    uint8_t hor_d[2];                   /** resolution radio */
    uint8_t hor_e;
    uint8_t ver_w;
    uint8_t ver_s;
    uint8_t ver_d[2];                   /** resolution radio */
    uint8_t ver_e;
    uint8_t tcon_sel;                   /** 0x00: no need set TCON */
}_serial_display_st;

typedef struct {
    uint8_t reserved;                   /** 0x00 */
    uint8_t enabled;                    /** 0x5A: valid */
    /**
     *  7-4b type: 0x0*
     *  3b: 电阻屏校准, 在SD下载时启用
     *  2-0b: [电容屏有效]
     *  2b: hor direct 0: 0 - X_max; 1: X_max - 0;
     *  1b: ver direct 0: 0 - Y_max; 1: Y_max - 0;
     *  0b: switch x/y: 0: xy; 1: yx.
     */
    uint8_t mode;
    /**
     * default: 0x14
     */
    uint8_t sense;
    uint8_t freq;       /** 0x01-0x14 fixed; 0x00 flex */
}_serial_touch_st;

typedef struct {
    uint8_t set_en;     /** 0x5A */
    uint8_t enabled;
    uint8_t div;
}_serial_clk_output_st;

typedef struct {
    uint8_t set_en;             /** 0x5a valid */
    uint8_t freq_div1;          /** 825753.6 / (freq_div1 * freq_div1) KHz */
    uint8_t freq_div2[2];       /** default: div1 = 0x6e, div2 = 0x0bb8, equal to 2.5 KHz */
    uint8_t freq_duty[2];       /** high level%: freq_duty / freq_div2; default: 0x00f0 = 8% */
    uint8_t time;               /** default: 0x0A; unit: 10ms */
}_serial_beep_st;

typedef struct {
    uint8_t set_en;         /** 0x5a valid */
    uint8_t file_id;        /** default: 0x16 */
}_serial_init_file_st;

typedef struct {
    uint8_t set_en;         /** 0x5a valid */
    uint8_t adj_set;        /** 0x00 */
}_serial_sys_clk_st;

typedef struct {
    uint8_t set_en;         /** 0x5a valid */
    uint8_t icl_id;         /** 0x00: cannot replace */
}_serial_pic_replace_st;

typedef struct {
    uint8_t period : 1;
    uint8_t uart1_crc : 1;
    uint8_t reserved : 6;   /** 0x00 */
}_expand_param_st;

typedef struct {
    uint8_t set_en_start;   /** 0x5a Start an extended parameter configuration */
    uint8_t set_en;         /** 0x5a valid */
    _expand_param_st param_1;
    uint8_t reserved[9];
}_serial_expand_param_st;

typedef struct {
    uint8_t set_en[2];      /** 0x5a, 0xa5 valid, save it in flash on the screen */
    uint8_t name_len;       /** folder name length range: 0x00-0x08 */
    uint8_t folder_name[8]; /** folder name */
    uint8_t reserved[37];   /** 0x00 */
}_serial_sd_st;

typedef struct {
    uint8_t set_en;         /** 0x5a valid */
    uint8_t delay;          /** default: 0x0a, unit: 10ms */
}_serial_backlight_delay_st;

typedef struct gt_serial_cfg_s {
    uint8_t header[5];
    /** system config */
    _serial_system_config_st system_config;

    /** backlight or sleep */
    _serial_backlight_sleep_st backlight_sleep;

    /** display */
    _serial_display_st display;

    /** touch */
    _serial_touch_st tp;

    /** clk output */
    _serial_clk_output_st clk;

    /** beep */
    _serial_beep_st beep;

    /** init file */
    _serial_init_file_st init_file;

    /** system clock */
    _serial_sys_clk_st sys_clk;

    /** picture replace */
    _serial_pic_replace_st pic_replace;

    /** expand param */
    _serial_expand_param_st expand_param;

    /** SD download folder setting */
    _serial_sd_st sd;

    /** backlight delay time */
    _serial_backlight_delay_st backlight_delay;

    uint8_t reserved[14];
}gt_serial_cfg_st;

/* static variables -----------------------------------------------------*/
static gt_serial_cfg_st _serial_cfg;


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_res_t _parse_header(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    uint8_t check[] = { 0x54, 0x35, 0x4c, 0x43, 0x31 };
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
    _serial_system_config_st * sys_cfg = (_serial_system_config_st * )&cfg->system_config;
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
    for (uint8_t i = 0; i < 2; ++i) {
        sys_cfg->uart_baud_rate[i] = buffer[5 + i];
    }

    return GT_RES_OK;
}

static gt_res_t _print_system_config(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    _serial_system_config_st * sys_cfg = (_serial_system_config_st * )&cfg->system_config;
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
    for (uint8_t i = 0; i < 2; ++i) {
        buffer[5 + i] = sys_cfg->uart_baud_rate[i];
    }

    return GT_RES_OK;
}

static gt_res_t _parse_backlight_sleep(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;


    return GT_RES_OK;
}

static gt_res_t _print_backlight_sleep(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;


    return GT_RES_OK;
}

static gt_res_t _parse_display(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    return GT_RES_OK;
}

static gt_res_t _print_display(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    return GT_RES_OK;
}

static gt_res_t _parse_touch(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    return GT_RES_OK;
}

static gt_res_t _print_touch(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    return GT_RES_OK;
}

static gt_res_t _parse_clock_output(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    return GT_RES_OK;
}

static gt_res_t _print_clock_output(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    return GT_RES_OK;
}

static gt_res_t _parse_beep(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    return GT_RES_OK;
}

static gt_res_t _print_beep(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    return GT_RES_OK;
}

static gt_res_t _parse_init_file(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    return GT_RES_OK;
}

static gt_res_t _print_init_file(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    return GT_RES_OK;
}

static gt_res_t _parse_sys_clk(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    return GT_RES_OK;
}

static gt_res_t _print_sys_clk(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    return GT_RES_OK;
}

static gt_res_t _parse_picture_replace(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    return GT_RES_OK;
}

static gt_res_t _print_picture_replace(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    return GT_RES_OK;
}

static gt_res_t _parse_expand_param(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    return GT_RES_OK;
}

static gt_res_t _print_expand_param(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    return GT_RES_OK;
}

static gt_res_t _parse_sd(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    return GT_RES_OK;
}

static gt_res_t _print_sd(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    return GT_RES_OK;
}

static gt_res_t _parse_backlight_delay(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    return GT_RES_OK;
}

static gt_res_t _print_backlight_delay(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    return GT_RES_OK;
}

static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_bin_item_st format[] = {
    { 5, _parse_header, _print_header },
    { 7, _parse_system_config, _print_system_config },
    { 4, _parse_backlight_sleep, _print_backlight_sleep },
    { 15, _parse_display, _print_display },
    { 5, _parse_touch, _print_touch },
    { 3, _parse_clock_output, _print_clock_output },
    { 7, _parse_beep, _print_beep },
    { 2, _parse_init_file, _print_init_file },
    { 2, _parse_sys_clk, _print_sys_clk },
    { 2, _parse_picture_replace, _print_picture_replace },
    { 12, _parse_expand_param, _print_expand_param },
    { 48, _parse_sd, _print_sd },
    { 2, _parse_backlight_delay, _print_backlight_delay },
    { 14, (parse_cb_t)NULL, (print_cb_t)NULL },

    /** ... */
    { 0, (parse_cb_t)NULL, (print_cb_t)NULL },
};

/* global functions / API interface -------------------------------------*/
void gt_serial_cfg_init(void)
{
    uint8_t buffer[128] = { 0x54, 0x35, 0x4c, 0x43, 0x31, 0xff };

    gt_bin_convert_st bin = {
        .tar = &_serial_cfg,
        .buffer = buffer,
        .byte_len = sizeof(buffer) / sizeof(uint8_t),
    };
    gt_bin_res_st ret = {0};
    // GT_LOG_A("", ">>>>>>>>>>>>>>>> total byte length: %d", gt_bin_get_total_byte_length(format));
    // GT_LOG_A("", ">>>>>>>>>>>>>>>>>>>>>> %x %x", buffer[5], buffer[6]);
    // ret = gt_bin_convert_parse(&bin, format);
    // if (GT_RES_OK != ret.res) {
    //     GT_LOG_A("", "Parse err code: %d, index: %d", ret.res, ret.index);
    // }
    // ret = gt_bin_convert_print(&bin, format);
    // if (GT_RES_OK != ret.res) {
    //     GT_LOG_A("", "Print err code: %d, index: %d", ret.res, ret.index);
    // }
    // GT_LOG_A("", ">>>>>>>>>>>>>>>>>>>>>> %x %x", buffer[5], buffer[6]);
}



/* end ------------------------------------------------------------------*/
#endif  /** GT_USE_SERIAL_CFG && GT_USE_BIN_CONVERT */
