/**
 * @file gt_toast.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2025-08-21
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_toast.h"

#if GT_CFG_ENABLE_TOAST
#include "../core/gt_disp.h"
#include "../core/gt_draw.h"
#include "../others/gt_anim.h"
#include "../others/gt_color.h"
#include "./gt_label.h"


/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_TOAST
#define MY_CLASS    &gt_toast_class


/* private typedef ------------------------------------------------------*/
typedef struct {
    gt_obj_st base;
    gt_obj_st * lab_p;
    gt_anim_st * anim_transit;

    gt_toast_cb open_cb;
    gt_toast_cb close_cb;
    void * data_p;          /** called by open_cb / close_cb, just a remark pointer */

    gt_opa_t max_opa;

    uint32_t duration;
    uint8_t position : 2;
}_gt_toast_st;

typedef struct {
    _gt_toast_st * toast_p;
}_gt_toast_ctrl_st;


/* static variables -----------------------------------------------------*/
static void GT_ATTRIBUTE_RAM_TEXT _toast_init_cb(gt_obj_st * obj);
static void GT_ATTRIBUTE_RAM_TEXT _toast_deinit_cb(gt_obj_st * obj);
static void GT_ATTRIBUTE_RAM_TEXT _toast_event_cb(struct gt_obj_s * obj, gt_event_st * e);

static const gt_obj_class_st gt_toast_class = {
    ._init_cb      = _toast_init_cb,
    ._deinit_cb    = _toast_deinit_cb,
    ._event_cb     = _toast_event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_toast_st)
};

static _gt_toast_ctrl_st GT_ATTRIBUTE_RAM_DATA _toast_ctrl = {
    .toast_p = NULL,
};



/* macros ---------------------------------------------------------------*/
static void GT_ATTRIBUTE_RAM_TEXT
_create_holding_display_anim(_gt_toast_st * toast);

static void GT_ATTRIBUTE_RAM_TEXT
_reset_global_setting(void);


/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void GT_ATTRIBUTE_RAM_TEXT _toast_init_cb(gt_obj_st * obj) {
    _gt_toast_st * style = (_gt_toast_st * )obj;

    if (obj->show_bg) {
        gt_attr_rect_st rect_attr;
        gt_graph_init_rect_attr(&rect_attr);
        rect_attr.reg.is_fill    = true;
        rect_attr.border_width   = 0;
        rect_attr.border_color   = obj->bgcolor;
        rect_attr.bg_opa         = obj->opa;
        rect_attr.bg_color       = obj->bgcolor;
        rect_attr.radius         = obj->radius;

        gt_area_st area_base = obj->area;
        draw_bg(obj->draw_ctx, &rect_attr, &area_base);
    }

    draw_focus(obj);
}

static void GT_ATTRIBUTE_RAM_TEXT _toast_deinit_cb(gt_obj_st * obj) {
}

static void GT_ATTRIBUTE_RAM_TEXT _toast_event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code_val = gt_event_get_code(e);

    switch(code_val) {
        case GT_EVENT_TYPE_DRAW_START:
        case GT_EVENT_TYPE_UPDATE_STYLE: {
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;
        }
        case GT_EVENT_TYPE_INPUT_PRESSED: {
            break;
        }
        case GT_EVENT_TYPE_INPUT_RELEASED: {
            break;
        }
        case GT_EVENT_TYPE_INPUT_HIDED: {
            break;
        }
        default:
            break;
    }
}

static inline _gt_toast_ctrl_st * GT_ATTRIBUTE_RAM_TEXT _get_ctrl(void) {
    return &_toast_ctrl;
}

static void GT_ATTRIBUTE_RAM_TEXT _del_toast_cb(gt_event_st * e) {
    _toast_ctrl.toast_p = NULL;
}

static gt_obj_st * GT_ATTRIBUTE_RAM_TEXT
_get_toast_obj(void) {
    _gt_toast_ctrl_st * ctrl_p = _get_ctrl();
    if (gt_obj_is_type((gt_obj_st * )ctrl_p->toast_p, OBJ_TYPE)) {
        return (gt_obj_st * )ctrl_p->toast_p;
    }
    return NULL;
}

static _gt_toast_st * GT_ATTRIBUTE_RAM_TEXT _toast_create(void) {
    _gt_toast_ctrl_st * ctrl_p = _get_ctrl();
    if (false == gt_obj_is_type((gt_obj_st * )ctrl_p->toast_p, OBJ_TYPE)) {
#if GT_USE_LAYER_TOP
        ctrl_p->toast_p = (_gt_toast_st * )gt_obj_class_create(MY_CLASS, gt_disp_get_layer_top());
        gt_obj_add_event_cb((gt_obj_st * )ctrl_p->toast_p, _del_toast_cb, GT_EVENT_TYPE_CHANGE_DELETED, NULL);
#endif
        if (ctrl_p->toast_p) {
            ctrl_p->toast_p->lab_p = gt_label_create((gt_obj_st * )ctrl_p->toast_p);
            _reset_global_setting();
        }
    }
    ctrl_p->toast_p->open_cb = NULL;
    ctrl_p->toast_p->close_cb = NULL;
    ctrl_p->toast_p->data_p = NULL;
    return ctrl_p->toast_p;
}

static void GT_ATTRIBUTE_RAM_TEXT
_transit_open_ready_cb(gt_anim_st * anim) {
    gt_obj_st * obj = (gt_obj_st * )anim->tar;
    _gt_toast_st * toast = (_gt_toast_st * )obj;
    _create_holding_display_anim((_gt_toast_st * )obj);
    if (toast->open_cb) {
        toast->open_cb(toast->data_p);
    }
}

static void GT_ATTRIBUTE_RAM_TEXT
_transit_close_ready_cb(gt_anim_st * anim) {
    gt_obj_st * obj = (gt_obj_st * )anim->tar;
    _gt_toast_st * toast = (_gt_toast_st * )obj;

    gt_obj_set_opa(obj, GT_OPA_0);
    gt_obj_set_visible(obj, GT_INVISIBLE);
}

static void GT_ATTRIBUTE_RAM_TEXT
_create_transit_anim(_gt_toast_st * toast, bool is_open) {
    gt_obj_st * obj = (gt_obj_st *)toast;
    if (is_open) {
        gt_obj_set_visible(obj, GT_VISIBLE);
        gt_obj_set_opa(obj, GT_OPA_0);
        gt_obj_set_opa_anim(toast->lab_p, toast->max_opa);
        toast->anim_transit = gt_obj_set_opa_anim(obj, toast->max_opa);
        gt_anim_set_ready_cb(toast->anim_transit, _transit_open_ready_cb);
    } else {
        gt_obj_set_opa_anim(toast->lab_p, 0);
        toast->anim_transit = gt_obj_set_opa_anim(obj, GT_OPA_0);
        gt_anim_set_ready_cb(toast->anim_transit, _transit_close_ready_cb);
    }
}

static void GT_ATTRIBUTE_RAM_TEXT
_holding_open_start_cb(gt_anim_st * anim) {
    _gt_toast_st * toast = (_gt_toast_st * )anim->tar;
    if (toast->open_cb) {
        toast->open_cb(toast->data_p);
    }
}

static void GT_ATTRIBUTE_RAM_TEXT
_holding_close_ready_cb(gt_anim_st * anim) {
    _gt_toast_st * toast = (_gt_toast_st * )anim->tar;
    _create_transit_anim(toast, false);
    if (toast->close_cb) {
        toast->close_cb(toast->data_p);
    }
}

static void GT_ATTRIBUTE_RAM_TEXT
_create_holding_display_anim(_gt_toast_st * toast) {
    gt_obj_st * obj = (gt_obj_st *)toast;
    gt_anim_st holding = {0};
    gt_anim_init(&holding);
    gt_anim_set_target(&holding, toast);
    gt_anim_set_time(&holding, toast->duration);
    gt_anim_set_start_cb(&holding, _holding_open_start_cb);
    gt_anim_set_ready_cb(&holding, _holding_close_ready_cb);
    gt_anim_start(&holding);
}

static inline void GT_ATTRIBUTE_RAM_TEXT
_set_label_info(gt_obj_st * lab_p, gt_toast_param_st * params) {
    if (false == gt_obj_is_type(lab_p, GT_TYPE_LAB)) {
        return;
    }
    if (params->message) {
        gt_label_set_text_by_len(lab_p, params->message, strlen(params->message));
    } else {
        gt_label_set_text_by_len(lab_p, " ", strlen(" "));
    }
    gt_color_t cur_color = gt_label_get_font_color(lab_p);
    if (cur_color.full != params->font_color.full) {
        gt_label_set_font_color(lab_p, params->font_color);
    }
}

static void GT_ATTRIBUTE_RAM_TEXT
_common_update_toast_data(_gt_toast_st * toast, gt_toast_param_st * params) {
    gt_obj_st * obj = (gt_obj_st * )toast;

    obj->bgcolor = params->bg_color;
    toast->duration = params->duration;
    toast->position = params->position;
    if (params->opa) {
        toast->max_opa = params->opa;
    }
    toast->open_cb = params->open_cb;
    toast->close_cb = params->close_cb;
    toast->data_p = params->data_p;
    _set_label_info(toast->lab_p, params);
}

/**
 * @brief calc widget and label size
 *      msg: width = (str_width + font_width) <= (scr_width - font_width)
 *      m: half of font width
 *
 *       /--------->  screen   <---------\
 *      |---------------------------------|
 *      | m |          msg            | m |
 *      |   |         max_w           |   |
 *      |---------------------------------|
 *
 * @param style
 * @return
 */
static void GT_ATTRIBUTE_RAM_TEXT
_calc_position_by_type(_gt_toast_st * style) {
    gt_obj_st * obj = (gt_obj_st * )style;
    uint16_t font_size = gt_label_get_font_size(style->lab_p);
    uint16_t str_len = gt_label_get_single_line_width(style->lab_p);
    uint16_t max_w = gt_disp_get_res_hor(NULL) - font_size;
    if (gt_obj_get_visible(obj)) {
        /** WARN: remove residual shadow, but new one need to display immediately! */
        gt_disp_invalid_area(obj);
    }

    obj->area.h = font_size + (font_size >> 1);
    if (GT_TOAST_POS_TOP == style->position) {
        /** (100% / 6) - h/2 */
        obj->area.y = (gt_disp_get_res_ver(NULL) / 6) - (obj->area.h >> 1);
    } else if (GT_TOAST_POS_BOTTOM == style->position) {
        obj->area.y = (gt_disp_get_res_ver(NULL) * 5 / 6) - (obj->area.h >> 1);
    } else {
        obj->area.y = (gt_disp_get_res_ver(NULL) >> 1) - (obj->area.h >> 1);
    }

    if (max_w > str_len + font_size) {
        obj->area.w = str_len + font_size;
        obj->area.x = (font_size + max_w - obj->area.w) >> 1;
        gt_label_set_auto_scroll_single_line(style->lab_p, false);
        gt_label_set_font_align(style->lab_p, GT_ALIGN_CENTER_MID);
    } else {
        obj->area.w = max_w;
        obj->area.x = font_size >> 1;
        str_len = obj->area.w - font_size;
        gt_label_set_auto_scroll_single_line(style->lab_p, true);
        gt_label_set_font_align(style->lab_p, GT_ALIGN_LEFT_MID);
    }

    gt_obj_set_size(style->lab_p, str_len, obj->area.h);
    gt_obj_set_pos_relative(style->lab_p, obj, font_size >> 1, 0);
}

static void GT_ATTRIBUTE_RAM_TEXT
_reset_global_setting(void) {
    _gt_toast_st * style = _toast_create();
    gt_obj_st * obj = (gt_obj_st * )style;
    if (false == gt_obj_is_type((gt_obj_st * )obj, OBJ_TYPE)) {
        GT_LOGE(GT_LOG_TAG_DATA, "Create toast failed.");
        return;
    }
    gt_color_t fc = gt_label_get_font_color(style->lab_p);
    gt_color_t white = gt_color_white();
    if (fc.full != white.full) {
        gt_label_set_font_color(style->lab_p, white);
    }
    obj->focus_dis = false;
    obj->show_bg = true;
    obj->visible = GT_INVISIBLE;
    obj->bgcolor = gt_color_black();
    obj->opa = GT_OPA_0;
    style->max_opa = GT_OPA_100;
    style->duration = 2000;
    style->position = GT_TOAST_POS_MIDDLE;
    gt_obj_set_visible(obj, GT_INVISIBLE);
}

/* global functions / API interface -------------------------------------*/
void gt_toast_reset(gt_toast_param_st * toast)
{
    if (NULL == toast) {
        _reset_global_setting();
        return;
    }
    toast->message = NULL;
    toast->bg_color = gt_color_black();
    toast->font_color = gt_color_white();
    toast->opa = GT_OPA_100;
    toast->duration = 2000;
    toast->position = GT_TOAST_POS_MIDDLE;
}

void gt_toast(gt_toast_param_st * toast)
{
    _gt_toast_st * style = _toast_create();
    if (false == gt_obj_is_type((gt_obj_st * )style, OBJ_TYPE)) {
        GT_LOGE(GT_LOG_TAG_DATA, "Create toast failed.");
        return;
    }
    gt_obj_bring_to_front((gt_obj_st * )style);
    _common_update_toast_data(style, toast);
    _calc_position_by_type(style);
    _create_transit_anim(style, true);
}


void gt_toast_loading(gt_toast_param_st * toast)
{
    _gt_toast_st * style = _toast_create();
    if (false == gt_obj_is_type((gt_obj_st * )style, OBJ_TYPE)) {
        GT_LOGE(GT_LOG_TAG_DATA, "Create toast failed.");
        return;
    }
    gt_obj_bring_to_front((gt_obj_st * )style);
    _common_update_toast_data(style, toast);
    _calc_position_by_type(style);
    _create_transit_anim(style, true);
}

bool gt_toast_is_showing(void)
{
    gt_obj_st * toast = _get_toast_obj();
    if (NULL == toast) {
        return false;
    }
    return GT_VISIBLE == gt_obj_get_visible(toast);
}

void gt_toast_close(void)
{
    _gt_toast_st * style = _toast_create();
    gt_obj_st * obj = (gt_obj_st * )style;
    if (false == gt_obj_is_type(obj, OBJ_TYPE)) {
        GT_LOGE(GT_LOG_TAG_DATA, "Create toast failed.");
        return;
    }
    if (style->anim_transit) {
        gt_anim_del_safe_by(obj);
        style->anim_transit = NULL;
    }
    gt_obj_set_visible(obj, GT_INVISIBLE);
    gt_obj_set_opa(obj, GT_OPA_0);
}

void gt_toast_global_set_font_size(uint8_t size)
{
    _gt_toast_st * style = _toast_create();
    if (false == gt_obj_is_type((gt_obj_st * )style, OBJ_TYPE)) {
        return ;
    }
    gt_label_set_font_size(style->lab_p, size);
}

void gt_toast_global_set_font_gray(uint8_t gray)
{
    _gt_toast_st * style = _toast_create();
    if (false == gt_obj_is_type((gt_obj_st * )style, OBJ_TYPE)) {
        return ;
    }
    gt_label_set_font_gray(style->lab_p, gray);
}

void gt_toast_global_set_font_scale_size(uint8_t scale_size)
{
    _gt_toast_st * style = _toast_create();
    if (false == gt_obj_is_type((gt_obj_st * )style, OBJ_TYPE)) {
        return ;
    }
    gt_label_set_font_scale_size(style->lab_p, scale_size);
}

void gt_toast_set_indent(uint16_t indent)
{
    _gt_toast_st * style = _toast_create();
    if (false == gt_obj_is_type((gt_obj_st * )style, OBJ_TYPE)) {
        return ;
    }
    gt_label_set_indent(style->lab_p, indent);
}

void gt_toast_global_set_font_align(gt_align_et align)
{
    _gt_toast_st * style = _toast_create();
    if (false == gt_obj_is_type((gt_obj_st * )style, OBJ_TYPE)) {
        return ;
    }
    gt_label_set_font_align(style->lab_p, align);
}
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
void gt_toast_global_set_font_family_cn(gt_family_t family)
{
    _gt_toast_st * style = _toast_create();
    if (false == gt_obj_is_type((gt_obj_st * )style, OBJ_TYPE)) {
        return ;
    }
    gt_label_set_font_family_cn(style->lab_p, family);
}
void gt_toast_global_set_font_family_en(gt_family_t family)
{
    _gt_toast_st * style = _toast_create();
    if (false == gt_obj_is_type((gt_obj_st * )style, OBJ_TYPE)) {
        return ;
    }
    gt_label_set_font_family_en(style->lab_p, family);
}

void gt_toast_global_set_font_family_fl(gt_family_t family)
{
    _gt_toast_st * style = _toast_create();
    if (false == gt_obj_is_type((gt_obj_st * )style, OBJ_TYPE)) {
        return ;
    }
    gt_label_set_font_family_fl(style->lab_p, family);
}
void gt_toast_global_set_font_family_numb(gt_family_t family)
{
    _gt_toast_st * style = _toast_create();
    if (false == gt_obj_is_type((gt_obj_st * )style, OBJ_TYPE)) {
        return ;
    }
    gt_label_set_font_family_numb(style->lab_p, family);
}
#else
void gt_toast_global_set_font_family(gt_family_t family)
{
    _gt_toast_st * style = _toast_create();
    if (false == gt_obj_is_type((gt_obj_st * )style, OBJ_TYPE)) {
        return ;
    }
    gt_label_set_font_family(style->lab_p, family);
}
void gt_toast_global_set_font_cjk(gt_obj_st* label, gt_font_cjk_et cjk)
{
    _gt_toast_st * style = _toast_create();
    if (false == gt_obj_is_type((gt_obj_st * )style, OBJ_TYPE)) {
        return ;
    }
    gt_label_set_font_cjk(style->lab_p, cjk);
}
#endif

void gt_toast_global_set_font_thick_en(uint8_t thick)
{
    _gt_toast_st * style = _toast_create();
    if (false == gt_obj_is_type((gt_obj_st * )style, OBJ_TYPE)) {
        return ;
    }
    gt_label_set_font_thick_en(style->lab_p, thick);
}

void gt_toast_global_set_font_thick_cn(uint8_t thick)
{
    _gt_toast_st * style = _toast_create();
    if (false == gt_obj_is_type((gt_obj_st * )style, OBJ_TYPE)) {
        return ;
    }
    gt_label_set_font_thick_cn(style->lab_p, thick);
}

void gt_toast_global_set_font_encoding(gt_encoding_et encoding)
{
    _gt_toast_st * style = _toast_create();
    if (false == gt_obj_is_type((gt_obj_st * )style, OBJ_TYPE)) {
        return ;
    }
    gt_label_set_font_encoding(style->lab_p, encoding);
}

void gt_toast_global_set_space(uint8_t space_x, uint8_t space_y)
{
    _gt_toast_st * style = _toast_create();
    if (false == gt_obj_is_type((gt_obj_st * )style, OBJ_TYPE)) {
        return ;
    }
    gt_label_set_space(style->lab_p, space_x, space_y);
}

void gt_toast_global_set_font_style(gt_font_style_et font_style)
{
    _gt_toast_st * style = _toast_create();
    if (false == gt_obj_is_type((gt_obj_st * )style, OBJ_TYPE)) {
        return ;
    }
    gt_label_set_font_style(style->lab_p, font_style);
}

uint8_t gt_toast_global_get_font_size(void)
{
    _gt_toast_st * style = _toast_create();
    if (false == gt_obj_is_type((gt_obj_st * )style, OBJ_TYPE)) {
        return 0;
    }
    return gt_label_get_font_size(style->lab_p);
}

void gt_toast_global_set_font_info(gt_font_info_st * font_info)
{
    _gt_toast_st * style = _toast_create();
    if (false == gt_obj_is_type((gt_obj_st * )style, OBJ_TYPE)) {
        GT_LOGE(GT_LOG_TAG_DATA, "Create toast failed.");
        return;
    }
    gt_label_set_font_info(style->lab_p, font_info);
}

gt_font_info_st * gt_toast_global_get_font_info(void)
{
    _gt_toast_st * toast = _toast_create();
    if (false == gt_obj_is_type((gt_obj_st * )toast, OBJ_TYPE)) {
        GT_LOGE(GT_LOG_TAG_DATA, "Create toast failed.");
        return NULL;
    }
    return gt_label_get_font_info(toast->lab_p);
}


#endif  /** GT_CFG_ENABLE_TOAST */
/* end ------------------------------------------------------------------*/
