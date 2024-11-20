/**
 * @file gt_serial_var_keypad.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-11-12 16:29:45
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_serial_var_keypad.h"
#if GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT
#include "../../utils/gt_bin_convert.h"
#include "../../others/gt_log.h"
#include "../../../gt.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/
/* regs
bit 0: 1-enable py 0-disable
*/
typedef struct{
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
    uint16_t font_color;
    uint16_t key_color;
    uint16_t ctrl_color;
    uint16_t bg_color;
    uint8_t radio;
    uint8_t font_id;
    uint8_t style;
    uint8_t regs;
}gt_var_keypad_param_st;

typedef struct{
    gt_var_head_st head;
    gt_var_keypad_param_st param;
}gt_var_keypad_st;


/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_res_t _parse_param(void * user_data, uint8_t const * const buffer, uint32_t len){
    gt_var_keypad_st * icon = (gt_var_keypad_st *)user_data;
    gt_var_keypad_param_st* param = &icon->param;
    param->x = gt_convert_parse_u16(buffer);
    param->y = gt_convert_parse_u16(buffer + 2);
    param->w = gt_convert_parse_u16(buffer + 4);
    param->h = gt_convert_parse_u16(buffer + 6);
    param->font_color = gt_convert_parse_u16(buffer + 8);
    param->key_color = gt_convert_parse_u16(buffer + 10);
    param->ctrl_color = gt_convert_parse_u16(buffer + 12);
    param->bg_color = gt_convert_parse_u16(buffer + 14);
    param->radio = buffer[16];
    param->font_id = buffer[17];
    param->style = buffer[18];
    param->regs = buffer[19];

    return GT_RES_OK;
}

static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_bin_item_st _format[] = {
    {8, gt_serial_var_head_parse, (print_cb_t)NULL,},
    {20, _parse_param, (print_cb_t)NULL,},
    { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL },
};

static bool _get_param(gt_var_st* var, gt_var_keypad_st* icon)
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
    if(icon->head.type != VAR_TYPE_KEYPAD) {return false;}

    GT_LOGI(GT_LOG_TAG_SERIAL, "type: %d, sp: 0x%X, vp: 0x%X x: %d, y: %d",\
                                icon->head.type, icon->head.sp, icon->head.vp, icon->param.x, icon->param.y);

    //
    return true;
}

static void _set_param(gt_var_st* var, gt_var_keypad_st* icon)
{
    gt_obj_set_pos(var->obj, icon->param.x, icon->param.y);
    gt_obj_set_size(var->obj, icon->param.w, icon->param.h);
    gt_keypad_set_font_family(var->obj, icon->param.font_id);
    gt_keypad_set_font_color(var->obj, gt_color_set(icon->param.font_color));
    gt_keypad_set_color_background(var->obj, gt_color_set(icon->param.bg_color));
    gt_keypad_set_key_color_background(var->obj, gt_color_set(icon->param.key_color));
    gt_keypad_set_ctrl_key_color_background(var->obj, gt_color_set(icon->param.ctrl_color));
    gt_keypad_set_radius(var->obj, icon->param.radio);

    bool py_en = icon->param.regs & 0x01;
    bool auto_hide = (icon->param.regs >> 1) & 0x01;
    gt_keypad_set_auto_hide(var->obj, auto_hide);

    gt_py_input_method_st* tmp_py = gt_keypad_get_py_input_method(var->obj);
    if(py_en) {
        if(!tmp_py) {
            tmp_py = gt_py_input_method_create();
        }
        gt_keypad_set_py_input_method(var->obj, tmp_py);
    }
    else {
        if(tmp_py) {
            gt_py_input_method_destroy(tmp_py);
        }
        gt_keypad_set_py_input_method(var->obj, NULL);
    }
}

static void _set_sp_value(uint8_t reg, gt_var_st* var, gt_var_value_st* value)
{
    // set sp value
    uint8_t tmp_reg = GT_VAR_GET_SP_REG(reg);
    gt_var_keypad_st var_icon;

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
gt_res_t gt_serial_var_keypad_create(gt_var_st* var)
{
    gt_var_keypad_st var_icon;

    if(!_get_param(var, &var_icon)){
        return GT_RES_INV;
    }

    gt_obj_st* new_obj = gt_keypad_create(var->obj);
    gt_obj_register_id(new_obj, var->id);

    var->obj = new_obj;
    _set_param(var, &var_icon);

    return GT_RES_OK;
}

gt_res_t gt_serial_var_keypad_set_value(gt_var_st* var, gt_var_value_st* value)
{
    _set_value(var, value);
    return GT_RES_OK;
}
/* end of file ----------------------------------------------------------*/
#endif /* GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT */

