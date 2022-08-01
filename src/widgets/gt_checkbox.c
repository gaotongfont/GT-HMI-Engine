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
    gt_color_t fcolor;
    uint8_t * fp = gt_mem_malloc(1024);
    uint32_t cnt_word = 0;
    gt_font_st font = {
        .style_cn = 1,
        .style_en = 2,
        .res = fp,
        .utf8 = (uint8_t *)style->text,
        .len = strlen(style->text),
        .size = GT_FONT_SIZE_16x16,
    };
    
    // set default size
    if( checkbox->area.w == 0 || checkbox->area.h == 0){
        checkbox->area.w = gt_font_get_string_width(&font)+20;
        checkbox->area.h = GT_FONT_SIZE_16x16+4;
    }
    sel_width = font.size - 3;

    x = checkbox->area.x;
    y = checkbox->area.y;
    w = checkbox->area.w;
    h = checkbox->area.h;


    checkbox->cache.cache_color = gt_mem_malloc( w * h * sizeof(gt_color_t) );
    
    /* base shape */
    fcolor.full = 0x0000;
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    if( style->selected == 1 ){
        rect_attr.is_fill = 1;
        rect_attr.border_width = 0;
        GT_COLOR_SET(rect_attr.fcolor, 0x0516);
        GT_COLOR_SET(rect_attr.bcolor, 0x0516);
        GT_COLOR_SET(rect_attr.border_color, 0x0516);
    }else{
        rect_attr.is_fill = 1;
        rect_attr.border_width = 2;
        GT_COLOR_SET(rect_attr.fcolor, 0xFFFF);
        GT_COLOR_SET(rect_attr.bcolor, 0x0516);
        GT_COLOR_SET(rect_attr.border_color, 0x0516);
    }
    gt_area_st area = {
        .w = w,
        .h = h,
        .x = x,
        .y = y
    };
	
    gt_color_t * color_widget_p = (gt_color_t *)checkbox->cache.cache_color;
    gt_memset(color_widget_p, 0xFF, w * h * sizeof(gt_color_t));

    gt_color_t * color_rect_p = gt_mem_malloc( sel_width * sel_width * sizeof(gt_color_t) );
    gt_area_st area1 = {
        .w = sel_width,
        .h = sel_width,
    };
    gt_graph_rect(&rect_attr, &area1, color_rect_p);
    gt_area_st area_dst = {
        .x = 0,
        .y = ((h - font.size)>>1) + (font.size - sel_width),
        .w = w,
        .h = h
    }, area_src = {
        .x = 0,
        .y = 0,
        .w = sel_width,
        .h = sel_width
    };
    gt_color_area_copy(color_widget_p, area_dst, color_rect_p, area_src, sel_width, sel_width);
    gt_mem_free(color_rect_p);

    if( style->selected == 1 ){
        const unsigned char gImage_dui[50] = { /* 0X10,0X04,0X00,0X0A,0X00,0X0A, */
        0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X2F,0XFF,
        0XFF,0XFF,0XF4,0X0F,0XFF,0XFF,0XFF,0X00,0XFF,0XE1,0X0E,0XE1,0X0E,0XFF,0XFE,0X11,
        0X11,0XDF,0XFF,0XFF,0XF1,0X1E,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
        0XFF,0XFF,};

        gt_color_t color_dui_p[10*10];
        extern void _gt_graph_gray_to_color(uint8_t *gray, gt_color_t *color,gt_size_t w, gt_size_t h, gt_color_t fcolor, gt_color_t bcolor, gt_size_t level_gray);
        gt_color_t _fcolor;
        _fcolor.full = 0xFFFF;
        _gt_graph_gray_to_color(gImage_dui, color_dui_p, 10, 10, rect_attr.bcolor, _fcolor, 4);
        gt_area_st area_dui = {.x = 0, .y = 0, .w = 10, .h = 10};
        area_dst.x = 2;
        area_dst.y = area_dst.y+1;
        gt_color_area_copy(color_widget_p, area_dst, color_dui_p,area_dui, 10, 10);
    }
    /* font */
    cnt_word = gt_font_count_word(style->text, strlen(style->text));
    gt_memset(fp, 0, 1024);
    
    uint32_t w_font = gt_font_get_string_width(&font);
    gt_font_get_string_dot(&font);

    int16_t x_start = sel_width + 2;
    int16_t y_start = (h - GT_FONT_SIZE_16x16)-1;

    if( x_start < 0 ){
        x_start = 0;
    }
    if( y_start < 0 ){
        y_start = 0;
    }

    // fill to shape
    uint16_t idx_dot = 0;
    area.x = x_start;
    area.y = y_start;

    while( cnt_word-- && (x_start+font.size) < w ){
        area.x = x_start;
        gt_graph_dot_draw_to_color(&font.res[idx_dot], color_widget_p, area, font.size, font.size, fcolor);
		x_start += gt_font_get_next_word_width(&font, !idx_dot);
        idx_dot += (font.size>>3)*font.size;
    }

    // free ptr
    gt_mem_free(fp);
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
    gt_mem_free(obj->cache.cache_color);
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
                _gt_checkbox_init_widget(obj);

                extern void gt_disp_obj(gt_obj_st * obj);
                gt_disp_obj(obj);
            }
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_DRAW_END:
            GT_LOGD(GT_LOG_TAG_GUI, "end draw");
            gt_mem_free(obj->cache.cache_color);
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
    _gt_style_checkbox_st * _style = checkbox->style;
    _style->text = gt_mem_malloc(sizeof("checkbox"));
    gt_memcpy(_style->text, "checkbox", sizeof("checkbox"));
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
