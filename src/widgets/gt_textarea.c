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

#define _FONT_AREA_REDUCE_SIZE      2

/* private typedef ------------------------------------------------------*/

typedef struct _textarea_reg_s {
    uint8_t hold_on : 1;                    /** finger hold on content for scroll */
    uint8_t click_valid : 1;                /** click status: scroll will set to false, only click event valid */
    uint8_t click_single_cn : 1;            /** click single chinese character */
    uint8_t pull_to_refresh : 1;            /** pull to refresh */
    uint8_t load_more : 1;                  /** load more */
    uint8_t always_scroll_to_newest : 1;    /** always scroll to newest string */
    uint8_t reserved : 2;
}_textarea_reg_st;

#if GT_TEXTAREA_SUB_STRING_SCROLL_CACHE
typedef struct {
    gt_size_t idx;
    gt_area_st offset_abs;
}_scroll_cache_item_st;

typedef struct {
    _scroll_cache_item_st * list;
    uint16_t item_count;
}_sub_string_scroll_cache_st;
#endif

typedef struct _gt_textarea_s {
    gt_obj_st obj;
    gt_textarea_param_st * contents;
    uint16_t cnt_contents;
    uint16_t max_height;            /** update when current display max height */
    gt_anim_st * scroll_to_newest_anim;     /** when add newest string */

    gt_color_t color_border;
    gt_color_t color_font;
    uint8_t border_width;

    gt_font_info_st font_info;

    uint16_t indent;
    uint8_t bg_opa;     /* @ref gt_color.h */
    uint8_t space_x;
    uint8_t space_y;
    uint8_t font_align;     //@ref gt_align_et
    uint8_t padding;

    uint8_t start_x_off;
    uint8_t start_y_off;

#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
    /** detail @ref gt_font.h */
    gt_font_touch_word_st select_word;
#endif

#if GT_TEXTAREA_SUB_STRING_SCROLL_CACHE
    _sub_string_scroll_cache_st scroll_cache;
#endif

    _textarea_reg_st reg;
}_gt_textarea_st;


/* static variables -----------------------------------------------------*/
static void _textarea_init_cb(gt_obj_st * obj);
static void _textarea_deinit_cb(gt_obj_st * obj);
static void _textarea_event_cb(struct gt_obj_s * obj, gt_event_st * e);
GT_ATTRIBUTE_RAM_TEXT static void _create_auto_scoll_to(gt_obj_st * textarea);

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_textarea_class = {
    ._init_cb      = _textarea_init_cb,
    ._deinit_cb    = _textarea_deinit_cb,
    ._event_cb     = _textarea_event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_textarea_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
static inline void _set_touch_word(_gt_textarea_st * style, char * word, uint16_t len) {
    style->select_word.word_p = word;
    style->select_word.len = len;
}
#endif

#if GT_TEXTAREA_SUB_STRING_SCROLL_CACHE
static void _free_reset_scroll_cache(_gt_textarea_st * style) {
    if (style->scroll_cache.list) {
        gt_mem_free(style->scroll_cache.list);
        style->scroll_cache.list = NULL;
    }
    style->scroll_cache.item_count = 0;
}

static void _reset_scroll_list_offset(_gt_textarea_st * style, uint16_t count) {
    if (NULL == style->scroll_cache.list) {
        return;
    }
    gt_memset(style->scroll_cache.list, 0, count);
}

static gt_res_t _resize_scroll_cache(_gt_textarea_st * style, uint16_t cnt) {
    if (0 == cnt) {
        _free_reset_scroll_cache(style);
        return GT_RES_OK;
    }
    uint16_t count = cnt * sizeof(_scroll_cache_item_st);
    if (style->scroll_cache.item_count == count) {
        _reset_scroll_list_offset(style, count);
        return GT_RES_OK;
    }
    if (NULL == style->scroll_cache.list) {
        style->scroll_cache.list = gt_mem_malloc(count);
    } else {
        style->scroll_cache.list = gt_mem_realloc(style->scroll_cache.list, count);
    }
    if (NULL == style->scroll_cache.list) {
        style->scroll_cache.item_count = 0;
        return GT_RES_FAIL;
    }
    style->scroll_cache.item_count = cnt;
    _reset_scroll_list_offset(style, count);
    return GT_RES_OK;
}

static _scroll_cache_item_st * _scroll_cache_get_remark_item_by(_gt_textarea_st * style, gt_size_t offset) {
    if (NULL == style->scroll_cache.list) {
        return NULL;
    }
    uint8_t is_cur_half_display = 0;
    offset = gt_abs(offset);

    /** [Warn] except the begin one */
    for (gt_size_t i = 0, cnt = style->scroll_cache.item_count; i < cnt; ++i) {
        if (style->scroll_cache.list[i].offset_abs.y + style->scroll_cache.list[i].offset_abs.h > offset) {
            return &style->scroll_cache.list[i];
        }
    }
    return NULL;
}

static void _scroll_cache_remark_item(_gt_textarea_st * style, gt_size_t idx, gt_area_st * item_area) {
    if (NULL == style->scroll_cache.list) {
        return;
    }
    if (idx >= style->scroll_cache.item_count) {
        return;
    }
    style->scroll_cache.list[idx].idx = idx;
    style->scroll_cache.list[idx].offset_abs = *item_area;
}
#endif  /** GT_TEXTAREA_SUB_STRING_SCROLL_CACHE */

static inline void _set_hold_on_contents_for_scroll(_gt_textarea_st * style, bool hold_on) {
    style->reg.hold_on = hold_on;
}

static inline bool _is_hold_on_contents_for_scroll(_gt_textarea_st * style) {
    return style->reg.hold_on;
}

static GT_ATTRIBUTE_RAM_TEXT void
_draw_content_or_get_words(gt_obj_st * obj, gt_point_st * touch_point_p, bool is_only_calc_height) {
    _gt_textarea_st * style = (_gt_textarea_st * )obj;
    uint16_t idx = 0;
    gt_font_st font = {
        .info       = style->font_info,
        .res        = NULL,
    };
    if (NULL == style->contents) {
        return;
    }
    if (0 == obj->area.w || 0 == obj->area.h) {
        return;
    }

    gt_attr_font_st font_attr = {
        .font       = &font,
        .space_x    = style->space_x,
        .space_y    = style->space_y,
        .align      = style->font_align,
        .indent     = style->indent,
        .opa        = is_only_calc_height ? GT_OPA_0 : obj->opa,
        .reg.immediately_return = _is_hold_on_contents_for_scroll(style),
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
        .reg.touch_point = touch_point_p ? true : false,
        .reg.single_cn = style->reg.click_single_cn,
        .touch_point = touch_point_p,
#endif
    };
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
    if (touch_point_p) {
        _set_touch_word(style, NULL, 0);
    }
#endif

    gt_size_t scroll_y = obj->process_attr.scroll.y;
    _gt_draw_font_res_st font_res = {0};
    gt_area_st padding_font = gt_area_reduce(obj->area, style->padding);
    gt_area_st area_font = gt_area_reduce(padding_font, style->border_width + _FONT_AREA_REDUCE_SIZE);
    font_attr.logical_area = area_font;
    font_attr.logical_area.y += scroll_y;
    font_attr.logical_area.h = 0xffff;

    uint16_t height = font_attr.font->info.size + (font_attr.space_y << 1);

#if GT_TEXTAREA_SUB_STRING_SCROLL_CACHE
    gt_area_st cur_offset = {0};
    if (_is_hold_on_contents_for_scroll(style)) {
        _scroll_cache_item_st * remark_item_p = _scroll_cache_get_remark_item_by(style, scroll_y);
        if (remark_item_p) {
            font_attr.reg.enabled_start = true;
            idx = remark_item_p->idx;
            font_attr.start_x = remark_item_p->offset_abs.x + font_attr.logical_area.x;
            font_attr.start_y = remark_item_p->offset_abs.y + font_attr.logical_area.y;
        }
    }
#endif

    if(0 != style->start_x_off || 0 != style->start_y_off) {
        font_attr.start_x = style->start_x_off + area_font.x + gt_font_get_indent_width(&font.info, style->indent);
        font_attr.start_y = style->start_y_off + area_font.y;
        font_attr.reg.enabled_start = true;
    }

    while (idx < style->cnt_contents) {
        font.utf8               = style->contents[idx].text;
        if (NULL == font.utf8) {
            ++idx;
            continue;
        }
        font.len                = style->contents[idx].len;
        font.info.style.all = style->contents[idx].mask_style;

#if GT_TEXTAREA_CUSTOM_FONT_STYLE
        font_attr.font_color    = style->contents[idx].font_info.palette;
        font.info               = style->contents[idx].font_info;
#else
        font_attr.font_color    = style->contents[idx].color;
#endif
        font_res = draw_text(obj->draw_ctx, &font_attr, &area_font);
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
        if (touch_point_p && font_res.touch_word.word_p) {
            _set_touch_word(style, font_res.touch_word.word_p, font_res.touch_word.len);
            break;
        }
#endif
        if (_is_hold_on_contents_for_scroll(style) && GT_RES_INV == font_res.res) {
            break;
        }
#if GT_TEXTAREA_SUB_STRING_SCROLL_CACHE
        if (false == _is_hold_on_contents_for_scroll(style)) {
            if (idx) {
                cur_offset.x = font_attr.start_x - font_attr.logical_area.x;
                cur_offset.y = font_attr.start_y - font_attr.logical_area.y;
            } else {
                cur_offset.x = 0;
                cur_offset.y = 0;
            }
            cur_offset.w = font_res.size.x;
            cur_offset.h = font_res.size.y;
            _scroll_cache_remark_item(style, idx, &cur_offset);
        }
#endif
        font_attr.start_x       = font_res.abs_pos.x;
        font_attr.start_y       = font_res.abs_pos.y;
        font_attr.reg.enabled_start = true;
        ++idx;
    }

    if (touch_point_p) {
        /** Get touch word handler, do not calc all text height to scroll */
        return ;
    }
    height += font_res.abs_pos.y + font.info.size - font_attr.logical_area.y;
    if (height > style->max_height) {
        style->max_height = height;
        if (style->reg.always_scroll_to_newest) {
            _create_auto_scoll_to(obj);
        }
    }
}

/**
 * @brief obj init textarea widget call back
 *
 * @param obj
 */
static GT_ATTRIBUTE_RAM_TEXT void
_textarea_init_cb(gt_obj_st * obj) {
    _gt_textarea_st * style = (_gt_textarea_st * )obj;

    /** draw border */
    if (obj->show_bg) {
        gt_attr_rect_st rect_attr;
        gt_graph_init_rect_attr(&rect_attr);
        rect_attr.radius = obj->radius;
        rect_attr.border_width = style->border_width;
        rect_attr.reg.is_fill = 1;
        rect_attr.bg_color = obj->bgcolor;
        rect_attr.border_color = style->color_border;
        rect_attr.bg_opa = style->bg_opa;

        draw_bg(obj->draw_ctx, &rect_attr, &obj->area);
    }

    _draw_content_or_get_words(obj, NULL, false);

    // focus
    draw_focus(obj);
}

static GT_ATTRIBUTE_RAM_TEXT inline void
_reset_scroll_value(gt_obj_process_attr_st * process_attr) {
    process_attr->scroll.y = 0;
    process_attr->scroll_prev.y = 0;
}

static GT_ATTRIBUTE_RAM_TEXT void
_free_contents(gt_obj_st * obj) {
    _gt_textarea_st * style_p = (_gt_textarea_st * )obj;
    if (style_p->contents) {
        _set_hold_on_contents_for_scroll(style_p, false);
#if GT_TEXTAREA_SUB_STRING_SCROLL_CACHE
        _free_reset_scroll_cache(style_p);
#endif

        for (gt_size_t i = style_p->cnt_contents - 1; i >= 0; i--) {
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
        _reset_scroll_value(&obj->process_attr);
    }
}

/**
 * @brief obj deinit call back
 *
 * @param obj
 */
static GT_ATTRIBUTE_RAM_TEXT void
_textarea_deinit_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");
    if (NULL == obj) {
        return ;
    }
    _free_contents(obj);
}

static GT_ATTRIBUTE_RAM_TEXT void
_ready_pull_to_refresh_cb(struct gt_anim_s * anim) {
    gt_obj_st * obj = (gt_obj_st *)anim->tar;
    _gt_textarea_st * style = (_gt_textarea_st * )obj;

    style->reg.pull_to_refresh = false;
    gt_event_send(obj, GT_EVENT_TYPE_PULL_TO_REFRESH, NULL);
}

static GT_ATTRIBUTE_RAM_TEXT void
_pull_wait_to_refresh(struct gt_obj_s * obj) {
    gt_anim_st anim;
    gt_anim_init(&anim);
    gt_anim_set_target(&anim, obj);
    gt_anim_set_time(&anim, 0);
    gt_anim_set_time_delay_start(&anim, 200);
    gt_anim_set_ready_cb(&anim, _ready_pull_to_refresh_cb);
    gt_anim_start(&anim);
}

static GT_ATTRIBUTE_RAM_TEXT void
_ready_load_more_cb(struct gt_anim_s * anim) {
    gt_obj_st * obj = (gt_obj_st *)anim->tar;
    _gt_textarea_st * style = (_gt_textarea_st * )obj;

    style->reg.load_more = false;
    gt_event_send(obj, GT_EVENT_TYPE_LOAD_MORE, NULL);
}

static GT_ATTRIBUTE_RAM_TEXT void
_load_more(struct gt_obj_s * obj) {
    gt_anim_st anim;
    gt_anim_init(&anim);
    gt_anim_set_target(&anim, obj);
    gt_anim_set_time(&anim, 0);
    gt_anim_set_time_delay_start(&anim, 200);
    gt_anim_set_ready_cb(&anim, _ready_load_more_cb);
    gt_anim_start(&anim);
}

GT_ATTRIBUTE_RAM_TEXT static inline bool
_is_scroll_to_bottom(gt_obj_st * obj) {
    _gt_textarea_st * style = (_gt_textarea_st * )obj;
    return gt_abs(obj->process_attr.scroll.y) + obj->area.h - style->padding > style->max_height ? true : false;
}

GT_ATTRIBUTE_RAM_TEXT static void
_anim_scroll_exec_cb(void * tv, int32_t val) {
    gt_obj_st * obj = (gt_obj_st * )tv;
    _gt_textarea_st * style = (_gt_textarea_st * )tv;

    if (_is_scroll_to_bottom(obj)) {
        gt_anim_stop(style->scroll_to_newest_anim);
        gt_anim_del_safe_by(style->scroll_to_newest_anim);
        style->scroll_to_newest_anim = NULL;
        return;
    }
    obj->process_attr.scroll.y -= 1;
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

GT_ATTRIBUTE_RAM_TEXT static void
_anim_scroll_ready_cb(struct gt_anim_s * anim) {
    _gt_textarea_st * style = (_gt_textarea_st * )anim->tar;
    style->scroll_to_newest_anim = NULL;
}

GT_ATTRIBUTE_RAM_TEXT static void
_create_auto_scoll_to(gt_obj_st * textarea) {
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    int32_t scroll_time = 3 * 60 * 1000;    /** predicted value */
    if (style->scroll_to_newest_anim) {
        /** update time */
        gt_anim_set_time(style->scroll_to_newest_anim, scroll_time);
        return;
    }
    gt_anim_st anim;
    gt_anim_init(&anim);
    gt_anim_set_target(&anim, textarea);
    gt_anim_set_time(&anim, scroll_time);
    gt_anim_set_value(&anim, 0, 0xffff);
    gt_anim_set_exec_cb(&anim, _anim_scroll_exec_cb);
    gt_anim_set_ready_cb(&anim, _anim_scroll_ready_cb);
    style->scroll_to_newest_anim = gt_anim_start(&anim);
}

/**
 * @brief obj event handler call back
 *
 * @param obj
 * @param e event
 */
static GT_ATTRIBUTE_RAM_TEXT void
_textarea_event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code_val = gt_event_get_code(e);
    _gt_textarea_st * style = (_gt_textarea_st * )obj;
    switch(code_val) {
        case GT_EVENT_TYPE_DRAW_START: {
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;
        }
        case GT_EVENT_TYPE_INPUT_PRESSED: {
            style->reg.click_valid = true;
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
            _set_touch_word(style, NULL, 0);
#endif
            break;
        }
        case GT_EVENT_TYPE_INPUT_SCROLL: {
            style->reg.click_valid = false;
            if (style->max_height < obj->area.h) {
                _reset_scroll_value(&obj->process_attr);
                break;
            }
            style->reg.load_more = false;
            style->reg.pull_to_refresh = false;
            if (obj->process_attr.scroll.y > 0) {
                if (false == style->reg.pull_to_refresh && obj->process_attr.scroll.y > obj->area.h >> 4) {
                    style->reg.pull_to_refresh = true;
                }
                _reset_scroll_value(&obj->process_attr);
            } else if (_is_scroll_to_bottom(obj)) {
                if (false == style->reg.load_more &&
                    gt_abs(obj->process_attr.scroll.y) + obj->area.h - (obj->area.h >> 4) > style->max_height) {
                    style->reg.load_more = true;
                }
                obj->process_attr.scroll.y = obj->area.h - style->max_height;
            }
            gt_disp_invalid_area(obj);
            break;
        }
        case GT_EVENT_TYPE_INPUT_SCROLL_END: {
            if (style->reg.pull_to_refresh) {
                _pull_wait_to_refresh(obj);
            } else if (style->reg.load_more) {
                _load_more(obj);
            } else {
                _set_hold_on_contents_for_scroll(style, true);
                gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            }
            break;
        }
        case GT_EVENT_TYPE_INPUT_RELEASED: {
            /* click event finish */
            if (false == style->reg.click_valid) {
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
                _set_touch_word(style, NULL, 0);
#endif
                break;
            }
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
            gt_point_st tp = gt_indev_get_point();
            _draw_content_or_get_words(obj, &tp, false);
#endif
            // gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;
        }
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
    obj->area.w = 100;
    obj->area.h = 100;
    obj->show_bg = true;

    _gt_textarea_st * style = (_gt_textarea_st * )obj;
    gt_font_info_init(&style->font_info);

    obj->bgcolor             = gt_color_white();
    style->color_font        = gt_color_black();
    style->bg_opa            = GT_OPA_100;
    style->font_align        = GT_ALIGN_NONE;
    style->space_x           = 0;
    style->space_y           = 0;
    style->color_border      = gt_color_hex(0xc7c7c7);
    style->border_width      = 2;

    _set_hold_on_contents_for_scroll(style, false);
    return obj;
}

gt_res_t gt_textarea_set_always_scroll_to_newest(gt_obj_st * textarea, bool always_scroll_to_newest)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return GT_RES_FAIL;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->reg.always_scroll_to_newest = always_scroll_to_newest ? true : false;
    return GT_RES_OK;
}

void gt_textarea_set_text(gt_obj_st * textarea, const char * fmt, ...)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }

    char buffer[8] = {0};
    va_list args;
    va_list args2;
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

    gt_textarea_set_text_by_len(textarea, (const char* )text, strlen(text));

    gt_mem_free(text);
    return ;
free_lb:
    va_end(args2);
}

void gt_textarea_set_text_by_len(gt_obj_st * textarea, const char * text, uint16_t len)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    uint8_t style_mask = GT_FONT_STYLE_NONE;
    gt_color_t color = style->color_font;

    if (style->cnt_contents) {
#if GT_TEXTAREA_CUSTOM_FONT_STYLE
        color = style->contents[style->cnt_contents - 1].font_info.palette;
#else
        color = style->contents[style->cnt_contents - 1].color;
#endif
        _free_contents(textarea);
        style->cnt_contents = 0;
    }
    if (NULL == text) {
        return;
    }
    style->contents  = gt_mem_malloc( sizeof(gt_textarea_param_st) );
    GT_CHECK_BACK(style->contents);
    style->contents[0].len  = len;
    style->contents[0].text = gt_mem_malloc(style->contents[0].len + 1);
    if (NULL == style->contents[0].text) {
        GT_CHECK_PRINT(style->contents[0].text);
        gt_mem_free(style->contents);
        style->contents = NULL;
        style->cnt_contents = 0;
        style->max_height = 0;
        return;
    }
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

#if GT_TEXTAREA_SUB_STRING_SCROLL_CACHE
    _resize_scroll_cache(style, style->cnt_contents);
#endif
    _set_hold_on_contents_for_scroll(style, false);

    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_textarea_hide_background(gt_obj_st * textarea, bool hide)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    gt_obj_show_bg(textarea, hide ? false : true);
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
    _free_contents(textarea);
}

void gt_textarea_add_str(gt_obj_st * textarea, char * str, gt_font_style_et style_mask, gt_color_t color)
{
    gt_textarea_add_str_by_len(textarea, str, strlen(str), style_mask, color);
}

void gt_textarea_add_str_by_len(gt_obj_st * textarea, char * str, uint32_t len, gt_font_style_et style_mask, gt_color_t color)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    if (0 == len) { return; }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    uint16_t idx = style->cnt_contents;

    if (idx) {
        style->contents = gt_mem_realloc(style->contents, (style->cnt_contents + 1) * sizeof(gt_textarea_param_st));
    } else {
        style->contents = gt_mem_malloc(sizeof(gt_textarea_param_st));
    }
    if (NULL == style->contents) {
        GT_CHECK_PRINT(style->contents);
        style->cnt_contents = 0;
        return;
    }
    style->contents[idx].len  = len;
    style->contents[idx].text = gt_mem_malloc(style->contents[idx].len + 1);
    if (NULL == style->contents[idx].text) {
        /** Reset to before */
        style->contents = gt_mem_realloc(style->contents, (style->cnt_contents) * sizeof(gt_textarea_param_st));
        return;
    }
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
#if GT_TEXTAREA_SUB_STRING_SCROLL_CACHE
    _resize_scroll_cache(style, style->cnt_contents);
#endif
    _set_hold_on_contents_for_scroll(style, false);
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

uint16_t gt_textarea_get_str_count(gt_obj_st * textarea)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return 0;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    return style->cnt_contents;
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
        GT_CHECK_PRINT(style->contents);
        style->cnt_contents = 0;
        return;
    }

    uint8_t new_line_len = param->new_line ? 1 : 0;

    style->contents[idx].len = param->len + new_line_len;
    style->contents[idx].text = gt_mem_malloc(style->contents[idx].len + 1);
    if (NULL == style->contents[idx].text) {
        /** Reset to before */
        style->contents = gt_mem_realloc(style->contents, (style->cnt_contents) * sizeof(gt_textarea_param_st));
        return;
    }
    gt_memcpy(style->contents[idx].text, param->text, param->len);
    if (param->new_line) {
        style->contents[idx].text[param->len] = '\n';
    }
    style->contents[idx].text[style->contents[idx].len] = '\0';
    style->contents[idx].mask_style = param->mask_style;
    style->contents[idx].font_info = param->font_info;
    gt_font_info_update_font_thick(&style->contents[idx].font_info);
    ++style->cnt_contents;
#if GT_TEXTAREA_SUB_STRING_SCROLL_CACHE
    _resize_scroll_cache(style, style->cnt_contents);
#endif
    _set_hold_on_contents_for_scroll(style, false);
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

void gt_textarea_set_indent(gt_obj_st * textarea, uint16_t indent)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->indent = indent;
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
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
    style->color_font = color;
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
    textarea->bgcolor = color;
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
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
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
#else
void gt_textarea_set_font_family(gt_obj_st * textarea, gt_family_t family)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    gt_font_set_family(&style->font_info, family);
}
void gt_textarea_set_font_cjk(gt_obj_st* textarea, gt_font_cjk_et cjk)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->font_info.cjk = cjk;
}

#endif
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

void gt_textarea_set_font_info(gt_obj_st * textarea, gt_font_info_st * font_info)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return ;
    }
    if (NULL == font_info) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    gt_memcpy(&style->font_info, font_info, sizeof(gt_font_info_st));
}

gt_font_info_st * gt_textarea_get_font_info(gt_obj_st * textarea)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return 0;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    return &style->font_info;
}

uint32_t gt_textarea_get_total_height(gt_obj_st * textarea)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return 0;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    _draw_content_or_get_words(textarea, NULL, true);
    return style->max_height;
}

uint32_t gt_textarea_get_display_offset(gt_obj_st * textarea)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return 0;
    }
    return gt_abs(textarea->process_attr.scroll.y);
}

float gt_textarea_get_scroll_percent(gt_obj_st * textarea)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return 0.0;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    int16_t scroll_height = style->max_height - gt_obj_get_h(textarea);
    if (0 == style->max_height) {
        return 0.0;
    }
    if (scroll_height <= 0) {
        return 100.0;
    }
    return (float)(100.0 * gt_abs(textarea->process_attr.scroll.y) / scroll_height);
}

gt_res_t gt_textarea_display_by_precent(gt_obj_st * textarea, float percent)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return GT_RES_FAIL;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    if (percent < 0.0)          { return GT_RES_FAIL; }
    if (percent > 100.0)        { percent = 100.0; }
    if (0 == style->max_height) { return GT_RES_FAIL; }

    int16_t scroll_height = style->max_height - gt_obj_get_h(textarea);
    if (scroll_height <= 0) {
        textarea->process_attr.scroll.y = 0;
    } else {
        textarea->process_attr.scroll.y = -(int32_t)(scroll_height * percent / 100.0);
    }
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
    return GT_RES_OK;
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

void gt_textarea_set_padding(gt_obj_st * textarea, uint8_t padding)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->padding = padding;
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

gt_split_line_ret_st gt_textarea_get_font_split_line_numb(gt_obj_st* obj, gt_font_split_line_st* sp_line)
{
    gt_split_line_ret_st s_line_ret = {0};

    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return s_line_ret;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )obj;
    uint16_t idx = 0, max_w = 0;

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
    gt_split_line_ret_st s_line_tmp_ret = {0};
    s_line_tmp_ret.disp_w = sp_line_tmp.start_w;

    while (idx < style->cnt_contents) {

        sp_line_tmp.text = style->contents[idx].text;
        sp_line_tmp.len = style->contents[idx].len;
        sp_line_tmp.indent = idx == 0 ? sp_line_tmp.indent : 0;
        if(0 == sp_line_tmp.len) {
            ++idx;
            continue;
        }

        s_line_tmp_ret = gt_font_split_line_numb(&style->font_info, &sp_line_tmp);
        sp_line_tmp.start_w = s_line_tmp_ret.disp_w;

        if(s_line_tmp_ret.line_numb > 1) {
            s_line_ret.line_numb += s_line_tmp_ret.line_numb - 1;
            s_line_ret.max_w = GT_MIN(s_line_tmp_ret.max_w, sp_line_tmp.max_w);
        }
        else if(s_line_tmp_ret.disp_w < style->font_info.size && idx != style->cnt_contents -1){
            s_line_ret.line_numb += 1;
            s_line_ret.max_w = GT_MIN(s_line_tmp_ret.max_w, sp_line_tmp.max_w);
            sp_line_tmp.start_w = sp_line_tmp.max_w;
        }
        else if(0x0A == sp_line_tmp.text[sp_line_tmp.len-1]) {
            s_line_ret.line_numb += (idx == style->cnt_contents-1) ? s_line_tmp_ret.line_numb - 1 : s_line_tmp_ret.line_numb;
            s_line_ret.max_w = GT_MIN(s_line_tmp_ret.max_w, sp_line_tmp.max_w);
            sp_line_tmp.start_w = sp_line_tmp.max_w;
        }

        ++idx;
    }

    s_line_ret.max_w = GT_MAX(s_line_ret.max_w, s_line_tmp_ret.max_w);
    s_line_ret.line_numb += 1;

    return s_line_ret;
}

void gt_textarea_append_text(gt_obj_st * obj, const char * str)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )obj;

#if 01
    if(style->cnt_contents > 0) {
        char * text = style->contents[0].text;
        gt_textarea_set_text(obj, "%s%s", text, str);
    }else{
        gt_textarea_set_text(obj, "%s", str);
    }
#else
    gt_textarea_add_str(obj, (char* )str, GT_FONT_STYLE_NONE, style->color_font);
#endif
}

void gt_textarea_set_start_point_offset(gt_obj_st * textarea, uint8_t x_off, uint8_t y_off)
{
    if (false == gt_obj_is_type(textarea, OBJ_TYPE)) {
        return;
    }
    _gt_textarea_st * style = (_gt_textarea_st * )textarea;
    style->start_x_off = x_off;
    style->start_y_off = y_off;
}

#endif  /** GT_CFG_ENABLE_TEXTAREA */
/* end ------------------------------------------------------------------*/
