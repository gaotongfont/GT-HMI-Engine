/**
 * @file gt_indev.c
 * @author yongg
 * @brief input device handler
 * @version 0.1
 * @date 2022-06-06 14:06:09
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

 /* include --------------------------------------------------------------*/
#include "gt_indev.h"
#include "stdbool.h"
#include "../hal/gt_hal_indev.h"
#include "../hal/gt_hal_disp.h"
#include "../hal/gt_hal_tick.h"
#include "../others/gt_log.h"
#include "../others/gt_math.h"
#include "../others/gt_anim.h"
#include "gt_disp.h"
#include "gt_obj_pos.h"
#include "gt_obj_scroll.h"
#include "gt_style.h"
#include "../widgets/gt_dialog.h"
#include "effects/gt_indev_effects.h"
/* private define -------------------------------------------------------*/
#ifndef GT_INDEV_SCROLL_THROW_SPEED_UP
    /**
     * @brief 0[default]: do not append release scroll throw sum calc distance;
     *        1: append release scroll throw sum calc distance;
     */
    #define GT_INDEV_SCROLL_THROW_SPEED_UP              0
#endif

#ifdef GT_INDEV_SCROLL_THROW_SPEED_UP
    #ifndef GT_INDEV_SCROLL_THROW_SPEED_UP_VER
        #define GT_INDEV_SCROLL_THROW_SPEED_UP_VER      1
    #endif

    #ifndef GT_INDEV_SCROLL_THROW_SPEED_UP_HOR
        #define GT_INDEV_SCROLL_THROW_SPEED_UP_HOR      1
    #endif
#endif  /** GT_INDEV_SCROLL_THROW_SPEED_UP */

#ifndef GT_INDEV_POINT_LOG
    /**
     * @brief 0[default]: disable log, 1: enable log
     */
    #define GT_INDEV_POINT_LOG      0
#endif

#ifndef GT_INDEV_KEYPAD_LOG
    /**
     * @brief 0[default]: disable log, 1: enable log
     */
    #define GT_INDEV_KEYPAD_LOG     0
#endif

#if GT_INDEV_CFG_VELOCITY
    #ifndef GT_INDEV_THROW_LOW_PASS_RAPID_CALC
    /**
     * @brief Low-pass filtering for rapid calculation
     *      [default: 1] true: 0.75 * a + 0.25 * b
     *                  false: 0.625 * a + 0.375 * b
     */
        #define GT_INDEV_THROW_LOW_PASS_RAPID_CALC      1
    #endif


    #ifndef GT_INDEV_THROW_PIXEL_PER_SECOND
        /**
         * @brief about n px/s to scroll slow down
         */
        #define GT_INDEV_THROW_PIXEL_PER_SECOND         1300
    #endif

    /**
     * @brief Scroll throw deceleration style
     *      Configured by GT_INDEV_THROW_STYLE in gt_conf.h
     */
    #if GT_INDEV_THROW_STYLE == 0
        /** iOS style parameters */
        #ifndef GT_INDEV_THROW_FRICTION
            #define GT_INDEV_THROW_FRICTION              5
        #endif
        #ifndef GT_INDEV_THROW_LOW_SPEED_THRESHOLD
            #define GT_INDEV_THROW_LOW_SPEED_THRESHOLD   40
        #endif
        #ifndef GT_INDEV_THROW_LINEAR_DECEL
            #define GT_INDEV_THROW_LINEAR_DECEL          10
        #endif
    #else
        /** Android style parameters */
        #ifndef GT_INDEV_THROW_FRICTION
            #define GT_INDEV_THROW_FRICTION              12
        #endif
        #ifndef GT_INDEV_THROW_LOW_SPEED_THRESHOLD
            #define GT_INDEV_THROW_LOW_SPEED_THRESHOLD   80
        #endif
        #ifndef GT_INDEV_THROW_LINEAR_DECEL
            #define GT_INDEV_THROW_LINEAR_DECEL          20
        #endif
    #endif

    #ifndef GT_INDEV_OVER_SCROLL_MAX_VELOCITY
        /**
         * @brief Limit over scroll max velocity
         */
        #define GT_INDEV_OVER_SCROLL_MAX_VELOCITY       1800
    #endif
#endif

#if GT_USE_UD_LR_TO_CONTROL_FOCUS_EN
// keyboard lock
#define _SET_KEYPAD_LOCK(_idv, _fg)     do{ \
                                            (_idv)->data.keypad.lock = (_fg); \
                                            gt_color_focus_set( _fg ? GT_FOCUS_COLOR_LOCK : GT_FOCUS_COLOR_SELECT);\
                                        }while(0)

#define _IS_KEYPAD_LOCKED(_idv)           ((_idv)->data.keypad.lock)
#endif
/* private typedef ------------------------------------------------------*/
typedef struct {
    bool val;
    gt_event_type_et ( * get_direct_event_type_cb)(gt_dir_et gesture);
}_indev_obj_props_handler_st;

typedef struct {
    uint16_t active;
    uint16_t working_distance;  /** active scroll valid working distance */
    uint16_t display_distance;  /** display scroll distance mapping by workding distance */
    uint8_t linear;
}_indev_gesture_over_scroll_st;

typedef struct {
    gt_point_st distance;
    bool is_over_scroll;
}_indev_over_scroll_ret_st;

/* static variables -----------------------------------------------------*/

static GT_ATTRIBUTE_RAM_DATA gt_indev_param_st _indev_params = {
    .enabled = true,
};

/* macros ---------------------------------------------------------------*/
static gt_res_t GT_ATTRIBUTE_RAM_TEXT
_indev_rebound_recover_handler(gt_indev_st * indev, gt_obj_st * target);



/* static functions -----------------------------------------------------*/
static GT_ATTRIBUTE_RAM_TEXT inline void _rotate_point_inv(
    gt_rotated_et rotation, uint16_t phy_w, uint16_t phy_h,
    gt_size_t * x, gt_size_t * y)
{
    gt_size_t tmp;
    switch (rotation) {
        case GT_ROTATED_90:
            /* physical(px,py) -> logical x=py, y=phy_w-1-px */
            tmp = *x; *x = *y; *y = (gt_size_t)(phy_w - 1) - tmp;
            break;
        case GT_ROTATED_180:
            *x = (gt_size_t)(phy_w - 1) - *x;
            *y = (gt_size_t)(phy_h - 1) - *y;
            break;
        case GT_ROTATED_270:
            /* physical(px,py) -> logical x=phy_h-1-py, y=px */
            tmp = *x; *x = (gt_size_t)(phy_h - 1) - *y; *y = tmp;
            break;
        default:
            break;
    }
}

static GT_ATTRIBUTE_RAM_TEXT inline bool _is_scroll_vertical(gt_dir_et dir) {
    return (GT_DIR_UP == dir || GT_DIR_DOWN == dir) ? true : false;
}

static GT_ATTRIBUTE_RAM_TEXT gt_size_t _scroll_predict(gt_indev_st * indev, gt_size_t v)
{
    gt_size_t result = 0;
    GT_CHECK_BACK_VAL(indev, result);

    gt_size_t limit_scroll_throw = indev->drv->limit_scroll_throw;
    while(v) {
        result += v;
        v = (100 - limit_scroll_throw) * v / 100;
    }

    return result;
}

static GT_ATTRIBUTE_RAM_TEXT gt_obj_st * _search_not_fixed_by_parent_recursive(gt_obj_st * obj) {
    gt_obj_st * ptr = obj;
    while(ptr) {
        if (false == ptr->fixed) {
            break;
        }
        ptr = ptr->parent;
    }
    return ptr;
}

static GT_ATTRIBUTE_RAM_TEXT gt_dir_et _get_scroll_dir(gt_indev_st * indev) {
    struct _point * point_p = &indev->data.point;
    gt_dir_et ret = (gt_dir_et)point_p->gesture;

    gt_size_t abs_sx = gt_abs(point_p->scroll_sum.x);
    gt_size_t abs_sy = gt_abs(point_p->scroll_sum.y);
    if (abs_sx > abs_sy) {
        if (point_p->scroll_sum.x > indev->drv->limit_pixel_gesture_hor) {
            ret = GT_DIR_RIGHT;
        } else if (point_p->scroll_sum.x < -indev->drv->limit_pixel_gesture_hor) {
            ret = GT_DIR_LEFT;
        }
        point_p->scroll_throw.y = 0;
    } else if (abs_sx || abs_sy) {
        if (point_p->scroll_sum.y > indev->drv->limit_pixel_gesture_ver) {
            ret = GT_DIR_DOWN;
        } else if (point_p->scroll_sum.y < -indev->drv->limit_pixel_gesture_ver) {
            ret = GT_DIR_UP;
        }
        point_p->scroll_throw.x = 0;
    } else {
        ret = GT_DIR_NONE;
    }

    return ret;
}

static GT_ATTRIBUTE_RAM_TEXT bool
_scr_has_home_gesture_events(gt_obj_st * target, gt_event_type_et event) {
    if (GT_EVENT_TYPE_NONE == event) {
        return false;
    }
    if (NULL == target) {
        return false;
    }
    gt_obj_st * tar_scr = gt_obj_within_which_scr(target);
#if GT_CFG_ENABLE_DIALOG
    if (gt_dialog_has_showing()) {
        tar_scr = gt_dialog_get_active_obj();
    }
#endif
    if (NULL == tar_scr) {
        return false;
    }
    return gt_event_has_event_cb(tar_scr, event);
}

static gt_indev_home_gesture_area_et GT_ATTRIBUTE_RAM_TEXT
_is_first_point_at_home_gesture_area(gt_point_st const * const p) {
    gt_indev_home_gesture_area_et first_area = GT_INDEV_FP_HOME_GESTURE_NONE;
    gt_size_t res_ver = gt_disp_get_res_ver(NULL);
    gt_size_t res_hor = gt_disp_get_res_hor(NULL);

    if (p->y + 1 < GT_CFG_DEFAULT_AREA_HOME_GESTURE_TOP) {
        first_area = GT_INDEV_FP_HOME_GESTURE_TOP;
    }
    else if (p->y + GT_CFG_DEFAULT_AREA_HOME_GESTURE_BOTTOM + 1 > res_ver) {
        first_area = GT_INDEV_FP_HOME_GESTURE_BOTTOM;
    }
    if (p->x + 1 < GT_CFG_DEFAULT_AREA_HOME_GESTURE_LEFT) {
        first_area = GT_INDEV_FP_HOME_GESTURE_LEFT;
    }
    else if (p->x + GT_CFG_DEFAULT_AREA_HOME_GESTURE_RIGHT + 1 > res_hor) {
        first_area = GT_INDEV_FP_HOME_GESTURE_RIGHT;
    }
    return first_area;
}

static inline gt_event_type_et GT_ATTRIBUTE_RAM_TEXT
_get_home_gesture_event_type(gt_indev_st * indev) {
    struct _point * point_p = &indev->data.point;
    gt_event_type_et gesture_event = GT_EVENT_TYPE_NONE;
    if (GT_INDEV_FP_HOME_GESTURE_NONE == point_p->home_gesture_area) {
        return gesture_event;
    }
    point_p->gesture = _get_scroll_dir(indev);
    if ((uint16_t)GT_DIR_NONE == point_p->gesture) {
        return gesture_event;
    }
    if (GT_DIR_DOWN == point_p->gesture &&
        GT_INDEV_FP_HOME_GESTURE_TOP == point_p->home_gesture_area) {
        // top
        gesture_event = GT_EVENT_TYPE_INPUT_HOME_GESTURE_TOP;
    }
    else if (GT_DIR_UP == point_p->gesture &&
        GT_INDEV_FP_HOME_GESTURE_BOTTOM == point_p->home_gesture_area) {
        // bottom
        gesture_event = GT_EVENT_TYPE_INPUT_HOME_GESTURE_BOTTOM;
    }
    else if (GT_DIR_LEFT == point_p->gesture &&
        GT_INDEV_FP_HOME_GESTURE_RIGHT == point_p->home_gesture_area) {
        // right
        gesture_event = GT_EVENT_TYPE_INPUT_HOME_GESTURE_RIGHT;
    }
    else if (GT_DIR_RIGHT == point_p->gesture &&
        GT_INDEV_FP_HOME_GESTURE_LEFT == point_p->home_gesture_area) {
        // left
        gesture_event = GT_EVENT_TYPE_INPUT_HOME_GESTURE_LEFT;
    }
    return gesture_event;
}

static GT_ATTRIBUTE_RAM_TEXT bool
_is_indev_home_gesture(gt_indev_st * indev, gt_obj_st * target, gt_event_type_et * event) {
    struct _point * point_p = &indev->data.point;
    if (GT_INDEV_FP_HOME_GESTURE_NONE == point_p->home_gesture_area) {
        return false;
    }
    gt_event_type_et gesture_type = _get_home_gesture_event_type(indev);
    if (event) { *event = gesture_type; }
    return _scr_has_home_gesture_events(target, gesture_type);
}

#if GT_CFG_ENABLE_INDEV_EFFECTS
/**
 * @brief
 *
 * @param indev
 * @param active touch pointer reality scroll distance
 * @param working_distance active scroll distance
 * @param display_distance display scroll distance
 * @return uint16_t
 */
static uint32_t GT_ATTRIBUTE_RAM_TEXT
_gesture_calc_overflow_distance(gt_indev_st * indev, _indev_gesture_over_scroll_st * param) {
    struct _point * point_p = &indev->data.point;
    if (param->active > param->working_distance) { param->active = param->working_distance; }
    gt_math_bezier_st bezier = {
        .t = gt_map(param->active, 0, param->working_distance, 0, GT_MATH_BEZIER_VAL_RESOLUTION),
        .p0 = 50, .p1 = 900, .p2 = 1000, .p3 = GT_MATH_BEZIER_VAL_RESOLUTION
    };
    int32_t step = param->linear ? bezier.t : gt_math_calc_bezier(&bezier);

    uint32_t value = param->display_distance * step;
    value >>= GT_MATH_BEZIER_VAL_SHIFT;

    return (uint32_t)(value >= 0 ? value : -value);
}
#endif  /** GT_CFG_ENABLE_INDEV_EFFECTS */

/**
 * @brief
 *
 * @param indev
 * @return true: is gesture
 */
static bool GT_ATTRIBUTE_RAM_TEXT
_home_gesture_scroll_effects(gt_indev_st * indev) {
    struct _point * point_p = &indev->data.point;
    gt_event_type_et gesture_type = GT_EVENT_TYPE_NONE;

    if (_is_indev_home_gesture(indev, point_p->obj_scroll, &gesture_type)) {
        if (false == point_p->is_home_gesture) {
            point_p->is_home_gesture = true;
            gt_event_send(point_p->obj_scroll, GT_EVENT_TYPE_INPUT_PRESS_LOST, NULL);
        }
#if GT_CFG_ENABLE_INDEV_EFFECTS
        _indev_gesture_over_scroll_st param = {
            .active = gt_abs(point_p->scroll_sum.x),
            .working_distance = GT_SCREEN_WIDTH >> 1,
            .display_distance = GT_INDEV_HOME_GESTURE_HOR_MAX_DISTANCE,
            .linear = false,
        };
        uint32_t distance = _gesture_calc_overflow_distance(indev, &param);
        gt_indev_effects_show(gesture_type, distance);
#endif
        return true;
    } else if (point_p->is_home_gesture) {
        point_p->is_home_gesture = false;
    }
    return false;
}

static bool GT_ATTRIBUTE_RAM_TEXT
_indev_send_home_gesture_event(gt_indev_st * indev) {
    gt_obj_st * scr = gt_disp_get_scr();
    gt_event_type_et e = GT_EVENT_TYPE_NONE;

#if GT_CFG_ENABLE_DIALOG
    if (gt_dialog_has_showing()) {
        if (false == _is_indev_home_gesture(indev, gt_disp_get_layer_top(), &e)) {
            return false;
        }

#if GT_CFG_ENABLE_INDEV_EFFECTS
    gt_indev_effects_hide();
#endif

        gt_event_send(gt_dialog_get_active_obj(), e, NULL);
        goto scroll_throw_lb;
    }
#endif
    if (false == _is_indev_home_gesture(indev, scr, &e)) {
        return false;
    }

#if GT_CFG_ENABLE_INDEV_EFFECTS
    gt_indev_effects_hide();
#endif

    gt_event_send(scr, e, NULL);

scroll_throw_lb:
    if (indev->scroll_throw_anim) {
        gt_anim_set_time(indev->scroll_throw_anim, 0);
    }
    return true;
}

static GT_ATTRIBUTE_RAM_TEXT inline void
_absorb_handler(gt_obj_st const * const obj_scroll, struct _point const * const point_p, gt_size_t * dx, gt_size_t * dy) {
    uint16_t w_parent = obj_scroll->parent->area.w;
    uint16_t h_parent = obj_scroll->parent->area.h;
    gt_size_t right = gt_disp_get_res_hor(NULL);
    gt_size_t bottom = gt_disp_get_res_ver(NULL);
    gt_size_t rmk_x = *dx, rmk_y = *dy;
    /* 缓存重复解引用的成员 */
    gt_size_t obj_x = obj_scroll->area.x;
    gt_size_t obj_y = obj_scroll->area.y;
    gt_size_t obj_w = obj_scroll->area.w;
    gt_size_t obj_h = obj_scroll->area.h;

    if (false == obj_scroll->absorb_dir) {
        /** hor */
        gt_size_t x = obj_x + rmk_x;  /** target x position */
        // outside the left
        if (x > 0) {
            rmk_x = -obj_x;
        }
        // outside the right
        else if (x + obj_w <= right) {
            rmk_x = right - obj_w - obj_x;
        }
        // inside
        else if (obj_scroll->scroll_snap_x) {
            gt_size_t tmp_x = 0 == rmk_x ? x : obj_x;
            if ((uint16_t)GT_DIR_LEFT == point_p->gesture) {
                rmk_x = ((x - w_parent) / w_parent) * w_parent - tmp_x;
            } else if ((uint16_t)GT_DIR_RIGHT == point_p->gesture) {
                rmk_x = (x / w_parent) * w_parent - tmp_x;
            }
        }
        *dx = rmk_x;
    } else {
        /** ver */
        gt_size_t y = obj_y + rmk_y;
        // top
        if (y > 0) {
            rmk_y = -obj_y;
        }
        // bottom
        else if (y + obj_h <= bottom) {
            rmk_y = bottom - obj_h - obj_y;
        }
        else if (obj_scroll->scroll_snap_y) {
            gt_size_t tmp_y = 0 == rmk_y ? y : obj_y;
            if ((uint16_t)GT_DIR_LEFT == point_p->gesture) {
                rmk_y = ((y - h_parent) / h_parent) * h_parent - tmp_y;
            } else if ((uint16_t)GT_DIR_RIGHT == point_p->gesture) {
                rmk_y = (y / h_parent) * h_parent - tmp_y;
            }
        }
        *dy = rmk_y;
    }
}

#if GT_INDEV_CFG_VELOCITY
static inline int32_t GT_ATTRIBUTE_RAM_TEXT
_calc_average_speed_inline(gt_size_t distance, int32_t dt) {
    return (gt_abs(distance) << 10) / dt;
}

static inline int32_t GT_ATTRIBUTE_RAM_TEXT
_calc_distance_inline(int32_t distance) {
    return distance >> 10;
}

static inline int32_t GT_ATTRIBUTE_RAM_TEXT
_calc_average_speed(gt_dir_et dir, gt_point_st const * const s, int32_t dt) {
    return _calc_average_speed_inline(_is_scroll_vertical(dir) ? s->y : s->x, dt);
}

static inline int32_t GT_ATTRIBUTE_RAM_TEXT
_calc_final_speed(int32_t prev_sum_v, int32_t next_v) {
#if GT_INDEV_THROW_LOW_PASS_RAPID_CALC
    /** 0.75 * a + 0.25 * b */
    return (3 * prev_sum_v + next_v) >> 2;
#else
    /** 0.625 * a + 0.375 * b */
    return (5 * prev_sum_v + 3 * next_v) >> 3;
#endif
}

static void GT_ATTRIBUTE_RAM_TEXT
_calc_throw_velocity(gt_indev_st * indev) {
    struct _point * point_p = &indev->data.point;
    uint32_t cur_time = gt_tick_get();
    int32_t delta_time = cur_time - indev->timestamp_prev_press;

    if (delta_time <= 0) {
        return;
    }
    int32_t velocity = _calc_average_speed((gt_dir_et)point_p->gesture, &point_p->scroll_diff, delta_time);
    point_p->throw_velocity = _calc_final_speed(point_p->throw_velocity, velocity);
    indev->timestamp_prev_press = cur_time;
    indev->timestamp_throw_prev_scroll = cur_time;
}

/**
 * @brief short detla time and less scroll points to speed up velocity
 *
 * @param indev
 * @return
 */
static void GT_ATTRIBUTE_RAM_TEXT
_velocity_less_scroll_points_need_to_speed_up(gt_indev_st * indev) {
    struct _point * point_p = &indev->data.point;
    if (point_p->diff_points > 1) {
        return;
    }
    uint32_t cur_time = gt_tick_get();
    int32_t delta_time = cur_time - indev->timestamp_prev_press;
    int32_t total_time = cur_time - indev->timestamp_start;
    gt_point_st diff = {0};
    if ((delta_time << 1) > total_time) {
        diff.x = point_p->scroll_sum.x << 6;
        diff.y = point_p->scroll_sum.y << 6;
        int32_t v = _calc_average_speed((gt_dir_et)point_p->gesture, &diff, total_time);
        point_p->throw_velocity = _calc_final_speed(point_p->throw_velocity, v);
    }
}

#if GT_CFG_ENABLE_INDEV_EFFECTS
static _indev_over_scroll_ret_st GT_ATTRIBUTE_RAM_TEXT
_scrolling_over_limit_area_effects(gt_indev_st * indev) {
    struct _point * point_p = &indev->data.point;
    gt_obj_st * obj_scroll = point_p->obj_scroll;
    gt_dir_et dir = point_p->gesture;
    gt_point_st * scroll = &point_p->scroll_sum;
    _indev_over_scroll_ret_st ret = {
        .is_over_scroll = false,
    };
    point_p->over_scroll = _gt_obj_get_over_scroll(obj_scroll, point_p->gesture);
    if (false == point_p->over_scroll.is_over_scroll) {
        point_p->over_scroll.distance.x = 0;
        point_p->over_scroll.distance.y = 0;
        return ret;
    }
    _indev_gesture_over_scroll_st param = {
        .linear = true,
    };
    if (point_p->over_scroll.top || point_p->over_scroll.bottom) {
        param.active = gt_abs(scroll->y);
        param.working_distance = obj_scroll->area.h;
        param.display_distance = param.working_distance >> 2;
        ret.distance.y = _gesture_calc_overflow_distance(indev, &param);
        if (point_p->over_scroll.bottom) {
            ret.distance.y = -ret.distance.y;
        }
    }
    if (point_p->over_scroll.left || point_p->over_scroll.right) {
        param.active = gt_abs(scroll->x);
        param.working_distance = obj_scroll->area.w;
        param.display_distance = param.working_distance >> 2;
        ret.distance.x = _gesture_calc_overflow_distance(indev, &param);
        if (point_p->over_scroll.right) {
            ret.distance.x = -ret.distance.x;
        }
    }
    ret.is_over_scroll = true;
    return ret;
}
#endif

static gt_res_t GT_ATTRIBUTE_RAM_TEXT
_throw_scroll_rebound_slow_down_handler(gt_indev_st * indev) {
    struct _point * point_p = &indev->data.point;
    gt_obj_st * scroll_obj = point_p->obj_scroll;
    int32_t max_v = GT_INDEV_OVER_SCROLL_MAX_VELOCITY;
    if (false == scroll_obj->rebound && false == scroll_obj->overflow) {
        return GT_RES_OK;
    }
    point_p->over_scroll =  _gt_obj_get_over_scroll(point_p->obj_scroll, point_p->gesture);
    if (false == point_p->over_scroll.is_over_scroll) {
        return GT_RES_OK;
    }
    do {
        point_p->throw_velocity = _calc_final_speed(point_p->throw_velocity, -point_p->throw_velocity);
    } while (point_p->throw_velocity > max_v);
    return GT_RES_OK;
}

static gt_res_t GT_ATTRIBUTE_RAM_TEXT
_indev_rebound_recover_handler(gt_indev_st * indev, gt_obj_st * target) {
    struct _point * point_p = &indev->data.point;
    if (GT_TYPE_UNKNOWN == gt_obj_class_get_type(target)) {
        return GT_RES_INV;
    }
    if (false == target->rebound) {
        return GT_RES_OK;
    }
    point_p->over_scroll =  _gt_obj_get_over_scroll(target, point_p->gesture);
    gt_indev_over_scroll_res_st * over_p = &point_p->over_scroll;
    if (false == over_p->is_over_scroll) {
        return GT_RES_OK;
    }
    if (over_p->top || over_p->bottom) {
        gt_obj_scroll_to_y(target, over_p->distance.y, GT_ANIM_ON);
    }
    if (over_p->left || over_p->right) {
        gt_obj_scroll_to_x(target, over_p->distance.x, GT_ANIM_ON);
    }
    return GT_RES_OK;
}

static gt_res_t GT_ATTRIBUTE_RAM_TEXT
_gt_indev_check_childs_rebound(gt_indev_st * indev, gt_obj_st * root) {
    /* 用显式栈替代递归，避免深层对象树的栈溢出和函数调用开销 */
    gt_obj_st * stack[64];
    int top = 0;

    if (gt_obj_is_type(root, GT_TYPE_UNKNOWN)) {
        return GT_RES_OK;
    }
    stack[top++] = root;

    while (top > 0) {
        gt_obj_st * obj = stack[--top];
        if (gt_obj_is_rebound(obj)) {
            _indev_rebound_recover_handler(indev, obj);
        }
        gt_size_t i = obj->cnt_child;
        while (i-- > 0) {
            gt_obj_st * child = obj->child[i];
            if (!gt_obj_is_type(child, GT_TYPE_UNKNOWN) && top < 64) {
                stack[top++] = child;
            }
        }
    }
    return GT_RES_OK;
}

static bool GT_ATTRIBUTE_RAM_TEXT
_calc_absorb_next_scroll_distance(
    struct _point * point_p, gt_obj_st * obj_scroll, gt_size_t * dx_p, gt_size_t * dy_p) {
    bool stop_throw_anim = false;
    gt_size_t dx = *dx_p, dy = *dy_p;
    if (0 == point_p->scroll_absorb_max_dist.x && 0 == point_p->scroll_absorb_max_dist.y) {
        gt_size_t absorb_dx = dx, absorb_dy = dy;
        _absorb_handler(obj_scroll, point_p, &absorb_dx, &absorb_dy);
        point_p->scroll_absorb_max_dist.x = obj_scroll->process_attr.scroll.x + absorb_dx;
        point_p->scroll_absorb_max_dist.y = obj_scroll->process_attr.scroll.y + absorb_dy;
    }
    gt_point_st diff = {
        .x = point_p->scroll_absorb_max_dist.x - obj_scroll->process_attr.scroll.x,
        .y = point_p->scroll_absorb_max_dist.y - obj_scroll->process_attr.scroll.y,
    };

    if (gt_abs(dx) >= gt_abs(diff.x)) {
        *dx_p = diff.x;
        stop_throw_anim = true;
    }
    if (gt_abs(dy) >= gt_abs(diff.y)) {
        *dy_p = diff.y;
        stop_throw_anim = true;
    }
    return stop_throw_anim;
}
#endif

/**
 * @brief Touch gestures make the act of throwing
 *
 * @param indev
 * @return bool stop throw anim
 */
static bool GT_ATTRIBUTE_RAM_TEXT
_indev_scroll_throw_handler(gt_indev_st * indev) {
    struct _point * point_p = &indev->data.point;
    gt_point_st detla_range = {0};
    point_p->gesture = _get_scroll_dir(indev);
    bool stop_throw_anim = false;

    if ((uint16_t)GT_DIR_NONE == point_p->gesture) {
        return stop_throw_anim;
    }

    gt_obj_st * obj_scroll = point_p->obj_scroll;
    gt_size_t dx = 0;
    gt_size_t dy = 0;
#if GT_INDEV_CFG_VELOCITY
    uint32_t cur_time = gt_tick_get();
    int32_t detla_time = cur_time - indev->timestamp_throw_prev_scroll;
    int8_t dir = 0;
    if (detla_time < 0) {
        detla_time = 0;
    }
    if (point_p->throw_velocity > GT_INDEV_THROW_LOW_SPEED_THRESHOLD) {
        /** Exponential decay: smooth deceleration with sliding feel */
        point_p->throw_velocity = (point_p->throw_velocity
            * (256 - GT_INDEV_THROW_FRICTION)) >> 8;
    } else {
        /** Linear deceleration: crisp stop without lingering */
        point_p->throw_velocity -= GT_INDEV_THROW_LINEAR_DECEL;
    }
    _throw_scroll_rebound_slow_down_handler(indev);
    if (point_p->throw_velocity <= 0) {
        point_p->throw_velocity = 0;
    }
#endif

    if (_is_scroll_vertical((gt_dir_et)point_p->gesture)) {
#if GT_INDEV_CFG_VELOCITY
        dir = point_p->scroll_sum.y < 0 ? -1 : 1;
        dy = _calc_distance_inline(dir * point_p->throw_velocity * detla_time);
#else
        point_p->scroll_throw.y = point_p->scroll_throw.y * (100 - indev->drv->limit_scroll_throw) / 100;
        gt_obj_scroll_to(obj_scroll, 0, point_p->scroll_throw.y, GT_ANIM_OFF);
        return stop_throw_anim;
#endif
    } else {
#if GT_INDEV_CFG_VELOCITY
        dir = point_p->scroll_sum.x < 0 ? -1 : 1;
        dx = _calc_distance_inline(dir * point_p->throw_velocity * detla_time);
#else
        point_p->scroll_throw.x = point_p->scroll_throw.x * (100 - indev->drv->limit_scroll_throw) / 100;
        gt_obj_scroll_to(obj_scroll, point_p->scroll_throw.x, 0, GT_ANIM_OFF);
        return stop_throw_anim;
#endif
    }

#if GT_INDEV_CFG_VELOCITY
    if (false == point_p->over_scroll.is_over_scroll && obj_scroll->rebound) {
        // limit max scroll distance to border
        detla_range = gt_obj_calc_scroll_limit_detla_distance(obj_scroll, dx, dy);
        dx = detla_range.x;
        dy = detla_range.y;
    }
    indev->timestamp_throw_prev_scroll = cur_time;

    if (obj_scroll->absorb) {
        stop_throw_anim = _calc_absorb_next_scroll_distance(point_p, obj_scroll, &dx, &dy);
    }
    gt_obj_scroll_to(obj_scroll, dx, dy, GT_ANIM_OFF);
#endif
    return stop_throw_anim;
}

static GT_ATTRIBUTE_RAM_TEXT inline bool
_indev_is_scroll(gt_indev_st * indev) {
    struct _point * point_p = &indev->data.point;
    gt_size_t limit = indev->drv->limit_pixel_scroll;
    if (gt_abs(point_p->scroll_sum.x) >= limit) {
        return true;
    }
    if (gt_abs(point_p->scroll_sum.y) >= limit) {
        return true;
    }
    return false;
}

static GT_ATTRIBUTE_RAM_TEXT inline bool
_indev_is_scroll_throw(gt_indev_st * indev) {
    struct _point * point_p = &indev->data.point;
#if GT_INDEV_CFG_VELOCITY
    if (point_p->throw_velocity <= 0) {
        return false;
    }
#endif
    return point_p->scroll_throw.x || point_p->scroll_throw.y ? true : false;
}

static GT_ATTRIBUTE_RAM_TEXT inline void
_indev_scroll_throw_reset(gt_indev_st * indev) {
    indev->scroll_throw_anim = NULL;
    indev->data.point.scroll_throw.x = 0;
    indev->data.point.scroll_throw.y = 0;
}

static GT_ATTRIBUTE_RAM_TEXT void _indev_scroll_throw_deleted_cb(struct gt_anim_s * anim) {
    gt_indev_st * dev_p = (gt_indev_st * )anim->tar;
    struct _point * point_p = &dev_p->data.point;

#if GT_INDEV_CFG_VELOCITY
    _indev_rebound_recover_handler(dev_p, point_p->obj_scroll);
#endif
    _indev_scroll_throw_reset(dev_p);
    point_p->obj_scroll = NULL;
}

static GT_ATTRIBUTE_RAM_TEXT gt_event_type_et
_get_scroll_direct_event_type_by(gt_dir_et gesture) {
    char scroll_dir[] = {
        [GT_DIR_NONE]  = GT_EVENT_TYPE_NONE,
        [GT_DIR_UP]    = GT_EVENT_TYPE_INPUT_SCROLL_DOWN,
        [GT_DIR_DOWN]  = GT_EVENT_TYPE_INPUT_SCROLL_UP,
        [GT_DIR_LEFT]  = GT_EVENT_TYPE_INPUT_SCROLL_RIGHT,
        [GT_DIR_RIGHT] = GT_EVENT_TYPE_INPUT_SCROLL_LEFT,
    };
    if ((int)gesture >= _GT_DIR_MAX_COUNT) {
        return GT_EVENT_TYPE_NONE;
    }
    return (int)gesture < _GT_DIR_MAX_COUNT ? scroll_dir[gesture] : GT_EVENT_TYPE_NONE;
}

static GT_ATTRIBUTE_RAM_TEXT gt_event_type_et
_get_swipe_direct_event_type_by(gt_dir_et gesture) {
    char swipe_dir[] = {
        [GT_DIR_NONE]  = GT_EVENT_TYPE_NONE,
        [GT_DIR_UP]    = GT_EVENT_TYPE_SWIPE_UP,
        [GT_DIR_DOWN]  = GT_EVENT_TYPE_SWIPE_DOWN,
        [GT_DIR_LEFT]  = GT_EVENT_TYPE_SWIPE_LEFT,
        [GT_DIR_RIGHT] = GT_EVENT_TYPE_SWIPE_RIGHT,
    };
    if ((int)gesture >= _GT_DIR_MAX_COUNT) {
        return GT_EVENT_TYPE_NONE;
    }
    return (int)gesture < _GT_DIR_MAX_COUNT ? swipe_dir[gesture] : GT_EVENT_TYPE_NONE;
}

static inline void GT_ATTRIBUTE_RAM_TEXT
_scroll_raw_handler(struct _point * point_p, gt_size_t dx, gt_size_t dy) {
    if (_is_scroll_vertical((gt_dir_et)point_p->gesture)) {
        dx = 0;
        point_p->scroll_throw.x = 0;
        gt_obj_set_scroll_ud(point_p->obj_scroll, GT_DIR_DOWN == point_p->gesture ? true : false);
    } else {
        dy = 0;
        point_p->scroll_throw.y = 0;
        gt_obj_set_scroll_lr(point_p->obj_scroll, GT_DIR_RIGHT == point_p->gesture ? true : false);
    }
    gt_obj_scroll_to(point_p->obj_scroll, dx, dy, GT_ANIM_OFF);
}

static void GT_ATTRIBUTE_RAM_TEXT
_indev_scroll_throw_start_cb(struct gt_anim_s * anim) {
    gt_indev_st * dev_p = (gt_indev_st * )anim->tar;
    struct _point * point_p = &dev_p->data.point;
    gt_obj_st * obj = (gt_obj_st * )point_p->obj_scroll;
    bool is_scroll = _indev_is_scroll(dev_p);

    if (GT_TYPE_UNKNOWN == gt_obj_class_get_type(obj)) {
        return;
    }
    if (false == obj->ignore_throw) {
        return;
    }
    if (false == is_scroll) { return; }
    /**
     * Using sum distance replace throw anim handle,
     * Throw scroll handler immediately by ignore_throw
     */
    _scroll_raw_handler(point_p, point_p->scroll_sum.x, point_p->scroll_sum.y);
    gt_event_send(obj, GT_EVENT_TYPE_INPUT_SCROLL_END, NULL);
    _indev_scroll_throw_deleted_cb(anim);
    gt_anim_del_safe_by(dev_p);
}

static GT_ATTRIBUTE_RAM_TEXT void _indev_scroll_throw_exec_cb(void * indev, int32_t v) {
    GT_UNUSED(v);
    gt_indev_st * ptr = (gt_indev_st *)indev;
    struct _point * point_p = &ptr->data.point;
    gt_obj_st * obj_scroll = point_p->obj_scroll;
    gt_point_st prev_scroll_pos = obj_scroll->process_attr.scroll;

    if (_indev_scroll_throw_handler(ptr)) {
        goto reset_stop_anim_lb;
    }
    /** pos has not change, stop anim right now */
    if (prev_scroll_pos.x == obj_scroll->process_attr.scroll.x &&
        prev_scroll_pos.y == obj_scroll->process_attr.scroll.y) {
        goto reset_stop_anim_lb;
    }
    if (GT_DIR_NONE == ptr->data.point.gesture) {
        goto stop_anim_lb;
    }
#if GT_INDEV_CFG_VELOCITY
    if (false == _indev_is_scroll_throw(indev)) {
        goto stop_anim_lb;
    }
#else
    if (0 == point_p->scroll_throw.x && 0 == point_p->scroll_throw.y) {
        goto stop_anim_lb;
    }
#endif
    return;

reset_stop_anim_lb:
    point_p->scroll_throw.x = 0;
    point_p->scroll_throw.y = 0;
stop_anim_lb:
    gt_anim_set_time(ptr->scroll_throw_anim, 0);
}

static inline void GT_ATTRIBUTE_RAM_TEXT
_indev_send_props_swipe_event(struct _point * point_p,
    _indev_obj_props_handler_st const * const props_hdl) {
    if (0 == props_hdl->val) {
        return;
    }
    gt_event_type_et e_type = props_hdl->get_direct_event_type_cb((gt_dir_et)point_p->gesture);
    if (GT_EVENT_TYPE_NONE == e_type) {
        return;
    }
    gt_event_send(point_p->obj_scroll, e_type, NULL);
}

static void GT_ATTRIBUTE_RAM_TEXT
_indev_send_finger_unfollow_event(struct _point * point_p) {
    _indev_obj_props_handler_st prop = {
        .val = point_p->obj_scroll->finger_unfollow,
        .get_direct_event_type_cb = _get_scroll_direct_event_type_by,
    };
    _indev_send_props_swipe_event(point_p, &prop);
}

static void GT_ATTRIBUTE_RAM_TEXT
_indev_send_swipe_event(struct _point * point_p) {
    _indev_obj_props_handler_st prop = {
    .val = point_p->obj_scroll->swipe_dir,
    .get_direct_event_type_cb = _get_swipe_direct_event_type_by,
    };
    _indev_send_props_swipe_event(point_p, &prop);
}

static void GT_ATTRIBUTE_RAM_TEXT
_indev_scroll_snap(struct _point * point_p) {
    gt_obj_st * scroll_obj = point_p->obj_scroll;
    gt_size_t dx = 0, dy = 0;
    if (_is_scroll_vertical((gt_dir_et)point_p->gesture)) {
        if (GT_SCROLL_SNAP_NONE == gt_obj_get_scroll_snap_y(scroll_obj)) {
            goto non_snap;
        }
    } else {
        if (GT_SCROLL_SNAP_NONE == gt_obj_get_scroll_snap_x(scroll_obj)) {
            goto non_snap;
        }
    }

    if (false == scroll_obj->absorb) {
        return;
    }
    _absorb_handler(scroll_obj, point_p, &dx, &dy);
    gt_obj_scroll_to_fast(scroll_obj, dx, dy, GT_ANIM_ON);
    return;

non_snap:
    gt_event_send(scroll_obj, GT_EVENT_TYPE_INPUT_SCROLL_END, NULL);
}

static void GT_ATTRIBUTE_RAM_TEXT
_indev_scroll_throw_ready_cb(struct gt_anim_s * anim) {
    gt_indev_st * dev_p = (gt_indev_st * )anim->tar;
    struct _point * point_p = &dev_p->data.point;
    bool is_scroll_throw = _indev_is_scroll_throw(dev_p);

    _indev_scroll_throw_reset(dev_p);
    if (false == gt_obj_check_scr(point_p->obj_scroll)) {
        return ;
    }
    if (false == is_scroll_throw) {
        _indev_send_finger_unfollow_event(point_p);
    }
    _indev_send_swipe_event(point_p);
    _indev_scroll_snap(point_p);
}

static GT_ATTRIBUTE_RAM_TEXT inline void
_indev_scroll_throw_handler_anim_start(gt_indev_st * indev) {
    gt_anim_st anim = {0};
    gt_anim_init(&anim);

#if GT_INDEV_CFG_VELOCITY
    int32_t delta_time = gt_tick_get() - indev->timestamp_prev_press;
    if (delta_time) {
        _velocity_less_scroll_points_need_to_speed_up(indev);
        _calc_throw_velocity(indev);
    }

    gt_anim_set_time(&anim, 50000);
#else
    gt_anim_set_time(&anim, 2000);
#endif
    gt_anim_set_target(&anim, indev);
    gt_anim_set_value(&anim, 0, 1024);
    gt_anim_set_start_cb(&anim, _indev_scroll_throw_start_cb);
    gt_anim_set_exec_cb(&anim, _indev_scroll_throw_exec_cb);
    gt_anim_set_ready_cb(&anim, _indev_scroll_throw_ready_cb);
    gt_anim_set_deleted_cb(&anim, _indev_scroll_throw_deleted_cb);
    indev->scroll_throw_anim = gt_anim_start(&anim);
}

/**
 * @brief Scroll when pressing
 *
 * @param indev
 */
static GT_ATTRIBUTE_RAM_TEXT void _indev_scroll_handler(gt_indev_st * indev) {
    struct _point * point_p = &indev->data.point;

    point_p->scroll_sum.x += point_p->scroll_diff.x;
    point_p->scroll_sum.y += point_p->scroll_diff.y;
    point_p->gesture = _get_scroll_dir(indev);

    if ((uint16_t)GT_DIR_NONE == point_p->gesture) { return; }

    if (NULL == point_p->obj_scroll) {
        if (point_p->obj_target->fixed) {
            /** if widget fixed, then search it parent's widget to scroll */
            point_p->obj_target = _search_not_fixed_by_parent_recursive(point_p->obj_target);
        }
        gt_event_send(point_p->obj_target, GT_EVENT_TYPE_INPUT_SCROLL_START, NULL);
    }
    point_p->obj_scroll = point_p->obj_target;

    if (_home_gesture_scroll_effects(indev)) {
        return;
    }

    if (GT_SCROLL_DISABLE == point_p->obj_scroll->scroll_dir) {
        point_p->obj_scroll = NULL;
        return;
    }

    if (point_p->obj_scroll->finger_unfollow) { return ; }

#if GT_INDEV_CFG_VELOCITY
    _calc_throw_velocity(indev);
#if GT_CFG_ENABLE_INDEV_EFFECTS
    if (point_p->obj_scroll->rebound) {
        _indev_over_scroll_ret_st os_ret = _scrolling_over_limit_area_effects(indev);
        if (os_ret.is_over_scroll) {
            _scroll_raw_handler(point_p,
                os_ret.distance.x - point_p->over_scroll_prev.x,
                os_ret.distance.y - point_p->over_scroll_prev.y);
            point_p->over_scroll_prev = os_ret.distance;
            return;
        }
    }
#endif  /** GT_CFG_ENABLE_INDEV_EFFECTS */
#endif

    _scroll_raw_handler(point_p, point_p->scroll_diff.x, point_p->scroll_diff.y);
}

static inline void GT_ATTRIBUTE_RAM_TEXT
_send_released_event(struct _point * point_p) {
    gt_event_type_et event_type = GT_EVENT_TYPE_INPUT_RELEASED;

    if (point_p->obj_act != point_p->obj_target) {
        event_type = GT_EVENT_TYPE_INPUT_PRESS_LOST;
    }

    if (gt_obj_is_untouchability(point_p->obj_target)) {
        return;
    }
    if (point_p->breaking_scrolling) {
        event_type = GT_EVENT_TYPE_INPUT_PRESS_LOST;
    }

    gt_event_send(point_p->obj_target, event_type, NULL);
#if GT_USE_LAYER_TOP
    gt_global_event_send(event_type, NULL);
#endif

    if (point_p->obj_origin != point_p->obj_target) {
        gt_event_send(point_p->obj_origin, GT_EVENT_TYPE_INPUT_PRESS_LOST, NULL);
    }
}

static GT_ATTRIBUTE_RAM_TEXT inline void _scroll_calc_throw_distance(struct _point * point_p) {
    gt_point_st diff = {0};
    diff.x = point_p->newly.x - point_p->last.x;
    diff.y = point_p->newly.y - point_p->last.y;

    point_p->scroll_diff = diff;
    if (diff.x || diff.y) {
        point_p->scroll_throw.x = (point_p->scroll_throw.x + diff.x) >> 1;
        point_p->scroll_throw.y = (point_p->scroll_throw.y + diff.y) >> 1;
#if GT_INDEV_CFG_VELOCITY
        ++point_p->diff_points;
#endif
    }
}

#if GT_INDEV_SCROLL_THROW_SPEED_UP_VER && !GT_INDEV_SCROLL_THROW_SPEED_UP_HOR
static GT_ATTRIBUTE_RAM_TEXT inline void _scroll_calc_throw_distance_ver(struct _point * point_p) {
    point_p->scroll_diff.y = point_p->newly.y - point_p->last.y;
    point_p->scroll_throw.y = (point_p->scroll_throw.y + point_p->scroll_diff.y) >> 1;
}
#endif

#if GT_INDEV_SCROLL_THROW_SPEED_UP_HOR && !GT_INDEV_SCROLL_THROW_SPEED_UP_VER
static GT_ATTRIBUTE_RAM_TEXT inline void _scroll_calc_throw_distance_hor(struct _point * point_p) {
    point_p->scroll_diff.x = point_p->newly.x - point_p->last.x;
    point_p->scroll_throw.x = (point_p->scroll_throw.x + point_p->scroll_diff.x) >> 1;
}
#endif

static GT_ATTRIBUTE_RAM_TEXT void
_indev_released_handle(gt_indev_st * indev) {
    struct _point * point_p = &indev->data.point;
    if (NULL == point_p->obj_target) {
        goto clr_lb;
    }
    bool is_home_gesture = point_p->obj_scroll ? _indev_send_home_gesture_event(indev) : false;

#if GT_INDEV_POINT_LOG
    GT_LOG_A(GT_LOG_TAG_TP, "release %p (%d, %d)", point_p->obj_target, point_p->newly.x, point_p->newly.y);
#endif

    if (false == is_home_gesture) {
        _send_released_event(point_p);
    }
    point_p->obj_act = NULL;

    if (NULL == point_p->obj_scroll) {
#if GT_INDEV_CFG_VELOCITY
        _indev_rebound_recover_handler(indev, point_p->obj_target);
#endif
        goto tar_clr_lb;
    }
#if GT_INDEV_SCROLL_THROW_SPEED_UP
#if GT_INDEV_SCROLL_THROW_SPEED_UP_VER && GT_INDEV_SCROLL_THROW_SPEED_UP_HOR
    _scroll_calc_throw_distance(point_p);
#elif GT_INDEV_SCROLL_THROW_SPEED_UP_VER
    _scroll_calc_throw_distance_ver(point_p);
#elif GT_INDEV_SCROLL_THROW_SPEED_UP_HOR
    _scroll_calc_throw_distance_hor(point_p);
#endif
#endif
    if (is_home_gesture) {
        _indev_scroll_throw_reset(indev);
    } else {
        _indev_scroll_throw_handler_anim_start(indev);
    }

tar_clr_lb:
    point_p->obj_target = NULL;
clr_lb:
    point_p->obj_origin = NULL;
    point_p->is_home_gesture = false;
#if GT_INDEV_CFG_VELOCITY
    point_p->diff_points = 0;
#endif
    point_p->breaking_scrolling = false;

#if GT_CFG_ENABLE_INDEV_EFFECTS
    gt_indev_effects_hide();
#endif
}

/**
 * @brief Objects outside the control cannot be activated
 *
 * @return true can not be activated
 * @return false can be activated
 */
static GT_ATTRIBUTE_RAM_TEXT bool _obj_outside_can_not_be_activated(void) {
#if GT_CFG_ENABLE_DIALOG
    if (gt_dialog_has_showing()) {
        return true;
    }
#endif
    return false;
}

/**
 * @brief Search the active object from all layers
 *
 * @param point physical point
 * @param new_action true: new action, false: continue action
 * @return gt_obj_st* NULL or active object
 */
static gt_obj_st * GT_ATTRIBUTE_RAM_TEXT
_search_active_obj_from_all_layers(gt_point_st * point, bool new_action) {
    gt_obj_st * scr = gt_disp_get_scr();
    gt_obj_st * ret = NULL;
#if GT_USE_LAYER_TOP
    gt_obj_st * layer_top = gt_disp_get_layer_top();

    if (layer_top->cnt_child) {
        ret = gt_find_clicked_obj_by_point(layer_top, point);
        if (true == new_action && NULL == ret) {
            /** clicked dialog, etc. widget's outside area, hided top widgets */
            _gt_disp_hided_layer_top_widgets(layer_top);
            if (_obj_outside_can_not_be_activated()) {
                return ret;
            }
        }
    }
#endif
    if (NULL == ret) {
        /** Layer top have no widget find out, search it within screen */
        ret = gt_find_clicked_obj_by_point(scr, point);
    }
    return ret;
}

static GT_ATTRIBUTE_RAM_TEXT uint32_t _indev_reset_start_timestamp(gt_indev_st * indev) {
    indev->timestamp_start = gt_tick_get();
    indev->timestamp_pressing = indev->timestamp_start;
    indev->timestamp_long_pressed = indev->timestamp_start;
#if GT_INDEV_CFG_VELOCITY
    indev->timestamp_throw_prev_scroll = indev->timestamp_start;
#endif
    return indev->timestamp_start;
}

static GT_ATTRIBUTE_RAM_TEXT inline bool _is_pressing_handler_after_long_pressed(gt_indev_st * indev) {
    return indev->timestamp_long_pressed != indev->timestamp_start;
}

static GT_ATTRIBUTE_RAM_TEXT inline bool _is_pressing_timeout(gt_indev_st * indev) {
    return (gt_tick_get() - indev->timestamp_pressing) >= indev->drv->limit_time_pressing;
}

static GT_ATTRIBUTE_RAM_TEXT inline bool _is_long_pressed_timeout(gt_indev_st * indev) {
    return indev->timestamp_long_pressed == indev->timestamp_start &&
        (gt_tick_get() - indev->timestamp_start) >= indev->drv->limit_time_long_pressed;
}

static inline void GT_ATTRIBUTE_RAM_TEXT
_touch_reset_base_info(gt_indev_st * indev) {
    struct _point * point_p = &indev->data.point;
    _indev_reset_start_timestamp(indev);

    _gt_indev_point_set_value(&point_p->scroll_sum, 0, 0);
    _gt_indev_point_set_value(&point_p->scroll_diff, 0, 0);
    /* save pressed obj */
    point_p->gesture = GT_DIR_NONE;
    point_p->is_home_gesture = false;
#if GT_INDEV_CFG_VELOCITY
    _gt_indev_point_set_value(&point_p->over_scroll_prev, 0, 0);
    point_p->throw_velocity = 0;
    point_p->diff_points = 0;
#endif
    if (indev->scroll_throw_anim) {
        if (point_p->obj_scroll) {
            /** End of prev scroll event */
            gt_event_send(point_p->obj_scroll, GT_EVENT_TYPE_INPUT_SCROLL_END, NULL);
            point_p->obj_scroll = NULL;
        }
        gt_anim_set_paused(indev->scroll_throw_anim, true);
        gt_anim_del_by(indev->scroll_throw_anim);
        _indev_scroll_throw_reset(indev);
        indev->data.point.breaking_scrolling = true;
    }
}

static GT_ATTRIBUTE_RAM_TEXT void _indev_pressed_handle(gt_indev_st * indev) {
    gt_point_st point_ret;
    struct _point * point_p = &indev->data.point;

    if (NULL == point_p->obj_target) {
        if (point_p->obj_origin) {
            /** prev screen or the last complete process did not complete */
            return;
        }
        _touch_reset_base_info(indev);
        point_p->obj_act = _search_active_obj_from_all_layers(&point_p->newly, true);
        if (NULL == point_p->obj_act) { return; }
        if (gt_obj_is_untouchability(point_p->obj_act)) {
            return;
        }
        point_p->obj_target = point_p->obj_act;
        point_p->obj_origin = point_p->obj_target;

        /* send clicking event to pressed obj */
        gt_obj_get_click_point_by_phy_point(point_p->obj_act, &point_p->newly, &point_ret);
        _gt_indev_point_set_value(&point_p->act, point_p->newly.x, point_p->newly.y);
        _gt_indev_point_set_value(&point_p->last, point_p->newly.x, point_p->newly.y);
        _gt_indev_point_set_value(&point_p->scroll_absorb_max_dist, 0, 0);
        _gt_obj_set_process_point(point_p->obj_act, &point_ret);
        point_p->home_gesture_area = _is_first_point_at_home_gesture_area(&point_p->act);
        gt_event_send(point_p->obj_act, GT_EVENT_TYPE_INPUT_PRESSED, NULL);
#if GT_USE_LAYER_TOP
        gt_global_event_send(GT_EVENT_TYPE_INPUT_PRESSED, NULL);
#endif
    }

    if (point_p->obj_target && point_p->obj_scroll) {
        /** Reset scroll throw data, prepare for next throw. */
        point_p->scroll_throw.x = 0;
        point_p->scroll_throw.y = 0;
    }

    /* 持续按压帧：重新搜索当前坐标下的对象（检测手指是否移到其他对象）
     * 新按下帧（obj_act 已在上方初始化块中赋值）跳过此次遍历 */
    if (point_p->obj_target) {
        point_p->obj_act = _search_active_obj_from_all_layers(&point_p->newly, false);
    }

#if GT_INDEV_POINT_LOG
    GT_LOG_A(GT_LOG_TAG_TP, "press %p(%d) %p(%d) (%d, %d) diff: (%d, %d)",
                point_p->obj_act, gt_obj_class_get_type(point_p->obj_act),
                point_p->obj_target, gt_obj_class_get_type(point_p->obj_target),
                point_p->newly.x, point_p->newly.y,
                point_p->newly.x - point_p->last.x, point_p->newly.y - point_p->last.y);
#endif

    if (point_p->obj_act == point_p->obj_target) {
        /** Hold for a long time to trigger multiple times */
        gt_obj_get_click_point_by_phy_point(point_p->obj_target, &point_p->newly, &point_ret);
        _gt_obj_set_process_point(point_p->obj_target, &point_ret);

        if (_is_pressing_handler_after_long_pressed(indev)) {
            if (_is_pressing_timeout(indev)) {
                indev->timestamp_pressing = gt_tick_get();
                gt_event_send(point_p->obj_target, GT_EVENT_TYPE_INPUT_PRESSING, NULL);
#if GT_USE_LAYER_TOP
                gt_global_event_send(GT_EVENT_TYPE_INPUT_PRESSING, NULL);
#endif
            }
        } else if (_is_long_pressed_timeout(indev)) {
            indev->timestamp_long_pressed = gt_tick_get();
            indev->timestamp_pressing = indev->timestamp_long_pressed;  /** skip pressing time to toggle */
            gt_event_send(point_p->obj_target, GT_EVENT_TYPE_INPUT_LONG_PRESSED, NULL);
#if GT_USE_LAYER_TOP
            gt_global_event_send(GT_EVENT_TYPE_INPUT_LONG_PRESSED, NULL);
#endif
        }
    }
    _scroll_calc_throw_distance(point_p);

    if (NULL == point_p->obj_target) { return ; }
    _indev_scroll_handler(indev);
}

static GT_ATTRIBUTE_RAM_TEXT void _gt_indev_handler_point(gt_indev_st * indev) {
    struct _point * point_p = &indev->data.point;
    gt_indev_data_st data_indev = {
        .point.x = point_p->newly.x,
        .point.y = point_p->newly.y,
        .state = GT_INDEV_STATE_INVALID,
    };

    /* call user hard interface */
    indev->drv->read_cb(indev->drv, &data_indev);
    if (GT_INDEV_STATE_INVALID == data_indev.state) {
        return;
    }

    /* transform physical touch coordinates to logical (UI) coordinates */
    gt_disp_st * _disp = gt_disp_get_default();
    if (_disp && GT_ROTATED_0 != (gt_rotated_et)_disp->drv->rotated) {
        _rotate_point_inv((gt_rotated_et)_disp->drv->rotated,
                          _disp->drv->res_hor, _disp->drv->res_ver,
                          &data_indev.point.x, &data_indev.point.y);
    }

    if (data_indev.point.x > gt_disp_get_res_hor(NULL) ||
        data_indev.point.y > gt_disp_get_res_ver(NULL)) {
        // GT_LOGW(GT_LOG_TAG_TP, "point out of range: x:%d, y:%d", data_indev.point.x, data_indev.point.y);
        return;
    }

    /* process point data */
    indev->state = data_indev.state;

    _indev_params.param.point.x = data_indev.point.x;
    _indev_params.param.point.y = data_indev.point.y;
    _gt_indev_point_set_value(&point_p->newly, data_indev.point.x, data_indev.point.y);

    /* have one touch event */
    if (GT_INDEV_STATE_RELEASED == indev->state) {
        _indev_released_handle(indev);
    } else {
        _indev_pressed_handle(indev);
    }

    _gt_indev_point_set_value(&point_p->last, point_p->newly.x, point_p->newly.y);
}

static GT_ATTRIBUTE_RAM_TEXT void _gt_indev_handler_button(gt_indev_st * indev) {
    static uint8_t flag_key_down = 0;
    static uint32_t last_run = 0;
    gt_indev_data_st data_indev = {
        .btn_id = indev->data.button.id,
        .state = GT_INDEV_STATE_INVALID,
    };
    indev->drv->read_cb(indev->drv, &data_indev);
    if (GT_INDEV_STATE_INVALID == data_indev.state) {
        return;
    }

    indev->state = data_indev.state;
    if (indev->state == GT_INDEV_STATE_PRESSED) {
        if (!flag_key_down) {
            flag_key_down = 1;
            last_run = gt_tick_get();
            indev->data.button.id = data_indev.btn_id;
            GT_LOGV(GT_LOG_TAG_GUI, "keydown the key id[%d]", indev->data.button.id);
        }
        indev->data.button.count_keydown++;
    }
    else {
        flag_key_down = 0;
        indev->data.button.count_keydown = 0;
    }

    if (indev->data.button.count_keydown != 0 && (gt_tick_get() - last_run) % indev->drv->limit_time_pressing == 0) {
        /* trigger an button event:continue mode */
        indev->data.button.count_keydown = 0;
        GT_LOGV(GT_LOG_TAG_GUI, "continue mode trigger an button event, key id[%d]",indev->data.button.id );
    }
}

#if GT_USE_LAYER_TOP
/**
 * @brief Whether the key is a global key value, need to be captured by the top layer
 *
 * @param key
 * @return true
 * @return false
 */
static GT_ATTRIBUTE_RAM_TEXT inline bool _kp_is_global_key_value(uint32_t key) {
    switch (key) {
        case GT_KEY_NEXT:
        case GT_KEY_PREV:
        case GT_KEY_ESC:
        case GT_KEY_BACK:
        case GT_KEY_MENU:
        case GT_KEY_ENTER: {
            return true;
        }
        default:
            break;
    }
    return false;
}
#endif


static GT_ATTRIBUTE_RAM_TEXT gt_obj_st* _find_keypad_target_obj(gt_indev_st* indev) {
    gt_obj_st * scr = gt_disp_get_scr();

    gt_obj_st* top_obj = NULL;
    gt_obj_st* scr_obj = gt_find_clicked_obj_by_focus(scr);

#if GT_USE_LAYER_TOP
    gt_obj_st* layer_top = gt_disp_get_layer_top();
    if(gt_disp_layer_top_has_active_obj()){
        top_obj = gt_find_clicked_obj_by_focus(layer_top);
    }

    if(top_obj){
        if(1 != indev->data.keypad.last_layer){
#if GT_USE_UD_LR_TO_CONTROL_FOCUS_EN
            _SET_KEYPAD_LOCK(indev, false);
#endif
            if(scr_obj != scr){
                gt_obj_set_focus(scr_obj, gt_obj_is_focus(scr_obj));
            }
            if(top_obj != layer_top){
                gt_obj_set_focus(top_obj, gt_obj_is_focus(top_obj));
            }
        }
    }
    else{
        if(0 != indev->data.keypad.last_layer) {
#if GT_USE_UD_LR_TO_CONTROL_FOCUS_EN
            _SET_KEYPAD_LOCK(indev, false);
#endif
            if(scr_obj != scr){
                gt_obj_set_focus(scr_obj, gt_obj_is_focus(scr_obj));
            }
        }
    }
#endif

    indev->data.keypad.last_layer = top_obj ? 1 : 0;


    return top_obj ? top_obj : scr_obj;
}

static GT_ATTRIBUTE_RAM_TEXT void _gt_indev_handler_keypad(gt_indev_st* indev) {
    gt_indev_data_st data_indev = {
        .key = GT_KEY_NONE,
        .state = GT_INDEV_STATE_INVALID,
    };
    static uint32_t last_key = GT_KEY_NONE;
#if GT_USE_UD_LR_TO_CONTROL_FOCUS_EN
    gt_obj_st * scr = gt_disp_get_scr();
#endif

    indev->drv->read_cb(indev->drv, &data_indev);
    if (GT_INDEV_STATE_INVALID == data_indev.state) {
        return;
    }

    indev->state = data_indev.state;
    gt_obj_st* layer_top = gt_disp_get_layer_top();
    gt_obj_st* obj_target = _find_keypad_target_obj(indev);


    if (GT_INDEV_STATE_RELEASED == indev->state) {
        if (GT_KEY_NONE == last_key) {
            last_key = GT_KEY_NONE;
            return;
        }

        _indev_params.param.keypad_key = indev->data.keypad.key;
#if GT_USE_LAYER_TOP
        if (_kp_is_global_key_value(indev->data.keypad.key)) {
            _gt_global_core_indev_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_KEY_RELEASED, &_indev_params);
        }
#endif
        gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_KEY_RELEASED, &_indev_params);

        if (GT_KEY_ENTER == indev->data.keypad.key &&
            false == gt_obj_is_untouchability(indev->data.keypad.obj_origin)) {
            gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_RELEASED, &_indev_params);
        }

#if GT_INDEV_KEYPAD_LOG
        GT_LOG_A(GT_LOG_TAG_TP, "keypad key: %#X, state: %d obj type: %d",
                    indev->data.keypad.key, indev->state, gt_obj_class_get_type(indev->data.keypad.obj_target));
#endif
        indev->data.keypad.obj_target = NULL;
        indev->data.keypad.obj_origin = NULL;
        last_key = GT_KEY_NONE;
        return;
    }
    // 新的按键
    if (GT_KEY_NONE == last_key || last_key != data_indev.key) {
        indev->data.keypad.key = data_indev.key;
        last_key = data_indev.key;
        indev->data.keypad.obj_target = obj_target;
        indev->data.keypad.obj_origin = obj_target;
        _indev_reset_start_timestamp(indev);
        _indev_params.param.keypad_key = indev->data.keypad.key;
#if GT_USE_UD_LR_TO_CONTROL_FOCUS_EN
        if(!_IS_KEYPAD_LOCKED(indev) && GT_KEY_RIGHT == indev->data.keypad.key){
            gt_obj_change_st chg = gt_obj_focus_change(indev->data.keypad.obj_origin, GT_FOCUS_DIR_RIGHT);
            if(!chg.dst) { goto _key_pressed; }
            gt_obj_focus_change_display(&chg);
            gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_FOCUSED, &_indev_params);
        }
        else if(!_IS_KEYPAD_LOCKED(indev) && GT_KEY_LEFT == indev->data.keypad.key){
            gt_obj_change_st chg = gt_obj_focus_change(indev->data.keypad.obj_origin, GT_FOCUS_DIR_LEFT);
            if(!chg.dst) { goto _key_pressed; }
            gt_obj_focus_change_display(&chg);
            gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_FOCUSED, &_indev_params);
        }
        else if(!_IS_KEYPAD_LOCKED(indev) && GT_KEY_DOWN == indev->data.keypad.key){
            gt_obj_change_st chg = gt_obj_focus_change(indev->data.keypad.obj_origin, GT_FOCUS_DIR_DOWN);
            if(!chg.dst) { goto _key_pressed; }
            gt_obj_focus_change_display(&chg);
            gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_FOCUSED, &_indev_params);
        }
        else if(!_IS_KEYPAD_LOCKED(indev) && GT_KEY_UP == indev->data.keypad.key){
            gt_obj_change_st chg = gt_obj_focus_change(indev->data.keypad.obj_origin, GT_FOCUS_DIR_UP);
            if(!chg.dst) { goto _key_pressed; }
            gt_obj_focus_change_display(&chg);
            gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_FOCUSED, &_indev_params);
        }
        else if (!_IS_KEYPAD_LOCKED(indev) && GT_KEY_ENTER == indev->data.keypad.key
                && scr != indev->data.keypad.obj_origin
#if GT_USE_LAYER_TOP
                && layer_top != indev->data.keypad.obj_origin
#endif
        ){
            _SET_KEYPAD_LOCK(indev, true);
            gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_FOCUSED, &_indev_params);
            gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_KEY, &_indev_params);
            gt_disp_invalid_area(indev->data.keypad.obj_origin);
        }
        else if (_IS_KEYPAD_LOCKED(indev) && GT_KEY_ESC == indev->data.keypad.key){
            _SET_KEYPAD_LOCK(indev, false);
            gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_FOCUSED, &_indev_params);
            gt_disp_invalid_area(indev->data.keypad.obj_origin);
        }
#else
        if(GT_KEY_NEXT == indev->data.keypad.key){
            gt_obj_change_st chg = gt_obj_next_focus_change(indev->data.keypad.obj_origin);
            gt_obj_focus_change_display(&chg);
            gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_FOCUSED, &_indev_params);
        }
        else if (GT_KEY_PREV == indev->data.keypad.key) {
            gt_obj_change_st chg = gt_obj_prev_focus_change(indev->data.keypad.obj_origin);
            gt_obj_focus_change_display(&chg);
            gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_FOCUSED, &_indev_params);
        }
#endif
        else {
#if GT_USE_UD_LR_TO_CONTROL_FOCUS_EN
_key_pressed:
#endif
#if GT_USE_LAYER_TOP
            if(GT_KEY_ESC == indev->data.keypad.key &&
                (gt_obj_is_child(indev->data.keypad.obj_origin, layer_top) || indev->data.keypad.obj_origin == layer_top)
            ){
                _gt_disp_hided_layer_top_widgets(layer_top);
                return ;
            }
            if (_kp_is_global_key_value(indev->data.keypad.key)) {
                _gt_global_core_indev_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_KEY, &_indev_params);
            }
#endif
            gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_KEY, &_indev_params);

            if (GT_KEY_ENTER == indev->data.keypad.key) {
#if GT_USE_UD_LR_TO_CONTROL_FOCUS_EN
                gt_obj_change_st chg;
                chg.dst = NULL;
#else
                gt_obj_change_st chg = gt_obj_into_focus_change(indev->data.keypad.obj_origin);
#endif
                if(!chg.dst && false == gt_obj_is_untouchability(indev->data.keypad.obj_origin)) {
                    gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_PRESSED, &_indev_params);
                }
            }
#if !GT_USE_UD_LR_TO_CONTROL_FOCUS_EN
            if (GT_KEY_ESC == indev->data.keypad.key) {
                gt_obj_out_focus_change(indev->data.keypad.obj_origin);
            }
#endif
        }
#if GT_INDEV_KEYPAD_LOG
        GT_LOG_A(GT_LOG_TAG_TP, "keypad key: %#X, state: %d obj type: %d",
                    indev->data.keypad.key, indev->state, gt_obj_class_get_type(indev->data.keypad.obj_origin));
#endif
        return ;
    }
    // 长按
    if (_is_pressing_handler_after_long_pressed(indev)) {
        if (_is_pressing_timeout(indev)) {
            indev->timestamp_pressing = gt_tick_get();
            _indev_params.param.keypad_key = indev->data.keypad.key;
#if GT_USE_UD_LR_TO_CONTROL_FOCUS_EN
            if(!_IS_KEYPAD_LOCKED(indev) && GT_KEY_RIGHT == indev->data.keypad.key){
                gt_obj_change_st chg = gt_obj_focus_change(indev->data.keypad.obj_origin, GT_FOCUS_DIR_RIGHT);
                if(!chg.dst){ goto _key_pressing; }
                gt_obj_focus_change_display(&chg);
                gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_FOCUSED, &_indev_params);
            }
            else if(!_IS_KEYPAD_LOCKED(indev) && GT_KEY_LEFT == indev->data.keypad.key){
                gt_obj_change_st chg = gt_obj_focus_change(indev->data.keypad.obj_origin, GT_FOCUS_DIR_LEFT);
                if(!chg.dst){ goto _key_pressing; }
                gt_obj_focus_change_display(&chg);
                gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_FOCUSED, &_indev_params);
            }
            else if(!_IS_KEYPAD_LOCKED(indev) && GT_KEY_DOWN == indev->data.keypad.key){
                gt_obj_change_st chg = gt_obj_focus_change(indev->data.keypad.obj_origin, GT_FOCUS_DIR_DOWN);
                if(!chg.dst){ goto _key_pressing; }
                gt_obj_focus_change_display(&chg);
                gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_FOCUSED, &_indev_params);
            }
            else if(!_IS_KEYPAD_LOCKED(indev) && GT_KEY_UP == indev->data.keypad.key){
                gt_obj_change_st chg = gt_obj_focus_change(indev->data.keypad.obj_origin, GT_FOCUS_DIR_UP);
                if(!chg.dst){ goto _key_pressing; }
                gt_obj_focus_change_display(&chg);
                gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_FOCUSED, &_indev_params);
            }
            else if (!_IS_KEYPAD_LOCKED(indev) && GT_KEY_ENTER == indev->data.keypad.key
                && scr != indev->data.keypad.obj_origin
#if GT_USE_LAYER_TOP
                && layer_top != indev->data.keypad.obj_origin
#endif
            ){
                _SET_KEYPAD_LOCK(indev, true);
                gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_FOCUSED, &_indev_params);
                gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_KEY, &_indev_params);
                gt_disp_invalid_area(indev->data.keypad.obj_origin);
            }
            else if (_IS_KEYPAD_LOCKED(indev) && GT_KEY_ESC == indev->data.keypad.key){
                _SET_KEYPAD_LOCK(indev, false);
                gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_FOCUSED, &_indev_params);
                gt_disp_invalid_area(indev->data.keypad.obj_origin);
            }
#else
            if(GT_KEY_NEXT == indev->data.keypad.key){
                gt_obj_change_st chg = gt_obj_next_focus_change(indev->data.keypad.obj_origin);
                gt_obj_focus_change_display(&chg);
                gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_FOCUSED, &_indev_params);
            }
            else if (GT_KEY_PREV == indev->data.keypad.key) {
                gt_obj_change_st chg = gt_obj_prev_focus_change(indev->data.keypad.obj_origin);
                gt_obj_focus_change_display(&chg);
                gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_FOCUSED, &_indev_params);
            }
#endif
            else {
#if GT_USE_UD_LR_TO_CONTROL_FOCUS_EN
_key_pressing:
#endif

#if GT_USE_LAYER_TOP
                if (_kp_is_global_key_value(indev->data.keypad.key)) {
                    _gt_global_core_indev_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_KEY_PRESSING, &_indev_params);
                }
#endif
                gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_KEY_PRESSING, &_indev_params);

                if (GT_KEY_ENTER == indev->data.keypad.key &&
                    false == gt_obj_is_untouchability(indev->data.keypad.obj_origin)) {
                    gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_PRESSING, &_indev_params);
                }
            }
        }
#if GT_INDEV_KEYPAD_LOG
        GT_LOG_A(GT_LOG_TAG_TP, "pressing key: %#X", indev->data.keypad.key);
#endif
    }
    else if (_is_long_pressed_timeout(indev)) {
        indev->timestamp_long_pressed = gt_tick_get();
        indev->timestamp_pressing = indev->timestamp_long_pressed;  /** skip pressing time to toggle */

        _indev_params.param.keypad_key = indev->data.keypad.key;
#if GT_USE_LAYER_TOP
        if (_kp_is_global_key_value(indev->data.keypad.key)) {
            _gt_global_core_indev_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_KEY_LONG_PRESSED, &_indev_params);
        }
#endif
        gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_KEY_LONG_PRESSED, &_indev_params);

        if (GT_KEY_ENTER == indev->data.keypad.key &&
            false == gt_obj_is_untouchability(indev->data.keypad.obj_origin)) {
            gt_event_send(indev->data.keypad.obj_origin, GT_EVENT_TYPE_INPUT_LONG_PRESSED, &_indev_params);
        }

#if GT_INDEV_KEYPAD_LOG
        GT_LOG_A(GT_LOG_TAG_TP, "long key: %#X", indev->data.keypad.key);
#endif
    }
}

/* global functions / API interface -------------------------------------*/
uint32_t gt_indev_get_key(void)
{
    return _indev_params.param.keypad_key;
}

gt_point_st gt_indev_get_point(void)
{
    return _indev_params.param.point;
}

void gt_indev_get_point_act(const gt_indev_st * indev, gt_point_st * point)
{
    point->x = indev->data.point.act.x;
    point->y = indev->data.point.act.y;
}

void gt_indev_get_point_newly(const gt_indev_st * indev, gt_point_st * point)
{
    point->x = indev->data.point.newly.x;
    point->y = indev->data.point.newly.y;
}

gt_dir_et gt_indev_get_gesture(const gt_indev_st * indev) {
    return (gt_dir_et)indev->data.point.gesture;
}

void gt_indev_handler(struct _gt_timer_s * timer)
{
    gt_indev_st * indev;
    uint8_t i, cnt_indev;

    if (false == gt_indev_is_enabled()) {
        return;
    }

    cnt_indev = gt_indev_get_dev_count();
    for(i=0; i<cnt_indev; i++){
        indev = gt_indev_get_dev_by_idx(i);
        switch(indev->drv->type){
            case GT_INDEV_TYPE_POINTER:
                _gt_indev_handler_point(indev);
                break;
            case GT_INDEV_TYPE_BUTTON:
                _gt_indev_handler_button(indev);
                break;
            case GT_INDEV_TYPE_KEYPAD:
                _gt_indev_handler_keypad(indev);
                break;
            default:
                break;
        }
    }
}

#if GT_USE_UD_LR_TO_CONTROL_FOCUS_EN
void gt_indev_release_focus_lock(void)
{
    gt_indev_st * indev;
    uint8_t i, cnt_indev;

    if (false == gt_indev_is_enabled()) {
        return;
    }

    cnt_indev = gt_indev_get_dev_count();
    for(i=0; i<cnt_indev; i++){
        indev = gt_indev_get_dev_by_idx(i);
        switch(indev->drv->type){
            case GT_INDEV_TYPE_KEYPAD:
                _SET_KEYPAD_LOCK(indev, false);
                break;
            default:
                break;
        }
    }
}
#endif


void gt_indev_set_enabled(bool enabled)
{
    _indev_params.enabled = enabled;
}

bool gt_indev_is_enabled(void)
{
    return _indev_params.enabled;
}

gt_res_t _gt_indev_reset_status(void)
{
    gt_indev_st * indev = NULL;
    uint8_t i, cnt_indev;
    union data_union * data_p = NULL;

    if (false == gt_indev_is_enabled()) {
        return GT_RES_INV;
    }

    cnt_indev = gt_indev_get_dev_count();
    for (i = 0; i < cnt_indev; i++) {
        indev = gt_indev_get_dev_by_idx(i);
        union data_union * data_p = &indev->data;
        if (indev->scroll_throw_anim) {
            gt_anim_del_by(indev->scroll_throw_anim);
        }
        if (GT_INDEV_TYPE_KEYPAD == indev->drv->type) {
            indev->data.keypad.obj_target = NULL;
            indev->data.keypad.obj_origin = NULL;
        }
    }
    return GT_RES_OK;
}

gt_res_t _gt_indev_check_reset_widgets_illegal_status(gt_obj_st * screen)
{
    gt_indev_st * indev = NULL;
    uint8_t i, cnt_indev;
    union data_union * data_p = NULL;

    if (false == gt_indev_is_enabled()) {
        return GT_RES_INV;
    }
    if (gt_obj_is_type(screen, GT_TYPE_UNKNOWN)) {
        return GT_RES_INV;
    }

    cnt_indev = gt_indev_get_dev_count();
    for (i = 0; i < cnt_indev; i++) {
        indev = gt_indev_get_dev_by_idx(i);
        if (NULL == indev) {
            continue;
        }
        if (GT_INDEV_TYPE_POINTER != indev->drv->type) {
            continue;
        }
#if GT_INDEV_CFG_VELOCITY
        _gt_indev_check_childs_rebound(indev, screen);
#endif
    }
    return GT_RES_OK;
}

bool _gt_indev_remove_want_delate_target(gt_obj_st * target)
{
    gt_indev_st * indev = NULL;
    struct _point * point_p = NULL;
    struct _keypad * keypad_p = NULL;
    uint8_t i, cnt_indev = gt_indev_get_dev_count();
    bool ret = false;

    for (i = 0; i < cnt_indev; i++) {
        indev = gt_indev_get_dev_by_idx(i);
        switch (indev->drv->type) {
            case GT_INDEV_TYPE_POINTER: {
                point_p = &indev->data.point;
                if (target == point_p->obj_target) {
                    point_p->obj_target = NULL;
                    ret = true;
                }
                if (target == point_p->obj_scroll) {
                    point_p->obj_scroll = NULL;
                    ret = true;
                }
                if (target == point_p->obj_origin) {
                    point_p->obj_origin = NULL;
                    ret = true;
                }
                if (target == point_p->obj_act) {
                    point_p->obj_act = NULL;
                    ret = true;
                }
                break;
            }
            case GT_INDEV_TYPE_BUTTON: {
                break;
            }
            case GT_INDEV_TYPE_KEYPAD: {
                keypad_p = &indev->data.keypad;
                if (target == keypad_p->obj_origin) {
                    keypad_p->obj_origin = NULL;
                    ret = true;
                }
                if (target == keypad_p->obj_target) {
                    keypad_p->obj_target = NULL;
                    ret = true;
                }
                break;
            }
            default:
                break;
        }
    }
    return ret;
}

#if GT_INDEV_SIMULATE_POINTER
void gt_indev_simulate_handler(gt_indev_state_et state, gt_size_t x, gt_size_t y)
{
    gt_indev_st * indev = gt_hal_indev_get_simulate_drv();
    if (false == gt_indev_is_enabled()) {
        return;
    }
    struct _point * point_p = &indev->data.point;

    /* call user hard interface */
    if (GT_INDEV_STATE_INVALID == state) {
        return;
    }

    if (x > gt_disp_get_res_hor(NULL) ||
        y > gt_disp_get_res_ver(NULL)) {
        // GT_LOGW(GT_LOG_TAG_TP, "point out of range: x:%d, y:%d", x, y);
        return;
    }

    /* process point data */
    indev->state = state;

    _indev_params.param.point.x = x;
    _indev_params.param.point.y = y;
    _gt_indev_point_set_value(&point_p->newly, x, y);

    /* have one touch event */
    if (GT_INDEV_STATE_RELEASED == indev->state) {
        _indev_released_handle(indev);
    } else {
        _indev_pressed_handle(indev);
    }

    _gt_indev_point_set_value(&point_p->last, point_p->newly.x, point_p->newly.y);
}
#endif

/* end ------------------------------------------------------------------*/
