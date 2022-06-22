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
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");
    /* code */
    
    GT_LOGV(GT_LOG_TAG_GUI, "end init_cb");
}
static void _deinit_cb(gt_obj_st * obj, const gt_obj_class_st * class) {
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");
    /* code */

    GT_LOGV(GT_LOG_TAG_GUI, "end deinit_cb");
}

static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);
    switch(code) {
        case GT_EVENT_TYPE_DRAW_START:
            GT_LOGV(GT_LOG_TAG_GUI, "start draw");
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
            break;

        case GT_EVENT_TYPE_INPUT_PROCESS:   /* remove clicking style and process click event */
            GT_LOGV(GT_LOG_TAG_GUI, "process");
            break;
        case GT_EVENT_TYPE_INPUT_PROCESSED: /*  */
            GT_LOGV(GT_LOG_TAG_GUI, "processed");
            break;
        default:
            break;
    }
}


static void _gt_btn_init_style(gt_obj_st * btn, _gt_style_btn_st * style_btn) {
    GT_COLOR_SET(style_btn->color_selected, 0x1537);    //default color_selected
    GT_COLOR_SET(style_btn->color_unselected, 0xFFFF);  //default color_unselected
    style_btn->text = gt_mem_malloc(sizeof("btn"));
    gt_memcpy(style_btn->text, "btn", sizeof("btn"));    
    style_btn->selected = 0;
}

static void _gt_btn_init_widget(gt_obj_st * btn) {
    _gt_style_btn_st * style = (_gt_style_btn_st *)&btn->style;
    gt_size_t x, y, w, h;   //base attr
    gt_size_t radius,border_w;   //widget attr
    gt_attr_line_st _line;
    gt_area_st _area;

    x = btn->area.x;
    y = btn->area.y;
    w = btn->area.w;
    h = btn->area.h;

    radius = style->radius;
    border_w = style->border_width;

    if(radius == 0) {

    }else{

    }
}


/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_btn_create(gt_obj_st * parent) {
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_btn_init_style(obj, (_gt_style_btn_st *)&obj->style);
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
