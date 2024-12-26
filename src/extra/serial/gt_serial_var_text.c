/**
 * @file gt_serial_var_text.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-10-21 14:07:44
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_serial_var_text.h"
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
    uint16_t xs;
    uint16_t ys;
    uint16_t xe;
    uint16_t ye;
    uint16_t len;
    uint8_t font0_id;
    uint8_t font1_id;
    uint8_t font_x_dots;
    uint8_t font_y_dots;
    uint8_t encode_mode;
    uint8_t hor_dis;
    uint8_t ver_dis;
}gt_text_param_st;

typedef struct{
    gt_var_head_st head;
    gt_text_param_st param;
}gt_var_text_st;



/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_res_t _parse_param(void * user_data, uint8_t const * const buffer, uint32_t len){
    gt_var_text_st * icon = (gt_var_text_st *)user_data;
    gt_text_param_st* param = &icon->param;

    param->x = gt_convert_parse_u16(buffer);
    param->y = gt_convert_parse_u16(buffer + 2);
    param->color = gt_convert_parse_u16(buffer + 4);
    param->xs = gt_convert_parse_u16(buffer + 6);
    param->ys = gt_convert_parse_u16(buffer + 8);
    param->xe = gt_convert_parse_u16(buffer + 10);
    param->ye = gt_convert_parse_u16(buffer + 12);
    param->len = gt_convert_parse_u16(buffer + 14);
    param->font0_id = buffer[16];
    param->font1_id = buffer[17];
    param->font_x_dots = buffer[18];
    param->font_y_dots = buffer[19];
    param->encode_mode = buffer[20];
    param->hor_dis = buffer[21];
    param->ver_dis = buffer[22];
    return GT_RES_OK;
}


static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_bin_item_st _format[] = {
    {8, gt_serial_var_head_parse, (print_cb_t)NULL,},
    {23, _parse_param, (print_cb_t)NULL,},
    { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL },
};

static bool _get_param(gt_var_st* var, gt_var_text_st* icon)
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
    if(icon->head.type != VAR_TYPE_TEXT) {return false;}

    GT_LOGI(GT_LOG_TAG_SERIAL, "type: %d, sp: 0x%X, vp: 0x%X x: %d, y: %d",\
                                icon->head.type, icon->head.sp, icon->head.vp, icon->param.x, icon->param.y);

    //
    return true;
}

static void _set_text(gt_obj_st* obj, gt_var_text_st* icon, uint16_t reg)
{
    uint8_t hor_align = 0, ver_align = 0, encode = 0;
    hor_align = (icon->param.encode_mode >> 5)&0x03;
    ver_align = (icon->param.encode_mode >> 4)&0x01;
    encode = icon->param.encode_mode & 0x0F;

    switch(hor_align){
        case 0:
            gt_label_set_font_align(obj, (ver_align) ? GT_ALIGN_LEFT : GT_ALIGN_LEFT_MID);
            break;
        case 1:
            gt_label_set_font_align(obj, (ver_align) ? GT_ALIGN_RIGHT : GT_ALIGN_RIGHT_MID);
            break;
        case 2:
            gt_label_set_font_align(obj, (ver_align) ? GT_ALIGN_CENTER : GT_ALIGN_CENTER_MID);
            break;
        case 3:
            gt_label_set_font_align(obj, (ver_align) ? GT_ALIGN_LEFT | GT_ALIGN_REVERSE : GT_ALIGN_LEFT_MID | GT_ALIGN_REVERSE);
            break;
        default:
            break;
    }

    const uint8_t * data = NULL;
    uint16_t len = gt_serial_var_buffer_get_pointer(reg, &data, icon->param.len);
    if(0 == len) {return;}

    switch(ver_align){
        case 0:
            // ascii
            gt_label_set_font_encoding(obj, GT_ENCODING_UTF8);
            gt_label_set_text_by_len(obj, data, len);
            break;
        case 1:
        case 2:
            // gbk
            gt_label_set_font_encoding(obj, GT_ENCODING_GB);
            gt_label_set_text_by_len(obj, data, len);
            break;
        case 3:
            // big5
            gt_label_set_font_encoding(obj, GT_ENCODING_GB);
            uint8_t *data_gbk = gt_mem_malloc(len);
            if(!data_gbk){
                GT_LOGE(GT_LOG_TAG_SERIAL, "big5 to gbk malloc err! len: %d\r\n", len);
                return;
            }
            len = gt_encoding_convert(data, len, data_gbk, len, BIG5_2_GBK);
            if(len != 0){
                gt_label_set_text_by_len(obj, data_gbk, len);
            }
            gt_mem_free(data_gbk);
            break;
        case 4:
            // sjis
            gt_label_set_font_encoding(obj, GT_ENCODING_SJIS);
            gt_label_set_text_by_len(obj, data, len);
            break;
        case 5:
            // unicode
            gt_label_set_font_encoding(obj, GT_ENCODING_UTF8);
            uint8_t *data_utf8 = gt_mem_malloc(len + (len >> 1));
            if(!data_utf8){
                GT_LOGE(GT_LOG_TAG_SERIAL, "unicode to utf8 malloc err! len: %d\r\n", len + (len >> 1));
                return;
            }
            len = gt_encoding_convert(data, len, data_utf8, len + (len >> 1), UNICODE_2_UTF8);
            if(len != 0){
                gt_label_set_text_by_len(obj, data_utf8, len);
            }
            gt_mem_free(data_utf8);
            break;
        default:
            break;
    }
}

static void _set_param(gt_var_st* var, gt_var_text_st* icon)
{
    uint16_t w = 0, h = 0;
    gt_obj_set_pos(var->obj, icon->param.x, icon->param.y);
    w = icon->param.xe - icon->param.xs + 1;
    h = icon->param.ye - icon->param.ys + 1;
    gt_obj_set_size(var->obj, w, h);
    gt_label_set_font_color(var->obj, gt_color_set(icon->param.color));
    gt_label_set_font_family(var->obj, icon->param.font0_id);
    gt_label_set_space(var->obj, icon->param.hor_dis, icon->param.ver_dis);

    _set_text(var->obj, icon, icon->head.vp);
}

static void _set_vp_value(gt_var_st* var, gt_var_value_st* value)
{
    gt_var_text_st var_icon;

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
    gt_var_text_st var_icon;

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
gt_res_t gt_serial_var_text_create(gt_var_st* var)
{
    gt_var_text_st var_icon;

    if(!_get_param(var, &var_icon)){
        return GT_RES_INV;
    }

    gt_obj_st* new_obj = gt_label_create(var->obj);
    gt_obj_register_id(new_obj, var->id);

    var->obj = new_obj;
    _set_param(var, &var_icon);

    return GT_RES_OK;
}

gt_res_t gt_serial_var_text_set_value(gt_var_st* var, gt_var_value_st* value)
{
    _set_value(var, value);
    return GT_RES_OK;
}
/* end of file ----------------------------------------------------------*/
#endif /* GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT */

