/**
 * @file gt_serial_var_icon.c
 * @author Yang (your@email.com)
 * @brief
 * @version 0.1
 * @date 2024-09-24 15:00:32
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_serial_var_icon.h"
#if GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT
#include "../../utils/gt_bin_convert.h"
#include "../../others/gt_log.h"
#include "../../../gt.h"

/* private define -------------------------------------------------------*/


/* private typedef ------------------------------------------------------*/
typedef struct{
    uint16_t x;
    uint16_t y;
    uint16_t v_min;
    uint16_t v_max;
    uint16_t icon_min;
    uint16_t icon_max;
    uint8_t icon_lib;
    uint8_t mode;
    uint8_t layer_mode;
    uint8_t icon_gamma;
    uint8_t pic_gamma;
    uint8_t filter;
}gt_icon_param_st;
typedef struct{
    gt_var_head_st head;
    gt_icon_param_st param;
}gt_var_icon_st;



/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_res_t _parse_param(void * user_data, uint8_t const * const buffer, uint32_t len){
    gt_var_icon_st * icon = (gt_var_icon_st *)user_data;
    gt_icon_param_st* param = &icon->param;

    param->x = buffer[0] << 8 | buffer[1];
    param->y = buffer[2] << 8 | buffer[3];
    param->v_min = buffer[4] << 8 | buffer[5];
    param->v_max = buffer[6] << 8 | buffer[7];
    param->icon_min = buffer[8] << 8 | buffer[9];
    param->icon_max = buffer[10] << 8 | buffer[11];
    param->icon_lib = buffer[12];
    param->mode = buffer[13];
    param->layer_mode = buffer[14];
    param->icon_gamma = buffer[15];
    param->pic_gamma = buffer[16];
    param->filter = buffer[17];
    return GT_RES_OK;
}


static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_bin_item_st _format[] = {
    {8, gt_serial_var_head_parse, (print_cb_t)NULL,},
    {18, _parse_param, (print_cb_t)NULL,},
    { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL },
};

void _set_value(gt_obj_st* obj, uint8_t reg, gt_var_value_st* value)
{
    // set vp value
    if(0 == reg){
        uint16_t index = 0;
        uint8_t data[2];
        if(0 == gt_serial_var_buffer_get(value, data, 2)){
            index = ((data[0] << 8) | data[1]);
        }
        gt_player_set_index(obj, index);
        return ;
    }

    // set sp value
    uint8_t tmp_reg = GT_VAR_GET_SP_REG(reg);
    printf("set sp reg: %d\n", tmp_reg);
    switch(reg)
    {
    case 0:
        break;
    default:
        break;
    }
}

bool _get_var_icon_param(gt_var_st* var, gt_var_icon_st* icon)
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

#if 0
    if(icon->head.sp != 0xFFFF){
        if(0 != gt_serial_var_buffer_get_reg(icon->head.sp, var->buffer + 6, icon->head.len_dsc)){
            GT_LOGE(GT_LOG_TAG_SERIAL, "Get sp value err!!");
            return false;
        }

        ret = gt_bin_convert_parse(&bin, _format);
        if (GT_RES_OK != ret.res) {
            GT_LOGE(GT_LOG_TAG_SERIAL, "Parse err code: %d, index: %d", ret.res, ret.index);
            return false;
        }
    }
#endif

    GT_LOGI(GT_LOG_TAG_SERIAL, "type: %d, sp: 0x%X, vp: 0x%X x: %d, y: %d, ivon_min: %d",\
                                icon->head.type, icon->head.sp, icon->head.vp, icon->param.x, icon->param.y, icon->param.icon_min);
    if(icon->head.type != VAR_TYPE_ICON) {return GT_RES_INV;}

    return true;
}


/* global functions / API interface -------------------------------------*/
gt_res_t gt_serial_var_icon_create(gt_var_st* var)
{
    gt_var_icon_st var_icon;

    if(!_get_var_icon_param(var, &var_icon)){
        return GT_RES_INV;
    }

    gt_obj_st* new_obj = gt_player_create(var->obj);
    gt_obj_register_id(new_obj, var->id);
    gt_obj_set_pos(new_obj, var_icon.param.x, var_icon.param.y);
    gt_obj_set_size(new_obj, 1, 1);
    gt_player_set_type(new_obj, GT_PLAYER_TYPE_IMG);
    gt_player_set_mode(new_obj, GT_PLAYER_MODE_NONE);
    // add imge

    gt_player_add_item(new_obj, "f:img_altitude_78x78.png", sizeof("f:img_altitude_78x78.png"));
	gt_player_add_item(new_obj, "f:img_blood_oxygen18_78x78.png", sizeof("f:img_blood_oxygen18_78x78.png"));
	gt_player_add_item(new_obj, "f:img_heart_rate15_78x78.png", sizeof("f:img_heart_rate15_78x78.png"));
	gt_player_add_item(new_obj, "f:img_information13_78x78.png", sizeof("f:img_information13_78x78.png"));
	gt_player_add_item(new_obj, "f:img_music14_78x78.png", sizeof("f:img_music14_78x78.png"));
	gt_player_add_item(new_obj, "f:img_record_78x78.png", sizeof("f:img_record_78x78.png"));
	gt_player_add_item(new_obj, "f:img_sleep5_78x78.png", sizeof("f:img_sleep5_78x78.png"));
	gt_player_add_item(new_obj, "f:img_sports14_78x78.png", sizeof("f:img_sports14_78x78.png"));
	gt_player_add_item(new_obj, "f:img_sports_record14_78x78.png", sizeof("f:img_sports_record14_78x78.png"));


    gt_player_set_index_offset(new_obj, var_icon.param.icon_min);
    uint16_t index = 0;
    uint8_t data[2];
    if(0 == gt_serial_var_buffer_get_reg(var_icon.head.vp, data, 2)){
        index = ((data[0] << 8) | data[1]);
    }
    gt_player_set_index(new_obj, index);
    printf(">> index: %d\n", index);

    return GT_RES_OK;
}

gt_res_t gt_serial_var_icon_set_value(gt_var_st* var, gt_var_value_st* value)
{
    uint8_t reg = gt_serial_var_check_value_is_set_this(var, value);
    if(reg < 0) { return GT_RES_FAIL; }
    _set_value( var->obj, reg, value);
    return GT_RES_OK;
}


/* end of file ----------------------------------------------------------*/
#endif /* GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT */

