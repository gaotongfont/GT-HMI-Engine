/**
 * @file gt_serial_var_icon.c
 * @author Yang
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
    if(icon->head.type != VAR_TYPE_ICON) {return false;}

    GT_LOGI(GT_LOG_TAG_SERIAL, "type: %d, sp: 0x%X, vp: 0x%X x: %d, y: %d, ivon_min: %d",\
                                icon->head.type, icon->head.sp, icon->head.vp, icon->param.x, icon->param.y, icon->param.icon_min);

    //
    return true;
}

static void _set_index(gt_obj_st* obj, uint16_t reg)
{
    uint8_t data[2];
    if(0 != gt_serial_var_buffer_get_reg(reg, data, 2)){
        return;
    }

    uint16_t index = gt_convert_parse_u16(data);
    gt_player_set_index(obj, index);
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
    for(uint16_t i = icon->param.icon_min; i <= icon->param.icon_max; ++i){
        fh.idx = i;
        gt_player_add_item_by_file_header(var->obj, &fh);
    }
    uint16_t w = 0, h = 0;
    fh.idx = icon->param.icon_min;
    gt_fs_fh_read_img_wh(&fh, &w, &h);
    gt_obj_set_size(var->obj, w, h);
#endif

    gt_obj_set_pos(var->obj, icon->param.x, icon->param.y);
    gt_player_set_index_offset(var->obj, icon->param.v_min);

    _set_index(var->obj, icon->head.vp);
}

static void _set_vp_value(gt_var_st* var, gt_var_value_st* value)
{
    uint16_t reg = gt_convert_parse_u16(value->buffer);
    _set_index(var->obj, reg);
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
        _set_vp_value(var, value);
    }
    else{
        _set_sp_value(reg, var, value);
    }
}
/* global functions / API interface -------------------------------------*/
gt_res_t gt_serial_var_icon_create(gt_var_st* var)
{
    gt_var_icon_st var_icon;

    if(!_get_param(var, &var_icon)){
        return GT_RES_INV;
    }

    gt_obj_st* new_obj = gt_player_create(var->obj);
    gt_obj_register_id(new_obj, var->id);
    gt_player_set_type(new_obj, GT_PLAYER_TYPE_IMG);
    gt_player_set_mode(new_obj, GT_PLAYER_MODE_NONE);

    var->obj = new_obj;
    _set_param(var, &var_icon);

    return GT_RES_OK;
}

gt_res_t gt_serial_var_icon_set_value(gt_var_st* var, gt_var_value_st* value)
{
    _set_value(var, value);
    return GT_RES_OK;
}


/* end of file ----------------------------------------------------------*/
#endif /* GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT */

