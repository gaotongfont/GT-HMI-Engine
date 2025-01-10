/**
 * @file gt_input_number.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2023-05-26 11:50:41
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_input_number.h"

#if GT_CFG_ENABLE_INPUT_NUMBER
#include "../core/gt_mem.h"
#include "../others/gt_types.h"
#include "./gt_label.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../core/gt_obj_pos.h"
#include "float.h"
#include "../others/gt_log.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_INPUT_NUMBER
#define MY_CLASS    &gt_input_number_class

#define _REG_DECIMAL_BIT_WIDTH      4
#define _REG_FILL_ZERO_BIT_WIDTH    1   /** fill "0" before integer */
#define _REG_RESERVED_BIT_WIDTH     3

/* private typedef ------------------------------------------------------*/
typedef struct _field_length_s {
    uint8_t len_integer;
    uint8_t len_decimal: _REG_DECIMAL_BIT_WIDTH;    /** 2: default, such as: 123.4567 -> 123.45 */
    uint8_t fill_zero: _REG_FILL_ZERO_BIT_WIDTH;    /** 0: default disable; 1: enable. such as: 1 or 001 */
    uint8_t reserved: _REG_RESERVED_BIT_WIDTH;
}_field_reg_st;

typedef struct _gt_input_number_s {
    gt_obj_st obj;
    gt_obj_st * label;  /** @ref gt_label.h */

    double value;
    double step;
    double min;
    double max;
#if GT_USE_SERIAL
    uint8_t* unit;
    gt_input_number_mode_et mode;  /** @ref gt_input_number.h */
#endif
    _field_reg_st reg;
}_gt_input_number_st;


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_input_number_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = (_gt_deinit_cb_t)NULL,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_input_number_st)
};

/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static inline bool _is_fill_zero_front(_gt_input_number_st * style) {
    return style->reg.fill_zero;
}

static void _update_label_value(_gt_input_number_st * style) {
    char buffer[32] = {0};
    uint8_t idx = 0;
    uint8_t append_len = style->reg.len_decimal ? (style->reg.len_decimal + 1) : style->reg.len_decimal;

    if (NULL == style->label) {
        return;
    }
    buffer[idx++] = '%';
    if (_is_fill_zero_front(style)) {
        buffer[idx++] = '0';
    }

#if GT_USE_SERIAL
    double tmp = style->value;

    if(GT_INPUT_NUMBER_MODE_INT == style->mode) {
        uint64_t n_power = 1;
        for(uint8_t i = 0; i < style->reg.len_decimal; i++) {
            n_power *= 10;
        }
        tmp = style->value / n_power;
    }
    char tmp_str[50];
    if(style->unit) {
        sprintf(&buffer[idx], "%d.%dlf%s", style->reg.len_integer + append_len, style->reg.len_decimal, style->unit);
    }
    else{
        sprintf(&buffer[idx], "%d.%dlf", style->reg.len_integer + append_len, style->reg.len_decimal);
    }
    sprintf(tmp_str, buffer, tmp);
    char* d_dot = strchr(tmp_str, '.');
    if(d_dot) {
        if((d_dot - tmp_str) > style->reg.len_integer) {
            gt_label_set_text(style->label, &tmp_str[(d_dot-tmp_str)-style->reg.len_integer], tmp);
        }
        else {
            gt_label_set_text(style->label, tmp_str, tmp);
        }
    }
    else {
        uint8_t unit_len = style->unit ? strlen(style->unit) : 0;
        gt_label_set_text(style->label, &tmp_str[strlen(tmp_str)-style->reg.len_integer - unit_len], tmp);
    }
#else
    sprintf(&buffer[idx], "%d.%dlf", style->reg.len_integer + append_len, style->reg.len_decimal);
    gt_label_set_text(style->label, buffer, style->value);
#endif
}

static void _init_cb(gt_obj_st * obj) {
    // focus
    draw_focus(obj);
}

static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    gt_event_type_et code_val = gt_event_get_code(e);

    switch(code_val) {
        case GT_EVENT_TYPE_DRAW_START:
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;
        case GT_EVENT_TYPE_UPDATE_VALUE:
        case GT_EVENT_TYPE_DRAW_REDRAW:
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;
        case GT_EVENT_TYPE_UPDATE_STYLE: {
            gt_obj_set_size(style->label, obj->area.w, obj->area.h);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;
        }
        default:
            break;
    }
}


/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_input_number_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return NULL;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    style->reg.len_decimal = 2;
    style->max = DBL_MAX;
    style->min = -DBL_MAX;

    style->label = gt_label_create(obj);
    style->label->focus_dis = GT_DISABLED;
    gt_label_set_text(style->label , "");
    return obj;
}

void gt_input_number_set_fill_zero_front(gt_obj_st * obj, bool enabled)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    style->reg.fill_zero = enabled ? 1 : 0;
    _update_label_value(style);
}

bool gt_input_number_get_fill_zero_front(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return false;
    }
    return _is_fill_zero_front((_gt_input_number_st * )obj);
}

void gt_input_number_set_value(gt_obj_st * obj, double value)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    style->value = value;
    if (style->value > style->max) {
        style->value = style->max;
    } else if (style->value < style->min) {
        style->value = style->min;
    }
    _update_label_value(style);
}

double gt_input_number_get_value(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return 0;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    return style->value;
}

void gt_input_number_set_max(gt_obj_st * obj, double max)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    style->max = max;
    if (style->value > style->max) {
        style->value = style->max;
    }
    _update_label_value(style);
}

void gt_input_number_set_min(gt_obj_st * obj, double min)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    style->min = min;
    if (style->value < style->min) {
        style->value = style->min;
    }
    _update_label_value(style);
}

void gt_input_number_set_step(gt_obj_st * obj, double step)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    style->step = step;
    // gt_event_send(obj, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

double gt_input_number_increase(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return 0;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    style->value += style->step;
    if (style->value > style->max) {
        style->value = style->max;
    }
    _update_label_value(style);
    // gt_event_send(obj, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
    return style->value;
}

double gt_input_number_decrease(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return 0;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    style->value -= style->step;
    if (style->value < style->min) {
        style->value = style->min;
    }
    // gt_event_send(obj, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
    _update_label_value(style);
    return style->value;
}

void gt_input_number_set_display_integer_length(gt_obj_st * obj, uint8_t length)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    if (length > 64) {
        length = 64;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    style->reg.len_integer = length;
    _update_label_value(style);
}

void gt_input_number_set_display_decimal_length(gt_obj_st * obj, uint8_t length)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    if (length > 64) {
        length = 64;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    style->reg.len_decimal = length;
    _update_label_value(style);
}

void gt_input_number_set_font_color(gt_obj_st * obj, gt_color_t color)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_font_color(style->label, color);
}

void gt_input_number_set_font_size(gt_obj_st * obj, uint8_t size)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_font_size(style->label, size);
}

void gt_input_number_set_font_gray(gt_obj_st * obj, uint8_t gray)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_font_gray(style->label, gray);
}

void gt_input_number_set_font_align(gt_obj_st * obj, gt_align_et align)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_font_align(style->label, align);
}
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
void gt_input_number_set_font_family_cn(gt_obj_st * obj, gt_family_t family)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_font_family_cn(style->label, family);
}

void gt_input_number_set_font_family_en(gt_obj_st * obj, gt_family_t family)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_font_family_en(style->label, family);
}

void gt_input_number_set_font_family_fl(gt_obj_st * obj, gt_family_t family)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_font_family_fl(style->label, family);
}

void gt_input_number_set_font_family_numb(gt_obj_st * obj, gt_family_t family)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_font_family_numb(style->label, family);
}
#else
void gt_input_number_set_font_family(gt_obj_st * obj, gt_family_t family)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_font_family(style->label, family);
}

void gt_input_number_set_font_cjk(gt_obj_st * obj, gt_font_cjk_et cjk)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_font_cjk(style->label, cjk);
}

#endif
void gt_input_number_set_font_thick_en(gt_obj_st * obj, uint8_t thick)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_font_thick_en(style->label, thick);
}

void gt_input_number_set_font_thick_cn(gt_obj_st * obj, uint8_t thick)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_font_thick_cn(style->label, thick);
}

void gt_input_number_set_font_style(gt_obj_st * obj, gt_font_style_et font_style)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_font_style(style->label, font_style);
}

void gt_input_number_set_space(gt_obj_st * obj, uint8_t space_x, uint8_t space_y)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    if (NULL == style->label) {
        return;
    }
    gt_label_set_space(style->label, space_x , space_y);

}

#if GT_USE_SERIAL
void gt_input_number_set_mode(gt_obj_st * obj, gt_input_number_mode_et mode)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    style->mode = mode;
}

void gt_input_number_set_uint(gt_obj_st * obj, uint8_t* unit)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    GT_CHECK_BACK(unit);

    uint16_t len = strlen((char*)unit);
    if(0 == len){ return ;}

    _gt_input_number_st * style = (_gt_input_number_st * )obj;

    if(style->unit) {
        style->unit = gt_mem_realloc(style->unit, len + 1);
    }
    else {
        style->unit = gt_mem_malloc(len + 1);
    }

    if (NULL == style->unit) { return ; }

    gt_memcpy(style->unit, unit, len);
    style->unit[len] = '\0';
    _update_label_value(style);
}
#endif


#endif /** #if GT_CFG_ENABLE_INPUT_NUMBER */
/* end ------------------------------------------------------------------*/
