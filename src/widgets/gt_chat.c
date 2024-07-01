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
#include "../core/gt_obj_pos.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../core/gt_mem.h"
#include "../core/gt_obj_scroll.h"
#include "../others/gt_log.h"
#include "../others/gt_anim.h"
#include "../widgets/gt_label.h"
#include "../widgets/gt_btn.h"
#include "../font/gt_symbol.h"
#include "../font/gt_font.h"
/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_CHAT
#define MY_CLASS    &gt_chat_class

#define _MSG_SPACE_Y    (8)
#define _MSG_SPACE_X    (8)
#define _MSG_MAX_W(w)   (w - (w >> 2) - _MSG_SPACE_X)

/* private typedef ------------------------------------------------------*/
typedef struct _gt_chat_param_s{
    gt_obj_st* msg_obj;
    uint8_t tim_s;
    uint8_t type : 3;
    uint8_t is_send : 1;
}_gt_chat_param_st;

typedef struct _gt_chat_s {
    gt_obj_st obj;

    gt_anim_st msg_anim;

    _gt_chat_param_st msg_list[GT_CHAT_MAX_MSG_CNT];
    gt_font_info_st font_info;

    gt_color_t bgcolor;
    gt_size_t paly_idx;
    uint16_t max_h;
    uint8_t msg_cnt;
    uint8_t msg_select_idx;

    uint8_t anim_play : 1;
}_gt_chat_st;

typedef enum{
    _SET_MSG_BG_COLOR = 0,
    _SET_MSG_BORDER_COLOR,
    _SET_MSG_BORDER_W,
    _SET_MSG_FONT_COLOR,
    _SET_MSG_FONT_STYLE_CN,
    _SET_MSG_FONT_STYLE_EN,
    _SET_MSG_FONT_STYLE_FL,
    _SET_MSG_FONT_STYLE_NUMB,
    _SET_MSG_FONT_SIZE,
    _SET_MSG_FONT_GRAY,
    _SET_MSG_FONT_THICK_EN,
    _SET_MSG_FONT_THICK_CN,
}_gt_chat_set_msg_te;

/* static prototypes ----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

static void _gt_chat_add_msg(gt_obj_st * chat, const char * msg, uint8_t tim_s, gt_chat_type_te type, bool is_send);
static void _gt_chat_update_msg_size(gt_obj_st * chat);
static void _scrolling_handler(gt_obj_st * obj);

static void _msg_click_cb(gt_event_st * e);
static void _msg_anim_start_cb(struct gt_anim_s * anim);
static void _msg_anim_ready_cb(struct gt_anim_s * anim);


/* static variables -----------------------------------------------------*/

const gt_obj_class_st gt_chat_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = NULL,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_chat_st)
};


/* macros ---------------------------------------------------------------*/

/* static functions -----------------------------------------------------*/

static void _init_cb(gt_obj_st * obj) {


    if(!obj || 0 == obj->area.w || 0 == obj->area.h) return;

    _gt_chat_st * style = (_gt_chat_st * )obj;
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill   = true;
    rect_attr.radius        = 0;
    rect_attr.bg_opa        = obj->opa;
    rect_attr.border_width  = 0;
    rect_attr.bg_color      = style->bgcolor;

    draw_bg(obj->draw_ctx, &rect_attr, &obj->area);
}


static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et type = gt_event_get_code(e);
    switch (type) {
    case GT_EVENT_TYPE_DRAW_START: {
        gt_disp_invalid_area(obj);
        break;
    }

    case GT_EVENT_TYPE_INPUT_SCROLL:
        GT_LOGV(GT_LOG_TAG_GUI, "scroll");
        _scrolling_handler(obj);
        break;

    default:
        break;
    }
}

static void _gt_set_voice_text(gt_obj_st* obj, uint8_t tim_s, bool is_send, uint8_t start) {
    // @GT_SYMBOL_VOICE_PLAY
    const char left_voice_icon[3][4] = {GT_SYMBOL_VOICE_PLAY_LEFT_1,GT_SYMBOL_VOICE_PLAY_LEFT_2,GT_SYMBOL_VOICE_PLAY_LEFT_3};
    const char right_voice_icon[3][4] = {GT_SYMBOL_VOICE_PLAY_RIGHT_1,GT_SYMBOL_VOICE_PLAY_RIGHT_2,GT_SYMBOL_VOICE_PLAY_RIGHT_3};
    if(is_send){
        gt_btn_set_text(obj, "%d\" %s", tim_s, right_voice_icon[start % GT_SYMBOL_VOICE_PLAY]);
    }
    else{
        gt_btn_set_text(obj, "%s %d\"", left_voice_icon[start % GT_SYMBOL_VOICE_PLAY], tim_s);
    }
}

static void _gt_set_msg_font_info_all(gt_obj_st * msg_obj, gt_font_info_st * font_info) {
    gt_btn_set_font_family_cn(msg_obj, font_info->style_cn);
    gt_btn_set_font_family_en(msg_obj, font_info->style_en);
    gt_btn_set_font_family_fl(msg_obj, font_info->style_fl);
    gt_btn_set_font_family_numb(msg_obj, font_info->style_numb);
    gt_btn_set_font_gray(msg_obj, font_info->gray);
    gt_btn_set_font_size(msg_obj, font_info->size);
    gt_btn_set_font_thick_cn(msg_obj, font_info->thick_cn);
    gt_btn_set_font_thick_en(msg_obj, font_info->thick_en);
}

static uint16_t _gt_update_msg_area_idx(gt_obj_st* chat, uint8_t idx) {
    _gt_chat_st * style = (_gt_chat_st * )chat;
    uint32_t w = 0, h = 0;
    gt_size_t x, y;

    h = gt_font_split_line_numb(&style->font_info, gt_btn_get_text(style->msg_list[idx].msg_obj), _MSG_MAX_W(chat->area.w), 0, &w);

    if( h > 1){
        gt_btn_set_font_align(style->msg_list[idx].msg_obj, GT_ALIGN_LEFT_MID);
    }
    else{
        gt_btn_set_font_align(style->msg_list[idx].msg_obj, GT_ALIGN_CENTER_MID);
    }
    h = (h + 1) * (style->font_info.size);
    w += style->font_info.size;
    w = w > _MSG_MAX_W(chat->area.w)  ? _MSG_MAX_W(chat->area.w) + 4 : w;
    x = style->msg_list[idx].is_send ? chat->area.x + chat->area.w - w - _MSG_SPACE_X : chat->area.x + _MSG_SPACE_X;
    y =  idx <= 0 ? _MSG_SPACE_Y + chat->area.y : style->msg_list[idx - 1].msg_obj->area.y + style->msg_list[idx - 1].msg_obj->area.h + _MSG_SPACE_Y;

    style->msg_list[idx].msg_obj->area.x = x;
    style->msg_list[idx].msg_obj->area.y = y;
    style->msg_list[idx].msg_obj->area.w = w;
    style->msg_list[idx].msg_obj->area.h = h;

    return (h + _MSG_SPACE_Y);
}

static void _gt_chat_msg_move_y(gt_obj_st * chat, gt_size_t offset_y) {
    _gt_chat_st * style = (_gt_chat_st * )chat;

    if(offset_y > 0) return;

    gt_size_t y = offset_y + chat->area.y + _MSG_SPACE_Y;

    for(int i = 0; i < style->msg_cnt; i++)
    {
        style->msg_list[i].msg_obj->area.y = y;
        y += style->msg_list[i].msg_obj->area.h + _MSG_SPACE_Y;

        if(GT_STATE_PRESSED == gt_obj_get_state(style->msg_list[i].msg_obj)){
            gt_obj_set_state(style->msg_list[i].msg_obj, GT_STATE_NONE);
        }
    }
}

static void _gt_chat_add_msg(gt_obj_st * chat, const char * msg, uint8_t tim_s, gt_chat_type_te type, bool is_send) {
    _gt_chat_st * style = (_gt_chat_st * )chat;

    if(style->msg_cnt >= GT_CHAT_MAX_MSG_CNT){
        style->max_h -= gt_obj_get_h(style->msg_list[0].msg_obj);
        _gt_obj_class_destroy(style->msg_list[0].msg_obj);
        gt_memmove (style->msg_list, style->msg_list + 1, sizeof(_gt_chat_param_st) * (GT_CHAT_MAX_MSG_CNT-1));
        style->msg_cnt = GT_CHAT_MAX_MSG_CNT - 1;
    }
    style->msg_list[style->msg_cnt].tim_s = tim_s;
    style->msg_list[style->msg_cnt].type = type;
    style->msg_list[style->msg_cnt].is_send = is_send;
    style->msg_list[style->msg_cnt].msg_obj = gt_btn_create(chat);

    gt_obj_set_inside(style->msg_list[style->msg_cnt].msg_obj, true);
    gt_obj_set_fixed(style->msg_list[style->msg_cnt].msg_obj, true);
    gt_obj_add_event_cb(style->msg_list[style->msg_cnt].msg_obj, _msg_click_cb, GT_EVENT_TYPE_INPUT_RELEASED, chat);
    _gt_set_msg_font_info_all(style->msg_list[style->msg_cnt].msg_obj, &style->font_info);

    if(GT_CHAT_TYPE_TEXT == type ){
        gt_btn_set_text(style->msg_list[style->msg_cnt].msg_obj, msg);
    }
    else if(GT_CHAT_TYPE_VOICE == type){
        _gt_set_voice_text(style->msg_list[style->msg_cnt].msg_obj, tim_s, is_send, GT_SYMBOL_VOICE_PLAY-1);
    }

    style->max_h += _gt_update_msg_area_idx(chat, style->msg_cnt);
    style->msg_cnt++;

    chat->process_attr.scroll.y = chat->area.h - style->max_h;
    _gt_chat_msg_move_y(chat, chat->process_attr.scroll.y);
}

static void _scrolling_handler(gt_obj_st * obj) {
    _gt_chat_st * style = (_gt_chat_st * )obj;
    gt_size_t bottom = gt_obj_get_limit_bottom(obj) + (-_MSG_SPACE_Y * 2);

    if (gt_obj_scroll_get_y(obj) > 0) {
        obj->process_attr.scroll.y = 0;
    }
    else if (gt_obj_scroll_get_y(obj) < bottom) {
        obj->process_attr.scroll.y = bottom;
    }
    _gt_chat_msg_move_y(obj, gt_obj_scroll_get_y(obj));

    gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
}

static void _gt_chat_update_msg_size(gt_obj_st * chat) {
    _gt_chat_st * style = (_gt_chat_st * )chat;
    uint32_t w = 0, h = 0;
    uint16_t max_h = _MSG_SPACE_Y;
    for(int i = 0; i < style->msg_cnt; i++)
    {
        max_h += _gt_update_msg_area_idx(chat, style->msg_cnt);
    }
    style->max_h = max_h;
}

static void _msg_click_cb(gt_event_st * e) {
    gt_obj_st * chat = e->user_data;
    _gt_chat_st * style = (_gt_chat_st * )chat;

    for(int i = 0; i < style->msg_cnt; i++){
        if(style->msg_list[i].msg_obj == e->target){
            style->msg_select_idx = i;
        }
    }
    //
    if(style->anim_play && GT_CHAT_TYPE_VOICE == style->msg_list[style->msg_select_idx].type)
    {
        if(style->paly_idx == style->msg_select_idx){
            return ;
        }

        if(style->paly_idx >= 0 && GT_CHAT_TYPE_VOICE == style->msg_list[style->paly_idx].type){
            _gt_set_voice_text( style->msg_list[style->paly_idx].msg_obj, \
                        style->msg_list[style->paly_idx].tim_s, \
                        style->msg_list[style->paly_idx].is_send, GT_SYMBOL_VOICE_PLAY - 1);
        }
    }
    //
    if(GT_CHAT_TYPE_VOICE == style->msg_list[style->msg_select_idx].type){
        style->paly_idx = style->msg_select_idx;
        gt_anim_set_time(&style->msg_anim, style->msg_list[style->paly_idx].tim_s * 1000);
        gt_anim_set_value(&style->msg_anim, 0, style->msg_list[style->paly_idx].tim_s * 3);
        gt_anim_start(&style->msg_anim);
        gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
    }
}

static void _msg_anim_start_cb(struct gt_anim_s * anim) {
    _gt_chat_st * style = (_gt_chat_st * )anim->tar;
    style->anim_play = true;
    if(style->paly_idx >= 0 && GT_CHAT_TYPE_VOICE == style->msg_list[style->paly_idx].type){
        gt_event_send((gt_obj_st * )anim->tar, GT_EVENT_TYPE_INPUT_PLAY_START, &style->paly_idx);
    }
}

static void _msg_anim_ready_cb(struct gt_anim_s * anim) {
    _gt_chat_st * style = (_gt_chat_st * )anim->tar;
    style->anim_play = false;

    if(style->paly_idx >= 0 && GT_CHAT_TYPE_VOICE == style->msg_list[style->paly_idx].type){
        _gt_set_voice_text( style->msg_list[style->paly_idx].msg_obj, \
                            style->msg_list[style->paly_idx].tim_s, \
                            style->msg_list[style->paly_idx].is_send, GT_SYMBOL_VOICE_PLAY - 1);
        gt_event_send((gt_obj_st * )anim->tar, GT_EVENT_TYPE_INPUT_PLAY_END, &style->paly_idx);
    }
    style->paly_idx = -1;
}

static void _msg_anim_exec_cb(void * obj, int32_t count) {
    _gt_chat_st * style = (_gt_chat_st * )obj;
    if(style->paly_idx >= 0 && GT_CHAT_TYPE_VOICE == style->msg_list[style->paly_idx].type){
        _gt_set_voice_text( style->msg_list[style->paly_idx].msg_obj, \
                            style->msg_list[style->paly_idx].tim_s, \
                            style->msg_list[style->paly_idx].is_send, count);
    }
}

static void _gt_set_msg_param(gt_obj_st* chat, _gt_chat_set_msg_te set_type, gt_color_t color, uint16_t border_w) {
    _gt_chat_st * style = (_gt_chat_st * )chat;
    for (int i = 0; i < style->msg_cnt; i++) {

        switch (set_type) {
        case _SET_MSG_BG_COLOR:
            gt_btn_set_color_background(style->msg_list[i].msg_obj, color);
            break;
        case _SET_MSG_BORDER_COLOR:
            gt_btn_set_color_border(style->msg_list[i].msg_obj, color);
            break;
        case _SET_MSG_BORDER_W:
            gt_btn_set_border_width(style->msg_list[i].msg_obj, border_w);
            break;
        case _SET_MSG_FONT_COLOR:
            gt_btn_set_font_color(style->msg_list[i].msg_obj, color);
            break;
        case _SET_MSG_FONT_STYLE_CN:
            gt_btn_set_font_family_cn(style->msg_list[i].msg_obj, style->font_info.style_cn);
            break;
        case _SET_MSG_FONT_STYLE_EN:
            gt_btn_set_font_family_en(style->msg_list[i].msg_obj, style->font_info.style_en);
            break;
        case _SET_MSG_FONT_STYLE_FL:
            gt_btn_set_font_family_fl(style->msg_list[i].msg_obj, style->font_info.style_fl);
            break;
        case _SET_MSG_FONT_STYLE_NUMB:
            gt_btn_set_font_family_numb(style->msg_list[i].msg_obj, style->font_info.style_numb);
            break;
        case _SET_MSG_FONT_SIZE:
            gt_btn_set_font_size(style->msg_list[i].msg_obj, style->font_info.size);
            break;
        case _SET_MSG_FONT_GRAY:
            gt_btn_set_font_gray(style->msg_list[i].msg_obj, style->font_info.gray);
            break;
        case _SET_MSG_FONT_THICK_EN:
            gt_btn_set_font_thick_en(style->msg_list[i].msg_obj, style->font_info.thick_en);
            break;
        case _SET_MSG_FONT_THICK_CN:
            gt_btn_set_font_thick_cn(style->msg_list[i].msg_obj, style->font_info.thick_cn);
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
    style->msg_cnt = 0;
    style->paly_idx = -1;
    style->max_h = _MSG_SPACE_Y;
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
    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->bgcolor = color;
}

void gt_chat_add_send_text_msg(gt_obj_st * chat, const char * msg)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_add_msg( chat, msg, 0, GT_CHAT_TYPE_TEXT, true);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_chat_add_send_voice_msg(gt_obj_st * chat, uint8_t tim_s)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_add_msg( chat, NULL, tim_s, GT_CHAT_TYPE_VOICE, true);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
}
//
void gt_chat_add_received_text_msg(gt_obj_st * chat, const char * msg)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_add_msg( chat, msg, 0, GT_CHAT_TYPE_TEXT, false);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_chat_add_received_voice_msg(gt_obj_st * chat, uint8_t tim_s)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_add_msg( chat, NULL, tim_s, GT_CHAT_TYPE_VOICE, false);
    gt_event_send(chat, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_chat_msg_set_color_background(gt_obj_st * chat, gt_color_t color)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_set_msg_param(chat, _SET_MSG_BG_COLOR, color, 0);
}

void gt_chat_msg_set_color_border(gt_obj_st * chat, gt_color_t color)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_set_msg_param(chat, _SET_MSG_BORDER_COLOR, color, 0);
}

void gt_chat_msg_set_border_width(gt_obj_st * chat, uint8_t width)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_set_msg_param(chat, _SET_MSG_BORDER_W, gt_color_hex(0), width);
}

void gt_chat_set_font_color(gt_obj_st * chat, gt_color_t color)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_set_msg_param(chat, _SET_MSG_FONT_COLOR, color, 0);
}

void gt_chat_set_font_size(gt_obj_st * chat, uint8_t size)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->font_info.size = size;
    _gt_set_msg_param(chat, _SET_MSG_FONT_SIZE, gt_color_hex(0), 0);
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
    _gt_set_msg_param(chat, _SET_MSG_FONT_GRAY, gt_color_hex(0), 0);
}

void gt_chat_set_font_family_cn(gt_obj_st * chat, gt_family_t font_family_cn)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }

    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->font_info.style_cn = font_family_cn;
    _gt_set_msg_param(chat, _SET_MSG_FONT_STYLE_CN, gt_color_hex(0), 0);
}

void gt_chat_set_font_family_en(gt_obj_st * chat, gt_family_t font_family_en)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }

    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->font_info.style_en = font_family_en;
    _gt_set_msg_param(chat, _SET_MSG_FONT_STYLE_EN, gt_color_hex(0), 0);
}

void gt_chat_set_font_family_fl(gt_obj_st * chat, gt_family_t font_family_fl)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }

    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->font_info.style_fl = font_family_fl;
    _gt_set_msg_param(chat, _SET_MSG_FONT_STYLE_FL, gt_color_hex(0), 0);
}

void gt_chat_set_font_family_numb(gt_obj_st * chat, gt_family_t font_family_numb)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }

    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->font_info.style_numb = font_family_numb;
    _gt_set_msg_param(chat, _SET_MSG_FONT_STYLE_NUMB, gt_color_hex(0), 0);
}

void gt_chat_set_font_thick_en(gt_obj_st * chat, uint8_t thick)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->font_info.thick_en = thick;
    _gt_set_msg_param(chat, _SET_MSG_FONT_THICK_EN, gt_color_hex(0), 0);
}

void gt_chat_set_font_thick_cn(gt_obj_st * chat, uint8_t thick)
{
    if (false == gt_obj_is_type(chat, OBJ_TYPE)) {
        return ;
    }
    _gt_chat_st * style = (_gt_chat_st * )chat;
    style->font_info.thick_cn = thick;
    _gt_set_msg_param(chat, _SET_MSG_FONT_THICK_CN, gt_color_hex(0), 0);
}

#endif /** GT_CFG_ENABLE_CHATTING */
/* end of file ----------------------------------------------------------*/


