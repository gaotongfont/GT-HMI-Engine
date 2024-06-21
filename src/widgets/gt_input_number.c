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
    _field_reg_st reg;
}_gt_input_number_st;


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_input_number_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
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
    sprintf(&buffer[idx], "%d.%dlf", style->reg.len_integer + append_len, style->reg.len_decimal);
    gt_label_set_text(style->label, buffer, style->value);
}

static void _init_cb(gt_obj_st * obj)
{
    // focus
    draw_focus(obj , 0);
}

static void _deinit_cb(gt_obj_st * obj)
{

}

static void _event_cb(struct gt_obj_s * obj, gt_event_st * e)
{
    _gt_input_number_st * style = (_gt_input_number_st * )obj;
    gt_event_type_et code = gt_event_get_code(e);

    switch(code) {
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
    style->min = DBL_MIN;

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


#endif /** #if GT_CFG_ENABLE_INPUT_NUMBER */
/* end ------------------------------------------------------------------*/
