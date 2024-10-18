/**
 * @file gt_serial_value_passing.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-10-15 16:43:50
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_serial_value_passing.h"

#if GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT
#include "../../core/gt_mem.h"
#include "./gt_serial_show.h"
#include "./gt_serial_system.h"
#include "../../utils/gt_bin_convert.h"


/* private define -------------------------------------------------------*/
#define GT_VALUE_PASSING_PRE_DEFINE_DATA_SIZE    22


/* private typedef ------------------------------------------------------*/
typedef struct {
    uint8_t auto_com_en;            /** 0xff: upload variable data to user */
    uint8_t reserved;               /** 0x00 */
    uint8_t pre_define_data[GT_VALUE_PASSING_PRE_DEFINE_DATA_SIZE];
}gt_value_passing_param_st;

typedef struct {
    gt_var_head_st head;
    gt_value_passing_param_st param;
}gt_value_passing_st;


/* static variables -----------------------------------------------------*/
static gt_res_t _parse_param(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_value_passing_st * val = (gt_value_passing_st *)user_data;
    gt_value_passing_param_st * param = &val->param;

    param->auto_com_en = buffer[0];
    gt_memcpy(param->pre_define_data, &buffer[2], GT_VALUE_PASSING_PRE_DEFINE_DATA_SIZE);
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
static void _set_vp_value(gt_var_st* var, gt_var_value_st* value) {

}

static void _set_sp_value(uint8_t reg, gt_var_st* var, gt_var_value_st* value)
{
}

static void _set_value(gt_var_st* var, gt_var_value_st* value) {
    int reg = gt_serial_var_check_value_is_set_this(var, value);
    if (reg < 0) { return ; }
    // set vp value
    if (0 == reg) {
        _set_vp_value(var, value);
    } else {
        _set_sp_value(reg, var, value);
    }
}



/* global functions / API interface -------------------------------------*/
gt_res_t gt_serial_value_passing_create(gt_var_st * var)
{
    gt_value_passing_st value_passing = {0};
    gt_bin_convert_st bin = {
        .tar = &value_passing,
        .buffer = var->buffer,
        .byte_len = var->len,
    };
    gt_bin_res_st ret = gt_bin_convert_parse(&bin, _format);
    if (GT_RES_OK != ret.res) {
        GT_LOGE(GT_LOG_TAG_SERIAL, "Parse err code: %d, index: %d", ret.res, ret.index);
        return GT_RES_FAIL;
    }
    GT_LOGI(GT_LOG_TAG_SERIAL, "var type: %d", value_passing.head.type);
    if (VAR_TYPE_VALUE_PASSING != value_passing.head.type) { return GT_RES_FAIL; }

    /** move pre_defined data to vp buffer */
    uint8_t cmd[26] = {0}, cnt = 0;
    cnt = gt_convert_print_u16(cmd, value_passing.head.vp);
    cnt += gt_serial_system_get_value_by_addr(&cmd[cnt], GT_SERIAL_SYS_ADDR_PIC_NOW, 1);
    gt_memcpy(&cmd[cnt], value_passing.param.pre_define_data, GT_VALUE_PASSING_PRE_DEFINE_DATA_SIZE);
    cnt += GT_VALUE_PASSING_PRE_DEFINE_DATA_SIZE;
    gt_serial_set_value(cmd, cnt);

    if (0xff == value_passing.param.auto_com_en) {
        // auto upload data
    }

    return GT_RES_OK;
}

gt_res_t gt_serial_value_passing_set_value(gt_var_st * var, gt_var_value_st * value)
{
    return GT_RES_OK;
}



/* end ------------------------------------------------------------------*/
#endif  /** GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT */
