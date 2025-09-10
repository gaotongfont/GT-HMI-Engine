/**
 * @file gt_indev_effects.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2025-09-10 12:03:17
 * @copyright Copyright (c) 2014-2025, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_indev_effects.h"

#if GT_CFG_ENABLE_INDEV_EFFECTS
#include "../core/gt_disp.h"
#include "../core/gt_draw.h"
#include "../others/gt_color.h"
#include "../widgets/gt_rect.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_INDEV_EFFECTS
#define MY_CLASS    &gt_indev_effects_class

//
#define _GT_EFFECTS_W           (GT_SCREEN_WIDTH >> 3)
#define _GT_EFFECTS_H           (GT_SCREEN_HEIGHT >> 1)
#define _GT_EFFECTS_Y           (GT_SCREEN_HEIGHT >> 2)
#define _GT_EFFECTS_X_L         (0)
#define _GT_EFFECTS_X_R         (GT_SCREEN_WIDTH - _GT_EFFECTS_W)
// rect
#define _GT_EFFECTS_RECT_W          (_GT_EFFECTS_W << 1)
#define _GT_EFFECTS_RECT_H          (_GT_EFFECTS_H)
#define _GT_EFFECTS_RECT_RADIUS     (_GT_EFFECTS_W)
#define _GT_EFFECTS_RECT_COLOR      gt_color_black()
#define _GT_EFFECTS_RECT_OPA        (GT_OPA_50)
//
#define _GT_EFFECTS_DISTANCE_MAX    (_GT_EFFECTS_RECT_W >> 2)

/* private typedef ------------------------------------------------------*/
typedef struct {
    gt_obj_st obj;
    gt_obj_st* rect_p;
}_gt_indev_effects_st;

typedef struct {
    _gt_indev_effects_st* _effects_p;
}_gt_indev_effects_ctrl_st;
/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static void GT_ATTRIBUTE_RAM_TEXT _indev_effects_init_cb(gt_obj_st * obj);
static void GT_ATTRIBUTE_RAM_TEXT _indev_effects_deinit_cb(gt_obj_st * obj);
static void GT_ATTRIBUTE_RAM_TEXT _indev_effects_event_cb(struct gt_obj_s * obj, gt_event_st * e);

static const gt_obj_class_st gt_indev_effects_class = {
    ._init_cb      = NULL,
    ._deinit_cb    = NULL,
    ._event_cb     = NULL,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_indev_effects_st)
};

_gt_indev_effects_ctrl_st _effects_ctrl = {
    ._effects_p    = NULL
};
/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void GT_ATTRIBUTE_RAM_TEXT _indev_effects_init_cb(gt_obj_st * obj) {

}

static void GT_ATTRIBUTE_RAM_TEXT _indev_effects_deinit_cb(gt_obj_st * obj) {

}

static void GT_ATTRIBUTE_RAM_TEXT _indev_effects_event_cb(struct gt_obj_s * obj, gt_event_st * e) {

}

static inline _gt_indev_effects_ctrl_st* GT_ATTRIBUTE_RAM_TEXT _get_effect_ctrl(void) {
    return &_effects_ctrl;
}

static void GT_ATTRIBUTE_RAM_TEXT _del_effect_cb(gt_event_st * e) {
    _effects_ctrl._effects_p = NULL;
}


static void GT_ATTRIBUTE_RAM_TEXT _effects_set_pos(gt_obj_st* obj, bool is_left, uint32_t distance) {
    _gt_indev_effects_st* style = (_gt_indev_effects_st*) obj;
    gt_obj_set_pos(obj, is_left ? _GT_EFFECTS_X_L : _GT_EFFECTS_X_R, _GT_EFFECTS_Y);
    gt_size_t x = gt_obj_get_x(obj);
    gt_size_t y = gt_obj_get_y(obj);

    distance = distance > _GT_EFFECTS_DISTANCE_MAX ? _GT_EFFECTS_DISTANCE_MAX : distance;

    if(is_left) {
        x = x - _GT_EFFECTS_RECT_W + distance;
    }
    else {
        x = x + _GT_EFFECTS_W - distance;
    }

    gt_obj_set_pos(style->rect_p, x, y);
}

static _gt_indev_effects_st* GT_ATTRIBUTE_RAM_TEXT _effects_create(void) {
    _gt_indev_effects_ctrl_st* ctrl = _get_effect_ctrl();
    if(false == gt_obj_is_type((gt_obj_st*)ctrl->_effects_p, OBJ_TYPE)) {
#if GT_USE_LAYER_TOP
        ctrl->_effects_p = (_gt_indev_effects_st*) gt_obj_class_create(MY_CLASS, gt_disp_get_layer_top());
        gt_obj_add_event_cb((gt_obj_st * )ctrl->_effects_p, _del_effect_cb, GT_EVENT_TYPE_CHANGE_DELETED, NULL);
        //
        gt_obj_set_pos((gt_obj_st*)ctrl->_effects_p, _GT_EFFECTS_X_L, _GT_EFFECTS_Y);
        gt_obj_set_size((gt_obj_st*)ctrl->_effects_p, _GT_EFFECTS_W, _GT_EFFECTS_H);
        gt_obj_set_visible((gt_obj_st*)ctrl->_effects_p, GT_INVISIBLE);
#endif
        if(ctrl->_effects_p) {
            ctrl->_effects_p->rect_p = gt_rect_create((gt_obj_st*)ctrl->_effects_p);
            _effects_set_pos((gt_obj_st*)ctrl->_effects_p, true, 0);

            gt_obj_set_size(ctrl->_effects_p->rect_p, _GT_EFFECTS_RECT_W, _GT_EFFECTS_RECT_H);
            gt_obj_set_radius(ctrl->_effects_p->rect_p, _GT_EFFECTS_RECT_RADIUS);
            gt_rect_set_bg_color(ctrl->_effects_p->rect_p, _GT_EFFECTS_RECT_COLOR);
            gt_obj_set_opa(ctrl->_effects_p->rect_p, _GT_EFFECTS_RECT_OPA);
        }
    }

    return ctrl->_effects_p;
}



/* global functions / API interface -------------------------------------*/
void gt_indev_effects_show(gt_event_type_et gesture_type, uint32_t distance)
{
    _gt_indev_effects_st* style = _effects_create();
    gt_obj_st* obj = (gt_obj_st*)style;

    if(gesture_type == GT_EVENT_TYPE_INPUT_HOME_GESTURE_LEFT) {
        _effects_set_pos(obj, true, distance);
    }
    else if(gesture_type == GT_EVENT_TYPE_INPUT_HOME_GESTURE_RIGHT) {
        _effects_set_pos(obj, false, distance);
    }
    else {
        return;
    }

    if(gt_obj_get_visible(obj) == GT_INVISIBLE){
        gt_obj_set_visible(obj, GT_VISIBLE);
    }
}

void gt_indev_effects_hide(void)
{
    _gt_indev_effects_st* style = _effects_create();
    gt_obj_st* obj = (gt_obj_st*)style;

    if(gt_obj_get_visible(obj) == GT_VISIBLE) {
        gt_obj_set_visible(obj, GT_INVISIBLE);
    }
}

void gt_indev_effects_set_color(gt_color_t color)
{
    _gt_indev_effects_st* style = _effects_create();
    gt_rect_set_bg_color(style->rect_p, color);
}

void gt_indev_effects_set_opa(uint8_t opa)
{
    _gt_indev_effects_st* style = _effects_create();
    gt_obj_set_opa(style->rect_p, opa);
}
/* end of file ----------------------------------------------------------*/
#endif //! GT_CFG_ENABLE_INDEV_EFFECTS

