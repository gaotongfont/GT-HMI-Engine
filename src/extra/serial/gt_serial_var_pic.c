/**
 * @file gt_serial_var_pic.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-10-11 15:23:43
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
    uint16_t pic_begin;
    uint16_t pic_end;
    uint8_t time;   // unit: 8ms
    uint8_t lib;
    uint16_t pic_end_exp;
}gt_pic_param_st;

typedef struct{
    gt_var_head_st head;
    gt_pic_param_st param;
}gt_var_icon_st;


/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_res_t _parse_param(void * user_data, uint8_t const * const buffer, uint32_t len){
    gt_var_icon_st * icon = (gt_var_icon_st *)user_data;
    gt_pic_param_st* param = &icon->param;

    param->pic_begin = gt_convert_parse_u16(buffer);
    param->pic_end = gt_convert_parse_u16(buffer + 2);
    param->time = buffer[4];
    param->lib = buffer[5];
    param->pic_end_exp = gt_convert_parse_u16(buffer + 6);
    return GT_RES_OK;
}


static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_bin_item_st _format[] = {
    {8, gt_serial_var_head_parse, (print_cb_t)NULL,},
    {8, _parse_param, (print_cb_t)NULL,},
    { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL },
};

static bool _get_param(gt_var_st* var, gt_var_icon_st* icon)
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
    if(icon->head.type != VAR_TYPE_PIC) {return false;}

    GT_LOGI(GT_LOG_TAG_SERIAL, "type: %d, sp: 0x%X, vp: 0x%X pic_end_exp: %d",\
                                icon->head.type, icon->head.sp, icon->head.vp, icon->param.pic_end_exp);

    //
    return true;
}

static void _set_param(gt_var_st* var, gt_var_icon_st* icon)
{
    // add imge
    if(0 != gt_player_get_item_count(var->obj)){
        gt_player_remove_all_items(var->obj);
    }

#if GT_USE_FILE_HEADER
    gt_file_header_param_st fh;
    gt_file_header_param_init(&fh);
    for(uint16_t i = icon->param.pic_begin; i <= icon->param.pic_end; ++i){
        fh.idx = i;
        gt_player_add_item_by_file_header(var->obj, &fh);
    }
#endif

    gt_player_set_auto_play_period(var->obj, icon->param.time * 8);
    gt_player_play(var->obj);
}

static void _set_sp_value(uint8_t reg, gt_var_st* var, gt_var_value_st* value)
{
    // set sp value
    uint8_t tmp_reg = GT_VAR_GET_SP_REG(reg);
    gt_var_icon_st var_icon;

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
        // _set_vp_value(var, value);
    }
    else{
        _set_sp_value(reg, var, value);
    }
}

/* global functions / API interface -------------------------------------*/
gt_res_t gt_serial_var_pic_create(gt_var_st* var)
{
    gt_var_icon_st var_icon;

    if(!_get_param(var, &var_icon)){
        return GT_RES_INV;
    }

    gt_obj_st* new_obj = gt_player_create(var->obj);
    gt_obj_register_id(new_obj, var->id);
    gt_obj_set_size(new_obj, 1, 1);
    gt_player_set_type(new_obj, GT_PLAYER_TYPE_IMG);
    gt_player_set_mode(new_obj, GT_PLAYER_MODE_ONCE);
    gt_obj_set_pos(new_obj, 0, 0);
    gt_player_set_index(var->obj, 0);

    var->obj = new_obj;
    _set_param(var, &var_icon);

    return GT_RES_OK;
}

gt_res_t gt_serial_var_pic_set_value(gt_var_st* var, gt_var_value_st* value)
{
    _set_value(var, value);
    return GT_RES_OK;
}

/* end of file ----------------------------------------------------------*/
#endif /* GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT */


