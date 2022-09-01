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

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_CHECKBOX
#define MY_CLASS    &gt_checkbox_class

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class);
static void _deinit_cb(gt_obj_st * obj,const gt_obj_class_st * class);
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

static void _gt_checkbox_init_widget(gt_obj_st * checkbox) {
    _gt_style_checkbox_st * style = checkbox->style;
    gt_size_t x, y, w, h;           //base attr
    uint8_t sel_width = 0;
    gt_color_t fg_color;
    uint8_t fp[64];
    uint32_t cnt_word = 0;
    gt_font_st font = {
        .style_cn = style->font_style_cn,
        .style_en = style->font_style_en,
        .res = fp,
        .utf8 = style->text,
        .len = strlen(style->text),
        .size = style->font_size,
    };
    
    // set default size
    if( checkbox->area.w == 0 || checkbox->area.h == 0){
        checkbox->area.w = gt_font_get_string_width(&font)+32;
        checkbox->area.h = style->font_size+4;
    }
    sel_width = font.size - 3;

    x = checkbox->area.x;
    y = checkbox->area.y;
    w = checkbox->area.w;
    h = checkbox->area.h;

    /* base shape */
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    if( style->selected == 1 ){
        rect_attr.is_fill = 1;
        rect_attr.border_width = 0;
        GT_COLOR_SET(rect_attr.bg_color, 0x0516);
        GT_COLOR_SET(rect_attr.border_color, 0x0516);
    }else{
        rect_attr.is_fill = 1;
        rect_attr.border_width = 2;
        GT_COLOR_SET(rect_attr.bg_color, 0xFFFF);
        GT_COLOR_SET(rect_attr.border_color, 0x0516);
    }
    gt_area_st area = {
        .w = w, .h = h, .x = x, .y = y
    };

    gt_area_st area_box = area;
    area_box.w = h;
    area_box.h = h;
    /* base shape */
    draw_bg(checkbox->draw_ctx, &rect_attr, &area_box);

    /* font */
    gt_attr_font_st font_dsc = {
        .font = &font,
        .space_x = 0,
        .space_y = 0,
        .font_color = gt_color_black(),
        .align = GT_ALIGN_LEFT,
    };
    area.x += h;
    draw_text(checkbox->draw_ctx, &font_dsc, &area);

    if( 0 ){
        const unsigned char gImage_dui[50] = { /* 0X10,0X04,0X00,0X0A,0X00,0X0A, */
        0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X2F,0XFF,
        0XFF,0XFF,0XF4,0X0F,0XFF,0XFF,0XFF,0X00,0XFF,0XE1,0X0E,0XE1,0X0E,0XFF,0XFE,0X11,
        0X11,0XDF,0XFF,0XFF,0XF1,0X1E,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
        0XFF,0XFF,};
    }
}

/**
 * @brief obj init checkbox widget cb
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");
    /* code */
    _gt_checkbox_init_widget(obj);
    GT_LOGV(GT_LOG_TAG_GUI, "end init_cb");
}

/**
 * @brief obj deinit cb
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj, const gt_obj_class_st * class) {
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");
    /* code */
    GT_LOGV(GT_LOG_TAG_GUI, "end deinit_cb");
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
            if( gt_obj_check_scr(obj) ){
                extern void gt_disp_ref_area(gt_area_st * area);
                gt_disp_ref_area(&obj->area);
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
            if( gt_checkbox_style_get_selceted_state(obj) ){
                gt_checkbox_style_set_unselceted(obj);
            }else{
                gt_checkbox_style_set_selceted(obj);
            }
            gt_event_send(obj, GT_EVENT_TYPE_INPUT_PROCESSED, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_PROCESSED: /* click event finish */
            GT_LOGD(GT_LOG_TAG_GUI, "processed");
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
}


static void _gt_checkbox_init_style(gt_obj_st * checkbox)
{
    _gt_style_checkbox_st * style = NULL;
    
    checkbox->style = gt_mem_malloc(sizeof(_gt_style_checkbox_st));
    style = checkbox->style;

    gt_memset(style, 0, sizeof(_gt_style_checkbox_st));

    style->text = gt_mem_malloc(sizeof("checkbox"));
    gt_memcpy(style->text, "checkbox", sizeof("checkbox"));

    style->font_style_cn    = GT_CFG_DEFAULT_FONT_STYLE_CN;
    style->font_style_en    = GT_CFG_DEFAULT_FONT_STYLE_EN;
    style->font_size        = GT_CFG_DEFAULT_FONT_SIZE;
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

void gt_checkbox_style_set_selceted(gt_obj_st * checkbox){
    _gt_style_checkbox_st * style = checkbox->style;
    style->selected = 1;
}
void gt_checkbox_style_set_unselceted(gt_obj_st * checkbox){
    _gt_style_checkbox_st * style = checkbox->style;
    style->selected = 0;
}

uint8_t gt_checkbox_style_get_selceted_state(gt_obj_st * checkbox){
    _gt_style_checkbox_st * style = checkbox->style;
    return style->selected;
}

void gt_checkbox_style_set_text(gt_obj_st * checkbox, char * text){
    _gt_style_checkbox_st * style = checkbox->style;
    if( style->text ){
        gt_mem_free(style->text);
    }
    style->text = gt_mem_malloc(strlen(text)+1);
    gt_memcpy(style->text, text, strlen(text));
    style->text[strlen(text)] = 0;
}
/* end ------------------------------------------------------------------*/
