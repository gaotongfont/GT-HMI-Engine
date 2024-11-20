/**
 * @file gt_serial_var_roller.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-10-24 16:19:09
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_serial_var_roller.h"
#if GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT
#include "../../utils/gt_bin_convert.h"
#include "../../others/gt_log.h"
#include "../../../gt.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/
/* disp mode
bit 0: 1-fill zero  0-no fill zero
bit 1: 1-fill +     0-no fill +
bit 2: 1-Infinite   0-Limited
*/
typedef struct{
    int16_t v_min;
    int16_t v_max;
    uint8_t integer;
    uint8_t decimal;
    uint8_t numb_digit;
    uint8_t disp_mode;
    uint16_t x;
    uint16_t y;
    uint16_t step;
    uint8_t font0_x;
    uint8_t font0_y;
    uint16_t font0_color;
    uint16_t w;
    uint16_t select_color;
    uint8_t font_lib;
}gt_roller_param_st;

typedef struct{
    gt_var_head_st head;
    gt_roller_param_st param;
}gt_var_roller_st;

/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_res_t _parse_param(void * user_data, uint8_t const * const buffer, uint32_t len){
    gt_var_roller_st * icon = (gt_var_roller_st *)user_data;
    gt_roller_param_st* param = &icon->param;

    param->v_min = gt_convert_parse_i16(buffer);
    param->v_max = gt_convert_parse_i16(buffer + 2);
    param->integer = buffer[4];
    param->decimal = buffer[5];
    param->numb_digit = buffer[6];
    param->disp_mode = buffer[7];
    param->x = gt_convert_parse_u16(buffer + 8);
    param->y = gt_convert_parse_u16(buffer + 10);
    param->step = gt_convert_parse_u16(buffer + 12);
    param->font0_x = buffer[14];
    param->font0_y = buffer[15];
    param->font0_color = gt_convert_parse_u16(buffer + 16);
    param->w = gt_convert_parse_u16(buffer + 18);
    param->select_color = gt_convert_parse_u16(buffer + 20);
    param->font_lib = buffer[22];
    return GT_RES_OK;
}

static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_bin_item_st _format[] = {
    {8, gt_serial_var_head_parse, (print_cb_t)NULL,},
    {23, _parse_param, (print_cb_t)NULL,},
    { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL },
};

static bool _get_param(gt_var_st* var, gt_var_roller_st* icon)
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
    if(icon->head.type != VAR_TYPE_ROLLER) {return false;}

    GT_LOGI(GT_LOG_TAG_SERIAL, "type: %d, sp: 0x%X, vp: 0x%X x: %d, y: %d",\
                                icon->head.type, icon->head.sp, icon->head.vp, icon->param.x, icon->param.y);

    //
    return true;
}

static void _set_options(gt_var_st* var, gt_var_roller_st* icon)
{
    bool fill_zero = icon->param.disp_mode & 0x01;
    bool fill_pull_sign = (icon->param.disp_mode >> 1) & 0x01;
    uint8_t mode = (icon->param.disp_mode >> 2) & 0x01;
    mode = mode ? GT_ROLLER_MODE_INFINITE : GT_ROLLER_MODE_NORMAL;

    uint8_t idx = 0;
    uint8_t append_len = icon->param.decimal ? (icon->param.decimal + 1) : icon->param.decimal;

    char buffer[32] = {0};

    buffer[idx++] = '%';
    if(fill_pull_sign){
        buffer[idx++] = '+';
    }

    if(fill_zero){
        buffer[idx++] = '0';
    }

    sprintf(&buffer[idx], "%d.%dlf", icon->param.integer + append_len, icon->param.decimal);

    uint32_t n_power = 1;
    for (int i = 0; i < icon->param.decimal; i++) {
        n_power *= 10;
    }
    double min = (double)icon->param.v_min / n_power;
    double max = (double)icon->param.v_max / n_power;
    double step = (double)icon->param.step / n_power;

    gt_roller_set_options_numb(var->obj, mode, buffer, min, max, step);
}

static void _set_selected(gt_obj_st* obj, gt_var_roller_st* icon, uint16_t reg)
{

    uint8_t data[2] = {0};
    if(0 != gt_serial_var_buffer_get_reg(reg, data, 2)){
        return ;
    }

    bool fill_zero = icon->param.disp_mode & 0x01;
    bool fill_pull_sign = (icon->param.disp_mode >> 1) & 0x01;
    uint8_t mode = (icon->param.disp_mode >> 2) & 0x01;
    mode = mode ? GT_ROLLER_MODE_INFINITE : GT_ROLLER_MODE_NORMAL;

    uint8_t idx = 0;
    uint8_t append_len = icon->param.decimal ? (icon->param.decimal + 1) : icon->param.decimal;

    char buffer[32] = {0};

    buffer[idx++] = '%';
    if(fill_pull_sign){
        buffer[idx++] = '+';
    }

    if(fill_zero){
        buffer[idx++] = '0';
    }

    sprintf(&buffer[idx], "%d.%dlf", icon->param.integer + append_len, icon->param.decimal);

    uint32_t n_power = 1;
    for (int i = 0; i < icon->param.decimal; i++) {
        n_power *= 10;
    }

    double value = (double)gt_convert_parse_u16(data);
    value /= n_power;

    char str[16] = {0};
    sprintf(str, buffer, value);
    gt_roller_set_selected_text(obj, str);
}

static void _set_param(gt_var_st* var, gt_var_roller_st* icon)
{
    gt_obj_set_pos(var->obj, icon->param.x, icon->param.y);
    uint16_t h = icon->param.font0_y * icon->param.numb_digit;
    gt_obj_set_size(var->obj, icon->param.w, h);

    gt_roller_set_font_align(var->obj, GT_ALIGN_CENTER_MID);
    gt_roller_set_font_color(var->obj, gt_color_set(icon->param.font0_color));
    gt_roller_set_display_item_count(var->obj, icon->param.numb_digit);
    gt_roller_set_select_bgcolor(var->obj, gt_color_set(icon->param.select_color));
    // gt_roller_set_font_size(var->obj, icon->param.font0_x);
    gt_roller_set_font_family(var->obj, icon->param.font_lib);

    _set_options(var, icon);
    _set_selected(var->obj, icon, icon->head.vp);
}

static void _set_vp_value(gt_var_st* var, gt_var_value_st* value)
{
    gt_var_roller_st var_icon;

    if(!_get_param(var, &var_icon)){
        return ;
    }

    uint16_t reg = gt_convert_parse_u16(value->buffer);
    _set_selected(var->obj, &var_icon, reg);
}

static void _set_sp_value(uint8_t reg, gt_var_st* var, gt_var_value_st* value)
{
    // set sp value
    uint8_t tmp_reg = GT_VAR_GET_SP_REG(reg);
    gt_var_roller_st var_icon;

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
gt_res_t gt_serial_var_roller_create(gt_var_st* var)
{
    gt_var_roller_st var_icon;

    if(!_get_param(var, &var_icon)){
        return GT_RES_INV;
    }

    gt_obj_st* new_obj = gt_roller_create(var->obj);
    gt_obj_register_id(new_obj, var->id);

    var->obj = new_obj;
    _set_param(var, &var_icon);

    return GT_RES_OK;
}

gt_res_t gt_serial_var_roller_set_value(gt_var_st* var, gt_var_value_st* value)
{
    _set_value(var, value);
    return GT_RES_OK;
}

/* end of file ----------------------------------------------------------*/
#endif /* GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT */

