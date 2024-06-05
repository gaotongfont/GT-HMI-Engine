/**
 * @file gt_textarea.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-11 15:03:35
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_textarea.h"

#if GT_CFG_ENABLE_TEXTAREA
#include "../core/gt_mem.h"
#include "../core/gt_indev.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../others/gt_assert.h"
#include "../core/gt_disp.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_TEXTAREA
#define MY_CLASS    &gt_textarea_class

/* private typedef ------------------------------------------------------*/

typedef struct _textarea_reg_s {
    uint8_t click_valid : 1;        /** click status: scroll will set to false, only click event valid */
    uint8_t click_single_cn : 1;    /** click single chinese character */
    uint8_t reserved    : 6;
}_textarea_reg_st;

typedef struct _gt_textarea_s {
    gt_obj_st obj;
    gt_textarea_param_st * contents;
    uint16_t cnt_contents;
    uint16_t max_height;

    gt_color_t color_background;
    gt_color_t color_border;
    uint8_t border_width;

    gt_font_info_st font_info;

    uint8_t bg_opa;     /* @ref gt_color.h */
    uint8_t space_x;
    uint8_t space_y;
    uint8_t font_align;     //@ref gt_align_et

#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
    /** detail @ref gt_font.h */
    gt_font_touch_word_st select_word;
#endif

    _textarea_reg_st reg;
}_gt_textarea_st;


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_textarea_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_textarea_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static void _draw_content_or_get_words(gt_obj_st * obj, gt_point_st * touch_point_p) {
    _gt_textarea_st * style = (_gt_textarea_st * )obj;
    uint16_t idx = 0;
    gt_font_st font = {
        .info       = style->font_info,
        .res        = NULL,
    };

    gt_attr_font_st font_attr = {
        .font       = &font,
        .space_x    = style->space_x,
        .space_y    = style->space_y,
        .align      = style->font_align,
        .opa        = obj->opa,
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
        .reg.touch_point = touch_point_p ? true : false,
        .reg.single_cn = style->reg.click_single_cn,
        .touch_point = touch_point_p,
#endif
    };
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
    if (touch_point_p) {
        style->select_word.word_p = NULL;
        style->select_word.len = 0;
    }
#endif

    _gt_draw_font_res_st font_res = {0};
    gt_area_st area_font = gt_area_reduce(obj->area, style->border_width + 2);
    font_attr.logical_area = area_font;
    font_attr.logical_area.y += obj->process_attr.scroll.y;
    font_attr.logical_area.h = 0xffff;

    uint16_t height = (obj->reduce << 1) + font_attr.font->info.size + (font_attr.space_y << 1);
    while (idx < style->cnt_contents) {
        font.utf8               = style->contents[idx].text;
        font.len                = style->contents[idx].len;
        font_attr.reg.style     = style->contents[idx].mask_style;
#if GT_TEXTAREA_CUSTOM_FONT_STYLE
        font_attr.font_color    = style->contents[idx].font_info.palette;
        font.info               = style->contents[idx].font_info;
#else
        font_attr.font_color    = style->contents[idx].color;
#endif

        font_res = draw_text(obj->draw_ctx, &font_attr, &area_font);
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
        if (touch_point_p && font_res.touch_word.word_p) {
            style->select_word = font_res.touch_word;
            break;
        }
#endif
        font_attr.start_x       = font_res.area.x;
        font_attr.start_y       = font_res.area.y;
        font_attr.reg.enabled_start = true;
        ++idx;
    }

    if (touch_point_p) {
        /** Get touch word handler, do not calc all text height to scroll */
        return ;
    }
    height += font_res.area.y - font_attr.logical_area.y;
    if (height > style->max_height) {
        style->max_height = height;
    }
}

/**
 * @brief obj init textarea widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    _gt_textarea_st * style = (_gt_textarea_st * )obj;

    /** draw border */
    gt_area_st area_font = gt_area_reduce(obj->area, gt_obj_get_reduce(obj));

    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.radius = obj->radius;
    rect_attr.border_width = style->border_width;
    rect_attr.reg.is_fill = 1;
    rect_attr.bg_color = style->color_background;
    rect_attr.border_color = style->color_border;
    rect_attr.bg_opa = style->bg_opa;

    draw_bg(obj->draw_ctx, &rect_attr, &area_font);

    _draw_content_or_get_words(obj, NULL);

    // focus
    draw_focus(obj, 0);
}

static void _free_contents(gt_obj_st * obj) {
    _gt_textarea_st * style_p = (_gt_textarea_st * )obj;
    if (NULL != style_p->contents) {
        gt_size_t i = style_p->cnt_contents - 1;
        for (; i >= 0; i--) {
            if (NULL == style_p->contents[i].text) {
                continue;
            }
            gt_mem_free(style_p->contents[i].text);
            style_p->contents[i].text = NULL;
            style_p->contents[i].len = 0;
        }

        gt_mem_free(style_p->contents);
        style_p->contents = NULL;
        style_p->max_height = 0;
        style_p->cnt_contents = 0;
        obj->process_attr.scroll.y = 0;
    }
}

/**
 * @brief obj deinit call back
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");
    if (NULL == obj) {
        return ;
    }
    _free_contents(obj);
}


/**
 * @brief obj event handler call back
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);
    _gt_textarea_st * style = (_gt_textarea_st * )obj;
    switch(code) {
        case GT_EVENT_TYPE_DRAW_START:
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_PRESSING:   /* add clicking style and process clicking event */
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_PRESSED:
            style->reg.click_valid = true;
            break;

        case GT_EVENT_TYPE_INPUT_SCROLL:
            style->reg.click_valid = false;
            if (style->max_height < obj->area.h) {
                obj->process_attr.scroll.y = 0;
                break;
            }
            if (obj->process_attr.scroll.y > 0) {
                obj->process_attr.scroll.y = 0;
            } else if (gt_abs(obj->process_attr.scroll.y) + obj->area.h > style->max_height) {
                obj->process_attr.scroll.y = obj->area.h - style->max_height;
            }
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_RELEASED: /* click event finish */
            if (false == style->reg.click_valid) {
                break;
            }
    #if _GT_FONT_GET_WORD_BY_TOUCH_POINT
            gt_point_st tp = gt_indev_get_point();
            _draw_content_or_get_words(obj, &tp);
    #endif
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
}

/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_textarea_create(gt_obj_st * parent) {
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    obj->fixed = false;
    obj->radius = 4;

    _gt_textarea_st * style = (_gt_textarea_st * )obj;
    gt_font_info_init(&style->font_info);

    style->color_background  = gt_color_white();
    style->bg_opa            = GT_OPA_100;
    style->font_align        = GT_ALIGN_NONE;
    style->space_x           = 0;
    style->space_y           = 0;
    style->color_border      = gt_color_hex(0xc7c7c7);
    style->border_width      = 2;
    return obj;
}

void gt_textarea_set_text(gt_obj_st * textarea, char * text)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    uint8_t style_mask = GT_FONT_STYLE_NONE;
    gt_color_t color = gt_color_black();
    uint16_t len = 0;

    if (style->cnt_contents) {
#if GT_TEXTAREA_CUSTOM_FONT_STYLE
        color = style->contents[style->cnt_contents - 1].font_info.palette;
#else
        color = style->contents[style->cnt_contents - 1].color;
#endif
        _free_contents(textarea);
    }
    if (NULL == text) {
        return;
    }
    len = strlen(text);
    style->contents  = gt_mem_malloc( sizeof(gt_textarea_param_st) );
    style->contents[0].len  = len;
    style->contents[0].text = gt_mem_malloc(style->contents[0].len + 1);
    gt_memcpy(style->contents[0].text, text, len);
    style->contents[0].text[len] = '\0';
    style->contents[0].mask_style = style_mask;
#if GT_TEXTAREA_CUSTOM_FONT_STYLE
    style->contents[0].font_info.palette = color;
#else
    style->contents[0].color = color;
#endif
    style->cnt_contents = 1;
    style->max_height = 0;

#if GT_TEXTAREA_CUSTOM_FONT_STYLE
    style->contents[0].font_info = style->font_info;
    gt_font_info_update_font_thick(&style->contents[0].font_info);
#endif

    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

char * gt_textarea_get_text(gt_obj_st * textarea,uint16_t idx)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return NULL;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    return style->contents[idx].text;
}

void gt_textarea_clear_all_str(gt_obj_st * textarea)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    _free_contents(textarea);
}

void gt_textarea_add_str(gt_obj_st * textarea, char * str, gt_font_style_et style_mask, gt_color_t color)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    uint16_t idx = style->cnt_contents;

    if (idx) {
        style->contents = gt_mem_realloc(style->contents, (style->cnt_contents + 1) * sizeof(gt_textarea_param_st));
    } else {
        style->contents = gt_mem_malloc(sizeof(gt_textarea_param_st));
    }
    if (NULL == style->contents) {
        return;
    }
    style->contents[idx].len  = strlen(str);
    style->contents[idx].text = gt_mem_malloc(style->contents[idx].len + 1);
    gt_memcpy(style->contents[idx].text, str, style->contents[idx].len);
    style->contents[idx].text[style->contents[idx].len] = '\0';
    style->contents[idx].mask_style = style_mask;

#if GT_TEXTAREA_CUSTOM_FONT_STYLE
    style->contents[idx].font_info = style->font_info;
    style->contents[idx].font_info.palette = color;
    gt_font_info_update_font_thick(&style->contents[idx].font_info);
#else
    style->contents[idx].color = color;
#endif

    ++style->cnt_contents;
}

#if GT_TEXTAREA_CUSTOM_FONT_STYLE
void gt_textarea_add_str_by_param(gt_obj_st * textarea, gt_textarea_param_st * param)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    if (NULL == param) {
        return;
    }
    if (NULL == param->text || 0 == param->len) {
        return;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    uint16_t idx = style->cnt_contents;

    if (idx) {
        style->contents = gt_mem_realloc(style->contents, (style->cnt_contents + 1) * sizeof(gt_textarea_param_st));
    } else {
        style->contents = gt_mem_malloc(sizeof(gt_textarea_param_st));
    }
    if (NULL == style->contents) {
        return;
    }

    uint8_t new_line_len = param->new_line ? 1 : 0;

    style->contents[idx].len = param->len + new_line_len;
    style->contents[idx].text = gt_mem_malloc(style->contents[idx].len + 1);
    gt_memcpy(style->contents[idx].text, param->text, param->len);
    if (param->new_line) {
        style->contents[idx].text[param->len] = '\n';
    }
    style->contents[idx].text[style->contents[idx].len] = '\0';
    style->contents[idx].mask_style = param->mask_style;
    style->contents[idx].font_info = param->font_info;
    gt_font_info_update_font_thick(&style->contents[idx].font_info);
    ++style->cnt_contents;
}
#endif

void gt_textarea_set_space(gt_obj_st * textarea, uint8_t space_x, uint8_t space_y)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->space_x = space_x;
    style->space_y = space_y;

    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_textarea_set_font_size(gt_obj_st * textarea, uint8_t size)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->font_info.size = size;
    gt_font_info_update_font_thick(&style->font_info);
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}
void gt_textarea_set_font_gray(gt_obj_st * textarea, uint8_t gray)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->font_info.gray = gray;
}
void gt_textarea_set_font_align(gt_obj_st * textarea, gt_align_et align)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;

    // NOTE : textarea only support left align
    align = GT_ALIGN_LEFT;

    style->font_align = align;
}
void gt_textarea_set_font_color(gt_obj_st * textarea, gt_color_t color)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    uint16_t idx = 0;
    while (idx < style->cnt_contents) {
#if GT_TEXTAREA_CUSTOM_FONT_STYLE
        style->contents[idx].font_info.palette = color;
#else
        style->contents[idx].color = color;
#endif
        idx ++;
    }
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_textarea_set_bg_color(gt_obj_st * textarea, gt_color_t color)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->color_background = color;
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_textarea_set_bg_opa(gt_obj_st * textarea, uint8_t opa)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->bg_opa = opa;
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_textarea_set_border_width(gt_obj_st * textarea, uint8_t width)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->border_width = width;
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_textarea_set_border_color(gt_obj_st * textarea, gt_color_t color)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->color_border = color;
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_textarea_set_radius(gt_obj_st * textarea, gt_radius_t radius)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    textarea->radius = radius;
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_textarea_set_font_family_cn(gt_obj_st * textarea, gt_family_t family)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->font_info.style_cn = family;
}

void gt_textarea_set_font_family_en(gt_obj_st * textarea, gt_family_t family)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->font_info.style_en = family;
}
void gt_textarea_set_font_family_fl(gt_obj_st * textarea, gt_family_t family)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->font_info.style_fl = family;
}
void gt_textarea_set_font_family_numb(gt_obj_st * textarea, gt_family_t family)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->font_info.style_numb = family;
}

void gt_textarea_set_font_thick_en(gt_obj_st * textarea, uint8_t thick)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->font_info.thick_en = thick;
    gt_font_info_update_font_thick(&style->font_info);
}

void gt_textarea_set_font_thick_cn(gt_obj_st * textarea, uint8_t thick)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->font_info.thick_cn = thick;
    gt_font_info_update_font_thick(&style->font_info);
}

void gt_textarea_set_font_encoding(gt_obj_st * textarea, gt_encoding_et encoding)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->font_info.encoding = encoding;
}

#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
void gt_textarea_set_touch_single_chinese_word(gt_obj_st * textarea, bool is_single_cn)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->reg.click_single_cn = is_single_cn ? 1 : 0;
}

gt_font_touch_word_st gt_textarea_get_touch_word(gt_obj_st * textarea)
{
    gt_font_touch_word_st ret = {0};
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ret;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    return style->select_word;
}
#endif

#endif  /** GT_CFG_ENABLE_TEXTAREA */
/* end ------------------------------------------------------------------*/
