/**
 * @file gt_switch.c
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-07-21 14:34:25
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_switch.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_SWITCH
#define MY_CLASS    &gt_switcher_class

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class);
static void _deinit_cb(gt_obj_st * obj,const gt_obj_class_st * class);
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_switcher_class = {
    ._init_cb = _init_cb,
    ._deinit_cb = _deinit_cb,
    ._event_cb = _event_cb,
    .type = OBJ_TYPE,
    .size_instance = (sizeof(gt_obj_st)),
    .size_style = (sizeof(_gt_style_switcher_st))
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _gt_switcher_init_widget(gt_obj_st * switcher)
{
    _gt_style_switcher_st * style = switcher->style;
    gt_size_t x, y, w, h;           //base attr
    gt_color_t fcolor;
    // set default size
    if( switcher->area.w == 0 || switcher->area.h == 0){
        switcher->area.w = 48;
        switcher->area.h = 24;
    }

    x = switcher->area.x;
    y = switcher->area.y;
    w = switcher->area.w;
    h = switcher->area.h;    

    switcher->cache.cache_color = gt_mem_malloc( w * h * sizeof(gt_color_t) );
    if( style->state ){
        GT_COLOR_SET(fcolor, GT_COLOR_GET(style->color_open));
    }else{
        GT_COLOR_SET(fcolor, GT_COLOR_GET(style->color_close));
    }

    /* base shape */
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    GT_COLOR_SET(rect_attr.fcolor, GT_COLOR_GET(fcolor));
    rect_attr.is_fill = 1;    
    GT_COLOR_SET(rect_attr.bcolor, 0xFFFF);

    // out size color
    rect_attr.border_width = 0;
    gt_color_t color_out,color_from,color_to;
    GT_COLOR_SET(color_from, GT_COLOR_GET(fcolor));
    GT_COLOR_SET(color_to, 0xFFFF);

    gt_area_st area = {
        .w = w, .h = h, .x = x, .y = y
    };
	
    gt_color_t * color_widget_p = (gt_color_t *)switcher->cache.cache_color;
    gt_graph_rect(&rect_attr, &area, color_widget_p);


    gt_attr_circle_st circle_attr = {
        .is_fill = 1,
        .fcolor.full = GT_COLOR_GET(fcolor),
        .bcolor.full = 0xFFFF,
		.border_color.full = gt_graph_get_gray(&color_out, color_from, color_to, 4,2),
        .border_width = 0,        
        .radius = h/2,
    };
    gt_area_st area_circle = {
        .x = 0, .y = 0, .w = h, .h = h
    };
    gt_color_t * color_circle_p = gt_mem_malloc(h * h * sizeof(gt_color_t));
    gt_memset(color_circle_p, 0xFF, h * h * sizeof(gt_color_t));
    gt_graph_circle(&circle_attr, &area_circle, color_circle_p);
    
    gt_area_st _area_dst = {
        .x = 0, .y = 0, .w = w, .h = h
    }, _area_src = {
        .x = 0, .y = 0, .w = h, .h = h
    };
    gt_color_area_copy(color_widget_p, _area_dst, color_circle_p, _area_src, h/2, h);

    _area_src.x = h/2;
    _area_dst.x = w - (h/2);
    gt_color_area_copy(color_widget_p, _area_dst, color_circle_p, _area_src, h/2, h);
    gt_mem_free(color_circle_p);

    // switch icon
    area_circle.w = h - 8;
    area_circle.h = h - 8;
    circle_attr.radius = (h - 8)>>1;
    circle_attr.is_fill = 1;
    circle_attr.fcolor.full = 0xFFFF;
    circle_attr.bcolor.full = GT_COLOR_GET(fcolor);
    circle_attr.border_width = 0;
    color_circle_p = gt_mem_malloc( area_circle.w * area_circle.w * sizeof(gt_color_t) );
    gt_graph_circle(&circle_attr, &area_circle, color_circle_p);

    if( style->state ){
        _area_dst.x = w - (h-8) -4;
        _area_dst.y = (h - (h-8))>>1;
    }else{
        _area_dst.x = 4;
        _area_dst.y = (h - (h-8))>>1;
    }
    
    _area_src.x = 0;
    _area_src.y = 0;
    _area_src.w = (h-8);
    _area_src.h = (h-8);
    gt_color_area_copy(color_widget_p, _area_dst, color_circle_p, _area_src, (h-8), (h-8));
    gt_mem_free(color_circle_p);
}

/**
 * @brief obj init switcher widget cb
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");
    /* code */
    _gt_switcher_init_widget(obj);
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
                _gt_switcher_init_widget(obj);
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
            if( gt_switch_style_get_state(obj) ){
                gt_switch_style_set_state(obj, 0);
            }else{
                gt_switch_style_set_state(obj, 1);
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


static void _gt_switcher_init_style(gt_obj_st * switcher)
{
    _gt_style_switcher_st * style = switcher->style;
    GT_COLOR_SET(style->color_close, 0xC619);
    GT_COLOR_SET(style->color_open, 0x0516);
    gt_switch_style_set_state(switcher, 0);
}




/* global functions / API interface -------------------------------------*/

/**
 * @brief create a switcher obj
 *
 * @param parent switcher's parent element
 * @return gt_obj_st* switcher obj
 */
gt_obj_st * gt_switch_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_switcher_init_style(obj);
    return obj;
}

void gt_switch_style_set_state(gt_obj_st * switcher, bool state){
    _gt_style_switcher_st * style = switcher->style;
    style->state = state;
}

bool gt_switch_style_get_state(gt_obj_st * switcher){
    _gt_style_switcher_st * style = switcher->style;
    return style->state;
}


/* end ------------------------------------------------------------------*/
