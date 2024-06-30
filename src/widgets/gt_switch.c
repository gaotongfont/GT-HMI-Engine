/**
 * @file gt_switch.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-21 14:34:25
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_switch.h"

#if GT_CFG_ENABLE_SWITCH
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
#define OBJ_TYPE    GT_TYPE_SWITCH
#define MY_CLASS    &gt_switcher_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_switcher_s {
    gt_obj_st obj;
    gt_color_t color_act;
    gt_color_t color_ina;
    gt_color_t color_point;

    bool state;             //switcher state ->true:open,false:close
}_gt_switcher_st;


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_switcher_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_switcher_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static inline void _gt_switcher_init_widget(gt_obj_st * switcher)
{
    _gt_switcher_st * style = (_gt_switcher_st * )switcher;
    gt_color_t fg_color;
    // set default size
    if( switcher->area.w == 0 || switcher->area.h == 0){
        switcher->area.w = 48;
        switcher->area.h = 24;
    }

    if( gt_obj_get_state(switcher) != GT_STATE_NONE ){
        fg_color = style->color_act;
    }else{
        fg_color = style->color_ina;
    }

    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill   = 1;
    rect_attr.bg_color      = fg_color;
    rect_attr.border_color  = fg_color;
    rect_attr.radius        = switcher->area.h>>1;
    rect_attr.border_width  = 0;
    rect_attr.bg_opa        = switcher->opa;

    gt_area_st box_area = gt_area_reduce(switcher->area , gt_obj_get_reduce(switcher));
    draw_bg(switcher->draw_ctx, &rect_attr, &box_area);

    // focus
    draw_focus(switcher , rect_attr.radius);

    /* circle icon */
    gt_area_st area_circle = switcher->area;
    area_circle.y += 4;
    area_circle.h -= 8;
    area_circle.w = area_circle.h;
    if( gt_obj_get_state(switcher) != GT_STATE_NONE ){
        area_circle.x += (switcher->area.w - area_circle.w - 4);
    }else{
        area_circle.x += 4;
    }
    rect_attr.radius        = area_circle.h>>1;
    rect_attr.border_width  = 0;
    rect_attr.bg_color      = style->color_point;
    draw_bg(switcher->draw_ctx, &rect_attr, &area_circle);
}

/**
 * @brief obj init switcher widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_switcher_init_widget(obj);
}

/**
 * @brief obj deinit call back
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj) {
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
            if( gt_obj_get_state(obj) ){
                gt_obj_set_state(obj, GT_STATE_NONE);
            }else{
                gt_obj_set_state(obj, GT_STATE_PRESSED);
            }
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
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
    GT_CHECK_BACK_VAL(obj, NULL);
    _gt_switcher_st * style = (_gt_switcher_st * )obj;

    style->color_ina    = gt_color_hex(0xebeef5);
    style->color_act    = gt_color_hex(0x13ce66);
    style->color_point  = gt_color_hex(0xffffff);

    return obj;
}

void gt_switch_set_color_act(gt_obj_st * switcher, gt_color_t color)
{
    _gt_switcher_st * style = (_gt_switcher_st * )switcher;
    style->color_act = color;
    gt_event_send(switcher, GT_EVENT_TYPE_DRAW_START, NULL);
}
void gt_switch_set_color_ina(gt_obj_st * switcher, gt_color_t color)
{
    _gt_switcher_st * style = (_gt_switcher_st * )switcher;
    style->color_ina = color;
    gt_event_send(switcher, GT_EVENT_TYPE_DRAW_START, NULL);
}
void gt_switch_set_color_point(gt_obj_st * switcher, gt_color_t color)
{
    _gt_switcher_st * style = (_gt_switcher_st * )switcher;
    style->color_point = color;
    gt_event_send(switcher, GT_EVENT_TYPE_DRAW_START, NULL);
}

#endif  /** GT_CFG_ENABLE_SWITCH */
/* end ------------------------------------------------------------------*/
