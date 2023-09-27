/**
 * @file gt_textarea.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-11 15:03:35
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_textarea.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_TEXTAREA
#define MY_CLASS    &gt_textarea_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_txt_s{
    char *      text;
    gt_color_t  color;
    uint16_t    len;
    uint8_t     mask_style;
}_gt_txt_st;

typedef struct _gt_textarea_s{
    _gt_txt_st * contents;
    uint16_t    cnt_contents;

    gt_color_t  color_background;
    uint8_t     bg_opa;     /* @ref gt_color.h */

    uint8_t     space_x;
    uint8_t     space_y;

    gt_family_t font_family_cn;
    gt_family_t font_family_en;
    gt_family_t font_family_numb;
    uint8_t     font_size;
    uint8_t     font_gray;
    uint8_t     font_align;
    uint8_t     thick_en;
    uint8_t     thick_cn;
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

static inline void _gt_textarea_init_widget(gt_obj_st * textarea) {
    _gt_textarea_st * style = (_gt_textarea_st * )textarea->style;
    int idx = 0;

    gt_font_st font = {
        .size       = style->font_size,
        .style_cn   = style->font_family_cn,
        .style_en   = style->font_family_en,
        .style_numb = style->font_family_numb,
        .gray       = style->font_gray,
        .res        = NULL,
    };
    font.thick_en = style->thick_en == 0 ? style->font_size + 6: style->thick_en;
    font.thick_cn = style->thick_cn == 0 ? style->font_size + 6: style->thick_cn;
    font.encoding = gt_project_encoding_get();
    gt_attr_font_st font_attr = {
        .font       = &font,
        .space_x    = style->space_x,
        .space_y    = style->space_y,
        .align      = style->font_align,
        .opa        = textarea->opa,
    };

    gt_area_st area_font , area_temp;
    area_font = gt_area_reduce(textarea->area , REDUCE_DEFAULT);

    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.radius = 4;
    rect_attr.border_width = 2;
    rect_attr.reg.is_fill = 1;
    rect_attr.bg_color = style->color_background;
    rect_attr.border_color = gt_color_hex(0x888888);
    rect_attr.bg_opa = style->bg_opa;

    draw_bg(textarea->draw_ctx, &rect_attr, &area_font);

    area_font = gt_area_reduce(textarea->area , rect_attr.border_width + 2);
    while( idx < style->cnt_contents ){
        font.utf8               = style->contents[idx].text;
        font.len                = style->contents[idx].len;
        font_attr.style         = style->contents[idx].mask_style;
        font_attr.font_color    = style->contents[idx].color;
        area_temp = draw_text(textarea->draw_ctx, &font_attr, &area_font);
        font_attr.start_x       = area_temp.x;
        font_attr.start_y       = area_temp.y;
        idx++;
    }

    // focus
    draw_focus(textarea , 0);
}

/**
 * @brief obj init textarea widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_textarea_init_widget(obj);
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

    _gt_textarea_st ** style_p = (_gt_textarea_st ** )&obj->style;
    if (NULL == *style_p) {
        return ;
    }

    if (NULL != (*style_p)->contents) {
        gt_size_t i = (*style_p)->cnt_contents - 1;
        for (; i >= 0; i--) {
            if (NULL == (*style_p)->contents[i].text) {
                continue;
            }
            gt_mem_free((*style_p)->contents[i].text);
            (*style_p)->contents[i].text = NULL;
            (*style_p)->contents[i].len = 0;
        }

        gt_mem_free((*style_p)->contents);
        (*style_p)->contents = NULL;
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

            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_SCROLL:
            GT_LOGV(GT_LOG_TAG_GUI, "scroll");
            break;

        case GT_EVENT_TYPE_INPUT_RELEASED: /* click event finish */
            GT_LOGV(GT_LOG_TAG_GUI, "processed");

            gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
            break;

        default:
            break;
    }
}

static void _gt_textarea_init_style(gt_obj_st * textarea)
{
    _gt_textarea_st * style = (_gt_textarea_st *)textarea->style;

    gt_memset(style, 0, sizeof(_gt_textarea_st ));

    style->font_family_cn    = GT_CFG_DEFAULT_FONT_FAMILY_CN;
    style->font_family_en    = GT_CFG_DEFAULT_FONT_FAMILY_EN;
    style->font_family_numb  = GT_CFG_DEFAULT_FONT_FAMILY_NUMB;
    style->font_size         = GT_CFG_DEFAULT_FONT_SIZE;
    style->color_background  = gt_color_white();
    style->bg_opa            = GT_OPA_100;
    style->font_gray         = 1;
    style->font_align        = GT_ALIGN_NONE;
    style->thick_en          = 0;
    style->thick_cn          = 0;
    style->space_x           = 0;
    style->space_y           = 0;
}

/* global functions / API interface -------------------------------------*/

gt_obj_st * gt_textarea_create(gt_obj_st * parent) {
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_textarea_init_style(obj);
    return obj;
}

void gt_textarea_set_text(gt_obj_st * textarea, char * text)
{
    _gt_textarea_st * style = (_gt_textarea_st * )textarea->style;
    uint8_t style_mask = GT_FONT_STYLE_NONE;
    gt_color_t color = gt_color_black();
    uint16_t len = 0;

    if(style->cnt_contents){
        style_mask = style->contents[0].mask_style;
        color = style->contents[0].color;
        gt_mem_free(style->contents);
    }
    if(text != NULL){
        len = strlen(text);
    }
    style->contents  = gt_mem_malloc( sizeof(_gt_txt_st) );
    style->contents[0].len  = len;
    style->contents[0].text = gt_mem_malloc(style->contents[0].len);
    gt_memcpy(style->contents[0].text, text, len);
    style->contents[0].mask_style = style_mask;
    style->contents[0].color = color;
    style->cnt_contents = 1;

    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

char * gt_textarea_get_text(gt_obj_st * textarea,uint16_t idx)
{
    _gt_textarea_st * style = (_gt_textarea_st * )textarea->style;
    return style->contents[idx].text;
}


void gt_textarea_add_str(gt_obj_st * textarea, char * str, gt_font_style_et style_mask, gt_color_t color)
{
    _gt_textarea_st * style = (_gt_textarea_st * )textarea->style;
    if( style->cnt_contents == 0 ){
        style->contents  = gt_mem_malloc( sizeof(_gt_txt_st) );
        style->contents[0].len  = strlen(str);
        style->contents[0].text = gt_mem_malloc(style->contents[0].len);
        gt_memcpy(style->contents[0].text, str, style->contents[0].len);
        style->contents[0].mask_style = style_mask;
        style->contents[0].color = color;
        style->cnt_contents = 1;
    }else{
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
    _gt_textarea_st * style = (_gt_textarea_st * )textarea->style;
    style->space_x = space_x;
    style->space_y = space_y;

    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_textarea_set_font_size(gt_obj_st * textarea, uint8_t size)
{
    _gt_textarea_st * style = (_gt_textarea_st * )textarea->style;
    style->font_size = size;
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}
void gt_textarea_set_font_gray(gt_obj_st * textarea, uint8_t gray)
{
    _gt_textarea_st * style = (_gt_textarea_st * )textarea->style;
    style->font_gray = gray;
}
void gt_textarea_set_font_align(gt_obj_st * textarea, uint8_t align)
{
    _gt_textarea_st * style = (_gt_textarea_st * )textarea->style;
    style->font_align = align;
}
void gt_textarea_set_font_color(gt_obj_st * textarea, gt_color_t color)
{
    _gt_textarea_st * style = (_gt_textarea_st * )textarea->style;
    uint16_t idx = 0;
    while( idx < style->cnt_contents ){
        style->contents[idx].color = color;
        idx ++;
    }
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_textarea_set_bg_color(gt_obj_st * textarea, gt_color_t color)
{
    _gt_textarea_st * style = (_gt_textarea_st * )textarea->style;
    style->color_background = color;
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_textarea_set_bg_opa(gt_obj_st * textarea, uint8_t opa)
{
    _gt_textarea_st * style = (_gt_textarea_st * )textarea->style;
    style->bg_opa = opa;
    gt_event_send(textarea, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_textarea_set_font_family_cn(gt_obj_st * textarea, gt_family_t family)
{
    _gt_textarea_st * style = (_gt_textarea_st * )textarea->style;
    style->font_family_cn = family;
}

void gt_textarea_set_font_family_en(gt_obj_st * textarea, gt_family_t family)
{
    _gt_textarea_st * style = (_gt_textarea_st * )textarea->style;
    style->font_family_en = family;
}

void gt_textarea_set_font_family_numb(gt_obj_st * textarea, gt_family_t family)
{
    _gt_textarea_st * style = (_gt_textarea_st * )textarea->style;
    style->font_family_numb = family;
}

void gt_textarea_set_font_thick_en(gt_obj_st * textarea, uint8_t thick)
{
    _gt_textarea_st * style = (_gt_textarea_st * )textarea->style;
    style->thick_en = thick;
}
void gt_textarea_set_font_thick_cn(gt_obj_st * textarea, uint8_t thick)
{
    _gt_textarea_st * style = (_gt_textarea_st * )textarea->style;
    style->thick_cn = thick;
}
/* end ------------------------------------------------------------------*/
