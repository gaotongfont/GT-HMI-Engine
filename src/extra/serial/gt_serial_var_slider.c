/**
 * @file gt_serial_var_slider.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-09-30 10:40:12
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_serial_var_slider.h"
#if GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT
#include "../../utils/gt_bin_convert.h"
#include "../../others/gt_log.h"
#include "../../../gt.h"

#include "gt_serial_show.h"
/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/
typedef struct{
    uint16_t v_begin;
    uint16_t v_end;
    uint16_t x_begin;
    uint16_t x_end;
    uint16_t icon_id;
    uint16_t y;
    uint8_t x_adj;
    uint8_t mode;
    uint8_t icon_lib;
    uint8_t icon_mode;
    uint8_t vp_data_mode;
    uint8_t layer_mode;
    uint8_t icon_gamma;
    uint8_t pic_gamma;
    uint8_t filter;
    uint16_t h;
}gt_var_slider_param_st;

typedef struct{
    gt_var_head_st head;
    gt_var_slider_param_st param;
}gt_var_slider_st;


/* static prototypes ----------------------------------------------------*/
static bool _get_param(gt_var_st* var, gt_var_slider_st* icon);



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* event cb ---------------------------------------------------------------*/
static void _slider_event_cb(gt_event_st* e)
{
    const uint8_t* buffer = gt_serial_get_info_by_obj(e->target);
    if(NULL == buffer){
        return;
    }

    gt_var_slider_st var_icon;
    gt_var_st var;
    var.buffer = (uint8_t*)buffer;
    var.len = GT_SERIAL_UI_PAGE_CONTROL_SIZE;

    if(!_get_param(&var, &var_icon)){
        return ;
    }

    uint8_t data[4] = {0};
    data[0] = (var_icon.head.vp >> 8) & 0xff;
    data[1] = var_icon.head.vp & 0xff;

    gt_size_t pos = gt_slider_get_pos(e->target);

    if(0x01 == var_icon.param.vp_data_mode){
        if(0 != gt_serial_var_buffer_get_reg(var_icon.head.vp, &data[2], 2)){
            return;
        }
        data[2] = pos & 0xff;
    }
    else if(0x02 == var_icon.param.vp_data_mode){
        if(0 != gt_serial_var_buffer_get_reg(var_icon.head.vp, &data[2], 2)){
            return;
        }
        data[3]  = pos & 0xff;
    }
    else{
        data[2] = (pos >> 8) & 0xff;
        data[3] = pos & 0xff;
    }
    gt_serial_set_value(data, 4);
}


/* static functions -----------------------------------------------------*/
static gt_res_t _parse_param(void * user_data, uint8_t const * const buffer, uint32_t len){
    gt_var_slider_st * icon = (gt_var_slider_st *)user_data;
    gt_var_slider_param_st* param = &icon->param;

    param->v_begin = gt_convert_parse_u16(buffer);
    param->v_end = gt_convert_parse_u16(buffer + 2);
    param->x_begin = gt_convert_parse_u16(buffer + 4);
    param->x_end = gt_convert_parse_u16(buffer + 6);
    param->icon_id = gt_convert_parse_u16(buffer + 8);
    param->y = gt_convert_parse_u16(buffer + 10);
    param->x_adj = buffer[12];
    param->mode = buffer[13];
    param->icon_lib = buffer[14];
    param->icon_mode = buffer[15];
    param->vp_data_mode = buffer[16];
    param->layer_mode = buffer[17];
    param->icon_gamma = buffer[18];
    param->pic_gamma = buffer[19];
    param->filter = buffer[20];
    param->h = gt_convert_parse_u16(buffer + 21);

    return GT_RES_OK;
}
static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_bin_item_st _format[] = {
    {8, gt_serial_var_head_parse, (print_cb_t)NULL,},
    {23, _parse_param, (print_cb_t)NULL,},
    { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL },
};

static bool _get_param(gt_var_st* var, gt_var_slider_st* icon)
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
    if(icon->head.type != VAR_TYPE_SLIDER) {return false;}

    GT_LOGI(GT_LOG_TAG_SERIAL, "type: %d, sp: 0x%X, vp: 0x%X x: %d, y: %d",\
                                icon->head.type, icon->head.sp, icon->head.vp, icon->param.x_begin, icon->param.y);

    //
    return true;
}

static void _set_pos_value(gt_obj_st* obj, uint8_t vp_mode, uint16_t reg)
{
    uint8_t data[2];
    uint16_t pos = 0;
    if(0 != gt_serial_var_buffer_get_reg(reg, data, 2)){
        return;
    }
    if(0x01 == vp_mode){
        pos = data[0];
    }else if(0x02 == vp_mode){
        pos = data[1];
    }else{
        pos = ((data[0] << 8) | data[1]);
    }

    gt_slider_set_pos(obj, pos);
}

static void _set_param(gt_var_st* var, gt_var_slider_st* icon)
{
    uint16_t w, h;
    gt_obj_set_pos(var->obj, icon->param.x_begin, icon->param.y);
    if(0 == icon->param.mode){
        gt_slider_set_dir(var->obj, GT_BAR_DIR_HOR_L2R);
        w = icon->param.x_end - icon->param.x_begin + 1;
        h = icon->param.h;
    }else
    {
        gt_slider_set_dir(var->obj, GT_BAR_DIR_VER_D2U);
        w = icon->param.h;
        h = icon->param.x_end - icon->param.x_begin + 1;
    }
    gt_obj_set_size(var->obj, w, h);
    gt_slider_set_start_end(var->obj, icon->param.v_begin, icon->param.v_end);
    _set_pos_value(var->obj, icon->param.vp_data_mode, icon->head.vp);

    // set img
    gt_file_header_param_st fh;
    gt_file_header_param_init(&fh);
    fh.idx = icon->param.icon_id;
    gt_slider_set_tag_by_file_header(var->obj, &fh);

}

static void _set_vp_value(gt_var_st* var, gt_var_value_st* value)
{
    gt_var_slider_st var_icon;

    if(!_get_param(var, &var_icon)){
        return ;
    }
    uint16_t reg = gt_convert_parse_u16(value->buffer);
    _set_pos_value(var->obj, var_icon.param.vp_data_mode, reg);
}

static void _set_sp_value(uint8_t reg, gt_var_st* var, gt_var_value_st* value)
{
    // set sp value
    uint8_t tmp_reg = GT_VAR_GET_SP_REG(reg);
    gt_var_slider_st var_icon;

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
gt_res_t gt_serial_var_slider_create(gt_var_st* var)
{
    gt_var_slider_st var_icon;

    if(!_get_param(var, &var_icon)){
        return GT_RES_INV;
    }

    gt_obj_st* new_obj = gt_slider_create(var->obj);
    gt_obj_register_id(new_obj, var->id);
    gt_slider_set_mode(new_obj, GT_SLIDER_MODE_DEFAULT);
    gt_slider_set_tag_visible(new_obj, true);

    gt_obj_add_event_cb(new_obj, _slider_event_cb, GT_EVENT_TYPE_INPUT_SCROLL, NULL);

    var->obj = new_obj;
    _set_param(var, &var_icon);

    return GT_RES_OK;
}

gt_res_t gt_serial_var_slider_set_value(gt_var_st* var, gt_var_value_st* value)
{
    _set_value(var, value);
    return GT_RES_OK;
}


/* end of file ----------------------------------------------------------*/
#endif /* GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT */


