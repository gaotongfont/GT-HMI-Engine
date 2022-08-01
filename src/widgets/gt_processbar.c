/**
 * @file gt_processbar.c
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-07-22 14:13:10
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_processbar.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_PROCESSBAR
#define MY_CLASS    &gt_processbar_class

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class);
static void _deinit_cb(gt_obj_st * obj,const gt_obj_class_st * class);
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_processbar_class = {
    ._init_cb = _init_cb,
    ._deinit_cb = _deinit_cb,
    ._event_cb = _event_cb,
    .type = OBJ_TYPE,
    .size_instance = (sizeof(gt_obj_st)),
    .size_style = (sizeof(_gt_style_processbar_st))
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _gt_processbar_init_widget(gt_obj_st * processbar) {
    _gt_style_processbar_st * style = processbar->style;
    gt_size_t x, y, w, h;
    
    // set default size
    if( !processbar->area.w || !processbar->area.h ){
        processbar->area.w = 100;
        processbar->area.h = 16;
    };

    x = processbar->area.x;
    y = processbar->area.y;
    w = processbar->area.w;
    h = processbar->area.h;

    gt_attr_rect_st rect_attr = {
        .is_fill = 1,
        .border_width = h/4,
        .fcolor.full = GT_COLOR_GET(style->color_ina),
        .bcolor.full = GT_COLOR_GET(style->bcolor),
        .border_color.full = GT_COLOR_GET(style->bcolor)
    };
    processbar->cache.cache_color = gt_mem_malloc(w * h * sizeof(gt_color_t));
    gt_color_t * color_widget_p = (gt_color_t *)processbar->cache.cache_color;
    gt_area_st area_rect = {
        .w = w, .h = h,
    };

    gt_graph_rect(&rect_attr, &area_rect, color_widget_p);
    gt_attr_circle_st circle_attr = {
        .is_fill = 1,
        .border_width = 0,
        .border_color.full = GT_COLOR_GET(style->color_ina),
        .bcolor.full = GT_COLOR_GET(style->bcolor),
        .fcolor.full = GT_COLOR_GET(style->color_ina),
        .radius = h/4,        
    };
    gt_color_t * color_circle_p = gt_mem_malloc(circle_attr.radius * 2 * circle_attr.radius * 2 * sizeof(gt_color_t) );
    gt_area_st area_circle = {
        .w = circle_attr.radius * 2, .h = circle_attr.radius * 2,
    };
    gt_graph_circle(&circle_attr, &area_circle, color_circle_p);

    // left gray color circle
    gt_area_st area_dst = {
        .x = 0, .y = (h/4), .w = w, .h = h
    };
    gt_area_st area_src = {
        .x = 0, .y = 0, .w = area_circle.w, .h = area_circle.h
    };
    gt_color_area_copy(color_widget_p, area_dst, color_circle_p, area_src, area_src.w/2, area_src.h);    
    
    // right gray color circle
    area_src.x = area_src.w/2;
    area_dst.x = w - area_src.w/2;
    gt_color_area_copy(color_widget_p, area_dst, color_circle_p, area_src, area_src.w/2, area_src.h);

    if( style->pos == style->start ){
        gt_mem_free(color_circle_p);
        return;
    }
    if( style->pos == 0 ){
        goto lab_end;
    }

    // left blue color circle
    GT_COLOR_SET(circle_attr.fcolor, GT_COLOR_GET(style->color_act));
    gt_graph_circle(&circle_attr, &area_circle, color_circle_p);
    area_src.x = 0;
    area_dst.x = 0;
    gt_color_area_copy(color_widget_p, area_dst, color_circle_p, area_src, area_src.w/2, area_src.h);

    // circle finish
    // free ptr
    gt_mem_free(color_circle_p);

    // rect val
    int all_pos = style->end - style->start;
    int w_pos = ((float)style->pos/(float)all_pos) * (float)w;
    w_pos = w_pos - area_src.w/2;
    if( w_pos < 0 ){
        w_pos = 1;
    }
    
    rect_attr.border_width = 0;
    GT_COLOR_SET(rect_attr.fcolor, GT_COLOR_GET(style->color_act));
    gt_color_t * color_rect = gt_mem_malloc( w_pos * area_src.h * sizeof(gt_color_t) );
    area_rect.w = w_pos;
    area_rect.h = area_src.h;
    gt_graph_rect(&rect_attr, &area_rect, color_rect);

    area_src.x = 0;
    area_src.w = w_pos;
    area_dst.x = area_src.h/2;
	if( (w_pos-area_src.h/4) > 0 ){
        gt_color_area_copy(color_widget_p, area_dst, color_rect, area_src, w_pos-area_src.h/4, area_src.h);
    }
    //rect finish
    // free ptr
    gt_mem_free(color_rect);

    // right blue circle
    GT_COLOR_SET(circle_attr.fcolor, GT_COLOR_GET(style->color_act));    
    GT_COLOR_SET(circle_attr.bcolor, GT_COLOR_GET(style->color_ina));
    if( style->pos >= style->end ){
        GT_COLOR_SET(circle_attr.bcolor, GT_COLOR_GET(style->bcolor));
    }
    color_circle_p = gt_mem_malloc( area_circle.w * area_circle.h * sizeof(gt_color_t) );
    gt_graph_circle(&circle_attr, &area_circle, color_circle_p);
    area_src.w = area_circle.w;
    area_src.x = area_src.w/2;
    area_dst.x = w_pos ;
    gt_color_area_copy(color_widget_p, area_dst, color_circle_p, area_src, area_src.w/2, area_src.h);

    // circle finish
    // free ptr
lab_end:
    gt_mem_free(color_circle_p);
}

/**
 * @brief obj init processbar widget cb
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");
    /* code */
    _gt_processbar_init_widget(obj);
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
    gt_point_st * point_click = NULL;
    switch(code) {
        case GT_EVENT_TYPE_DRAW_START:
            GT_LOGD(GT_LOG_TAG_GUI, "start draw");
            if( gt_obj_check_scr(obj) ){
                _gt_processbar_init_widget(obj);

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

            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
}


static void _gt_processbar_init_style(gt_obj_st * processbar)
{
    _gt_style_processbar_st * style = processbar->style;
    GT_COLOR_SET(style->bcolor, 0xFFFF);
    GT_COLOR_SET(style->color_act, 0x0516);
    GT_COLOR_SET(style->color_ina, 0xC619);
    style->start = 0;
    style->end = 100;
    style->pos = 50;
}



/* global functions / API interface -------------------------------------*/

/**
 * @brief create a processbar obj
 *
 * @param parent processbar's parent element
 * @return gt_obj_st* processbar obj
 */
gt_obj_st * gt_processbar_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_processbar_init_style(obj);
    return obj;
}


/**
 * @brief set scorllbar pos
 * 
 * @param processbar scorllbar
 * @param pos pos
 */
void gt_processbar_style_set_pos(gt_obj_st * processbar, gt_size_t pos)
{
    _gt_style_processbar_st * style = (_gt_style_processbar_st *)processbar->style;
    if( pos < style->start ){
        style->pos = style->start;
    }else if( pos > style->end ){
        style->pos = style->end;
    }else{
        style->pos = pos;
    }
    printf("pos:%d\n", pos);
    gt_event_send(processbar, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_processbar_style_set_start_end(gt_obj_st * processbar, gt_size_t start, gt_size_t end)
{
    _gt_style_processbar_st * style = (_gt_style_processbar_st *)processbar->style;
    style->start = start;
    style->end = end;
}

gt_size_t gt_processbar_style_get_pos(gt_obj_st * processbar)
{
    _gt_style_processbar_st * style = (_gt_style_processbar_st *)processbar->style;
    return style->pos;
}

gt_size_t gt_processbar_style_get_start(gt_obj_st * processbar)
{
    _gt_style_processbar_st * style = (_gt_style_processbar_st *)processbar->style;
    return style->start;
}

gt_size_t gt_processbar_style_get_end(gt_obj_st * processbar)
{
    _gt_style_processbar_st * style = (_gt_style_processbar_st *)processbar->style;
    return style->end;
}

gt_size_t gt_processbar_style_get_total(gt_obj_st * processbar)
{
    _gt_style_processbar_st * style = (_gt_style_processbar_st *)processbar->style;
    return ( style->end - style->start);
}
/* end ------------------------------------------------------------------*/
