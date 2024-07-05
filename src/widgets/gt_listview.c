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

#if GT_CFG_ENABLE_LISTVIEW
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../core/gt_obj_scroll.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../widgets/gt_obj.h"
#include "../widgets/gt_rect.h"

#include "./gt_label.h"
#include "./gt_img.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_LISTVIEW
#define MY_CLASS    &gt_listview_class

#define _DOUBLE_SCALE_ICON   25
#define _DOUBLE_SCALE_LABEL  75

#define _TRIPLE_SCALE_LEFT   20
#define _TRIPLE_SCALE_CENTER 60
#define _TRIPLE_SCALE_RIGHT  20


/* private typedef ------------------------------------------------------*/
typedef struct _double_scale_s {
    uint8_t icon;
    uint8_t label;
}_double_scale_st;

typedef struct _triple_scale_s {
    uint8_t left;
    uint8_t center;
    uint8_t right;
}_triple_scale_st;

typedef struct _column_s {
    uint8_t count;          /** The subsequent rows will be displayed column number of items */
    uint8_t idx;            /** The current index of the column, idx >= column will reset to 0. */
    gt_size_t offset_y;     /** offset line y value */
}_column_st;

typedef struct _gt_listview_s {
    gt_obj_st obj;
    gt_obj_st * last_active;
    uint16_t item_height;       /** The height of item */
    gt_font_info_st font_info;
    gt_color_t font_color;

    gt_color_t item_bg_color;
    gt_color_t border_color;

    _double_scale_st double_scale;
    _triple_scale_st triple_scale;
    gt_size_t scroll;       /** The offset of the last scroll */

    _column_st column;

    uint8_t align;      //@ref gt_align_et
    uint8_t border_width;
    uint8_t space_x;
    uint8_t space_y;

    uint8_t item_reduce;    /** The reduce of item area */
    uint8_t item_radius;    /** The radius of item */

    uint8_t highlight       : 1;    /** holding highlight state the last active item, 0: release restores the effect, 1: switch state */
    uint8_t septal_line     : 1;    /** Display septal line between items */
    uint8_t icon_right      : 1;
    uint8_t multiple        : 1;    /** Selected multiple item: 0: single selected; 1: multiple selected */
    uint8_t item_show_bg    : 1;
    uint8_t omit_sl         : 1;    /** Omit single line */
    uint8_t auto_scroll_sl  : 1;    /** auto scroll single line */
    uint8_t selected_effect : 1;    /** 1[defalut]: Enabled  effect; 0: close selected effect */
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
    _LABEL_FONT_ENCODING,
    _LABEL_OMIT_SL,         /** Omit single line */
    _LABEL_AUTO_SCROLL_SL,  /** Auto scroll single line */
}_label_prop_em;

typedef enum {
    _DOUBLE_POS_ICON = 0,
    _DOUBLE_POS_LABEL,
}_double_pos_em;

typedef enum {
    _TRIPLE_POS_LEFT = 0,
    _TRIPLE_POS_CENTER,
    _TRIPLE_POS_RIGHT,
}_triple_pos_em;

typedef struct _triple_param_s {
    gt_obj_st * item;
    gt_obj_st * left;
    gt_obj_st * center;
    gt_obj_st * right;
}_triple_param_st;

/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_listview_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = NULL,
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

    draw_focus(obj, 0);
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

    _gt_obj_move_child_by(obj, 0, gt_obj_scroll_get_y(obj) - style->scroll);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
    style->scroll = gt_obj_scroll_get_y(obj);
}

static inline uint8_t _column_plus_one(_gt_listview_st * style) {
    if (0 == style->column.count) {
        style->column.count = 1;
    }
    uint8_t current_idx = style->column.idx;
    style->column.idx = (current_idx + 1) % style->column.count;

    return current_idx;
}


static void _set_last_active_obj(gt_obj_st * listview, gt_obj_st * target) {
    _gt_listview_st * style = (_gt_listview_st * )listview;
    if (listview == target) {
        style->last_active = NULL;
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
    _gt_listview_st * style = (_gt_listview_st * )listview;
    if (false == style->selected_effect) {
        return;
    }
    if (gt_obj_get_trigger_mode(target)) {
        return;
    }
    gt_obj_set_state(target, GT_STATE_NONE);
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
    if (false == style->selected_effect) {
        return;
    }
    if (false == gt_obj_get_trigger_mode(style->last_active)) {
        return;
    }
    if (GT_STATE_NONE == gt_obj_get_state(style->last_active)) {
        gt_obj_set_state(style->last_active, GT_STATE_PRESSED);
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
    if (false == style->selected_effect) {
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
        gt_obj_set_state(item, GT_STATE_NONE);
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
                gt_label_set_font_align(obj->child[item], (gt_align_et)val);
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
            } else if (_LABEL_FONT_ENCODING == type) {
                gt_label_set_font_encoding(obj->child[item], (gt_encoding_et)val);
            } else if (_LABEL_OMIT_SL == type) {
                gt_label_set_omit_single_line(obj->child[item], (bool)val);
            } else if (_LABEL_AUTO_SCROLL_SL == type) {
                gt_label_set_auto_scroll_single_line(obj->child[item], (bool)val);
            }
        }
    }
}

static inline uint16_t _get_double_real_width(_gt_listview_st * style, gt_obj_st * item, _double_pos_em pos) {
    uint16_t total = style->double_scale.icon + style->double_scale.label;
    uint8_t value = 0;

    if (_DOUBLE_POS_ICON == pos) {
        value = style->double_scale.icon;
    } else if (_DOUBLE_POS_LABEL == pos) {
        value = style->double_scale.label;
    }
    return (gt_obj_get_w(item) * value) / total;
}

static inline uint16_t _get_triple_real_width(_gt_listview_st * style, gt_obj_st * item, _triple_pos_em pos) {
    uint16_t total = style->triple_scale.left + style->triple_scale.center + style->triple_scale.right;
    uint8_t value = 0;

    if (_TRIPLE_POS_LEFT == pos) {
        value = style->triple_scale.left;
    } else if (_TRIPLE_POS_CENTER == pos) {
        value = style->triple_scale.center;
    } else if (_TRIPLE_POS_RIGHT == pos) {
        value = style->triple_scale.right;
    }
    return (gt_obj_get_w(item) * value) / total;
}

static gt_obj_st * _create_item_obj(gt_obj_st * listview) {
    _gt_listview_st * style = (_gt_listview_st * )listview;
    gt_size_t offset_line = listview->area.y;
    gt_size_t width = listview->area.w;
    uint8_t idx = _column_plus_one(style);

    if (style->column.count > 1) {
        width = listview->area.w / style->column.count;
    }
    if (listview->cnt_child) {
        if (idx) {
            offset_line = style->column.offset_y;
        } else {
            offset_line = listview->child[listview->cnt_child - 1]->area.y + style->item_height;
            offset_line += gt_abs(listview->process_attr.scroll.y);
            style->column.offset_y = offset_line;
        }
    } else {
        style->column.offset_y = listview->area.y;
    }
    gt_obj_st * obj = gt_obj_create(listview);
    /** set object item size */
    obj->area.x = listview->area.x + idx * width;
    obj->area.y = offset_line + listview->process_attr.scroll.y;
    obj->area.w = width;
    obj->area.h = style->item_height;

    gt_obj_set_fixed(obj, true);
    gt_obj_set_inside(obj, true);
    gt_obj_set_mask_effect(obj, style->selected_effect);
    gt_obj_set_bubble_notify(obj, true);
    gt_obj_set_septal_line(obj, style->septal_line);
    gt_obj_set_trigger_mode(obj, (gt_obj_trigger_mode_et)style->highlight);

    gt_obj_set_bgcolor(obj, style->item_bg_color);
    gt_obj_show_bg(obj, style->item_show_bg);
    gt_obj_set_reduce(obj, style->item_reduce);
    gt_obj_set_radius(obj, style->item_radius);

    return obj;
}

static void _center_icon_pos(gt_obj_st * icon, gt_obj_st * item, gt_size_t offset_x, uint16_t width) {
    gt_area_copy(&icon->area, &item->area);
    icon->area.w = width;

    if (NULL == gt_img_get_src(icon)) {
        return ;
    }
    icon->area.x += ((width - gt_img_get_width(icon)) >> 1) + offset_x;
    icon->area.y = ((gt_obj_get_h(item) - gt_img_get_height(icon)) >> 1) + gt_obj_get_y(icon);
}

static void _set_label_pos(gt_obj_st * lab, gt_obj_st * item, gt_size_t offset_x, uint16_t width) {
    gt_area_copy(&lab->area, &item->area);
    lab->area.x += offset_x;
    lab->area.w = width;
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
    gt_obj_st * icon = gt_img_create(obj);

    gt_img_set_src(icon, img);
    gt_obj_set_inside(icon, true);

    gt_obj_set_touch_parent(icon, true);
    gt_obj_set_bubble_notify(icon, true);

    return icon;
}

/**
 * @brief Set label object property
 *
 * @param listview
 * @param lab
 * @param font_info_p Using extra font information to init label object.
 */
static inline void _set_label_prop(gt_obj_st * listview, gt_obj_st * lab, gt_font_info_st * font_info_p) {
    _gt_listview_st * style = (_gt_listview_st * )listview;
    if (NULL == font_info_p) {
        font_info_p = &style->font_info;
    }

    gt_label_set_font_align(lab, style->align);
    gt_label_set_font_color(lab, style->font_color);
    gt_label_set_font_family_cn(lab, font_info_p->style_cn);
    gt_label_set_font_family_en(lab, font_info_p->style_en);
    gt_label_set_font_family_fl(lab, font_info_p->style_fl);
    gt_label_set_font_family_numb(lab, font_info_p->style_numb);
    gt_label_set_font_size(lab, font_info_p->size);
    gt_label_set_font_gray(lab, font_info_p->gray);
    gt_label_set_font_thick_cn(lab, font_info_p->thick_cn);
    gt_label_set_font_thick_en(lab, font_info_p->thick_en);
    gt_label_set_font_encoding(lab, font_info_p->encoding);
    gt_label_set_space(lab, style->space_x, style->space_y);
    gt_obj_set_touch_parent(lab, true);
    gt_obj_set_bubble_notify(lab, true);
    gt_obj_set_inside(lab, true);

    if (style->omit_sl) {
        gt_label_set_omit_single_line(lab, true);
    } else if (style->auto_scroll_sl) {
        gt_label_set_auto_scroll_single_line(lab, true);
    }
}

/**
 * @brief Create a label object
 *
 * @param listview listview object
 * @param obj item object
 * @param text label text
 * @return gt_obj_st* label object
 */
static gt_obj_st * _create_item_label(gt_obj_st * listview, gt_obj_st * obj, char const * text) {
    gt_obj_st * lab = gt_label_create(obj);

    _set_label_prop(listview, lab, NULL);
    gt_label_set_text(lab, text);

    return lab;
}

/**
 * @brief Create a label object
 *
 * @param listview
 * @param obj
 * @param text
 * @param len encoding byte count, such as: strlen(text).
 * @return gt_obj_st* label object
 */
static gt_obj_st * _create_item_label_by_len(gt_obj_st * listview, gt_obj_st * obj,
                                                char const * text, uint16_t len, gt_font_info_st * font_p) {
    gt_obj_st * lab = gt_label_create(obj);

    _set_label_prop(listview, lab, font_p);
    gt_label_set_text_by_len(lab, text, len);

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

static void _double_calc_pos(_gt_listview_st * style, gt_obj_st * obj, gt_obj_st * icon, gt_obj_st * lab) {
    uint16_t width_icon = _get_double_real_width(style, obj, _DOUBLE_POS_ICON);
    uint16_t width_lab = _get_double_real_width(style, obj, _DOUBLE_POS_LABEL);

    if (style->icon_right) {
        _center_icon_pos(icon, obj, width_lab, width_icon);
        _set_label_pos(lab, obj, 0, width_lab);
    } else {
        _center_icon_pos(icon, obj, 0, width_icon);
        _set_label_pos(lab, obj, width_icon, width_lab);
    }
}

static void _triple_calc_pos(_gt_listview_st * style, _triple_param_st * param) {
    uint16_t width_left = _get_triple_real_width(style, param->item, _TRIPLE_POS_LEFT);
    uint16_t width_center = _get_triple_real_width(style, param->item, _TRIPLE_POS_CENTER);
    uint16_t width_right = _get_triple_real_width(style, param->item, _TRIPLE_POS_RIGHT);

    _center_icon_pos(param->left, param->item, 0, width_left);
    _set_label_pos(param->center, param->item, width_left, width_center);
    _center_icon_pos(param->right, param->item, width_left + width_center, width_right);
}

static gt_obj_st * _set_item_obj_prop(gt_obj_st * obj, gt_obj_st * listview) {
    gt_obj_child_set_prop(obj, GT_OBJ_PROP_TYPE_FIXED, true);
    gt_obj_child_set_prop(obj, GT_OBJ_PROP_TYPE_INSIDE, true);

    gt_event_send(listview, GT_EVENT_TYPE_DRAW_START, NULL);
    return obj;
}

#if GT_LISTVIEW_USE_ELEMENT_TYPE_RECT
/**
 * @brief Create a rect object
 *
 * @param listview
 * @param obj
 * @param rect rect object detail
 * @return gt_obj_st*
 */
static gt_obj_st * _create_item_rect(gt_obj_st * listview, gt_obj_st * obj) {
    gt_obj_st * r = gt_rect_create(obj);

    gt_obj_set_inside(r, true);

    gt_obj_set_touch_parent(r, true);
    gt_obj_set_bubble_notify(r, true);
    return r;
}

static inline void _set_rect_prop(gt_obj_st * rect, gt_listview_rect_st const * const prop) {
    if (false == gt_obj_is_type(rect, GT_TYPE_RECT)) {
        return;
    }
    gt_rect_set_bg_color(rect, prop->bg_color);
    gt_rect_set_color_border(rect, prop->border_color);
    gt_rect_set_border(rect, prop->border);
    gt_rect_set_radius(rect, prop->radius);
    gt_rect_set_fill(rect, prop->is_fill);
}
#endif


/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_listview_create(gt_obj_st * parent)
{
    if (NULL == parent) {
        return NULL;
    }
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    obj->fixed = false;

    _gt_listview_st * style = (_gt_listview_st * )obj;
    gt_font_info_init(&style->font_info);

    style->item_height = style->font_info.size + (style->font_info.size >> 1);
    style->font_color = gt_color_hex(0x000000);
    style->align = GT_ALIGN_CENTER_MID;
    style->space_x = 0;
    style->space_y = 0;
    style->septal_line = true;
    style->border_width = 2;
    style->border_color = gt_color_hex(0xc7c7c7);

    style->item_bg_color =  obj->bgcolor;
    style->item_show_bg = false;
    style->item_reduce = 4;
    style->item_radius = 10;

    style->column.count = 1;
    style->column.idx = 0;

    style->double_scale.icon = _DOUBLE_SCALE_ICON;
    style->double_scale.label =_DOUBLE_SCALE_LABEL;

    style->triple_scale.left = _TRIPLE_SCALE_LEFT;
    style->triple_scale.center = _TRIPLE_SCALE_CENTER;
    style->triple_scale.right = _TRIPLE_SCALE_RIGHT;

    style->selected_effect = true;

    gt_obj_set_scroll_dir(obj, GT_SCROLL_VERTICAL);
    return obj;
}

void gt_listview_set_next_row_item_count(gt_obj_st * listview, uint8_t count)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->column.count = count;
    style->column.idx = 0;
}

gt_obj_st * gt_listview_add_item(gt_obj_st * listview, char const * text)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return NULL;
    }
    gt_obj_st * obj = _create_item_obj(listview);
    gt_obj_st * lab = _create_item_label(listview, obj, text);

    _set_label_pos(lab, obj, 0, gt_obj_get_w(obj));

    return _set_item_obj_prop(obj, listview);
}

gt_obj_st * gt_listview_add_item_icon(gt_obj_st * listview, char * img, char const * text)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return NULL;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    gt_obj_st * obj = _create_item_obj(listview);

    gt_obj_st * icon = _create_item_icon(listview, obj, img);
    gt_obj_st * lab = _create_item_label(listview, obj, text);

    _double_calc_pos(style, obj, icon, lab);

    return _set_item_obj_prop(obj, listview);
}

gt_obj_st * gt_listview_add_item_icons(gt_obj_st * listview, char * left_icon, char const * text, char * right_icon)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return NULL;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    gt_obj_st * obj = _create_item_obj(listview);
    _triple_param_st param = { obj, NULL, NULL, NULL };

    param.left = _create_item_icon(listview, obj, left_icon);
    param.center = _create_item_label(listview, obj, text);
    param.right = _create_item_icon(listview, obj, right_icon);

    _triple_calc_pos(style, &param);

    return _set_item_obj_prop(obj, listview);
}

gt_obj_st * gt_listview_add_item_by_param(gt_obj_st * listview, gt_listview_param_st * param)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return NULL;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    gt_obj_st * obj = _create_item_obj(listview);
    _triple_param_st pm = { obj, NULL, NULL, NULL };

    if (param->left_icon) {
        pm.left = _create_item_icon(listview, obj, param->left_icon);
    }
    if (param->text_ptr) {
#if GT_LISTVIEW_CUSTOM_FONT_STYLE
        pm.center = _create_item_label_by_len(listview, obj, param->text_ptr, param->text_len, param->font_info_p);
#else
        pm.center = _create_item_label_by_len(listview, obj, param->text_ptr, param->text_len, NULL);
#endif
    }
    if (param->right_icon) {
        pm.right = _create_item_icon(listview, obj, param->right_icon);
    }

    if (pm.left && pm.center && pm.right) {
        _triple_calc_pos(style, &pm);
    } else if (pm.left && pm.center) {
        _double_calc_pos(style, obj, pm.left, pm.center);
    } else if (pm.center) {
        _set_label_pos(pm.center, obj, 0, gt_obj_get_w(obj));
    }

    return _set_item_obj_prop(obj, listview);
}

gt_obj_st * gt_listview_custom_item_set_element(gt_obj_st * listview, gt_listview_custom_item_st * item)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return NULL;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    gt_obj_st * obj = NULL;
    gt_obj_st * element = NULL;
    gt_font_info_st * font_p = NULL;

    if (_GT_LISTVIEW_ELEMENT_TYPE_MAX <= item->type) {
        return NULL;
    }

    /** item object */
    if (item->item_idx < listview->cnt_child) {
        obj = listview->child[item->item_idx];
    } else {
        obj = _set_item_obj_prop(_create_item_obj(listview), listview);
    }

    /** element */
    if (item->element_idx < obj->cnt_child) {
        element = obj->child[item->element_idx];
    }
    if (GT_LISTVIEW_ELEMENT_TYPE_IMG == item->type) {
        if (NULL == element) {
            element = _create_item_icon(listview, obj, item->src);
        } else {
            gt_img_set_src(element, item->src);
        }
#if GT_LISTVIEW_USE_ELEMENT_TYPE_RECT
    } if (GT_LISTVIEW_ELEMENT_TYPE_RECT == item->type) {
        if (NULL == element) {
            element = _create_item_rect(listview, obj);
        }
        _set_rect_prop(element, &item->rect);
#endif
    } else {
        if (NULL == element) {
        #if GT_LISTVIEW_CUSTOM_FONT_STYLE
            font_p = item->font_info_p;
        #endif
            element = _create_item_label_by_len(listview, obj, item->src, item->src_len, font_p);
        } else {
            gt_label_set_text_by_len(element, item->src, item->src_len);
        }
    }
    element->area = item->area;
    element->area.x += obj->area.x;
    element->area.y += obj->area.y;
    gt_event_send(listview, GT_EVENT_TYPE_DRAW_START, NULL);

    return obj;
}

void gt_listview_clear_all_items(gt_obj_st * listview)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return ;
    }
    listview->process_attr.scroll.x = 0;
    listview->process_attr.scroll.y = 0;

    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->column.idx = 0;
    style->column.offset_y = 0;
    style->scroll = 0;

    _gt_obj_class_destroy_children(listview);
    gt_event_send(listview, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_listview_set_scale(gt_obj_st * listview, uint8_t icon, uint8_t label)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    gt_obj_st * item = NULL;
    gt_obj_st * icon_obj = NULL;
    gt_obj_st * lab_obj = NULL;

    style->double_scale.icon = icon;
    style->double_scale.label = label;

    for (gt_size_t i = 0, cnt = listview->cnt_child; i < cnt; ++i) {
        item = listview->child[i];
        if (2 != item->cnt_child) {
            continue;   /** only label or two icon, do not switch */
        }
        icon_obj = _get_obj_by_item(item, GT_TYPE_IMG);
        lab_obj = _get_obj_by_item(item, GT_TYPE_LAB);

        _double_calc_pos(style, item, icon_obj, lab_obj);
    }

    gt_event_send(listview, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_listview_set_scale_triple(gt_obj_st * listview, uint8_t left, uint8_t center,  uint8_t right)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    _triple_param_st param = { NULL, NULL, NULL, NULL };
    bool is_first = true;

    style->triple_scale.left = left;
    style->triple_scale.center = center;
    style->triple_scale.right = right;

    for (gt_size_t i = 0, cnt = listview->cnt_child; i < cnt; ++i) {
        param.item = listview->child[i];
        if (3 != param.item->cnt_child) {
            continue;
        }
        is_first = true;

        for (gt_size_t j = 0; j < 3; ++j) {
            if (GT_TYPE_LAB == gt_obj_class_get_type(param.item->child[j])) {
                param.center = param.item->child[j];
            } else if (GT_TYPE_IMG == gt_obj_class_get_type(param.item->child[j])) {
                if (is_first) {
                    param.left = param.item->child[j];
                    is_first = false;
                    continue;
                }
                param.right = param.item->child[j];
            }
        }
        _triple_calc_pos(style, &param);
    }

    gt_event_send(listview, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_listview_set_icon_to_right(gt_obj_st * listview, bool enabled)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    gt_obj_st * item = NULL, * icon = NULL, * lab = NULL;
    gt_size_t i = 0, cnt_obj = 0;

    style->icon_right = enabled;

    for (i = 0, cnt_obj = listview->cnt_child; i < cnt_obj; i++) {
        item = listview->child[i];

        if (2 != item->cnt_child) {
            continue;   /** only label or two icon, do not switch */
        }
        lab = _get_obj_by_item(item, GT_TYPE_LAB);
        icon = _get_obj_by_item(item, GT_TYPE_IMG);

        _double_calc_pos(style, item, icon, lab);
    }
    gt_event_send(listview, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_listview_set_item_height(gt_obj_st * listview, uint16_t height)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->item_height = height;
    gt_event_send(listview, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_listview_set_selected_effect(gt_obj_st * listview, bool enabled)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;

    if (style->selected_effect == (uint8_t)enabled) {
        return;
    }
    style->selected_effect = enabled ? 1 : 0;
    for (gt_size_t i = 0, cnt = listview->cnt_child; i < cnt; i++) {
        gt_obj_set_mask_effect(listview->child[i], enabled);
    }
}

void gt_listview_set_highlight_mode(gt_obj_st * listview, bool enabled)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;

    if (false == enabled) {
        style->multiple = false;
        _reset_other_active_state(listview);
        gt_obj_set_state(_get_last_active_obj(listview), false);
    }
    style->highlight = enabled ? 1 : 0;

    for (gt_size_t i = 0, cnt = listview->cnt_child; i < cnt; i++) {
        gt_obj_set_trigger_mode(listview->child[i], style->highlight);
    }
}

void gt_listview_set_multiple(gt_obj_st * listview, bool enabled)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->multiple = enabled ? 1 : 0;
}

void gt_listview_set_selected_item_by_idx(gt_obj_st * listview, gt_size_t idx)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    if (0 == listview->cnt_child) {
        return;
    }
    if (idx > listview->cnt_child) {
        idx = listview->cnt_child - 1;
    }
    gt_event_send(listview->child[idx], GT_EVENT_TYPE_INPUT_PRESSED, NULL);
    gt_event_send(listview->child[idx], GT_EVENT_TYPE_INPUT_RELEASED, NULL);
}

void gt_listview_set_bg_color(gt_obj_st * listview, gt_color_t color)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    listview->bgcolor = color;
    gt_event_send(listview, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_listview_set_item_bg_color(gt_obj_st * listview, gt_color_t color)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->item_bg_color = color;
    for (gt_size_t i = 0, cnt = listview->cnt_child; i < cnt; i++) {
        gt_obj_set_bgcolor(listview->child[i], color);
    }
    gt_event_send(listview, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_listview_show_item_bg(gt_obj_st * listview, bool show)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->item_show_bg = show;
    for (gt_size_t i = 0, cnt = listview->cnt_child; i < cnt; i++) {
        gt_obj_show_bg(listview->child[i], show);
    }
    gt_event_send(listview, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_listview_set_item_reduce(gt_obj_st * listview, uint8_t reduce)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->item_reduce = reduce;
    for (gt_size_t i = 0, cnt = listview->cnt_child; i < cnt; i++) {
        gt_obj_set_reduce(listview->child[i], reduce);
    }
    gt_event_send(listview, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_listview_set_item_radius(gt_obj_st * listview, uint8_t radius)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->item_radius = radius;
    for (gt_size_t i = 0, cnt = listview->cnt_child; i < cnt; i++) {
        gt_obj_set_radius(listview->child[i], radius);
    }
    gt_event_send(listview, GT_EVENT_TYPE_DRAW_START, NULL);
}

gt_size_t gt_listview_get_list_items_cnt(gt_obj_st * listview)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return -1;
    }
    return listview->cnt_child;
}

gt_size_t gt_listview_get_idx_selected(gt_obj_st * listview)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
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
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return NULL;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    if (NULL == style->last_active) {
        return NULL;
    }
    gt_obj_st * item = style->last_active;
    char * text = NULL;

    for (gt_size_t i = 0, cnt = item->cnt_child; i < cnt; i++) {
        if (false == gt_obj_is_type(item->child[i], GT_TYPE_LAB)) {
            continue;
        }
        text = gt_label_get_text(item->child[i]);
        break;
    }
    return text;
}

void gt_listview_set_border_color(gt_obj_st * listview, gt_color_t color)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->border_color = color;
}

void gt_listview_set_border_width(gt_obj_st * listview, uint8_t width)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->border_width = width;
}

void gt_listview_set_septal_line(gt_obj_st * listview, bool enabled)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
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
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.size = size;
    _set_label_prop_by_type(listview, _LABEL_FONT_SIZE, size);
}
void gt_listview_set_font_gray(gt_obj_st * listview, uint8_t gray)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.gray = gray;
    _set_label_prop_by_type(listview, _LABEL_FONT_GRAY, gray);
}
void gt_listview_set_font_align(gt_obj_st * listview, gt_align_et align)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->align = align;
    _set_label_prop_by_type(listview, _LABEL_FONT_ALIGN, align);
}
void gt_listview_set_font_color(gt_obj_st * listview, gt_color_t color)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_color = color;
    _set_label_prop_by_type(listview, _LABEL_FONT_COLOR, color.full);
}

void gt_listview_set_font_family_en(gt_obj_st * listview, gt_family_t family)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.style_en = family;
    _set_label_prop_by_type(listview, _LABEL_FONT_EN, family);
}
void gt_listview_set_font_family_fl(gt_obj_st * listview, gt_family_t family)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.style_fl = family;
    _set_label_prop_by_type(listview, _LABEL_FONT_FL, family);
}
void gt_listview_set_font_family_cn(gt_obj_st * listview, gt_family_t family)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.style_cn = family;
    _set_label_prop_by_type(listview, _LABEL_FONT_CN, family);
}

void gt_listview_set_font_family_numb(gt_obj_st * listview, gt_family_t family)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.style_numb = family;
    _set_label_prop_by_type(listview, _LABEL_FONT_NUMB, family);
}

void gt_listview_set_font_thick_en(gt_obj_st * listview, uint8_t thick)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.thick_en = thick;
    _set_label_prop_by_type(listview, _LABEL_FONT_THICK_EN, thick);
}

void gt_listview_set_font_thick_cn(gt_obj_st * listview, uint8_t thick)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.thick_cn = thick;
    _set_label_prop_by_type(listview, _LABEL_FONT_THICK_CN, thick);
}

void gt_listview_set_font_encoding(gt_obj_st * listview, gt_encoding_et encoding)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->font_info.encoding = encoding;
    _set_label_prop_by_type(listview, _LABEL_FONT_ENCODING, encoding);
}

void gt_listview_set_space(gt_obj_st * listview, uint8_t space_x, uint8_t space_y)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->space_x = space_x;
    style->space_y = space_y;
    _set_label_prop_by_type(listview, _LABEL_FONT_SPACE, (space_x << 8) | space_y);
}

void gt_listview_set_label_omit_single_line_by(gt_obj_st * listview, bool is_omit)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->omit_sl = is_omit ? 1 : 0;
    style->auto_scroll_sl = 0;
    _set_label_prop_by_type(listview, _LABEL_OMIT_SL, is_omit);
}

void gt_listview_set_label_auto_scroll_single_line_by(gt_obj_st * listview, bool is_auto_scroll)
{
    if (!gt_obj_is_type(listview, OBJ_TYPE)) {
        return;
    }
    _gt_listview_st * style = (_gt_listview_st * )listview;
    style->omit_sl = 0;
    style->auto_scroll_sl = is_auto_scroll ? 1 : 0;
    _set_label_prop_by_type(listview, _LABEL_AUTO_SCROLL_SL, is_auto_scroll);
}

#endif  /** GT_CFG_ENABLE_LISTVIEW */
/* end ------------------------------------------------------------------*/
