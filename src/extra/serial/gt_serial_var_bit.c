/**
 * @file gt_serial_var_bit.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-10-11 17:04:42
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_serial_var_bit.h"
#if GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT
#include "../../utils/gt_bin_convert.h"
#include "../../others/gt_log.h"
#include "../../../gt.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/

typedef struct{
    uint16_t vp_aux;
    uint16_t bit_set;
    uint8_t disp_mode;
    uint8_t move_mode;
    uint8_t icon_mode;
    uint8_t icon_lib;
    uint16_t icon0s;
    uint16_t icon0e;
    uint16_t icon1s;
    uint16_t icon1e;
    uint16_t x;
    uint16_t y;
    uint16_t move_gap;
    uint8_t filter;
    uint8_t time;       // unit: 20ms
}gt_var_bit_param_st;

typedef struct{
    gt_var_head_st head;
    gt_var_bit_param_st param;
}gt_var_bit_st;


/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_res_t _parse_param(void * user_data, uint8_t const * const buffer, uint32_t len) {
    gt_var_bit_st * icon = (gt_var_bit_st *)user_data;
    gt_var_bit_param_st* param = &icon->param;
    param->vp_aux = gt_convert_parse_u16(buffer);
    param->bit_set = gt_convert_parse_u16(buffer + 2);
    param->disp_mode = buffer[4];
    param->move_mode = buffer[5];
    param->icon_mode = buffer[6];
    param->icon_lib = buffer[7];
    param->icon0s = gt_convert_parse_u16(buffer + 8);
    param->icon0e = gt_convert_parse_u16(buffer + 10);
    param->icon1s = gt_convert_parse_u16(buffer + 12);
    param->icon1e = gt_convert_parse_u16(buffer + 14);
    param->x = gt_convert_parse_u16(buffer + 16);
    param->y = gt_convert_parse_u16(buffer + 18);
    param->move_gap = gt_convert_parse_u16(buffer + 20);
    param->filter = buffer[22];
    param->time = buffer[23];
    return GT_RES_OK;
}
static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_bin_item_st _format[] = {
    {8, gt_serial_var_head_parse, (print_cb_t)NULL,},
    {24, _parse_param, (print_cb_t)NULL,},
    { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL },
};

static bool _get_param(gt_var_st* var, gt_var_bit_st* icon)
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
    if(icon->head.type != VAR_TYPE_BIT) {return false;}

    GT_LOGI(GT_LOG_TAG_SERIAL, "type: %d, sp: 0x%X, vp: 0x%X set: 0x%04X, x: %d, y: %d\r\n",\
                                icon->head.type, icon->head.sp, icon->head.vp, icon->param.bit_set, icon->param.x, icon->param.y);

    //
    return true;
}

static void _set_value_reg(gt_obj_st* obj, uint16_t reg)
{
    uint8_t data[2] = {0};
    uint16_t val = 2;
    if(0 != gt_serial_var_buffer_get_reg(reg, data, 2)){
        return;
    }
    val = gt_convert_parse_u16(data);
    gt_bit_img_set_value(obj, val);
}

static void _set_param(gt_var_st* var, gt_var_bit_st* icon)
{
    gt_bit_img_remove_all_items(var->obj);

#if GT_USE_FILE_HEADER
    gt_file_header_param_st fh;
    gt_file_header_param_init(&fh);
    for (uint16_t i = icon->param.icon0s; i <= icon->param.icon0e; ++i) {
        fh.idx = i;
        gt_bit_img_add_item0_by_file_header(var->obj, &fh);
    }

    for(uint16_t i = icon->param.icon1s; i <= icon->param.icon1e; ++i) {
        fh.idx = i;
        gt_bit_img_add_item1_by_file_header(var->obj, &fh);
    }
#endif

    gt_obj_set_pos(var->obj, icon->param.x, icon->param.y);
    gt_bit_img_set_disp_mode(var->obj, icon->param.disp_mode);
    gt_bit_img_set_move_mode(var->obj, icon->param.move_mode);
    gt_bit_img_set_move_gap(var->obj, icon->param.move_gap);
    icon->param.time = icon->param.time ? icon->param.time : 1;
    gt_bit_img_set_time(var->obj, icon->param.time * 20);
    gt_bit_img_set_active(var->obj, icon->param.bit_set);
    _set_value_reg(var->obj, icon->head.vp);
}

static void _set_vp_value(gt_var_st* var, gt_var_value_st* value)
{
    uint8_t data[2] = {0};
    uint16_t val = 2;
    if(0 != gt_serial_var_buffer_get(value, data, 2)){
        return;
    }
    val = gt_convert_parse_u16(data);
    gt_bit_img_set_value(var->obj, val);
}

static void _set_sp_value(uint8_t reg, gt_var_st* var, gt_var_value_st* value)
{
    // set sp value
    uint8_t tmp_reg = GT_VAR_GET_SP_REG(reg);
    gt_var_bit_st var_icon;

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
gt_res_t gt_serial_var_bit_create(gt_var_st* var)
{
    gt_var_bit_st var_icon;

    if(!_get_param(var, &var_icon)){
        return GT_RES_INV;
    }

    gt_obj_st* new_obj = gt_bit_img_create(var->obj);
    gt_obj_register_id(new_obj, var->id);
    gt_obj_set_size(new_obj, 1, 1);

    var->obj = new_obj;
    _set_param(var, &var_icon);

    return GT_RES_OK;
}

gt_res_t gt_serial_var_bit_set_value(gt_var_st* var, gt_var_value_st* value)
{
    _set_value(var, value);
    return GT_RES_OK;
}

/* end of file ----------------------------------------------------------*/
#endif /* GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT */


