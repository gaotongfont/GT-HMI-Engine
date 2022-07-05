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
    gt_size_t radius,border_w;      //widget attr
    gt_attr_line_st _line;
    gt_area_st _area;
    gt_color_t fcolor;

    x = btn->area.x;
    y = btn->area.y;
    w = btn->area.w;
    h = btn->area.h;

    radius = style->radius;
    border_w = style->border_width;

    btn->cache.cache_color = gt_mem_malloc( w * h * sizeof(gt_color_t) );
    if( style->selected ){
        GT_COLOR_SET(fcolor, GT_COLOR_GET(style->color_selected));
    }else{
        GT_COLOR_SET(fcolor, GT_COLOR_GET(style->color_unselected));
    }

    gt_attr_rect_st rect_attr;
    GT_COLOR_SET(rect_attr.fcolor, GT_COLOR_GET(fcolor));
    rect_attr.is_fill = 1;
    GT_COLOR_SET(rect_attr.bcolor, GT_COLOR_GET(fcolor));

    gt_area_st area = {
        .w = w,
        .h = h,
        .x = x,
        .y = y
    };

    if(radius == 0) {
        gt_color_t * color_p = btn->cache.cache_color;
        gt_graph_rect(&rect_attr, &area, color_p);
        // extern void _flush_cb(struct _gt_disp_drv_s * drv, gt_area_st * area, gt_color_t * color);
        // _flush_cb(NULL, &area, color_p);
    }else{

    }
	gt_mem_free(btn->cache.cache_color);
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
            GT_LOGV(GT_LOG_TAG_GUI, "start draw");
            _gt_btn_init_widget(obj);
            break;

        case GT_EVENT_TYPE_DRAW_END:
            GT_LOGV(GT_LOG_TAG_GUI, "end draw");
            break;

        case GT_EVENT_TYPE_CHANGE_CHILD_REMOVE: /* remove child from screen but not delete */
            GT_LOGV(GT_LOG_TAG_GUI, "child remove");
			break;

        case GT_EVENT_TYPE_CHANGE_CHILD_DELETE: /* delete child */
            GT_LOGV(GT_LOG_TAG_GUI, "child delete");
            break;

        case GT_EVENT_TYPE_INPUT_CLICKING:   /* add clicking style and process clicking event */
            GT_LOGV(GT_LOG_TAG_GUI, "clicking");
            gt_btn_style_set_selected(obj, 1);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_SCROLL:
            GT_LOGV(GT_LOG_TAG_GUI, "scroll");
            break;

        case GT_EVENT_TYPE_INPUT_PROCESS:   /* remove clicking style and process click event */
            GT_LOGV(GT_LOG_TAG_GUI, "process");
            /* code here */

            gt_event_send(obj, GT_EVENT_TYPE_INPUT_PROCESSED, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_PROCESSED: /* click event finish */
            GT_LOGV(GT_LOG_TAG_GUI, "processed");
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
    GT_COLOR_SET(_style->color_selected, 0xF800);    //default color_selected
    GT_COLOR_SET(_style->color_unselected, 0x1537);  //default color_unselected
}




/* global functions / API interface -------------------------------------*/

/**
 * @brief create a btn obj
 *
 * @param parent btn's parent element
 * @return gt_obj_st* btn obj
 */
gt_obj_st * gt_btn_create(gt_obj_st * parent) {
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_btn_init_style(obj);
    return obj;
}

void gt_btn_style_set_color_selected(gt_obj_st * btn, gt_color_t color) {
    _gt_style_btn_st * style = btn->style;
    GT_COLOR_SET(style->color_selected, GT_COLOR_GET(color));
}

void gt_btn_style_set_color_unselected(gt_obj_st * btn, gt_color_t color) {
    _gt_style_btn_st * style = btn->style;
    GT_COLOR_SET(style->color_unselected, GT_COLOR_GET(color));
}

void gt_btn_style_set_selected(gt_obj_st * btn, uint8_t selected) {
    _gt_style_btn_st * style = btn->style;
    style->selected = selected!=0?1:0;
}

void gt_btn_style_set_text(gt_obj_st * btn, char *text) {
    _gt_style_btn_st * style = btn->style;
    gt_mem_realloc(style->text, strlen(text));
    gt_memcpy(style->text, text, strlen(text));
}

/* end ------------------------------------------------------------------*/
