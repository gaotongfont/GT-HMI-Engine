/**
 * @file gt_label.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-11 15:03:35
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_label.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_LAB
#define MY_CLASS    &gt_label_class

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class);
static void _deinit_cb(gt_obj_st * obj,const gt_obj_class_st * class);
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_label_class = {
    ._init_cb = _init_cb,
    ._deinit_cb = _deinit_cb,
    ._event_cb = _event_cb,
    .type = OBJ_TYPE,
    .size_instance = (sizeof(gt_obj_st)),
    .size_style = (sizeof(_gt_style_label_st))
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _gt_label_init_widget(gt_obj_st * label) {
    _gt_style_label_st * style = label->style;
    gt_size_t x, y, w, h;           //base attr
    
    if( 0 == label->area.w || 0 == label->area.h ){
        label->area.h = 20;
        gt_font_st font = {
            .utf8 = (uint8_t *)style->text,
            .len = strlen(style->text),
            .size = 16
        };
        label->area.w =  gt_font_get_string_width(&font) + 20;
    }

    x = label->area.x;
    y = label->area.y;
    w = label->area.w;
    h = label->area.h;

    label->cache.cache_color = gt_mem_malloc( w * h * sizeof(gt_color_t) );
    gt_color_t * color_widget_p = (gt_color_t *)label->cache.cache_color;
    gt_area_st area = {
        .w = w,
        .h = h,
        .x = x,
        .y = y
    };
    /* base shape */
    // set bcolor
    gt_memset(label->cache.cache_color, 0xFF, w * h * sizeof(gt_color_t));
   
    /* font */
    uint32_t cnt_word = gt_font_count_word(style->text, strlen(style->text));
    uint8_t fp[1024];
    gt_memset(fp, 0, 1024);
    gt_font_st font = {
        .style_cn = 1,
        .style_en = 2,
        .res = fp,
        .utf8 = (uint8_t *)style->text,
        .len = strlen(style->text),
        .size = GT_FONT_SIZE_16x16,
    };
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

    while( cnt_word-- && (x_start+font.size) < w ){
        area.x = x_start;
        gt_graph_dot_draw_to_color(&font.res[idx_dot], color_widget_p, area, font.size, font.size, style->color_font);
        x_start += gt_font_get_next_word_width(&font, !idx_dot);
        idx_dot += (font.size>>3)*font.size;
    }
}

/**
 * @brief obj init label widget cb
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");
    /* code */
    _gt_label_init_widget(obj);
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
            if( 0 && gt_obj_check_scr(obj) ){
                _gt_label_init_widget(obj);

                extern void gt_disp_obj(gt_obj_st * obj);
                gt_disp_obj(obj);
            }
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_DRAW_END:
            GT_LOGD(GT_LOG_TAG_GUI, "end draw");
            gt_mem_free(obj->cache.cache_color);
            break;

        case GT_EVENT_TYPE_UPDATE_VALUE:
            GT_LOGD(GT_LOG_TAG_GUI, "value update");
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_CHANGE_CHILD_REMOVE: /* remove child from screen but not delete */
            GT_LOGD(GT_LOG_TAG_GUI, "child remove");
			break;

        case GT_EVENT_TYPE_CHANGE_CHILD_DELETE: /* delete child */
            GT_LOGD(GT_LOG_TAG_GUI, "child delete");
            break;

        case GT_EVENT_TYPE_INPUT_CLICKING:   /* add clicking style and process clicking event */
            GT_LOGD(GT_LOG_TAG_GUI, "clicking");
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
            
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
}


static void _gt_label_init_style(gt_obj_st * label)
{
    _gt_style_label_st * _style = label->style;
    
    _style->text = gt_mem_malloc(sizeof("label"));
    strcpy(_style->text, "label");
    
    GT_COLOR_SET(_style->color_font, 0x0000);
}




/* global functions / API interface -------------------------------------*/

/**
 * @brief create a label obj
 *
 * @param parent label's parent element
 * @return gt_obj_st* label obj
 */
gt_obj_st * gt_label_create(gt_obj_st * parent) {
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_label_init_style(obj);
    return obj;
}


void gt_label_style_set_text(gt_obj_st * label, char * text)
{
    _gt_style_label_st * style = label->style;
    if( NULL != style->text ){
        gt_mem_free(style->text);
    }
    uint16_t size = strlen(text);
    style->text = gt_mem_malloc(size+1);
    gt_memset(style->text, 0, size+1);
    gt_memcpy(style->text, text, size);
    gt_event_send(label, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

char * gt_label_style_get_text(gt_obj_st * label){
    GT_ASSERT_NULL(label);
    _gt_style_label_st * style = label->style;
    return style->text;
}

void gt_label_style_set_font_color(gt_obj_st * label, gt_color_t color){
    _gt_style_label_st * style = label->style;    
    GT_COLOR_SET(style->color_font, GT_COLOR_GET(color));
}
/* end ------------------------------------------------------------------*/
