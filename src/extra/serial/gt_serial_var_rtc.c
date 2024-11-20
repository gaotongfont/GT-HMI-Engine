/**
 * @file gt_serial_var_rtc.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-10-23 09:46:52
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_serial_var_rtc.h"
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
    uint8_t lib_id;
    uint8_t font_size;
    uint8_t string_code[16];
}gt_rtc_param_st;

typedef struct{
    gt_var_head_st head;
    gt_rtc_param_st param;
}gt_var_rtc_st;


/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_res_t _parse_param(void * user_data, uint8_t const * const buffer, uint32_t len){
    gt_var_rtc_st * icon = (gt_var_rtc_st *)user_data;
    gt_rtc_param_st* param = &icon->param;

    param->x = gt_convert_parse_u16(buffer);
    param->y = gt_convert_parse_u16(buffer + 2);
    param->color = gt_convert_parse_u16(buffer + 4);
    param->lib_id = buffer[6];
    param->font_size = buffer[7];
    memcpy(param->string_code, buffer + 8, 16);
    return GT_RES_OK;
}

static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_bin_item_st _format[] = {
    {8, gt_serial_var_head_parse, (print_cb_t)NULL,},
    {24, _parse_param, (print_cb_t)NULL,},
    { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL },
};

static bool _get_param(gt_var_st* var, gt_var_rtc_st* icon)
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
    if(icon->head.type != VAR_TYPE_RTC) {return false;}

    GT_LOGI(GT_LOG_TAG_SERIAL, "type: %d, sp: 0x%X, vp: 0x%X x: %d, y: %d",\
                                icon->head.type, icon->head.sp, icon->head.vp, icon->param.x, icon->param.y);

    //
    return true;
}


static void _get_rtc_time(gt_clock_time_st* time)
{
    // get system rtc time

#if 01 // this test code
    time->hour = 12;
    time->minute = 30;
    time->second = 30;
    time->day = 30;
    time->month = 10;
    time->year = 2024;
    time->week = 3;
#endif
}


static void _set_time(gt_obj_st* obj)
{
    gt_clock_time_st time = {0};
    _get_rtc_time(&time);
    gt_clock_set_time(obj, time.hour, time.minute, time.second);
    gt_clock_set_date(obj, time.year, time.month, time.day);
    gt_clock_set_week(obj, time.week);
    gt_clock_set_alert_time(obj, time.hour + 1, 0, 0);
}

static void _time_alert_cb(gt_obj_st * obj, void * user_data)
{
    gt_clock_stop(obj);
    _set_time(obj);
    gt_clock_start(obj);
}

static void _set_format(gt_obj_st* obj, gt_var_rtc_st* icon)
{
    uint8_t tmp_str[32] = {0}, count = 0;
    for(int i = 0; i < 16; i++){
        if(icon->param.string_code[i] == 'Y'){
            gt_memset(&tmp_str[count], 'y', 4);
            count += 4;
        }else if(icon->param.string_code[i] == 'M'){
            gt_memset(&tmp_str[count], 'M', 2);
            count += 2;
        }else if(icon->param.string_code[i] == 'D'){
            gt_memset(&tmp_str[count], 'd', 2);
            count += 2;
        }else if(icon->param.string_code[i] == 'W'){
            gt_memset(&tmp_str[count], 'E', 3);
            count += 3;
        }else if(icon->param.string_code[i] == 'H'){
            gt_memset(&tmp_str[count], 'h', 2);
            count += 2;
        }else if(icon->param.string_code[i] == 'Q'){
            gt_memset(&tmp_str[count], 'm', 2);
            count += 2;
        }else if(icon->param.string_code[i] == 'S'){
            gt_memset(&tmp_str[count], 's', 2);
            count += 2;
        }else{
            tmp_str[count] = icon->param.string_code[i];
            count++;
        }
    }
    tmp_str[count] = '\0';
    gt_clock_set_format(obj, tmp_str);
    uint16_t size = gt_font_family_get_size(icon->param.font_size);
    uint16_t h = size + 6;
    uint16_t w = size * strlen(tmp_str);
    gt_obj_set_size(obj, w, h);
}

static void _set_param(gt_var_st* var, gt_var_rtc_st* icon)
{

    gt_obj_set_pos(var->obj, icon->param.x, icon->param.y);
    gt_clock_set_font_color(var->obj, gt_color_set(icon->param.color));
    // gt_clock_set_font_size(var->obj, icon->param.font_size);
    gt_clock_set_font_family(var->obj, icon->param.lib_id);
    _set_format(var->obj, icon);
    //
    _set_time(var->obj);

    gt_clock_start(var->obj);
}

static void _set_vp_value(gt_var_st* var, gt_var_value_st* value)
{
    _set_time(var->obj);
}

static void _set_sp_value(uint8_t reg, gt_var_st* var, gt_var_value_st* value)
{
    // set sp value
    uint8_t tmp_reg = GT_VAR_GET_SP_REG(reg);
    gt_var_rtc_st var_icon;

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
gt_res_t gt_serial_var_rtc_create(gt_var_st* var)
{
    gt_var_rtc_st var_icon;

    if(!_get_param(var, &var_icon)){
        return GT_RES_INV;
    }

    gt_obj_st* new_obj = gt_clock_create(var->obj);
    gt_obj_register_id(new_obj, var->id);
    gt_clock_set_font_align(new_obj, GT_ALIGN_LEFT);
    gt_clock_set_mode(new_obj, GT_CLOCK_MODE_TIME);
    gt_clock_set_alert_cb(new_obj, _time_alert_cb, NULL);

    var->obj = new_obj;
    _set_param(var, &var_icon);

    return GT_RES_OK;
}
gt_res_t gt_serial_var_rtc_set_value(gt_var_st* var, gt_var_value_st* value)
{
    _set_value(var, value);
    return GT_RES_OK;
}


/* end of file ----------------------------------------------------------*/
#endif /* GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT */

