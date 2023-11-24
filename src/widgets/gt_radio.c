/**
 * @file gt_radio.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-20 15:29:04
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_radio.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_RADIO
#define MY_CLASS    &gt_radio_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_radio_s
{
    char *      text;
    gt_color_t  font_color;
    gt_family_t font_family_cn;
    gt_family_t font_family_en;
    gt_family_t font_family_numb;
    uint8_t     font_size;
    uint8_t     font_gray;
    uint8_t     font_align;
    uint8_t     thick_en;
    uint8_t     thick_cn;
    uint8_t     space_x;
    uint8_t     space_y;
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
    _gt_radio_st * style = radio->style;

    gt_font_st font = {
        .style_cn   = style->font_family_cn,
        .style_en   = style->font_family_en,
        .style_numb = style->font_family_numb,
        .res        = NULL,
        .utf8       = style->text,
        .len        = strlen(style->text),
        .size       = style->font_size,
        .gray       = style->font_gray,
    };
    font.thick_en = style->thick_en == 0 ? style->font_size + 6: style->thick_en;
    font.thick_cn = style->thick_cn == 0 ? style->font_size + 6: style->thick_cn;
    font.encoding = gt_project_encoding_get();

    // set default size
    if( radio->area.w == 0 || radio->area.h == 0){
        radio->area.w = style->font_size+4 + strlen(style->text)*11;
        radio->area.h = style->font_size+4;

    }

    gt_area_st area_base = gt_area_reduce(radio->area, REDUCE_DEFAULT);
    area_base.w = style->font_size+4;
    area_base.h = area_base.w;
    // area_base.y = radio->area.y + ((radio->area.h - area_base.h) >> 1);
    area_base.x = radio->area.x;
    // font
    gt_area_st area_font = radio->area;
    area_font.x = area_base.x + (area_base.w + 2);
    area_font.y += 2;
    area_font.w = radio->area.w > area_base.w ? (radio->area.w - area_base.w) : 0;

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
        .align      = style->font_align,
        .opa        = radio->opa,
    };

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
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");
    if (NULL == obj) {
        return ;
    }

    _gt_radio_st ** style_p = (_gt_radio_st ** )&obj->style;
    if (NULL ==  *style_p) {
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
    switch(code) {
        case GT_EVENT_TYPE_DRAW_START:
            GT_LOGV(GT_LOG_TAG_GUI, "start draw");
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_DRAW_END:
            GT_LOGV(GT_LOG_TAG_GUI, "end draw");
            break;

        case GT_EVENT_TYPE_CHANGE_CHILD_REMOVE: /* remove child from screen but not delete */
            GT_LOGV(GT_LOG_TAG_GUI, "child remove");
			break;

        case GT_EVENT_TYPE_CHANGE_CHILD_DELETE: /* delete child */
            GT_LOGV(GT_LOG_TAG_GUI, "child delete");
            break;

        case GT_EVENT_TYPE_INPUT_PRESSING:   /* add clicking style and process clicking event */
            GT_LOGV(GT_LOG_TAG_GUI, "clicking");
            break;

        case GT_EVENT_TYPE_INPUT_SCROLL:
            GT_LOGV(GT_LOG_TAG_GUI, "scroll");
            break;

        case GT_EVENT_TYPE_INPUT_RELEASED: /* click event finish */
            GT_LOGV(GT_LOG_TAG_GUI, "processed");
            gt_radio_set_selected(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
}


static void _gt_radio_init_style(gt_obj_st * radio)
{
    _gt_radio_st * style = (_gt_radio_st * )radio->style;

    gt_memset(style,0,sizeof(_gt_radio_st));

    style->text = gt_mem_malloc(sizeof("radio"));
    gt_memcpy(style->text, "radio\0", sizeof("radio"));

    style->font_family_cn    = GT_CFG_DEFAULT_FONT_FAMILY_CN;
    style->font_family_en    = GT_CFG_DEFAULT_FONT_FAMILY_EN;
    style->font_family_numb  = GT_CFG_DEFAULT_FONT_FAMILY_NUMB;
    style->font_size         = GT_CFG_DEFAULT_FONT_SIZE;
    style->font_gray         = 1;
    style->font_align        = GT_ALIGN_NONE;
    style->thick_en          = 0;
    style->thick_cn          = 0;
    style->space_x           = 0;
    style->space_y           = 0;
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
    _gt_radio_init_style(obj);
    return obj;
}

void gt_radio_set_selected(gt_obj_st * radio){
    int idx = 0;
    gt_obj_st * obj = radio;
    while( obj->parent->class->type == GT_TYPE_GROUP && idx < obj->parent->cnt_child ){
        if( obj->parent->child[idx]->class->type == GT_TYPE_RADIO ){
            gt_obj_set_state(obj->parent->child[idx], GT_STATE_NONE);
            gt_event_send(obj->parent->child[idx], GT_EVENT_TYPE_DRAW_START, NULL);
        }
        idx++;
    }
    gt_obj_set_state(obj, GT_STATE_PRESSED);
    gt_event_send(radio, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_radio_set_text(gt_obj_st * radio, const char * fmt, ...)
{
    char buffer[8] = {0};
    va_list args;
    va_list args2;
    va_start(args, fmt);
    va_copy(args2, args);

    _gt_radio_st * style = radio->style;
    if( style->text ){
        gt_mem_free(style->text);
    }
    uint16_t size = (NULL == fmt) ? 0 : (vsnprintf(buffer, sizeof(buffer), fmt, args) + 1);
    if (!size) {
        goto free_lb;
    }
    style->text = gt_mem_malloc(size);
    vsnprintf(style->text, size, fmt, args2);
    gt_event_send(radio, GT_EVENT_TYPE_DRAW_START, NULL);

free_lb:
    va_end(args2);
    va_end(args);
}

void gt_radio_set_font_color(gt_obj_st * radio, gt_color_t color)
{
    _gt_radio_st * style = radio->style;
    style->font_color = color;
    gt_event_send(radio, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_radio_set_font_size(gt_obj_st * radio, uint8_t size)
{
    _gt_radio_st * style = radio->style;
    style->font_size = size;
}
void gt_radio_set_font_gray(gt_obj_st * radio, uint8_t gray)
{
    _gt_radio_st * style = radio->style;
    style->font_gray = gray;
}
void gt_radio_set_font_align(gt_obj_st * radio, uint8_t align)
{
    _gt_radio_st * style = radio->style;
    style->font_align = align;
}
void gt_radio_set_font_family_cn(gt_obj_st * radio, gt_family_t family)
{
    _gt_radio_st * style = radio->style;
    style->font_family_cn = family;
}

void gt_radio_set_font_family_en(gt_obj_st * radio, gt_family_t family)
{
    _gt_radio_st * style = radio->style;
    style->font_family_en = family;
}

void gt_radio_set_font_family_numb(gt_obj_st * radio, gt_family_t family)
{
    _gt_radio_st * style = radio->style;
    style->font_family_numb = family;
}

void gt_radio_set_font_thick_en(gt_obj_st * radio, uint8_t thick)
{
    _gt_radio_st * style = (_gt_radio_st * )radio->style;
    style->thick_en = thick;
}
void gt_radio_set_font_thick_cn(gt_obj_st * radio, uint8_t thick)
{
    _gt_radio_st * style = (_gt_radio_st * )radio->style;
    style->thick_cn = thick;
}
void gt_radio_set_space(gt_obj_st * radio, uint8_t space_x, uint8_t space_y)
{
    _gt_radio_st * style = (_gt_radio_st * )radio->style;
    style->space_x = space_x;
    style->space_y = space_y;
}

/* end ------------------------------------------------------------------*/
