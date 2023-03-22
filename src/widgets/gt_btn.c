/**
 * @file gt_btn.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-11 15:03:35
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_btn.h"
#include "../core/gt_mem.h"
#include "../hal/gt_hal_disp.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_event.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_BTN
#define MY_CLASS    &gt_btn_class

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_btn_class = {
    ._init_cb = _init_cb,
    ._deinit_cb = _deinit_cb,
    ._event_cb = _event_cb,
    .type = OBJ_TYPE,
    .size_instance = (sizeof(gt_obj_st)),
    .size_style = (sizeof(_gt_style_btn_st))
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static inline void _gt_btn_init_widget(gt_obj_st * btn) {
    _gt_style_btn_st * style = btn->style;
    gt_size_t x, y, w, h;           //base attr
    gt_size_t radius;               //widget attr

    gt_color_t fg_color;
    uint8_t fp[64];
    gt_font_st font = {
        .style_cn   = style->font_family_cn,
        .style_en   = style->font_family_en,
        .res        = fp,
        .utf8       = style->text,
        .len        = strlen(style->text),
        .size       = style->font_size,
    };
    font.encoding = gt_project_encoding_get();
    uint8_t width_font = gt_font_get_string_width(&font);
    // set default size
    if( btn->area.w == 0 || btn->area.h == 0){
        btn->area.w = width_font + 32;
        btn->area.h = style->font_size+16;
    }
    if( style->radius == 0 ){
        radius = btn->area.h >> 2;
    }else{
        radius = style->radius;
    }
    if( gt_obj_style_get_state(btn) != GT_STATE_NONE ){
        fg_color = style->color_pressed;
    }else{
        fg_color = style->color_background;
    }

    /* base shape */
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.is_fill       = style->fill;
    rect_attr.radius        = radius;
    rect_attr.bg_opa        = btn->opa;
    rect_attr.border_width  = 0;
    rect_attr.fg_color      = fg_color;
    rect_attr.bg_color      = fg_color;
    rect_attr.border_color  = fg_color;

    gt_area_st area = btn->area;
    // gt_area_copy(&area, btn->area);
    // 1:base shape
    draw_bg(btn->draw_ctx, &rect_attr, &area);

    // 2:font
    gt_attr_font_st font_attr = {
        .font           = &font,
        .space_x        = 0,
        .space_y        = 0,
        .font_color     = style->font_color,
        .align          = GT_ALIGN_CENTER,
        .opa            = btn->opa,
    };
    draw_text(btn->draw_ctx, &font_attr, &area);

    return;
}

/**
 * @brief obj init btn widget cb
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_btn_init_widget(obj);
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

    _gt_style_btn_st ** style_p = (_gt_style_btn_st ** )&obj->style;
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

    switch(code) {
        case GT_EVENT_TYPE_DRAW_START:
            GT_LOGD(GT_LOG_TAG_GUI, "start draw");
            if( gt_obj_check_scr(obj) && obj->opa > GT_OPA_MIN){
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
            gt_obj_style_set_state(obj, GT_STATE_PRESSED);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
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
            gt_obj_style_set_state(obj, GT_STATE_NONE);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_PROCESS_LOST:
            GT_LOGD(GT_LOG_TAG_GUI, "process lost");
            gt_obj_style_set_state(obj, GT_STATE_NONE);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
}


static void _gt_btn_init_style(gt_obj_st * btn)
{
    _gt_style_btn_st * style = (_gt_style_btn_st * )btn->style;

    gt_memset(style, 0, sizeof(_gt_style_btn_st));

    style->text = gt_mem_malloc(sizeof("btn"));
    strcpy(style->text, "btn");
    style->radius           = 0;
    style->color_pressed    = gt_color_hex(0x1C82D0);    //default color_selected
    style->color_background = gt_color_hex(0x17A7BF);  //default color_unselected
    style->fill             = 1;
    style->font_color       = gt_color_white();        //default color_font
    style->font_family_cn   = GT_CFG_DEFAULT_FONT_FAMILY_CN;
    style->font_family_en   = GT_CFG_DEFAULT_FONT_FAMILY_EN;
    style->font_size        = GT_CFG_DEFAULT_FONT_SIZE;
}




/* global functions / API interface -------------------------------------*/

/**
 * @brief create a btn obj
 *
 * @param parent btn's parent element
 * @return gt_obj_st* btn obj
 */
gt_obj_st * gt_btn_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_btn_init_style(obj);
    return obj;
}

void gt_btn_style_set_color_pressed(gt_obj_st * btn, gt_color_t color)
{
    _gt_style_btn_st * style = btn->style;
    style->color_pressed = color;
    gt_event_send(btn, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btn_style_set_color_background(gt_obj_st * btn, gt_color_t color)
{
    _gt_style_btn_st * style = btn->style;
    style->color_background = color;
    gt_event_send(btn, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btn_style_set_text(gt_obj_st * btn, char * text)
{
    _gt_style_btn_st * style = btn->style;
    if( NULL != style->text ){
        gt_mem_free(style->text);
    }
    uint16_t size = text == NULL ? 0 : strlen(text);
    style->text = gt_mem_malloc(size+1);
    gt_memcpy(style->text, text, size);
    style->text[size] = 0;
    gt_event_send(btn, GT_EVENT_TYPE_DRAW_START, NULL);
}

char * gt_btn_style_get_text(gt_obj_st * btn){
    GT_ASSERT_NULL(btn);
    _gt_style_btn_st * style = btn->style;
    return style->text;
}

void gt_btn_style_set_font_color(gt_obj_st * btn, gt_color_t color){
    _gt_style_btn_st * style = btn->style;
    style->font_color = color;
    gt_event_send(btn, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btn_style_set_font_size(gt_obj_st * btn, uint8_t size)
{
    _gt_style_btn_st * style = btn->style;
    style->font_size = size;
}

void gt_btn_style_set_font_family_cn(gt_obj_st * btn, gt_family_t font_family_cn)
{
    _gt_style_btn_st * style = btn->style;
    style->font_family_cn = font_family_cn;
}

void gt_btn_style_set_font_family_en(gt_obj_st * btn, gt_family_t font_family_en)
{
    _gt_style_btn_st * style = btn->style;
    style->font_family_en = font_family_en;
}

void gt_btn_style_set_radius(gt_obj_st * btn, uint8_t radius)
{
    _gt_style_btn_st * style = btn->style;
    style->radius = radius;
    gt_event_send(btn, GT_EVENT_TYPE_DRAW_START, NULL);
}


/* end ------------------------------------------------------------------*/
