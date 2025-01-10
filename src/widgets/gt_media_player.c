/**
 * @file gt_media_player.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-08-23 11:10:16
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_media_player.h"

#if GT_CFG_ENABLE_MEDIA_PLAYER
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../core/gt_fs.h"
#include "../font/gt_font.h"
#include "string.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../core/gt_event.h"
#include "../font/gt_symbol.h"
#include "../others/gt_anim.h"
#include "./gt_label.h"



/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_MEDIA_PLAYER
#define MY_CLASS    &gt_media_player_class

#define _SLIDER_PADDING_HOR         20
#define _SLIDER_PADDING_VER         10

/* private typedef ------------------------------------------------------*/
typedef struct {
    uint8_t playing : 1; /** 0[default]: stop; 1: playing */
    uint8_t hide_title : 1; /** 0[default]: show; 1: hide */
}_media_player_reg_st;

typedef struct {
    gt_media_player_handler_cb media_cb;
    void * user_data;
}_media_player_cb_st;

typedef struct _gt_media_player_s {
    gt_obj_st obj;
    gt_obj_st * media_p;
    gt_obj_st * slider_p;
    gt_obj_st * status_p;

    _media_player_cb_st play_hdr;
    _media_player_cb_st stop_hdr;

    _media_player_reg_st reg;
}_gt_media_player_st;

/* static variables -----------------------------------------------------*/
static void _media_player_init_cb(gt_obj_st * obj);
static void _media_player_deinit_cb(gt_obj_st * obj);
static void _media_player_event_cb(struct gt_obj_s * obj, gt_event_st * e);

static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_media_player_class = {
    ._init_cb      = _media_player_init_cb,
    ._deinit_cb    = _media_player_deinit_cb,
    ._event_cb     = _media_player_event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_media_player_st)
};



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void _hide_center_content(gt_obj_st * media) {
    _gt_media_player_st * style = (_gt_media_player_st *)media;
    if (NULL == style->status_p) {
        return;
    }
    gt_obj_set_visible(style->status_p, GT_INVISIBLE);
}

static void _show_center_content(gt_obj_st * media, gt_size_t font_size, char const * const text) {
    _gt_media_player_st * style = (_gt_media_player_st *)media;
    gt_obj_st * status_p = style->status_p;
    if (NULL == status_p) {
        return;
    }
    status_p->area = media->area;
    gt_label_set_text(status_p, (const char * )text);
    gt_label_set_font_size(status_p, font_size);
    gt_label_set_font_color(status_p, gt_color_white());
    gt_label_set_font_align(status_p, GT_ALIGN_CENTER_MID);
    gt_obj_set_visible(status_p, GT_VISIBLE);
}

static void _show_slider_bar_exec_cb(void * t, int32_t val) {
    gt_obj_st * obj = (gt_obj_st *)t;
    gt_obj_set_opa(obj, val);
}

static void _update_slider_bar_pos_size(gt_obj_st * obj) {
    _gt_media_player_st * style = (_gt_media_player_st *)obj;

    gt_obj_st * slp = style->slider_p;
    if (slp->area.w + _SLIDER_PADDING_HOR == obj->area.w) {
        return;
    }
    gt_obj_set_pos_relative(slp, obj, _SLIDER_PADDING_HOR >> 1, obj->area.h - slp->area.h - _SLIDER_PADDING_VER);
    gt_obj_set_size(slp, obj->area.w - _SLIDER_PADDING_HOR, slp->area.h);
}

static void _show_slider_bar_start_cb(gt_anim_st * anim) {
    gt_obj_st * obj = (gt_obj_st *)anim->tar;
    gt_obj_set_visible(obj, GT_VISIBLE);
}

static void _show_slider_bar_ready_cb(gt_anim_st * anim) {
    gt_obj_st * obj = (gt_obj_st *)anim->tar;
    gt_obj_set_visible(obj, GT_VISIBLE);
    gt_obj_set_opa(obj, GT_OPA_100);
}

static void _show_slider_bar(gt_obj_st * obj) {
    _gt_media_player_st * style = (_gt_media_player_st *)obj;

    /** opa 0% -> 100% */
    gt_anim_st anim = {0};
    gt_anim_init(&anim);
    gt_anim_set_time(&anim, 150);
    gt_anim_set_target(&anim, style->slider_p);
    gt_anim_set_value(&anim, GT_OPA_0, GT_OPA_100);
    gt_anim_set_exec_cb(&anim, _show_slider_bar_exec_cb);
    gt_anim_set_start_cb(&anim, _show_slider_bar_start_cb);
    gt_anim_set_ready_cb(&anim, _show_slider_bar_ready_cb);
    gt_anim_start(&anim);
}

static void _hide_slider_bar_exec_cb(void * t, int32_t val) {
    gt_obj_st * obj = (gt_obj_st *)t;
    gt_obj_set_opa(obj, val);
}

static void _hide_slider_bar_ready_cb(gt_anim_st * anim) {
    gt_obj_st * obj = (gt_obj_st *)anim->tar;
    gt_obj_set_visible(obj, GT_INVISIBLE);
    gt_obj_set_opa(obj, GT_OPA_0);
}

static void _hide_slider_bar(gt_obj_st * obj, bool immediately) {
    _gt_media_player_st * style = (_gt_media_player_st *)obj;
    if (NULL == style->slider_p) {
        return;
    }
    if (immediately) {
        gt_obj_set_opa(style->slider_p, GT_OPA_0);
        gt_obj_set_visible(style->slider_p, GT_INVISIBLE);
        return;
    }
    /** opa 100% -> 0% */
    gt_anim_st anim = {0};
    gt_anim_init(&anim);
    gt_anim_set_time(&anim, 200);
    gt_anim_set_target(&anim, style->slider_p);
    gt_anim_set_value(&anim, GT_OPA_100, GT_OPA_0);
    gt_anim_set_exec_cb(&anim, _hide_slider_bar_exec_cb);
    gt_anim_set_ready_cb(&anim, _hide_slider_bar_ready_cb);
    gt_anim_start(&anim);
}

static void _media_player_init_cb(gt_obj_st * obj) {
    _gt_media_player_st * style = (_gt_media_player_st *)obj;
    gt_attr_rect_st rect_attr;

    if (gt_obj_is_show_bg(obj)) {
        gt_graph_init_rect_attr(&rect_attr);
        rect_attr.reg.is_fill = true;
        rect_attr.radius      = obj->radius;
        rect_attr.bg_opa      = obj->opa;
        rect_attr.bg_color    = obj->bgcolor;
        draw_bg(obj->draw_ctx, &rect_attr, &obj->area);

        // text
        if (false == style->reg.hide_title) {
            _show_center_content(obj, 16, "media player");
            return;
        }
    }

    if (false == style->reg.playing) {
        _show_center_content(obj, 24, GT_SYMBOL_PAUSE);
        return;
    }
    /** playing */
    _hide_center_content(obj);
}

static void _media_player_deinit_cb(gt_obj_st * obj) {

}

static bool _media_player_switch_status(gt_obj_st * obj) {
    _gt_media_player_st * style = (_gt_media_player_st *)obj;
    if (false == style->reg.hide_title) {
        return false;
    }
    style->reg.playing = !style->reg.playing;
    if (style->reg.playing) {
        if (style->play_hdr.media_cb) {
            style->play_hdr.media_cb(obj, style->play_hdr.user_data);
        }
        _hide_slider_bar(obj, false);
        return true;
    }
    /** stop */
    if (style->stop_hdr.media_cb) {
        style->stop_hdr.media_cb(obj, style->stop_hdr.user_data);
    }
    _show_slider_bar(obj);
    return true;
}

static void _media_player_event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code_val = gt_event_get_code(e);

    if (GT_EVENT_TYPE_DRAW_START == code_val) {
        gt_disp_invalid_area(obj);
    }
    else if (GT_EVENT_TYPE_INPUT_RELEASED == code_val) {
        /** stop or run */
        if (_media_player_switch_status(obj)) {
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
        }
    }
    else if (GT_EVENT_TYPE_UPDATE_STYLE == code_val) {
        _update_slider_bar_pos_size(obj);
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
    }
}



/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_media_player_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    if (NULL == obj) {
        return obj;
    }
    obj->show_bg = true;
    if (obj->show_bg) {
        obj->bgcolor = gt_color_black();
        obj->radius = 0;
    }
    _gt_media_player_st * style = (_gt_media_player_st *)obj;

    style->media_p = gt_img_create(obj);
    gt_obj_set_touch_parent(style->media_p, true);

    style->slider_p = gt_slider_create(obj);
    gt_obj_set_size(style->slider_p, 20, 16);
    _hide_slider_bar(obj, true);

    style->status_p = gt_label_create(obj);
    gt_obj_set_touch_parent(style->status_p, true);
    return obj;
}

void gt_media_player_set_play_cb(gt_obj_st * obj, gt_media_player_handler_cb play_cb, void * user_data)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    if (NULL == play_cb) { return; }

    _gt_media_player_st * style = (_gt_media_player_st *)obj;
    style->play_hdr.media_cb = play_cb;
    style->play_hdr.user_data = user_data;
}

void gt_media_player_set_stop_cb(gt_obj_st * obj, gt_media_player_handler_cb stop_cb, void * user_data)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    if (NULL == stop_cb) { return; }

    _gt_media_player_st * style = (_gt_media_player_st *)obj;
    style->stop_hdr.media_cb = stop_cb;
    style->stop_hdr.user_data = user_data;
}

void gt_media_player_set_raw(gt_obj_st * obj, gt_color_img_raw_st * raw)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    if (NULL == raw) { return; }

    _gt_media_player_st * style = (_gt_media_player_st *)obj;
    if (false == style->reg.playing) { style->reg.playing = true; }
    if (false == style->reg.hide_title) { style->reg.hide_title = true; }

    gt_obj_set_pos_relative(style->media_p, obj, (obj->area.w - raw->width) >> 1, (obj->area.h - raw->height) >> 1);
    gt_img_set_raw_data(style->media_p, raw);
    gt_disp_invalid_area(obj);
}

void gt_media_player_set_total_time(gt_obj_st * obj, gt_size_t total_time)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_media_player_st * style = (_gt_media_player_st *)obj;
    if (0 == total_time) {
        total_time = 100;
    }
    gt_slider_set_start_end(style->slider_p, 0, total_time);
}

void gt_media_player_set_current_time(gt_obj_st * obj, gt_size_t current_time)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_media_player_st * style = (_gt_media_player_st *)obj;
    gt_slider_set_pos(style->slider_p, current_time);
}

gt_size_t gt_media_player_get_current_time(gt_obj_st * obj)
{
    if (true == gt_obj_is_type(obj, GT_TYPE_SLIDER)) {
        return gt_slider_get_pos(obj);
    }
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return -1;
    }
    _gt_media_player_st * style = (_gt_media_player_st *)obj;
    return gt_slider_get_pos(style->slider_p);
}


void gt_media_player_set_slider_change_cb(gt_obj_st * obj, gt_event_cb_t value_change_cb, void * user_data)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return;
    }
    _gt_media_player_st * style = (_gt_media_player_st *)obj;
    gt_obj_add_event_cb(style->slider_p, value_change_cb, GT_EVENT_TYPE_UPDATE_VALUE, user_data);
}



/* end ------------------------------------------------------------------*/
#endif
