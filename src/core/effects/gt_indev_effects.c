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
#include "../../core/gt_disp.h"
#include "../../core/gt_draw.h"
#include "../../core/gt_indev.h"
#include "../../others/gt_color.h"
#include "../../widgets/gt_rect.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_INDEV_EFFECTS
#define MY_CLASS    &gt_indev_effects_class

/* Geometry must follow the current display resolution instead of the
 * compile-time screen size because some pages switch orientation at runtime. */
#define _GT_EFFECTS_RECT_COLOR      gt_color_black()
#define _GT_EFFECTS_RECT_OPA        (GT_OPA_50)

/* private typedef ------------------------------------------------------*/
typedef struct {
    gt_obj_st base;
    gt_obj_st* rect_p;
    gt_anim_st* anim_p;

    gt_indev_effects_distance_cb_t left_cb;
    gt_indev_effects_distance_cb_t right_cb;
    gt_indev_effects_distance_cb_t top_cb;
    gt_indev_effects_distance_cb_t bottom_cb;
    gt_indev_effects_hide_cb_t hide_cb;
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
    ._event_cb     = _indev_effects_event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_indev_effects_st)
};

_gt_indev_effects_ctrl_st _effects_ctrl = {
    ._effects_p    = NULL
};
/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
GT_ATTRIBUTE_RAM_TEXT
static void _indev_effects_init_cb(gt_obj_st * obj) {

}

GT_ATTRIBUTE_RAM_TEXT
static void _indev_effects_deinit_cb(gt_obj_st * obj) {

}

GT_ATTRIBUTE_RAM_TEXT
static void _indev_effects_event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code_val = gt_event_get_code(e);
    if (GT_EVENT_TYPE_DRAW_START != code_val) {
        return;
    }
#if GT_USE_LAYER_TOP
    gt_disp_st * disp = gt_disp_get_default();
    if (NULL == disp || !disp->enabled) {
        goto _effects_draw_end_lb;
    }
    gt_area_st invalid = gt_area_reduce(obj->area, -(gt_obj_get_focus_offset_value(obj)));
    if (disp->layer_top && gt_obj_is_child(obj, disp->layer_top)) {
        invalid.x += disp->scr_act->area.x;
        invalid.y += disp->scr_act->area.y;
    }
    _gt_disp_refr_append_area(&invalid);
#else
    gt_disp_invalid_area(obj);
#endif
_effects_draw_end_lb:
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
}

GT_ATTRIBUTE_RAM_TEXT
static inline _gt_indev_effects_ctrl_st* _get_effect_ctrl(void) {
    return &_effects_ctrl;
}

GT_ATTRIBUTE_RAM_TEXT
static void _del_effect_cb(gt_event_st * e) {
    _effects_ctrl._effects_p = NULL;
}

GT_ATTRIBUTE_RAM_TEXT
static bool _effects_set_obj_pos(_gt_indev_effects_st * style, gt_size_t x, gt_size_t y) {
    gt_size_t diff_x = 0;
    gt_size_t diff_y = 0;

    if (NULL == style) {
        return false;
    }

    diff_x = x - style->obj.area.x;
    diff_y = y - style->obj.area.y;
    if (0 == diff_x && 0 == diff_y) {
        return false;
    }

    style->obj.area.x = x;
    style->obj.area.y = y;

    for (uint16_t i = 0; i < style->obj.cnt_child; ++i) {
        gt_obj_st * child = style->obj.child[i];
        if (NULL == child) {
            continue;
        }
        child->area.x += diff_x;
        child->area.y += diff_y;
    }

    return true;
}

GT_ATTRIBUTE_RAM_TEXT
static void _effects_get_geometry(gt_size_t * effects_w,
                                  gt_size_t * effects_h,
                                  gt_size_t * effects_y,
                                  gt_size_t * effects_x_l,
                                  gt_size_t * effects_x_r,
                                  gt_size_t * effects_rect_w,
                                  gt_size_t * effects_rect_h,
                                  gt_size_t * effects_rect_radius,
                                  gt_size_t * effects_distance_max) {
    uint16_t res_hor = 0;
    uint16_t res_ver = 0;
    gt_obj_st * scr = gt_disp_get_scr();
    gt_obj_st * layer_top = gt_disp_get_layer_top();
    gt_size_t base_x = 0;
    gt_size_t base_y = 0;

    if (scr && scr->area.w && scr->area.h) {
        res_hor = scr->area.w;
        res_ver = scr->area.h;
        if (layer_top) {
            base_x = scr->area.x - layer_top->area.x;
            base_y = scr->area.y - layer_top->area.y;
        } else {
            base_x = scr->area.x;
            base_y = scr->area.y;
        }
    } else {
        res_hor = gt_disp_get_res_hor(NULL);
        res_ver = gt_disp_get_res_ver(NULL);
    }

    if (0 == res_hor) {
        res_hor = GT_SCREEN_WIDTH;
    }
    if (0 == res_ver) {
        res_ver = GT_SCREEN_HEIGHT;
    }

    gt_size_t local_effects_w = (gt_size_t)res_hor >> 3;
    gt_size_t local_effects_h = (gt_size_t)res_ver >> 1;
    gt_size_t local_effects_rect_w = local_effects_w << 1;

    if (effects_w) { *effects_w = local_effects_w; }
    if (effects_h) { *effects_h = local_effects_h; }
    if (effects_y) { *effects_y = base_y + ((gt_size_t)res_ver >> 2); }
    if (effects_x_l) { *effects_x_l = base_x; }
    if (effects_x_r) { *effects_x_r = base_x + (gt_size_t)res_hor - local_effects_w; }
    if (effects_rect_w) { *effects_rect_w = local_effects_rect_w; }
    if (effects_rect_h) { *effects_rect_h = local_effects_h; }
    if (effects_rect_radius) { *effects_rect_radius = local_effects_w; }
    if (effects_distance_max) { *effects_distance_max = local_effects_rect_w >> 2; }
}

GT_ATTRIBUTE_RAM_TEXT
static bool _effects_refresh_geometry(_gt_indev_effects_st * style) {
    gt_obj_st * obj = (gt_obj_st *)style;
    gt_size_t effects_w = 0, effects_h = 0, effects_y = 0;
    gt_size_t effects_rect_w = 0, effects_rect_h = 0, effects_rect_radius = 0;
    bool changed = false;

    if (NULL == style || NULL == style->rect_p) {
        return false;
    }

    _effects_get_geometry(&effects_w, &effects_h, &effects_y, NULL, NULL,
                          &effects_rect_w, &effects_rect_h,
                          &effects_rect_radius, NULL);

    if (gt_obj_get_w(obj) != effects_w || gt_obj_get_h(obj) != effects_h) {
        gt_obj_set_size(obj, effects_w, effects_h);
        changed = true;
    }
    if (gt_obj_get_y(obj) != effects_y) {
        changed |= _effects_set_obj_pos(style, gt_obj_get_x(obj), effects_y);
    }
    if (gt_obj_get_w(style->rect_p) != effects_rect_w || gt_obj_get_h(style->rect_p) != effects_rect_h) {
        gt_obj_set_size(style->rect_p, effects_rect_w, effects_rect_h);
        changed = true;
    }
    if (style->rect_p->radius != effects_rect_radius) {
        gt_obj_set_radius(style->rect_p, effects_rect_radius);
        changed = true;
    }

    return changed;
}


GT_ATTRIBUTE_RAM_TEXT
static void _effects_set_pos(gt_obj_st* obj, bool is_left, uint32_t distance) {
    _gt_indev_effects_st* style = (_gt_indev_effects_st*) obj;
    bool geom_changed = _effects_refresh_geometry(style);

    gt_size_t effects_w = 0, effects_y = 0, effects_x_l = 0, effects_x_r = 0;
    gt_size_t effects_rect_w = 0, effects_distance_max = 0;
    _effects_get_geometry(&effects_w, NULL, &effects_y, &effects_x_l, &effects_x_r,
                          &effects_rect_w, NULL, NULL, &effects_distance_max);

    gt_size_t obj_x = is_left ? effects_x_l : effects_x_r;
    bool pos_changed = false;
    if (obj_x != gt_obj_get_x(obj)) {
        pos_changed = _effects_set_obj_pos(style, obj_x, effects_y);
    }
    gt_size_t x = gt_obj_get_x(obj);
    gt_size_t y = gt_obj_get_y(obj);

    distance = distance > (uint32_t)effects_distance_max ? (uint32_t)effects_distance_max : distance;

    if(is_left) {
        x = x - effects_rect_w + distance;
    }
    else {
        x = x + effects_w - distance;
    }
    if (gt_obj_get_x(style->rect_p) == x && false == geom_changed && false == pos_changed) {
        return;
    }
    /* Directly update rect_p position without triggering gt_obj_pos_change,
     * which would submit a raw physical-coord dirty area that conflicts with
     * the scr-offset-adjusted area submitted by _indev_effects_event_cb. */
    style->rect_p->area.x = x;
    style->rect_p->area.y = y;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

GT_ATTRIBUTE_RAM_TEXT
static _gt_indev_effects_st*  _effects_create(void) {
    _gt_indev_effects_ctrl_st* ctrl = _get_effect_ctrl();
    if(gt_obj_is_type((gt_obj_st*)ctrl->_effects_p, OBJ_TYPE)) {
        _effects_refresh_geometry(ctrl->_effects_p);
        return ctrl->_effects_p;
    }

    gt_size_t effects_w = 0, effects_h = 0, effects_y = 0;
    gt_size_t effects_rect_w = 0, effects_rect_h = 0, effects_rect_radius = 0;
    _effects_get_geometry(&effects_w, &effects_h, &effects_y, NULL, NULL,
                          &effects_rect_w, &effects_rect_h,
                          &effects_rect_radius, NULL);

#if GT_USE_LAYER_TOP
    ctrl->_effects_p = (_gt_indev_effects_st*) gt_obj_class_create(MY_CLASS, gt_disp_get_layer_top());
    gt_obj_add_event_cb((gt_obj_st * )ctrl->_effects_p, _del_effect_cb, GT_EVENT_TYPE_CHANGE_DELETED, NULL);
    //
    gt_obj_set_pos((gt_obj_st*)ctrl->_effects_p, 0, effects_y);
    gt_obj_set_size((gt_obj_st*)ctrl->_effects_p, effects_w, effects_h);
    gt_obj_set_visible((gt_obj_st*)ctrl->_effects_p, GT_INVISIBLE);
#endif
    if(ctrl->_effects_p) {
        ctrl->_effects_p->rect_p = gt_rect_create((gt_obj_st*)ctrl->_effects_p);
        _effects_set_pos((gt_obj_st*)ctrl->_effects_p, true, 0);

        gt_obj_set_size(ctrl->_effects_p->rect_p, effects_rect_w, effects_rect_h);
        gt_obj_set_radius(ctrl->_effects_p->rect_p, effects_rect_radius);
        gt_rect_set_bg_color(ctrl->_effects_p->rect_p, _GT_EFFECTS_RECT_COLOR);
        gt_obj_set_opa(ctrl->_effects_p->rect_p, _GT_EFFECTS_RECT_OPA);
    }

    return ctrl->_effects_p;
}

GT_ATTRIBUTE_RAM_TEXT
void _effects_anim_ready_cb(struct gt_anim_s * anim) {
    gt_indev_effects_hide();
}

GT_ATTRIBUTE_RAM_TEXT
static void _effects_anim_create(gt_obj_st* obj)
{
    _gt_indev_effects_st* style = (_gt_indev_effects_st*)obj;
    if(style->anim_p) {
        gt_anim_restart(style->anim_p);
        return;
    }

    gt_anim_st anim = {0};
    gt_anim_init(&anim);
    gt_anim_set_target(&anim, obj);
    gt_anim_set_time(&anim, 200);
    gt_anim_set_ready_cb(&anim, _effects_anim_ready_cb);
    style->anim_p = gt_anim_start(&anim);
}
GT_ATTRIBUTE_RAM_TEXT
static void _effects_anim_del(gt_obj_st* obj)
{
    _gt_indev_effects_st* style = (_gt_indev_effects_st*)obj;
    if(style->anim_p) {
        gt_anim_del_safe_by(obj);
        style->anim_p = NULL;
    }
}

static void _effects_distance_cb(gt_event_type_et gesture_type, uint32_t distance)
{
    _gt_indev_effects_st* style = _effects_create();

    switch(gesture_type) {
        case GT_EVENT_TYPE_INPUT_HOME_GESTURE_LEFT:
            if(style->left_cb) {
                style->left_cb(distance);
            }
            break;
        case GT_EVENT_TYPE_INPUT_HOME_GESTURE_RIGHT:
            if(style->right_cb) {
                style->right_cb(distance);
            }
            break;
        case GT_EVENT_TYPE_INPUT_HOME_GESTURE_TOP:
            if(style->top_cb) {
                style->top_cb(distance);
            }
            break;
        case GT_EVENT_TYPE_INPUT_HOME_GESTURE_BOTTOM:
            if(style->bottom_cb) {
                style->bottom_cb(distance);
            }
            break;
        default:
            break;
    }
}
/* global functions / API interface -------------------------------------*/
GT_ATTRIBUTE_RAM_TEXT
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
    _effects_anim_create(obj);

    _effects_distance_cb(gesture_type, distance);
}

GT_ATTRIBUTE_RAM_TEXT
void gt_indev_effects_hide(void)
{
    _gt_indev_effects_st* style = _effects_create();
    gt_obj_st* obj = (gt_obj_st*)style;
    if (GT_INVISIBLE == gt_obj_get_visible(obj)) {
        return;
    }

    if(gt_obj_get_visible(obj) == GT_VISIBLE) {
        gt_obj_set_visible(obj, GT_INVISIBLE);
        if(style->hide_cb) {
            style->hide_cb();
        }
    }

    _effects_anim_del(obj);
}

GT_ATTRIBUTE_RAM_TEXT
void gt_indev_effects_set_color(gt_color_t color)
{
    _gt_indev_effects_st* style = _effects_create();
    if (color.full == style->rect_p->bgcolor.full) {
        return;
    }
    gt_rect_set_bg_color(style->rect_p, color);
}

GT_ATTRIBUTE_RAM_TEXT
void gt_indev_effects_set_opa(uint8_t opa)
{
    _gt_indev_effects_st* style = _effects_create();
    if (opa == style->rect_p->opa) {
        return;
    }
    gt_obj_set_opa(style->rect_p, opa);
}

GT_ATTRIBUTE_RAM_TEXT
void gt_indev_effects_set_distance_cb(gt_indev_effects_distance_cb_t left_cb,
                                    gt_indev_effects_distance_cb_t right_cb,
                                    gt_indev_effects_distance_cb_t top_cb,
                                    gt_indev_effects_distance_cb_t bottom_cb)
{
    _gt_indev_effects_st* style = _effects_create();
    style->left_cb = left_cb;
    style->right_cb = right_cb;
    style->top_cb = top_cb;
    style->bottom_cb = bottom_cb;
}

GT_ATTRIBUTE_RAM_TEXT
void gt_indev_effects_set_hide_cb(gt_indev_effects_hide_cb_t hide_cb)
{
    _gt_indev_effects_st* style = _effects_create();
    style->hide_cb = hide_cb;
}

/* end of file ----------------------------------------------------------*/
#endif //! GT_CFG_ENABLE_INDEV_EFFECTS
