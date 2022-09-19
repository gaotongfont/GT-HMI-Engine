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
#include "../core/gt_draw.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_PROCESS_BAR
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
        .is_fill        = 1,
        .bg_color       = style->color_ina,
        .border_color   = style->color_ina,
        .radius         = h/2,
        .bg_opa         = GT_OPA_COVER,
    };


    gt_area_st area_base = processbar->area;
    draw_bg(processbar->draw_ctx, &rect_attr, &area_base);

    gt_area_st area_val = area_base;
    int all_pos = style->end - style->start;
    int w_pos = ((float)style->pos/(float)all_pos) * (float)w;
    w_pos = w_pos;
    rect_attr.bg_color = style->color_act,
    area_val.w = w_pos;
    if( w_pos > area_val.h ){
        draw_bg(processbar->draw_ctx, &rect_attr, &area_val);
    }
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
                extern void gt_disp_ref_area(gt_area_st * area);
                gt_disp_ref_area(&obj->area);
            }
            // gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
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


static void _gt_processbar_init_style(gt_obj_st * processbar)
{
    _gt_style_processbar_st * style = NULL;

    processbar->style = gt_mem_malloc(sizeof(_gt_style_processbar_st));
    style = processbar->style;

    gt_memset(style, 0, sizeof(_gt_style_processbar_st));

    style->bg_color = gt_color_hex(0xFFFFFF);
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
