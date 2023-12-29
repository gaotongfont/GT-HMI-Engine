/**
 * @file gt_roller.c
 * @author Feyoung
 * @brief Roller widget When the selection is modified,
 *      the GT_EVENT_TYPE_UPDATE_VALUE event of the current control is fired.
 * @version 0.1
 * @date 2023-11-21 10:47:15
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_roller.h"

#if GT_CFG_ENABLE_ROLLER
#include "../others/gt_types.h"
#include "../core/gt_style.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "../core/gt_obj_scroll.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_draw.h"
#include "../others/gt_anim.h"
#include "gt_label.h"


/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_ROLLER
#define MY_CLASS    &gt_roller_class



/* private typedef ------------------------------------------------------*/
/**
 * @brief Adsorption alignment state after the end of rolling
 */
typedef enum {
    _SCROLL_SNAP_NONE = 0,  //No scroll snap
    _SCROLL_SNAP_READY,     //Ready to scroll snap
    _SCROLL_SNAP_DONE,      //Scroll snap done, ready to next scroll snap. ignore repeat scrolling event
}_gt_roller_state_em;

/**
 * @brief The register struct of roller
 */
typedef struct {
    uint8_t mode : 2;       //Roller running mode @ref gt_roller_mode_em
    uint8_t state : 2;      //Item scroll snap state @ref _gt_roller_state_em
    uint8_t reserved : 4;
}_gt_roller_reg_st;

/**
 * @brief Roller style struct
 */
typedef struct _gt_roller_s {
    gt_obj_st * text;                   // Options text, free by class
    gt_anim_st * anim_snap;             // The anim object to scroll snap
    uint16_t scroll_vertical_length;    // The length of vertical scroll
    uint16_t selected;                  // The index of selected item, Infinite mode need to calculate the real index
    uint8_t count_total;                // The total count of items
    uint8_t count_of_show;              // The count of items to show within area
    uint8_t line_space;                 // The space between lines
    _gt_roller_reg_st reg;
}_gt_roller_st;


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_roller_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_roller_st)
};

/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj) {
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill    = true;
    rect_attr.border_width   = 2;
    rect_attr.border_color   = gt_color_hex(0xc7c7c7);
    rect_attr.bg_opa         = obj->opa;
    rect_attr.bg_color       = gt_color_white();
    rect_attr.radius         = 2;

    draw_bg(obj->draw_ctx, &rect_attr, &obj->area);

    /** selected */
    gt_graph_init_rect_attr(&rect_attr);
    gt_area_st selected = {
        .x = obj->area.x,
        .y = obj->area.y + (style->count_of_show >> 1) * (obj->area.h / style->count_of_show),
        .w = obj->area.w,
        .h = obj->area.h / style->count_of_show
    };
    rect_attr.reg.is_fill    = true;
    rect_attr.bg_color       = gt_color_hex(0x00a8ff);
    rect_attr.bg_opa         = obj->opa;

    draw_bg(obj->draw_ctx, &rect_attr, &selected);

    draw_focus(obj, 0);
}

static void _deinit_cb(gt_obj_st * obj) {

}

/**
 * @brief The index is calculated according to the actual number of items
 *
 * @param index The index of selected item
 * @param total The total count of lines
 * @return uint8_t The index of selected item
 */
static inline uint16_t _get_infinite_real_selected_index(gt_size_t index, uint16_t total) {
    return index % (total / GT_ROLLER_INFINITE_PAGES_COUNT);
}

/**
 * @brief Reset the change increment in infinite scroll mode
 *
 * @param total_length Total rolling distance
 * @return uint16_t The increase in variation
 */
static inline uint16_t _get_infinite_scroll_increment(uint16_t total_length) {
    return ((GT_ROLLER_INFINITE_PAGES_COUNT >> 1) * total_length) / GT_ROLLER_INFINITE_PAGES_COUNT;
}

/**
 * @brief Calc total line count and total height value
 *
 * @param obj
 * @param options The text of items
 */
static void _calc_scroll_vertical_length(gt_obj_st * obj, char const * const options) {
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    gt_size_t len = strlen(options);
    gt_size_t count = 1;
    gt_size_t i = 0;
    for (i = 0; i < len; i++) {
        if ('\n' == options[i]) {
            ++count;
        }
    }
    style->count_total = count;
    if (GT_ROLLER_MODE_INFINITE == style->reg.mode) {
        style->scroll_vertical_length = count *
            (gt_label_get_font_size(style->text) + gt_label_get_space_y(style->text));
    } else {
        style->scroll_vertical_length = (count - style->count_of_show) *
            (gt_label_get_font_size(style->text) + gt_label_get_space_y(style->text));
    }
}

/**
 * @brief limit label position y value
 *
 * @param obj
 * @param val The y value of current time.
 * @return gt_size_t The real value where label be set to.
 */
static gt_size_t _normal_limit_y(gt_obj_st * obj, gt_size_t val) {
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    gt_size_t ret_val = val;
    gt_size_t top_y = (style->count_of_show >> 1) * (obj->area.h / style->count_of_show);
    gt_size_t bottom_y = - style->scroll_vertical_length - top_y;

    if (GT_ROLLER_MODE_NORMAL == style->reg.mode) {
        if (val > top_y) {
            /** limit top */
            ret_val = top_y;
        }
        else if (val < bottom_y) {
            ret_val = bottom_y;
        }
    }
    else if (GT_ROLLER_MODE_INFINITE == style->reg.mode) {
        top_y = -style->scroll_vertical_length / GT_ROLLER_INFINITE_PAGES_COUNT;
        bottom_y = -style->scroll_vertical_length - top_y;

        if (val > top_y) {
            /** limit top, reset to bottom */
            ret_val -= _get_infinite_scroll_increment(style->scroll_vertical_length);
        }
        else if (val < bottom_y) {
            ret_val += _get_infinite_scroll_increment(style->scroll_vertical_length);
        }
    }
    return ret_val;
}

static void _scroll_text_handler(gt_obj_st * obj) {
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    if (NULL == style->text) {
        return;
    }
    obj->process_attr.scroll.y = _normal_limit_y(obj, gt_obj_scroll_get_y(obj));
    gt_obj_set_pos(style->text, obj->area.x, obj->area.y + obj->process_attr.scroll.y + (style->line_space >> 1));
}

static void _scroll_snap(gt_obj_st * obj) {
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    gt_size_t scroll_y = gt_obj_scroll_get_y(obj);

    if (GT_ROLLER_MODE_INFINITE == style->reg.mode) {
        scroll_y = _normal_limit_y(obj, scroll_y);
    }

    gt_size_t target_y = scroll_y;
    gt_size_t item_height = gt_label_get_font_size(style->text) + gt_label_get_space_y(style->text);

    if (gt_obj_is_scroll_up(obj)) {
        if (scroll_y < 0) {
            target_y -= (scroll_y % item_height) + item_height;
        } else {
            target_y -= scroll_y % item_height;
        }
    }
    else if (gt_obj_is_scroll_down(obj)) {
        if (scroll_y < 0) {
            target_y -= scroll_y % item_height;
        } else {
            target_y -= (scroll_y % item_height) - item_height;
        }
    }
    if (gt_abs(target_y - scroll_y) >= item_height) {
        /** Do not scroll by anim when variation over item height */
        return;
    }
    else if (target_y == scroll_y) {
        /** Do not scroll by anim when target y equal to current scroll value */
        return;
    }
    target_y = _normal_limit_y(obj, target_y);
    gt_obj_scroll_to_y(obj, target_y - scroll_y, GT_ANIM_ON);
}

static void _scroll_anim_ready_handler(struct gt_anim_s * anim) {
    _gt_roller_st * style = (_gt_roller_st * )anim->target->style;
    if (_SCROLL_SNAP_DONE == style->reg.state) {
        return ;
    }
    style->reg.state = _SCROLL_SNAP_READY;
    _scroll_snap(anim->target);
    style->anim_snap = NULL;
}

static void _scroll_exec_cb(gt_obj_st * obj, int32_t y) { }

static void _notify_update_value_cb(gt_obj_st * obj, int32_t val) { }

static void _notify_update_value_ready_cb(struct gt_anim_s * anim) {
    gt_event_send(anim->target, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

static void _select_target_item(gt_obj_st * obj) {
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    uint16_t item_height = style->text->area.h / style->count_total;
    uint16_t item_offset = gt_abs(obj->process_attr.scroll.y - (style->count_of_show >> 1) * item_height);

    style->selected = item_offset / item_height;

    gt_anim_st anim;
    gt_anim_init(&anim);
    gt_anim_set_target(&anim, obj);
    gt_anim_set_time(&anim, 0);
    gt_anim_set_value(&anim, 0, 100);
    gt_anim_set_time_delay_start(&anim, 10);
    gt_anim_set_exec_cb(&anim, _notify_update_value_cb);
    gt_anim_set_ready_cb(&anim, _notify_update_value_ready_cb);
    gt_anim_start(&anim);
}

static void _ready_scroll_snap_anim(struct gt_obj_s * obj) {
    _gt_roller_st * style = (_gt_roller_st * )obj->style;

    if (_SCROLL_SNAP_DONE == style->reg.state) {
        style->reg.state = _SCROLL_SNAP_NONE;
        return ;
    }
    if (_SCROLL_SNAP_READY == style->reg.state) {
        style->reg.state = _SCROLL_SNAP_DONE;
        _select_target_item(obj);
        return ;
    }

    if (NULL == style->anim_snap) {
        gt_anim_st anim;
        gt_anim_init(&anim);
        gt_anim_set_target(&anim, obj);
        gt_anim_set_time(&anim, 0);
        gt_anim_set_value(&anim, 0, 100);
        gt_anim_set_time_delay_start(&anim, 100);
        gt_anim_set_exec_cb(&anim, _scroll_exec_cb);
        gt_anim_set_ready_cb(&anim, _scroll_anim_ready_handler);
        style->anim_snap = gt_anim_start(&anim);
        return ;
    }

    gt_anim_set_time_delay_start(style->anim_snap, 100);
    gt_anim_restart(style->anim_snap);
}

static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);

    if (GT_EVENT_TYPE_INPUT_SCROLL == code) {
        _scroll_text_handler(obj);
    }
    else if (GT_EVENT_TYPE_INPUT_SCROLL_END == code) {
        _ready_scroll_snap_anim(obj);
    }
    else if (GT_EVENT_TYPE_INPUT_SCROLL_UP == code || GT_EVENT_TYPE_INPUT_SCROLL_LEFT == code) {
        gt_roller_go_prev(obj);
    }
    else if (GT_EVENT_TYPE_INPUT_SCROLL_DOWN == code || GT_EVENT_TYPE_INPUT_SCROLL_RIGHT == code) {
        gt_roller_go_next(obj);
    }
}

static inline bool _set_mode(_gt_roller_st * style, gt_roller_mode_em mode) {
    if (NULL == style) {
        return false;
    }
    if (mode >= GT_ROLLER_MODE_MAX_COUNT) {
        return false;
    }
    style->reg.mode = mode;
    return true;
}

/**
 * @brief Resize roller height, include label widget
 *
 * @param obj
 */
static void _resize_display_area_height(gt_obj_st * obj) {
    _gt_roller_st * style = (_gt_roller_st * )obj->style;

    if (NULL == style->text) {
        return;
    }

    if (0 == gt_label_get_font_size(style->text)) {
        return;
    }

    gt_obj_set_size(obj, gt_obj_get_w(obj),
        (gt_label_get_font_size(style->text) + gt_label_get_space_y(style->text)) * style->count_of_show);

    gt_obj_set_size(style->text, obj->area.w,
        style->count_total * (gt_label_get_font_size(style->text) + gt_label_get_space_y(style->text)));
}

static void _reset_scroll_size(gt_obj_st * obj) {
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    _calc_scroll_vertical_length(obj, gt_label_get_text(style->text));

    _resize_display_area_height(obj);
    /** set position and scroll default position, top margin half of space y size */

    obj->process_attr.scroll.y = (style->count_of_show >> 1) * (obj->area.h / style->count_of_show);
    _scroll_text_handler(obj);
}

static gt_obj_st * _create_text(gt_obj_st * obj) {
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    if (style->text) {
        return style->text;
    }

    style->text = gt_label_create(obj);
    gt_label_set_font_align(style->text, GT_ALIGN_CENTER);
    return style->text;
}

/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_roller_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return NULL;
    }
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    if (NULL == style) {
        gt_obj_destroy(obj);
        return NULL;
    }
    gt_memset(style, 0, sizeof(_gt_roller_st));
    style->count_of_show = 3;
    style->line_space = 10;

    obj->area.w = 80;
    obj->area.h = 20 * style->count_of_show;

    return obj;
}

void gt_roller_set_display_item_count(gt_obj_st * obj, uint8_t count)
{
    if (NULL == obj) {
        return;
    }
    if (GT_TYPE_ROLLER != gt_obj_class_get_type(obj)) {
        return;
    }
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    if (NULL == style) {
        return;
    }
    if (0 == count) {
        return;
    }
    if (count < 4) {
        style->count_of_show = 3;
    } else {
        style->count_of_show = count - ((0 == count % 2) ? 1 : 0);
    }

    _resize_display_area_height(obj);
}

void gt_roller_set_options(gt_obj_st * obj, char * options, gt_roller_mode_em mode)
{
    if (NULL == obj) {
        return;
    }
    if (GT_TYPE_ROLLER != gt_obj_class_get_type(obj)) {
        return;
    }
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    if (NULL == style) {
        return;
    }
    if (NULL == options) {
        return;
    }
    if (false == _set_mode(style, mode)) {
        return;
    }

    if (GT_ROLLER_MODE_NORMAL == mode) {
        _create_text(obj);
        gt_label_set_text(style->text, options);
    }
    else if (GT_ROLLER_MODE_INFINITE == mode) {
        _create_text(obj);
        gt_size_t len = strlen(options) + 1;
        char * infinite_options = gt_mem_malloc(len * GT_ROLLER_INFINITE_PAGES_COUNT);
        if (NULL == infinite_options) {
            return;
        }
        for (uint16_t i = 0; i < GT_ROLLER_INFINITE_PAGES_COUNT; i++) {
            gt_memcpy(&infinite_options[i * len], options, len);
            infinite_options[(i + 1) * len - 1] = '\n';
        }
        infinite_options[GT_ROLLER_INFINITE_PAGES_COUNT * len - 1] = '\0';
        gt_label_set_text(style->text, infinite_options);

        if (infinite_options) {
            gt_mem_free(infinite_options);
            infinite_options = NULL;
        }
    }
    else {
        /** other unknown mode */
        return;
    }
    gt_label_set_space(style->text, 0, style->line_space);

    gt_obj_set_fixed(style->text, true);
    _reset_scroll_size(obj);
    gt_obj_set_inside(style->text, true);
    gt_obj_set_scroll_dir(obj, GT_SCROLL_VERTICAL);
}

gt_res_t gt_roller_set_selected(gt_obj_st * obj, gt_size_t index)
{
    if (NULL == obj) {
        return GT_RES_FAIL;
    }
    if (GT_TYPE_ROLLER != gt_obj_class_get_type(obj)) {
        return GT_RES_FAIL;
    }
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    if (_SCROLL_SNAP_READY == style->reg.state) {
        /** Prevent intermediate animation events from being removed */
        return GT_RES_INV;
    }
    if (NULL == style) {
        return GT_RES_FAIL;
    }
    gt_size_t diff_count = index - style->selected;

    if (GT_ROLLER_MODE_NORMAL == style->reg.mode) {
        if (index > style->count_total) {
            return GT_RES_FAIL;
        }
    } else if (GT_ROLLER_MODE_INFINITE == style->reg.mode) {
        if (index > style->count_total / GT_ROLLER_INFINITE_PAGES_COUNT) {
            return GT_RES_FAIL;
        }
        diff_count = index - _get_infinite_real_selected_index(style->selected, style->count_total);
    }
    if (0 == diff_count) {
        /** avoid state can not change to done */
        return GT_RES_OK;
    }
    uint16_t item_height = style->text->area.h / style->count_total;

    obj->process_attr.scroll.y -= item_height * diff_count;
    _scroll_text_handler(obj);

    return GT_RES_OK;
}

gt_res_t gt_roller_set_selected_anim(gt_obj_st * obj, gt_size_t index)
{
    if (NULL == obj) {
        return GT_RES_FAIL;
    }
    if (GT_TYPE_ROLLER != gt_obj_class_get_type(obj)) {
        return GT_RES_FAIL;
    }
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    if (_SCROLL_SNAP_READY == style->reg.state) {
        /** Prevent intermediate animation events from being removed */
        return GT_RES_INV;
    }
    if (NULL == style) {
        return GT_RES_FAIL;
    }
    gt_size_t diff_count = index - style->selected;

    if (GT_ROLLER_MODE_NORMAL == style->reg.mode) {
        if (index > style->count_total) {
            return GT_RES_FAIL;
        }
    } else if (GT_ROLLER_MODE_INFINITE == style->reg.mode) {
        if (index > style->count_total / GT_ROLLER_INFINITE_PAGES_COUNT) {
            return GT_RES_FAIL;
        }
        diff_count = index - _get_infinite_real_selected_index(style->selected, style->count_total);
    }
    if (0 == diff_count) {
        /** avoid state can not change to done */
        return GT_RES_OK;
    }
    uint16_t item_height = style->text->area.h / style->count_total;

    style->reg.state = _SCROLL_SNAP_READY;
    gt_obj_scroll_to_y(obj, -item_height * diff_count, GT_ANIM_ON);

    return GT_RES_OK;
}

gt_size_t gt_roller_get_selected(gt_obj_st * obj)
{
    if (NULL == obj) {
        return -1;
    }
    if (GT_TYPE_ROLLER != gt_obj_class_get_type(obj)) {
        return -1;
    }
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    if (NULL == style) {
        return -1;
    }
    if (GT_ROLLER_MODE_INFINITE == style->reg.mode) {
        return _get_infinite_real_selected_index(style->selected, style->count_total);
    }
    return style->selected;
}

bool gt_roller_get_selected_text(gt_obj_st * obj, char * result)
{
    if (NULL == obj) {
        return false;
    }
    if (NULL == result) {
        return false;
    }
    if (GT_TYPE_ROLLER != gt_obj_class_get_type(obj)) {
        return false;
    }
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    if (NULL == style) {
        return false;
    }
    char * text = gt_label_get_text(style->text);
    uint16_t offset = 0;
    uint16_t i = 0, len = strlen(text), count = 0, remark_idx = 0;

    for (i = 0; i < len; i++) {
        if ('\n' == text[i]) {
            ++count;
            continue;
        }

        if (count == style->selected) {
            result[offset++] = text[i];
        }
        else if (count > style->selected) {
            break;
        }
    }
    result[offset] = '\0';

    return offset ? true : false;
}

uint8_t gt_roller_go_prev(gt_obj_st * obj)
{
	gt_roller_set_selected(obj, gt_roller_get_selected(obj) - 1);
}

uint8_t gt_roller_go_next(gt_obj_st * obj)
{
    gt_roller_set_selected(obj, gt_roller_get_selected(obj) + 1);
}

uint8_t gt_roller_get_total_count(gt_obj_st * obj)
{
    if (NULL == obj) {
        return 0;
    }
    if (GT_TYPE_ROLLER != gt_obj_class_get_type(obj)) {
        return 0;
    }
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    if (NULL == style) {
        return 0;
    }
    if (GT_ROLLER_MODE_INFINITE == style->reg.mode) {
        return style->count_total / GT_ROLLER_INFINITE_PAGES_COUNT;
    }
    return style->count_total;
}

void gt_roller_set_line_space(gt_obj_st * obj, uint8_t space)
{
    if (NULL == obj) {
        return ;
    }
    if (GT_TYPE_ROLLER != gt_obj_class_get_type(obj)) {
        return ;
    }
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    if (NULL == style) {
        return ;
    }
    style->line_space = space;

    _create_text(obj);
    gt_label_set_space(style->text, 0, style->line_space);
    _reset_scroll_size(obj);
}

void gt_roller_set_font_color(gt_obj_st * obj, gt_color_t color)
{
    if (NULL == obj) {
        return ;
    }
    if (GT_TYPE_ROLLER != gt_obj_class_get_type(obj)) {
        return ;
    }
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    if (NULL == style) {
        return ;
    }
    _create_text(obj);
    gt_label_set_font_color(style->text, color);
}

void gt_roller_set_font_size(gt_obj_st * obj, uint8_t size)
{
    if (NULL == obj) {
        return ;
    }
    if (GT_TYPE_ROLLER != gt_obj_class_get_type(obj)) {
        return ;
    }
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    if (NULL == style) {
        return ;
    }
    _create_text(obj);
    gt_label_set_font_size(style->text, size);
}

void gt_roller_set_font_gray(gt_obj_st * obj, uint8_t gray)
{
    if (NULL == obj) {
        return ;
    }
    if (GT_TYPE_ROLLER != gt_obj_class_get_type(obj)) {
        return ;
    }
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    if (NULL == style) {
        return ;
    }
    _create_text(obj);
    gt_label_set_font_gray(style->text, gray);
}

void gt_roller_set_font_align(gt_obj_st * obj, uint8_t align)
{
    if (NULL == obj) {
        return ;
    }
    if (GT_TYPE_ROLLER != gt_obj_class_get_type(obj)) {
        return ;
    }
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    if (NULL == style) {
        return ;
    }
    _create_text(obj);
    gt_label_set_font_align(style->text, align);
}

void gt_roller_set_font_family_cn(gt_obj_st * obj, gt_family_t family)
{
    if (NULL == obj) {
        return ;
    }
    if (GT_TYPE_ROLLER != gt_obj_class_get_type(obj)) {
        return ;
    }
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    if (NULL == style) {
        return ;
    }
    _create_text(obj);
    gt_label_set_font_family_cn(style->text, family);
}

void gt_roller_set_font_family_en(gt_obj_st * obj, gt_family_t family)
{
    if (NULL == obj) {
        return ;
    }
    if (GT_TYPE_ROLLER != gt_obj_class_get_type(obj)) {
        return ;
    }
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    if (NULL == style) {
        return ;
    }
    _create_text(obj);
    gt_label_set_font_family_en(style->text, family);
}

void gt_roller_set_font_family_fl(gt_obj_st * obj, gt_family_t family)
{
    if (NULL == obj) {
        return ;
    }
    if (GT_TYPE_ROLLER != gt_obj_class_get_type(obj)) {
        return ;
    }
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    if (NULL == style) {
        return ;
    }
    _create_text(obj);
    gt_label_set_font_family_fl(style->text, family);
}

void gt_roller_set_font_family_numb(gt_obj_st * obj, gt_family_t family)
{
    if (NULL == obj) {
        return ;
    }
    if (GT_TYPE_ROLLER != gt_obj_class_get_type(obj)) {
        return ;
    }
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    if (NULL == style) {
        return ;
    }
    _create_text(obj);
    gt_label_set_font_family_numb(style->text, family);
}

void gt_roller_set_font_thick_en(gt_obj_st * obj, uint8_t thick)
{
    if (NULL == obj) {
        return ;
    }
    if (GT_TYPE_ROLLER != gt_obj_class_get_type(obj)) {
        return ;
    }
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    if (NULL == style) {
        return ;
    }
    _create_text(obj);
    gt_label_set_font_thick_en(style->text, thick);
}

void gt_roller_set_font_thick_cn(gt_obj_st * obj, uint8_t thick)
{
    if (NULL == obj) {
        return ;
    }
    if (GT_TYPE_ROLLER != gt_obj_class_get_type(obj)) {
        return ;
    }
    _gt_roller_st * style = (_gt_roller_st * )obj->style;
    if (NULL == style) {
        return ;
    }
    _create_text(obj);
    gt_label_set_font_thick_cn(style->text, thick);
}


#endif  /** GT_CFG_ENABLE_ROLLER */
/* end ------------------------------------------------------------------*/
