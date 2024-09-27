/**
 * @file gt_serial_var_anim_icon.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-09-26 11:07:56
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_serial_var_anim_icon.h"
#if GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT
#include "../../utils/gt_bin_convert.h"
#include "../../others/gt_log.h"
#include "../../../gt.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/
typedef struct{
    uint16_t x;
    uint16_t y;
    uint16_t reset_icon_en;
    uint16_t v_stop;
    uint16_t v_start;
    uint16_t icon_stop;
    uint16_t icon_start;
    uint16_t icon_end;
    uint8_t icon_lib;
    uint8_t mode;
    uint8_t layer_mode;
    uint8_t icon_gamma;
    uint8_t pic_gamma;
    uint8_t time;
    uint8_t anim_mode;
    uint8_t filter;
}gt_anim_icon_param_st;

typedef struct{
    gt_var_head_st head;
    gt_anim_icon_param_st param;
}gt_var_anim_icon_st;


/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_res_t _parse_param(void * user_data, uint8_t const * const buffer, uint32_t len){
    gt_var_anim_icon_st * var = (gt_var_anim_icon_st *)user_data;
    gt_anim_icon_param_st* param = &var->param;

    param->x = gt_convert_parse_u16(buffer);
    param->y = gt_convert_parse_u16(buffer + 2);
    param->reset_icon_en = gt_convert_parse_u16(buffer + 4);
    param->v_stop = gt_convert_parse_u16(buffer + 6);
    param->v_start = gt_convert_parse_u16(buffer + 8);
    param->icon_stop = gt_convert_parse_u16(buffer + 10);
    param->icon_start = gt_convert_parse_u16(buffer + 12);
    param->icon_end = gt_convert_parse_u16(buffer + 14);
    param->icon_lib = buffer[16];
    param->mode = buffer[17];
    param->layer_mode = buffer[18];
    param->icon_gamma = buffer[19];
    param->pic_gamma = buffer[20];
    param->time = buffer[21];
    param->anim_mode = buffer[22];
    param->filter = buffer[23];
    return GT_RES_OK;
}

static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_bin_item_st _format[] = {
    {8, gt_serial_var_head_parse, (print_cb_t)NULL,},
    {24, _parse_param, (print_cb_t)NULL,},
    { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL },
};


static bool _get_param(gt_var_st* var, gt_var_anim_icon_st* icon)
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
    if(icon->head.type != VAR_TYPE_ANIM_ICON) {return false;}

#if 0
    if(icon->head.sp != 0xFFFF){
        if(0 != gt_serial_var_buffer_get_reg(icon->head.sp, var->buffer + 6, (icon->head.len_dsc)*2)){
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

    GT_LOGI(GT_LOG_TAG_SERIAL, "type: %d, sp: 0x%X, vp: 0x%X x: %d, y: %d",\
                                icon->head.type, icon->head.sp, icon->head.vp, icon->param.x, icon->param.y);

    //

    return true;
}

static void _set_vp_value(gt_var_st* var, gt_var_value_st* value)
{
    uint8_t data[2];
    uint16_t index = 0;
    if(0 == gt_serial_var_buffer_get(value, data, 2)){
        index = ((data[0] << 8) | data[1]);
    }

    gt_var_anim_icon_st var_icon;

    if(!_get_param(var, &var_icon)){
        return ;
    }

    if(index == var_icon.param.v_stop){
        gt_player_stop(var->obj);
        return ;
    }

    if(index == var_icon.param.v_start){
        gt_player_play(var->obj);
        return ;
    }

}

static void _set_sp_value(uint8_t reg, gt_var_st* var, gt_var_value_st* value)
{
    // set sp value
    uint8_t tmp_reg = GT_VAR_GET_SP_REG(reg);
    switch(reg)
    {
    case 0:
        break;
    default:
        break;
    }
}

static void _set_value(gt_var_st* var, gt_var_value_st* value)
{
    uint8_t reg = gt_serial_var_check_value_is_set_this(var, value);
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
gt_res_t gt_serial_var_anim_icon_create(gt_var_st* var)
{
    gt_var_anim_icon_st var_icon;

    if(!_get_param(var, &var_icon)){
        return GT_RES_INV;
    }

    gt_obj_st* new_obj = gt_player_create(var->obj);
    gt_obj_register_id(new_obj, var->id);
    gt_obj_set_pos(new_obj, var_icon.param.x, var_icon.param.y);
    gt_obj_set_size(new_obj, 1, 1);
    gt_player_set_type(new_obj, GT_PLAYER_TYPE_IMG);
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

    //
    gt_player_set_mode(new_obj, var_icon.param.anim_mode ? GT_PLAYER_MODE_ONCE : GT_PLAYER_MODE_LOOP);
    gt_player_set_stop_index(new_obj, var_icon.param.icon_stop);
    gt_player_set_auto_play_period(new_obj, var_icon.param.time);

    uint16_t index = 0;
    uint8_t data[2];
    if(0 == gt_serial_var_buffer_get_reg(var_icon.head.vp, data, 2)){
        index = ((data[0] << 8) | data[1]);
    }
    if(index == var_icon.param.v_stop){
        gt_player_stop(new_obj);
    }
    else if(index == var_icon.param.v_start){
        gt_player_play(new_obj);
    }

    return GT_RES_OK;
}

gt_res_t gt_serial_var_anim_icon_set_value(gt_var_st* var, gt_var_value_st* value)
{
    _set_value(var, value);
    return GT_RES_OK;
}

/* end of file ----------------------------------------------------------*/
#endif /* GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT */

