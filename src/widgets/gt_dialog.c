/**
 * @file gt_dialog.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-03-04 15:53:48
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_dialog.h"

#if GT_USE_LAYER_TOP && GT_CFG_ENABLE_DIALOG
#include "../hal/gt_hal_disp.h"
#include "../core/gt_obj_pos.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../others/gt_log.h"
#include "../others/gt_anim.h"
#include "../widgets/gt_label.h"
#include "../widgets/gt_btn.h"
#include "../font/gt_symbol.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_DIALOG
#define MY_CLASS    &gt_dialog_class

#define _PADDING_VER    4
#define _PADDING_HOR    8

#define _BTN_SIZE_16    30
#define _BTN_SIZE_24    40

/** the height of title bar area */
#define _TITLE_BAR_HEIGHT_16    ((_PADDING_VER << 1) + _BTN_SIZE_16)
/** the height of title bar area */
#define _TITLE_BAR_HEIGHT_24    ((_PADDING_VER << 1) + _BTN_SIZE_24)

/* private typedef ------------------------------------------------------*/

typedef struct _gt_dialog_s {
    gt_obj_st obj;

    gt_obj_st * btn_close;
    gt_obj_st * title;
    gt_obj_st * content;
    gt_obj_st * btn_cancel;
    gt_obj_st * btn_confirm;

    uint32_t anim_time;

    gt_color_t border_color;
    uint8_t border_width;
    uint8_t border_radius;

    uint8_t outside_auto_hide : 1;
    uint8_t reserved : 7;
}_gt_dialog_st;


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_dialog_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = NULL,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_dialog_st)
};


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void _update_display_area(gt_obj_st * obj, gt_obj_st * top) {
    obj->area.x = top->area.x + (top->area.w >> 2);
    obj->area.y = top->area.y + (top->area.h >> 2);
    obj->area.w = top->area.w >> 1;
    obj->area.h = top->area.h >> 1;
}

static void _init_cb(gt_obj_st * obj) {
    _gt_dialog_st * style = (_gt_dialog_st * )obj;
    gt_attr_rect_st rect_attr;

    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill   = true;
    rect_attr.radius        = style->border_radius;
    rect_attr.bg_opa        = obj->opa;
    rect_attr.border_width  = style->border_width;
    rect_attr.border_color  = style->border_color;
    rect_attr.bg_color      = obj->bgcolor;

    // 1:base shape
    draw_bg(obj->draw_ctx, &rect_attr, &obj->area);
}

static void _dialog_anim_exec_cb(void * obj, int32_t val) {
    gt_obj_child_set_prop((gt_obj_st * )obj, GT_OBJ_PROP_TYPE_OPA, (uint8_t)val);
    gt_obj_set_opa((gt_obj_st * )obj, (gt_opa_t)val);
}

static void _dialog_show_anim_start_cb(struct gt_anim_s * anim) {
    gt_obj_st * obj = (gt_obj_st * )anim->tar;
    obj->opa = GT_OPA_0;
    gt_obj_set_visible(obj, GT_VISIBLE);
}

static void _dialog_close_anim_ready_cb(struct gt_anim_s * anim) {
    gt_obj_st * obj = (gt_obj_st * )anim->tar;
    obj->opa = GT_OPA_0;
    gt_obj_set_visible(obj, GT_INVISIBLE);
    gt_obj_destroy(obj);
}

/**
 * @brief Create animation to display or close the dialog
 *
 * @param obj
 * @param is_create true: create anim to display the dialog, false: create anim to close and free the dialog
 * @param start The opa of widget at the start of the animation @ref gt_opa_t
 * @param end The opa of widget at the end of the animation @ref gt_opa_t
 */
static void _create_anim_handler(gt_obj_st *obj, bool is_create, int32_t start, int32_t end) {
    gt_anim_st anim;

    if (gt_obj_is_untouchability(obj)) {
        return;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )obj;

    gt_anim_init(&anim);
    gt_anim_set_value(&anim, start, end);
    gt_anim_set_target(&anim, obj);
    gt_anim_set_time(&anim, style->anim_time);
    gt_anim_set_exec_cb(&anim, _dialog_anim_exec_cb);
    if (is_create) {
        /** want to display the dialog */
        gt_anim_set_start_cb(&anim, _dialog_show_anim_start_cb);
    } else {
        /** want to close and free the dialog */
        gt_anim_set_ready_cb(&anim, _dialog_close_anim_ready_cb);

        gt_obj_set_untouchability(obj, true);
        gt_obj_child_set_prop(obj, GT_OBJ_PROP_TYPE_UNTOUCHABILITY, true);
    }
    gt_anim_set_path_type(&anim, GT_ANIM_PATH_TYPE_EASE_OUT);
    gt_anim_start(&anim);
}

static void _close_release_cb(gt_event_st * e) {
    gt_obj_st * dialog = (gt_obj_st * )e->user_data;
    gt_event_send(dialog, GT_EVENT_TYPE_NOTIFY_CLOSE, NULL);
}

static void _set_close_btn_pos(gt_obj_st * close_btn, gt_obj_st * dialog) {
    if (NULL == close_btn) {
        return ;
    }
    close_btn->area.w = _BTN_SIZE_16;
    close_btn->area.h = _BTN_SIZE_16;
    close_btn->area.x = dialog->area.x + dialog->area.w - close_btn->area.w - _PADDING_HOR;
    close_btn->area.y = dialog->area.y + _PADDING_VER;
}

static gt_obj_st * _create_close_btn(gt_obj_st * dialog) {
    gt_obj_st * close_btn = gt_btn_create(dialog);
    if (NULL == close_btn) {
        return NULL;
    }
    _set_close_btn_pos(close_btn, dialog);

    gt_btn_set_color_background(close_btn, dialog->bgcolor);
    gt_btn_set_font_color(close_btn, gt_color_hex(0x7f7f7f));
    gt_btn_set_color_pressed(close_btn, gt_color_hex(0xdfdfdf));
    gt_obj_set_opa(close_btn, GT_OPA_0);
    gt_btn_set_text(close_btn, GT_SYMBOL_CLOSE);
    gt_obj_add_event_cb(close_btn, _close_release_cb, GT_EVENT_TYPE_INPUT_RELEASED, dialog);
    gt_obj_set_inside(close_btn, true);

    return close_btn;
}

static void _set_title_pos(gt_obj_st * title, gt_obj_st * dialog) {
    if (NULL == title) {
        return ;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )dialog;
    gt_obj_st * close_btn = style->btn_close;

    gt_size_t btn_width = close_btn ? close_btn->area.w : 0;

    title->area.x = dialog->area.x + _PADDING_HOR;
    title->area.y = dialog->area.y + _PADDING_VER;
    title->area.w = dialog->area.w - (_PADDING_HOR << 1) - btn_width;
    title->area.h = _BTN_SIZE_16;
}

static gt_obj_st * _create_title(gt_obj_st * dialog, char * title_text) {
    gt_obj_st * title = gt_label_create(dialog);
    if (NULL == title) {
        return NULL;
    }
    _set_title_pos(title, dialog);

    gt_obj_set_opa(title, GT_OPA_0);
    gt_obj_set_inside(title, true);
    gt_label_set_font_align(title, GT_ALIGN_LEFT_MID);
    gt_label_set_font_color(title, gt_color_hex(0x2f2f2f));
    gt_label_set_text(title, title_text);

    return title;
}

static void _set_content_pos(gt_obj_st * content, gt_obj_st * dialog) {
    if (NULL == content) {
        return ;
    }
    gt_size_t content_padding = (_PADDING_HOR << 1);
    content->area.x = dialog->area.x + content_padding;
    content->area.y = dialog->area.y + _TITLE_BAR_HEIGHT_16;
    content->area.w = dialog->area.w - (content_padding << 1);

    gt_size_t other_height = _TITLE_BAR_HEIGHT_16 + _PADDING_VER;
    gt_size_t btn_size = _BTN_SIZE_16;

    content->area.h = (dialog->area.h - other_height) < btn_size ? btn_size : (dialog->area.h - other_height);
}

static gt_obj_st * _create_content(gt_obj_st * dialog, char * content_text) {
    gt_obj_st * content = gt_label_create(dialog);
    if (NULL == content) {
        return NULL;
    }
    _set_content_pos(content, dialog);

    gt_obj_set_opa(content, GT_OPA_0);
    gt_obj_set_inside(content, true);
    gt_label_set_font_align(content, GT_ALIGN_LEFT);
    gt_label_set_font_color(content, gt_color_hex(0x4f4f4f));
    gt_label_set_text(content, content_text);

    return content;
}

static void _set_cancel_btn_pos(gt_obj_st * cancel, gt_obj_st * dialog) {
    if (NULL == cancel) {
        return ;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )dialog;
    gt_obj_st * confirm = style->btn_confirm;
    gt_size_t confirm_width = confirm ? confirm->area.w : 0;

    cancel->area.w = _BTN_SIZE_16 << 1;
    cancel->area.h = _BTN_SIZE_16;
    cancel->area.x = dialog->area.x + dialog->area.w - cancel->area.w - confirm_width - (_PADDING_HOR << 1);
    cancel->area.y = dialog->area.y + dialog->area.h - cancel->area.h - _PADDING_VER;
}

static gt_obj_st * _create_cancel_btn(gt_obj_st * dialog, gt_event_cb_t cancel_cb, void * cancel_cb_user_data) {
    gt_obj_st * cancel_btn = gt_btn_create(dialog);
    if (NULL == cancel_btn) {
        return NULL;
    }
    _set_cancel_btn_pos(cancel_btn, dialog);

    gt_btn_set_color_background(cancel_btn, dialog->bgcolor);
    gt_btn_set_color_pressed(cancel_btn, gt_color_hex(0xdfdfdf));
    gt_btn_set_font_color(cancel_btn, gt_color_hex(0x5f5f5f));
    gt_btn_set_border_width(cancel_btn, 1);
    gt_btn_set_color_border(cancel_btn, gt_color_hex(0x7f7f7f));

    gt_obj_set_opa(cancel_btn, GT_OPA_0);
    gt_btn_set_text(cancel_btn, GT_SYMBOL_CLOSE);
    gt_btn_set_radius(cancel_btn, _BTN_SIZE_16 >> 2);
    gt_obj_set_inside(cancel_btn, true);
    if (cancel_cb) {
        gt_obj_add_event_cb(cancel_btn, cancel_cb, GT_EVENT_TYPE_INPUT_RELEASED, cancel_cb_user_data);
    }
    gt_obj_add_event_cb(cancel_btn, _close_release_cb, GT_EVENT_TYPE_INPUT_RELEASED, dialog);

    return cancel_btn;
}

static void _set_confirm_btn_pos(gt_obj_st * confirm, gt_obj_st * dialog) {
    if (NULL == confirm) {
        return ;
    }
    confirm->area.w = _BTN_SIZE_16 << 1;
    confirm->area.h = _BTN_SIZE_16;
    confirm->area.x = dialog->area.x + dialog->area.w - confirm->area.w - (_PADDING_HOR << 1);
    confirm->area.y = dialog->area.y + dialog->area.h - confirm->area.h - _PADDING_VER;
}

static gt_obj_st * _create_confirm_btn(gt_obj_st * dialog, gt_event_cb_t confirm_cb, void * confirm_cb_user_data) {
    gt_obj_st * confirm_btn = gt_btn_create(dialog);
    if (NULL == confirm_btn) {
        return NULL;
    }
    _set_confirm_btn_pos(confirm_btn, dialog);

    gt_obj_set_opa(confirm_btn, GT_OPA_0);
    gt_btn_set_text(confirm_btn, GT_SYMBOL_OK);
    gt_btn_set_radius(confirm_btn, _BTN_SIZE_16 >> 2);
    gt_obj_set_inside(confirm_btn, true);
    if (confirm_cb) {
        gt_obj_add_event_cb(confirm_btn, confirm_cb, GT_EVENT_TYPE_INPUT_RELEASED, confirm_cb_user_data);
    }
    gt_obj_add_event_cb(confirm_btn, _close_release_cb, GT_EVENT_TYPE_INPUT_RELEASED, dialog);

    return confirm_btn;
}

static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et type = gt_event_get_code(e);
    _gt_dialog_st * style = (_gt_dialog_st * )obj;

    switch (type) {
        case GT_EVENT_TYPE_DRAW_START: {
            gt_disp_invalid_area(obj);
            break;
        }
        case GT_EVENT_TYPE_INPUT_HIDED: {
            if (style->outside_auto_hide) {
                _create_anim_handler(obj, false, GT_OPA_100, GT_OPA_0);
            }
            break;
        }
        case GT_EVENT_TYPE_NOTIFY_CLOSE: {
            _create_anim_handler(obj, false, GT_OPA_100, GT_OPA_0);
            break;
        }
        case GT_EVENT_TYPE_UPDATE_STYLE: {
            _set_close_btn_pos(style->btn_close, obj);
            _set_title_pos(style->title, obj);
            _set_content_pos(style->content, obj);
            _set_confirm_btn_pos(style->btn_confirm, obj);
            _set_cancel_btn_pos(style->btn_cancel, obj);
            break;
        }
        default:
            break;
    }
}

static gt_obj_st * _find_showing_recursive_by_type(gt_obj_st * obj, gt_obj_type_et type) {
    if (NULL == obj) {
        return NULL;
    }
    if (gt_obj_is_type(obj, type) && gt_obj_get_visible(obj)) {
        return obj;
    }

    gt_obj_st * ret = NULL;
    for (gt_size_t i = obj->cnt_child - 1; i >= 0; i--) {
        ret = _find_showing_recursive_by_type(obj->child[i], type);
        if (ret) {
            return ret;
        }
    }
    return ret;
}

/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_dialog_create(bool show_close_btn)
{
    gt_disp_st * disp = gt_disp_get_default();
    if (NULL == disp || NULL == disp->layer_top) {
        return NULL;
    }
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, disp->layer_top);
    if (NULL == obj) {
        return obj;
    }

    _gt_dialog_st * style = (_gt_dialog_st * )obj;

    obj->visible = GT_INVISIBLE;
    obj->opa = GT_OPA_0;

    style->outside_auto_hide = true;
    style->border_color = gt_color_hex(0xc7c7c7);
    style->border_width = 2;
    style->border_radius = 10;
    style->anim_time = 250;

    /** Center 50% area */
    _update_display_area(obj, disp->layer_top);

    if (show_close_btn) {
        style->btn_close = _create_close_btn(obj);
    }

    return obj;
}

gt_obj_st * gt_dialog_create_issue(bool show_close_btn, gt_dialog_param_st const * const param)
{
    gt_obj_st * obj = gt_dialog_create(show_close_btn);
    if (NULL == obj) {
        return obj;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )obj;

    if (param->title) {
        style->title = _create_title(obj, param->title);
    }
    if (param->content) {
        style->content = _create_content(obj, param->content);
    }
    /** Must exist */
    style->btn_confirm = _create_confirm_btn(obj, param->confirm_cb, param->confirm_cb_user_data);

    if (param->cancel_cb) {
        style->btn_cancel = _create_cancel_btn(obj, param->cancel_cb, param->cancel_cb_user_data);
    }

    return obj;
}

void gt_dialog_show(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }

    _create_anim_handler(obj, true, GT_OPA_0, GT_OPA_100);
}

void gt_dialog_close(gt_obj_st * obj)
{
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        return ;
    }

    /** Better than: _create_anim_handler(obj, false, GT_OPA_100, GT_OPA_0); */
    gt_event_send(obj, GT_EVENT_TYPE_NOTIFY_CLOSE, NULL);
}

gt_obj_st * gt_dialog_get_active_obj(void)
{
    gt_disp_st * disp = gt_disp_get_default();
    if (NULL == disp || NULL == disp->layer_top) {
        return false;
    }
    return _find_showing_recursive_by_type(disp->layer_top, OBJ_TYPE);
}

bool gt_dialog_has_showing(void)
{
    return gt_dialog_get_active_obj() ? true : false;
}

void gt_dialog_set_border_color(gt_obj_st * dialog, gt_color_t color)
{
    if (false == gt_obj_is_type(dialog, OBJ_TYPE)) {
        return ;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )dialog;
    style->border_color = color;
}

void gt_dialog_set_border_width(gt_obj_st * dialog, uint8_t width)
{
    if (false == gt_obj_is_type(dialog, OBJ_TYPE)) {
        return ;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )dialog;
    style->border_width = width;
}

void gt_dialog_set_bgcolor(gt_obj_st * dialog, gt_color_t color)
{
    if (false == gt_obj_is_type(dialog, OBJ_TYPE)) {
        return ;
    }
    dialog->bgcolor = color;
}

void gt_dialog_set_border_radius(gt_obj_st * dialog, uint8_t radius)
{
    if (false == gt_obj_is_type(dialog, OBJ_TYPE)) {
        return ;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )dialog;
    style->border_radius = radius;
}

void gt_dialog_set_outside_auto_hide(gt_obj_st * dialog, bool auto_hide)
{
    if (false == gt_obj_is_type(dialog, OBJ_TYPE)) {
        return ;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )dialog;
    style->outside_auto_hide = auto_hide ? true : false;
}

void gt_dialog_set_anim_time(gt_obj_st * dialog, uint32_t time)
{
    if (false == gt_obj_is_type(dialog, OBJ_TYPE)) {
        return ;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )dialog;
    style->anim_time = time;
}

/* ------------------- Issue mode api ------------------- */

void gt_dialog_set_title_font_color(gt_obj_st * dialog, gt_color_t color)
{
    if (false == gt_obj_is_type(dialog, OBJ_TYPE)) {
        return ;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )dialog;
    if (NULL == style->title) {
        return ;
    }
    gt_label_set_font_color(style->title, color);
}

void gt_dialog_set_title_font_size(gt_obj_st * dialog, uint8_t size)
{
    if (false == gt_obj_is_type(dialog, OBJ_TYPE)) {
        return ;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )dialog;
    if (NULL == style->title) {
        return ;
    }
    gt_label_set_font_size(style->title, size);
}

void gt_dialog_set_title_font_align(gt_obj_st * dialog, gt_align_et align)
{
    if (false == gt_obj_is_type(dialog, OBJ_TYPE)) {
        return ;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )dialog;
    if (NULL == style->title) {
        return ;
    }
    gt_label_set_font_align(style->title, align);
}

void gt_dialog_set_title_font_family_cn(gt_obj_st * dialog, gt_family_t family)
{
    if (false == gt_obj_is_type(dialog, OBJ_TYPE)) {
        return ;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )dialog;
    if (NULL == style->title) {
        return ;
    }
    gt_label_set_font_family_cn(style->title, family);
}

void gt_dialog_set_title_font_family_en(gt_obj_st * dialog, gt_family_t family)
{
    if (false == gt_obj_is_type(dialog, OBJ_TYPE)) {
        return ;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )dialog;
    if (NULL == style->title) {
        return ;
    }
    gt_label_set_font_family_en(style->title, family);
}

void gt_dialog_set_title_font_family_fl(gt_obj_st * dialog, gt_family_t family)
{
    if (false == gt_obj_is_type(dialog, OBJ_TYPE)) {
        return ;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )dialog;
    if (NULL == style->title) {
        return ;
    }
    gt_label_set_font_family_fl(style->title, family);
}

void gt_dialog_set_title_font_thick_en(gt_obj_st * dialog, uint8_t thick)
{
    if (false == gt_obj_is_type(dialog, OBJ_TYPE)) {
        return ;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )dialog;
    if (NULL == style->title) {
        return ;
    }
    gt_label_set_font_thick_en(style->title, thick);
}

void gt_dialog_set_title_font_thick_cn(gt_obj_st * dialog, uint8_t thick)
{
    if (false == gt_obj_is_type(dialog, OBJ_TYPE)) {
        return ;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )dialog;
    if (NULL == style->title) {
        return ;
    }
    gt_label_set_font_thick_cn(style->title, thick);
}

void gt_dialog_set_content_font_color(gt_obj_st * dialog, gt_color_t color)
{
    if (false == gt_obj_is_type(dialog, OBJ_TYPE)) {
        return ;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )dialog;
    if (NULL == style->content) {
        return ;
    }
    gt_label_set_font_color(style->content, color);
}

void gt_dialog_set_content_font_size(gt_obj_st * dialog, uint8_t size)
{
    if (false == gt_obj_is_type(dialog, OBJ_TYPE)) {
        return ;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )dialog;
    if (NULL == style->content) {
        return ;
    }
    gt_label_set_font_size(style->content, size);
}

void gt_dialog_set_content_font_align(gt_obj_st * dialog, gt_align_et align)
{
    if (false == gt_obj_is_type(dialog, OBJ_TYPE)) {
        return ;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )dialog;
    if (NULL == style->content) {
        return ;
    }
    gt_label_set_font_align(style->content, align);
}

void gt_dialog_set_content_font_family_cn(gt_obj_st * dialog, gt_family_t family)
{
    if (false == gt_obj_is_type(dialog, OBJ_TYPE)) {
        return ;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )dialog;
    if (NULL == style->content) {
        return ;
    }
    gt_label_set_font_family_cn(style->content, family);
}

void gt_dialog_set_content_font_family_en(gt_obj_st * dialog, gt_family_t family)
{
    if (false == gt_obj_is_type(dialog, OBJ_TYPE)) {
        return ;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )dialog;
    if (NULL == style->content) {
        return ;
    }
    gt_label_set_font_family_en(style->content, family);
}

void gt_dialog_set_content_font_family_fl(gt_obj_st * dialog, gt_family_t family)
{
    if (false == gt_obj_is_type(dialog, OBJ_TYPE)) {
        return ;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )dialog;
    if (NULL == style->content) {
        return ;
    }
    gt_label_set_font_family_fl(style->content, family);
}

void gt_dialog_set_content_font_thick_en(gt_obj_st * dialog, uint8_t thick)
{
    if (false == gt_obj_is_type(dialog, OBJ_TYPE)) {
        return ;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )dialog;
    if (NULL == style->content) {
        return ;
    }
    gt_label_set_font_thick_en(style->content, thick);
}

void gt_dialog_set_content_font_thick_cn(gt_obj_st * dialog, uint8_t thick)
{
    if (false == gt_obj_is_type(dialog, OBJ_TYPE)) {
        return ;
    }
    _gt_dialog_st * style = (_gt_dialog_st * )dialog;
    if (NULL == style->content) {
        return ;
    }
    gt_label_set_font_thick_cn(style->content, thick);
}

#endif  /** GT_USE_LAYER_TOP && GT_CFG_ENABLE_DIALOG */
/* end ------------------------------------------------------------------*/
