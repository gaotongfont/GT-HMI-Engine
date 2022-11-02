/**
 * @file gt_scrollbar.c
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-07-21 19:53:32
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_scrollbar.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_SCROLLBAR
#define MY_CLASS    &gt_scrollbar_class

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class);
static void _deinit_cb(gt_obj_st * obj,const gt_obj_class_st * class);
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_scrollbar_class = {
    ._init_cb = _init_cb,
    ._deinit_cb = _deinit_cb,
    ._event_cb = _event_cb,
    .type = OBJ_TYPE,
    .size_instance = (sizeof(gt_obj_st)),
    .size_style = (sizeof(_gt_style_scrollbar_st))
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _gt_scrollbar_init_widget(gt_obj_st * scrollbar) {
    _gt_style_scrollbar_st * style = scrollbar->style;
    gt_size_t w, h;
    
    // set default size
    if( !scrollbar->area.w || !scrollbar->area.h ){
        scrollbar->area.w = 100;
        scrollbar->area.h = 16;
    };

    w = scrollbar->area.w;
    h = scrollbar->area.h;

    gt_attr_rect_st rect_attr = {
        .is_fill = 1,
        .radius = h>>2,
        .bg_color = style->color_ina,
        .border_color = style->bg_color,
        .bg_opa = scrollbar->opa,
    };
    


    /* 1 base shape */
    gt_area_st area_base = scrollbar->area;
    if( style->dir == 0 ){
        area_base.h -= h>>1;
        area_base.y += (h>>1)>>1;
        rect_attr.radius = area_base.h>>1;
    }else{
        area_base.w -= w>>1;
        area_base.x += (w>>1)>>1;
        rect_attr.radius = area_base.w>>1;
    }
    
    draw_bg(scrollbar->draw_ctx, &rect_attr, &area_base);

    /* 2 val shape */
    // rect val
    int all_pos = style->end - style->start;
    int dist = 0;
    gt_area_st area_val = area_base;
    if( style->dir == 0 ){
        dist = ((float)style->pos/(float)all_pos) * (float)w;
        area_val.w = dist;
    }else{
        dist = ((float)style->pos/(float)all_pos) * (float)h;
        area_val.h = dist;
    }
    
    rect_attr.bg_color = style->color_act;
    if( style->dir == 0 ){
        if( dist > (h>>1) ){
            draw_bg(scrollbar->draw_ctx, &rect_attr, &area_val);
        }
    }else{
        if( dist > (w>>1) ){
            draw_bg(scrollbar->draw_ctx, &rect_attr, &area_val);
        }
    }

    /* big big big operate circle */
    gt_area_st area_big_circle = scrollbar->area;
    if( style->dir == 0 ){
        rect_attr.radius = area_big_circle.h>>1;
    }else{
        rect_attr.radius = area_big_circle.w>>1;
    }
    area_big_circle.y = scrollbar->area.y;
    area_big_circle.x = scrollbar->area.x;
    if( style->dir == 0 ){
        area_big_circle.h = h;
        area_big_circle.x += (dist - (h>>1));
    }else{
        area_big_circle.h = w;
        area_big_circle.y += (dist - (w>>1));
    }
    area_big_circle.w = area_big_circle.h;
    if( style->dir == 0 ){
        if( dist > (scrollbar->area.w - area_big_circle.h ) ){
            area_big_circle.x = scrollbar->area.x + (scrollbar->area.w - area_big_circle.h);
        }
    }else{
        if( dist > (scrollbar->area.h - area_big_circle.h ) ){
            area_big_circle.y = scrollbar->area.y + (scrollbar->area.h - area_big_circle.h);
        }
    }
    if( area_big_circle.x < scrollbar->area.x ){
        area_big_circle.x = scrollbar->area.x;
    }
    if( area_big_circle.y < scrollbar->area.y ){
        area_big_circle.y = scrollbar->area.y;
    }
    draw_bg(scrollbar->draw_ctx, &rect_attr, &area_big_circle);
}

/**
 * @brief obj init scrollbar widget cb
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");
    /* code */
    _gt_scrollbar_init_widget(obj);
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
    extern void _gt_disp_refr_append_area(gt_area_st * area);
    gt_event_type_et code = gt_event_get_code(e);
    gt_point_st point_click;
    int _pos;
    float pos;
    switch(code) {
        case GT_EVENT_TYPE_DRAW_START:
            GT_LOGD(GT_LOG_TAG_GUI, "start draw");
            if( gt_obj_check_scr(obj) ){
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

            break;

        case GT_EVENT_TYPE_INPUT_SCROLL:
            GT_LOGD(GT_LOG_TAG_GUI, "scroll");
            pos = gt_scrollbar_style_get_pos(obj);
            if( gt_scrollbar_style_get_dir(obj) == 0 ){
                pos += obj->process_attr.x_scroll;
            }else{
                pos += obj->process_attr.y_scroll;
            }
            gt_scrollbar_style_set_pos(obj, (int)pos);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;
        
        case GT_EVENT_TYPE_INPUT_PROCESS:   /* remove clicking style and process click event */
            GT_LOGD(GT_LOG_TAG_GUI, "process");
            /* code here */
            point_click.x = obj->process_attr.point.x;
            point_click.y = obj->process_attr.point.y;
            if( gt_scrollbar_style_get_dir(obj) == 0 ){
                _pos = point_click.x;
                pos = ((float)_pos)/((float)obj->area.w);
            }else{
                _pos = point_click.y;
                pos = ((float)_pos)/((float)obj->area.h);
            }
            
           
            pos = pos * (float)gt_scrollbar_style_get_total(obj);
            pos += gt_scrollbar_style_get_start(obj);
            gt_scrollbar_style_set_pos(obj, (int)pos);
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


static void _gt_scrollbar_init_style(gt_obj_st * scrollbar)
{
    _gt_style_scrollbar_st * style = NULL;
    
    scrollbar->style = gt_mem_malloc(sizeof(_gt_style_scrollbar_st));
    style = scrollbar->style;

    gt_memset(style,0,sizeof(_gt_style_scrollbar_st));

    style->bg_color =  gt_color_white();
    GT_COLOR_SET(style->color_act, 0x0516);
    GT_COLOR_SET(style->color_ina, 0xC619);
    style->start = 0;
    style->end = 100;
    style->pos = 50;
    style->dir = 0;
    style->step = 1;
}




/* global functions / API interface -------------------------------------*/

/**
 * @brief create a scrollbar obj
 *
 * @param parent scrollbar's parent element
 * @return gt_obj_st* scrollbar obj
 */
gt_obj_st * gt_scrollbar_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_scrollbar_init_style(obj);
    return obj;
}


/**
 * @brief set scorllbar pos
 * 
 * @param scrollbar scorllbar
 * @param pos pos
 */
void gt_scrollbar_style_set_pos(gt_obj_st * scrollbar, gt_size_t pos)
{
    _gt_style_scrollbar_st * style = (_gt_style_scrollbar_st *)scrollbar->style;
    if( pos < style->start ){
        style->pos = style->start;
    }else if( pos > style->end ){
        style->pos = style->end;
    }else{
        style->pos = ( (pos / style->step) * style->step );
    }
    gt_event_send(scrollbar, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_scrollbar_style_set_start_end(gt_obj_st * scrollbar, gt_size_t start, gt_size_t end)
{
    _gt_style_scrollbar_st * style = (_gt_style_scrollbar_st *)scrollbar->style;
    style->start = start;
    style->end = end;
}

gt_size_t gt_scrollbar_style_get_pos(gt_obj_st * scrollbar)
{
    _gt_style_scrollbar_st * style = (_gt_style_scrollbar_st *)scrollbar->style;
    return style->pos;
}

gt_size_t gt_scrollbar_style_get_start(gt_obj_st * scrollbar)
{
    _gt_style_scrollbar_st * style = (_gt_style_scrollbar_st *)scrollbar->style;
    return style->start;
}

gt_size_t gt_scrollbar_style_get_end(gt_obj_st * scrollbar)
{
    _gt_style_scrollbar_st * style = (_gt_style_scrollbar_st *)scrollbar->style;
    return style->end;
}

gt_size_t gt_scrollbar_style_get_total(gt_obj_st * scrollbar)
{
    _gt_style_scrollbar_st * style = (_gt_style_scrollbar_st *)scrollbar->style;
    return ( style->end - style->start);
}

void gt_scrollbar_style_set_color_act(gt_obj_st * scrollbar, gt_color_t color)
{
    _gt_style_scrollbar_st * style = scrollbar->style;
    style->color_act = color;
}
void gt_scrollbar_style_set_color_ina(gt_obj_st * scrollbar, gt_color_t color)
{
    _gt_style_scrollbar_st * style = scrollbar->style;
    style->color_ina = color;
}
void gt_scrollbar_style_set_color_bg(gt_obj_st * scrollbar, gt_color_t color)
{
    _gt_style_scrollbar_st * style = scrollbar->style;
    style->bg_color = color;
}

void gt_scrollbar_style_set_dir(gt_obj_st * scrollbar, uint8_t dir)
{
    _gt_style_scrollbar_st * style = (_gt_style_scrollbar_st *)scrollbar->style;
    style->dir = dir;
}
uint8_t gt_scrollbar_style_get_dir(gt_obj_st * scrollbar)
{
    _gt_style_scrollbar_st * style = (_gt_style_scrollbar_st *)scrollbar->style;
    return ( style->dir);
}

void gt_scrollbar_style_set_step(gt_obj_st * scrollbar, uint8_t step)
{
    _gt_style_scrollbar_st * style = (_gt_style_scrollbar_st *)scrollbar->style;
    style->step = step;
}

uint8_t gt_scrollbar_style_get_step(gt_obj_st * scrollbar)
{
    _gt_style_scrollbar_st * style = (_gt_style_scrollbar_st *)scrollbar->style;
    return ( style->step);
}

/* end ------------------------------------------------------------------*/
