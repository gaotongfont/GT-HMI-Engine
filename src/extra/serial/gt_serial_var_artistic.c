/**
 * @file gt_serial_var_artistic.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-09-30 15:51:20
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_serial_var_artistic.h"
#if GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT
#include "../../utils/gt_bin_convert.h"
#include "../../others/gt_log.h"
#include "../../../gt.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/


/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/
typedef enum{
    VP_DATA_MODE_SHORT = 0,         //整数 2byte -32768~32767
    VP_DATA_MODE_LONG,              //长整数 4byte -2147483648~2147483647
    VP_DATA_MODE_VPH_UCHAR,         //字符 1byte 0~255
    VP_DATA_MODE_VPL_UCHAR,         //字符 1byte 0~255
    VP_DATA_MODE_LONG_LONG,         //长整数 8byte -9223372036854775808~9223372036854775807
    VP_DATA_MODE_USHORT,            //无符号整数 2byte 0~65535
    VP_DATA_MODE_ULONG,             //无符号长整数 4byte 0~4294967295
    VP_DATA_MODE_FLOAT,             //浮点数 4byte
    VP_DATA_MODE_DOUBLE,            //双精度浮点数 8byte
}_gt_vp_data_mode_et;

typedef struct{
    uint16_t x;
    uint16_t y;
    uint16_t icon0;
    uint8_t icon_lib;
    uint8_t icon_mode;
    uint8_t int_numb;
    uint8_t dec_numb;
    uint8_t vp_data_mode;
    uint8_t mode;
    uint8_t layer_mode;
    uint8_t icon_gamma;
    uint8_t pic_gamma;
    uint8_t filter;
    uint16_t w;
    uint16_t h;
}gt_var_artistic_param_st;

typedef struct{
    gt_var_head_st head;
    gt_var_artistic_param_st param;
}gt_var_artistic_st;


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_res_t _parse_param(void * user_data, uint8_t const * const buffer, uint32_t len){
    gt_var_artistic_st * icon = (gt_var_artistic_st *)user_data;
    gt_var_artistic_param_st* param = &icon->param;
    param->x = gt_convert_parse_u16(buffer);
    param->y = gt_convert_parse_u16(buffer + 2);
    param->icon0 = gt_convert_parse_u16(buffer + 4);
    param->icon_lib = buffer[6];
    param->icon_mode = buffer[7];
    param->int_numb = buffer[8];
    param->dec_numb = buffer[9];
    param->vp_data_mode = buffer[10];
    param->mode = buffer[11];
    param->layer_mode = buffer[12];
    param->icon_gamma = buffer[13];
    param->pic_gamma = buffer[14];
    param->filter = buffer[15];
    param->w = gt_convert_parse_u16(buffer + 16);
    param->h = gt_convert_parse_u16(buffer + 18);
    return GT_RES_OK;
}
static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_bin_item_st _format[] = {
    {8, gt_serial_var_head_parse, (print_cb_t)NULL,},
    {20, _parse_param, (print_cb_t)NULL,},
    { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL },
};

static bool _get_param(gt_var_st* var, gt_var_artistic_st* icon)
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
    if(icon->head.type != VAR_TYPE_ARTISTIC) {return false;}

    GT_LOGI(GT_LOG_TAG_SERIAL, "type: %d, sp: 0x%X, vp: 0x%X x: %d, y: %d, ivon_min: %d",\
                                icon->head.type, icon->head.sp, icon->head.vp, icon->param.x, icon->param.y);

    //
    return true;
}

static void _set_text(gt_obj_st* obj, gt_var_artistic_st* icon, uint16_t reg)
{
    uint8_t i_numb = icon->param.int_numb;
    uint8_t d_numb = icon->param.dec_numb;
    uint8_t data[8] = {0};
    bool is_float = false;
    //
    uint8_t len = 2;
    switch (icon->param.vp_data_mode){
        case VP_DATA_MODE_ULONG:
        case VP_DATA_MODE_LONG:
        case VP_DATA_MODE_FLOAT:
            len = 4;
            break;
        case VP_DATA_MODE_LONG_LONG:
        case VP_DATA_MODE_DOUBLE:
            len = 8;
            break;
        case VP_DATA_MODE_SHORT:
        case VP_DATA_MODE_USHORT:
        case VP_DATA_MODE_VPH_UCHAR:
        case VP_DATA_MODE_VPL_UCHAR:
        default:
            len = 2;
            break;
    }

    if(0 != gt_serial_var_buffer_get_reg(reg, data, len)){
        return;
    }

    double number = 0;

    switch (icon->param.vp_data_mode)
    {
    case VP_DATA_MODE_LONG:
        number = (double)gt_convert_parse_i32(data);
        break;
    case VP_DATA_MODE_VPH_UCHAR:
        number = (double)data[0];
        break;
    case VP_DATA_MODE_VPL_UCHAR:
        number = (double)data[1];
        break;
    case VP_DATA_MODE_LONG_LONG:
        number = (double)gt_convert_parse_i64(data);
        break;
    case VP_DATA_MODE_USHORT:
        number = (double)gt_convert_parse_u16(data);
        break;
    case VP_DATA_MODE_ULONG:
        number = (double)gt_convert_parse_u32(data);
        break;
    case VP_DATA_MODE_FLOAT:
        number = (double)gt_convert_parse_float(data);
        is_float = true;
        break;
    case VP_DATA_MODE_DOUBLE:
        number = (double)gt_convert_parse_double(data);
        is_float = true;
        break;
    case VP_DATA_MODE_SHORT:
    default:
        number = (double)gt_convert_parse_i16(data);
        break;
    }

    gt_wordart_set_number(obj, number, i_numb, d_numb, is_float);
}

static void _set_param(gt_var_st* var, gt_var_artistic_st* icon)
{
    if(0 != gt_wordart_get_items_count(var->obj)){
        gt_wordart_remove_all_items(var->obj);
    }
    gt_obj_set_pos(var->obj, icon->param.x, icon->param.y);
    // gt_obj_set_size(var->obj, icon->param.w, icon->param.h);
    gt_obj_set_size(var->obj, 200, 50);

    gt_wordart_items_init(var->obj, 12);

#if GT_USE_FILE_HEADER
    uint8_t tmp[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x2D, 0x2E};
    gt_file_header_param_st fh;
    gt_file_header_param_init(&fh);
    for(int i = 0; i < 12; i++){
        fh.idx = icon->param.icon0 + i;
        gt_wordart_add_item_by_file_header(var->obj, tmp[i], &fh);
    }
#endif
    _set_text(var->obj, icon, icon->head.vp);
}

static void _set_vp_value(gt_var_st* var, gt_var_value_st* value)
{
    gt_var_artistic_st var_icon;

    if(!_get_param(var, &var_icon)){
        return ;
    }
    uint16_t reg = gt_convert_parse_u16(value->buffer);
    _set_text(var->obj, &var_icon, reg);
}

static void _set_sp_value(uint8_t reg, gt_var_st* var, gt_var_value_st* value)
{
    // set sp value
    uint8_t tmp_reg = GT_VAR_GET_SP_REG(reg);
    gt_var_artistic_st var_icon;

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
gt_res_t gt_serial_var_artistic_create(gt_var_st* var)
{
    gt_var_artistic_st var_icon;

    if(!_get_param(var, &var_icon)){
        return GT_RES_INV;
    }

    gt_obj_st* new_obj = gt_wordart_create(var->obj);
    gt_obj_register_id(new_obj, var->id);

    var->obj = new_obj;
    _set_param(var, &var_icon);

    return GT_RES_OK;
}

gt_res_t gt_serial_var_artistic_set_value(gt_var_st* var, gt_var_value_st* value)
{
    _set_value(var, value);
    return GT_RES_OK;
}

/* end of file ----------------------------------------------------------*/
#endif /* GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT */


