/**
 * @file gt_listview.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-18 13:39:15
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_listview.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../core/gt_obj_scroll.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"

#include "./gt_label.h"
#include "./gt_img.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_LISTVIEW
#define MY_CLASS    &gt_listview_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_listview_s {
    gt_obj_st obj;
    gt_obj_st * last_active;
    uint16_t item_height;       /** The height of item */
    gt_font_info_st font_info;
    gt_color_t font_color;
    gt_align_et align;

    gt_color_t border_color;
    uint8_t border_width;
    uint8_t space_x;
    uint8_t space_y;

    uint8_t highlight      : 1; /** holding highlight state the last active item, 0: release restores the effect, 1: switch state */
    uint8_t septal_line    : 1; /** Display septal line between items */
    uint8_t icon_right     : 1;
    uint8_t multiple       : 1; /** Selected multiple item: 0: single selected; 1: multiple selected */
    uint8_t reserved       : 4;
}_gt_listview_st;


typedef enum {
    _LABEL_FONT_SIZE = 0,
    _LABEL_FONT_GRAY,
    _LABEL_FONT_ALIGN,
    _LABEL_FONT_COLOR,
    _LABEL_FONT_CN,
    _LABEL_FONT_EN,
    _LABEL_FONT_FL,
    _LABEL_FONT_NUMB,
    _LABEL_FONT_THICK_CN,
    _LABEL_FONT_THICK_EN,
    _LABEL_FONT_SPACE,
}_label_prop_em;

/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_listview_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_listview_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/


/**
 * @brief obj init listview widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    _gt_listview_st * style = (_gt_listview_st * )obj;

    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill    = true;
    rect_attr.border_width   = style->border_width;
    rect_attr.radius         = style->border_width << 1;
    rect_attr.bg_opa         = obj->opa;
    rect_attr.border_color  = style->border_color;
    rect_attr.bg_color      = obj->bgcolor;
    // 外框
    draw_bg(obj->draw_ctx, &rect_attr, &obj->area);
    // gt_area_st area_item = gt_area_reduce(obj->area, REDUCE_DEFAULT);
    // draw_bg(obj->draw_ctx, &rect_attr, &area_item);
}


/**
 * @brief obj deinit call back
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj) {
}

static void _scrolling_handler(gt_obj_st * obj) {
    _gt_listview_st * style = (_gt_listview_st * )obj;
    gt_size_t bottom = gt_obj_get_limit_bottom(obj);

    if (gt_obj_scroll_get_y(obj) > 0) {
        obj->process_attr.scroll.y = 0;
    }
    else if (gt_obj_scroll_get_y(obj) < bottom) {
        obj->process_attr.scroll.y = bottom;
    }

    gt_size_t offset = obj->area.y + gt_obj_scroll_get_y(obj);
    for (gt_size_t i = 0, cnt = obj->cnt_child; i < cnt; i++) {
        gt_obj_set_pos(obj->child[i], gt_obj_get_x(obj->child[i]), offset);
        offset += style->item_height;
    }
}

static void _set_last_active_obj(gt_obj_st * listview, gt_obj_st * target) {
    _gt_listview_st * style = (_gt_listview_st * )listview;
    if (listview == target) {
        return;
    }
    style->last_active = target;
}

static gt_obj_st * _get_last_active_obj(gt_obj_st * listview) {
    _gt_listview_st * style = (_gt_listview_st * )listview;
    return style->last_active;
}

static void _reset_last_active_state(gt_obj_st * listview, gt_obj_st * target) {
    if (listview == target) {
        return;
    }
    if (gt_obj_get_trigger_mode(target)) {
        return;
    }
    gt_obj_set_state(target, false);
    gt_disp_invalid_area(listview);
}

static void _highlight_set_obj_active_state(gt_obj_st * listview) {
    _gt_listview_st * style = (_gt_listview_st * )listview;
    if (false == style->highlight) {
        return;
    }
    if (style->multiple) {
        return;
    }
    if (false == gt_obj_get_trigger_mode(style->last_active)) {
        return;
    }
    if (false == gt_obj_get_state(style->last_active)) {
        gt_obj_set_state(style->last_active,  true);
    }
}

static void _reset_other_active_state(gt_obj_st * listview) {
    _gt_listview_st * style = (_gt_listview_st * )listview;
    gt_obj_st * item = NULL;
    if (false == style->highlight) {
        return;
    }
    if (style->multiple) {
        return;
    }

    for (gt_size_t i = 0, cnt = listview->cnt_child; i < cnt; i++) {
        item = listview->child[i];
        if (style->last_active == item) {
            continue;
        }
        if (false == gt_obj_get_trigger_mode(item)) {
            continue;
        }
        gt_obj_set_state(item, false);
    }
    gt_event_send(listview, GT_EVENT_TYPE_DRAW_START, NULL);
}

/**
 * @brief obj event handler call back
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);

    if (GT_EVENT_TYPE_DRAW_START == code) {
        gt_disp_invalid_area(obj);
    }
    else if (GT_EVENT_TYPE_INPUT_SCROLL == code) {
        if (gt_obj_get_childs_max_height(obj) < gt_obj_get_h(obj)) {
            /** all items can display with current page, reset scroll selected effect */
            _reset_last_active_state(obj, _get_last_active_obj(obj));
            _set_last_active_obj(obj, NULL);
            return;
        }
        _scrolling_handler(obj);
    }
    else if (GT_EVENT_TYPE_INPUT_PRESSED == code) {
        _set_last_active_obj(obj, e->origin);
    }
    else if (GT_EVENT_TYPE_INPUT_RELEASED == code) {
        _set_last_active_obj(obj, e->origin);
        _reset_other_active_state(obj);
        _highlight_set_obj_active_state(obj);
    }
    else if (GT_EVENT_TYPE_INPUT_SCROLL_END == code) {
        _reset_last_active_state(obj, e->origin);
    }
}

static void _set_label_prop_by_type(gt_obj_st * listview, _label_prop_em type, int val) {
    gt_size_t item = 0, cnt_item = 0;
    gt_obj_st * obj = NULL;

    for (gt_size_t i = 0, cnt = listview->cnt_child; i < cnt; i++) {
        obj = listview->child[i];
        for (item = 0, cnt_item = obj->cnt_child; item < cnt_item; item++) {
            if (false == gt_obj_is_type(obj->child[item], GT_TYPE_LAB)) {
                continue;
            }
            if (_LABEL_FONT_SIZE == type) {
                gt_label_set_font_size(obj->child[item], val);
            } else if (_LABEL_FONT_GRAY == type) {
                gt_label_set_font_gray(obj->child[item], val);
            } else if (_LABEL_FONT_ALIGN == type) {
                gt_label_set_font_align(obj->child[item], val);
            } else if (_LABEL_FONT_COLOR == type) {
                gt_label_set_font_color(obj->child[item], gt_color_hex((uint32_t)val));
            } else if (_LABEL_FONT_CN == type) {
                gt_label_set_font_family_cn(obj->child[item], (gt_family_t)val);
            } else if (_LABEL_FONT_EN == type) {
                gt_label_set_font_family_en(obj->child[item], (gt_family_t)val);
            } else if (_LABEL_FONT_FL == type) {
                gt_label_set_font_family_fl(obj->child[item], (gt_family_t)val);
            } else if (_LABEL_FONT_NUMB == type) {
                gt_label_set_font_family_numb(obj->child[item], (gt_family_t)val);
            } else if (_LABEL_FONT_THICK_CN == type) {
                gt_label_set_font_thick_cn(obj->child[item], val);
            } else if (_LABEL_FONT_THICK_EN == type) {
                gt_label_set_font_thick_en(obj->child[item], val);
            } else if (_LABEL_FONT_SPACE == type) {
                gt_label_set_space(obj->child[item], (val >> 8) & 0xff, val & 0xff);
            }
        }
    }
}

static inline uint16_t _get_scale_offset(gt_obj_st * item) {
    return gt_obj_get_w(item) >> 2;
}

static gt_obj_st * _create_item_obj(gt_obj_st * listview) {
    _gt_listview_st * style = (_gt_listview_st * )listview;
    gt_size_t item_offset = listview->area.y;
    if (listview->cnt_child) {
        item_offset = listview->child[listview->cnt_child - 1]->area.y + style->item_height;
    }
    gt_obj_st * obj = gt_obj_create(listview);
    /** set object item size */
    obj->area.x = listview->area.x;
    obj->area.y = item_offset;
    obj->area.w = listview->area.w;
    obj->area.h = style->item_height;

    gt_obj_set_fixed(obj, true);
    gt_obj_set_inside(obj, true);
    gt_obj_set_mask_effect(obj, true);
    gt_obj_set_bubble_notify(obj, true);
    gt_obj_set_septal_line(obj, style->septal_line);
    gt_obj_set_trigger_mode(obj, style->highlight);

    return obj;
}

static void _center_icon_pos(gt_obj_st * icon, gt_obj_st * item, bool icon_right) {
    uint16_t offset = _get_scale_offset(item);
    gt_size_t x = gt_obj_get_x(item);

    gt_area_copy(&icon->area, &item->area);
    icon->area.w = offset;

    if (icon_right) {
        x += gt_obj_get_w(item) - offset;
    }

    icon->area.x = ((offset - gt_img_get_width(icon)) >> 1) + x;
    icon->area.y = ((gt_obj_get_h(item) - gt_img_get_height(icon)) >> 1) + gt_obj_get_y(icon);
}

static void _set_label_pos(gt_obj_st * lab, gt_obj_st * item, bool icon_right) {
    uint16_t offset = _get_scale_offset(item);

    gt_area_copy(&lab->area, &item->area);
    lab->area.w -= offset;

    if (false == icon_right) {
        lab->area.x += offset;
    }
}

/**
 * @brief Create a image object
 *
 * @param listview listview object
 * @param obj item object
 * @param area display area
 * @param img image path
 * @return gt_obj_st* icon object
 */
static gt_obj_st * _create_item_icon(gt_obj_st * listview, gt_obj_st * obj, char * img) {
    _gt_listview_st * style = (_gt_listview_st * )listview;
    gt_obj_st * icon = gt_img_create(obj);

    gt_img_set_src(icon, img);
    _center_icon_pos(icon, obj, style->icon_right);

    gt_obj_set_touch_parent(icon, true);
    gt_obj_set_bubble_notify(icon, true);

    return icon;
}

/**
 * @brief Create a label object
 *
 * @param listview listview object
 * @param obj item object
 * @param area display area
 * @param text label text
 * @return gt_obj_st* label object
 */
static gt_obj_st * _create_item_label(gt_obj_st * listview, gt_obj_st * obj, char * text) {
    _gt_listview_st * style = (_gt_listview_st * )listview;
    gt_obj_st * lab = gt_label_create(obj);

    _set_label_pos(lab, obj, style->icon_right);

    gt_label_set_font_align(lab, style->align);
    gt_label_set_font_color(lab, style->font_color);
    gt_label_set_font_family_cn(lab, style->font_info.style_cn);
    gt_label_set_font_family_en(lab, style->font_info.style_en);
    gt_label_set_font_family_fl(lab, style->font_info.style_fl);
    gt_label_set_font_family_numb(lab, style->font_info.style_numb);
    gt_label_set_font_size(lab, style->font_info.size);
    gt_label_set_font_gray(lab, style->font_info.gray);
    gt_label_set_font_thick_cn(lab, style->font_info.thick_cn);
    gt_label_set_font_thick_en(lab, style->font_info.thick_en);
    gt_label_set_space(lab, style->space_x, style->space_y);
    gt_label_set_text(lab, text);
    gt_obj_set_touch_parent(lab, true);
    gt_obj_set_bubble_notify(lab, true);

    return lab;
}

static gt_obj_st * _get_obj_by_item(gt_obj_st * item, gt_obj_type_et type) {
    gt_obj_st * ret = NULL;
    for (gt_size_t i = 0, cnt = item->cnt_child; i < cnt; i++) {
        if (type != gt_obj_class_get_type(item->child[i])) {
            continue;
        }
        ret = item->child[i];
        break;
    }
    return ret;
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief create a listview obj
 *
 * @param parent listview's parent element
 * @return gt_obj_st* listview obj
 */
gt_obj_st * gt_listview_create(gt_obj_st * parent)
{
    if (NULL == parent) {
        return NULL;
    }
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    obj->bgcolor = gt_color_hex(0xffffff);

    _gt_listview_st * style = (_gt_listview_st * )obj;
    style->font_info.style_cn   = GT_CFG_DEFAULT_FONT_FAMILY_CN;
    style->font_info.style_en   = GT_CFG_DEFAULT_FONT_FAMILY_EN;
    style->font_info.style_fl   = GT_CFG_DEFAULT_FONT_FAMILY_FL;
    style->font_info.style_numb = GT_CFG_DEFAULT_FONT_FAMILY_NUMB;
    style->font_info.size       = GT_CFG_DEFAULT_FONT_SIZE;
    style->font_info.gray       = 1;
    style->font_info.thick_en   = 0;
    style->font_info.thick_cn   = 0;

    style->item_height = style->font_info.size + (style->font_info.size >> 1);
    style->font_color = gt_color_hex(0x000000);
    style->align = GT_ALIGN_CENTER_MID;
    style->space_x = 0;
    style->space_y = 0;
    style->septal_line = true;
    style->border_width = 2;
    style->border_color = gt_color_hex(0xc7c7c7);

    gt_obj_set_scroll_dir(obj, GT_SCROLL_VERTICAL);
    return obj;
}


void gt_listview_add_item(gt_obj_st * listview, char * text)
{
    if (!gt_obj_is_type(listview, GT_TYPE_LISTVIEW)) {
        return;
    }
    gt_obj_st * obj = _create_item_obj(listview);
    gt_obj_st * lab = _create_item_label(listview, obj, text);

    gt_obj_child_set_prop(obj, GT_OBJ_PROP_TYPE_FIXED, true);
    gt_obj_child_set_prop(obj, GT_OBJ_PROP_TYPE_INSIDE, true);
}

void gt_listview_set_item_height(gt_obj_st * listview, uint16_t height)
{
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->item_height = height;
    gt_event_send(listview, GT_EVENT_TYPE_DRAW_START, NULL);
}


void gt_listview_add_item_icon(gt_obj_st * listview, char * img, char * text)
{
    if (!gt_obj_is_type(listview, GT_TYPE_LISTVIEW)) {
        return;
    }
    gt_obj_st * obj = _create_item_obj(listview);
    uint16_t offset = _get_scale_offset(obj);

    if (img) {
        _create_item_icon(listview, obj, img);
    }
    _create_item_label(listview, obj, text);

    gt_obj_child_set_prop(obj, GT_OBJ_PROP_TYPE_FIXED, true);
    gt_obj_child_set_prop(obj, GT_OBJ_PROP_TYPE_INSIDE, true);
}

void gt_listview_set_icon_to_right(gt_obj_st * listview, bool enabled)
{
    _gt_listview_st * style = (_gt_listview_st * )listview;
    gt_obj_st * item = NULL, * icon = NULL, * lab = NULL;
    gt_size_t i = 0, cnt_obj = 0;
    gt_size_t idx_item = 0, cnt_item = 0;

    style->icon_right = enabled;

    for (i = 0, cnt_obj = listview->cnt_child; i < cnt_obj; i++) {
        item = listview->child[i];
        cnt_item = item->cnt_child;

        if (1 == cnt_item) {
            continue;   /** only label do not switch */
        }
        lab = _get_obj_by_item(item, GT_TYPE_LAB);
        icon = _get_obj_by_item(item, GT_TYPE_IMG);

        _center_icon_pos(icon, item, style->icon_right);
        _set_label_pos(lab, item, style->icon_right);
    }
    gt_event_send(listview, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_listview_set_highlight_mode(gt_obj_st * listview, bool enabled)
{
    if (!gt_obj_is_type(listview, GT_TYPE_LISTVIEW)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;

    if (false == enabled) {
        style->multiple = false;
        _reset_other_active_state(listview);
        gt_obj_set_state(_get_last_active_obj(listview), false);
    }
    style->highlight = enabled;

    for (gt_size_t i = 0, cnt = listview->cnt_child; i < cnt; i++) {
        gt_obj_set_trigger_mode(listview->child[i], style->highlight);
    }
}

void gt_listview_set_multiple(gt_obj_st * listview, bool enabled)
{
    if (!gt_obj_is_type(listview, GT_TYPE_LISTVIEW)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->multiple = enabled;
}

void gt_listview_set_selected_item_by_idx(gt_obj_st * listview, gt_size_t idx)
{
    if (!gt_obj_is_type(listview, GT_TYPE_LISTVIEW)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    if (0 == listview->cnt_child) {
        return;
    }
    if (idx > listview->cnt_child) {
        idx = listview->cnt_child - 1;
    }
    gt_event_send(listview->child[idx], GT_EVENT_TYPE_INPUT_PRESSED, NULL);
    gt_event_send(listview->child[idx], GT_EVENT_TYPE_INPUT_RELEASED, NULL);
}


gt_size_t gt_listview_get_list_items_cnt(gt_obj_st * listview)
{
    if (!gt_obj_is_type(listview, GT_TYPE_LISTVIEW)) {
        return -1;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    return listview->cnt_child;
}

gt_size_t gt_listview_get_idx_selected(gt_obj_st * listview)
{
    if (!gt_obj_is_type(listview, GT_TYPE_LISTVIEW)) {
        return -1;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    gt_size_t idx = -1;
    if (NULL == style->last_active) {
        return idx;
    }
    for (gt_size_t i = 0, cnt = listview->cnt_child; i < cnt; i++) {
        if (style->last_active != listview->child[i]) {
            continue;
        }
        idx = i;
        break;
    }
    return idx;
}

char * gt_listview_get_selected_item_text(gt_obj_st * listview)
{
    if (!gt_obj_is_type(listview, GT_TYPE_LISTVIEW)) {
        return NULL;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    if (NULL == style->last_active) {
        return NULL;
    }
    gt_obj_st * item = style->last_active;
    char * text = NULL;

    for (gt_size_t i = 0, cnt = item->cnt_child; i < cnt; i++) {
        if (GT_TYPE_LAB != gt_obj_class_get_type(item->child[i])) {
            continue;
        }
        text = gt_label_get_text(item->child[i]);
        break;
    }
    return text;
}

void gt_listview_set_border_color(gt_obj_st * listview, gt_color_t color)
{
    if (!gt_obj_is_type(listview, GT_TYPE_LISTVIEW)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->border_color = color;
}

void gt_listview_set_border_width(gt_obj_st * listview, uint8_t width)
{
    if (!gt_obj_is_type(listview, GT_TYPE_LISTVIEW)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->border_width = width;
}

void gt_listview_set_septal_line(gt_obj_st * listview, bool enabled)
{
    if (!gt_obj_is_type(listview, GT_TYPE_LISTVIEW)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->septal_line = enabled;

    for (gt_size_t i = 0, cnt = listview->cnt_child; i < cnt; i++) {
        gt_obj_set_septal_line(listview->child[i], enabled);
    }
    gt_event_send(listview, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_listview_set_font_size(gt_obj_st * listview, uint8_t size)
{
    if (!gt_obj_is_type(listview, GT_TYPE_LISTVIEW)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.size = size;
    _set_label_prop_by_type(listview, _LABEL_FONT_SIZE, size);
}
void gt_listview_set_font_gray(gt_obj_st * listview, uint8_t gray)
{
    if (!gt_obj_is_type(listview, GT_TYPE_LISTVIEW)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.gray = gray;
    _set_label_prop_by_type(listview, _LABEL_FONT_GRAY, gray);
}
void gt_listview_set_font_align(gt_obj_st * listview, gt_align_et align)
{
    if (!gt_obj_is_type(listview, GT_TYPE_LISTVIEW)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->align = align;
    _set_label_prop_by_type(listview, _LABEL_FONT_ALIGN, align);
}
void gt_listview_set_font_color(gt_obj_st * listview, gt_color_t color)
{
    if (!gt_obj_is_type(listview, GT_TYPE_LISTVIEW)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_color = color;
    _set_label_prop_by_type(listview, _LABEL_FONT_COLOR, color.full);
}

void gt_listview_set_font_family_en(gt_obj_st * listview, gt_family_t family)
{
    if (!gt_obj_is_type(listview, GT_TYPE_LISTVIEW)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.style_en = family;
    _set_label_prop_by_type(listview, _LABEL_FONT_EN, family);
}
void gt_listview_set_font_family_fl(gt_obj_st * listview, gt_family_t family)
{
    if (!gt_obj_is_type(listview, GT_TYPE_LISTVIEW)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.style_fl = family;
    _set_label_prop_by_type(listview, _LABEL_FONT_FL, family);
}
void gt_listview_set_font_family_cn(gt_obj_st * listview, gt_family_t family)
{
    if (!gt_obj_is_type(listview, GT_TYPE_LISTVIEW)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.style_cn = family;
    _set_label_prop_by_type(listview, _LABEL_FONT_CN, family);
}

void gt_listview_set_font_family_numb(gt_obj_st * listview, gt_family_t family)
{
    if (!gt_obj_is_type(listview, GT_TYPE_LISTVIEW)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.style_numb = family;
    _set_label_prop_by_type(listview, _LABEL_FONT_NUMB, family);
}

void gt_listview_set_font_thick_en(gt_obj_st * listview, uint8_t thick)
{
    if (!gt_obj_is_type(listview, GT_TYPE_LISTVIEW)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.thick_en = thick;
    _set_label_prop_by_type(listview, _LABEL_FONT_THICK_EN, thick);
}
void gt_listview_set_font_thick_cn(gt_obj_st * listview, uint8_t thick)
{
    if (!gt_obj_is_type(listview, GT_TYPE_LISTVIEW)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.thick_cn = thick;
    _set_label_prop_by_type(listview, _LABEL_FONT_THICK_CN, thick);
}
void gt_listview_set_space(gt_obj_st * listview, uint8_t space_x, uint8_t space_y)
{
    if (!gt_obj_is_type(listview, GT_TYPE_LISTVIEW)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->space_x = space_x;
    style->space_y = space_y;
    _set_label_prop_by_type(listview, _LABEL_FONT_SPACE, (space_x << 8) | space_y);
}
/* end ------------------------------------------------------------------*/
