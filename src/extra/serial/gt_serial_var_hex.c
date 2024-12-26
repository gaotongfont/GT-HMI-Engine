/**
 * @file gt_serial_var_hex.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-10-23 16:42:13
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_serial_var_hex.h"
#if GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT
#include "../../utils/gt_bin_convert.h"
#include "../../others/gt_log.h"
#include "../../../gt.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/
typedef struct{
    uint16_t x;
    uint16_t y;
    uint16_t color;
    uint8_t mode;
    uint8_t lib_id;
    uint8_t font_size;
    uint8_t string_code[15];
}gt_hex_param_st;

typedef struct{
    gt_var_head_st head;
    gt_hex_param_st param;
}gt_var_hex_st;


/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_res_t _parse_param(void * user_data, uint8_t const * const buffer, uint32_t len){
    gt_var_hex_st * icon = (gt_var_hex_st *)user_data;
    gt_hex_param_st* param = &icon->param;

    param->x = gt_convert_parse_u16(buffer);
    param->y = gt_convert_parse_u16(buffer + 2);
    param->color = gt_convert_parse_u16(buffer + 4);
    param->mode = buffer[6];
    param->lib_id = buffer[7];
    param->font_size = buffer[8];
    memcpy(param->string_code, buffer + 9, 15);
    return GT_RES_OK;
}

static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_bin_item_st _format[] = {
    {8, gt_serial_var_head_parse, (print_cb_t)NULL,},
    {24, _parse_param, (print_cb_t)NULL,},
    { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL },
};

static bool _get_param(gt_var_st* var, gt_var_hex_st* icon)
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
    if(icon->head.type != VAR_TYPE_HEX) {return false;}

    GT_LOGI(GT_LOG_TAG_SERIAL, "type: %d, sp: 0x%X, vp: 0x%X x: %d, y: %d",\
                                icon->head.type, icon->head.sp, icon->head.vp, icon->param.x, icon->param.y);

    //
    return true;
}

static void _set_hex(gt_obj_st* obj, gt_var_hex_st* icon, uint16_t reg)
{
    uint8_t bcd = (icon->param.mode >> 7) & 0x01;
    uint8_t len = (icon->param.mode) & 0x0F;
    uint8_t tmp_str[48] = {0}, count = 0;
    uint8_t data[16] = {0}, idx = 0;
    uint8_t *p = icon->param.string_code;
    uint8_t max_len = 0, tmp_len = 0, line = 1;

    if(0 != gt_serial_var_buffer_get_reg(reg, data, len)){
        return ;
    }

    if(bcd){
        sprintf(&tmp_str[count], "%02X", data[idx]);
    }else{
        sprintf(&tmp_str[count], "%02d", data[idx]);
    }
    idx += 1;
    count += 2;
    tmp_len += 2;

    for(; idx < len; ++idx){
        if((*p) == 0x0D || (*p) == 0x0A){
            max_len = GT_MAX(tmp_len, max_len);
            tmp_len = 0;
            ++line;
            *p = 0x0A;
        }

        if(bcd){
            if((*p) == 0x00){
                sprintf(&tmp_str[count], "%02X", data[idx]);
                count += 2;
                tmp_len += 2;
            }else{
                sprintf(&tmp_str[count], "%c%02X", *p, data[idx]);
                count += 3;
                tmp_len += 3;
            }
            ++p;
        }else{
            if((*p) == 0x00){
                sprintf(&tmp_str[count], "%02d", data[idx]);
                count += 2;
                tmp_len += 2;
            }else{
                sprintf(&tmp_str[count], "%c%02d", *p, data[idx]);
                count += 3;
                tmp_len += 3;
            }
            ++p;
        }
    }
    tmp_str[count] = '\0';
    max_len = GT_MAX(tmp_len, max_len) + 4;
    uint8_t size = gt_font_family_get_size(icon->param.lib_id);
    gt_obj_set_size(obj, max_len * (size >> 1), (size + 6) * line);
    gt_label_set_text(obj, tmp_str);
}

static void _set_param(gt_var_st* var, gt_var_hex_st* icon)
{
    gt_obj_set_pos(var->obj, icon->param.x, icon->param.y);
    gt_label_set_font_color(var->obj, gt_color_set(icon->param.color));
    // gt_label_set_font_size(var->obj, icon->param.font_size);
    gt_label_set_font_family(var->obj, icon->param.lib_id);
    _set_hex(var->obj, icon, icon->head.vp);
}

static void _set_vp_value(gt_var_st* var, gt_var_value_st* value)
{
    gt_var_hex_st var_icon;

    if(!_get_param(var, &var_icon)){
        return ;
    }

    uint16_t reg = gt_convert_parse_u16(value->buffer);
    _set_hex(var->obj, &var_icon, reg);
}

static void _set_sp_value(uint8_t reg, gt_var_st* var, gt_var_value_st* value)
{
    // set sp value
    uint8_t tmp_reg = GT_VAR_GET_SP_REG(reg);
    gt_var_hex_st var_icon;

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
gt_res_t gt_serial_var_hex_create(gt_var_st* var)
{
    gt_var_hex_st var_icon;

    if(!_get_param(var, &var_icon)){
        return GT_RES_INV;
    }

    gt_obj_st* new_obj = gt_label_create(var->obj);
    gt_obj_register_id(new_obj, var->id);

    var->obj = new_obj;
    _set_param(var, &var_icon);

    return GT_RES_OK;
}

gt_res_t gt_serial_var_hex_set_value(gt_var_st* var, gt_var_value_st* value)
{
    _set_value(var, value);
    return GT_RES_OK;
}
/* end of file ----------------------------------------------------------*/
#endif /* GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT */

