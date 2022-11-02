/**
 * @file gt_rect.c
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-11-02 09:50:01
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
/* include --------------------------------------------------------------*/
#include "gt_rect.h"
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
#define MY_CLASS    &gt_rect_class

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class);
static void _deinit_cb(gt_obj_st * obj,const gt_obj_class_st * class);
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_rect_class = {
    ._init_cb = _init_cb,
    ._deinit_cb = _deinit_cb,
    ._event_cb = _event_cb,
    .type = OBJ_TYPE,
    .size_instance = (sizeof(gt_obj_st)),
    .size_style = (sizeof(_gt_style_rect_st))
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _gt_rect_init_widget(gt_obj_st * rect) {
    _gt_style_rect_st * style = rect->style;

    if( rect->area.w == 0 || rect->area.h == 0){
        return;
    }
    gt_attr_rect_st rect_attr = {
        .is_fill        = style->is_full,
        .border_width   = style->border,
        .border_color   = style->color_bd,
        .bg_opa         = rect->opa,
        .bg_color       = style->color_bg,
        .radius         = style->radius,
    };

    gt_area_st area_base = rect->area;
    draw_bg(rect->draw_ctx, &rect_attr, &area_base);
}

/**
 * @brief obj init rect widget cb
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");
    /* code */
    _gt_rect_init_widget(obj);
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


static void _gt_rect_init_style(gt_obj_st * rect)
{
    _gt_style_rect_st * style = NULL;

    rect->style = gt_mem_malloc(sizeof(_gt_style_rect_st));
    style = rect->style;

    gt_memset(style,0,sizeof(_gt_style_rect_st));

    style->is_full = 1;
    style->radius = 0;
    style->color_bg = gt_color_black();
    style->color_bd = gt_color_black();
}




/* global functions / API interface -------------------------------------*/

/**
 * @brief create a rect obj
 *
 * @param parent rect's parent element
 * @return gt_obj_st* rect obj
 */
gt_obj_st * gt_rect_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_rect_init_style(obj);
    return obj;
}
void gt_rect_style_set_bg_color(gt_obj_st * rect, gt_color_t color)
{
    _gt_style_rect_st * style = rect->style;
    style->color_bg = color;
}
void gt_rect_style_set_border_color(gt_obj_st * rect, gt_color_t color)
{
    _gt_style_rect_st * style = rect->style;
    style->color_bd = color;
}
void gt_rect_style_set_radius(gt_obj_st * rect, uint16_t radius)
{
    _gt_style_rect_st * style = rect->style;
    style->radius = radius;
}
void gt_rect_style_set_border(gt_obj_st * rect, uint16_t border)
{
    _gt_style_rect_st * style = rect->style;
    style->border = border;
}
void gt_rect_style_set_full(gt_obj_st * rect, uint8_t if_full)
{
    _gt_style_rect_st * style = rect->style;
    style->is_full = if_full;
}

/* end ------------------------------------------------------------------*/
