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

typedef struct gt_serial_cfg_s {
    uint8_t header[5];
    _serial_param_0_st param_0;
    _serial_param_1_st param_1;
    uint8_t wae_id;
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

static gt_res_t _parse_param_0(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;

    cfg->param_0.crc_checksum = (buffer[0] >> 7) & 0x01;
    cfg->param_0.beep = (buffer[0] >> 6) & 0x01;
    cfg->param_0.load_22 = (buffer[0] >> 5) & 0x01;
    cfg->param_0.touch_auto_upload = (buffer[0] >> 4) & 0x01;
    cfg->param_0.touch_beep = (buffer[0] >> 3) & 0x01;
    cfg->param_0.touch_back_light_sleep = (buffer[0] >> 2) & 0x01;
    cfg->param_0.disp_direct = (buffer[0] >> 0) & 0x03;

    return GT_RES_OK;
}

static gt_res_t _print_param_0(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;

    buffer[0] = (cfg->param_0.crc_checksum << 7) & 0x80;
    buffer[0] |= (cfg->param_0.beep << 6) & 0x40;
    buffer[0] |= (cfg->param_0.load_22 << 5) & 0x20;
    buffer[0] |= (cfg->param_0.touch_auto_upload << 4) & 0x10;
    buffer[0] |= (cfg->param_0.touch_beep << 3) & 0x08;
    buffer[0] |= (cfg->param_0.touch_back_light_sleep << 2) & 0x04;
    buffer[0] |= (cfg->param_0.disp_direct << 0) & 0x03;

    return GT_RES_OK;
}

static gt_res_t _parse_param_1(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;

    cfg->param_1.pwm_output = (buffer[0] >> 7) & 0x01;
    cfg->param_1.nand_flash_expand = (buffer[0] >> 6) & 0x01;
    cfg->param_1.nand_flash_format_clean_once = (buffer[0] >> 5) & 0x01;
    cfg->param_1.nand_flash_size = (buffer[0] >> 4) & 0x01;
    cfg->param_1.nand_flash_expand_format_clean = (buffer[0] >> 3) & 0x01;
    cfg->param_1.ad_resolution = (buffer[0] >> 1) & 0x03;
    /** reserved 1 bit */
    cfg->param_1.sd_download_os_run = (buffer[0] >> 0) & 0x01;

    return GT_RES_OK;
}

static gt_res_t _print_param_1(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;

    buffer[0] = (cfg->param_1.pwm_output << 7) & 0x80;
    buffer[0] |= (cfg->param_1.nand_flash_expand << 6) & 0x40;
    buffer[0] |= (cfg->param_1.nand_flash_format_clean_once << 5) & 0x20;
    buffer[0] |= (cfg->param_1.nand_flash_size << 4) & 0x10;
    buffer[0] |= (cfg->param_1.nand_flash_expand_format_clean << 3) & 0x08;
    buffer[0] |= (cfg->param_1.ad_resolution << 1) & 0x06;
    /** reserved 1 bit */
    buffer[0] |= (cfg->param_1.sd_download_os_run << 0) & 0x01;

    return GT_RES_OK;
}

static gt_res_t _parse_wae_id(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;
    cfg->wae_id = buffer[0];
    return GT_RES_OK;
}

static gt_res_t _print_wae_id(void const * const user_data, uint8_t * const buffer, uint32_t len) {
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;
    buffer[0] = cfg->wae_id;
    return GT_RES_OK;
}

static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_bin_item_st format[] = {
    { 0x5, _parse_header, _print_header },
    { 0x1, _parse_param_0, _print_param_0 },
    { 0x1, _parse_param_1, _print_param_1 },
    { 0x1, _parse_wae_id, _print_wae_id },

    /** ... */
    { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL },
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
    // GT_LOG_A("", ">>>>>>>>>>>>>>>>>>>>>> %x %x", buffer[5], buffer[6]);
    ret = gt_bin_convert_parse(&bin, format);
    if (GT_RES_OK != ret.res) {
        GT_LOG_A("", "Parse err code: %d, index: %d", ret.res, ret.index);
    }
    ret = gt_bin_convert_print(&bin, format);
    if (GT_RES_OK != ret.res) {
        GT_LOG_A("", "Print err code: %d, index: %d", ret.res, ret.index);
    }
    // GT_LOG_A("", ">>>>>>>>>>>>>>>>>>>>>> %x %x", buffer[5], buffer[6]);
}



/* end ------------------------------------------------------------------*/
#endif  /** GT_USE_SERIAL_CFG && GT_USE_BIN_CONVERT */
