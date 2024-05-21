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
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_TEXTAREA
#define MY_CLASS    &gt_textarea_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_txt_s {
    char *      text;
    gt_color_t  color;
    uint16_t    len;
    uint8_t     mask_style;
}_gt_txt_st;

typedef struct _gt_textarea_s {
    gt_obj_st obj;
    _gt_txt_st * contents;
    uint16_t cnt_contents;
    uint16_t max_height;

    gt_color_t color_background;
    gt_color_t color_border;
    uint8_t border_width;
    gt_size_t border_radius;

    gt_font_info_st font_info;

    uint8_t bg_opa;     /* @ref gt_color.h */
    uint8_t space_x;
    uint8_t space_y;
    uint8_t font_align;     //@ref gt_align_et
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

/**
 * @brief obj init textarea widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    _gt_textarea_st * style = (_gt_textarea_st * )obj;
    int idx = 0;

    gt_font_st font = {
        .info       = style->font_info,
        .res        = NULL,
    };
    font.info.thick_en = style->font_info.thick_en == 0 ? style->font_info.size + 6: style->font_info.thick_en;
    font.info.thick_cn = style->font_info.thick_cn == 0 ? style->font_info.size + 6: style->font_info.thick_cn;

    gt_attr_font_st font_attr = {
        .font       = &font,
        .space_x    = style->space_x,
        .space_y    = style->space_y,
        .align      = style->font_align,
        .opa        = obj->opa,
    };

    gt_area_st area_font;
    _gt_draw_font_res_st font_res;
    area_font = gt_area_reduce(obj->area, gt_obj_get_reduce(obj));

    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.radius = style->border_radius;
    rect_attr.border_width = style->border_width;
    rect_attr.reg.is_fill = 1;
    rect_attr.bg_color = style->color_background;
    rect_attr.border_color = style->color_border;
    rect_attr.bg_opa = style->bg_opa;

    draw_bg(obj->draw_ctx, &rect_attr, &area_font);

    area_font = gt_area_reduce(obj->area, rect_attr.border_width + 2);
    font_attr.logical_area = area_font;
    font_attr.logical_area.y += obj->process_attr.scroll.y;
    font_attr.logical_area.h = 0xffff;

    uint16_t height = (obj->reduce << 1) + font_attr.font->info.size + (font_attr.space_y << 1);
    while (idx < style->cnt_contents) {
        font.utf8               = style->contents[idx].text;
        font.len                = style->contents[idx].len;
        font_attr.reg.style     = style->contents[idx].mask_style;
        font_attr.font_color    = style->contents[idx].color;
        font_res = draw_text(obj->draw_ctx, &font_attr, &area_font);
        font_attr.start_x       = font_res.area.x;
        font_attr.start_y       = font_res.area.y;
        font_attr.reg.enabled_start = true;
        ++idx;
    }

    height += font_res.area.y - font_attr.logical_area.y;
    if (height > style->max_height) {
        style->max_height = height;
    }

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

        case GT_EVENT_TYPE_INPUT_SCROLL:
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

    _gt_textarea_st * style = (_gt_textarea_st * )obj;
    gt_font_info_init(&style->font_info);

    style->color_background  = gt_color_white();
    style->bg_opa            = GT_OPA_100;
    style->font_align        = GT_ALIGN_NONE;
    style->space_x           = 0;
    style->space_y           = 0;
    style->color_border      = gt_color_hex(0xc7c7c7);
    style->border_radius     = 4;
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
        color = style->contents[style->cnt_contents - 1].color;
        _free_contents(textarea);
    }
    if (text != NULL) {
        len = strlen(text);
    }
    style->contents  = gt_mem_malloc( sizeof(_gt_txt_st) );
    style->contents[0].len  = len;
    style->contents[0].text = gt_mem_malloc(style->contents[0].len);
    gt_memcpy(style->contents[0].text, text, len);
    style->contents[0].mask_style = style_mask;
    style->contents[0].color = color;
    style->cnt_contents = 1;
    style->max_height = 0;

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


void gt_textarea_add_str(gt_obj_st * textarea, char * str, gt_font_style_et style_mask, gt_color_t color)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    if (style->cnt_contents == 0) {
        style->contents  = gt_mem_malloc( sizeof(_gt_txt_st) );
        style->contents[0].len  = strlen(str);
        style->contents[0].text = gt_mem_malloc(style->contents[0].len);
        gt_memcpy(style->contents[0].text, str, style->contents[0].len);
        style->contents[0].mask_style = style_mask;
        style->contents[0].color = color;
        style->cnt_contents = 1;
    } else {
        style->contents  = gt_mem_realloc(style->contents, (style->cnt_contents + 1) * sizeof(_gt_txt_st) );
        style->contents[style->cnt_contents].len  = strlen(str);
        style->contents[style->cnt_contents].text = gt_mem_malloc(style->contents[style->cnt_contents].len);
        gt_memcpy(style->contents[style->cnt_contents].text, str, style->contents[style->cnt_contents].len);
        style->contents[style->cnt_contents].mask_style = style_mask;
        style->contents[style->cnt_contents].color = color;
        style->cnt_contents++;
    }
}

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
    style->font_align = align;
}
void gt_textarea_set_font_color(gt_obj_st * textarea, gt_color_t color)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    uint16_t idx = 0;
    while( idx < style->cnt_contents ){
        style->contents[idx].color = color;
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

void gt_textarea_set_radius(gt_obj_st * textarea, gt_size_t radius)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->border_radius = radius;
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
}

void gt_textarea_set_font_thick_cn(gt_obj_st * textarea, uint8_t thick)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->font_info.thick_cn = thick;
}

void gt_textarea_set_font_encoding(gt_obj_st * textarea, gt_encoding_et encoding)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->font_info.encoding = encoding;
}

#endif  /** GT_CFG_ENABLE_TEXTAREA */
/* end ------------------------------------------------------------------*/
