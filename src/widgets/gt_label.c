/**
 * @file gt_label.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-11 15:03:35
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_label.h"

#if GT_CFG_ENABLE_LABEL
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../others/gt_anim.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_LAB
#define MY_CLASS    &gt_label_class

/* private typedef ------------------------------------------------------*/

typedef struct _auto_scroll_s {
    gt_anim_st * anim;
    gt_area_st area;        // single line auto scroll
    uint32_t total_time;    // scroll total time [ms]
}_auto_scroll_st;

typedef struct _gt_label_s {
    gt_obj_st obj;
    char * text;

    _auto_scroll_st * auto_scroll;
    gt_color_t  font_color;
    gt_font_info_st font_info;

    uint16_t indent;
    uint8_t font_align;     //@ref gt_align_et
    uint8_t space_x;
    uint8_t space_y;
    _gt_text_style_st text_style;
}_gt_label_st;


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_label_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_label_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

/**
 * @brief obj init label widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    _gt_label_st * style = (_gt_label_st * )obj;
    uint32_t len = strlen((char * )style->text);
    gt_font_st font = {
        .info       = style->font_info,
        .res        = NULL,
        .utf8       = (char * )style->text,
        .len        = len,
    };
    gt_font_info_update_font_thick(&font.info);

    gt_area_st box_area = gt_area_reduce(obj->area, gt_obj_get_reduce(obj));
    /*draw font*/
    gt_attr_font_st font_attr = {
        .font       = &font,
        .font_color = style->font_color,
        .space_x    = style->space_x,
        .space_y    = style->space_y,
        .align      = style->font_align,
        .opa        = obj->opa,
        .reg        = style->text_style,
        .logical_area = box_area,
        .indent     = style->indent,
    };
    if (style->text_style.single_line && style->auto_scroll) {
        font_attr.logical_area = style->auto_scroll->area;
        font_attr.logical_area.x += box_area.x;

        font_attr.logical_area.y += box_area.y;
        font_attr.logical_area.h = box_area.h;
    }

    _gt_draw_font_res_st font_res = draw_text(obj->draw_ctx, &font_attr, &box_area);
    if (style->text_style.single_line && style->auto_scroll) {
        /** first time to calc label widget need to scroll and begin to scroll */
        if (font_res.size.x > box_area.w && gt_anim_is_paused(style->auto_scroll->anim)) {
            style->auto_scroll->area.w = font_res.size.x;
            gt_anim_set_time(style->auto_scroll->anim, style->auto_scroll->total_time);
            gt_anim_set_value(style->auto_scroll->anim, 0, box_area.w - style->auto_scroll->area.w);
            gt_anim_restart(style->auto_scroll->anim);
        }
    }

    // focus
    draw_focus(obj , obj->radius);
}

static void _free_auto_scroll_st(_gt_label_st * style) {
    if (NULL == style->auto_scroll) {
        return ;
    }

    if (style->auto_scroll->anim) {
        gt_anim_del_by(style->auto_scroll->anim);
        style->auto_scroll->anim = NULL;
    }
    gt_mem_free(style->auto_scroll);
    style->auto_scroll = NULL;
}

/**
 * @brief obj deinit call back
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "label deinit");
    if (NULL == obj) {
        return ;
    }

    _gt_label_st * style_p = (_gt_label_st * )obj;
    _free_auto_scroll_st(style_p);

    if (style_p->text) {
        gt_mem_free(style_p->text);
        style_p->text = NULL;
    }
}

/**
 * @brief obj event handler call back
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code_val = gt_event_get_code(e);

    if (GT_EVENT_TYPE_DRAW_START == code_val) {
        gt_disp_invalid_area(obj);
    }
    else if (GT_EVENT_TYPE_UPDATE_VALUE == code_val) {
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
    }
}

static _auto_scroll_st * _reset_auto_scroll_st(_auto_scroll_st * as) {
    if (NULL == as) {
        return NULL;
    }
    if (as->anim) {
        gt_anim_restart(as->anim);
        if (false == gt_anim_is_paused(as->anim)) {
            gt_anim_set_paused(as->anim, true);
        }
        gt_anim_set_value(as->anim, 0, 0);
    }
    as->area.x = 0;
    as->area.y = 0;
    as->area.w = 0xffff;
    as->area.h = 0;

    return as;
}

static void _auto_scroll_init(_auto_scroll_st * const auto_scroll_p) {
    if (NULL == auto_scroll_p) {
        return ;
    }
    gt_memset(auto_scroll_p, 0, sizeof(_auto_scroll_st));
    auto_scroll_p->total_time = 2000;   /** 2s */
    auto_scroll_p->area.w = 0xffff;
}

static _auto_scroll_st * _create_auto_scroll_st(gt_obj_st * label) {
    _gt_label_st * style = (_gt_label_st * )label;
    if (NULL == style->auto_scroll) {
        style->auto_scroll = gt_mem_malloc(sizeof(_auto_scroll_st));
        GT_CHECK_BACK_VAL(style->auto_scroll, NULL);
        _auto_scroll_init(style->auto_scroll);
    }
    return _reset_auto_scroll_st(style->auto_scroll);
}

static void _auto_scroll_exec_cb(void * obj, int32_t value) {
    _gt_label_st * style = (_gt_label_st * )obj;
    if (NULL == style->auto_scroll) {
        return ;
    }
    style->auto_scroll->area.x = value;
    gt_event_send((gt_obj_st * )obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

static gt_anim_st * _create_auto_scroll_anim(gt_obj_st * label) {
    _gt_label_st * style = (_gt_label_st * )label;
    if (NULL == style->auto_scroll) {
        style->auto_scroll = _create_auto_scroll_st(label);
        GT_CHECK_BACK_VAL(style->auto_scroll, NULL);
    }
    if (style->auto_scroll->anim) {
        _reset_auto_scroll_st(style->auto_scroll);
        return style->auto_scroll->anim;
    }
    gt_anim_st anim;
    gt_anim_init(&anim);
    gt_anim_set_target(&anim, label);
    gt_anim_set_value(&anim, 0, 0);
    gt_anim_set_time_delay_start(&anim, 500);
    gt_anim_set_repeat_delay(&anim, 500);
    gt_anim_set_exec_cb(&anim, _auto_scroll_exec_cb);
    gt_anim_set_playback(&anim, true);
    gt_anim_set_repeat_count(&anim, GT_ANIM_REPEAT_INFINITE);
    style->auto_scroll->anim = gt_anim_start(&anim);
    gt_anim_set_paused(style->auto_scroll->anim, true);

    return style->auto_scroll->anim;
}

static void _update_label_size(gt_obj_st * obj, uint16_t len) {
    _gt_label_st * style = (_gt_label_st * )obj;
    if (obj->area.w && obj->area.h) {
        return;
    }
    obj->area.h = style->font_info.size + 6;
    obj->area.w = style->font_info.size * len;
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief create a label obj
 *
 * @param parent label's parent element
 * @return gt_obj_st* label obj
 */
gt_obj_st * gt_label_create(gt_obj_st * parent) {
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    _gt_label_st * style = (_gt_label_st * )obj;

    uint16_t len = sizeof("label");
    style->text = gt_mem_malloc(len);
    if (style->text) {
        gt_memcpy(style->text, "label\0", len);
    }

    style->font_color        = gt_color_hex(0x00);

    gt_font_info_init(&style->font_info);
    style->font_align           = GT_ALIGN_LEFT;
    style->space_x              = 0;
    style->space_y              = 0;

    _auto_scroll_init(style->auto_scroll);

    _update_label_size(obj, len);

    return obj;
}


void gt_label_set_text(gt_obj_st * label, const char * fmt, ...)
{
    char buffer[8] = {0};
    va_list args;
    va_list args2;

    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return;
    }
    if (NULL == fmt) {
        return;
    }

    _gt_label_st * style = (_gt_label_st * )label;
    _reset_auto_scroll_st(style->auto_scroll);
    va_start(args, fmt);
    va_copy(args2, args);
    uint16_t size = (NULL == fmt) ? 0 : (vsnprintf(buffer, sizeof(buffer), fmt, args) + 1);
    va_end(args);
    if (!size) {
        goto free_lb;
    }

    if (NULL == style->text) {
        style->text = gt_mem_malloc(size);
    } else if (size != strlen(style->text) + 1) {
        style->text = gt_mem_realloc(style->text, size);
    }
    if (NULL == style->text) {
        goto free_lb;
    }
    gt_memset(style->text, 0, size);
    va_start(args2, fmt);
    vsnprintf(style->text, size, fmt, args2);
    va_end(args2);

    _update_label_size(label, size);
    gt_event_send(label, GT_EVENT_TYPE_UPDATE_VALUE, NULL);

    return;

free_lb:
    va_end(args2);
}

void gt_label_set_text_by_len(gt_obj_st * label, const char * text, uint16_t len)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return;
    }
    if (NULL == text || 0 == len) {
        return;
    }

    _gt_label_st * style = (_gt_label_st * )label;
    char * ptr = (char * )text;
    char * dst = NULL;
    uint16_t i = 0;
    uint16_t real_len = strlen(text);

    if (real_len < len) {
        len = real_len;
    }

    _reset_auto_scroll_st(style->auto_scroll);

    if (NULL == style->text) {
        style->text = gt_mem_malloc(len + 1);
    } else if (len != strlen(style->text)) {
        style->text = gt_mem_realloc(style->text, len + 1);
    }
    if (NULL == style->text) {
        return;
    }

    dst = style->text;
    while (i < len) {
        *dst = *ptr;
        ++dst;
        ++ptr;
        ++i;
    }
    *dst = '\0';

    _update_label_size(label, len);
    gt_event_send(label, GT_EVENT_TYPE_UPDATE_VALUE, NULL);
}

char * gt_label_get_text(gt_obj_st * label)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return NULL;
    }
    return ((_gt_label_st * )label)->text;
}

void gt_label_set_font_color(gt_obj_st * label, gt_color_t color)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->font_color = color;
    gt_event_send(label, GT_EVENT_TYPE_DRAW_START, NULL);
}

gt_color_t gt_label_get_font_color(gt_obj_st * label)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return gt_color_black();
    }
    _gt_label_st * style = (_gt_label_st * )label;
    return style->font_color;
}

void gt_label_set_font_size(gt_obj_st * label, uint8_t size)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->font_info.size = size;
    _reset_auto_scroll_st(style->auto_scroll);
}

void gt_label_set_font_gray(gt_obj_st * label, uint8_t gray)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->font_info.gray = gray;
}

void gt_label_set_indent(gt_obj_st * label, uint16_t indent)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->indent = indent;
}

void gt_label_set_font_align(gt_obj_st * label, gt_align_et align)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->font_align = align;
    gt_event_send(label, GT_EVENT_TYPE_DRAW_START, NULL);
}
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
void gt_label_set_font_family_cn(gt_obj_st * label, gt_family_t family)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->font_info.style_cn = family;
}
void gt_label_set_font_family_en(gt_obj_st * label, gt_family_t family)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->font_info.style_en = family;
}

void gt_label_set_font_family_fl(gt_obj_st * label, gt_family_t family)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->font_info.style_fl = family;
}
void gt_label_set_font_family_numb(gt_obj_st * label, gt_family_t family)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->font_info.style_numb = family;
}
#else
void gt_label_set_font_family(gt_obj_st * label, gt_family_t family)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    gt_font_set_family(&style->font_info, family);
}
void gt_label_set_font_cjk(gt_obj_st* label, gt_font_cjk_et cjk)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->font_info.cjk = cjk;
}
#endif

void gt_label_set_font_thick_en(gt_obj_st * label, uint8_t thick)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->font_info.thick_en = thick;
}

void gt_label_set_font_thick_cn(gt_obj_st * label, uint8_t thick)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->font_info.thick_cn = thick;
}

void gt_label_set_font_encoding(gt_obj_st * label, gt_encoding_et encoding)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->font_info.encoding = encoding;
}

void gt_label_set_space(gt_obj_st * label, uint8_t space_x, uint8_t space_y)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->space_x = space_x;
    style->space_y = space_y;
    _reset_auto_scroll_st(style->auto_scroll);
}

void gt_label_set_font_style(gt_obj_st * label, gt_font_style_et font_style)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->font_info.style.all = font_style;
    gt_event_send(label, GT_EVENT_TYPE_DRAW_START, NULL);
}

uint8_t gt_label_get_font_size(gt_obj_st * label)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return 0;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    return style->font_info.size;
}

uint8_t gt_label_get_space_x(gt_obj_st * label)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return 0;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    return style->space_x;
}

uint8_t gt_label_get_space_y(gt_obj_st * label)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return 0;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    return style->space_y;
}

void gt_label_set_font_info(gt_obj_st * label, gt_font_info_st * font_info)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    if (NULL == font_info) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    gt_memcpy(&style->font_info, font_info, sizeof(gt_font_info_st));
}

gt_font_info_st * gt_label_get_font_info(gt_obj_st * label)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return 0;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    return &style->font_info;
}

uint16_t gt_label_get_longest_line_substring_width(gt_obj_st * label)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return 0;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    return gt_font_get_longest_line_substring_width(&style->font_info, style->text, style->space_x);
}

void gt_label_set_single_line(gt_obj_st * label, bool is_single_line)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    if (is_single_line == style->text_style.single_line) {
        return ;
    }
    style->text_style.single_line = is_single_line;

    if (false == style->text_style.single_line) {
        _free_auto_scroll_st(style);
    }

    gt_event_send(label, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_label_set_omit_single_line(gt_obj_st * label, bool is_omit)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    style->text_style.omit_line = is_omit;
    style->text_style.single_line = is_omit;

    _free_auto_scroll_st(style);
    gt_event_send(label, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_label_set_auto_scroll_single_line(gt_obj_st * label, bool is_auto_scroll)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
    if (is_auto_scroll) {
        style->text_style.single_line = true;
        style->text_style.omit_line = false;
        _create_auto_scroll_anim(label);
    } else {
        _free_auto_scroll_st(style);
        style->text_style.omit_line = true;
    }

    gt_event_send(label, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_label_set_auto_scroll_total_time(gt_obj_st * label, uint32_t total_time_ms)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return ;
    }
    _gt_label_st * style = (_gt_label_st * )label;
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
        gt_event_send(label, GT_EVENT_TYPE_DRAW_START, NULL);   /** restart scroll */
    }
}

bool gt_label_is_single_line(gt_obj_st * label)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return false;
    }
    _gt_label_st * style = (_gt_label_st * )label;

    return style->text_style.single_line ? true : false;
}

bool gt_label_is_omit_single_line(gt_obj_st * label)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return false;
    }
    _gt_label_st * style = (_gt_label_st * )label;

    return style->text_style.omit_line ? true : false;
}

bool gt_label_is_auto_scroll_single_line(gt_obj_st * label)
{
    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return false;
    }
    _gt_label_st * style = (_gt_label_st * )label;

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

void gt_label_add_text(gt_obj_st * label, const char * fmt, ...)
{
    char buffer[8] = {0};
    va_list args;
    va_list args2;

    if (false == gt_obj_is_type(label, OBJ_TYPE)) {
        return;
    }
    if (NULL == fmt) {
        return;
    }

    _gt_label_st * style = (_gt_label_st * )label;
    _reset_auto_scroll_st(style->auto_scroll);
    va_start(args, fmt);
    va_copy(args2, args);
    uint16_t size = (NULL == fmt) ? 0 : (vsnprintf(buffer, sizeof(buffer), fmt, args) + 1);
    va_end(args);
    if (!size) {
        goto free_lb;
    }

    uint16_t len = 0;

    if (NULL == style->text) {
        style->text = gt_mem_malloc(size);
    } else {
        len = strlen(style->text);
        style->text = gt_mem_realloc(style->text, size + len);
    }
    if (NULL == style->text) {
        goto free_lb;
    }

    gt_memset(&style->text[len], 0, size);
    va_start(args2, fmt);
    vsnprintf(&style->text[len], size, fmt, args2);
    va_end(args2);

    _update_label_size(label, size + len);
    gt_event_send(label, GT_EVENT_TYPE_UPDATE_VALUE, NULL);

    return;

free_lb:
    va_end(args2);
}
#endif  /** GT_CFG_ENABLE_LABEL */
/* end ------------------------------------------------------------------*/
