/**
 * @file gt_line.c
 * @author Li
 * @brief
 * @version 0.1
 * @date 2023-01-05 16:28:01
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_line.h"

#if GT_CFG_ENABLE_LINE
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
#define OBJ_TYPE    GT_TYPE_LINE
#define MY_CLASS    &gt_line_class

/* private typedef ------------------------------------------------------*/

typedef struct _gt_line_s {
    gt_obj_st obj;
    gt_attr_line_st line;
}_gt_line_st;


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

static const gt_obj_class_st gt_line_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_line_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
/**
 * @brief obj init line widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    _gt_line_st * widget = (_gt_line_st * )obj;
    gt_attr_line_st * style = &widget->line;
    uint8_t r = (style->line.width + 1) >> 1;
    obj->area.x = style->start.x;
    obj->area.y = style->start.y;

    obj->area.w = r << 1;
    obj->area.h = r << 1;
    if (obj->area.w > GT_SCREEN_WIDTH) {
        obj->area.w = GT_SCREEN_WIDTH;
    }
    if (obj->area.h > GT_SCREEN_HEIGHT) {
        obj->area.h = GT_SCREEN_HEIGHT;
    }

    // gt_attr_rect_st rect_attr;
    // gt_graph_init_rect_attr(&rect_attr);
    // rect_attr.reg.is_fill    = true;
    // rect_attr.border_width   = 0;
    // rect_attr.radius         = r;
    // rect_attr.reg.is_line    = true;
    // rect_attr.color       = gt_color_black();
    // gt_area_st area_base = obj->area;

    // rect_attr.obj = style;
    // draw_bg(obj->draw_ctx, &rect_attr, &area_base);
}

/**
 * @brief obj deinit call back
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj) {
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
    gt_event_type_et code_val = gt_event_get_code(e);
    switch(code_val) {
        case GT_EVENT_TYPE_DRAW_START:
            GT_LOGV(GT_LOG_TAG_GUI, "start draw");
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_DRAW_END:
            GT_LOGV(GT_LOG_TAG_GUI, "end draw");
            break;

        case GT_EVENT_TYPE_UPDATE_VALUE:
            GT_LOGV(GT_LOG_TAG_GUI, "value update");
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
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
            break;

        default:
            break;
    }
}

/* global functions / API interface -------------------------------------*/
/**
 * @brief create a line obj
 *
 * @param parent line's parent element
 * @return gt_obj_st* line obj
 */
gt_obj_st * gt_line_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    _gt_line_st * widget = (_gt_line_st * )obj;
    gt_attr_line_st * style = &widget->line;

    style->line.width = 1;
    style->line.color = gt_color_black();
    return obj;
}

void gt_line_set_color(gt_obj_st * line, gt_color_t color)
{
    if (false == gt_obj_is_type(line, OBJ_TYPE)) {
        return ;
    }
    _gt_line_st * widget = (_gt_line_st * )line;
    gt_attr_line_st * style = &widget->line;
    style->line.color = color;
    gt_event_send(line, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_line_set_start_point(gt_obj_st * line, uint16_t xs, uint16_t ys)
{
    if (false == gt_obj_is_type(line, OBJ_TYPE)) {
        return ;
    }
    _gt_line_st * widget = (_gt_line_st * )line;
    gt_attr_line_st * style = &widget->line;
    style->start.x = xs;
    style->start.y = ys;
    gt_event_send(line, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_line_set_end_point(gt_obj_st * line, uint16_t xe, uint16_t ye)
{
    if (false == gt_obj_is_type(line, OBJ_TYPE)) {
        return ;
    }
    _gt_line_st * widget = (_gt_line_st * )line;
    gt_attr_line_st * style = &widget->line;
    style->end.x = xe;
    style->end.y = ye;
    gt_event_send(line, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

void gt_line_set_line_width(gt_obj_st * line, uint16_t line_width)
{
    if (false == gt_obj_is_type(line, OBJ_TYPE)) {
        return ;
    }
    _gt_line_st * widget = (_gt_line_st * )line;
    gt_attr_line_st * style = &widget->line;
    style->line.width = line_width;
    gt_event_send(line, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

#endif  /** GT_CFG_ENABLE_LINE */
/* end ------------------------------------------------------------------*/
