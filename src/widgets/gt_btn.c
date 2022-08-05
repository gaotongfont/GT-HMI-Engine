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
    
    gt_color_t fcolor;
    uint8_t fp[1024];
    gt_font_st font = {
        .style_cn = 1,
        .style_en = 2,
        .res = fp,
        .utf8 = (uint8_t *)style->text,
        .len = strlen(style->text),
        .size = GT_FONT_SIZE_16x16,
    };
    // set default size
    if( btn->area.w == 0 || btn->area.h == 0){
        btn->area.w = gt_font_get_string_width(&font)+32;
        btn->area.h = GT_FONT_SIZE_16x16+16;
    }

    x = btn->area.x;
    y = btn->area.y;
    w = btn->area.w;
    h = btn->area.h;

    radius = h/4;//style->radius;
    btn->cache.cache_color = gt_mem_malloc( w * h * sizeof(gt_color_t) );
    if( style->selected ){
        GT_COLOR_SET(fcolor, GT_COLOR_GET(style->color_selected));
    }else{
        GT_COLOR_SET(fcolor, GT_COLOR_GET(style->color_unselected));
    }

    /* base shape */
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    GT_COLOR_SET(rect_attr.fcolor, GT_COLOR_GET(fcolor));
    rect_attr.is_fill = 1;
    rect_attr.radius = radius;
    GT_COLOR_SET(rect_attr.bcolor, 0xFFFF);

    gt_area_st area = {
        .w = w,
        .h = h,
        .x = x,
        .y = y
    };
	
    gt_color_t * color_widget_p = (gt_color_t *)btn->cache.cache_color;

    gt_attr_circle_st circle_attr = {
        .is_fill = 1,
        .fcolor.full = GT_COLOR_GET(fcolor),
        .bcolor.full = 0xFFFF,
		.border_color.full = GT_COLOR_GET(fcolor),
        .border_width = 1,
        .radius = h/4,
    };

    gt_graph_rect(&rect_attr, &area, color_widget_p);
    // if(0 && radius == 0) {
    //     gt_graph_rect(&rect_attr, &area, color_widget_p);
    // }else{
    //     gt_graph_rect(&rect_attr, &area, color_widget_p);

    //     gt_area_st area_circle = {
    //         .x = 0, .y = 0, .w = h, .h = h
    //     };
    //     gt_color_t * color_circle = gt_mem_malloc(h * h * sizeof(gt_color_t));
    //     gt_memset(color_circle, 0xFF, h * h * sizeof(gt_color_t));
    //     gt_graph_circle(&circle_attr, &area_circle, color_circle);
        
    //     gt_area_st _area_dst = {
    //         .x = 0, .y = 0, .w = w, .h = h,
    //     }, _area_src = {
    //         .x = 0, .y = 0, .w = h, .h = h
    //     };
    //     gt_color_area_copy(color_widget_p, _area_dst, color_circle, _area_src, h/2, h);

    //     _area_src.x = h/2;
    //     _area_dst.x = w - (h/2);
    //     gt_color_area_copy(color_widget_p, _area_dst, color_circle, _area_src, h/2, h);
	//     gt_mem_free(color_circle);
    // }




    /* font */
    uint32_t cnt_word = gt_font_count_word(style->text, strlen(style->text));
    gt_memset(fp, 0, 1024);
    
    uint32_t w_font = gt_font_get_string_width(&font);
    gt_font_get_string_dot(&font);

    int16_t x_start = (w - w_font)>>1;
    int16_t y_start = (h - GT_FONT_SIZE_16x16)>>1;

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
    
    // need to refactor here, to func
    while( cnt_word-- && (x_start+font.size) < w ){
        area.x = x_start;
        gt_graph_dot_draw_to_color(&font.res[idx_dot], color_widget_p, area, font.size, font.size, style->color_font);
        x_start += gt_font_get_next_word_width(&font, !idx_dot);
        idx_dot += (font.size>>3)*font.size;
    }
    
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
                _gt_btn_init_widget(obj);
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
    _gt_style_btn_st * _style = btn->style;
    
    _style->text = gt_mem_malloc(sizeof("btn"));
    strcpy(_style->text, "btn");
    _style->radius = 0;
	_style->selected = 0;
    GT_COLOR_SET(_style->color_font, 0xFFFF);        //default color_font
    GT_COLOR_SET(_style->color_selected, 0x1C1A);    //default color_selected
    GT_COLOR_SET(_style->color_unselected, 0x1537);  //default color_unselected
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
