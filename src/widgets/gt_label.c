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
typedef struct _gt_label_s{
    char * text;

    gt_color_t  font_color;

    gt_font_info_st font_info;

    uint8_t     font_align;
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
    _gt_label_st * style = (_gt_label_st * )obj->style;

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

    gt_area_st box_area = gt_area_reduce(obj->area , REDUCE_DEFAULT);
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

    _gt_label_st ** style_p = (_gt_label_st ** )&obj->style;
    if (NULL == *style_p) {
        return ;
    }

    if (NULL != (*style_p)->text) {
        gt_mem_free((*style_p)->text);
        (*style_p)->text = NULL;
    }

    gt_mem_free(*style_p);
    *style_p = NULL;
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
        return NULL;
    }
    _gt_label_st * style = (_gt_label_st * )obj->style;
    if (NULL == style) {
        gt_obj_destroy(obj);
        return NULL;
    }

    gt_memset(style, 0, sizeof(_gt_label_st));

    style->text = gt_mem_malloc(sizeof("label"));
    gt_memcpy(style->text, "label\0", sizeof("label"));

    style->font_color        = gt_color_hex(0x00);

    style->font_info.style_cn   = GT_CFG_DEFAULT_FONT_FAMILY_CN;
    style->font_info.style_en   = GT_CFG_DEFAULT_FONT_FAMILY_EN;
    style->font_info.style_fl   = GT_CFG_DEFAULT_FONT_FAMILY_FL;
    style->font_info.style_numb = GT_CFG_DEFAULT_FONT_FAMILY_NUMB;
    style->font_info.size       = GT_CFG_DEFAULT_FONT_SIZE;
    style->font_info.gray       = 1;
    style->font_align           = GT_ALIGN_LEFT;
    style->font_info.thick_en   = 0;
    style->font_info.thick_cn   = 0;
    style->space_x              = 0;
    style->space_y              = 0;
    return obj;
}


void gt_label_set_text(gt_obj_st * label, const char * fmt, ...)
{
    char buffer[8] = {0};
    va_list args;
    va_list args2;
    va_start(args, fmt);
    va_copy(args2, args);

    _gt_label_st * style = (_gt_label_st * )label->style;
    if( NULL != style->text ){
        gt_mem_free(style->text);
    }
    uint16_t size = (NULL == fmt) ? 0 : (vsnprintf(buffer, sizeof(buffer), fmt, args) + 1);
    if (!size) {
        goto free_lb;
    }
    style->text = gt_mem_malloc(size);
    vsnprintf(style->text, size, fmt, args2);
    gt_event_send(label, GT_EVENT_TYPE_UPDATE_VALUE, NULL);

free_lb:
    va_end(args2);
    va_end(args);
}

char * gt_label_get_text(gt_obj_st * label)
{
    if (!label) {
        return NULL;
    }
    return ((_gt_label_st * )label->style)->text;
}

void gt_label_set_font_color(gt_obj_st * label, gt_color_t color)
{
    _gt_label_st * style = (_gt_label_st * )label->style;
    style->font_color = color;
    gt_event_send(label, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_label_set_font_size(gt_obj_st * label, uint8_t size)
{
    _gt_label_st * style = (_gt_label_st * )label->style;
    style->font_info.size = size;
}

void gt_label_set_font_gray(gt_obj_st * label, uint8_t gray)
{
    _gt_label_st * style = (_gt_label_st * )label->style;
    style->font_info.gray = gray;
}

void gt_label_set_font_align(gt_obj_st * label, uint8_t align)
{
    _gt_label_st * style = (_gt_label_st * )label->style;
    style->font_align = align;
}

void gt_label_set_font_family_cn(gt_obj_st * label, gt_family_t family)
{
    _gt_label_st * style = (_gt_label_st * )label->style;
    style->font_info.style_cn = family;
}
void gt_label_set_font_family_en(gt_obj_st * label, gt_family_t family)
{
    _gt_label_st * style = (_gt_label_st * )label->style;
    style->font_info.style_en = family;
}

void gt_label_set_font_family_fl(gt_obj_st * label, gt_family_t family)
{
    _gt_label_st * style = (_gt_label_st * )label->style;
    style->font_info.style_fl = family;
}

void gt_label_set_font_thick_en(gt_obj_st * label, uint8_t thick)
{
    _gt_label_st * style = (_gt_label_st * )label->style;
    style->font_info.thick_en = thick;
}
void gt_label_set_font_thick_cn(gt_obj_st * label, uint8_t thick)
{
    _gt_label_st * style = (_gt_label_st * )label->style;
    style->font_info.thick_cn = thick;
}
void gt_label_set_font_family_numb(gt_obj_st * label, gt_family_t family)
{
    _gt_label_st * style = (_gt_label_st * )label->style;
    style->font_info.style_numb = family;
}
void gt_label_set_space(gt_obj_st * label, uint8_t space_x, uint8_t space_y)
{
    _gt_label_st * style = (_gt_label_st * )label->style;
    style->space_x = space_x;
    style->space_y = space_y;
}

uint8_t gt_label_get_font_size(gt_obj_st * label)
{
    _gt_label_st * style = (_gt_label_st * )label->style;
    return style->font_info.size;
}

uint8_t gt_label_get_space_x(gt_obj_st * label)
{
    _gt_label_st * style = (_gt_label_st * )label->style;
    return style->space_x;
}

uint8_t gt_label_get_space_y(gt_obj_st * label)
{
    _gt_label_st * style = (_gt_label_st * )label->style;
    return style->space_y;
}

uint16_t gt_label_get_longest_line_substring_width(gt_obj_st * label)
{
    if (NULL == label) {
        return 0;
    }
    if (GT_TYPE_LAB != gt_obj_class_get_type(label)) {
        return 0;
    }
    _gt_label_st * style = (_gt_label_st * )label->style;
    if (NULL == style) {
        return 0;
    }

    return gt_font_get_longest_line_substring_width(&style->font_info, style->text, style->space_x);
}

/* end ------------------------------------------------------------------*/
