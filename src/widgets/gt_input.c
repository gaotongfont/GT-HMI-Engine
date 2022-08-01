/**
 * @file gt_input.c
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-07-20 13:37:26
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_input.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_INPUT
#define MY_CLASS    &gt_input_class

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class);
static void _deinit_cb(gt_obj_st * obj,const gt_obj_class_st * class);
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_input_class = {
    ._init_cb = _init_cb,
    ._deinit_cb = _deinit_cb,
    ._event_cb = _event_cb,
    .type = OBJ_TYPE,
    .size_instance = (sizeof(gt_obj_st)),
    .size_style = (sizeof(_gt_style_input_st))
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _gt_input_init_widget(gt_obj_st * input) {
    _gt_style_input_st * style = input->style;
    gt_size_t x, y, w, h;           //base attr

    if( 0 == input->area.w || 0 == input->area.h ){
        input->area.w = 128;
        input->area.h = 24;
    }

    w = input->area.w;
    h = input->area.h;

    if( input->cache.cache_color ){
        gt_mem_free(input->cache.cache_color);
    }
    input->cache.cache_color = gt_mem_malloc( w * h * sizeof(gt_color_t) );
    
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.is_fill = 1;
    GT_COLOR_SET(rect_attr.fcolor, 0xFFFF);
    GT_COLOR_SET(rect_attr.bcolor, 0xFFFF);
    rect_attr.border_width = 2;
    GT_COLOR_SET(rect_attr.border_color, 0x0000);
    gt_color_t * color_widget_p = (gt_color_t *)input->cache.cache_color;
    gt_graph_rect(&rect_attr, &input->area, color_widget_p);

    /* font */
    
    uint16_t cnt_word = 0;
    uint16_t x_start = 2, y_start = (h - GT_FONT_SIZE_16x16)>>1;
    // if input value is null
    gt_font_st font;
    uint8_t fp[1024];
    gt_memset(fp, 0, 1024);
    gt_color_t color_font;
    if( !style->value ){
        font.style_cn = 1;
        font.style_en = 2;
        font.utf8 = style->placeholder;
        font.len = strlen(style->placeholder);
        font.size = GT_FONT_SIZE_16x16;
        font.res = fp;
        
        color_font.full = 0x8410;
    }
    // if input value is not null
    else{
        font.style_cn = 1;
        font.style_en = 2;
        font.utf8 = style->value;
        font.len = strlen(style->value);
        font.size = GT_FONT_SIZE_16x16;
        font.res = fp;
        color_font.full = 0x0000;

    }
    
    cnt_word = gt_font_count_word(font.utf8, strlen(font.utf8));
    gt_font_get_string_dot(&font);
    gt_area_st area = {
        .w = w,
        .h = h,
        .x = x_start,
        .y = y_start
    };
    uint16_t idx_dot = 0;
    
    while( cnt_word-- && (x_start+font.size) < w ){
        area.x = x_start;
        gt_graph_dot_draw_to_color(&font.res[idx_dot], color_widget_p, area, font.size, font.size, color_font);
        x_start += gt_font_get_next_word_width(&font, !idx_dot);
        idx_dot += (font.size>>3)*font.size;
    }
}

/**
 * @brief obj init input widget cb
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");
    /* code */
    _gt_input_init_widget(obj);
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
                _gt_input_init_widget(obj);

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

            gt_event_send(obj, GT_EVENT_TYPE_INPUT_PROCESSED, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_PROCESSED: /* click event finish */
            GT_LOGD(GT_LOG_TAG_GUI, "processed");
            break;

        default:
            break;
    }
}


static void _gt_input_init_style(gt_obj_st * input)
{
    _gt_style_input_st * style = input->style;
    gt_memset(style, 0, sizeof(_gt_style_input_st));
    gt_input_style_set_placeholder(input, "please input");
}




/* global functions / API interface -------------------------------------*/

/**
 * @brief create a input obj
 *
 * @param parent input's parent element
 * @return gt_obj_st* input obj
 */
gt_obj_st * gt_input_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_input_init_style(obj);
    return obj;
}

/**
 * @brief set input value
 * 
 * @param input input obj
 * @param value set data value
 */
void gt_input_style_set_value(gt_obj_st * input, char * value)
{
    _gt_style_input_st * style = input->style;
    if( style->placeholder ){
        gt_mem_free(style->value);
    }
    uint16_t len = strlen(value);
    style->value = gt_mem_malloc( len + 1 );
    gt_memcpy(style->value, value, len);
    style->value[len] = 0;
}

/**
 * @brief append value to input obj value
 * 
 * @param input input obj
 * @param value append data value
 */
void gt_input_style_append_value(gt_obj_st * input, char * value)
{
    _gt_style_input_st * style = input->style;
    uint16_t len = strlen(value);
    if( !style->value ){
        style->value = gt_mem_malloc(len+1);
        gt_memcpy(style->value, value, len);
        style->value[len] = 0;
    }else{
        uint16_t len_src = strlen(style->value);
        style->value = gt_mem_realloc(style->value, len + len_src + 1);
        gt_memcpy(&style->value[len_src], value, len);
        style->value[len + len_src] = 0;
    }
}

/**
 * @brief set placeholder value
 * 
 * @param input input obj
 * @param placeholder placeholder value
 */
void gt_input_style_set_placeholder(gt_obj_st * input, const char * placeholder)
{
    _gt_style_input_st * style = input->style;
    if( style->placeholder ){
        gt_mem_free(style->placeholder);
    }
    uint16_t len = strlen(placeholder);
    style->placeholder = gt_mem_malloc( len + 1 );
    gt_memcpy(style->placeholder, placeholder, len);
    style->placeholder[len] = 0;
}

/* end ------------------------------------------------------------------*/
