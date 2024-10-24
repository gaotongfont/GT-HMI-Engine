/**
 * @file gt_serial_area_light.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-10-18 16:30:51
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_serial_area_light.h"

#if GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT
#include "stdint.h"
#include "../../core/gt_mem.h"
#include "./gt_serial_show.h"
#include "./gt_serial_system.h"
#include "../../utils/gt_bin_convert.h"
#include "../../others/gt_log.h"
#include "../../core/gt_style.h"
#include "../../widgets/gt_rect.h"
#include "../../widgets/gt_obj.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/
typedef struct {
    gt_point_st start;  /** left of top start point */
    gt_point_st end;    /** right of bottom end point */
    uint8_t reserved[18];
}gt_area_light_param_st;

typedef struct {
    gt_var_head_st head;
    gt_area_light_param_st param;
}gt_area_light_st;

/* static variables -----------------------------------------------------*/
static gt_res_t _parse_param(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_area_light_st * val = (gt_area_light_st *)user_data;
    gt_area_light_param_st * param = &val->param;

    param->start.x = gt_convert_parse_u16(buffer);
    param->start.y = gt_convert_parse_u16(&buffer[2]);
    param->end.x = gt_convert_parse_u16(&buffer[4]);
    param->end.y = gt_convert_parse_u16(&buffer[6]);
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
static bool _get_param(gt_var_st * var, gt_area_light_st * area_light) {
    gt_bin_convert_st bin = {
        .tar = area_light,
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

static void _set_param(gt_var_st* var, gt_area_light_st * area_light) {
    gt_obj_set_pos(var->obj, area_light->param.start.x, area_light->param.start.y);
    gt_obj_set_size(var->obj, area_light->param.end.x - area_light->param.start.x, area_light->param.end.y - area_light->param.start.y);
}

static void _set_vp_value(gt_var_st * var, gt_var_value_st * value) {
    uint8_t data[2];
    uint16_t opa = 0;
    if(0 == gt_serial_var_buffer_get(value, data, 2)){
        opa = gt_convert_parse_u16(data);
    }
    if (opa > GT_OPA_100) { opa = GT_OPA_100; }
    gt_obj_set_opa(var->obj, opa);
}

static void _set_sp_value(uint8_t reg, gt_var_st * var, gt_var_value_st * value) {
    // set sp value
    uint8_t tmp_reg = GT_VAR_GET_SP_REG(reg);
    gt_area_light_st area_light;

    if (!_get_param(var, &area_light)) {
        return ;
    }

    // read sp;
    if (0 != gt_serial_var_buffer_get_reg(area_light.head.sp, var->buffer + 6, area_light.head.len_dsc * 2)) {
        return ;
    }

    if (!_get_param(var, &area_light)) {
        return ;
    }

    _set_param(var, &area_light);
}

/* global functions / API interface -------------------------------------*/
gt_res_t gt_serial_area_light_create(gt_var_st * var)
{
    gt_area_light_st area_light = {0};
    if (false == _get_param(var, &area_light)) {
        return GT_RES_FAIL;
    }
    if (VAR_TYPE_AREA_LIGHT != area_light.head.type) { return GT_RES_FAIL; }

    uint8_t vp_data[2] = {0};
    gt_serial_var_buffer_get_reg(area_light.head.vp, vp_data, 2);
    uint16_t value = gt_convert_parse_u16(vp_data);

    gt_obj_st * new_obj = gt_rect_create(var->obj);
    gt_obj_register_id(new_obj, var->id);
    gt_obj_set_bgcolor(new_obj, gt_color_white());
    gt_obj_set_opa(new_obj, value);
    gt_obj_set_pos(new_obj, area_light.param.start.x, area_light.param.start.y);
    gt_obj_set_size(new_obj, area_light.param.end.x - area_light.param.start.x, area_light.param.end.y - area_light.param.start.y);

    return GT_RES_OK;
}

gt_res_t gt_serial_area_light_set_value(gt_var_st * var, gt_var_value_st * value)
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
