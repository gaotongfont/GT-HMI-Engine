/**
 * @file gt_checkbox.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-18 14:11:47
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_checkbox.h"
#include "../core/gt_mem.h"
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
#define OBJ_TYPE    GT_TYPE_CHECKBOX
#define MY_CLASS    &gt_checkbox_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_checkbox_s
{
    char *      text;
    gt_color_t  font_color;
    gt_font_info_st font_info;
    uint8_t     font_align;
    uint8_t     space_x;
    uint8_t     space_y;
}_gt_checkbox_st;

/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_checkbox_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_checkbox_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static inline void _gt_checkbox_init_widget(gt_obj_st * checkbox) {
    _gt_checkbox_st * style = checkbox->style;
    gt_font_st font = {
        .info     = style->font_info,
        .res      = NULL,
        .utf8     = style->text,
        .len      = strlen(style->text),

    };
    font.info.thick_en = style->font_info.thick_en == 0 ? style->font_info.size + 6: style->font_info.thick_en;
    font.info.thick_cn = style->font_info.thick_cn == 0 ? style->font_info.size + 6: style->font_info.thick_cn;

    font.info.encoding = gt_project_encoding_get();

    // set default size
    if( checkbox->area.w == 0 || checkbox->area.h == 0){
        checkbox->area.w = style->font_info.size+4 + strlen(style->text)*11;
        checkbox->area.h = style->font_info.size+4;
    }

    /* base shape */
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.bg_opa = checkbox->opa;
    if( checkbox->state == GT_STATE_PRESSED ){
        rect_attr.reg.is_fill = 1;
        rect_attr.border_width = 0;
        rect_attr.bg_color = gt_color_hex(0x409EFF);
        rect_attr.border_color = gt_color_hex(0x409EFF);
    }else{
        rect_attr.reg.is_fill = 1;
        rect_attr.border_width = 2;
        rect_attr.bg_color = gt_color_white();
        rect_attr.border_color = gt_color_hex(0x409EFF);
    }
    gt_area_st area = gt_area_reduce(checkbox->area , REDUCE_DEFAULT);

    gt_area_st area_box = area;
    area_box.w = style->font_info.size+4;
    area_box.h = area_box.w;
    // area_box.y = area.y + ((area.h - area_box.h) >> 1);
    area_box.x = area.x;

   /* base shape */
    draw_bg(checkbox->draw_ctx, &rect_attr, &area_box);

    /* font */
    gt_attr_font_st font_dsc = {
        .font           = &font,
        .space_x        = style->space_x,
        .space_y        = style->space_y,
        .font_color     = style->font_color,
        .align          = style->font_align,
        .opa            = checkbox->opa,
    };
    area.x += (area_box.w + 2);
    area.y += 2;
    area.w = area.w >  area_box.w ? (area.w - area_box.w) : 0;
    draw_text(checkbox->draw_ctx, &font_dsc, &area);

     // focus
    draw_focus(checkbox , 0);

}

/**
 * @brief obj init checkbox widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_checkbox_init_widget(obj);
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

    _gt_checkbox_st ** style_p = (_gt_checkbox_st ** )&obj->style;
    if (NULL == *style_p) {
        return ;
    }

    if (NULL != (*style_p)->text) {
        gt_mem_free((*style_p)->text);
        (*style_p)->text = NULL;
    }

    gt_mem_free(*style_p);
    *style_p = NULL;
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

            break;

        case GT_EVENT_TYPE_INPUT_SCROLL:
            GT_LOGV(GT_LOG_TAG_GUI, "scroll");
            break;

        case GT_EVENT_TYPE_INPUT_RELEASED: /* click event finish */
            GT_LOGV(GT_LOG_TAG_GUI, "processed");
            if( gt_obj_get_state(obj) != GT_STATE_NONE ){
                gt_obj_set_state(obj, GT_STATE_NONE);
            }else{
                gt_obj_set_state(obj, GT_STATE_PRESSED);
            }
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
}


static void _gt_checkbox_init_style(gt_obj_st * checkbox)
{
    _gt_checkbox_st * style = (_gt_checkbox_st * )checkbox->style;

    gt_memset(style, 0, sizeof(_gt_checkbox_st));

    style->text = gt_mem_malloc(sizeof("checkbox"));
    gt_memcpy(style->text, "checkbox", sizeof("checkbox"));

    style->font_info.style_cn   = GT_CFG_DEFAULT_FONT_FAMILY_CN;
    style->font_info.style_en   = GT_CFG_DEFAULT_FONT_FAMILY_EN;
    style->font_info.style_fl    = GT_CFG_DEFAULT_FONT_FAMILY_FL;
    style->font_info.style_numb  = GT_CFG_DEFAULT_FONT_FAMILY_NUMB;
    style->font_info.size        = GT_CFG_DEFAULT_FONT_SIZE;
    style->font_color       = gt_color_black();
    style->font_info.gray        = 1;
    style->font_align = GT_ALIGN_NONE;
    style->font_info.thick_en         = 0;
    style->font_info.thick_cn         = 0;
    style->space_x          = 0;
    style->space_y          = 0;
}




/* global functions / API interface -------------------------------------*/

/**
 * @brief create a checkbox obj
 *
 * @param parent checkbox's parent element
 * @return gt_obj_st* checkbox obj
 */
gt_obj_st * gt_checkbox_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_checkbox_init_style(obj);
    return obj;
}


void gt_checkbox_set_text(gt_obj_st * checkbox, const char * fmt, ...)
{
    char buffer[8] = {0};
    va_list args;
    va_list args2;
    va_start(args, fmt);
    va_copy(args2, args);

    _gt_checkbox_st * style = checkbox->style;
    if( style->text ){
        gt_mem_free(style->text);
    }
    uint16_t size = (NULL == fmt) ? 0 : (vsnprintf(buffer, sizeof(buffer), fmt, args) + 1);
    if (!size) {
        goto free_lb;
    }
    style->text = gt_mem_malloc(size);
    vsnprintf(style->text, size, fmt, args2);
    gt_event_send(checkbox, GT_EVENT_TYPE_DRAW_START, NULL);

free_lb:
    va_end(args2);
    va_end(args);
}
char * gt_checkbox_get_text(gt_obj_st * checkbox)
{
    _gt_checkbox_st * style = checkbox->style;
    return style->text;
}

void gt_checkbox_set_font_family_cn(gt_obj_st * checkbox, gt_family_t family)
{
    _gt_checkbox_st * style = checkbox->style;
    style->font_info.style_cn = family;
}

void gt_checkbox_set_font_family_en(gt_obj_st * checkbox, gt_family_t family)
{
    _gt_checkbox_st * style = checkbox->style;
    style->font_info.style_en = family;
}

void gt_checkbox_set_font_family_fl(gt_obj_st * checkbox, gt_family_t family)
{
    _gt_checkbox_st * style = checkbox->style;
    style->font_info.style_fl = family;
}

void gt_checkbox_set_font_family_numb(gt_obj_st * checkbox, gt_family_t family)
{
    _gt_checkbox_st * style = checkbox->style;
    style->font_info.style_numb = family;
}

void gt_checkbox_set_font_size(gt_obj_st * checkbox, uint8_t size)
{
    _gt_checkbox_st * style = checkbox->style;
    style->font_info.size = size;
}
void gt_checkbox_set_font_gray(gt_obj_st * checkbox, uint8_t gray)
{
    _gt_checkbox_st * style = checkbox->style;
    style->font_info.gray = gray;
}
void gt_checkbox_set_font_align(gt_obj_st * checkbox, uint8_t align)
{
    _gt_checkbox_st * style = checkbox->style;
    style->font_align = align;
}
void gt_checkbox_set_font_color(gt_obj_st * checkbox, gt_color_t color)
{
    _gt_checkbox_st * style = checkbox->style;
    style->font_color = color;
    gt_event_send(checkbox, GT_EVENT_TYPE_DRAW_START, NULL);
}
void gt_checkbox_set_font_thick_en(gt_obj_st * checkbox, uint8_t thick)
{
    _gt_checkbox_st * style = (_gt_checkbox_st * )checkbox->style;
    style->font_info.thick_en = thick;
}
void gt_checkbox_set_font_thick_cn(gt_obj_st * checkbox, uint8_t thick)
{
    _gt_checkbox_st * style = (_gt_checkbox_st * )checkbox->style;
    style->font_info.thick_cn = thick;
}
void gt_checkbox_set_space(gt_obj_st * checkbox, uint8_t space_x, uint8_t space_y)
{
    _gt_checkbox_st * style = checkbox->style;
    style->space_x = space_x;
    style->space_y = space_y;
}
/* end ------------------------------------------------------------------*/
