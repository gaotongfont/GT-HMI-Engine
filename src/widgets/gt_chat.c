/**
 * @file gt_chat.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-03-19 14:32:13
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_chat.h"

#if GT_CFG_ENABLE_CHAT
#include "../hal/gt_hal_disp.h"
#include "../hal/gt_hal_indev.h"
#include "../core/gt_obj_pos.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../core/gt_mem.h"
#include "../core/gt_obj_scroll.h"
#include "../others/gt_log.h"
#include "../others/gt_anim.h"
#include "../widgets/gt_label.h"
#include "../font/gt_symbol.h"
#include "../font/gt_font.h"
/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_CHAT
#define MY_CLASS    &gt_chat_class

#define _MSG_SPACE_Y    (8)
#define _MSG_SPACE_X    (8)
#define _MSG_MAX_W(w)   (w - (w >> 2) - _MSG_SPACE_X)

#define _MULTI_LINE_CONTENT_INDENT_DEFAULT      (2)

/* private typedef ------------------------------------------------------*/
typedef struct _gt_chat_param_s{
    gt_obj_st* msg_obj;
    gt_obj_st* source_audio_obj;
    gt_obj_st* translation_audio_obj;
    char *source_text;
    char *translation_text;
    uint32_t message_id;
    uint8_t source_tim_s;
    uint8_t translation_tim_s;
    uint8_t tim_s;
    uint8_t type : 3;
    uint8_t is_send : 1;
    uint8_t has_translation : 1;
    uint8_t translation_expanded : 1;
}_gt_chat_param_st;

typedef struct _gt_chat_s {
    gt_obj_st base;
    gt_chat_item_cb_st item_cb;
    gt_anim_st msg_anim;
    gt_chat_play_event_st play_event;

    _gt_chat_param_st msg_list[GT_CHAT_MAX_MSG_CNT];
    gt_font_info_st font_info;

    gt_color_t send_bgcolor;
    gt_color_t send_border_color;
    gt_color_t send_font_color;
    gt_color_t recv_bgcolor;
    gt_color_t recv_border_color;
    gt_color_t recv_font_color;

    gt_size_t paly_idx;
    gt_size_t msg_select_idx;

    gt_radius_t msg_radius;

    uint16_t max_h;
    uint8_t msg_cnt;

    uint8_t send_border_w;
    uint8_t recv_border_w;
    uint8_t space_x;
    uint8_t space_y;

    uint8_t anim_play : 1;
    uint8_t play_segment : 1;
    uint32_t next_message_id;
}_gt_chat_st;

typedef enum{
    _SET_MSG_BG_COLOR = 0,
    _SET_MSG_BORDER_COLOR,
    _SET_MSG_BORDER_W,
    _SET_MSG_FONT_COLOR,
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
    _SET_MSG_FONT_STYLE_CN,
    _SET_MSG_FONT_STYLE_EN,
    _SET_MSG_FONT_STYLE_FL,
    _SET_MSG_FONT_STYLE_NUMB,
#else
    _SET_MSG_FONT_FAMILY,
    _SET_MSG_FONT_CJK,
#endif
    _SET_MSG_FONT_SIZE,
    _SET_MSG_FONT_GRAY,
    _SET_MSG_FONT_THICK_EN,
    _SET_MSG_FONT_THICK_CN,
    _SET_MSG_RADIUS,
    _SET_MSG_FONT_STYLE,
}_gt_chat_set_msg_te;

/* static prototypes ----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

static GT_ATTRIBUTE_RAM_TEXT gt_obj_st* _gt_chat_add_msg(gt_obj_st * chat, const char * msg, uint16_t msg_len, uint8_t tim_s, gt_chat_type_te type, bool is_send);
static GT_ATTRIBUTE_RAM_TEXT void _gt_chat_update_msg_size(gt_obj_st * chat);
static GT_ATTRIBUTE_RAM_TEXT void _scrolling_handler(gt_obj_st * obj);

static GT_ATTRIBUTE_RAM_TEXT void _msg_click_cb(gt_event_st * e);
static GT_ATTRIBUTE_RAM_TEXT void _msg_anim_start_cb(struct gt_anim_s * anim);
static GT_ATTRIBUTE_RAM_TEXT void _msg_anim_ready_cb(struct gt_anim_s * anim);
static GT_ATTRIBUTE_RAM_TEXT void _msg_anim_exec_cb(void * obj, int32_t count);
static GT_ATTRIBUTE_RAM_TEXT void _msg_segment_click_cb(gt_event_st * e);
static bool _gt_chat_start_message_play(_gt_chat_st * style, uint8_t index, gt_chat_play_segment_te segment);
static void _gt_chat_free_msg_text(_gt_chat_param_st * msg_p);
static bool _gt_chat_set_translation(_gt_chat_st * style, uint8_t index, const char * translation);
static void _gt_chat_render_translation(_gt_chat_st * style, uint8_t index, uint8_t frame);


/* static variables -----------------------------------------------------*/

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_chat_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_chat_st)
};


/* macros ---------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/
static void _chat_item_cb_copy(gt_chat_item_cb_st* dst, const gt_chat_item_cb_st* src)
{
    if(!dst || !src) return;
    gt_memcpy(dst, src, sizeof(gt_chat_item_cb_st));
}


static void _init_cb(gt_obj_st * obj) {
    if(!obj || 0 == obj->area.w || 0 == obj->area.h) return;

    if (obj->show_bg) {
        gt_attr_rect_st rect_attr;
        gt_graph_init_rect_attr(&rect_attr);
        rect_attr.reg.is_fill   = true;
        rect_attr.radius        = 0;
        rect_attr.bg_opa        = obj->opa;
        rect_attr.border_width  = 0;
        rect_attr.bg_color      = obj->bgcolor;

        draw_bg(obj->draw_ctx, &rect_attr, &obj->area);
    }
    draw_focus(obj);
}

static void _deinit_cb(gt_obj_st * obj)
{
    if (!obj) return;
    _gt_chat_st * style = (_gt_chat_st *)obj;
    gt_anim_del(style, _msg_anim_exec_cb);
    for (uint8_t i = 0; i < style->msg_cnt; i++) {
        _gt_chat_free_msg_text(&style->msg_list[i]);
    }
}


static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et type = gt_event_get_code(e);
    switch (type) {
    case GT_EVENT_TYPE_DRAW_START: {
        gt_disp_invalid_area(obj);
        break;
    }
    case GT_EVENT_TYPE_INPUT_SCROLL:
        _scrolling_handler(obj);
        break;
    case GT_EVENT_TYPE_INPUT_SCROLL_UP: {
        gt_obj_scroll_to_y(obj, gt_obj_get_h(obj) >> 1, GT_ANIM_ON);
        break;
    }
    case GT_EVENT_TYPE_INPUT_SCROLL_DOWN: {
        gt_obj_scroll_to_y(obj, - gt_obj_get_h(obj) >> 1, GT_ANIM_ON);
        break;
    }

    default:
        break;
    }
}

static GT_ATTRIBUTE_RAM_TEXT void _gt_set_voice_text(_gt_chat_st * style, gt_obj_st* obj, uint8_t tim_s, bool is_send, uint8_t start) {
    for (uint8_t i = 0; i < style->msg_cnt; i++) {
        if (style->msg_list[i].msg_obj == obj && style->msg_list[i].has_translation) {
            _gt_chat_render_translation(style, i, start);
            return;
        }
    }
    // @GT_SYMBOL_VOICE_PLAY
    const char left_voice_icon[3][4] = {GT_SYMBOL_VOICE_PLAY_LEFT_1,GT_SYMBOL_VOICE_PLAY_LEFT_2,GT_SYMBOL_VOICE_PLAY_LEFT_3};
    const char right_voice_icon[3][4] = {GT_SYMBOL_VOICE_PLAY_RIGHT_1,GT_SYMBOL_VOICE_PLAY_RIGHT_2,GT_SYMBOL_VOICE_PLAY_RIGHT_3};
    if(is_send){
        style->item_cb.set_text_cb(obj, "%d\" %s", tim_s, right_voice_icon[start % GT_SYMBOL_VOICE_PLAY]);
    }
    else{
        style->item_cb.set_text_cb(obj, "%s %d\"", left_voice_icon[start % GT_SYMBOL_VOICE_PLAY], tim_s);
    }
}

static GT_ATTRIBUTE_RAM_TEXT void _gt_set_msg_font_info_all(_gt_chat_st * style, gt_obj_st * msg_obj, gt_font_info_st * font_info) {
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
    style->item_cb.set_font_family_cn_cb(msg_obj, font_info->style_cn);
    style->item_cb.set_font_family_en_cb(msg_obj, font_info->style_en);
    style->item_cb.set_font_family_fl_cb(msg_obj, font_info->style_fl);
    style->item_cb.set_font_family_numb_cb(msg_obj, font_info->style_numb);
#else
    style->item_cb.set_font_family_cb(msg_obj, font_info->family);
    style->item_cb.set_font_cjk_cb(msg_obj, font_info->cjk);
#endif
    style->item_cb.set_font_gray_cb(msg_obj, font_info->gray);
    style->item_cb.set_font_size_cb(msg_obj, font_info->size);
    style->item_cb.set_font_thick_cn_cb(msg_obj, font_info->thick_cn);
    style->item_cb.set_font_thick_en_cb(msg_obj, font_info->thick_en);
    style->item_cb.set_font_encoding_cb(msg_obj, font_info->encoding);
    if(style->item_cb.set_font_style_cb) {
        style->item_cb.set_font_style_cb(msg_obj, font_info->style.all);
    }
}

static GT_ATTRIBUTE_RAM_TEXT uint16_t _gt_update_msg_area_idx(gt_obj_st* chat, uint8_t idx, bool is_send) {
    _gt_chat_st * style = (_gt_chat_st * )chat;
    uint32_t w = 0, h = 0;
    gt_size_t x, y;
    gt_radius_t radius = style->msg_radius;

    gt_font_split_line_st sp_line = {
        .text = NULL,
        .max_w = _MSG_MAX_W(chat->area.w) - w,
        .start_w =  _MSG_MAX_W(chat->area.w) - w,
        .space = style->space_x,
        .indent = _MULTI_LINE_CONTENT_INDENT_DEFAULT,
    };
    gt_split_line_ret_st s_line_ret = {0};
    _gt_chat_param_st * msg_p = &style->msg_list[idx];
    s_line_ret = style->item_cb.get_font_split_line_numb_cb(msg_p->msg_obj, &sp_line);
    h = s_line_ret.line_numb;
    w = s_line_ret.max_w;

    uint8_t border_w = is_send ? style->send_border_w : style->recv_border_w;
    uint8_t border_w2 = border_w << 1;
    uint16_t font_size = style->font_info.size;
    uint8_t space_y = style->space_y;

    /* Keep a real right/left safety margin. Without it the last Latin glyph
     * (for example the 'y' in "bye") can be clipped by one pixel. */
    w += 10 + border_w2;
    if (s_line_ret.line_numb > 1) {
        style->item_cb.set_font_align_cb(msg_p->msg_obj, GT_ALIGN_LEFT_MID);
        style->item_cb.set_indent_cb(msg_p->msg_obj, _MULTI_LINE_CONTENT_INDENT_DEFAULT);
        style->item_cb.set_space_cb(msg_p->msg_obj, style->space_x, space_y);
        h = (h + 1) * (font_size + space_y + 2) + border_w2 + 4;
        if(style->item_cb.set_start_point_offset_cb) {
            style->item_cb.set_start_point_offset_cb(msg_p->msg_obj, 0, 0);
        }
    } else {
        style->item_cb.set_font_align_cb(msg_p->msg_obj, GT_ALIGN_CENTER_MID);
        style->item_cb.set_indent_cb(msg_p->msg_obj, 0);
        style->item_cb.set_space_cb(msg_p->msg_obj, style->space_x, 0);
        h = (h + 1) * font_size + border_w2;
        if(style->item_cb.set_start_point_offset_cb) {
            style->item_cb.set_start_point_offset_cb(msg_p->msg_obj,
                                                    (gt_font_get_indent_width(&style->font_info, _MULTI_LINE_CONTENT_INDENT_DEFAULT) + (w - s_line_ret.max_w)) >> 1 ,
                                                    0);
        }
    }

    x = (msg_p->is_send ?  (chat->area.w - w - _MSG_SPACE_X) : _MSG_SPACE_X) + chat->area.x;
    y =  idx <= 0 ? _MSG_SPACE_Y + chat->area.y : style->msg_list[idx - 1].msg_obj->area.y + style->msg_list[idx - 1].msg_obj->area.h + _MSG_SPACE_Y;

    if(radius > GT_MIN(w >> 1, h >> 1)) {
        radius = GT_MIN(w >> 1, h >> 1);
    }

    gt_obj_set_radius(msg_p->msg_obj, radius);

    msg_p->msg_obj->area.x = x;
    msg_p->msg_obj->area.y = y;
    msg_p->msg_obj->area.w = w;
    msg_p->msg_obj->area.h = h;

    if (msg_p->has_translation && msg_p->translation_expanded) {
        /* A source row may wrap to several lines. Cover each logical half of
         * the bubble instead of assuming a fixed one-line source height. */
        uint16_t hit_w = 52;
        uint16_t source_h = h >> 1;
        if (msg_p->source_audio_obj) {
            msg_p->source_audio_obj->area.x = is_send ? (x + w - hit_w - 4) : (x + 4);
            msg_p->source_audio_obj->area.y = y;
            msg_p->source_audio_obj->area.w = hit_w;
            msg_p->source_audio_obj->area.h = source_h;
            gt_obj_set_pos(msg_p->source_audio_obj, msg_p->source_audio_obj->area.x, msg_p->source_audio_obj->area.y);
            gt_obj_set_size(msg_p->source_audio_obj, hit_w, source_h);
        }
        if (msg_p->translation_audio_obj) {
            msg_p->translation_audio_obj->area.x = is_send ? (x + w - hit_w - 4) : (x + 4);
            msg_p->translation_audio_obj->area.y = y + source_h;
            msg_p->translation_audio_obj->area.w = hit_w;
            msg_p->translation_audio_obj->area.h = h - source_h;
            gt_obj_set_pos(msg_p->translation_audio_obj, msg_p->translation_audio_obj->area.x, msg_p->translation_audio_obj->area.y);
            gt_obj_set_size(msg_p->translation_audio_obj, hit_w, h - source_h);
        }
    } else {
        if (msg_p->source_audio_obj) {
            uint16_t hit_w = 52;
            uint16_t hit_h = h;
            msg_p->source_audio_obj->area.x = is_send ? (x + w - hit_w - 4) : (x + 4);
            msg_p->source_audio_obj->area.y = y;
            msg_p->source_audio_obj->area.w = hit_w;
            msg_p->source_audio_obj->area.h = hit_h;
            gt_obj_set_pos(msg_p->source_audio_obj, msg_p->source_audio_obj->area.x, msg_p->source_audio_obj->area.y);
            gt_obj_set_size(msg_p->source_audio_obj, hit_w, hit_h);
        }
        if (msg_p->translation_audio_obj) {
            msg_p->translation_audio_obj->area.w = 0;
            msg_p->translation_audio_obj->area.h = 0;
        }
    }

    return (h + _MSG_SPACE_Y);
}

static GT_ATTRIBUTE_RAM_TEXT void _gt_chat_msg_move_y(gt_obj_st * chat, gt_size_t offset_y) {
    _gt_chat_st * style = (_gt_chat_st * )chat;

    if(offset_y > 0) return;

    gt_size_t prev_offset = gt_obj_get_scroll_prev_y(chat);
    gt_obj_set_scroll_prev(chat, gt_obj_get_scroll_prev_x(chat), offset_y);

    gt_size_t diff_y = offset_y - prev_offset;
    if (0 == diff_y) return;

    for(int i = 0; i < style->msg_cnt; i++)
    {
        gt_obj_set_pos(style->msg_list[i].msg_obj,
                       style->msg_list[i].msg_obj->area.x,
                       style->msg_list[i].msg_obj->area.y + diff_y);
        if (style->msg_list[i].source_audio_obj) {
            gt_obj_set_pos(style->msg_list[i].source_audio_obj,
                           style->msg_list[i].source_audio_obj->area.x,
                           style->msg_list[i].source_audio_obj->area.y + diff_y);
        }
        if (style->msg_list[i].translation_audio_obj) {
            gt_obj_set_pos(style->msg_list[i].translation_audio_obj,
                           style->msg_list[i].translation_audio_obj->area.x,
                           style->msg_list[i].translation_audio_obj->area.y + diff_y);
        }

        if(GT_STATE_PRESSED == gt_obj_get_state(style->msg_list[i].msg_obj)){
            gt_obj_set_state(style->msg_list[i].msg_obj, GT_STATE_NONE);
        }
    }
}

static GT_ATTRIBUTE_RAM_TEXT void _gt_chat_destroy_obj(gt_obj_st * obj)
{
    if (NULL == obj) {
        return;
    }
    if (_gt_obj_in_event_dispatch(obj)) {
        gt_obj_destroy(obj);
    } else {
        _gt_obj_class_destroy(obj);
    }
}

static GT_ATTRIBUTE_RAM_TEXT gt_obj_st*
_gt_chat_add_msg(gt_obj_st * chat, const char * msg, uint16_t msg_len, uint8_t tim_s, gt_chat_type_te type, bool is_send) {
    _gt_chat_st * style = (_gt_chat_st * )chat;

    if (style->msg_cnt >= GT_CHAT_MAX_MSG_CNT) {
        style->max_h -= (style->msg_list[0].msg_obj->area.h + _MSG_SPACE_Y);
        _gt_chat_free_msg_text(&style->msg_list[0]);
        _gt_chat_destroy_obj(style->msg_list[0].source_audio_obj);
        _gt_chat_destroy_obj(style->msg_list[0].translation_audio_obj);
        _gt_chat_destroy_obj(style->msg_list[0].msg_obj);
        gt_memmove (style->msg_list, style->msg_list + 1, sizeof(_gt_chat_param_st) * (GT_CHAT_MAX_MSG_CNT-1));
        style->msg_cnt = GT_CHAT_MAX_MSG_CNT - 1;
    }
    _gt_chat_param_st * msg_p = &style->msg_list[style->msg_cnt];
    msg_p->source_text = NULL;
    msg_p->translation_text = NULL;
    msg_p->source_audio_obj = NULL;
    msg_p->translation_audio_obj = NULL;
    msg_p->has_translation = 0;
    msg_p->message_id = style->next_message_id++;
    if (style->next_message_id == 0) style->next_message_id = 1;
    msg_p->tim_s = tim_s;
    msg_p->source_tim_s = tim_s;
    msg_p->translation_tim_s = tim_s;
    msg_p->type = type;
    msg_p->is_send = is_send;
    msg_p->msg_obj = style->item_cb.create_cb(chat);
    if (gt_obj_is_type(msg_p->msg_obj, GT_TYPE_UNKNOWN)) {
        return NULL;
    }

    gt_obj_set_inside(msg_p->msg_obj, true);
    gt_obj_set_touch_parent(msg_p->msg_obj, false);
    gt_obj_set_fixed(msg_p->msg_obj, true);
    gt_obj_add_event_cb(msg_p->msg_obj, _msg_click_cb, GT_EVENT_TYPE_INPUT_RELEASED, chat);
    _gt_set_msg_font_info_all(style, msg_p->msg_obj, &style->font_info);

    gt_color_t* bg_color = is_send ? &style->send_bgcolor : &style->recv_bgcolor;
    gt_color_t* border_color = is_send ? &style->send_border_color : &style->recv_border_color;
    uint8_t* border_w = is_send ? &style->send_border_w : &style->recv_border_w;
    gt_color_t* font_color = is_send ? &style->send_font_color : &style->recv_font_color;

    style->item_cb.set_color_background_cb(msg_p->msg_obj, *bg_color);
    style->item_cb.set_color_border_cb(msg_p->msg_obj, *border_color);
    style->item_cb.set_border_width_cb(msg_p->msg_obj, *border_w);
    style->item_cb.set_font_color_cb(msg_p->msg_obj, *font_color);

    if(GT_CHAT_TYPE_TEXT == type ){
        style->item_cb.set_text_by_len_cb(msg_p->msg_obj, msg, msg_len);
    }
    else if(GT_CHAT_TYPE_VOICE == type){
        _gt_set_voice_text(style, msg_p->msg_obj, tim_s, is_send, GT_SYMBOL_VOICE_PLAY-1);
    }

    style->max_h += _gt_update_msg_area_idx(chat, style->msg_cnt, is_send);
    style->msg_cnt++;

    chat->process_attr.scroll.y = chat->area.h - style->max_h;
    _gt_chat_msg_move_y(chat, chat->process_attr.scroll.y);

    return msg_p->msg_obj;
}

static void _gt_chat_free_msg_text(_gt_chat_param_st * msg_p)
{
    if (!msg_p) return;
    if (msg_p->source_text) gt_mem_free(msg_p->source_text);
    if (msg_p->translation_text) gt_mem_free(msg_p->translation_text);
    msg_p->source_text = NULL;
    msg_p->translation_text = NULL;
    msg_p->has_translation = 0;
}

static bool _gt_chat_set_translation(_gt_chat_st * style, uint8_t index, const char * translation)
{
    if (!style || index >= style->msg_cnt || !style->msg_list[index].msg_obj) return false;
    _gt_chat_param_st * msg_p = &style->msg_list[index];
    if (msg_p->type != GT_CHAT_TYPE_TEXT || !msg_p->source_text) return false;
    if (msg_p->translation_text) { gt_mem_free(msg_p->translation_text); msg_p->translation_text = NULL; }
    msg_p->has_translation = 0;
    if (translation && translation[0]) {
        uint32_t len = (uint32_t)strlen(translation);
        msg_p->translation_text = (char *)gt_mem_malloc(len + 1);
        if (!msg_p->translation_text) return false;
        gt_memcpy(msg_p->translation_text, translation, len);
        msg_p->translation_text[len] = '\0';
        msg_p->has_translation = 1;
        msg_p->translation_expanded = 1;
    }
    _gt_chat_render_translation(style, index, GT_SYMBOL_VOICE_PLAY - 1);
    return true;
}

static void _gt_chat_render_translation(_gt_chat_st * style, uint8_t index, uint8_t frame)
{
    _gt_chat_param_st * msg_p = &style->msg_list[index];
    {
        const char * sep = "\n----------------\n";
        const char * icons[3] = { msg_p->is_send ? GT_SYMBOL_VOICE_PLAY_RIGHT_1 : GT_SYMBOL_VOICE_PLAY_LEFT_1,
                                  msg_p->is_send ? GT_SYMBOL_VOICE_PLAY_RIGHT_2 : GT_SYMBOL_VOICE_PLAY_LEFT_2,
                                  msg_p->is_send ? GT_SYMBOL_VOICE_PLAY_RIGHT_3 : GT_SYMBOL_VOICE_PLAY_LEFT_3 };
        uint8_t active_frame = (style->anim_play && style->play_segment == GT_CHAT_PLAY_TRANSLATION) ? frame : (GT_SYMBOL_VOICE_PLAY - 1);
        const char * icon = icons[active_frame % GT_SYMBOL_VOICE_PLAY];
        const char * source_icon = icons[(style->anim_play && style->play_segment == GT_CHAT_PLAY_SOURCE) ? (frame % GT_SYMBOL_VOICE_PLAY) : (GT_SYMBOL_VOICE_PLAY - 1)];
        uint32_t sl = (uint32_t)strlen(msg_p->source_text);
        uint32_t tl = msg_p->translation_text ? (uint32_t)strlen(msg_p->translation_text) : 0;
        uint32_t il = (uint32_t)strlen(icon) + 1;
        bool show_translation = msg_p->has_translation && msg_p->translation_expanded;
        uint32_t total = il + sl + (show_translation ? (uint32_t)strlen(sep) + il + tl : 0);
        char * text = (char *)gt_mem_malloc(total + 1);
        if (!text) return;
        /* Both source and translation are playable. The source icon is
         * repeated in the static and animated frame, so both rows visibly
         * track the same playback state. */
        gt_memcpy(text, source_icon, il - 1);
        text[il - 1] = ' ';
        gt_memcpy(text + il, msg_p->source_text, sl);
        if (show_translation) {
            uint32_t off = il + sl;
            gt_memcpy(text + off, sep, strlen(sep));
            off += (uint32_t)strlen(sep);
            gt_memcpy(text + off, icon, il - 1);
            text[off + il - 1] = ' ';
            gt_memcpy(text + off + il, msg_p->translation_text, tl);
        }
        text[total] = '\0';
        style->item_cb.set_text_by_len_cb(msg_p->msg_obj, text, total);
        gt_mem_free(text);
    }
}

static GT_ATTRIBUTE_RAM_TEXT gt_obj_st *
_gt_chat_append_msg(gt_obj_st* chat, const char* msg, bool is_scroll)
{
    _gt_chat_st * style = (_gt_chat_st * )chat;

    int16_t index = style->msg_cnt - 1;
    if(index < 0) return NULL;

    if(GT_CHAT_TYPE_TEXT != style->msg_list[index].type) return NULL;

    gt_obj_st * obj = style->msg_list[index].msg_obj;

    if(style->item_cb.append_text_cb){
        style->item_cb.append_text_cb(obj, msg);
    }

    style->max_h -= (obj->area.h + _MSG_SPACE_Y);
    style->max_h += _gt_update_msg_area_idx(chat, index, style->msg_list[index].is_send);
    if(is_scroll) {
        chat->process_attr.scroll.y = chat->area.h - style->max_h;
        _gt_chat_msg_move_y(chat, chat->process_attr.scroll.y);
    }
    return obj;
}

static GT_ATTRIBUTE_RAM_TEXT gt_obj_st *
_gt_chat_replace_msg(gt_obj_st* chat, const char* msg, bool is_scroll)
{
    _gt_chat_st * style = (_gt_chat_st * )chat;

    int16_t index = style->msg_cnt - 1;
    if(index < 0) return NULL;

    if(GT_CHAT_TYPE_TEXT != style->msg_list[index].type) return NULL;

    gt_obj_st * obj = style->msg_list[index].msg_obj;

    style->item_cb.set_text_cb(obj, msg);

    style->max_h -= (obj->area.h + _MSG_SPACE_Y);
    style->max_h += _gt_update_msg_area_idx(chat, index, style->msg_list[index].is_send);
    if(is_scroll) {
        chat->process_attr.scroll.y = chat->area.h - style->max_h;
        _gt_chat_msg_move_y(chat, chat->process_attr.scroll.y);
    }
    return obj;
}

static gt_obj_st * GT_ATTRIBUTE_RAM_TEXT
_gt_chat_replace_msg_by_len(gt_obj_st* chat, const char* msg, uint16_t msg_len, bool is_scroll)
{
    _gt_chat_st * style = (_gt_chat_st * )chat;

    int16_t index = style->msg_cnt - 1;
    if(index < 0) return NULL;

    if(GT_CHAT_TYPE_TEXT != style->msg_list[index].type) return NULL;

    gt_obj_st * obj = style->msg_list[index].msg_obj;

    style->item_cb.set_text_by_len_cb(obj, msg, msg_len);

    style->max_h -= (obj->area.h + _MSG_SPACE_Y);
    style->max_h += _gt_update_msg_area_idx(chat, index, style->msg_list[index].is_send);
    if(is_scroll) {
        chat->process_attr.scroll.y = chat->area.h - style->max_h;
        _gt_chat_msg_move_y(chat, chat->process_attr.scroll.y);
    }
    return obj;
}
static GT_ATTRIBUTE_RAM_TEXT void _scrolling_handler(gt_obj_st * obj) {

    _gt_obj_scroll_internal(obj, 0, (_MSG_SPACE_Y << 1));
}

static GT_ATTRIBUTE_RAM_TEXT void _gt_chat_update_msg_size(gt_obj_st * chat) {
    _gt_chat_st * style = (_gt_chat_st * )chat;
    uint16_t max_h = _MSG_SPACE_Y;
    for(int i = 0; i < style->msg_cnt; i++)
    {
        max_h += _gt_update_msg_area_idx(chat, i, style->msg_list[i].is_send);
    }
    style->max_h = max_h;
}

static GT_ATTRIBUTE_RAM_TEXT void _gt_chat_relayout(gt_obj_st * chat, bool scroll_to_end)
{
    _gt_chat_st * style = (_gt_chat_st *)chat;
    gt_obj_set_scroll_prev(chat, 0, 0);
    for (uint8_t i = 0; i < style->msg_cnt; i++) {
        style->msg_list[i].msg_obj->area.y = chat->area.y;
    }
    _gt_chat_update_msg_size(chat);
    chat->process_attr.scroll.y = scroll_to_end ? (chat->area.h - style->max_h) : 0;
    _gt_chat_msg_move_y(chat, chat->process_attr.scroll.y);
}

static GT_ATTRIBUTE_RAM_TEXT void _msg_click_cb(gt_event_st * e) {
    gt_obj_st * chat = e->user_data;
    _gt_chat_st * style = (_gt_chat_st * )chat;
    bool found = false;

    for(int i = 0; i < style->msg_cnt; i++){
        if(style->msg_list[i].msg_obj == e->target){
            style->msg_select_idx = i;
            found = true;
            break;
        }
    }
    if (!found) return;

    /* Some input ports resolve a child hit to the parent button. Use the
     * release coordinates as a robust fallback so the speaker hotspot still
     * wins over the bubble toggle action. */
    if (e->param) {
        const gt_indev_param_st * indev_param = (const gt_indev_param_st *)e->param;
        gt_point_st p = indev_param->param.point;
        _gt_chat_param_st * msg = &style->msg_list[style->msg_select_idx];
        if (msg->has_translation && msg->translation_expanded &&
            p.y >= msg->msg_obj->area.y && p.y < msg->msg_obj->area.y + msg->msg_obj->area.h) {
            bool near_speaker = msg->is_send
                ? (p.x >= msg->msg_obj->area.x + msg->msg_obj->area.w - 52)
                : (p.x <= msg->msg_obj->area.x + 52);
            if (near_speaker) {
                gt_chat_play_segment_te segment =
                    (p.y < msg->msg_obj->area.y + (msg->msg_obj->area.h >> 1))
                    ? GT_CHAT_PLAY_SOURCE : GT_CHAT_PLAY_TRANSLATION;
                _gt_chat_start_message_play(style, (uint8_t)style->msg_select_idx, segment);
                return;
            }
        }
        if (msg->source_audio_obj && p.x >= msg->source_audio_obj->area.x &&
            p.x < msg->source_audio_obj->area.x + msg->source_audio_obj->area.w &&
            p.y >= msg->source_audio_obj->area.y &&
            p.y < msg->source_audio_obj->area.y + msg->source_audio_obj->area.h) {
            _gt_chat_start_message_play(style, (uint8_t)style->msg_select_idx, GT_CHAT_PLAY_SOURCE);
            return;
        }
        if (msg->translation_audio_obj && p.x >= msg->translation_audio_obj->area.x &&
            p.x < msg->translation_audio_obj->area.x + msg->translation_audio_obj->area.w &&
            p.y >= msg->translation_audio_obj->area.y &&
            p.y < msg->translation_audio_obj->area.y + msg->translation_audio_obj->area.h) {
            _gt_chat_start_message_play(style, (uint8_t)style->msg_select_idx, GT_CHAT_PLAY_TRANSLATION);
            return;
        }
    }

    if (style->msg_select_idx >= 0 && style->msg_list[style->msg_select_idx].has_translation) {
        _gt_chat_param_st * msg = &style->msg_list[style->msg_select_idx];
        gt_anim_del(style, _msg_anim_exec_cb);
        style->anim_play = false;
        style->paly_idx = -1;
        msg->translation_expanded = !msg->translation_expanded;
        _gt_chat_render_translation(style, (uint8_t)style->msg_select_idx, GT_SYMBOL_VOICE_PLAY - 1);
        _gt_chat_relayout(chat, true);
        gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
        return;
    }

    if(style->anim_play && (GT_CHAT_TYPE_VOICE == style->msg_list[style->msg_select_idx].type || style->msg_list[style->msg_select_idx].has_translation))
    {
        if(style->paly_idx == style->msg_select_idx){
            return ;
        }

        if(style->paly_idx >= 0 && (GT_CHAT_TYPE_VOICE == style->msg_list[style->paly_idx].type || style->msg_list[style->paly_idx].has_translation)){
            _gt_set_voice_text( style, \
                                style->msg_list[style->paly_idx].msg_obj, \
                                style->msg_list[style->paly_idx].tim_s, \
                                style->msg_list[style->paly_idx].is_send, GT_SYMBOL_VOICE_PLAY - 1);
        }
    }
    if(GT_CHAT_TYPE_VOICE == style->msg_list[style->msg_select_idx].type || style->msg_list[style->msg_select_idx].has_translation){
        style->play_segment = GT_CHAT_PLAY_SOURCE;
        style->paly_idx = style->msg_select_idx;
        gt_anim_set_time(&style->msg_anim, style->msg_list[style->paly_idx].tim_s * 1000);
        gt_anim_set_value(&style->msg_anim, 0, style->msg_list[style->paly_idx].tim_s * 3);
        gt_anim_start(&style->msg_anim);
        gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
    }
}

static GT_ATTRIBUTE_RAM_TEXT void _msg_segment_click_cb(gt_event_st * e)
{
    gt_obj_st * chat = e->user_data;
    _gt_chat_st * style = (_gt_chat_st *)chat;
    for (uint8_t i = 0; i < style->msg_cnt; i++) {
        if (style->msg_list[i].source_audio_obj == e->target) {
            _gt_chat_start_message_play(style, i, GT_CHAT_PLAY_SOURCE);
            return;
        }
        if (style->msg_list[i].translation_audio_obj == e->target) {
            _gt_chat_start_message_play(style, i, GT_CHAT_PLAY_TRANSLATION);
            return;
        }
    }
}

static GT_ATTRIBUTE_RAM_TEXT void _msg_anim_start_cb(struct gt_anim_s * anim) {
    _gt_chat_st * style = (_gt_chat_st * )anim->tar;
    style->anim_play = true;
    if(style->paly_idx >= 0 && (GT_CHAT_TYPE_VOICE == style->msg_list[style->paly_idx].type || style->msg_list[style->paly_idx].has_translation)){
        style->play_event.index = (uint8_t)style->paly_idx;
        style->play_event.message_id = style->msg_list[style->paly_idx].message_id;
        style->play_event.segment = (gt_chat_play_segment_te)style->play_segment;
        gt_event_send((gt_obj_st * )anim->tar, GT_EVENT_TYPE_INPUT_PLAY_START, &style->play_event);
    }
}

static GT_ATTRIBUTE_RAM_TEXT void _msg_anim_ready_cb(struct gt_anim_s * anim) {
    _gt_chat_st * style = (_gt_chat_st * )anim->tar;
    style->anim_play = false;

    if(style->paly_idx >= 0 && (GT_CHAT_TYPE_VOICE == style->msg_list[style->paly_idx].type || style->msg_list[style->paly_idx].has_translation)){
        _gt_set_voice_text( style,\
                            style->msg_list[style->paly_idx].msg_obj, \
                            style->msg_list[style->paly_idx].tim_s, \
                            style->msg_list[style->paly_idx].is_send, GT_SYMBOL_VOICE_PLAY - 1);
        style->play_event.index = (uint8_t)style->paly_idx;
        style->play_event.message_id = style->msg_list[style->paly_idx].message_id;
        style->play_event.segment = (gt_chat_play_segment_te)style->play_segment;
        gt_event_send((gt_obj_st * )anim->tar, GT_EVENT_TYPE_INPUT_PLAY_END, &style->play_event);
    }
    style->paly_idx = -1;
}

static GT_ATTRIBUTE_RAM_TEXT void _msg_anim_exec_cb(void * obj, int32_t count) {
    _gt_chat_st * style = (_gt_chat_st * )obj;
    if(style->paly_idx >= 0 && (GT_CHAT_TYPE_VOICE == style->msg_list[style->paly_idx].type || style->msg_list[style->paly_idx].has_translation)){
        _gt_set_voice_text( style, \
                            style->msg_list[style->paly_idx].msg_obj, \
                            style->msg_list[style->paly_idx].tim_s, \
                            style->msg_list[style->paly_idx].is_send, count);
    }
}

static int32_t _gt_chat_find_message_id(_gt_chat_st * style, uint32_t message_id)
{
    if (!style || !message_id) return -1;
    for (int32_t i = 0; i < style->msg_cnt; i++) {
        if (style->msg_list[i].message_id == message_id) return i;
    }
    return -1;
}

static bool _gt_chat_start_message_play(_gt_chat_st * style, uint8_t index,
                                        gt_chat_play_segment_te segment)
{
    if (!style || index >= style->msg_cnt) return false;
    _gt_chat_param_st * msg = &style->msg_list[index];
    if (segment == GT_CHAT_PLAY_TRANSLATION && !msg->has_translation) return false;
    if (msg->type != GT_CHAT_TYPE_VOICE && !msg->has_translation) return false;
    gt_anim_del(style, _msg_anim_exec_cb);
    style->msg_select_idx = index;
    style->paly_idx = index;
    style->play_segment = segment;
    style->anim_play = false;
    gt_anim_set_time(&style->msg_anim, (msg->tim_s ? msg->tim_s : 1) * 1000);
    {
        uint8_t play_time = (segment == GT_CHAT_PLAY_TRANSLATION) ? msg->translation_tim_s : msg->source_tim_s;
        if (!play_time) play_time = msg->tim_s;
        if (!play_time) play_time = 1;
        gt_anim_set_time(&style->msg_anim, play_time * 1000);
        gt_anim_set_value(&style->msg_anim, 0, play_time * 3);
    }
    gt_anim_start(&style->msg_anim);
    gt_event_send((gt_obj_st *)style, GT_EVENT_TYPE_DRAW_START, NULL);
    return true;
}

static GT_ATTRIBUTE_RAM_TEXT void _gt_set_msg_param(gt_obj_st* chat, _gt_chat_set_msg_te set_type) {
    _gt_chat_st * style = (_gt_chat_st * )chat;
    for (int i = 0; i < style->msg_cnt; i++) {

        switch (set_type) {
        case _SET_MSG_BG_COLOR:
            style->item_cb.set_color_background_cb(style->msg_list[i].msg_obj, \
                                    style->msg_list[i].is_send ? style->send_bgcolor : style->recv_bgcolor);
            break;
        case _SET_MSG_BORDER_COLOR:
            style->item_cb.set_color_border_cb(style->msg_list[i].msg_obj, \
                                    style->msg_list[i].is_send ? style->send_border_color : style->recv_border_color);
            break;
        case _SET_MSG_BORDER_W:
            style->item_cb.set_border_width_cb(style->msg_list[i].msg_obj, \
                                    style->msg_list[i].is_send ? style->send_border_w : style->recv_border_w);
            break;
        case _SET_MSG_FONT_COLOR:
            style->item_cb.set_font_color_cb(style->msg_list[i].msg_obj, \
                                    style->msg_list[i].is_send ? style->send_font_color : style->recv_font_color);
            break;
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
        case _SET_MSG_FONT_STYLE_CN:
            style->item_cb.set_font_family_cn_cb(style->msg_list[i].msg_obj, style->font_info.style_cn);
            break;
        case _SET_MSG_FONT_STYLE_EN:
            style->item_cb.set_font_family_en_cb(style->msg_list[i].msg_obj, style->font_info.style_en);
            break;
        case _SET_MSG_FONT_STYLE_FL:
            style->item_cb.set_font_family_fl_cb(style->msg_list[i].msg_obj, style->font_info.style_fl);
            break;
        case _SET_MSG_FONT_STYLE_NUMB:
            style->item_cb.set_font_family_numb_cb(style->msg_list[i].msg_obj, style->font_info.style_numb);
            break;
#else
        case _SET_MSG_FONT_FAMILY:
            style->item_cb.set_font_family_cb(style->msg_list[i].msg_obj, style->font_info.family);
            break;
        case _SET_MSG_FONT_CJK:
            style->item_cb.set_font_cjk_cb(style->msg_list[i].msg_obj, style->font_info.cjk);
            break;
#endif
        case _SET_MSG_FONT_SIZE:
            style->item_cb.set_font_size_cb(style->msg_list[i].msg_obj, style->font_info.size);
            break;
        case _SET_MSG_FONT_GRAY:
            style->item_cb.set_font_gray_cb(style->msg_list[i].msg_obj, style->font_info.gray);
            break;
        case _SET_MSG_FONT_THICK_EN:
            style->item_cb.set_font_thick_en_cb(style->msg_list[i].msg_obj, style->font_info.thick_en);
            break;
        case _SET_MSG_FONT_THICK_CN:
            style->item_cb.set_font_thick_cn_cb(style->msg_list[i].msg_obj, style->font_info.thick_cn);
            break;
        case _SET_MSG_RADIUS:
            gt_obj_set_radius(style->msg_list[i].msg_obj, style->msg_radius);
            break;
        case _SET_MSG_FONT_STYLE:
            if(style->item_cb.set_font_style_cb){
                style->item_cb.set_font_style_cb(style->msg_list[i].msg_obj, style->font_info.style.all);
            }
            break;
        default:
            break;
        }
    }
}

/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_chat_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    _gt_chat_st * style = (_gt_chat_st * )obj;

    gt_obj_set_scroll_dir(obj, GT_SCROLL_VERTICAL);

    //set default style
    GT_CHAT_BTN_ITEM_CB_INIT(style->item_cb);

    style->msg_cnt = 0;
    style->paly_idx = -1;
    style->max_h = _MSG_SPACE_Y;
    style->next_message_id = 1;
    gt_memset_0(style->msg_list, sizeof(style->msg_list));
    gt_font_info_init(&style->font_info);
    gt_obj_set_fixed(obj, false);

    gt_anim_init(&style->msg_anim);
    gt_anim_set_target(&style->msg_anim, obj);
    gt_anim_set_start_cb(&style->msg_anim, _msg_anim_start_cb);
    gt_anim_set_ready_cb(&style->msg_anim, _msg_anim_ready_cb);
    gt_anim_set_exec_cb(&style->msg_anim, _msg_anim_exec_cb);

    return obj;
}

void gt_chat_set_bg_color(gt_obj_st * chat, gt_color_t color)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    gt_obj_set_bgcolor(chat, color);
    gt_obj_show_bg(chat, true);
}

gt_obj_st * gt_chat_add_send_text_msg(gt_obj_st * chat, const char * msg)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return NULL;
    }
    gt_obj_st * item = _gt_chat_add_msg( chat, msg, msg ? strlen(msg) : 0, 0, GT_CHAT_TYPE_TEXT, true);
    if (NULL == item) {
        return NULL;
    }
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
    return item;
}

gt_obj_st * gt_chat_add_send_text_msg_by_len(gt_obj_st * chat, const char * msg, uint16_t len)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return NULL;
    }
    gt_obj_st * item = _gt_chat_add_msg( chat, msg, len, 0, GT_CHAT_TYPE_TEXT, true);
    if (NULL == item) {
        return NULL;
    }
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
    return item;
}

gt_obj_st * gt_chat_add_send_voice_msg(gt_obj_st * chat, uint8_t tim_s)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return NULL;
    }
    gt_obj_st * item = _gt_chat_add_msg( chat, NULL, 0, tim_s, GT_CHAT_TYPE_VOICE, true);
    if (NULL == item) {
        return NULL;
    }
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
    return item;
}
//
gt_obj_st * gt_chat_add_received_text_msg(gt_obj_st * chat, const char * msg)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return NULL;
    }
    gt_obj_st * item = _gt_chat_add_msg( chat, msg, msg ? strlen(msg) : 0, 0, GT_CHAT_TYPE_TEXT, false);
    if (NULL == item) {
        return NULL;
    }
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
    return item;
}

gt_obj_st * gt_chat_add_received_text_msg_by_len(gt_obj_st * chat, const char * msg, uint16_t len)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return NULL;
    }
    gt_obj_st * item = _gt_chat_add_msg( chat, msg, len, 0, GT_CHAT_TYPE_TEXT, false);
    if (NULL == item) {
        return NULL;
    }
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
    return item;
}

gt_obj_st * gt_chat_add_received_voice_msg(gt_obj_st * chat, uint8_t tim_s)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return NULL;
    }
    gt_obj_st * item = _gt_chat_add_msg( chat, NULL, 0, tim_s, GT_CHAT_TYPE_VOICE, false);
    if (NULL == item) {
        return NULL;
    }
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
    return item;
}

static gt_obj_st * _gt_chat_add_translated(gt_obj_st * chat, const char * source,
                                           const char * translation, uint8_t tim_s, bool is_send)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE) || !source) return NULL;
    gt_obj_st * item = _gt_chat_add_msg(chat, source, (uint16_t)strlen(source), tim_s,
                                        GT_CHAT_TYPE_TEXT, is_send);
    if (!item) return NULL;
    _gt_chat_st * style = (_gt_chat_st *)chat;
    uint8_t index = (uint8_t)(style->msg_cnt - 1);
    _gt_chat_param_st * msg_p;
    uint32_t len = (uint32_t)strlen(source);
    style->msg_list[index].source_text = (char *)gt_mem_malloc(len + 1);
    if (!style->msg_list[index].source_text) return item;
    gt_memcpy(style->msg_list[index].source_text, source, len);
    style->msg_list[index].source_text[len] = '\0';
    _gt_chat_set_translation(style, index, translation);
    msg_p = &style->msg_list[index];
    /* Invisible hit-test objects: do not use gt_btn here, because a btn
     * paints its pressed background and would cover the bubble. */
    msg_p->source_audio_obj = gt_obj_create(chat);
    msg_p->translation_audio_obj = gt_obj_create(chat);
    if (msg_p->source_audio_obj) {
        msg_p->source_audio_obj->show_bg = false;
        gt_obj_set_inside(msg_p->source_audio_obj, true);
        gt_obj_set_touch_parent(msg_p->source_audio_obj, false);
        gt_obj_set_fixed(msg_p->source_audio_obj, true);
        gt_obj_add_event_cb(msg_p->source_audio_obj, _msg_segment_click_cb, GT_EVENT_TYPE_INPUT_RELEASED, chat);
    }
    if (msg_p->translation_audio_obj) {
        msg_p->translation_audio_obj->show_bg = false;
        gt_obj_set_inside(msg_p->translation_audio_obj, true);
        gt_obj_set_touch_parent(msg_p->translation_audio_obj, false);
        gt_obj_set_fixed(msg_p->translation_audio_obj, true);
        gt_obj_add_event_cb(msg_p->translation_audio_obj, _msg_segment_click_cb, GT_EVENT_TYPE_INPUT_RELEASED, chat);
    }
    _gt_chat_update_msg_size(chat);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
    return item;
}

gt_obj_st * gt_chat_add_send_translated_msg(gt_obj_st * chat, const char * source, const char * translation)
{
    return _gt_chat_add_translated(chat, source, translation, 3, true);
}

gt_obj_st * gt_chat_add_received_translated_msg(gt_obj_st * chat, const char * source, const char * translation)
{
    return _gt_chat_add_translated(chat, source, translation, 3, false);
}

gt_obj_st * gt_chat_add_send_translated_msg_with_time(gt_obj_st * chat, const char * source, const char * translation, uint8_t tim_s)
{
    return _gt_chat_add_translated(chat, source, translation, tim_s, true);
}

gt_obj_st * gt_chat_add_received_translated_msg_with_time(gt_obj_st * chat, const char * source, const char * translation, uint8_t tim_s)
{
    return _gt_chat_add_translated(chat, source, translation, tim_s, false);
}

gt_obj_st * gt_chat_set_last_msg_translation(gt_obj_st * chat, const char * translation)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) return NULL;
    _gt_chat_st * style = (_gt_chat_st *)chat;
    if (!style->msg_cnt) return NULL;
    uint8_t index = (uint8_t)(style->msg_cnt - 1);
    if (!style->msg_list[index].source_text) {
        return NULL;
    }
    if (!_gt_chat_set_translation(style, index, translation)) return NULL;
    _gt_chat_update_msg_size(chat);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
    return style->msg_list[index].msg_obj;
}

bool gt_chat_set_message_translation(gt_obj_st * chat, uint32_t message_id, const char * translation)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) return false;
    _gt_chat_st * style = (_gt_chat_st *)chat;
    int32_t index = _gt_chat_find_message_id(style, message_id);
    if (index < 0 || !_gt_chat_set_translation(style, (uint8_t)index, translation)) return false;
    _gt_chat_relayout(chat, true);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
    return true;
}

void gt_chat_set_last_msg_audio_time(gt_obj_st * chat, uint8_t tim_s)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) return;
    _gt_chat_st * style = (_gt_chat_st *)chat;
    if (!style->msg_cnt) return;
    style->msg_list[style->msg_cnt - 1].tim_s = tim_s;
    style->msg_list[style->msg_cnt - 1].source_tim_s = tim_s;
    style->msg_list[style->msg_cnt - 1].translation_tim_s = tim_s;
}

bool gt_chat_set_message_audio_time(gt_obj_st * chat, uint32_t message_id, uint8_t tim_s)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) return false;
    _gt_chat_st * style = (_gt_chat_st *)chat;
    int32_t index = _gt_chat_find_message_id(style, message_id);
    if (index < 0) return false;
    style->msg_list[index].tim_s = tim_s;
    style->msg_list[index].source_tim_s = tim_s;
    style->msg_list[index].translation_tim_s = tim_s;
    return true;
}

bool gt_chat_set_message_segment_audio_time(gt_obj_st * chat, uint32_t message_id,
                                            gt_chat_play_segment_te segment, uint8_t tim_s)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) return false;
    _gt_chat_st * style = (_gt_chat_st *)chat;
    int32_t index = _gt_chat_find_message_id(style, message_id);
    if (index < 0) return false;
    if (segment == GT_CHAT_PLAY_TRANSLATION) style->msg_list[index].translation_tim_s = tim_s;
    else style->msg_list[index].source_tim_s = tim_s;
    return true;
}

uint32_t gt_chat_get_last_msg_id(gt_obj_st * chat)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) return 0;
    _gt_chat_st * style = (_gt_chat_st *)chat;
    return style->msg_cnt ? style->msg_list[style->msg_cnt - 1].message_id : 0;
}

bool gt_chat_play_message_segment(gt_obj_st * chat, uint32_t message_id, gt_chat_play_segment_te segment)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) return false;
    _gt_chat_st * style = (_gt_chat_st *)chat;
    int32_t index = _gt_chat_find_message_id(style, message_id);
    return index >= 0 ? _gt_chat_start_message_play(style, (uint8_t)index, segment) : false;
}

void gt_chat_stop_message_playback(gt_obj_st * chat)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) return;
    _gt_chat_st * style = (_gt_chat_st *)chat;
    /* A manual stop must notify the audio owner just like natural animation
     * completion, so it can stop/flush the active decoder immediately. */
    bool was_playing = style->paly_idx >= 0 && style->paly_idx < style->msg_cnt;
    if (was_playing) {
        style->play_event.index = (int32_t)style->paly_idx;
        style->play_event.message_id = style->msg_list[style->paly_idx].message_id;
        style->play_event.segment = (gt_chat_play_segment_te)style->play_segment;
    }
    gt_anim_del(style, _msg_anim_exec_cb);
    style->anim_play = false;
    style->paly_idx = -1;
    style->play_segment = GT_CHAT_PLAY_SOURCE;
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
    if (was_playing) {
        gt_event_send(chat, GT_EVENT_TYPE_INPUT_PLAY_END, &style->play_event);
    }
}

gt_obj_st * gt_chat_append_msg_text(gt_obj_st * chat, const char * msg)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return NULL;
    }
    gt_obj_st * item = _gt_chat_append_msg(chat, msg, true);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
    return item;
}

gt_obj_st * gt_chat_append_msg_text_control_scroll(gt_obj_st * chat, const char * msg, bool is_scroll)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return NULL;
    }
    gt_obj_st * item = _gt_chat_append_msg(chat, msg, is_scroll);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
    return item;
}

gt_chat_msg_type_te gt_chat_get_last_msg_type(gt_obj_st * chat)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return GT_CHAT_MSG_TYPE_NONE;
    }
    _gt_chat_st * style = (_gt_chat_st * )chat;
    if(style->msg_cnt <= 0) {
        return GT_CHAT_MSG_TYPE_NONE;
    }
    return style->msg_list[style->msg_cnt - 1].is_send ? GT_CHAT_MSG_TYPE_SEND : GT_CHAT_MSG_TYPE_RECV;
}

gt_obj_st * gt_chat_replace_msg_text(gt_obj_st * chat, const char * msg)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return NULL;
    }
    gt_obj_st * item = _gt_chat_replace_msg(chat, msg, true);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
    return item;
}

gt_obj_st * gt_chat_replace_msg_text_control_scroll(gt_obj_st * chat, const char * msg, bool is_scroll)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return NULL;
    }
    gt_obj_st * item = _gt_chat_replace_msg(chat, msg, is_scroll);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
    return item;
}

gt_obj_st * gt_chat_replace_msg_text_by_len(gt_obj_st * chat, const char * msg, uint16_t len)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return NULL;
    }
    gt_obj_st * item = _gt_chat_replace_msg_by_len(chat, msg, len, true);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
    return item;
}

gt_obj_st * gt_chat_replace_msg_text_by_len_control_scroll(gt_obj_st * chat, const char * msg, uint16_t len, bool is_scroll)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return NULL;
    }
    gt_obj_st * item = _gt_chat_replace_msg_by_len(chat, msg, len, is_scroll);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
    return item;
}
void gt_chat_update_item_size(gt_obj_st * chat, bool is_scroll)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }

    _gt_chat_st * style = (_gt_chat_st * )chat;

    int16_t index = style->msg_cnt - 1;
    if(index < 0) return ;

    if(GT_CHAT_TYPE_TEXT != style->msg_list[index].type) return ;

    gt_obj_st * obj = style->msg_list[index].msg_obj;

    style->max_h -= (obj->area.h + _MSG_SPACE_Y);
    style->max_h += _gt_update_msg_area_idx(chat, index, style->msg_list[index].is_send);
    if(is_scroll) {
        chat->process_attr.scroll.y = chat->area.h - style->max_h;
        _gt_chat_msg_move_y(chat, chat->process_attr.scroll.y);
    }
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_chat_set_send_msg_bg_color(gt_obj_st * chat, gt_color_t color)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->send_bgcolor = color;
    _gt_set_msg_param(chat, _SET_MSG_BG_COLOR);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);

}

void gt_chat_set_received_msg_bg_color(gt_obj_st * chat, gt_color_t color)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->recv_bgcolor = color;
    _gt_set_msg_param(chat, _SET_MSG_BG_COLOR);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
}
void gt_chat_set_send_msg_border_color(gt_obj_st * chat, gt_color_t color)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->send_border_color = color;
    _gt_set_msg_param(chat, _SET_MSG_BORDER_COLOR);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
}
void gt_chat_set_received_msg_border_color(gt_obj_st * chat, gt_color_t color)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->recv_border_color = color;
    _gt_set_msg_param(chat, _SET_MSG_BORDER_COLOR);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
}
void gt_chat_set_send_msg_border_width(gt_obj_st * chat, uint8_t width)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->send_border_w = width;
    _gt_set_msg_param(chat, _SET_MSG_BORDER_W);
    _gt_chat_update_msg_size(chat);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
}
void gt_chat_set_received_msg_border_width(gt_obj_st * chat, uint8_t width)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->recv_border_w = width;
    _gt_set_msg_param(chat, _SET_MSG_BORDER_W);
    _gt_chat_update_msg_size(chat);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
}
void gt_chat_set_send_msg_font_color(gt_obj_st * chat, gt_color_t color)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->send_font_color = color;
    _gt_set_msg_param(chat, _SET_MSG_FONT_COLOR);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
}
void gt_chat_set_received_msg_font_color(gt_obj_st * chat, gt_color_t color)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->recv_font_color = color;
    _gt_set_msg_param(chat, _SET_MSG_FONT_COLOR);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
}


void gt_chat_set_font_size(gt_obj_st * chat, gt_font_size_t size)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->font_info.size = size;
    _gt_set_msg_param(chat, _SET_MSG_FONT_SIZE);
    _gt_chat_update_msg_size(chat);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_chat_set_font_gray(gt_obj_st * chat, uint8_t gray)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->font_info.gray = gray;
    _gt_set_msg_param(chat, _SET_MSG_FONT_GRAY);
}

void gt_chat_set_font_scale_size(gt_obj_st * chat, gt_font_size_t scale_size)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->font_info.scale_size = scale_size;
    _gt_set_msg_param(chat, _SET_MSG_FONT_SIZE);
}

#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
void gt_chat_set_font_family_cn(gt_obj_st * chat, gt_family_t font_family_cn)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }

    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->font_info.style_cn = font_family_cn;
    _gt_set_msg_param(chat, _SET_MSG_FONT_STYLE_CN);
}

void gt_chat_set_font_family_en(gt_obj_st * chat, gt_family_t font_family_en)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }

    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->font_info.style_en = font_family_en;
    _gt_set_msg_param(chat, _SET_MSG_FONT_STYLE_EN);
}

void gt_chat_set_font_family_fl(gt_obj_st * chat, gt_family_t font_family_fl)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }

    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->font_info.style_fl = font_family_fl;
    _gt_set_msg_param(chat, _SET_MSG_FONT_STYLE_FL);
}

void gt_chat_set_font_family_numb(gt_obj_st * chat, gt_family_t font_family_numb)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }

    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->font_info.style_numb = font_family_numb;
    _gt_set_msg_param(chat, _SET_MSG_FONT_STYLE_NUMB);
}
#else
void gt_chat_set_font_family(gt_obj_st * chat, gt_family_t font_family)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_st * style = (_gt_chat_st * )chat;
    gt_font_set_family(&style->font_info, font_family);
    _gt_set_msg_param(chat, _SET_MSG_FONT_FAMILY);
    gt_chat_set_font_size(chat, style->font_info.size);
}
void gt_chat_set_font_cjk(gt_obj_st* chat, gt_font_cjk_et cjk)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->font_info.cjk = cjk;
    _gt_set_msg_param(chat, _SET_MSG_FONT_CJK);
}
#endif
void gt_chat_set_font_thick_en(gt_obj_st * chat, uint8_t thick)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->font_info.thick_en = thick;
    _gt_set_msg_param(chat, _SET_MSG_FONT_THICK_EN);
}

void gt_chat_set_font_thick_cn(gt_obj_st * chat, uint8_t thick)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->font_info.thick_cn = thick;
    _gt_set_msg_param(chat, _SET_MSG_FONT_THICK_CN);
}

void gt_chat_set_font_style(gt_obj_st * chat, gt_font_style_et font_style)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->font_info.style.all = font_style;
    _gt_set_msg_param(chat, _SET_MSG_FONT_STYLE);
}

void gt_chat_clean_all_msg(gt_obj_st * chat)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }

    chat->process_attr.scroll.x = 0;
    chat->process_attr.scroll.y = 0;
    gt_obj_set_scroll_prev(chat, 0, 0);
    _gt_chat_st * style = (_gt_chat_st * )chat;

    gt_anim_del(style, _msg_anim_exec_cb);
    for (int i = 0; i < style->msg_cnt; i++) _gt_chat_free_msg_text(&style->msg_list[i]);
    style->msg_cnt = 0;
    style->max_h = 0;
    style->msg_select_idx = -1;
    style->paly_idx = -1;
    gt_memset_0(style->msg_list, sizeof(style->msg_list));

    _gt_obj_class_destroy_children(chat);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_chat_set_msg_radius(gt_obj_st * chat, gt_radius_t radius)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->msg_radius = radius;
    _gt_set_msg_param(chat, _SET_MSG_RADIUS);
    _gt_chat_update_msg_size(chat);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);

}

void gt_chat_set_msg_space(gt_obj_st * chat, uint8_t space_x, uint8_t space_y)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->space_x = space_x;
    style->space_y = space_y;
    _gt_chat_update_msg_size(chat);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_chat_set_item_cb(gt_obj_st * chat, gt_chat_item_cb_st* cb)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_st * style = (_gt_chat_st * )chat;

    if(0 != style->msg_cnt){
        return ;
    }

    if(cb) {
        _chat_item_cb_copy(&style->item_cb, cb);
    }
}

int32_t gt_chat_get_item_index(gt_obj_st* chat, gt_obj_st* item)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return -1;
    }

    _gt_chat_st * style = (_gt_chat_st * )chat;

    for(int32_t i = 0; i < style->msg_cnt; i++) {
        if(item == style->msg_list[i].msg_obj) {
            return i;
        }
    }

    return -1;
}

/* end of file ----------------------------------------------------------*/
#endif /** GT_CFG_ENABLE_CHATTING */
