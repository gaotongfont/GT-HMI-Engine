/**
 * @file gt_btn.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-11 15:03:35
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_btn.h"
#include "../core/gt_mem.h"
#include "../hal/gt_hal_disp.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../core/gt_event.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_BTN
#define MY_CLASS    &gt_btn_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_btn_reg_s {
    uint8_t fill: 1;
    uint8_t reserved: 7;
}_gt_btn_reg_st;

typedef struct _gt_btn_s
{
    char * text;
    _gt_vector_st * contents;

    gt_color_t color_background;
    gt_color_t color_pressed;

    gt_size_t   radius;

    gt_color_t      font_color;
    uint8_t         font_size;
    gt_family_t     font_family_cn;
    gt_family_t     font_family_en;
    gt_family_t     font_family_numb;
    uint8_t         font_gray;
    uint8_t     font_align;
    uint8_t     fill;
    _gt_btn_reg_st reg;
    uint8_t     thick_en;
    uint8_t     thick_cn;
    uint8_t     space_x;
    uint8_t     space_y;
}_gt_btn_st;


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_btn_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_btn_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static inline void _gt_btn_init_widget(gt_obj_st * btn) {
    _gt_btn_st * style = (_gt_btn_st * )btn->style;
    gt_size_t radius;               //widget attr

    gt_color_t fg_color;
    gt_font_st font = {
        .style_cn   = style->font_family_cn,
        .style_en   = style->font_family_en,
        .style_numb = style->font_family_numb,
        .res        = NULL,
        .utf8       = style->text,
        .len        = strlen(style->text),
        .size       = style->font_size,
        .gray       = style->font_gray,
    };
    font.thick_en = style->thick_en == 0 ? style->font_size + 6: style->thick_en;
    font.thick_cn = style->thick_cn == 0 ? style->font_size + 6: style->thick_cn;

    font.encoding = gt_project_encoding_get();
    // set default size
    if( btn->area.w == 0 || btn->area.h == 0){
        btn->area.w = font.size*strlen(font.utf8) + 32;
        btn->area.h = style->font_size+16;
    }
    if( style->radius == 0 ){
        radius = btn->area.h >> 2;
    }else{
        radius = style->radius;
    }
    if( gt_obj_get_state(btn) != GT_STATE_NONE ){
        fg_color = style->color_pressed;
    }else{
        fg_color = style->color_background;
    }

    /* base shape */
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill   = style->reg.fill;
    rect_attr.radius        = radius;
    rect_attr.bg_opa        = btn->opa;
    rect_attr.border_width  = 0;
    rect_attr.fg_color      = fg_color;
    rect_attr.bg_color      = fg_color;
    rect_attr.border_color  = fg_color;

    gt_area_st area = gt_area_reduce(btn->area , REDUCE_DEFAULT);
    // gt_area_copy(&area, btn->area);
    // 1:base shape
    draw_bg(btn->draw_ctx, &rect_attr, &area);

    // 2:font
    gt_attr_font_st font_attr = {
        .font           = &font,
        .space_x        = style->space_x,
        .space_y        = style->space_y,
        .font_color     = style->font_color,
        .align          = style->font_align,
        .opa            = btn->opa,
    };
    draw_text(btn->draw_ctx, &font_attr, &area);

    // focus
    draw_focus(btn , radius);
    
    return;
}

/**
 * @brief obj init btn widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_btn_init_widget(obj);
}

static void _free_contents(_gt_vector_st ** ptr) {
    _gt_vector_free(ptr);
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

    _gt_btn_st ** style_p = (_gt_btn_st ** )&obj->style;
    if (NULL == *style_p) {
        return ;
    }

    if (NULL != (*style_p)->contents) {
        _free_contents(&(*style_p)->contents);
        (*style_p)->contents = NULL;
    }

    if (NULL != (*style_p)->text) {
        gt_mem_free((*style_p)->text);
        (*style_p)->text = NULL;
    }

    gt_mem_free(*style_p);
    *style_p = NULL;
}

static bool _turn_next_content(gt_obj_st * obj) {
    _gt_btn_st * style = (_gt_btn_st * )obj->style;
    if (NULL == style->contents) {
        return false;
    }

    gt_btn_set_text(obj, _gt_vector_turn_next(style->contents));
    return true;
}


/**
 * @brief obj event handler call back
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);

    switch(code) {
        case GT_EVENT_TYPE_DRAW_START:
            GT_LOGV(GT_LOG_TAG_GUI, "start draw");
            gt_disp_invalid_area(obj);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_DRAW_END:
            GT_LOGV(GT_LOG_TAG_GUI, "end draw");
            break;

        case GT_EVENT_TYPE_CHANGE_CHILD_REMOVE: /* remove child from screen but not delete */
            GT_LOGV(GT_LOG_TAG_GUI, "child remove");
			break;

        case GT_EVENT_TYPE_CHANGE_CHILD_DELETE: /* delete child */
            GT_LOGV(GT_LOG_TAG_GUI, "child delete");
            break;

        case GT_EVENT_TYPE_INPUT_PRESSING:   /* add clicking style and process clicking event */
            GT_LOGV(GT_LOG_TAG_GUI, "clicking");
            gt_obj_set_state(obj, GT_STATE_PRESSED);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_SCROLL:
            GT_LOGV(GT_LOG_TAG_GUI, "scroll");
            break;

        case GT_EVENT_TYPE_INPUT_RELEASED: /* click event finish */
            GT_LOGV(GT_LOG_TAG_GUI, "processed");
            gt_obj_set_state(obj, GT_STATE_NONE);
            if (!_turn_next_content(obj)) {
                gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            }
            break;

        case GT_EVENT_TYPE_INPUT_PROCESS_LOST:
            GT_LOGV(GT_LOG_TAG_GUI, "process lost");
            gt_obj_set_state(obj, GT_STATE_NONE);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
}

static void _gt_btn_init_style(gt_obj_st * btn)
{
    _gt_btn_st * style = (_gt_btn_st * )btn->style;

    gt_memset(style, 0, sizeof(_gt_btn_st));

    style->text = gt_mem_malloc(sizeof("btn"));
    strcpy(style->text, "btn");
    style->radius           = 0;
    style->color_pressed    = gt_color_hex(0x1C82D0);    //default color_selected
    style->color_background = gt_color_hex(0x17A7BF);  //default color_unselected
    style->reg.fill         = 1;
    style->font_color       = gt_color_white();        //default color_font
    style->font_family_cn   = GT_CFG_DEFAULT_FONT_FAMILY_CN;
    style->font_family_en   = GT_CFG_DEFAULT_FONT_FAMILY_EN;
    style->font_family_numb = GT_CFG_DEFAULT_FONT_FAMILY_NUMB;
    style->font_size        = GT_CFG_DEFAULT_FONT_SIZE;
    style->font_gray        = 1;
    style->font_align = GT_ALIGN_CENTER_MID;
    style->thick_en         = 0;
    style->thick_cn         = 0;
    style->space_x          = 0;
    style->space_y          = 0;
}

static bool _contents_free_cb(void * item) {
    gt_mem_free(item);
    return true;
}

static bool _contents_equal_cb(void * item, void * target) {
    return strcmp(item, target) ? false : true;
}

static void _contents_init(_gt_btn_st * style) {
    if (style->contents) {
        return;
    }
    _gt_vector_add_free_item_cb(&style->contents, _contents_free_cb);
    _gt_vector_add_equal_item_cb(&style->contents, _contents_equal_cb);
}

/* global functions / API interface -------------------------------------*/

/**
 * @brief create a btn obj
 *
 * @param parent btn's parent element
 * @return gt_obj_st* btn obj
 */
gt_obj_st * gt_btn_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_btn_init_style(obj);
    return obj;
}

void gt_btn_set_color_pressed(gt_obj_st * btn, gt_color_t color)
{
    _gt_btn_st * style = (_gt_btn_st * )btn->style;
    style->color_pressed = color;
    gt_event_send(btn, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btn_set_color_background(gt_obj_st * btn, gt_color_t color)
{
    _gt_btn_st * style = (_gt_btn_st * )btn->style;
    style->color_background = color;
    gt_event_send(btn, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btn_set_text(gt_obj_st * btn, const char * fmt, ...)
{
    char buffer[8] = {0};
    va_list args;
    va_list args2;
    va_start(args, fmt);
    va_copy(args2, args);

    _gt_btn_st * style = (_gt_btn_st * )btn->style;
    if( NULL != style->text ){
        gt_mem_free(style->text);
    }
    uint16_t size = (NULL == fmt) ? 0 : (vsnprintf(buffer, sizeof(buffer), fmt, args) + 1);
    if (!size) {
        goto free_lb;
    }
    style->text = gt_mem_malloc(size);
    vsnprintf(style->text, size, fmt, args2);
    gt_event_send(btn, GT_EVENT_TYPE_DRAW_START, NULL);

free_lb:
    va_end(args2);
    va_end(args);
}

char * gt_btn_get_text(gt_obj_st * btn){
    _gt_btn_st * style = (_gt_btn_st * )btn->style;
    return style->text;
}

void gt_btn_set_font_color(gt_obj_st * btn, gt_color_t color){
    _gt_btn_st * style = (_gt_btn_st * )btn->style;
    style->font_color = color;
    gt_event_send(btn, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_btn_set_font_size(gt_obj_st * btn, uint8_t size)
{
    _gt_btn_st * style = (_gt_btn_st * )btn->style;
    style->font_size = size;
}

void gt_btn_set_font_gray(gt_obj_st * btn, uint8_t gray)
{
    _gt_btn_st * style = (_gt_btn_st * )btn->style;
    style->font_gray = gray;
}

void gt_btn_set_font_align(gt_obj_st * btn, uint8_t align)
{
    _gt_btn_st * style = (_gt_btn_st * )btn->style;
    style->font_align = align;
}

void gt_btn_set_font_family_cn(gt_obj_st * btn, gt_family_t font_family_cn)
{
    _gt_btn_st * style = (_gt_btn_st * )btn->style;
    style->font_family_cn = font_family_cn;
}

void gt_btn_set_font_family_en(gt_obj_st * btn, gt_family_t font_family_en)
{
    _gt_btn_st * style = (_gt_btn_st * )btn->style;
    style->font_family_en = font_family_en;
}

void gt_btn_set_font_family_numb(gt_obj_st * btn, gt_family_t font_family_numb)
{
    _gt_btn_st * style = (_gt_btn_st * )btn->style;
    style->font_family_numb = font_family_numb;
}

void gt_btn_set_radius(gt_obj_st * btn, uint8_t radius)
{
    _gt_btn_st * style = (_gt_btn_st * )btn->style;
    style->radius = radius;
    gt_event_send(btn, GT_EVENT_TYPE_DRAW_START, NULL);
}

bool gt_btn_add_state_content(gt_obj_st * btn, const char * str)
{
    if (!btn) {
        return false;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn->style;
    if (!style) {
        return false;
    }
    if (!str || !strlen(str)) {
        return false;
    }

    _contents_init(style);
    /** set first item */
    if (_gt_vector_get_count(style->contents) <= 0) {
        _gt_vector_add_item(&style->contents, (void *)style->text, strlen(style->text) + 1);
    }
    return _gt_vector_add_item(&style->contents, (void *)str, strlen(str) + 1);
}

bool gt_btn_remove_state_content(gt_obj_st * btn, const char * str)
{
    if (!btn) {
        return false;
    }
    _gt_btn_st * style = (_gt_btn_st * )btn->style;
    if (!style) {
        return false;
    }
    if (!str || !strlen(str)) {
        return false;
    }
    return _gt_vector_remove_item(&style->contents, (void * )str);
}

bool gt_btn_clear_all_state_content(gt_obj_st * obj)
{
    if (NULL == obj) {
        return false;
    }
    _gt_btn_st * style = (_gt_btn_st * )obj->style;
    if (NULL == style) {
        return false;
    }
    return _gt_vector_clear_all_items(style->contents);
}

int16_t gt_btn_get_state_content_index(gt_obj_st * obj)
{
    if (!obj) {
        return -1;
    }
    _gt_btn_st * style = (_gt_btn_st * )obj->style;
    if (NULL == style) {
        return -1;
    }
    return _gt_vector_get_index(style->contents);
}

void gt_btn_set_font_thick_en(gt_obj_st * btn, uint8_t thick)
{
    _gt_btn_st * style = (_gt_btn_st * )btn->style;
    style->thick_en = thick;
}
void gt_btn_set_font_thick_cn(gt_obj_st * btn, uint8_t thick)
{
    _gt_btn_st * style = (_gt_btn_st * )btn->style;
    style->thick_cn = thick;
}
void gt_btn_set_space(gt_obj_st * btn, uint8_t space_x, uint8_t space_y)
{
    _gt_btn_st * style = (_gt_btn_st * )btn->style;
    style->space_x = space_x;
    style->space_y = space_y;
}
/* end ------------------------------------------------------------------*/
