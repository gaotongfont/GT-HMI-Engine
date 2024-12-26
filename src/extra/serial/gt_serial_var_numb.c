/**
 * @file gt_serial_var_numb.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-10-18 15:42:38
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_serial_var_numb.h"
#if GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT
#include "../../utils/gt_bin_convert.h"
#include "../../others/gt_log.h"
#include "../../../gt.h"
#include "float.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/
typedef enum{
    _TYPE_SHORT = 0,         //整数 2byte -32768~32767
    _TYPE_LONG,              //长整数 4byte -2147483648~2147483647
    _TYPE_VPH_UCHAR,         //字符 1byte 0~255
    _TYPE_VPL_UCHAR,         //字符 1byte 0~255
    _TYPE_LONG_LONG,         //长整数 8byte -9223372036854775808~9223372036854775807
    _TYPE_USHORT,            //无符号整数 2byte 0~65535
    _TYPE_ULONG,             //无符号长整数 4byte 0~4294967295
    _TYPE_FLOAT,             //浮点数 4byte
    _TYPE_DOUBLE,            //双精度浮点数 8byte
}_gt_vp_data_mode_et;

typedef struct{
    uint16_t x;
    uint16_t y;
    uint16_t color;
    uint8_t family;
    uint8_t size;
    uint8_t align;
    uint8_t integer;
    uint8_t decimal;
    uint8_t type;
    uint8_t unit_len;
    uint8_t unit_str[7];
    uint16_t w;
    uint16_t h;
}gt_numb_param_st;

typedef struct{
    gt_var_head_st head;
    gt_numb_param_st param;
}gt_var_numb_st;



/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_res_t _parse_param(void * user_data, uint8_t const * const buffer, uint32_t len){
    gt_var_numb_st * icon = (gt_var_numb_st *)user_data;
    gt_numb_param_st* param = &icon->param;

    param->x = gt_convert_parse_u16(buffer);
    param->y = gt_convert_parse_u16(buffer + 2);
    param->color = gt_convert_parse_u16(buffer + 4);
    param->family = buffer[6];
    param->size = buffer[7];
    param->align = buffer[8];
    param->integer = buffer[9];
    param->decimal = buffer[10];
    param->type = buffer[11];
    param->unit_len = buffer[12];
    memcpy(param->unit_str, buffer + 13, 6);
    param->unit_str[(param->unit_len < 6 ? param->unit_len : 6)] = '\0';
    param->w = gt_convert_parse_u16(buffer + 20);
    param->h = gt_convert_parse_u16(buffer + 22);
    return GT_RES_OK;
}


static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_bin_item_st _format[] = {
    {8, gt_serial_var_head_parse, (print_cb_t)NULL,},
    {24, _parse_param, (print_cb_t)NULL,},
    { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL },
};

static bool _get_param(gt_var_st* var, gt_var_numb_st* icon)
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
    if(icon->head.type != VAR_TYPE_NUMB) {return false;}

    GT_LOGI(GT_LOG_TAG_SERIAL, "type: %d, sp: 0x%X, vp: 0x%X x: %d, y: %d",\
                                icon->head.type, icon->head.sp, icon->head.vp, icon->param.x, icon->param.y);

    //
    return true;
}

static void _set_number(gt_obj_st* obj, gt_var_numb_st* icon, uint16_t reg)
{
    uint8_t len = 2;
    uint8_t data[8] = {0};
    switch (icon->param.type){
        case _TYPE_ULONG:
        case _TYPE_LONG:
        case _TYPE_FLOAT:
            len = 4;
            break;
        case _TYPE_LONG_LONG:
        case _TYPE_DOUBLE:
            len = 8;
            break;
        case _TYPE_SHORT:
        case _TYPE_USHORT:
        case _TYPE_VPH_UCHAR:
        case _TYPE_VPL_UCHAR:
        default:
            len = 2;
            break;
    }

    if(0 != gt_serial_var_buffer_get_reg(reg, data, len)){
        return;
    }
    double numb = 0, min = 0, max = 0;
    switch (icon->param.type)
    {
    case _TYPE_LONG:
        numb = (double)gt_convert_parse_i32(data);
        min = INT32_MIN;
        max = INT32_MAX;
        break;
    case _TYPE_VPH_UCHAR:
        numb = (double)data[0];
        min = 0;
        max = UINT8_MAX;
        break;
    case _TYPE_VPL_UCHAR:
        numb = (double)data[1];
        min = 0;
        max = UINT8_MAX;
        break;
    case _TYPE_LONG_LONG:
        numb = (double)gt_convert_parse_i64(data);
        min = (double)INT64_MIN;
        max = (double)INT64_MAX;
        break;
    case _TYPE_USHORT:
        numb = (double)gt_convert_parse_u16(data);
        min = 0;
        max = UINT16_MAX;
        break;
    case _TYPE_ULONG:
        numb = (double)gt_convert_parse_u32(data);
        min = 0;
        max = UINT32_MAX;
        break;
    case _TYPE_FLOAT:
        numb = (double)gt_convert_parse_float(data);
        min = -FLT_MAX;
        max = FLT_MAX;
        break;
    case _TYPE_DOUBLE:
        numb = (double)gt_convert_parse_double(data);
        min = -DBL_MAX;
        max = DBL_MAX;
        break;
    default:
    case _TYPE_SHORT:
        numb = (double)gt_convert_parse_i16(data);
        min = INT16_MIN;
        max = INT16_MAX;
        break;
    }

    gt_input_number_set_min(obj, min);
    gt_input_number_set_max(obj, max);
    gt_input_number_set_uint(obj, icon->param.unit_str);
    gt_input_number_set_value(obj, numb);

}

static void _set_param(gt_var_st* var, gt_var_numb_st* icon)
{
    gt_obj_set_pos(var->obj, icon->param.x, icon->param.y);
    gt_obj_set_size(var->obj, icon->param.w, icon->param.h);
    gt_input_number_set_font_color(var->obj, gt_color_set(icon->param.color));

    bool file_zero = 0x01 & (icon->param.align >> 6);
    gt_input_number_set_fill_zero_front(var->obj, file_zero);
    uint8_t align = icon->param.align & 0x0F;
    if(0x01 == align){
        gt_input_number_set_font_align(var->obj, GT_ALIGN_RIGHT);
    }
    else if(0x02 == align){
        gt_input_number_set_font_align(var->obj, GT_ALIGN_CENTER);
    }
    else {
        gt_input_number_set_font_align(var->obj, GT_ALIGN_LEFT);
    }
    // gt_input_number_set_font_size(var->obj, icon->param.size);
    gt_input_number_set_font_family(var->obj, icon->param.family);

    gt_input_number_set_display_integer_length(var->obj, icon->param.integer);
    gt_input_number_set_display_decimal_length(var->obj, icon->param.decimal);
    gt_input_number_set_step(var->obj, 1);
    _set_number(var->obj, icon, icon->head.vp);
}
static void _set_vp_value(gt_var_st* var, gt_var_value_st* value)
{
    gt_var_numb_st var_icon;

    if(!_get_param(var, &var_icon)){
        return ;
    }

    uint16_t reg = gt_convert_parse_u16(value->buffer);
    _set_number(var->obj, &var_icon, reg);
}

static void _set_sp_value(uint8_t reg, gt_var_st* var, gt_var_value_st* value)
{
    // set sp value
    uint8_t tmp_reg = GT_VAR_GET_SP_REG(reg);
    gt_var_numb_st var_icon;

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
gt_res_t gt_serial_var_numb_create(gt_var_st* var)
{
    gt_var_numb_st var_icon;

    if(!_get_param(var, &var_icon)){
        return GT_RES_INV;
    }

    gt_obj_st* new_obj = gt_input_number_create(var->obj);
    gt_obj_register_id(new_obj, var->id);

    var->obj = new_obj;
    _set_param(var, &var_icon);

    return GT_RES_OK;
}

gt_res_t gt_serial_var_numb_set_value(gt_var_st* var, gt_var_value_st* value)
{
    _set_value(var, value);
    return GT_RES_OK;
}
/* end of file ----------------------------------------------------------*/
#endif /* GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT */

