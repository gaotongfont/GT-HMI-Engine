/**
 * @file gt_serial_qrcode.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-10-21 16:38:35
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_serial_qrcode.h"

#if GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT
#include "stdint.h"
#include "../../core/gt_mem.h"
#include "./gt_serial_show.h"
#include "./gt_serial_system.h"
#include "../../utils/gt_bin_convert.h"
#include "../../others/gt_log.h"
#include "../../core/gt_style.h"
#include "../../widgets/gt_obj.h"
#include "../../widgets/gt_qrcode.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/
typedef struct {
    /** 0: fixed background while, fore color black; 1: depend by color0, color1 */
    uint8_t color_type : 1;
    uint8_t reserved : 7;
}_serial_qrcode_display_mode_st;

typedef struct {
    gt_point_st pos;
    uint16_t unit_pixel;    /** 0x01 ~ 0x07: enlarge pixels, 1 pixel convert to unit pixel size */
    uint8_t fix_mode;       /** 0x01: fix 73x73; others: auto adapt */
    _serial_qrcode_display_mode_st display_mode;
    uint16_t color0;        /** background color */
    uint16_t color1;        /** foreground color */
    uint8_t reserved[12];
}gt_qrcode_param_st;

typedef struct {
    gt_var_head_st head;
    gt_qrcode_param_st param;
}gt_serial_qrcode_st;


/* static variables -----------------------------------------------------*/
static gt_res_t _parse_param(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_serial_qrcode_st * var = (gt_serial_qrcode_st *)user_data;
    gt_qrcode_param_st * param = &var->param;

    param->pos.x = gt_convert_parse_i16(buffer);
    param->pos.y = gt_convert_parse_i16(&buffer[2]);
    param->unit_pixel = gt_convert_parse_u16(&buffer[4]);
    param->fix_mode = buffer[6];
    param->display_mode.color_type = (buffer[7] >> 7) & 0x01;
    param->color0 = gt_convert_parse_u16(&buffer[8]);
    param->color1 = gt_convert_parse_u16(&buffer[10]);

    return GT_RES_OK;
}

static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_bin_item_st _format[] = {
    { 8, gt_serial_var_head_parse, (print_cb_t)NULL },
    { 24, _parse_param, (print_cb_t)NULL },
    { 0x00, (parse_cb_t)NULL, (print_cb_t)NULL },
};

/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static bool _get_param(gt_var_st * var, gt_serial_qrcode_st * qr_code) {
    gt_bin_convert_st bin = {
        .tar = qr_code,
        .buffer = var->buffer,
        .byte_len = var->len,
    };
    gt_bin_res_st ret = gt_bin_convert_parse(&bin, _format);
    if (GT_RES_OK != ret.res) {
        GT_LOGE(GT_LOG_TAG_SERIAL, "Parse err code: %d, index: %d", ret.res, ret.index);
        return false;
    }
    return true;
}

static void _set_qr_code_string(gt_obj_st * qr, gt_serial_qrcode_st * qr_code) {
    uint32_t len_zero = gt_serial_var_buffer_get_len_by_eof(qr_code->head.vp, 0x0000);
    uint32_t len = gt_serial_var_buffer_get_len_by_eof(qr_code->head.vp, 0xFFFF);
    len = GT_MIN(len, len_zero);
    if (0 == len) {
        return;
    }
    if (len > 459) { len = 459; }
    uint8_t * str = (uint8_t *)gt_mem_malloc(len);
    if (str) {
        gt_serial_var_buffer_get_reg(qr_code->head.vp, str, len);
        gt_qrcode_set_str(qr, (char *)str);
    }
    gt_mem_free(str);
    str = NULL;
}

static void _set_vp_value(gt_var_st * var, gt_var_value_st * value) {
    gt_serial_qrcode_st qr_code = {0};
    if (false == _get_param(var, &qr_code)) {
        return;
    }
    _set_qr_code_string(var->obj, &qr_code);
}

static void _set_param(gt_var_st* var, gt_serial_qrcode_st * qr_code) {
    gt_obj_set_pos(var->obj, qr_code->param.pos.x, qr_code->param.pos.y);
    if (qr_code->param.display_mode.color_type) {
        gt_qrcode_set_background(var->obj, gt_color_set(qr_code->param.color0));
        gt_qrcode_set_forecolor(var->obj, gt_color_set(qr_code->param.color1));
    } else {
        gt_qrcode_set_background(var->obj, gt_color_white());
        gt_qrcode_set_forecolor(var->obj, gt_color_black());
    }
}

static void _set_sp_value(uint8_t reg, gt_var_st * var, gt_var_value_st * value) {
    gt_serial_qrcode_st qr_code = {0};

    if (!_get_param(var, &qr_code)) {
        return ;
    }
    // read sp;
    if (0 != gt_serial_var_buffer_get_reg(qr_code.head.sp, var->buffer + 6, qr_code.head.len_dsc*2)) {
        return ;
    }
    if (!_get_param(var, &qr_code)) {
        return ;
    }
    _set_param(var, &qr_code);
}

/* global functions / API interface -------------------------------------*/
gt_res_t gt_serial_qrcode_create(gt_var_st * var)
{
    gt_serial_qrcode_st qr_code = {0};
    if (false == _get_param(var, &qr_code)) {
        return GT_RES_FAIL;
    }
    if (VAR_TYPE_QR_CODE != qr_code.head.type) { return GT_RES_FAIL; }

    gt_obj_st * qr = gt_qrcode_create(var->obj);
    if (NULL == qr) {
        return GT_RES_FAIL;
    }
    _set_qr_code_string(qr, &qr_code);
    gt_obj_set_pos(qr, qr_code.param.pos.x, qr_code.param.pos.y);
    if (qr_code.param.display_mode.color_type) {
        gt_qrcode_set_background(qr, gt_color_set(qr_code.param.color0));
        gt_qrcode_set_forecolor(qr, gt_color_set(qr_code.param.color1));
    } else {
        gt_qrcode_set_background(qr, gt_color_white());
        gt_qrcode_set_forecolor(qr, gt_color_black());
    }

    return GT_RES_OK;
}

gt_res_t gt_serial_qrcode_set_value(gt_var_st * var, gt_var_value_st * value)
{
    int reg = gt_serial_var_check_value_is_set_this(var, value);
    if (reg < 0) { return GT_RES_FAIL; }
    if (0 == reg) {
        _set_vp_value(var, value);
    } else {
        _set_sp_value(reg, var, value);
    }
    return GT_RES_OK;
}



/* end ------------------------------------------------------------------*/
#endif  /** GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT */
