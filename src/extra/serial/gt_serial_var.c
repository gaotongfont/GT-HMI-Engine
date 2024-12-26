/**
 * @file gt_serial_var.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-09-23 17:12:41
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_serial_var.h"

#if GT_USE_SERIAL_VAR && GT_USE_BIN_CONVERT
#include "../../utils/gt_bin_convert.h"
#include "../../others/gt_log.h"
#include "../../../gt.h"
#include "gt_serial_resource.h"

//
#include "gt_serial_var_icon.h"
#include "gt_serial_var_anim_icon.h"
#include "gt_serial_var_slider.h"
#include "gt_serial_var_artistic.h"
#include "gt_serial_var_pic.h"
#include "gt_serial_var_bit.h"
#include "gt_serial_qrcode.h"
#include "gt_serial_area_light.h"
#include "gt_serial_value_passing.h"
#include "gt_serial_var_numb.h"
#include "gt_serial_var_text.h"
#include "gt_serial_var_rtc.h"
#include "gt_serial_var_hex.h"
#include "gt_serial_var_roller.h"
#include "gt_serial_var_progress_bar.h"
#include "gt_serial_var_input.h"
#include "gt_serial_var_keypad.h"
/* private define -------------------------------------------------------*/
#define _GT_SERIAL_FILE_22                  (1)
#define _GT_SERIAL_VAR_ADDR_OFFSET          (0x1000)

#ifndef GT_SERIAL_UI_VAR_SIZE
    /* serial ui variable size */
    #define GT_SERIAL_UI_VAR_SIZE           (128 * 1024)
#endif

#define _REFACTOR_PARSE     0

/* private typedef ------------------------------------------------------*/



/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/
/**
 * @brief serial ui variable
 */
static GT_ATTRIBUTE_LARGE_RAM_ARRAY uint8_t _serial_var_buf[GT_SERIAL_UI_VAR_SIZE] = {0};

static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_var_handler_item_st _var_handler_list[] = {
    {VAR_TYPE_ICON, gt_serial_var_icon_create, gt_serial_var_icon_set_value},
    {VAR_TYPE_ANIM_ICON, gt_serial_var_anim_icon_create, gt_serial_var_anim_icon_set_value},
    {VAR_TYPE_SLIDER, gt_serial_var_slider_create, gt_serial_var_slider_set_value},
    {VAR_TYPE_ARTISTIC, gt_serial_var_artistic_create, gt_serial_var_artistic_set_value},
    {VAR_TYPE_PIC, gt_serial_var_pic_create, gt_serial_var_pic_set_value},
    {VAR_TYPE_BIT, gt_serial_var_bit_create, gt_serial_var_bit_set_value},
    {VAR_TYPE_NUMB, gt_serial_var_numb_create, gt_serial_var_numb_set_value},
    {VAR_TYPE_TEXT, gt_serial_var_text_create, gt_serial_var_text_set_value},
    {VAR_TYPE_RTC, gt_serial_var_rtc_create, gt_serial_var_rtc_set_value},
    {VAR_TYPE_HEX, gt_serial_var_hex_create, gt_serial_var_hex_set_value},
    {VAR_TYPE_ROLLER, gt_serial_var_roller_create, gt_serial_var_roller_set_value},

    {VAR_TYPE_PROGRESS_BAR, gt_serial_var_progress_bar_create, gt_serial_var_progress_bar_set_value},

    {VAR_TYPE_QR_CODE, gt_serial_qrcode_create, gt_serial_qrcode_set_value},
    {VAR_TYPE_AREA_LIGHT, gt_serial_area_light_create, gt_serial_area_light_set_value},
    {VAR_TYPE_VALUE_PASSING, gt_serial_value_passing_create, gt_serial_value_passing_set_value},

    {VAR_TYPE_INPUT, gt_serial_var_input_create, gt_serial_var_input_set_value},
    {VAR_TYPE_KEYPAD, gt_serial_var_keypad_create, gt_serial_var_keypad_set_value},
};

/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_bin_item_st _format[] = {
    { 8, gt_serial_var_head_parse, (print_cb_t)NULL },
    { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL },
};

static gt_res_t _get_var_head(gt_var_st *var, gt_var_head_st *var_head)
{
    gt_bin_convert_st bin = {
        .tar = var_head,
        .buffer = var->buffer,
        .byte_len = var->len,
    };
    gt_bin_res_st ret = {0};
    ret = gt_bin_convert_parse(&bin, _format);

    if (GT_RES_OK != ret.res) {
        GT_LOGE(GT_LOG_TAG_SERIAL, "var head parse err code: %d, index: %d", ret.res, ret.index);
    }
    return ret.res;
}

#if _REFACTOR_PARSE
static gt_res_t _parse_reg(void * user_data, uint8_t const * const buffer, uint32_t len){
    uint16_t* reg = (uint16_t*)user_data;
    *reg = buffer[0] << 8 | buffer[1];
    return GT_RES_OK;
}

static GT_ATTRIBUTE_LARGE_RAM_ARRAY const gt_bin_item_st _format_value[] = {
    { GT_VAR_REG_LEN, _parse_reg, (print_cb_t)NULL },
    { 0x0, (parse_cb_t)NULL, (print_cb_t)NULL },
};
#endif


static gt_res_t _var_create(gt_var_handler_st *handler)
{
    if(!handler) {return GT_RES_INV;}
    for(uint16_t i = 0; i < handler->count; i++) {
        if(handler->type == handler->list[i].type) {
            if(handler->list[i].create_cb){
                return handler->list[i].create_cb(handler->var);
            }
            break;
        }
    }
    return GT_RES_INV;  /** No valid related type control was found */
}

static gt_res_t _var_set_value(gt_var_handler_st *handler)
{
    if(!handler) {return GT_RES_INV;}
    for(uint16_t i = 0; i < handler->count; i++) {
        if(handler->type == handler->list[i].type) {
            if(handler->list[i].set_value_cb){
                return handler->list[i].set_value_cb(handler->var, handler->value);
            }
            break;
        }
    }
    return GT_RES_OK;
}

/* global functions / API interface -------------------------------------*/
void gt_serial_var_init(bool load)
{
    // memset(_serial_var_buf, 0, GT_SERIAL_UI_VAR_SIZE);

    if(!load) { return; }
    uint32_t addr = gt_serial_resource_get_addr_by(_GT_SERIAL_FILE_22) + (2 * _GT_SERIAL_VAR_ADDR_OFFSET);

    gt_fs_read_direct_physical(addr, \
                                GT_SERIAL_UI_VAR_SIZE - (2 * _GT_SERIAL_VAR_ADDR_OFFSET), \
                                &_serial_var_buf[(2 * _GT_SERIAL_VAR_ADDR_OFFSET)]);
}

uint16_t gt_serial_var_buffer_get_pointer(uint16_t reg, const uint8_t** res_ptr, uint16_t len)
{
    if(!res_ptr){return 0;}
    if(reg + len > 0xFFFF) {return 0;}
    *res_ptr = (const char*)&_serial_var_buf[reg * 2];
    return len;
}

int gt_serial_var_buffer_get_reg(uint16_t reg, uint8_t* data, uint16_t len)
{
    if(!data){return -1;}
    if(reg + len > 0xFFFF) {return -1;}
    memcpy(data, &_serial_var_buf[reg * 2], len);
    return 0;
}

uint32_t gt_serial_var_buffer_get_len_by_eof(uint16_t reg, uint16_t eof_value)
{
    uint16_t end = 0xFFFF - 1;
    uint8_t * ptr = (uint8_t * )&_serial_var_buf[reg << 1];
    uint8_t * end_ptr = (uint8_t * )&_serial_var_buf[end << 1];
    uint8_t check[2] = {0};
    gt_convert_print_u16(check, eof_value);
    while (ptr < end_ptr) {
        if (0 == gt_memcmp(ptr, check, 2)) {
            return ptr - (uint8_t * )&_serial_var_buf[reg << 1];
        }
        ++ptr;
    }
    return 0;
}

int gt_serial_var_buffer_set_reg(uint16_t reg, uint8_t* data, uint16_t len)
{
    if(!data){return -1;}
    if(reg + len > 0xFFFF) {return -1;}
    memcpy(&_serial_var_buf[reg * 2], data, len);
    return 0;
}

int gt_serial_var_buffer_get(gt_var_value_st* value, uint8_t* data, uint16_t len)
{
    if(!value || !data || !value->buffer) {return -1;}
    if(value->len < GT_VAR_REG_LEN || 0 == len) {return -1;}

#if !_REFACTOR_PARSE
    uint16_t reg = gt_convert_parse_u16(value->buffer);
#else
    uint16_t reg;
    gt_bin_convert_st bin_value = {
        .tar = &reg,
        .buffer = value->buffer,
        .byte_len = value->len,
    };
    gt_bin_res_st ret = {0};
    ret = gt_bin_convert_parse(&bin_value, _format_value);
    if (GT_RES_OK != ret.res) {
        GT_LOGE(GT_LOG_TAG_SERIAL, "value parse err code: %d, index: %d", ret.res, ret.index);
        return -1;
    };
#endif

    return gt_serial_var_buffer_get_reg(reg,  data, len);
}

int gt_serial_var_buffer_set(gt_var_value_st* value)
{
    if(!value || !value->buffer || value->len < GT_VAR_REG_LEN) {return -1;}

#if !_REFACTOR_PARSE
    uint16_t reg = gt_convert_parse_u16(value->buffer);
#else
    uint16_t reg;
    gt_bin_convert_st bin_value = {
        .tar = &reg,
        .buffer = value->buffer,
        .byte_len = value->len,
    };
    gt_bin_res_st ret = {0};
    ret = gt_bin_convert_parse(&bin_value, _format_value);
    if (GT_RES_OK != ret.res) {
        GT_LOGE(GT_LOG_TAG_SERIAL, "value parse err code: %d, index: %d", ret.res, ret.index);
        return -1;
    };
#endif
    uint32_t len = value->len - GT_VAR_REG_LEN;

    return gt_serial_var_buffer_set_reg(reg, &value->buffer[GT_VAR_REG_LEN], len);
}

int gt_serial_var_check_value_is_sys_config(gt_var_value_st* value)
{
#if !_REFACTOR_PARSE
    return GT_VAR_IS_SYS_CONFIG(gt_convert_parse_u16(value->buffer)) ? 0 : -1;
#else
    uint16_t reg;
    gt_bin_res_st ret = {0};
    gt_bin_convert_st bin_value = {
        .tar = &reg,
        .buffer = value->buffer,
        .byte_len = value->len,
    };
    ret = gt_bin_convert_parse(&bin_value, _format_value);
    if (GT_RES_OK != ret.res) {
        GT_LOGE(GT_LOG_TAG_SERIAL, "value parse err code: %d, index: %d", ret.res, ret.index);
        return -1;
    };

    if(GT_VAR_IS_SYS_CONFIG(reg)) { return 0;}

    return -1;
#endif
}

int gt_serial_var_check_value_is_set_this(gt_var_st *var, gt_var_value_st* value)
{
    gt_var_head_st var_head = {0};
    if(GT_RES_OK != _get_var_head(var, &var_head)){
        return -1;
    }

#if !_REFACTOR_PARSE
    uint16_t reg = gt_convert_parse_u16(value->buffer);
#else
    uint16_t reg;
    gt_bin_convert_st bin_value = {
        .tar = &reg,
        .buffer = value->buffer,
        .byte_len = value->len,
    };
    ret = gt_bin_convert_parse(&bin_value, _format_value);
    if (GT_RES_OK != ret.res) {
        GT_LOGE(GT_LOG_TAG_SERIAL, "value parse err code: %d, index: %d", ret.res, ret.index);
        return -1;
    };
#endif

    if(GT_VAR_IS_VP(reg, var_head.vp)) { return 0;}
    if(GT_VAR_IS_SP(reg, var_head.sp, var_head.len_dsc)) {
        return ((reg - var_head.sp) | 0x80);
    }
    return -1;
}

gt_res_t gt_serial_var_head_parse(void * user_data, uint8_t const * const buffer, uint32_t len)
{
    gt_var_head_st * var_head = (gt_var_head_st *)user_data;
    var_head->prio = buffer[0];
    var_head->type = buffer[1];
    var_head->sp = (buffer[2] << 8) | buffer[3];
    var_head->len_dsc = (buffer[4] << 8) | buffer[5];
    var_head->vp = (buffer[6] << 8) | buffer[7];
    return GT_RES_OK;
}

gt_res_t gt_serial_var_create(gt_var_st* var)
{
    if(!var || !var->buffer || !var->obj) {return GT_RES_INV;}

    gt_var_head_st var_head = {0};
    if(GT_RES_OK != _get_var_head(var, &var_head)){
        return -1;
    }

    if(0xFFFF != var_head.sp){
        gt_serial_var_buffer_get_reg(var_head.sp, var->buffer + 6, var_head.len_dsc * 2);
    }

    gt_var_handler_st handler = {0};
    handler.var = var;
    handler.type = var_head.type;
    handler.list = _var_handler_list;
    handler.count = sizeof(_var_handler_list) / sizeof(gt_var_handler_item_st);
    return _var_create(&handler);
}

gt_res_t gt_serial_var_set_value(gt_var_st* var, gt_var_value_st* value)
{
    if(!var || !var->buffer || !var->obj || !value || !value->buffer) {return GT_RES_INV;}

    gt_var_head_st var_head = {0};
    if(GT_RES_OK != _get_var_head(var, &var_head)){
        return -1;
    }

    gt_var_handler_st handler = {0};
    handler.var = var;
    handler.value = value;
    handler.type = var_head.type;
    handler.list = _var_handler_list;
    handler.count = sizeof(_var_handler_list) / sizeof(gt_var_handler_item_st);
    return _var_set_value(&handler);
}

uint16_t gt_serial_var_get_value(gt_var_value_st * value, uint8_t * res_buffer)
{
    if(!value || !res_buffer || !value->buffer || value->len < 3) {return 0;}

    uint16_t ret_len = value->buffer[2];;
    // max length is 0x7C
    if(ret_len > 0x7C) { return 0; }

    ret_len <<= 1;

    gt_memcpy(res_buffer, value->buffer, 3);

    if(0 != gt_serial_var_buffer_get(value, res_buffer + 3, ret_len)) {
        return 0;
    }
    ret_len += 3;

    return ret_len;
}


/* end of file ----------------------------------------------------------*/
#endif

