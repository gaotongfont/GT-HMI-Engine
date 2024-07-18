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

#if GT_CFG_ENABLE_BTN
#include "../core/gt_mem.h"
#include "../hal/gt_hal_disp.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
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
    uint8_t reserved: 7;
}_gt_btn_reg_st;

typedef struct _gt_btn_s {
    gt_obj_st obj;
    _gt_vector_st * contents;

    gt_color_t color_pressed;
    gt_color_t font_color;
    gt_color_t font_color_pressed;
    gt_color_t color_border;

    gt_font_info_st font_info;

    uint8_t border_width;

    _gt_btn_reg_st reg;
    uint8_t font_align;     //@ref gt_align_et
    uint8_t space_x;
    uint8_t space_y;
}_gt_btn_st;


/* static variables -----------------------------------------------------*/
static void _btn_init_cb(gt_obj_st * obj);
static void _btn_deinit_cb(gt_obj_st * obj);
static void _btn_event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_btn_class = {
    ._init_cb      = _btn_init_cb,
    ._deinit_cb    = _btn_deinit_cb,
    ._event_cb     = _btn_event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_btn_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

/**
 * @brief obj init btn widget call back
 *
 * @param obj
 */
static void _btn_init_cb(gt_obj_st * obj) {
    _gt_btn_st * style = (_gt_btn_st * )obj;
    gt_radius_t radius;

    gt_color_t fg_color;
    char * text = (char * )_gt_vector_get_item(style->contents, _gt_vector_get_index(style->contents));
    uint32_t len = (uint32_t)strlen(text);
    GT_CHECK_BACK(text);

    gt_font_st font = {
        .res        = NULL,
        .utf8       = text,
        .len        = len,
        .info       = style->font_info,
    };

    font.info.thick_en = style->font_info.thick_en == 0 ? style->font_info.size + 6: style->font_info.thick_en;
    font.info.thick_cn = style->font_info.thick_cn == 0 ? style->font_info.size + 6: style->font_info.thick_cn;

    // set default size
    if (obj->area.w == 0 || obj->area.h == 0) {
        obj->area.w = style->font_info.size * len + 32;
        obj->area.h = style->font_info.size + 16;
    }
    if (obj->radius == 0) {
        radius = obj->area.h >> 2;
    } else {
        radius = obj->radius;
    }
    if (gt_obj_get_state(obj) == GT_STATE_PRESSED) {
        fg_color = style->color_pressed;
    } else {
        fg_color = obj->bgcolor;
    }

    /* base shape */
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill   = style->reg.fill;
    rect_attr.radius        = radius;
    rect_attr.bg_opa        = obj->opa;
    rect_attr.border_width  = style->border_width;
    rect_attr.fg_color      = fg_color;
    rect_attr.bg_color      = fg_color;
    rect_attr.border_color  = style->color_border;

    gt_area_st area = gt_area_reduce(obj->area, gt_obj_get_reduce(obj));
    // 1:base shape
    draw_bg(obj->draw_ctx, &rect_attr, &area);

    // 2:font
    gt_attr_font_st font_attr = {
        .font           = &font,
        .space_x        = style->space_x,
        .space_y        = style->space_y,
        .font_color     = style->font_color,
        .align          = style->font_align,
        .opa            = obj->opa,
        .logical_area   = area,
    };

    if (gt_obj_get_state(obj) == GT_STATE_PRESSED) {
        font_attr.font_color = style->font_color_pressed;
    } else {
        font_attr.font_color = style->font_color;
    }
    draw_text(obj->draw_ctx, &font_attr, &area);

    // focus
    draw_focus(obj, radius);
}

/**
 * @brief obj deinit call back
 *
 * @param obj
 */
static void _btn_deinit_cb(gt_obj_st * obj) {
    _gt_btn_st * style_p = (_gt_btn_st * )obj;

    if (NULL != style_p->contents) {
        _gt_vector_free(style_p->contents);
        style_p->contents = NULL;
    }
}

static bool _turn_next_content(gt_obj_st * obj) {
    _gt_btn_st * style = (_gt_btn_st * )obj;
    if (NULL == style->contents) {
        return false;
    }

    gt_btn_set_text(obj, (char * )_gt_vector_turn_next(style->contents));
    return true;
}

static void _press_btn_handler(gt_obj_st * obj) {
    if (GT_TYPE_GROUP == gt_obj_class_get_type(obj->parent)) {
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
    }
    gt_obj_set_state(obj, GT_STATE_PRESSED);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

static inline void _released_btn_handler(gt_obj_st * obj) {
    if (GT_TYPE_GROUP != gt_obj_class_get_type(obj->parent)) {
        gt_obj_set_state(obj, GT_STATE_NONE);
    }
    if (!_turn_next_content(obj)) {
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
    }
}

static inline void _lost_focus_btn_handler(gt_obj_st *obj) {
    if (GT_TYPE_GROUP != gt_obj_class_get_type(obj->parent)) {
        gt_obj_set_state(obj, GT_STATE_NONE);
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
    gt_event_type_et code = gt_event_get_code(e);

    switch(code) {
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

        default:
            break;
    }
}

static bool _contents_free_cb(void * item) {
    if (NULL == item) {
        return false;
    }
    gt_mem_free(item);
    return true;
}

static bool _contents_equal_cb(void * item, void * target) {
    return strcmp(item, target) ? false : true;
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief create a btn obj
 *
 * @param parent btn's parent element
 * @return gt_obj_st* btn obj
 */
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
    style->font_color       = gt_color_white();        //default color_font
    style->font_color_pressed    = gt_color_gray();        //default color_font
    gt_font_info_init(&style->font_info);
    style->font_align = GT_ALIGN_CENTER_MID;
    style->space_x          = 0;
    style->space_y          = 0;

    style->contents = _gt_vector_create(_contents_free_cb, _contents_equal_cb);

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
    return obj;
}

void gt_btn_set_color_pressed(gt_obj_st * btn, gt_color_t color)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->color_pressed = color;
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

void gt_btn_set_text(gt_obj_st * btn, const char * fmt, ...)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    char buffer[8] = {0};
    va_list args;
    va_list args2;

    _gt_btn_st * style = (_gt_btn_st * )btn;
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

    if (0 == _gt_vector_get_count(style->contents)) {
        if (false == _gt_vector_add_item(style->contents, (void * )text)) {
            goto text_lb;
        }
    } else {
        if (false == _gt_vector_replace_item(style->contents, 0, (void * )text)) {
            goto text_lb;
        }
    }

    gt_event_send(btn, GT_EVENT_TYPE_DRAW_START, NULL);

    return ;

text_lb:
    gt_mem_free(text);
    text = NULL;
free_lb:
    va_end(args2);
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
}

void gt_btn_set_font_gray(gt_obj_st * btn, uint8_t gray)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->font_info.gray = gray;
}

void gt_btn_set_font_align(gt_obj_st * btn, gt_align_et align)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->font_align = align;
}

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

void gt_btn_set_radius(gt_obj_st * btn, gt_radius_t radius)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    btn->radius = radius;
    gt_event_send(btn, GT_EVENT_TYPE_DRAW_START, NULL);
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

void gt_btn_set_space(gt_obj_st * btn, uint8_t space_x, uint8_t space_y)
{
    if (false == gt_obj_is_type(btn, OBJ_TYPE)) {
        return ;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn;
    style->space_x = space_x;
    style->space_y = space_y;
}

void gt_btn_set_selected(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _press_btn_handler(obj);
}

#endif  /** GT_CFG_ENABLE_BTN */
/* end ------------------------------------------------------------------*/
