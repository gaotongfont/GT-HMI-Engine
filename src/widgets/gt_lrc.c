/**
 * @file gt_lrc.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2025-11-17 19:35:22
 * @copyright Copyright (c) 2014-2025, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_lrc.h"

#if GT_CFG_ENABLE_LRC
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../core/gt_obj_scroll.h"
#include "gt_label.h"
/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_LRC
#define MY_CLASS    &gt_lrc_class


/* private typedef ------------------------------------------------------*/
typedef struct _gt_lrc_s {
    gt_obj_st obj;

    gt_anim_st *anim;
    uint32_t* times;
    uint32_t offset_h;
    uint32_t fixed_line;
    uint32_t times_total;
    uint32_t times_count;
    uint32_t index;

    gt_font_info_st font_info;
    gt_color_t color_border;
    gt_color_t select_color;

    uint8_t border_width;
    uint8_t space_x;
    uint8_t space_y;

}_gt_lrc_st;


/* static prototypes ----------------------------------------------------*/
GT_ATTRIBUTE_RAM_TEXT
static void _lrc_init_cb(gt_obj_st * obj);
GT_ATTRIBUTE_RAM_TEXT
static void _lrc_deinit_cb(gt_obj_st * obj);
GT_ATTRIBUTE_RAM_TEXT
static void _lrc_event_cb(struct gt_obj_s * obj, gt_event_st * e);

//
GT_ATTRIBUTE_RAM_TEXT
static void _lrc_start_anim(gt_obj_st* obj);
/* static variables -----------------------------------------------------*/
static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_lrc_class = {
    ._init_cb      = _lrc_init_cb,
    ._deinit_cb    = _lrc_deinit_cb,
    ._event_cb     = _lrc_event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_lrc_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
GT_ATTRIBUTE_RAM_TEXT
static void _lrc_init_cb(gt_obj_st * obj)
{
    _gt_lrc_st* style = (_gt_lrc_st*)obj;

    if (obj->show_bg) {
        gt_attr_rect_st rect_attr;
        gt_graph_init_rect_attr(&rect_attr);
        rect_attr.radius = obj->radius;
        rect_attr.border_width = style->border_width;
        rect_attr.reg.is_fill = 1;
        rect_attr.bg_color = obj->bgcolor;
        rect_attr.border_color = style->color_border;
        rect_attr.bg_opa = obj->opa;
        draw_bg(obj->draw_ctx, &rect_attr, &obj->area);
    }

}

GT_ATTRIBUTE_RAM_TEXT
static void _lrc_deinit_cb(gt_obj_st * obj)
{
    _gt_lrc_st* style = (_gt_lrc_st*)obj;

    if(style->times) {
        gt_mem_free(style->times);
        style->times = NULL;
        style->times_count = 0;
        style->times_total = 0;
        style->index = 1;
    }

    if(style->anim) {
        gt_anim_del_safe_by(obj);
        style->anim = NULL;
    }

}

GT_ATTRIBUTE_RAM_TEXT
static void _lrc_event_cb(struct gt_obj_s * obj, gt_event_st * e)
{
    gt_event_type_et code_val = gt_event_get_code(e);

    switch (code_val)
    {
    case GT_EVENT_TYPE_DRAW_START:
        gt_disp_invalid_area(obj);
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
        break;
    case GT_EVENT_TYPE_UPDATE_VALUE:
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
        break;

    case GT_EVENT_TYPE_INPUT_SCROLL:
        _gt_obj_scroll_internal(obj, 0, 0);
        break;

    default:
        break;
    }
}

GT_ATTRIBUTE_RAM_TEXT
static gt_obj_st* _lrc_add_text(gt_obj_st* obj, uint32_t time, const char* text, uint32_t len)
{
    _gt_lrc_st* style = (_gt_lrc_st*)obj;

    if(style->times_count >= style->times_total) {
        style->times_total = style->times_count + 10;
        uint32_t* tmp = (uint32_t*) gt_mem_realloc(style->times, style->times_total * sizeof(uint32_t));
        if (NULL == tmp) {
            GT_LOGE(GT_LOG_TAG_UI,"realloc failed, size: %d", style->times_total * sizeof(uint32_t));
            return NULL;
        }
        style->times = tmp;
        gt_memset_0(&style->times[style->times_count], (style->times_total - style->times_count) * sizeof(uint32_t));
    }
    gt_obj_st* child = gt_label_create(obj);
    if (NULL == child) {
        GT_LOGE(GT_LOG_TAG_UI,"create child failed");
        return NULL;
    }
    style->times[style->times_count++] = time;

    //
    gt_label_set_text_by_len(child, text, len);
    gt_obj_set_pos_relative(child, obj, 0, style->offset_h);
    gt_label_set_font_info(child, &style->font_info);
    gt_label_set_font_align(child, GT_ALIGN_CENTER);
    gt_label_set_indent(child, 0);
    gt_label_set_space(child, style->space_x, style->space_y);

    gt_font_split_line_st sp_line = {
        .text = (char*)text,
        .len = len,
        .max_w = obj->area.w,
        .start_w = obj->area.w,
        .space = style->space_x,
        .indent = 0,
    };

    gt_split_line_ret_st line_ret = gt_font_split_line_numb(&style->font_info, &sp_line);
    uint32_t h = line_ret.line_numb * (style->font_info.size + style->space_y);
    gt_obj_set_size(child, obj->area.w, h);
    style->offset_h += h;
    return child;
}

GT_ATTRIBUTE_RAM_TEXT
static void _scroll_to_child(gt_obj_st* obj, gt_obj_st* child)
{
    if(!obj || !child) {
        return ;
    }

    _gt_lrc_st* style = (_gt_lrc_st*)obj;

    uint32_t dy = style->fixed_line * (style->font_info.size + style->space_y);

    dy = obj->area.y + dy - child->area.y;
    gt_obj_scroll_to(obj, 0, dy, GT_ANIM_ON);
}

GT_ATTRIBUTE_RAM_TEXT
static void _anim_ready_cb (struct gt_anim_s * anim)
{
    _gt_lrc_st* style = (_gt_lrc_st*)anim->tar;
    ++style->index;
    _lrc_start_anim(anim->tar);
}

GT_ATTRIBUTE_RAM_TEXT
static void _lrc_start_anim(gt_obj_st* obj)
{
    _gt_lrc_st* style = (_gt_lrc_st*)obj;

    if (style->index >= style->times_count || style->index >= obj->cnt_child) {
        if(style->anim) {
            gt_anim_set_paused(style->anim, true);
        }
        return ;
    }

    gt_obj_st* child = NULL;
    if(style->index != 0) {
        child = obj->child[style->index - 1];
        gt_label_set_font_color(child, style->font_info.palette);
    }
    child = obj->child[style->index];
    gt_label_set_font_color(child, style->select_color);

    _scroll_to_child(obj, child);

    if(style->index + 1 >= style->times_count) {
        // gt_anim_del_safe_by(obj);
        // style->anim = NULL;
        gt_anim_set_paused(style->anim, true);
        return ;
    }

    uint32_t time = style->times[style->index + 1] - style->times[style->index];
    if(style->anim) {
        gt_anim_set_paused(style->anim, true);
        gt_anim_set_time(style->anim, time);
        gt_anim_set_paused(style->anim, false);
        return ;
    }

    gt_anim_st anim;
    gt_anim_init(&anim);
    gt_anim_set_target(&anim, obj);
    gt_anim_set_ready_cb(&anim, _anim_ready_cb);
    gt_anim_set_time(&anim, time);
    gt_anim_set_repeat_count(&anim, GT_ANIM_REPEAT_INFINITE);
    style->anim = gt_anim_start(&anim);
}

GT_ATTRIBUTE_RAM_TEXT
static void _lrc_correction_time(gt_obj_st* obj, uint32_t time)
{
    _gt_lrc_st* style = (_gt_lrc_st*)obj;
    if(!style->anim) {
        return ;
    }

    uint32_t index = 0;
    for(uint32_t i = 0; i < style->times_count; ++i) {
        if(time >= style->times[i]) {
            index = i;
            continue;
        }

        if(time < style->times[i]) {
            break;
        }
    }

    gt_anim_set_paused(style->anim, true);
    gt_lrc_set_select_index(obj, index);
    index = index + 1;
    if(index >= style->times_count) {
        return ;
    }

    uint32_t pt = style->times[index] - time;
    gt_anim_set_time(style->anim, pt);
    gt_anim_set_paused(style->anim, false);
}

/* global functions / API interface -------------------------------------*/
GT_ATTRIBUTE_RAM_TEXT
gt_obj_st * gt_lrc_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    obj->fixed = false;

    _gt_lrc_st* style = (_gt_lrc_st*)obj;
    gt_font_info_init(&style->font_info);
    return obj;
}

GT_ATTRIBUTE_RAM_TEXT
void gt_lrc_set_border_width(gt_obj_st * obj, uint8_t width)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_lrc_st* style = (_gt_lrc_st*)obj;
    style->border_width = width;
}

GT_ATTRIBUTE_RAM_TEXT
void gt_lrc_set_border_color(gt_obj_st * obj, gt_color_t color)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_lrc_st* style = (_gt_lrc_st*)obj;
    style->color_border = color;
}

GT_ATTRIBUTE_RAM_TEXT
gt_obj_st* gt_lrc_add_text(gt_obj_st* obj, uint32_t time, const char* text)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return NULL;
    }

    if(!text) {
        return NULL;
    }

    return _lrc_add_text(obj, time, text, strlen(text));
}

GT_ATTRIBUTE_RAM_TEXT
void gt_lrc_clear_all_text(gt_obj_st* obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_lrc_st* style = (_gt_lrc_st*)obj;
    style->times_count = 0;
    style->offset_h = 0;
    style->index = 0;
    _gt_obj_class_destroy_children(obj);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

GT_ATTRIBUTE_RAM_TEXT
void gt_lrc_start(gt_obj_st* obj)
{
    if(false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _lrc_start_anim(obj);
}

GT_ATTRIBUTE_RAM_TEXT
void gt_lrc_paused(gt_obj_st* obj)
{
    if(false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }

    _gt_lrc_st* style = (_gt_lrc_st*)obj;

    if(style->anim) {
        gt_anim_set_paused(style->anim, true);
    }
}

GT_ATTRIBUTE_RAM_TEXT
void gt_lrc_resume(gt_obj_st* obj)
{
    if(false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }

    _gt_lrc_st* style = (_gt_lrc_st*)obj;

    if(style->anim) {
        gt_anim_set_paused(style->anim, false);
    }
}

GT_ATTRIBUTE_RAM_TEXT
void gt_lrc_set_select_color(gt_obj_st * obj, gt_color_t color)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_lrc_st* style = (_gt_lrc_st*)obj;
    style->select_color = color;
}

GT_ATTRIBUTE_RAM_TEXT
void gt_lrc_set_select_index(gt_obj_st* obj, uint32_t index)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }

    _gt_lrc_st* style = (_gt_lrc_st*)obj;
    if(index >= style->times_count || index >= obj->cnt_child) {
        return ;
    }

    if(style->index >= 0) {
        gt_label_set_font_color(obj->child[style->index], style->font_info.palette);
    }

    style->index = index;

    if(style->anim && false == style->anim->paused) {
        _lrc_start_anim(obj);
    }
    else{
        gt_label_set_font_color(obj->child[style->index], style->select_color);
        _scroll_to_child(obj, obj->child[style->index]);
    }
}

GT_ATTRIBUTE_RAM_TEXT
void gt_lrc_set_fixed_line(gt_obj_st* obj, uint32_t line)
{
    if(false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }

    _gt_lrc_st* style = (_gt_lrc_st*)obj;
    style->fixed_line = line;
}

GT_ATTRIBUTE_RAM_TEXT
void gt_lrc_correction_time(gt_obj_st* obj, uint32_t time)
{
    if(false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }

    _gt_lrc_st* style = (_gt_lrc_st*)obj;
    if(style->anim) {
        _lrc_correction_time(obj, time);
    }
}

GT_ATTRIBUTE_RAM_TEXT
void gt_lrc_set_space(gt_obj_st * obj, uint8_t space_x, uint8_t space_y)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_lrc_st* style = (_gt_lrc_st*)obj;
    style->space_x = space_x;
    style->space_y = space_y;

    for(uint32_t i = 0; i < obj->cnt_child; ++i) {
        gt_label_set_space(obj->child[i], style->space_x, style->space_y);
    }
}

GT_ATTRIBUTE_RAM_TEXT
void gt_lrc_set_font_color(gt_obj_st* obj, gt_color_t color)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_lrc_st* style = (_gt_lrc_st*)obj;
    style->font_info.palette = color;
    for(uint32_t i = 0; i < obj->cnt_child; ++i) {
        gt_label_set_font_color(obj->child[i], style->font_info.palette);
    }
}

GT_ATTRIBUTE_RAM_TEXT
void gt_lrc_set_font_size(gt_obj_st * obj, uint8_t size)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_lrc_st* style = (_gt_lrc_st*)obj;
    style->font_info.size = size;
    for(uint32_t i = 0; i < obj->cnt_child; ++i) {
        gt_label_set_font_size(obj->child[i], style->font_info.size);
    }
}

GT_ATTRIBUTE_RAM_TEXT
void gt_lrc_set_font_gray(gt_obj_st * obj, uint8_t gray)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_lrc_st* style = (_gt_lrc_st*)obj;
    style->font_info.gray = gray;
    for(uint32_t i = 0; i < obj->cnt_child; ++i) {
        gt_label_set_font_gray(obj->child[i], style->font_info.gray);
    }
}

GT_ATTRIBUTE_RAM_TEXT
void gt_lrc_set_font_thick_en(gt_obj_st * obj, uint8_t thick)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_lrc_st* style = (_gt_lrc_st*)obj;
    style->font_info.thick_en = thick;
    for(uint32_t i = 0; i < obj->cnt_child; ++i) {
        gt_label_set_font_thick_en(obj->child[i], style->font_info.thick_en);
    }
}

GT_ATTRIBUTE_RAM_TEXT
void gt_lrc_set_font_thick_cn(gt_obj_st * obj, uint8_t thick)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_lrc_st* style = (_gt_lrc_st*)obj;
    style->font_info.thick_cn = thick;
    for(uint32_t i = 0; i < obj->cnt_child; ++i) {
        gt_label_set_font_thick_cn(obj->child[i], style->font_info.thick_cn);
    }
}

GT_ATTRIBUTE_RAM_TEXT
void gt_lrc_set_font_encoding(gt_obj_st * obj, gt_encoding_et encoding)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_lrc_st* style = (_gt_lrc_st*)obj;
    style->font_info.encoding = encoding;
    for(uint32_t i = 0; i < obj->cnt_child; ++i) {
        gt_label_set_font_encoding(obj->child[i], style->font_info.encoding);
    }
}

#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
GT_ATTRIBUTE_RAM_TEXT
void gt_lrc_set_font_family_cn(gt_obj_st* obj, gt_family_t family)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_lrc_st* style = (_gt_lrc_st*)obj;
    style->font_info.family_cn = family;
    for(uint32_t i = 0; i < obj->cnt_child; ++i) {
        gt_label_set_font_family_cn(obj->child[i], style->font_info.family_cn);
    }
}

GT_ATTRIBUTE_RAM_TEXT
void gt_lrc_set_font_family_en(gt_obj_st* obj, gt_family_t family)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_lrc_st* style = (_gt_lrc_st*)obj;
    style->font_info.family_en = family;
    for(uint32_t i = 0; i < obj->cnt_child; ++i) {
        gt_label_set_font_family_en(obj->child[i], style->font_info.family_en);
    }
}

GT_ATTRIBUTE_RAM_TEXT
void gt_lrc_set_font_family_fl(gt_obj_st* obj, gt_family_t family)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_lrc_st* style = (_gt_lrc_st*)obj;
    style->font_info.family_fl = family;
    for(uint32_t i = 0; i < obj->cnt_child; ++i) {
        gt_label_set_font_family_fl(obj->child[i], style->font_info.family_fl);
    }
}

GT_ATTRIBUTE_RAM_TEXT
void gt_lrc_set_font_family_numb(gt_obj_st* obj, gt_family_t family)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_lrc_st* style = (_gt_lrc_st*)obj;
    style->font_info.family_numb = family;
    for(uint32_t i = 0; i < obj->cnt_child; ++i) {
        gt_label_set_font_family_numb(obj->child[i], style->font_info.family_numb);
    }
}
#else

GT_ATTRIBUTE_RAM_TEXT
void gt_lrc_set_font_family(gt_obj_st* obj, gt_family_t family)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_lrc_st* style = (_gt_lrc_st*)obj;
    gt_font_set_family(&style->font_info, family);
    for(uint32_t i = 0; i < obj->cnt_child; ++i) {
        gt_label_set_font_family(obj->child[i], style->font_info.family);
    }
}

GT_ATTRIBUTE_RAM_TEXT
void gt_lrc_set_font_cjk(gt_obj_st* obj, gt_font_cjk_et cjk)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }
    _gt_lrc_st* style = (_gt_lrc_st*)obj;
    style->font_info.cjk = cjk;
    for(uint32_t i = 0; i < obj->cnt_child; ++i) {
        gt_label_set_font_cjk(obj->child[i], style->font_info.cjk);
    }
}

#endif


/* end of file ----------------------------------------------------------*/
#endif // !GT_CFG_ENABLE_LRC

