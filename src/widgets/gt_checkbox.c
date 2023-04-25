/**
 * @file gt_checkbox.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-18 14:11:47
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_checkbox.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_event.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_CHECKBOX
#define MY_CLASS    &gt_checkbox_class

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_checkbox_class = {
    ._init_cb = _init_cb,
    ._deinit_cb = _deinit_cb,
    ._event_cb = _event_cb,
    .type = OBJ_TYPE,
    .size_instance = (sizeof(gt_obj_st)),
    .size_style = (sizeof(_gt_style_checkbox_st))
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static inline void _gt_checkbox_init_widget(gt_obj_st * checkbox) {
    _gt_style_checkbox_st * style = checkbox->style;
    uint8_t fp[64];
    gt_font_st font = {
        .style_cn = style->font_family_cn,
        .style_en = style->font_family_en,
        .res = fp,
        .utf8 = style->text,
        .len = strlen(style->text),
        .size = style->font_size,
        .gray       = style->font_gray,
    };
    font.encoding = gt_project_encoding_get();

    // set default size
    if( checkbox->area.w == 0 || checkbox->area.h == 0){
        checkbox->area.w = style->font_size+4 + strlen(style->text)*11;
        checkbox->area.h = style->font_size+4;
    }

    /* base shape */
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.bg_opa = checkbox->opa;
    if( checkbox->state == GT_STATE_PRESSED ){
        rect_attr.is_fill = 1;
        rect_attr.border_width = 0;
        rect_attr.bg_color = gt_color_hex(0x409EFF);
        rect_attr.border_color = gt_color_hex(0x409EFF);
    }else{
        rect_attr.is_fill = 1;
        rect_attr.border_width = 2;
        rect_attr.bg_color = gt_color_white();
        rect_attr.border_color = gt_color_hex(0x409EFF);
    }
    gt_area_st area = checkbox->area;

    gt_area_st area_box = area;
    area_box.w = style->font_size+4;
    area_box.h = area_box.w;
    // area_box.y = area.y + ((area.h - area_box.h) >> 1);
    area_box.x = area.x;

   /* base shape */
    draw_bg(checkbox->draw_ctx, &rect_attr, &area_box);

    /* font */
    gt_attr_font_st font_dsc = {
        .font           = &font,
        .space_x        = 0,
        .space_y        = 0,
        .font_color     = style->font_color,
        .align          = style->font_align,
        .opa            = checkbox->opa,
    };
    area.x += (area_box.w + 2);
    area.y += 2;
    area.w = area.w >  area_box.w ? (area.w - area_box.w) : 0;
    draw_text(checkbox->draw_ctx, &font_dsc, &area);

    // if( 0 ){
    //     const unsigned char gImage_dui[50] = { /* 0X10,0X04,0X00,0X0A,0X00,0X0A, */
    //     0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X2F,0XFF,
    //     0XFF,0XFF,0XF4,0X0F,0XFF,0XFF,0XFF,0X00,0XFF,0XE1,0X0E,0XE1,0X0E,0XFF,0XFE,0X11,
    //     0X11,0XDF,0XFF,0XFF,0XF1,0X1E,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
    //     0XFF,0XFF,};
    // }
}

/**
 * @brief obj init checkbox widget cb
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_checkbox_init_widget(obj);
}

/**
 * @brief obj deinit cb
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");
    if (NULL == obj) {
        return ;
    }

    _gt_style_checkbox_st ** style_p = (_gt_style_checkbox_st ** )&obj->style;
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
 * @brief obj event handler cb
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);
    extern void _gt_disp_refr_append_area(gt_area_st * area);
    switch(code) {
        case GT_EVENT_TYPE_DRAW_START:
            GT_LOGD(GT_LOG_TAG_GUI, "start draw");
            if( gt_obj_check_scr(obj) ){
                _gt_disp_refr_append_area(&obj->area);
            }
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_DRAW_END:
            GT_LOGD(GT_LOG_TAG_GUI, "end draw");
            break;

        case GT_EVENT_TYPE_CHANGE_CHILD_REMOVE: /* remove child from screen but not delete */
            GT_LOGD(GT_LOG_TAG_GUI, "child remove");
			break;

        case GT_EVENT_TYPE_CHANGE_CHILD_DELETE: /* delete child */
            GT_LOGD(GT_LOG_TAG_GUI, "child delete");
            break;

        case GT_EVENT_TYPE_INPUT_CLICKING:   /* add clicking style and process clicking event */
            GT_LOGD(GT_LOG_TAG_GUI, "clicking");

            break;

        case GT_EVENT_TYPE_INPUT_SCROLL:
            GT_LOGD(GT_LOG_TAG_GUI, "scroll");
            break;

        case GT_EVENT_TYPE_INPUT_PROCESS:   /* remove clicking style and process click event */
            GT_LOGD(GT_LOG_TAG_GUI, "process");
            /* code here */
            gt_event_send(obj, GT_EVENT_TYPE_INPUT_PROCESSED, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_PROCESSED: /* click event finish */
            GT_LOGD(GT_LOG_TAG_GUI, "processed");
            if( gt_obj_style_get_state(obj) != GT_STATE_NONE ){
                gt_obj_style_set_state(obj, GT_STATE_NONE);
            }else{
                gt_obj_style_set_state(obj, GT_STATE_PRESSED);
            }
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
}


static void _gt_checkbox_init_style(gt_obj_st * checkbox)
{
    _gt_style_checkbox_st * style = (_gt_style_checkbox_st * )checkbox->style;

    gt_memset(style, 0, sizeof(_gt_style_checkbox_st));

    style->text = gt_mem_malloc(sizeof("checkbox"));
    gt_memcpy(style->text, "checkbox", sizeof("checkbox"));

    style->font_family_cn   = GT_CFG_DEFAULT_FONT_FAMILY_CN;
    style->font_family_en   = GT_CFG_DEFAULT_FONT_FAMILY_EN;
    style->font_size        = GT_CFG_DEFAULT_FONT_SIZE;
    style->font_color       = gt_color_black();
    style->font_gray        = 1;
    style->font_align = GT_ALIGN_NONE;
}




/* global functions / API interface -------------------------------------*/

/**
 * @brief create a checkbox obj
 *
 * @param parent checkbox's parent element
 * @return gt_obj_st* checkbox obj
 */
gt_obj_st * gt_checkbox_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_checkbox_init_style(obj);
    return obj;
}


void gt_checkbox_style_set_text(gt_obj_st * checkbox, char * text){
    _gt_style_checkbox_st * style = checkbox->style;
    if( style->text ){
        gt_mem_free(style->text);
    }
    uint16_t len = text == NULL ? 0 : strlen(text);
    style->text = gt_mem_malloc(len+1);
    gt_memcpy(style->text, text, len);
    style->text[len] = 0;
    gt_event_send(checkbox, GT_EVENT_TYPE_DRAW_START, NULL);
}
char * gt_checkbox_style_get_text(gt_obj_st * checkbox, char * text){
    _gt_style_checkbox_st * style = checkbox->style;
    return style->text;
}

void gt_checkbox_style_set_font_family_cn(gt_obj_st * checkbox, gt_family_t family)
{
    _gt_style_checkbox_st * style = checkbox->style;
    style->font_family_cn = family;
}

void gt_checkbox_style_set_font_family_en(gt_obj_st * checkbox, gt_family_t family)
{
    _gt_style_checkbox_st * style = checkbox->style;
    style->font_family_en = family;
}

void gt_checkbox_style_set_font_size(gt_obj_st * checkbox, uint8_t size)
{
    _gt_style_checkbox_st * style = checkbox->style;
    style->font_size = size;
}
void gt_checkbox_style_set_font_gray(gt_obj_st * checkbox, uint8_t gray)
{
    _gt_style_checkbox_st * style = checkbox->style;
    style->font_gray = gray;
}
void gt_checkbox_style_set_font_align(gt_obj_st * checkbox, uint8_t align)
{
    _gt_style_checkbox_st * style = checkbox->style;
    style->font_align = align;
}
void gt_checkbox_style_set_font_color(gt_obj_st * checkbox, gt_color_t color)
{
    _gt_style_checkbox_st * style = checkbox->style;
    style->font_color = color;
    gt_event_send(checkbox, GT_EVENT_TYPE_DRAW_START, NULL);
}
/* end ------------------------------------------------------------------*/
