/**
 * @file gt_rect.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-11-02 09:50:01
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

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
#include "../core/gt_disp.h"
/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_RECT
#define MY_CLASS    &gt_rect_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_rect_s {
    gt_color_t  color_background;
    gt_color_t  color_border;
    uint16_t    radius;
    uint16_t    border;
    uint8_t     fill;
}_gt_rect_st;


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_rect_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_rect_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static inline void _gt_rect_init_widget(gt_obj_st * rect) {
    _gt_rect_st * style = rect->style;

    if( rect->area.w == 0 || rect->area.h == 0){
        return;
    }
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill    = style->fill;
    rect_attr.border_width   = style->border;
    rect_attr.border_color   = style->color_border;
    rect_attr.bg_opa         = rect->opa;
    rect_attr.bg_color       = style->color_background;
    rect_attr.radius         = style->radius;

    gt_area_st area_base = rect->area;
    draw_bg(rect->draw_ctx, &rect_attr, &area_base);

    // focus
    draw_focus(rect , 0);
}

/**
 * @brief obj init rect widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_rect_init_widget(obj);
}

/**
 * @brief obj deinit call back
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");
    if (NULL == obj) {
        return ;
    }

}


/**
 * @brief obj event handler call back
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);
    switch(code) {
        case GT_EVENT_TYPE_DRAW_START:
            GT_LOGV(GT_LOG_TAG_GUI, "start draw");
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
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

        case GT_EVENT_TYPE_INPUT_PRESSING:   /* add clicking style and process clicking event */
            GT_LOGV(GT_LOG_TAG_GUI, "clicking");
            break;

        case GT_EVENT_TYPE_INPUT_SCROLL:
            GT_LOGV(GT_LOG_TAG_GUI, "scroll");
            break;

        case GT_EVENT_TYPE_INPUT_RELEASED: /* click event finish */
            GT_LOGV(GT_LOG_TAG_GUI, "processed");
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
}


static void _gt_rect_init_style(gt_obj_st * rect)
{
    _gt_rect_st * style = (_gt_rect_st * )rect->style;

    gt_memset(style,0,sizeof(_gt_rect_st));

    style->fill     = 1;
    style->radius   = 0;
    style->color_background = gt_color_black();
    style->color_border     = gt_color_black();
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
    if (NULL == obj) {
        return NULL;
    }
    _gt_rect_init_style(obj);
    return obj;
}
void gt_rect_set_bg_color(gt_obj_st * rect, gt_color_t color)
{
    _gt_rect_st * style = (_gt_rect_st * )rect->style;
    style->color_background = color;
    gt_event_send(rect, GT_EVENT_TYPE_DRAW_START, NULL);
}
void gt_rect_set_color_border(gt_obj_st * rect, gt_color_t color)
{
    _gt_rect_st * style = (_gt_rect_st * )rect->style;
    style->color_border = color;
    gt_event_send(rect, GT_EVENT_TYPE_DRAW_START, NULL);
}
void gt_rect_set_radius(gt_obj_st * rect, uint16_t radius)
{
    _gt_rect_st * style = (_gt_rect_st * )rect->style;
    style->radius = radius;
    gt_event_send(rect, GT_EVENT_TYPE_DRAW_START, NULL);
}
void gt_rect_set_border(gt_obj_st * rect, uint16_t border)
{
    _gt_rect_st * style = (_gt_rect_st * )rect->style;
    style->border = border;
    gt_event_send(rect, GT_EVENT_TYPE_DRAW_START, NULL);
}
void gt_rect_set_fill(gt_obj_st * rect, uint8_t fill)
{
    _gt_rect_st * style = (_gt_rect_st * )rect->style;
    style->fill = fill;
    gt_event_send(rect, GT_EVENT_TYPE_DRAW_START, NULL);
}

/* end ------------------------------------------------------------------*/
