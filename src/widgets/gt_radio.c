/**
 * @file gt_radio.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-20 15:29:04
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_radio.h"

#if GT_CFG_ENABLE_RADIO
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_RADIO
#define MY_CLASS    &gt_radio_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_radio_s {
    gt_obj_st obj;
    char *      text;
    gt_color_t  font_color;

    gt_font_info_st font_info;

    uint8_t     space_x;
    uint8_t     space_y;

    gt_point_st font_point_offset;

}_gt_radio_st;


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_radio_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_radio_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static inline void _gt_radio_init_widget(gt_obj_st * radio) {
    _gt_radio_st * style = (_gt_radio_st * )radio;

    gt_font_st font = {
        .info       = style->font_info,
        .res        = NULL,
        .utf8       = style->text,
        .len        = strlen(style->text),

    };
    font.info.thick_en = style->font_info.thick_en == 0 ? style->font_info.size + 6: style->font_info.thick_en;
    font.info.thick_cn = style->font_info.thick_cn == 0 ? style->font_info.size + 6: style->font_info.thick_cn;

    // set default size
    if( radio->area.w == 0 || radio->area.h == 0){
        radio->area.w = style->font_info.size+4 + strlen(style->text)*11;
        radio->area.h = style->font_info.size+4;
    }

    gt_area_st area_base = gt_area_reduce(radio->area, gt_obj_get_reduce(radio));
    area_base.w = style->font_info.size+4;
    area_base.h = area_base.w;
    // area_base.y = radio->area.y + ((radio->area.h - area_base.h) >> 1);
    area_base.x = radio->area.x;
    // font
    gt_area_st area_font = radio->area;
    area_font.x = area_base.x + (area_base.w + 2) + style->font_point_offset.x;
    if(area_font.x < area_base.x) {
        area_font.x =  area_base.x;
    }
    area_font.y = area_base.y + 2 + style->font_point_offset.y;
    if(area_font.y < area_base.y){
        area_font.y = area_base.y;
    }
    area_font.w = radio->area.w > area_base.w ? (radio->area.w - area_base.w) : 0;
    area_font.h = style->font_info.size;
    if(area_font.y + area_font.h > radio->area.y + radio->area.h){
        area_font.h = radio->area.y + radio->area.h - area_font.y;
    }

    //
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill    = 0;
    rect_attr.border_width   = 1;
    rect_attr.border_color   = gt_color_hex(0xA0A0A0);
    rect_attr.bg_opa         = radio->opa;
    rect_attr.bg_color       = gt_color_hex(0xA0A0A0);
    rect_attr.radius         = area_base.h>>1;

    draw_bg(radio->draw_ctx, &rect_attr, &area_base);

    gt_color_t color = gt_color_white();

    if( gt_obj_get_state(radio) != GT_STATE_NONE ){
        color = gt_color_hex(0x808080);
    }

    rect_attr.reg.is_fill   = 1;
    rect_attr.border_color  = color;
    rect_attr.bg_color      = color;
    rect_attr.border_width  = 0;
    area_base.h -= 6;
    area_base.w -= 6;
    area_base.x += 3;
    area_base.y += 3;
    draw_bg(radio->draw_ctx, &rect_attr, &area_base);

    gt_attr_font_st font_attr = {
        .font       = &font,
        .space_x    = style->space_x,
        .space_y    = style->space_y,
        .font_color = style->font_color,
        .align      = GT_ALIGN_NONE,
        .opa        = radio->opa,
    };

    area_font.x += style->font_point_offset.x;
    area_font.y += style->font_point_offset.y;
    font_attr.logical_area = area_font;
    draw_text(radio->draw_ctx, &font_attr, &area_font);

    // focus
    draw_focus(radio , 0);
}

/**
 * @brief obj init radio widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_radio_init_widget(obj);
}

/**
 * @brief obj deinit call back
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj) {
    _gt_radio_st * style_p = (_gt_radio_st * )obj;
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
    switch(code) {
        case GT_EVENT_TYPE_DRAW_START:
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_RELEASED: /* click event finish */
            gt_radio_set_selected(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief create a radio obj
 *
 * @param parent radio's parent element
 * @return gt_obj_st* radio obj
 */
gt_obj_st * gt_radio_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    _gt_radio_st * style = (_gt_radio_st * )obj;

    style->text = gt_mem_malloc(sizeof("radio"));
    gt_memcpy(style->text, "radio\0", sizeof("radio"));

    gt_font_info_init(&style->font_info);
    style->space_x           = 0;
    style->space_y           = 0;

    return obj;
}

void gt_radio_set_selected(gt_obj_st * radio)
{
    if (false == gt_obj_is_type(radio, OBJ_TYPE)) {
        return ;
    }
    gt_size_t idx = 0;
    gt_obj_st * parent = radio->parent;
    gt_obj_st * child = NULL;
    if (GT_TYPE_GROUP == gt_obj_class_get_type(parent)) {
        while(  idx < parent->cnt_child ){
            child = parent->child[idx];
            if( OBJ_TYPE == gt_obj_class_get_type(child) &&
                GT_STATE_NONE != gt_obj_get_state(child)) {
                gt_obj_set_state(child, GT_STATE_NONE);
                gt_event_send(child, GT_EVENT_TYPE_DRAW_START, NULL);
            }
            idx++;
        }
    }
    gt_obj_set_state(radio, GT_STATE_PRESSED);
    gt_event_send(radio, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_radio_set_text(gt_obj_st * radio, const char * fmt, ...)
{
    if (false == gt_obj_is_type(radio, OBJ_TYPE)) {
        return ;
    }
    char buffer[8] = {0};
    va_list args;
    va_list args2;

    _gt_radio_st * style = (_gt_radio_st * )radio;
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

    gt_event_send(radio, GT_EVENT_TYPE_DRAW_START, NULL);

    return;

free_lb:
    va_end(args2);
}

void gt_radio_set_font_color(gt_obj_st * radio, gt_color_t color)
{
    if (false == gt_obj_is_type(radio, OBJ_TYPE)) {
        return ;
    }
    _gt_radio_st * style = (_gt_radio_st * )radio;
    style->font_color = color;
    gt_event_send(radio, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_radio_set_font_size(gt_obj_st * radio, uint8_t size)
{
    if (false == gt_obj_is_type(radio, OBJ_TYPE)) {
        return ;
    }
    _gt_radio_st * style = (_gt_radio_st * )radio;
    style->font_info.size = size;
}
void gt_radio_set_font_gray(gt_obj_st * radio, uint8_t gray)
{
    if (false == gt_obj_is_type(radio, OBJ_TYPE)) {
        return ;
    }
    _gt_radio_st * style = (_gt_radio_st * )radio;
    style->font_info.gray = gray;
}

void gt_radio_set_font_family_cn(gt_obj_st * radio, gt_family_t family)
{
    if (false == gt_obj_is_type(radio, OBJ_TYPE)) {
        return ;
    }
    _gt_radio_st * style = (_gt_radio_st * )radio;
    style->font_info.style_cn = family;
}

void gt_radio_set_font_family_en(gt_obj_st * radio, gt_family_t family)
{
    if (false == gt_obj_is_type(radio, OBJ_TYPE)) {
        return ;
    }
    _gt_radio_st * style = (_gt_radio_st * )radio;
    style->font_info.style_en = family;
}
void gt_radio_set_font_family_fl(gt_obj_st * radio, gt_family_t family)
{
    if (false == gt_obj_is_type(radio, OBJ_TYPE)) {
        return ;
    }
    _gt_radio_st * style = (_gt_radio_st * )radio;
    style->font_info.style_fl = family;
}
void gt_radio_set_font_family_numb(gt_obj_st * radio, gt_family_t family)
{
    if (false == gt_obj_is_type(radio, OBJ_TYPE)) {
        return ;
    }
    _gt_radio_st * style = (_gt_radio_st * )radio;
    style->font_info.style_numb = family;
}

void gt_radio_set_font_thick_en(gt_obj_st * radio, uint8_t thick)
{
    if (false == gt_obj_is_type(radio, OBJ_TYPE)) {
        return ;
    }
    _gt_radio_st * style = (_gt_radio_st * )radio;
    style->font_info.thick_en = thick;
}

void gt_radio_set_font_thick_cn(gt_obj_st * radio, uint8_t thick)
{
    if (false == gt_obj_is_type(radio, OBJ_TYPE)) {
        return ;
    }
    _gt_radio_st * style = (_gt_radio_st * )radio;
    style->font_info.thick_cn = thick;
}

void gt_radio_set_font_encoding(gt_obj_st * radio, gt_encoding_et encoding)
{
    if (false == gt_obj_is_type(radio, OBJ_TYPE)) {
        return ;
    }
    _gt_radio_st * style = (_gt_radio_st * )radio;
    style->font_info.encoding = encoding;
}

void gt_radio_set_space(gt_obj_st * radio, uint8_t space_x, uint8_t space_y)
{
    if (false == gt_obj_is_type(radio, OBJ_TYPE)) {
        return ;
    }
    _gt_radio_st * style = (_gt_radio_st * )radio;
    style->space_x = space_x;
    style->space_y = space_y;
}

void gt_radio_set_font_point_offset_x(gt_obj_st * radio , gt_size_t x)
{
    if (false == gt_obj_is_type(radio, OBJ_TYPE)) {
        return ;
    }
    _gt_radio_st * style = (_gt_radio_st * )radio;
    style->font_point_offset.x = x;
}

void gt_radio_set_font_point_offset_y(gt_obj_st * radio , gt_size_t y)
{
    if (false == gt_obj_is_type(radio, OBJ_TYPE)) {
        return ;
    }
    _gt_radio_st * style = (_gt_radio_st * )radio;
    style->font_point_offset.y = y;
}

void gt_radio_set_font_point_offset(gt_obj_st * radio , gt_size_t x , gt_size_t y)
{
    if (false == gt_obj_is_type(radio, OBJ_TYPE)) {
        return ;
    }
    gt_radio_set_font_point_offset_x( radio , x);
    gt_radio_set_font_point_offset_y( radio , y);
}

#endif  /** GT_CFG_ENABLE_RADIO */
/* end ------------------------------------------------------------------*/
