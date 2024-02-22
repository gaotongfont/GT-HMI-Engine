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
    uint8_t space_x;
    uint8_t space_y;

    uint16_t selected_mode  : 1;
    uint16_t reserved       : 15;
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
    rect_attr.border_width   = 2;
    rect_attr.radius         = 2;
    rect_attr.bg_opa         = obj->opa;
    rect_attr.border_color  = gt_color_hex(0xc7c7c7);
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

static void _update_text_item_y(gt_obj_st * item, gt_size_t offset) {
    if (NULL == item) {
        return;
    }
    gt_obj_set_pos(item, item->area.x, offset);

    if (0 == item->cnt_child) {
        return;
    }
    gt_obj_set_pos(item->child[0], item->child[0]->area.x, offset);
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
        _update_text_item_y(obj->child[i], offset);
        offset += style->item_height;
    }
}

static void _set_last_active_obj(gt_obj_st * listview, gt_obj_st * target) {
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->last_active = target;
}

static void _reset_last_active_state(gt_obj_st * target) {
    if (gt_obj_get_trigger_mode(target)) {
        return;
    }
    gt_obj_set_state(target, false);
}

/**
 * @brief obj event handler call back
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);

    if (GT_EVENT_TYPE_INPUT_SCROLL == code) {
        _scrolling_handler(obj);
    }
    else if (GT_EVENT_TYPE_INPUT_PRESSED == code) {
        _set_last_active_obj(obj, e->origin);
    }
    else if (GT_EVENT_TYPE_INPUT_RELEASED == code) {
        _set_last_active_obj(obj, e->origin);
    }
    else if (GT_EVENT_TYPE_INPUT_SCROLL_END) {
        _reset_last_active_state(e->origin);
    }
}

static void _set_label_prop_by_type(gt_obj_st * listview, _label_prop_em type, int val) {
    gt_size_t item = 0, cnt_item = 0;
    gt_obj_st * obj = NULL;

    for (gt_size_t i = 0, cnt = listview->cnt_child; i < cnt; i++) {
        obj = listview->child[i];
        for (item = 0, cnt_item = obj->cnt_child; item < cnt_item; item++) {
            if (GT_TYPE_LAB != gt_obj_class_get_type(obj->child[item])) {
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

/* global functions / API interface -------------------------------------*/

/**
 * @brief create a listview obj
 *
 * @param parent listview's parent element
 * @return gt_obj_st* listview obj
 */
gt_obj_st * gt_listview_create(gt_obj_st * parent)
{
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

    style->selected_mode = 1;

    gt_obj_set_scroll_dir(obj, GT_SCROLL_VERTICAL);
    return obj;
}


void gt_listview_add_item(gt_obj_st * listview, char * item)
{
    if (NULL == listview) {
        return;
    }
    if (GT_TYPE_LISTVIEW  != gt_obj_class_get_type(listview)) {
        return;
    }
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
    gt_obj_set_bubble_notify(obj, true);
    // TODO
    gt_obj_set_septal_line(obj, true);
    gt_obj_set_mask_effect(obj, true);
    // gt_obj_set_trigger_mode(obj, true);

    /** label */
    gt_obj_st * lab = gt_label_create(obj);
    gt_area_copy(&lab->area, &obj->area);
    gt_obj_set_bubble_notify(lab, true);
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
    gt_label_set_text(lab, item);
    gt_obj_set_touch_parent(lab, true);

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
    if (NULL == listview) {
        return;
    }
    if (GT_TYPE_LISTVIEW != gt_obj_class_get_type(listview)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    // TODO
}

void gt_listview_set_selected_item_by_idx(gt_obj_st * listview, gt_size_t idx) {
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
    _gt_listview_st * style = (_gt_listview_st * )listview;
    return listview->cnt_child;
}

gt_size_t gt_listview_get_idx_selected(gt_obj_st * listview)
{
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

void gt_listview_set_font_size(gt_obj_st * listview, uint8_t size)
{
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.size = size;
    _set_label_prop_by_type(listview, _LABEL_FONT_SIZE, size);
}
void gt_listview_set_font_gray(gt_obj_st * listview, uint8_t gray)
{
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.gray = gray;
    _set_label_prop_by_type(listview, _LABEL_FONT_GRAY, gray);
}
void gt_listview_set_font_align(gt_obj_st * listview, uint8_t align)
{
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->align = align;
    _set_label_prop_by_type(listview, _LABEL_FONT_ALIGN, align);
}
void gt_listview_set_font_color(gt_obj_st * listview, gt_color_t color)
{
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_color = color;
    _set_label_prop_by_type(listview, _LABEL_FONT_COLOR, color.full);
}

void gt_listview_set_font_family_en(gt_obj_st * listview, gt_family_t family)
{
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.style_en = family;
    _set_label_prop_by_type(listview, _LABEL_FONT_EN, family);
}
void gt_listview_set_font_family_fl(gt_obj_st * listview, gt_family_t family)
{
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.style_fl = family;
    _set_label_prop_by_type(listview, _LABEL_FONT_FL, family);
}
void gt_listview_set_font_family_cn(gt_obj_st * listview, gt_family_t family)
{
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.style_cn = family;
    _set_label_prop_by_type(listview, _LABEL_FONT_CN, family);
}

void gt_listview_set_font_family_numb(gt_obj_st * listview, gt_family_t family)
{
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.style_numb = family;
    _set_label_prop_by_type(listview, _LABEL_FONT_NUMB, family);
}

void gt_listview_set_font_thick_en(gt_obj_st * listview, uint8_t thick)
{
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.thick_en = thick;
    _set_label_prop_by_type(listview, _LABEL_FONT_THICK_EN, thick);
}
void gt_listview_set_font_thick_cn(gt_obj_st * listview, uint8_t thick)
{
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.thick_cn = thick;
    _set_label_prop_by_type(listview, _LABEL_FONT_THICK_CN, thick);
}
void gt_listview_set_space(gt_obj_st * listview, uint8_t space_x, uint8_t space_y)
{
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->space_x = space_x;
    style->space_y = space_y;
    _set_label_prop_by_type(listview, _LABEL_FONT_SPACE, (space_x << 8) | space_y);
}
/* end ------------------------------------------------------------------*/
