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
/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_RADIO
#define MY_CLASS    &gt_radio_class

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class);
static void _deinit_cb(gt_obj_st * obj,const gt_obj_class_st * class);
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_radio_class = {
    ._init_cb = _init_cb,
    ._deinit_cb = _deinit_cb,
    ._event_cb = _event_cb,
    .type = OBJ_TYPE,
    .size_instance = (sizeof(gt_obj_st)),
    .size_style = (sizeof(_gt_style_radio_st))
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _gt_radio_init_widget(gt_obj_st * radio) {
    _gt_style_radio_st * style = radio->style;
    gt_size_t x, y, w, h;
    uint8_t sel_width = 0;
    gt_color_t color_font;

    GT_COLOR_SET(color_font, 0x0000);   //radio font color;
    
    uint8_t fp[1024];
    gt_font_st font = {
        .style_cn = style->font_style_cn,
        .style_en = style->font_style_en,
        .res = fp,
        .utf8 = (uint8_t *)style->text,
        .len = strlen(style->text),
        .size = style->font_size,
    };

    
    
    // set default size
    if( radio->area.w == 0 || radio->area.h == 0){
        radio->area.w = gt_font_get_string_width(&font)+24;
        radio->area.h = GT_FONT_SIZE_16x16+4;
    }
    sel_width = font.size;

    x = radio->area.x;
    y = radio->area.y;
    w = radio->area.w;
    h = radio->area.h;


    gt_attr_rect_st rect_attr = {
        .is_fill        = 0,
        .border_width   = 2,
        .border_color   = gt_color_make(0xA0,0xA0,0xA0),
        .bg_opa         = GT_OPA_COVER,
        .bg_color       = gt_color_make(0xA0,0xA0,0xA0),
        .radius         = h>>1,
    };
    
    gt_area_st area_base = radio->area;
    area_base.h = h;
    area_base.w = area_base.h;
    draw_bg(radio->draw_ctx, &rect_attr, &area_base);

    if( style->selected == 1 ){
        gt_area_st area_sel = area_base;
        area_sel.y += rect_attr.border_width<<1;
        area_sel.x += rect_attr.border_width<<1;
        area_sel.w -= rect_attr.border_width<<2;
        area_sel.h -= rect_attr.border_width<<2;
        rect_attr.radius = area_sel.h>>1;
        rect_attr.is_fill = 1;
        rect_attr.bg_color = gt_color_make(0xA0,0xA0,0xA0);
        draw_bg(radio->draw_ctx, &rect_attr, &area_sel);
    }

    gt_attr_font_st font_attr = {
        .font = &font,
        .space_x = 0,
        .space_y = 0,
        .font_color = color_font,
        .align = GT_ALIGN_LEFT,
    };
    gt_area_st area_font = radio->area;
    area_font.x += area_base.h;
    area_font.w = w + 60;
    draw_text(radio->draw_ctx, &font_attr, &area_font);
}

/**
 * @brief obj init radio widget cb
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");
    /* code */
    _gt_radio_init_widget(obj);
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

            // test
            int idx = 0;
            while( idx < obj->parent->cnt_child ){
                if( obj->parent->child[idx]->class->type == GT_TYPE_RADIO ){
                    gt_radio_style_set_unselceted(obj->parent->child[idx++]);
                }
            }
            gt_radio_style_set_selceted(obj);
            
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


static void _gt_radio_init_style(gt_obj_st * radio)
{
    _gt_style_radio_st * style = NULL;
    
    radio->style = gt_mem_malloc(sizeof(_gt_style_radio_st));
    style = radio->style;

    gt_memset(style,0,sizeof(_gt_style_radio_st));

    style->text = gt_mem_malloc(sizeof("radio"));
    gt_memcpy(style->text, "radio", sizeof("radio"));

    style->font_style_cn    = GT_CFG_DEFAULT_FONT_STYLE_CN;
    style->font_style_en    = GT_CFG_DEFAULT_FONT_STYLE_EN;
    style->font_size        = GT_CFG_DEFAULT_FONT_SIZE;
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

void gt_radio_style_set_selceted(gt_obj_st * radio){
    _gt_style_radio_st * style = radio->style;
    style->selected = 1;
}
void gt_radio_style_set_unselceted(gt_obj_st * radio){
    _gt_style_radio_st * style = radio->style;
    style->selected = 0;
    gt_event_send(radio, GT_EVENT_TYPE_DRAW_START, NULL);
}

uint8_t gt_radio_style_get_selceted_state(gt_obj_st * radio){
    _gt_style_radio_st * style = radio->style;
    return style->selected;
}

void gt_radio_style_set_text(gt_obj_st * radio, char * text){
    _gt_style_radio_st * style = radio->style;
    if( style->text ){
        gt_mem_free(style->text);
    }
    style->text = gt_mem_malloc(strlen(text)+1);
    gt_memcpy(style->text, text, strlen(text));
    style->text[strlen(text)] = 0;
}
/* end ------------------------------------------------------------------*/
