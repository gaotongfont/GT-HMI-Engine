/**
 * @file gt_status_bar.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-03-29 10:52:06
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_status_bar.h"

#if GT_USE_LAYER_TOP && GT_CFG_ENABLE_STATUS_BAR
#include "stddef.h"
#include "string.h"
#include "../others/gt_types.h"
#include "../others/gt_log.h"
#include "../hal/gt_hal_disp.h"
#include "../core/gt_disp.h"
#include "../core/gt_draw.h"
#include "../core/gt_graph_base.h"
#include "../widgets/gt_obj.h"
#include "../widgets/gt_img.h"
#include "../widgets/gt_label.h"
#include "../core/gt_layout.h"


/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_STATUS_BAR
#define MY_CLASS    &gt_status_bar_class

/** Status bar default height */
#define _DEFAULT_HEIGHT     26

#define _DEFAULT_LEFT_SCALE     30
#define _DEFAULT_CENTER_SCALE   40
#define _DEFAULT_RIGHT_SCALE    30

/* private typedef ------------------------------------------------------*/
typedef struct _part_item_s {
    gt_obj_st * obj;
    uint8_t scale;
}_part_item_st;

typedef struct _gt_status_bar_s {
    gt_obj_st obj;

#if GT_STATUS_BAR_THREE_PART_SCALE_MODE
    _part_item_st left;
    _part_item_st center;
    _part_item_st right;
#endif

#if GT_STATUS_BAR_CUSTOM_MODE
    gt_obj_st * custom;
#endif

    gt_font_info_st font_info;
    gt_color_t font_color;
    uint16_t height;
    uint8_t bg_opa;

    uint8_t show_bg : 1;
    uint8_t reserved : 7;
}_gt_status_bar_st;

typedef void (* _font_param_cb_t)(gt_obj_st *, uint8_t);

typedef void (* _font_color_cb_t)(gt_obj_st *, gt_color_t);

/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_status_bar_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = NULL,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_status_bar_st)
};


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _init_cb(gt_obj_st * obj) {
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;

    if (style->show_bg) {
        gt_attr_rect_st rect_attr;
        gt_graph_init_rect_attr(&rect_attr);
        rect_attr.reg.is_fill    = true;
        rect_attr.bg_opa         = style->bg_opa;
        rect_attr.bg_color       = obj->bgcolor;

        draw_bg(obj->draw_ctx, &rect_attr, &obj->area);
    }
}

static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et type = gt_event_get_code(e);
    if (GT_EVENT_TYPE_DRAW_START == type) {
        gt_disp_invalid_area(obj);
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
    }
}

static gt_obj_st * _find_obj_recursive_by_type(gt_obj_st * self, gt_obj_type_et type) {
    GT_CHECK_BACK_VAL(self, NULL);
    if (gt_obj_is_type(self, type)) {
        return self;
    }
    gt_obj_st * ret = NULL;

    for (gt_size_t i = 0, cnt = self->cnt_child; i < cnt; ++i) {
        ret = _find_obj_recursive_by_type(self->child[i], type);
        if (ret) {
            break;
        }
    }
    return ret;
}

static gt_obj_st * _find_by_str(gt_obj_st * parent, char * text_or_src) {
    if (NULL == parent) {
        return NULL;
    }
    gt_obj_st * ret = NULL;
    gt_obj_st * child = NULL;
    for (gt_size_t i = 0, cnt = parent->cnt_child; i < cnt; ++i) {
        child = parent->child[i];
        if (gt_obj_is_type(child, GT_TYPE_IMG)) {
            if (0 == strcmp(text_or_src, gt_img_get_src(child))) {
                ret = child;
                break;
            }
        } else if (gt_obj_is_type(child, GT_TYPE_LAB)) {
            if (0 == strcmp(text_or_src, gt_label_get_text(child))) {
                ret = child;
                break;
            }
        }
    }
    return ret;
}

#if GT_STATUS_BAR_THREE_PART_SCALE_MODE

static void _adjust_img_center_line(gt_obj_st * icon, gt_obj_st * obj) {
    uint16_t w = gt_img_get_width(icon);
    uint16_t h = gt_img_get_height(icon);
    gt_obj_set_size(icon, w, h);
    gt_obj_set_pos(icon,
        gt_obj_get_x(icon) - (obj->grow_invert ? w : 0),
        ((gt_obj_get_h(obj) - h) >> 1) + gt_obj_get_y(obj));
}

static void _hor_center_widgets(gt_obj_st * obj) {
    gt_obj_st * child = NULL;
    for (gt_size_t i = 0, cnt = obj->cnt_child; i < cnt; ++i) {
        child = obj->child[i];
        if (gt_obj_is_type(child, GT_TYPE_IMG)) {
            _adjust_img_center_line(child, obj);
        }
    }
}

static void _resize_part_width(gt_obj_st * obj) {
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;
    gt_area_st area = obj->area;
    uint16_t total = style->left.scale + style->center.scale + style->right.scale;

    if (style->left.obj) {
        gt_obj_set_pos(style->left.obj, area.x, area.y);
        gt_obj_set_size(style->left.obj, area.w * style->left.scale / total, area.h);
        gt_layout_row_grow(style->left.obj);
        // center
        _hor_center_widgets(style->left.obj);
    }
    if (style->center.obj) {
        gt_obj_set_pos(style->center.obj, gt_obj_get_x(style->left.obj) + gt_obj_get_w(style->left.obj), area.y);
        gt_obj_set_size(style->center.obj, area.w * style->center.scale / total, area.h);
    }
    if (style->right.obj) {
        gt_obj_set_pos(style->right.obj, gt_obj_get_x(style->center.obj) + gt_obj_get_w(style->center.obj), area.y);
        gt_obj_set_size(style->right.obj, area.w * style->right.scale / total, area.h);
        gt_layout_row_grow(style->right.obj);
        // center
        _hor_center_widgets(style->right.obj);
    }
}

static gt_obj_st * _create_img_obj(gt_obj_st * parent, char * src) {

    gt_obj_st * icon = _find_by_str(parent, src);
    if (icon) {
        return icon;
    }
    icon = gt_img_create(parent);
    gt_img_set_src(icon, src);
    gt_obj_set_inside(icon, true);
    _adjust_img_center_line(icon, parent);

    return icon;
}

static bool _change_img_src(gt_obj_st * parent, gt_obj_st * icon, char * src) {
    if (NULL == parent) {
        return false;
    }
    if (false == gt_obj_is_child(icon, parent)) {
        return false;
    }
    if (false == gt_obj_is_type(icon, GT_TYPE_IMG)) {
        return false;
    }
    gt_img_set_src(icon, src);
    gt_layout_row_grow(parent);
    _adjust_img_center_line(icon, parent);

    return true;
}

static bool _remove_by(gt_obj_st * parent, gt_obj_st * target) {
    if (NULL == parent) {
        return false;
    }
    if (false == gt_obj_is_child(target, parent)) {
        return false;
    }
    _gt_obj_class_destroy(target);
    gt_layout_row_grow(parent);

    return true;
}

#endif  /** GT_STATUS_BAR_THREE_PART_SCALE_MODE */

static gt_obj_st * _create_lab_obj(gt_obj_st * parent, _gt_status_bar_st * style, char * text, uint16_t width) {
    if (NULL == parent) {
        return NULL;
    }
    gt_obj_st * lab = _find_by_str(parent, text);
    if (lab) {
        return lab;
    }
    lab = gt_label_create(parent);
    gt_label_set_font_family_cn(lab, style->font_info.style_cn);
    gt_label_set_font_family_en(lab, style->font_info.style_en);
    gt_label_set_font_family_fl(lab, style->font_info.style_fl);
    gt_label_set_font_family_numb(lab, style->font_info.style_numb);
    gt_label_set_font_size(lab, style->font_info.size);
    gt_label_set_font_gray(lab, style->font_info.gray);
    gt_label_set_font_thick_cn(lab, style->font_info.thick_cn);
    gt_label_set_font_thick_en(lab, style->font_info.thick_en);
    gt_label_set_font_color(lab, style->font_color);
    gt_label_set_text(lab, text);
    gt_label_set_font_align(lab, GT_ALIGN_CENTER_MID);
    gt_obj_set_size(lab, width, gt_obj_get_h(parent));

    gt_obj_set_inside(lab, true);

    return lab;
}

static void _set_font_param_call(gt_obj_st * parent, _font_param_cb_t cb, uint8_t value) {
    gt_obj_st * part = NULL, * child = NULL;
    gt_size_t i = 0, cnt = 0, k = 0, cnt_k = 0;
    if (NULL == cb) {
        return ;
    }

    for (i = 0, cnt = parent->cnt_child; i < cnt; ++i) {
        part = parent->child[i];
        for (k = 0, cnt_k = part->cnt_child; k < cnt_k; ++k) {
            child = part->child[k];
            if (false == gt_obj_is_type(child, GT_TYPE_LAB)) {
                continue;
            }
            cb(child, value);
        }
    }
}

static void _set_font_color_call(gt_obj_st * parent, _font_color_cb_t cb, gt_color_t color) {
    gt_obj_st * part = NULL, * child = NULL;
    gt_size_t i = 0, cnt = 0, k = 0, cnt_k = 0;
    if (NULL == cb) {
        return ;
    }

    for (i = 0, cnt = parent->cnt_child; i < cnt; ++i) {
        part = parent->child[i];
        for (k = 0, cnt_k = part->cnt_child; k < cnt_k; ++k) {
            child = part->child[k];
            if (false == gt_obj_is_type(child, GT_TYPE_LAB)) {
                continue;
            }
            cb(child, color);
        }
    }
}

static bool _change_lab_text(gt_obj_st * parent, gt_obj_st * lab, char * text) {
    if (NULL == parent) {
        return false;
    }
    if (false == gt_obj_is_child(lab, parent)) {
        return false;
    }
    if (false == gt_obj_is_type(lab, GT_TYPE_LAB)) {
        return false;
    }
    gt_label_set_text(lab, text);

    return true;
}

static bool _remove_all_by(gt_obj_st * parent) {
    if (0 == parent->cnt_child) {
        return true;
    }

    _gt_obj_class_destroy_children(parent);
    gt_event_send(parent->parent, GT_EVENT_TYPE_DRAW_START, NULL);
    return true;
}

/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_status_bar_create(bool have_background)
{
    gt_disp_st * disp = gt_disp_get_default();
    if (NULL == disp || NULL == disp->layer_top) {
        return NULL;
    }
    gt_obj_st * obj = gt_status_bar_get_obj();
    if (NULL == obj) {
        obj = gt_obj_class_create(MY_CLASS, disp->layer_top);
    }
    GT_CHECK_BACK_VAL(obj, NULL);
    obj->area = disp->layer_top->area;
    obj->area.h = _DEFAULT_HEIGHT;

    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;
    gt_font_info_init(&style->font_info);
    style->font_color = gt_color_black();
    style->bg_opa = GT_OPA_100;
    style->show_bg = have_background;
#if GT_STATUS_BAR_THREE_PART_SCALE_MODE
    style->left.scale = _DEFAULT_LEFT_SCALE;
    style->center.scale = _DEFAULT_CENTER_SCALE;
    style->right.scale = _DEFAULT_RIGHT_SCALE;

    if (NULL == style->left.obj) {
        style->left.obj = gt_obj_create(obj);
        gt_obj_set_row_layout(style->left.obj, true);
    }
    if (NULL == style->right.obj) {
        style->right.obj = gt_obj_create(obj);
        gt_obj_set_row_layout(style->right.obj, true);
        gt_obj_set_grow_invert(style->right.obj, true);
    }
    if (NULL == style->center.obj) {
        style->center.obj = gt_obj_create(obj);
    }

    _resize_part_width(obj);
#endif

#if GT_STATUS_BAR_CUSTOM_MODE
    if (NULL == style->custom) {
        style->custom = gt_obj_create(obj);
        gt_obj_set_area(style->custom, obj->area);
    }
#endif

    return obj;
}

gt_obj_st * gt_status_bar_get_obj(void)
{
    gt_disp_st * disp = gt_disp_get_default();
    if (NULL == disp || NULL == disp->layer_top) {
        return NULL;
    }
    return _find_obj_recursive_by_type(disp->layer_top, OBJ_TYPE);
}

bool gt_status_bar_set_hide(bool hide)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    GT_CHECK_BACK_VAL(obj, false);

    gt_obj_set_visible(obj, hide ? GT_INVISIBLE : GT_VISIBLE);
    return true;
}

bool gt_status_bar_is_hide(void)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    GT_CHECK_BACK_VAL(obj, false);
    return gt_obj_get_visible(obj);
}

void gt_status_bar_set_height(uint16_t height)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    GT_CHECK_BACK(obj);
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;
    gt_obj_set_size(obj, gt_obj_get_w(obj), height);
#if GT_STATUS_BAR_THREE_PART_SCALE_MODE
    _resize_part_width(obj);
#endif

#if GT_STATUS_BAR_CUSTOM_MODE
    if (style->custom) {
        gt_obj_set_h(style->custom, height);
    }
#endif
}

#if GT_STATUS_BAR_THREE_PART_SCALE_MODE

void gt_status_bar_set_scale(uint8_t left, uint8_t center, uint8_t right)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    GT_CHECK_BACK(obj);
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;

    style->left.scale = left;
    style->center.scale = center;
    style->right.scale = right;

    _resize_part_width(obj);
}

gt_obj_st * gt_status_bar_left_find_by(char * text_or_src)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    if (NULL == obj || NULL == text_or_src) {
        return NULL;
    }
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;

    return _find_by_str(style->left.obj, text_or_src);
}

bool gt_status_bar_left_remove(gt_obj_st * target)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    GT_CHECK_BACK_VAL(obj, false);
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;

    return _remove_by(style->left.obj, target);
}

bool gt_status_bar_left_remove_all(void)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    GT_CHECK_BACK_VAL(obj, false);
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;

    return _remove_all_by(style->left.obj);
}

gt_obj_st * gt_status_bar_left_add_icon(char * src)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    if (NULL == obj || NULL == src) {
        return NULL;
    }
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;
    if (NULL == style->left.obj) {
        return NULL;
    }

    return _create_img_obj(style->left.obj, src);
}

bool gt_status_bar_left_change_icon(gt_obj_st * icon, char * src)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    if (NULL == obj || NULL == src) {
        return false;
    }
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;

    return _change_img_src(style->left.obj, icon, src);
}

gt_obj_st * gt_status_bar_left_add_text(char * text, uint16_t width)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    if (NULL == obj || NULL == text) {
        return NULL;
    }
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;

    return _create_lab_obj(style->left.obj, style, text, width);
}

bool gt_status_bar_left_change_text(gt_obj_st * lab, char * text)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    if (NULL == obj || NULL == text) {
        return false;
    }
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;

    return _change_lab_text(style->left.obj, lab, text);
}

gt_obj_st * gt_status_bar_right_find_by(char * text_or_src)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    if (NULL == obj || NULL == text_or_src) {
        return NULL;
    }
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;

    return _find_by_str(style->right.obj, text_or_src);
}

bool gt_status_bar_right_remove(gt_obj_st * target)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    GT_CHECK_BACK_VAL(obj, false);
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;

    return _remove_by(style->right.obj, target);
}

bool gt_status_bar_right_remove_all(void)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    GT_CHECK_BACK_VAL(obj, false);
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;

    return _remove_all_by(style->right.obj);
}

gt_obj_st * gt_status_bar_right_add_icon(char * src)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    if (NULL == obj || NULL == src) {
        return NULL;
    }
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;
    if (NULL == style->right.obj) {
        return NULL;
    }

    return _create_img_obj(style->right.obj, src);
}

bool gt_status_bar_right_change_icon(gt_obj_st * icon, char * src)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    if (NULL == obj || NULL == src) {
        return NULL;
    }
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;

    return _change_img_src(style->right.obj, icon, src);
}

gt_obj_st * gt_status_bar_right_add_text(char * text, uint16_t width)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    if (NULL == obj || NULL == text) {
        return NULL;
    }
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;

    gt_obj_st * lab = _create_lab_obj(style->right.obj, style, text, width);

    gt_layout_row_grow(style->right.obj);

    return lab;
}

bool gt_status_bar_right_change_text(gt_obj_st * lab, char * text)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    if (NULL == obj || NULL == text) {
        return NULL;
    }
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;

    return _change_lab_text(style->right.obj, lab, text);
}

bool gt_status_bar_center_remove_all(void)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    GT_CHECK_BACK_VAL(obj, NULL);
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;

    return _remove_all_by(style->center.obj);
}

gt_obj_st * gt_status_bar_center_set_text(char * text)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    if (NULL == obj || NULL == text) {
        return NULL;
    }
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;

    gt_obj_st * lab =  _create_lab_obj(style->center.obj, style, text, gt_obj_get_w(style->center.obj));
    if (strcmp(text, gt_label_get_text(lab))) {
        gt_label_set_text(lab, text);
    }
    return lab;
}

#endif  /** GT_STATUS_BAR_THREE_PART_SCALE_MODE */

#if GT_STATUS_BAR_CUSTOM_MODE

gt_obj_st * gt_status_bar_custom_find_by(char * text_or_src)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    if (NULL == obj || NULL == text_or_src) {
        return NULL;
    }
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;

    return _find_by_str(style->custom, text_or_src);
}

bool gt_status_bar_custom_remove(gt_obj_st * target)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    GT_CHECK_BACK_VAL(obj, false);
    if (NULL == target) {
        return false;
    }
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;

    if (false == gt_obj_is_child(target, style->custom)) {
        return false;
    }
    _gt_obj_class_destroy(target);
    return true;
}

bool gt_status_bar_custom_remove_all(void)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    GT_CHECK_BACK_VAL(obj, NULL);
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;

    return _remove_all_by(style->custom);
}

gt_obj_st * gt_status_bar_custom_add_icon(char * src)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    if (NULL == obj || NULL == src) {
        return NULL;
    }
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;
    GT_CHECK_BACK_VAL(style->custom, NULL);

    gt_obj_st * icon = _find_by_str(style->custom, src);
    if (icon) {
        return icon;
    }
    icon = gt_img_create(style->custom);
    gt_img_set_src(icon, src);
    gt_obj_set_inside(icon, true);

    return icon;
}

bool gt_status_bar_custom_change_icon(gt_obj_st * icon, char * src)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    if (NULL == obj || NULL == src) {
        return false;
    }
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;
    GT_CHECK_BACK_VAL(style->custom, false);
    if (false == gt_obj_is_child(icon, style->custom)) {
        return false;
    }
    if (false == gt_obj_is_type(icon, GT_TYPE_IMG)) {
        return false;
    }
    gt_img_set_src(icon, src);
    return true;
}

gt_obj_st * gt_status_bar_custom_add_text(char * text, uint16_t width)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    if (NULL == obj || NULL == text) {
        return NULL;
    }
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;

    return _create_lab_obj(style->custom, style, text, width);
}

bool gt_status_bar_custom_change_text(gt_obj_st * lab, char * text)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    if (NULL == obj || NULL == text) {
        return false;
    }
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;

    return _change_lab_text(style->custom, lab, text);
}

#endif  /** GT_STATUS_BAR_CUSTOM_MODE */

void gt_status_bar_show_bg(bool show)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    GT_CHECK_BACK(obj);
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;
    style->show_bg = show;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_status_bar_set_bg_opa(gt_opa_t opa)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    GT_CHECK_BACK(obj);
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;
    style->bg_opa = opa;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_status_bar_set_bg_color(gt_color_t color)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    GT_CHECK_BACK(obj);
    obj->bgcolor = color;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_status_bar_set_font_family_cn(gt_family_t family)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    GT_CHECK_BACK(obj);
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;
    style->font_info.style_cn = family;

    _set_font_param_call(obj, (_font_param_cb_t)gt_label_set_font_family_cn, family);

    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_status_bar_set_font_family_en(gt_family_t family)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    GT_CHECK_BACK(obj);
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;
    style->font_info.style_en = family;
    _set_font_param_call(obj, (_font_param_cb_t)gt_label_set_font_family_en, family);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_status_bar_set_font_family_fl(gt_family_t family)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    GT_CHECK_BACK(obj);
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;
    style->font_info.style_fl = family;
    _set_font_param_call(obj, (_font_param_cb_t)gt_label_set_font_family_fl, family);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_status_bar_set_font_family_numb(gt_family_t family)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    GT_CHECK_BACK(obj);
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;
    style->font_info.style_numb = family;
    _set_font_param_call(obj, (_font_param_cb_t)gt_label_set_font_family_numb, family);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_status_bar_set_font_size(uint8_t size)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    GT_CHECK_BACK(obj);
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;
    style->font_info.size = size;
    _set_font_param_call(obj, (_font_param_cb_t)gt_label_set_font_size, size);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_status_bar_set_font_gray(uint8_t gray)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    GT_CHECK_BACK(obj);
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;
    style->font_info.gray = gray;
    _set_font_param_call(obj, (_font_param_cb_t)gt_label_set_font_gray, gray);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_status_bar_set_font_color(gt_color_t color)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    GT_CHECK_BACK(obj);
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;
    style->font_color = color;
    _set_font_color_call(obj, gt_label_set_font_color, color);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_status_bar_set_font_thick_cn(uint8_t thick)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    GT_CHECK_BACK(obj);
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;
    style->font_info.thick_cn = thick;
    _set_font_param_call(obj, (_font_param_cb_t)gt_label_set_font_thick_cn, thick);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_status_bar_set_font_thick_en(uint8_t thick)
{
    gt_obj_st * obj = gt_status_bar_get_obj();
    GT_CHECK_BACK(obj);
    _gt_status_bar_st * style = (_gt_status_bar_st * )obj;
    style->font_info.thick_en = thick;
    _set_font_param_call(obj, (_font_param_cb_t)gt_label_set_font_thick_en, thick);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

#endif  /** GT_USE_LAYER_TOP && GT_CFG_ENABLE_STATUS_BAR */

/* end ------------------------------------------------------------------*/
