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
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_BTN
#define MY_CLASS    &gt_btn_class

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class);
static void _deinit_cb(gt_obj_st * obj,const gt_obj_class_st * class);
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

static void _gt_btn_init_widget(gt_obj_st * btn) {
    _gt_style_btn_st * style = btn->style;
    gt_size_t x, y, w, h;           //base attr
    gt_size_t radius;      //widget attr


    gt_color_t fg_color;
    uint8_t fp[64];
    gt_font_st font = {
        .style_cn   = style->font_style_cn,
        .style_en   = style->font_style_en,
        .res        = fp,
        .utf8       = (uint8_t *)style->text,
        .len        = strlen(style->text),
        .size       = style->font_size,
    };
    uint8_t width_font = gt_font_get_string_width(&font);
    // set default size
    if( btn->area.w == 0 || btn->area.h == 0){
        btn->area.w = width_font + 32;
        btn->area.h = GT_FONT_SIZE_16x16+16;
    }

    x = btn->area.x;
    y = btn->area.y;
    w = btn->area.w;
    h = btn->area.h;

    radius = h/4;
    if( style->selected ){
        fg_color = style->color_selected;
    }else{
        fg_color = style->color_unselected;
    }

    /* base shape */
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.is_fill = 1;
    rect_attr.radius = radius;
    rect_attr.border_width = 0;
    rect_attr.fg_color = fg_color;
    rect_attr.bg_color = fg_color;
    rect_attr.border_color = fg_color;

    gt_area_st area = {
        .w = w, .h = h, .x = x, .y = y
    };
    
    // 1:base shape
    draw_bg(btn->draw_ctx, &rect_attr, &area);
    
    // 2:font
    gt_attr_font_st font_attr = {
        .font = &font,
        .space_x = 0,
        .space_y = 0,
        .font_color = style->color_font,
        .align = GT_ALIGN_CENTER,
    };
    draw_text(btn->draw_ctx, &font_attr, &area);
    
    return;

}

/**
 * @brief obj init btn widget cb
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");
    /* code */
    _gt_btn_init_widget(obj);
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
            gt_btn_style_set_selected(obj, 1);
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
            gt_btn_style_set_selected(obj, 0);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
}


static void _gt_btn_init_style(gt_obj_st * btn)
{
    _gt_style_btn_st * style = NULL;
    
    btn->style = gt_mem_malloc(sizeof(_gt_style_btn_st));
    style = btn->style;

    gt_memset(style, 0, sizeof(_gt_style_btn_st));
    
    style->text = gt_mem_malloc(sizeof("btn"));
    strcpy(style->text, "btn");
    style->radius = 0;
	style->selected = 0;
    style->color_font = gt_color_white();        //default color_font
    style->color_selected = gt_color_hex(0x1C82D0);    //default color_selected
    style->color_unselected = gt_color_make(0x17A7BF);  //default color_unselected
    
    style->font_style_cn    = GT_CFG_DEFAULT_FONT_STYLE_CN;
    style->font_style_en    = GT_CFG_DEFAULT_FONT_STYLE_EN;
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

void gt_btn_style_set_color_selected(gt_obj_st * btn, gt_color_t color)
{
    _gt_style_btn_st * style = btn->style;
    GT_COLOR_SET(style->color_selected, GT_COLOR_GET(color));
}

void gt_btn_style_set_color_unselected(gt_obj_st * btn, gt_color_t color)
{
    _gt_style_btn_st * style = btn->style;
    GT_COLOR_SET(style->color_unselected, GT_COLOR_GET(color));
}

void gt_btn_style_set_selected(gt_obj_st * btn, uint8_t selected)
{
    _gt_style_btn_st * style = btn->style;
    style->selected = selected!=0?1:0;
}

void gt_btn_style_set_text(gt_obj_st * btn, char * text)
{
    _gt_style_btn_st * style = btn->style;
    if( NULL != style->text ){
        gt_mem_free(style->text);
    }
    uint16_t size = strlen(text);
    style->text = gt_mem_malloc(size+1);
    gt_memset(style->text, 0, size+1);
    gt_memcpy(style->text, text, size);
}

char * gt_btn_style_get_text(gt_obj_st * btn){
    GT_ASSERT_NULL(btn);
    _gt_style_btn_st * style = btn->style;
    return style->text;
}

void gt_btn_style_set_font_color(gt_obj_st * btn, gt_color_t color){
    _gt_style_btn_st * style = btn->style;
    GT_COLOR_SET(style->color_font, GT_COLOR_GET(color));
}
/* end ------------------------------------------------------------------*/
