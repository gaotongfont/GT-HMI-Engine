/**
 * @file gt_style.h
 * @author yonogg
 * @brief
 * @version 0.1
 * @date 2022-05-12 18:37:46
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_STYLE_H_
#define _GT_STYLE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../others/gt_color.h"
#include "../widgets/gt_obj.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef enum _gt_align_e {
    GT_ALIGN_NONE = 0,
    GT_ALIGN_LEFT,              // left top
    GT_ALIGN_LEFT_MID,
    GT_ALIGN_LEFT_BOTTOM,
    GT_ALIGN_RIGHT,             // right top
    GT_ALIGN_RIGHT_MID,
    GT_ALIGN_RIGHT_BOTTOM,
    GT_ALIGN_CENTER,            // center top
    GT_ALIGN_CENTER_MID,
    GT_ALIGN_CENTER_BOTTOM,

    GT_ALIGN_REVERSE = 0x80,    //end of enum
}gt_align_et;

typedef enum _gt_text_long_mode_e {
    GT_TEXT_LONG_MODE_WRAP,
    GT_TEXT_LONG_MODE_DOT,
    GT_TEXT_LONG_MODE_SCROLL,
}gt_text_long_mode_et;

typedef enum {
    GT_BAR_DIR_HOR_L2R,
    GT_BAR_DIR_HOR_R2L,
    GT_BAR_DIR_VER_U2D,
    GT_BAR_DIR_VER_D2U,
}gt_bar_dir_et;

/**
 * @brief @see gt_obj_st
 */
typedef enum {
    GT_OBJ_PROP_TYPE_VISIBLE = 0,
    GT_OBJ_PROP_TYPE_DELATE,
    GT_OBJ_PROP_TYPE_FOCUS,
    GT_OBJ_PROP_TYPE_FOCUS_DISABLED,
    GT_OBJ_PROP_TYPE_DISABLED,
    GT_OBJ_PROP_TYPE_FIXED,
    GT_OBJ_PROP_TYPE_OVERFLOW,
    GT_OBJ_PROP_TYPE_INSIDE,
    GT_OBJ_PROP_TYPE_OPA,
    GT_OBJ_PROP_TYPE_UNTOUCHABILITY,
}gt_obj_prop_type_em;

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_res_t gt_obj_set_area(gt_obj_st * obj, gt_area_st area);
void gt_obj_set_pos(gt_obj_st * obj, gt_size_t x, gt_size_t y);
void _gt_obj_move_child_by(gt_obj_st * obj, gt_size_t dx, gt_size_t dy);
void gt_obj_set_size(gt_obj_st * obj, uint16_t w, uint16_t h);
void gt_obj_set_x(gt_obj_st * obj, gt_size_t x);
void gt_obj_set_y(gt_obj_st * obj, gt_size_t y);
void gt_obj_set_w(gt_obj_st * obj, uint16_t w);
void gt_obj_set_h(gt_obj_st * obj, uint16_t h);

void gt_obj_set_opa(gt_obj_st * obj, gt_opa_t opa);

gt_size_t gt_obj_get_x(gt_obj_st * obj);
gt_size_t gt_obj_get_y(gt_obj_st * obj);
uint16_t gt_obj_get_w(gt_obj_st * obj);
uint16_t gt_obj_get_h(gt_obj_st * obj);

void gt_obj_set_visible(gt_obj_st * obj, gt_visible_et is_visible);
gt_visible_et gt_obj_get_visible(gt_obj_st * obj);

void gt_obj_set_disabled(gt_obj_st * obj, gt_disabled_et is_disabled);
bool gt_obj_is_disabled(gt_obj_st * obj);

void gt_obj_size_change(gt_obj_st * obj, gt_area_st * area_new);
void gt_obj_pos_change(gt_obj_st * obj, gt_area_st * area_new);

void gt_obj_child_set_prop(gt_obj_st * obj, gt_obj_prop_type_em type, uint8_t val);

void gt_obj_set_state(gt_obj_st * obj, gt_state_et state);
gt_state_et gt_obj_get_state(gt_obj_st * obj);


void gt_obj_set_focus(gt_obj_st * obj, bool is_focus);
bool gt_obj_is_focus(gt_obj_st * obj);
void gt_obj_set_focus_disabled(gt_obj_st * obj, gt_disabled_et is_disabled);
bool gt_obj_is_focus_disabled(gt_obj_st * obj);

/**
 * @brief Set the obj's fixed state, The scrolling property depends on the parent class
 *
 * @param obj
 * @param is_fixed true: fixed, false: unfixed
 */
void gt_obj_set_fixed(gt_obj_st * obj, bool is_fixed);
bool gt_obj_get_fixed(gt_obj_st * obj);

/**
 * @brief Control size beyond the scope of the screen, independent way to slide display
 *
 * @param obj
 * @param is_overflow true: overflow, false: not overflow
 */
void gt_obj_set_overflow(gt_obj_st * obj, bool is_overflow);
bool gt_obj_get_overflow(gt_obj_st * obj);

/**
 * @brief The object only displays within the parent area
 *
 * @param obj
 * @param is_inside true: enabled, false: disabled
 */
void gt_obj_set_inside(gt_obj_st * obj, bool is_inside);
bool gt_obj_get_inside(gt_obj_st * obj);

/**
 * @brief Draw the vertical septal line
 *
 * @param obj
 * @param enabled true: enabled, false: disabled
 */
void gt_obj_set_septal_line(gt_obj_st * obj, bool enabled);
bool gt_obj_get_septal_line(gt_obj_st * obj);

void gt_obj_set_mask_effect(gt_obj_st * obj, bool is_keep_alive);
bool gt_obj_get_mask_effect(gt_obj_st * obj);

/**
 * @brief touch event will be called to parent object
 *
 * @param obj
 * @param is_touch_parent true: enabled, false: disabled
 */
void gt_obj_set_touch_parent(gt_obj_st * obj, bool is_touch_parent);
bool gt_obj_get_touch_parent(gt_obj_st * obj);

/**
 * @brief state toggle trigger mode
 *
 * @param obj
 * @param is_trigger_mode 0:[default] hold-on; 1: switch
 */
void gt_obj_set_trigger_mode(gt_obj_st * obj, gt_obj_trigger_mode_et is_trigger_mode);

/**
 * @brief
 *
 * @param obj
 * @return gt_obj_trigger_mode_et 0:[default] hold-on; 1: switch
 */
gt_obj_trigger_mode_et gt_obj_get_trigger_mode(gt_obj_st * obj);

/**
 * @brief Controls are virtual logic, not entities, and cannot receive touch responses
 *
 * @param obj
 * @param is_virtual 0: un-virtual[default], 1: virtual
 */
void gt_obj_set_virtual(gt_obj_st * obj, bool is_virtual);
bool gt_obj_get_virtual(gt_obj_st * obj);

void gt_obj_set_scroll_dir(gt_obj_st * obj, gt_scroll_dir_et dir);
gt_scroll_dir_et gt_obj_get_scroll_dir(gt_obj_st * obj);
bool gt_obj_is_scroll_dir(gt_obj_st * obj, gt_scroll_dir_et dir);

/**
 * @brief Set object scroll horizontal left or right direction.
 *
 * @param obj
 * @param is_scroll_right 0: left; 1: right
 */
void gt_obj_set_scroll_lr(gt_obj_st * obj, bool is_scroll_right);
bool gt_obj_is_scroll_left(gt_obj_st * obj);
bool gt_obj_is_scroll_right(gt_obj_st * obj);

/**
 * @brief Set object scroll vertical up or down direction.
 *
 * @param obj
 * @param is_scroll_down 0: up; 1: down
 */
void gt_obj_set_scroll_ud(gt_obj_st * obj, bool is_scroll_down);
bool gt_obj_is_scroll_up(gt_obj_st * obj);
bool gt_obj_is_scroll_down(gt_obj_st * obj);

void gt_obj_set_scroll_snap_x(gt_obj_st * obj, gt_scroll_snap_em snap);
void gt_obj_set_scroll_snap_y(gt_obj_st * obj, gt_scroll_snap_em snap);
gt_scroll_snap_em gt_obj_get_scroll_snap_x(gt_obj_st * obj);
gt_scroll_snap_em gt_obj_get_scroll_snap_y(gt_obj_st * obj);

/**
 * @brief Filter the maximum viewable screen range based on the properties of the control
 *
 * @param obj current object
 * @return true Ignore to calculate the maximum viewable screen range
 * @return false Need to calculate the maximum viewable screen range
 */
bool _gt_obj_is_ignore_calc_max_area(gt_obj_st * obj);

/**
 * @brief Set control's event needs to be notified to the parent control
 *
 * @param obj
 * @param is_bubble_notify true: enabled, false: disabled
 */
void gt_obj_set_bubble_notify(gt_obj_st * obj, bool is_bubble_notify);

/**
 * @brief Control's event needs to be notified to the parent control or not.
 *
 * @param obj
 * @return true
 * @return false
 */
bool gt_obj_is_bubble_notify(gt_obj_st * obj);

/**
 * @brief Set the object is untouchable
 *
 * @param obj
 * @param is_untouchability true: object can not be touch, false[default]: object can be touch
 */
void gt_obj_set_untouchability(gt_obj_st * obj, bool is_untouchability);

/**
 * @brief The object is untouchable or not
 *
 * @param obj
 * @return true The object is untouchable
 * @return false The object can be touch
 */
bool gt_obj_is_untouchability(gt_obj_st * obj);

/**
 * @brief Reduce the object background area
 *
 * @param obj
 * @param reduce REDUCE_DEFAULT [default]: 2
 */
void gt_obj_set_reduce(gt_obj_st * obj, uint8_t reduce);

uint8_t gt_obj_get_reduce(gt_obj_st * obj);

void gt_obj_set_radius(gt_obj_st * obj, gt_radius_t radius);

gt_radius_t gt_obj_get_radius(gt_obj_st * obj);

/**
 * @brief Set the object is only row layout
 *
 * @param obj
 * @param is_row_layout true: enabled row layout, false[default]: disabled row layout
 */
void gt_obj_set_row_layout(gt_obj_st * obj, bool is_row_layout);

/**
 * @brief The object invert grow or not in row layout mode
 *
 * @param obj
 * @param is_grow_invert true: grow invert, false[default]: grow
 */
void gt_obj_set_grow_invert(gt_obj_st * obj, bool is_grow_invert);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_STYLE_H_
