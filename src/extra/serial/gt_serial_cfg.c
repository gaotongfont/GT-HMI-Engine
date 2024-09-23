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

#if GT_USE_SERIAL_CFG
#include "../../utils/gt_bin_convert.h"
#include "../../others/gt_log.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/

typedef struct {
    uint8_t crc_checksum : 1;
    uint8_t beep : 1;
    uint8_t load_22 : 1;
    uint8_t touch_auto_upload : 1;
    uint8_t touch_beep : 1;
    uint8_t touch_back_light_sleep : 1;
    uint8_t disp_direct : 2;
}_serial_param_0_st;

typedef struct {
    uint8_t pwm_output : 1;
    uint8_t nand_flash_expand : 1;
    uint8_t nand_flash_format_clean_once : 1;
    uint8_t nand_flash_size : 1;
    uint8_t nand_flash_expand_format_clean : 1;
    uint8_t ad_resolution : 2;
    uint8_t reserved : 1;
    uint8_t sd_download_os_run : 1;
}_serial_param_1_st;

typedef struct gt_serial_cfg_s {
    _serial_param_0_st param_0;
    _serial_param_1_st param_1;
}gt_serial_cfg_st;

/* static variables -----------------------------------------------------*/
static gt_serial_cfg_st _serial_cfg;


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_res_t _parse_param_0_cb(void * user_data, uint8_t const * const buffer, uint32_t len)
{
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;

    cfg->param_0.crc_checksum = (buffer[0] >> 7) & 0x01;

    return GT_RES_OK;
}

static gt_res_t _print_param_0_cb(void const * const user_data, uint8_t * const buffer, uint32_t len)
{
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;

    buffer[0] = (cfg->param_0.crc_checksum << 7) & 0x80;

    return GT_RES_OK;
}

static gt_res_t _parse_param_1_cb(void * user_data, uint8_t const * const buffer, uint32_t len)
{
    gt_serial_cfg_st * cfg = (gt_serial_cfg_st *)user_data;

    cfg->param_1.pwm_output = (buffer[0] >> 7) & 0x01;

    return GT_RES_OK;
}

static gt_res_t _print_param_1_cb(void const * const user_data, uint8_t * const buffer, uint32_t len)
{
    gt_serial_cfg_st const * cfg = (gt_serial_cfg_st const *)user_data;

    buffer[0] = (cfg->param_1.pwm_output << 7) & 0x80;

    return GT_RES_OK;
}

/* global functions / API interface -------------------------------------*/
void gt_serial_cfg_init(void)
{
#if GT_USE_BIN_CONVERT
    gt_bin_item_st format[] = {
        { 0x1, _parse_param_0_cb, _print_param_0_cb },
        { 0x1, _parse_param_1_cb, _print_param_1_cb },
        { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL },
        /** ... */
    };
    uint8_t buffer[2] = { 0xFF, 0x00 };

    gt_bin_convert_st bin = {
        .tar = &_serial_cfg,
        .buffer = buffer,
        .byte_len = 2,
    };
    // GT_LOG_A("", ">>>>>>>>>>>>>>>>>>>>>> %x %x", buffer[0], buffer[1]);
    gt_bin_convert_parse(&bin, format);
    gt_bin_convert_print(&bin, format);
    // GT_LOG_A("", ">>>>>>>>>>>>>>>>>>>>>> %x %x", buffer[0], buffer[1]);
#endif
}



/* end ------------------------------------------------------------------*/
#endif  /** GT_USE_SERIAL_CFG */
