/**
 * @file gt_label.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-11 15:03:35
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_label.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../font/gt_font.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_LAB
#define MY_CLASS    &gt_label_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_label_s {
    gt_obj_st obj;
    char * text;

    gt_color_t  font_color;

    gt_font_info_st font_info;

    gt_align_et font_align;
    uint8_t     space_x;
    uint8_t     space_y;
}_gt_label_st;


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_label_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_label_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

/**
 * @brief obj init label widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    _gt_label_st * style = (_gt_label_st * )obj;

    gt_font_st font = {
        .info       = style->font_info,
        .res        = NULL,
        .utf8       = (char * )style->text,
        .len        = strlen((char * )style->text),
    };

    font.info.thick_en = style->font_info.thick_en == 0 ? style->font_info.size + 6: style->font_info.thick_en;
    font.info.thick_cn = style->font_info.thick_cn == 0 ? style->font_info.size + 6: style->font_info.thick_cn;


    font.info.encoding = gt_project_encoding_get();

    if( 0 == obj->area.w || 0 == obj->area.h ){
        obj->area.h = 20;
        obj->area.w = font.info.size * strlen(font.utf8);
    }

    gt_area_st box_area = gt_area_reduce(obj->area , gt_obj_get_reduce(obj));
    /*draw font*/
    gt_attr_font_st font_attr = {
        .font       = &font,
        .font_color = style->font_color,
        .space_x    = style->space_x,
        .space_y    = style->space_y,
        .align      = style->font_align,
        .opa        = obj->opa,
    };
    draw_text(obj->draw_ctx, &font_attr, &box_area);

    // focus
    draw_focus(obj , 0);
}

/**
 * @brief obj deinit call back
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "label deinit");
    if (NULL == obj) {
        return ;
    }

    _gt_label_st * style_p = (_gt_label_st * )obj;
    if (NULL != style_p->text) {
        gt_mem_free(style_p->text);
        style_p->text = NULL;
    }
}


/**
 * @brief obj event handler call back
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);

    if (GT_EVENT_TYPE_DRAW_START == code) {
        gt_disp_invalid_area(obj);
    }
    else if (GT_EVENT_TYPE_UPDATE_VALUE == code) {
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
    }
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief create a label obj
 *
 * @param parent label's parent element
 * @return gt_obj_st* label obj
 */
gt_obj_st * gt_label_create(gt_obj_st * parent) {
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    _gt_label_st * style = (_gt_label_st * )obj;

    style->text = gt_mem_malloc(sizeof("label"));
    gt_memcpy(style->text, "label\0", sizeof("label"));

    style->font_color        = gt_color_hex(0x00);

    gt_font_info_init(&style->font_info);
    style->font_align           = GT_ALIGN_LEFT;
    style->space_x              = 0;
    style->space_y              = 0;
    return obj;
}


void gt_label_set_text(gt_obj_st * label, const char * fmt, ...)
{
    char buffer[8] = {0};
    va_list args;
    va_list args2;

    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return;
    }

    _gt_label_st * style = (_gt_label_st * )label;
    va_start(args, fmt);
    va_copy(args2, args);
    uint16_t size = (NULL == fmt) ? 0 : (vsnprintf(buffer, sizeof(buffer), fmt, args) + 1);
    va_end(args);
    if (!size) {
        goto free_lb;
    }

    if (NULL == style->text) {
        style->text = gt_mem_malloc(size);
    } else if (size != strlen(style->text) + 1) {
        style->text = gt_mem_realloc(style->text, size);
    }
    if (NULL == style->text) {
        goto free_lb;
    }

    va_start(args2, fmt);
    vsnprintf(style->text, size, fmt, args2);
    va_end(args2);

    gt_event_send(label, GT_EVENT_TYPE_UPDATE_VALUE, NULL);

    return;

free_lb:
    va_end(args2);
}

char * gt_label_get_text(gt_obj_st * label)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return NULL;
    }
    return ((_gt_label_st * )label)->text;
}

void gt_label_set_font_color(gt_obj_st * label, gt_color_t color)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->font_color = color;
    gt_event_send(label, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_label_set_font_size(gt_obj_st * label, uint8_t size)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->font_info.size = size;
}

void gt_label_set_font_gray(gt_obj_st * label, uint8_t gray)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->font_info.gray = gray;
}

void gt_label_set_font_align(gt_obj_st * label, gt_align_et align)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->font_align = align;
}

void gt_label_set_font_family_cn(gt_obj_st * label, gt_family_t family)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->font_info.style_cn = family;
}
void gt_label_set_font_family_en(gt_obj_st * label, gt_family_t family)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->font_info.style_en = family;
}

void gt_label_set_font_family_fl(gt_obj_st * label, gt_family_t family)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->font_info.style_fl = family;
}

void gt_label_set_font_thick_en(gt_obj_st * label, uint8_t thick)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->font_info.thick_en = thick;
}

void gt_label_set_font_thick_cn(gt_obj_st * label, uint8_t thick)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->font_info.thick_cn = thick;
}

void gt_label_set_font_family_numb(gt_obj_st * label, gt_family_t family)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->font_info.style_numb = family;
}

void gt_label_set_space(gt_obj_st * label, uint8_t space_x, uint8_t space_y)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->space_x = space_x;
    style->space_y = space_y;
}

uint8_t gt_label_get_font_size(gt_obj_st * label)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return 0;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    return style->font_info.size;
}

uint8_t gt_label_get_space_x(gt_obj_st * label)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return 0;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    return style->space_x;
}

uint8_t gt_label_get_space_y(gt_obj_st * label)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return 0;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    return style->space_y;
}

uint16_t gt_label_get_longest_line_substring_width(gt_obj_st * label)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return 0;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    return gt_font_get_longest_line_substring_width(&style->font_info, style->text, style->space_x);
}

/* end ------------------------------------------------------------------*/
