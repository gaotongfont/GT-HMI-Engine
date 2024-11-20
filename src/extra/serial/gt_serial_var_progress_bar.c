/**
 * @file gt_serial_var_progress_bar.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-10-29 10:43:32
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_serial_var_progress_bar.h"

#if GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT
#include "../../utils/gt_bin_convert.h"
#include "../../others/gt_log.h"
#include "../../../gt.h"
/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/
typedef struct{
    uint16_t xs;
    uint16_t ys;
    uint16_t xe;
    uint16_t ye;
    uint16_t b_color;
    uint16_t f_color;
    uint16_t bg_color;
    int16_t max;
    int16_t min;
    uint8_t mode;
    uint8_t dir;
    uint8_t type;
    uint8_t reserved;
    uint16_t vp_rt;
}gt_progress_bar_param_st;

typedef struct{
    gt_var_head_st head;
    gt_progress_bar_param_st param;
}gt_var_progress_bar_st;


/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_res_t _parse_param(void * user_data, uint8_t const * const buffer, uint32_t len){
    gt_var_progress_bar_st * icon = (gt_var_progress_bar_st *)user_data;
    gt_progress_bar_param_st* param = &icon->param;

    param->xs = gt_convert_parse_u16(buffer);
    param->ys = gt_convert_parse_u16(buffer + 2);
    param->xe = gt_convert_parse_u16(buffer + 4);
    param->ye = gt_convert_parse_u16(buffer + 6);
    param->b_color = gt_convert_parse_u16(buffer + 8);
    param->f_color = gt_convert_parse_u16(buffer + 10);
    param->bg_color = gt_convert_parse_u16(buffer + 12);
    param->max = gt_convert_parse_i16(buffer + 14);
    param->min = gt_convert_parse_i16(buffer + 16);
    param->mode = buffer[18];
    param->dir = buffer[19];
    param->type = buffer[20];
    param->reserved = buffer[21];
    param->vp_rt = gt_convert_parse_u16(buffer + 22);

    return GT_RES_OK;
}

static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_bin_item_st _format[] = {
    {8, gt_serial_var_head_parse, (print_cb_t)NULL,},
    {24, _parse_param, (print_cb_t)NULL,},
    { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL },
};

static bool _get_param(gt_var_st* var, gt_var_progress_bar_st* icon)
{
    gt_bin_convert_st bin = {
        .tar = icon,
        .buffer = var->buffer,
        .byte_len = var->len,
    };

    gt_bin_res_st ret = {0};
    ret = gt_bin_convert_parse(&bin, _format);
    if (GT_RES_OK != ret.res) {
        GT_LOGE(GT_LOG_TAG_SERIAL, "Parse err code: %d, index: %d", ret.res, ret.index);
        return false;
    }
    GT_LOGI(GT_LOG_TAG_SERIAL, "var type: %d", icon->head.type);
    if(icon->head.type != VAR_TYPE_PROGRESS_BAR) {return false;}

    GT_LOGI(GT_LOG_TAG_SERIAL, "type: %d, sp: 0x%X, vp: 0x%X x: %d, y: %d",\
                                icon->head.type, icon->head.sp, icon->head.vp, icon->param.xs, icon->param.ys);

    //
    return true;
}

static void _set_pos(gt_obj_st* obj, gt_var_progress_bar_st* icon, uint16_t reg)
{
    uint8_t data[2] = {0};
    if(0 != gt_serial_var_buffer_get_reg(reg, data, 2)){
        return ;
    }
    int16_t pos = 0;
    if(0x01 == icon->param.type){
        pos = data[0];
    }
    else if(0x02 == icon->param.type){
        pos = data[1];
    }
    else{
        pos = gt_convert_parse_i16(data);
    }
    gt_progress_bar_set_pos(obj, pos);
}

static void _set_param(gt_var_st* var, gt_var_progress_bar_st* icon)
{
    uint16_t w = icon->param.xe - icon->param.xs + 1;
    uint16_t h = icon->param.ye - icon->param.ys + 1;
    gt_obj_set_pos(var->obj, icon->param.xs, icon->param.ys);
    gt_obj_set_size(var->obj, w, h);
    gt_progress_bar_set_color_ina(var->obj, gt_color_set(icon->param.bg_color));
    gt_progress_bar_set_color_act(var->obj, gt_color_set(icon->param.f_color));
    gt_progress_bar_set_dir(var->obj, icon->param.dir);
    gt_progress_bar_set_start_end(var->obj, icon->param.min, icon->param.max);

    _set_pos(var->obj, icon, icon->head.vp);
}

static void _set_vp_value(gt_var_st* var, gt_var_value_st* value)
{
    gt_var_progress_bar_st var_icon;

    if(!_get_param(var, &var_icon)){
        return ;
    }

    uint16_t reg = gt_convert_parse_u16(value->buffer);
    _set_pos(var->obj, &var_icon, reg);
}

static void _set_sp_value(uint8_t reg, gt_var_st* var, gt_var_value_st* value)
{
    // set sp value
    uint8_t tmp_reg = GT_VAR_GET_SP_REG(reg);
    gt_var_progress_bar_st var_icon;

    if(!_get_param(var, &var_icon)){
        return ;
    }

    // read sp;
    if(0 != gt_serial_var_buffer_get_reg(var_icon.head.sp, var->buffer + 6, var_icon.head.len_dsc*2)){
        return ;
    }

    if(!_get_param(var, &var_icon)){
        return ;
    }

    _set_param(var, &var_icon);
}
static void _set_value(gt_var_st* var, gt_var_value_st* value)
{
    int reg = gt_serial_var_check_value_is_set_this(var, value);
    if(reg < 0) { return ; }
    // set vp value
    if(0 == reg){
        _set_vp_value(var, value);
    }
    else{
        _set_sp_value(reg, var, value);
    }
}
/* global functions / API interface -------------------------------------*/
gt_res_t gt_serial_var_progress_bar_create(gt_var_st* var)
{
    gt_var_progress_bar_st var_icon;

    if(!_get_param(var, &var_icon)){
        return GT_RES_INV;
    }

    gt_obj_st* new_obj = gt_progress_bar_create(var->obj);
    gt_obj_register_id(new_obj, var->id);

    var->obj = new_obj;
    _set_param(var, &var_icon);

    return GT_RES_OK;
}

gt_res_t gt_serial_var_progress_bar_set_value(gt_var_st* var, gt_var_value_st* value)
{
    _set_value(var, value);
    return GT_RES_OK;
}
/* end of file ----------------------------------------------------------*/
#endif /* GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT */

