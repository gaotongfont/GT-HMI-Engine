/**
 * @file gt_btn.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-11 15:03:35
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_btn.h"
#include "../core/gt_theme.h"

#if GT_CFG_ENABLE_BTN
#include "../core/gt_mem.h"
#include "../hal/gt_hal_disp.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_mask.h"
#include "../core/gt_obj_pos.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../core/gt_event.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_BTN
#define MY_CLASS    &gt_btn_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_btn_reg_s {
    uint8_t fill: 1;
    uint8_t state_enabled : 1;  /** 1[default]: change state */
    uint8_t corner_tl : 1;      /** 1[default]: top-left corner rounded */
    uint8_t corner_tr : 1;      /** 1[default]: top-right corner rounded */
    uint8_t corner_bl : 1;      /** 1[default]: bottom-left corner rounded */
    uint8_t corner_br : 1;      /** 1[default]: bottom-right corner rounded */
    uint8_t reserved: 2;
}_gt_btn_reg_st;

/** Invalid button id, means the button is not mapped to any physical button */
#define GT_BTN_ID_INVALID    0xFF

typedef struct _gt_btn_s {
    gt_obj_st base;
    _gt_vector_st * contents;

    gt_color_t color_pressed;
    gt_color_t color_border;

    _auto_scroll_st * auto_scroll;
    gt_color_t font_color;
    gt_color_t font_color_pressed;
    gt_font_info_st font_info;

    uint8_t border_width;

    _gt_btn_reg_st reg;
    uint16_t indent;
    uint8_t font_align;     //@ref gt_align_et
    uint8_t space_x;
    uint8_t space_y;
    uint8_t roll_space;
    uint8_t button_id;      /** physical button id, GT_BTN_ID_INVALID means not mapped */
    _gt_text_style_st text_style;
}_gt_btn_st;


/* static variables -----------------------------------------------------*/
static void _btn_init_cb(gt_obj_st * obj);
static void _btn_deinit_cb(gt_obj_st * obj);
static void _btn_event_cb(struct gt_obj_s * obj, gt_event_st * e);
static void _btn_apply_theme_cb(gt_obj_st * obj);

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_btn_class = {
    ._init_cb         = _btn_init_cb,
    ._deinit_cb       = _btn_deinit_cb,
    ._event_cb        = _btn_event_cb,
    ._apply_theme_cb  = _btn_apply_theme_cb,
    .type             = OBJ_TYPE,
    .size_style       = sizeof(_gt_btn_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static _gt_draw_font_res_st GT_ATTRIBUTE_RAM_TEXT
_draw_or_calc_dislpay_content(gt_obj_st * obj, bool is_only_calc)
{
    _gt_btn_st * style = (_gt_btn_st * )obj;
    _gt_draw_font_res_st ret = {0} ;
    char * text = (char * )_gt_vector_get_item(style->contents, _gt_vector_get_index(style->contents));
    uint32_t len = (uint32_t)strlen(text);
    if (!text) { GT_LOGW(GT_LOG_TAG_CHECK, "Failed: [%s] value is NULL!", text); return ret; }

    gt_font_st font = {
        .res        = NULL,
        .utf8       = text,
        .len        = len,
        .info       = style->font_info,
    };

    font.info.thick_en = style->font_info.thick_en == 0 ? style->font_info.size + 6: style->font_info.thick_en;
    font.info.thick_cn = style->font_info.thick_cn == 0 ? style->font_info.size + 6: style->font_info.thick_cn;

    gt_area_st area = gt_area_reduce(obj->area, style->border_width);
    if (style->text_style.single_line && style->auto_scroll)
    {
        area.x += style->roll_space;
        area.w -= style->roll_space << 1;
    }

    gt_area_st box_area = obj->area;
    _gt_text_style_st text_sty = style->text_style;
    text_sty.only_calc_display_info = is_only_calc ? true : false;

    // 2:font
    gt_attr_font_st font_attr = {
        .font           = &font,
        .space_x        = style->space_x,
        .space_y        = style->space_y,
        .font_color     = style->font_color,
        .align          = style->font_align,
        .opa            = obj->opa,
        .reg            = text_sty,
        .logical_area   = area,
        .indent         = style->indent,
    };

    if (gt_obj_get_state(obj) == GT_STATE_PRESSED) {
        font_attr.font_color = style->font_color_pressed;
    } else {
        font_attr.font_color = style->font_color;
    }

    if (style->text_style.single_line && style->auto_scroll) {
        font_attr.logical_area = style->auto_scroll->area;
        font_attr.logical_area.x += box_area.x;

        font_attr.logical_area.y += box_area.y;
        font_attr.logical_area.h = box_area.h;
    }
    else
    {
        if( style->font_align == GT_ALIGN_LEFT
        || style->font_align == GT_ALIGN_LEFT_MID
        || style->font_align == GT_ALIGN_LEFT_BOTTOM){
        font_attr.logical_area.x += 4;
        font_attr.logical_area.w -= 8;
        }
    }

    return draw_text(obj->draw_ctx, &font_attr, &area);
}

/**
 * @brief obj init btn widget call back
 *
 * @param obj
 */
static GT_ATTRIBUTE_RAM_TEXT void
_btn_init_cb(gt_obj_st * obj) {
    _gt_btn_st * style = (_gt_btn_st * )obj;
    gt_color_t fg_color = gt_obj_get_state(obj) == GT_STATE_PRESSED ? style->color_pressed : obj->bgcolor;
    char * text = (char * )_gt_vector_get_item(style->contents, _gt_vector_get_index(style->contents));
    uint32_t len = (uint32_t)strlen(text);

    // set default size
    if (obj->area.w == 0 || obj->area.h == 0) {
        obj->area.w = style->font_info.size * len + 32;
        obj->area.h = style->font_info.size + 16;
    }
    if (obj->radius > GT_MIN(obj->area.w, obj->area.h) >> 1) {
        obj->radius = GT_MIN(obj->area.w, obj->area.h) >> 1;
    }

    uint8_t cm = (style->reg.corner_tl ? GT_MASK_CORNER_TL : 0)
               | (style->reg.corner_tr ? GT_MASK_CORNER_TR : 0)
               | (style->reg.corner_bl ? GT_MASK_CORNER_BL : 0)
               | (style->reg.corner_br ? GT_MASK_CORNER_BR : 0);

    /* base shape */
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill   = style->reg.fill;
    rect_attr.radius        = cm ? obj->radius : 0;
    rect_attr.bg_opa        = obj->opa;
    rect_attr.border_width  = style->border_width;
    rect_attr.fg_color      = fg_color;
    rect_attr.bg_color      = fg_color;
    rect_attr.border_color  = style->color_border;
    rect_attr.corner_mask   = cm;

    // 1:base shape
    draw_bg(obj->draw_ctx, &rect_attr, &obj->area);

    gt_area_st area = gt_area_reduce(obj->area, style->border_width);
    gt_area_st box_area = obj->area;

    _gt_draw_font_res_st font_res = _draw_or_calc_dislpay_content(obj, 0);

    _roll_anim_first_restart_scroll(style->text_style, style->auto_scroll, box_area, font_res);
    // focus
    draw_focus(obj);
}

/**
 * @brief obj deinit call back
 *
 * @param obj
 */
static GT_ATTRIBUTE_RAM_TEXT void _btn_deinit_cb(gt_obj_st * obj) {
    _gt_btn_st * style_p = (_gt_btn_st * )obj;
    _roll_anim_free_auto_ctrl(&style_p->auto_scroll);

    if (NULL != style_p->contents) {
        _gt_vector_free(style_p->contents);
        style_p->contents = NULL;
    }
}

static GT_ATTRIBUTE_RAM_TEXT bool _turn_next_content(gt_obj_st * obj) {
    _gt_btn_st * style = (_gt_btn_st * )obj;
    if (NULL == style->contents) {
        return false;
    }

    _gt_draw_font_res_st font_res = _draw_or_calc_dislpay_content(obj, 1);

    if (font_res.size.x > obj->area.w && _gt_vector_get_count(style->contents) <= 1)
    {
        return true;
    }
    gt_btn_set_text(obj, (char * )_gt_vector_turn_next(style->contents));
    return true;
}

static GT_ATTRIBUTE_RAM_TEXT void _press_btn_handler(gt_obj_st * obj) {
    _gt_btn_st * style = (_gt_btn_st * )obj;
    if (false == style->reg.state_enabled) {
        return;
    }
    gt_obj_set_state(obj, GT_STATE_PRESSED);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

static GT_ATTRIBUTE_RAM_TEXT void
_reset_other_btn_state_except_current_btn(gt_obj_st * obj) {
    gt_obj_st * parent = obj->parent;
    /** reset group all btn state */
    for (uint16_t i = 0, cnt = parent->cnt_child; i < cnt; i++) {
        if (OBJ_TYPE != gt_obj_class_get_type(parent->child[i])) {
            continue;
        }
        if (GT_STATE_PRESSED != gt_obj_get_state(parent->child[i])) {
            continue;
        }
        gt_obj_set_state(parent->child[i], GT_STATE_NONE);
        gt_event_send(parent->child[i], GT_EVENT_TYPE_DRAW_START, NULL);
    }
    gt_obj_set_state(obj, GT_STATE_PRESSED);
}


static GT_ATTRIBUTE_RAM_TEXT inline void
_released_btn_handler(gt_obj_st * obj) {
    _gt_btn_st * style = (_gt_btn_st * )obj;
    if (false == style->reg.state_enabled) {
        return;
    }
    if (GT_TYPE_GROUP == gt_obj_class_get_type(obj->parent)) {
        _reset_other_btn_state_except_current_btn(obj);
    } else {
        gt_obj_set_state(obj, GT_STATE_NONE);
    }
    if (!_turn_next_content(obj)) {
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
    }
}

static GT_ATTRIBUTE_RAM_TEXT void
_reset_btn_state_when_group_has_pressed_btn_before(gt_obj_st *obj) {
    gt_obj_st * parent = obj->parent;
    bool has_pressed = false;
    for (uint16_t i = 0, cnt = parent->cnt_child; i < cnt; i++) {
        if (OBJ_TYPE != gt_obj_class_get_type(parent->child[i])) {
            continue;
        }
        if (parent->child[i] == obj) {
            continue;
        }
        if (GT_STATE_PRESSED == gt_obj_get_state(parent->child[i])) {
            has_pressed = true;
            break;
        }
    }
    if (has_pressed) { gt_obj_set_state(obj, GT_STATE_NONE); }
}

static GT_ATTRIBUTE_RAM_TEXT inline void
_lost_focus_btn_handler(gt_obj_st *obj) {
    _gt_btn_st * style = (_gt_btn_st * )obj;
    if (false == style->reg.state_enabled) {
        return;
    }
    if (GT_TYPE_GROUP != gt_obj_class_get_type(obj->parent)) {
        gt_obj_set_state(obj, GT_STATE_NONE);
    } else {
        _reset_btn_state_when_group_has_pressed_btn_before(obj);
    }
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}


/**
 * @brief obj event handler call back
 *
 * @param obj
 * @param e event
 */
static void _btn_event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code_val = gt_event_get_code(e);

    switch(code_val) {
        case GT_EVENT_TYPE_DRAW_START:
        case GT_EVENT_TYPE_UPDATE_STYLE:
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_PRESSED:
        case GT_EVENT_TYPE_INPUT_PRESSING:
            _press_btn_handler(obj);
            break;

        case GT_EVENT_TYPE_INPUT_RELEASED: /* click event finish */
            _released_btn_handler(obj);
            break;

        case GT_EVENT_TYPE_INPUT_PRESS_LOST:
            _lost_focus_btn_handler(obj);
            break;

        case GT_EVENT_TYPE_UPDATE_VALUE:
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
        break;

        default:
            break;
    }
}

static GT_ATTRIBUTE_RAM_TEXT bool _contents_free_cb(void * item) {
    if (NULL == item) {
        return false;
    }
    gt_mem_free(item);
    return true;
}

static GT_ATTRIBUTE_RAM_TEXT bool _contents_equal_cb(void * item, void * target) {
    return strcmp(item, target) ? false : true;
}

static void GT_ATTRIBUTE_RAM_TEXT
_update_btn_size(gt_obj_st * obj, uint16_t len) {
    _gt_btn_st * style = (_gt_btn_st * )obj;
    if (obj->area.w && obj->area.h) {
        return;
    }
    obj->area.h = style->font_info.size + 6;
    obj->area.w = style->font_info.size * len;
}

/* global functions / API interface -------------------------------------*/

static void _btn_apply_theme_cb(gt_obj_st * obj) {
    gt_theme_t * t = gt_theme_get();
    if (!t) { return; }
    _gt_btn_st * style = (_gt_btn_st *)obj;
    gt_theme_id_et id = gt_theme_get_id();
    if (id == GT_THEME_DARK) {
        gt_obj_set_bgcolor(obj, t->bg_card);
        style->color_pressed = t->bg_secondary;
        style->font_color    = t->text_primary;
    } else {
        gt_obj_set_bgcolor(obj, t->accent);
        style->color_pressed = t->accent_pressed;
        style->font_color    = t->text_primary;
    }
    obj->use_theme = 1;
}

gt_obj_st * gt_btn_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    obj->fixed = false;
    obj->radius = 4;
    _gt_btn_st * style = (_gt_btn_st * )obj;

    style->color_pressed    = gt_color_hex(0x0097e6);    //default color_selected
    obj->bgcolor            = gt_color_hex(0x00a8ff);    //default color_unselected
    style->color_border     = gt_color_hex(0x00a8ff);    //default color_border
    style->border_width     = 0;
    style->reg.fill         = 1;
    style->reg.state_enabled = 1;
    style->reg.corner_tl    = 1;
    style->reg.corner_tr    = 1;
    style->reg.corner_bl    = 1;
    style->reg.corner_br    = 1;
    style->font_color       = gt_color_white();        //default color_font
    style->font_color_pressed    = gt_color_gray();        //default color_font
    gt_font_info_init(&style->font_info);
    style->font_align = GT_ALIGN_CENTER_MID;
    style->space_x          = 0;
    style->space_y          = 0;
    style->roll_space       = 2;
    style->button_id        = GT_BTN_ID_INVALID;  /** default: not mapped to any physical button */

    style->contents = _gt_vector_create(_contents_free_cb, _contents_equal_cb);
    _roll_anim_auto_scroll_init(obj, style->auto_scroll);

    uint16_t len = strlen("btn");
    char * str = gt_mem_malloc(len);
    if (NULL == str) {
        return obj;
    }
    gt_memcpy(str, "btn", len);
    str[len] = '\0';

    if (false == _gt_vector_add_item(style->contents, (void * )str)) {
        gt_mem_free(str);
        str = NULL;
    }
    _update_btn_size(obj, len);

#if GT_BTN_DEFAULT_AUTO_SCROLL_MODE
    gt_btn_set_auto_scroll_single_line(obj, true);
    #if GT_BTN_DEFAULT_AUTO_SCROLL_TIME_MODE
    gt_btn_set_one_word_scroll_time(obj, 500);
    #endif //!GT_BTN_DEFAULT_AUTO_SCROLL_TIME_MODE
#endif //GT_BTN_DEFAULT_AUTO_SCROLL_MODE

    return obj;
}

void gt_btn_set_color_pressed(gt_obj_st * btn, gt_color_t color)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->color_pressed = color;
    btn->use_theme = 0;
    gt_event_send(btn, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btn_set_color_background(gt_obj_st * btn, gt_color_t color)
{
    gt_obj_set_bgcolor(btn, color);
    gt_event_send(btn, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btn_set_color_border(gt_obj_st *btn, gt_color_t color)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->color_border = color;
    gt_event_send(btn, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btn_set_border_width(gt_obj_st * btn, uint8_t width)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->border_width = width;
    gt_event_send(btn, GT_EVENT_TYPE_DRAW_START, NULL);
}

static gt_res_t _update_default_text(_gt_btn_st * style, char * text) {
    if (0 == _gt_vector_get_count(style->contents)) {
        if (false == _gt_vector_add_item(style->contents, (void * )text)) {
            return GT_RES_FAIL;
        }
    } else {
        if (false == _gt_vector_replace_item(style->contents, 0, (void * )text)) {
            return GT_RES_FAIL;
        }
    }
    return GT_RES_OK;
}

void gt_btn_set_text(gt_obj_st * btn, const char * fmt, ...)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    char buffer[8] = {0};
    va_list args;
    va_list args2;

    _gt_btn_st * style = (_gt_btn_st * )btn;
    _roll_anim_reset_auto_scroll_ctrl(style->auto_scroll);
    va_start(args, fmt);
    va_copy(args2, args);
    uint16_t size = (NULL == fmt) ? 0 : (vsnprintf(buffer, sizeof(buffer), fmt, args) + 1);
    va_end(args);
    if (!size) {
        goto free_lb;
    }
    char * text = gt_mem_malloc(size);
    if (NULL == text) {
        goto free_lb;
    }
    gt_memset(text, 0, size);
    va_start(args2, fmt);
    vsnprintf(text, size, fmt, args2);
    va_end(args2);

    if (GT_RES_FAIL == _update_default_text(style, text)) {
        goto text_lb;
    }

    #if GT_BTN_DEFAULT_AUTO_SCROLL_TIME_MODE
    if (style->text_style.single_line)
    {
        gt_btn_set_one_word_scroll_time(btn, 500);
    }
    #endif //!GT_BTN_DEFAULT_AUTO_SCROLL_TIME_MODE

    _update_btn_size(btn, size);
    gt_event_send(btn, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
    return ;

text_lb:
    gt_mem_free(text);
    text = NULL;
free_lb:
    va_end(args2);
}

void gt_btn_set_text_by_len(gt_obj_st * btn, const char * text, uint16_t len)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    _roll_anim_reset_auto_scroll_ctrl(style->auto_scroll);
    char * tmp_str = gt_mem_malloc(len + 1);
    if (NULL == tmp_str) {
        goto err_lb;
    }
    gt_memcpy(tmp_str, text, len);
    tmp_str[len] = '\0';
    if (GT_RES_FAIL == _update_default_text(style, tmp_str)) {
        goto err_lb;
    }

    #if GT_BTN_DEFAULT_AUTO_SCROLL_TIME_MODE
    if (style->text_style.single_line)
    {
        gt_btn_set_one_word_scroll_time(btn, 500);
    }
    #endif //!GT_BTN_DEFAULT_AUTO_SCROLL_TIME_MODE

    _update_btn_size(btn, len);
    gt_event_send(btn, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
    return;

err_lb:
    gt_mem_free(tmp_str);
    tmp_str = NULL;
}

char * gt_btn_get_text(gt_obj_st * btn)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return NULL;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    return (char * )_gt_vector_get_item(style->contents, _gt_vector_get_index(style->contents));
}

void gt_btn_set_font_color(gt_obj_st * btn, gt_color_t color)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->font_color = color;
    btn->use_theme = 0;
    gt_event_send(btn, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btn_set_font_color_pressed(gt_obj_st * btn, gt_color_t color)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->font_color_pressed = color;
    gt_event_send(btn, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btn_set_font_size(gt_obj_st * btn, uint8_t size)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->font_info.size = size;
    _roll_anim_reset_auto_scroll_ctrl(style->auto_scroll);
}

void gt_btn_set_font_gray(gt_obj_st * btn, uint8_t gray)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->font_info.gray = gray;
}

void gt_btn_set_font_scale_size(gt_obj_st * btn, uint8_t scale_size)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->font_info.scale_size = scale_size;
    gt_event_send(btn, GT_EVENT_TYPE_UPDATE_STYLE, NULL);
}

void gt_btn_set_font_align(gt_obj_st * btn, gt_align_et align)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->font_align = align;
}
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
void gt_btn_set_font_family_cn(gt_obj_st * btn, gt_family_t font_family_cn)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->font_info.style_cn = font_family_cn;
}

void gt_btn_set_font_family_en(gt_obj_st * btn, gt_family_t font_family_en)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->font_info.style_en = font_family_en;
}

void gt_btn_set_font_family_fl(gt_obj_st * btn, gt_family_t font_family_fl)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->font_info.style_fl = font_family_fl;
}

void gt_btn_set_font_family_numb(gt_obj_st * btn, gt_family_t font_family_numb)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->font_info.style_numb = font_family_numb;
}
#else
void gt_btn_set_font_family(gt_obj_st * btn, gt_family_t font_family)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    gt_font_set_family(&style->font_info, font_family);
}

void gt_btn_set_font_cjk(gt_obj_st* btn, gt_font_cjk_et cjk)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->font_info.cjk = cjk;
}
#endif

void gt_btn_set_indent(gt_obj_st * btn, uint16_t indent)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->indent = indent;
}

void gt_btn_set_radius(gt_obj_st * btn, gt_radius_t radius)
{
    gt_obj_set_radius(btn, radius);
}

bool gt_btn_add_state_content(gt_obj_st * btn, const char * str)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return false;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    uint16_t len = strlen(str);
    if (!str || 0 == len) {
        return false;
    }

    /** set first item */
    char * cur_str = gt_mem_malloc(len + 1);
    GT_CHECK_BACK_VAL(cur_str, false);
    gt_memcpy(cur_str, str, len);
    cur_str[len] = '\0';
    if (false == _gt_vector_add_item(style->contents, (void * )cur_str)) {
        gt_mem_free(cur_str);
        cur_str = NULL;
        return false;
    }
    return true;
}

bool gt_btn_remove_state_content(gt_obj_st * btn, const char * str)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return false;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    if (!str || !strlen(str)) {
        return false;
    }
    return _gt_vector_remove_item(style->contents, (void * )str);
}

bool gt_btn_clear_all_state_content(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return false;
    }
    _gt_btn_st * style = (_gt_btn_st * )obj;
    return _gt_vector_clear_all_items(style->contents);
}

int16_t gt_btn_get_state_content_index(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return -1;
    }
    _gt_btn_st * style = (_gt_btn_st * )obj;
    return _gt_vector_get_index(style->contents);
}

void gt_btn_set_font_thick_en(gt_obj_st * btn, uint8_t thick)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->font_info.thick_en = thick;
}

void gt_btn_set_font_thick_cn(gt_obj_st * btn, uint8_t thick)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->font_info.thick_cn = thick;
}

void gt_btn_set_font_encoding(gt_obj_st * btn, gt_encoding_et encoding)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->font_info.encoding = encoding;
}

void gt_btn_set_font_style(gt_obj_st * btn, gt_font_style_et font_style)
{
    if(false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->font_info.style.all = font_style;
}

void gt_btn_set_space(gt_obj_st * btn, uint8_t space_x, uint8_t space_y)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->space_x = space_x;
    style->space_y = space_y;
    _roll_anim_reset_auto_scroll_ctrl(style->auto_scroll);
}

void gt_btn_set_selected(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _press_btn_handler(obj);
}

void gt_btn_set_state_enabled(gt_obj_st * obj, bool enabled)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_btn_st * style = (_gt_btn_st * )obj;
    if (false == enabled) {
        _released_btn_handler(obj);
    }
    style->reg.state_enabled = enabled ? true : false;
}

gt_split_line_ret_st gt_btn_get_font_split_line_numb(gt_obj_st* obj, gt_font_split_line_st* sp_line)
{
    gt_split_line_ret_st s_line_ret = {0};
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return s_line_ret;
    }

    _gt_btn_st * style = (_gt_btn_st * )obj;
    gt_font_split_line_st sp_line_tmp;
    if (sp_line) {
        sp_line_tmp.indent = sp_line->indent;
        sp_line_tmp.max_w = sp_line->max_w;
        sp_line_tmp.space = sp_line->space;
        sp_line_tmp.start_w = sp_line->start_w;
    }else{
        sp_line_tmp.indent = style->indent;
        sp_line_tmp.max_w = obj->area.w;
        sp_line_tmp.start_w = obj->area.w;
        sp_line_tmp.space = style->space_x;
    }

    sp_line_tmp.text = (char * )_gt_vector_get_item(style->contents, _gt_vector_get_index(style->contents));
    if(!sp_line_tmp.text) {
        return s_line_ret;
    }
    sp_line_tmp.len = strlen(sp_line_tmp.text);

    s_line_ret = gt_font_split_line_numb(&style->font_info, &sp_line_tmp);
    return s_line_ret;
}

void gt_btn_append_text(gt_obj_st * obj, const char * str)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )obj;

    char* cur_str = (char * )_gt_vector_get_item(style->contents, _gt_vector_get_index(style->contents));
    if(cur_str) {
        gt_btn_set_text(obj, "%s%s", cur_str, str);
    }else{
        gt_btn_set_text_by_len(obj, str, strlen(str));
    }
}

void gt_btn_set_single_line(gt_obj_st * btn, bool is_single_line)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    if (is_single_line == style->text_style.single_line) {
        return ;
    }
    style->text_style.single_line = is_single_line;

    if (false == style->text_style.single_line) {
        _roll_anim_free_auto_ctrl(&style->auto_scroll);
    }

    gt_event_send(btn, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btn_set_omit_single_line(gt_obj_st * btn, bool is_omit)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->text_style.omit_line = is_omit;
    style->text_style.single_line = is_omit;

    _roll_anim_free_auto_ctrl(&style->auto_scroll);
    gt_event_send(btn, GT_EVENT_TYPE_DRAW_START, NULL);
}


void gt_btn_set_auto_scroll_single_line(gt_obj_st * obj, bool is_auto_scroll)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )obj;
    if (is_auto_scroll) {
        style->text_style.single_line = true;
        style->text_style.omit_line = false;
        style->auto_scroll = _roll_anim_create_auto_scroll_anim(obj, style->auto_scroll);
    } else {
        _roll_anim_free_auto_ctrl(&style->auto_scroll);
        style->text_style.omit_line = true;
    }

    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}


void gt_btn_set_auto_scroll_total_time(gt_obj_st * btn, uint32_t total_time_ms)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    if (NULL == style->auto_scroll) {
        return ;
    }

    style->auto_scroll->total_time = total_time_ms;
    if (NULL == style->auto_scroll->anim) {
        return ;
    }
    if (0 == style->auto_scroll->total_time) {
        style->auto_scroll->total_time = 2000;
        gt_anim_set_paused(style->auto_scroll->anim, true);
    }
    gt_anim_set_time(style->auto_scroll->anim, style->auto_scroll->total_time);

    if (gt_anim_is_paused(style->auto_scroll->anim)) {
        gt_event_send(btn, GT_EVENT_TYPE_DRAW_START, NULL);   /** restart scroll */
    }
}

void gt_btn_set_one_word_scroll_time(gt_obj_st * btn, uint32_t time_ms)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    if (NULL == style->auto_scroll) {
        return ;
    }

    char * text_p = (char * )_gt_vector_get_item(style->contents, _gt_vector_get_index(style->contents));
    if (style->font_info.size == 0)
    {
        GT_LOGE(GT_LOG_TAG_GUI, "btn font size error");
        return ;
    }

    _gt_draw_font_res_st font_res = _draw_or_calc_dislpay_content(btn, 1);
    uint16_t text_font_num = font_res.size.x / (style->font_info.size) + (font_res.size.x % (style->font_info.size) ? 1 : 0);
    gt_btn_set_auto_scroll_total_time(btn, text_font_num * time_ms);
}

bool gt_btn_is_single_line(gt_obj_st * btn)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return false;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;

    return style->text_style.single_line ? true : false;
}

bool gt_btn_is_omit_single_line(gt_obj_st * btn)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return false;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;

    return style->text_style.omit_line ? true : false;
}

bool gt_btn_is_auto_scroll_single_line(gt_obj_st * btn)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return false;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;

    if (true == style->text_style.omit_line) {
        return false;
    }
    else if (false == style->text_style.single_line) {
        return false;
    }
    if (NULL == style->auto_scroll) {
        return false;
    }
    if (NULL == style->auto_scroll->anim) {
        return false;
    }
    return true;
}

void gt_btn_set_roll_space(gt_obj_st * btn, uint8_t roll_space)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }

    _gt_btn_st * style = (_gt_btn_st * )btn;

    style->roll_space = roll_space;
}

uint8_t gt_btn_get_roll_space(gt_obj_st * btn)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE))
    {
        return 0;
    }

    _gt_btn_st * style = (_gt_btn_st * )btn;

    return style->roll_space;
}

gt_res_t gt_btn_scroll_pause(gt_obj_st * btn, bool paused)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return GT_RES_INV;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    if (NULL == style->auto_scroll) {
        return GT_RES_INV;
    }
    if (NULL == style->auto_scroll->anim) {
        return GT_RES_INV;
    }
    _roll_anim_paused(style->auto_scroll, paused);
}

void gt_btn_set_corner_radius(gt_obj_st * btn, bool tl, bool tr, bool bl, bool br)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->reg.corner_tl = tl ? 1 : 0;
    style->reg.corner_tr = tr ? 1 : 0;
    style->reg.corner_bl = bl ? 1 : 0;
    style->reg.corner_br = br ? 1 : 0;
    gt_event_send(btn, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btn_set_button_id(gt_obj_st * btn, uint8_t button_id)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return;
    }
    _gt_btn_st * style = (_gt_btn_st *)btn;
    style->button_id = button_id;
}

uint8_t gt_btn_get_button_id(gt_obj_st * btn)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return GT_BTN_ID_INVALID;
    }
    _gt_btn_st * style = (_gt_btn_st *)btn;
    return style->button_id;
}

#endif  /** GT_CFG_ENABLE_BTN */
/* end ------------------------------------------------------------------*/
