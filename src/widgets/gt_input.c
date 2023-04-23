/**
 * @file gt_input.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-20 13:37:26
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_input.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../others/gt_txt.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_INPUT
#define MY_CLASS    &gt_input_class

/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_input_class = {
    ._init_cb = _init_cb,
    ._deinit_cb = _deinit_cb,
    ._event_cb = _event_cb,
    .type = OBJ_TYPE,
    .size_instance = (sizeof(gt_obj_st)),
    .size_style = (sizeof(_gt_style_input_st))
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void _gt_input_cursor_anim(gt_obj_st * input)
{
    // static bool flag = 0;
    _gt_style_input_st * style = input->style;
    gt_area_st area = {
        .x = style->point_pos.x,
        .y = style->point_pos.y,
        .w = 2,
        .h = style->font_size
    };

    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.is_fill = 1;
    rect_attr.bg_color = gt_color_hex(0x666666);
    // if( flag ){
    //     flag = 0;
    //     rect_attr.bg_opa = GT_OPA_COVER;
    // }else{
    //     flag = 1;
    //     rect_attr.bg_opa = GT_OPA_TRANSP;
    // }
    draw_bg(input->draw_ctx, &rect_attr, &area);
}

static inline void _gt_input_init_widget(gt_obj_st * input) {
    _gt_style_input_st * style = input->style;

    if( 0 == input->area.w || 0 == input->area.h ){
        input->area.w = style->font_size * 8;
        input->area.h = style->font_size + 4;
    }


    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.bg_opa = input->opa;
    rect_attr.is_fill = 1;
    rect_attr.border_width = style->border_width;
    rect_attr.radius = (0 == style->border_width) ? 0 : 4;
    rect_attr.bg_color = style->bg_color;
    rect_attr.border_color = style->border_color;

    /* draw base shape */
    draw_bg(input->draw_ctx, &rect_attr, &input->area);

    /* draw font */
    gt_font_st font;
    font.encoding = gt_project_encoding_get();
    gt_color_t color_font;
    if( !style->value ){
        font.style_cn = style->font_family_cn;
        font.style_en = style->font_family_en;
        font.utf8 = style->placeholder;
        font.len = strlen(style->placeholder);
        font.size = style->font_size;
        color_font = gt_color_hex(0x808080);
        font.gray = style->font_gray;
    }
    // if input value is not null
    else{
        font.style_cn = style->font_family_cn;
        font.style_en = style->font_family_en;
        font.utf8 = style->value;
        font.len = style->pos_cursor;
        font.size = style->font_size;
        color_font = style->font_color;
        font.gray = style->font_gray;
    }
    gt_attr_font_st font_attr = {
        .font = &font,
        .align = style->font_align,
        .space_x = 0,
        .space_y = 0,
        .font_color = color_font,
        .opa = input->opa,
    };

    gt_area_st area_font = input->area;
    area_font.x += style->border_width + 2;
    area_font.y += style->border_width + 2;
    gt_area_st area_res = draw_text(input->draw_ctx, &font_attr, &area_font);

    /*draw cursor*/
    style->point_pos.x = area_res.x;
    style->point_pos.y = area_res.y;
    // gt_graph_init_rect_attr(&rect_attr);
    // rect_attr.is_fill = 1;
    // rect_attr.bg_color = gt_color_hex(0x666666);
    // area_res.w = 2;
    // area_res.h = style->font_size;
    // draw_bg(input->draw_ctx, &rect_attr, &area_res);
    if( style->value )
        _gt_input_cursor_anim(input);

    /*draw text after cursor*/
    if( style->value && style->pos_cursor != strlen(style->value) ){
        font.utf8 = &style->value[style->pos_cursor];
        font.len = strlen(&style->value[style->pos_cursor]);
        font.size = style->font_size;
        color_font = style->font_color;

        font_attr.font = &font,
        font_attr.font_color = color_font,

        font_attr.start_x = area_res.x + style->border_width + 2;
        font_attr.start_y = area_res.y;
        draw_text(input->draw_ctx, &font_attr, &area_font);
    }
}

/**
 * @brief obj init input widget cb
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_input_init_widget(obj);
}

/**
 * @brief obj deinit cb
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");

    if (NULL == obj) {
        return ;
    }

    _gt_style_input_st ** style_p = (_gt_style_input_st ** )&obj->style;
    if (NULL == *style_p) {
        return ;
    }

    if (NULL != (*style_p)->value) {
        gt_mem_free((*style_p)->value);
        (*style_p)->value = NULL;
    }

    if (NULL != (*style_p)->placeholder) {
        gt_mem_free((*style_p)->placeholder);
        (*style_p)->placeholder = NULL;
    }

    gt_mem_free(*style_p);
    *style_p = NULL;
}


/**
 * @brief obj event handler cb
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e) {
    extern void _gt_disp_refr_append_area(gt_area_st * area);
    gt_event_type_et code = gt_event_get_code(e);
    switch(code) {
        case GT_EVENT_TYPE_DRAW_START:
            GT_LOGD(GT_LOG_TAG_GUI, "start draw");
            if( gt_obj_check_scr(obj) ){
                _gt_disp_refr_append_area(&obj->area);
            }
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
            break;

        case GT_EVENT_TYPE_DRAW_END:
            GT_LOGD(GT_LOG_TAG_GUI, "end draw");
            break;

        case GT_EVENT_TYPE_CHANGE_CHILD_REMOVE: /* remove child from screen but not delete */
            GT_LOGD(GT_LOG_TAG_GUI, "child remove");
			break;

        case GT_EVENT_TYPE_CHANGE_CHILD_DELETE: /* delete child */
            GT_LOGD(GT_LOG_TAG_GUI, "child delete");
            break;

        case GT_EVENT_TYPE_INPUT_CLICKING:   /* add clicking style and process clicking event */
            GT_LOGD(GT_LOG_TAG_GUI, "clicking");

            break;

        case GT_EVENT_TYPE_INPUT_SCROLL:
            GT_LOGD(GT_LOG_TAG_GUI, "scroll");
            break;

        case GT_EVENT_TYPE_INPUT_PROCESS:   /* remove clicking style and process click event */
            GT_LOGD(GT_LOG_TAG_GUI, "process");
            /* code here */

            gt_event_send(obj, GT_EVENT_TYPE_INPUT_PROCESSED, NULL);
            break;

        case GT_EVENT_TYPE_INPUT_PROCESSED: /* click event finish */
            GT_LOGD(GT_LOG_TAG_GUI, "processed");
            break;

        default:
            break;
    }
}


static void _gt_input_init_style(gt_obj_st * input)
{
    _gt_style_input_st * style = (_gt_style_input_st * )input->style;

    gt_memset(style, 0, sizeof(_gt_style_input_st));

    gt_input_style_set_placeholder(input, "please input");

    style->font_family_cn = GT_CFG_DEFAULT_FONT_FAMILY_CN;
    style->font_family_en = GT_CFG_DEFAULT_FONT_FAMILY_EN;
    style->font_size      = GT_CFG_DEFAULT_FONT_SIZE;
    style->font_color     = gt_color_black();
    style->border_width   = 2;
    style->border_color   = gt_color_black();
	style->bg_color 	  = gt_color_hex(0xffffff);
    style->font_gray        = 1;
    style->font_align = GT_ALIGN_NONE;
}




/* global functions / API interface -------------------------------------*/

/**
 * @brief create a input obj
 *
 * @param parent input's parent element
 * @return gt_obj_st* input obj
 */
gt_obj_st * gt_input_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_input_init_style(obj);
    return obj;
}

/**
 * @brief set input value
 *
 * @param input input obj
 * @param value set data value
 */
void gt_input_style_set_value(gt_obj_st * input, char * value)
{
    _gt_style_input_st * style = input->style;
    if( style->value ){
        gt_mem_free(style->value);
    }
    uint16_t len = value == NULL ? 0 : strlen(value);
    style->value = gt_mem_malloc( len + 1 );
    GT_CHECK_BACK(style->value);
    gt_memcpy(style->value, value, len);
    style->value[len] = 0;

    style->pos_cursor = len;
    gt_event_send(input, GT_EVENT_TYPE_DRAW_START, NULL);
}

char * gt_input_style_get_value(gt_obj_st * input)
{
    _gt_style_input_st * style = input->style;
    return style->value;
}

/**
 * @brief append value to input obj value
 *
 * @param input input obj
 * @param value append data value
 */
void gt_input_style_append_value(gt_obj_st * input, char * value)
{
    _gt_style_input_st * style = input->style;
    uint16_t len = value == NULL ? 0 : strlen(value);
    if( !style->value ){
        style->value = gt_mem_malloc(len+1);
        GT_CHECK_BACK(style->value);
        gt_memcpy(style->value, value, len);
        style->value[len] = 0;
        style->pos_cursor = len;
    }else{
        style->value = gt_txt_ins(style->value, style->pos_cursor, value);
        style->pos_cursor += value == NULL ? 0 : strlen(value);
    }

    gt_event_send(input, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_input_style_append_value_encoding(gt_obj_st * input, char * value , uint8_t encoding)
{
    if(encoding == gt_project_encoding_get())
    {
        gt_input_style_append_value(input , value);
        return ;
    }

#if GT_CFG_ENABLE_ZK_FONT == 1
    uint16_t len = value == NULL ? 0 : strlen(value);
    uint16_t idx = 0;
    char *tmp_str = gt_mem_malloc(len + 1);
    GT_CHECK_BACK(tmp_str);

    while(idx < len)
    {
        gt_memset_0(tmp_str, len+1);
        if(encoding == GT_ENCODING_UTF8){
            idx += gt_encoding_table_one_char(&value[idx] , tmp_str , UTF8_2_GB);
        }
        else if(encoding == GT_ENCODING_GB){
            idx += gt_encoding_table_one_char(&value[idx] , tmp_str , GB_2_UTF8);
        }
        tmp_str[strlen(tmp_str)] = '\0';
        gt_input_style_append_value(input , tmp_str);
    }

    gt_mem_free(tmp_str);
    tmp_str = NULL;
#endif
}

void gt_input_style_del_value(gt_obj_st * input)
{
    _gt_style_input_st * style = input->style;
    if( style->value == NULL ){
        return;
    }
    if( strlen(style->value) == 1 ){
        gt_mem_free(style->value);
        style->pos_cursor = 0;
        style->value = NULL;
        goto lab_end;
    }
    if( style->pos_cursor <= 0 ){
        return;
    }
    // style->value = gt_txt_cut(style->value, style->pos_cursor - 1, style->pos_cursor);
    // style->pos_cursor -= 1;

    style->pos_cursor -= gt_txt_cut(style->value, style->pos_cursor - 1, style->pos_cursor);

lab_end:
    gt_event_send(input, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_input_style_set_placeholder(gt_obj_st * input, const char * placeholder)
{
    _gt_style_input_st * style = input->style;
    if( style->placeholder ){
        gt_mem_free(style->placeholder);
    }
    uint16_t len = strlen(placeholder);
    style->placeholder = gt_mem_malloc( len + 1 );
    GT_CHECK_BACK(style->placeholder);
    gt_memcpy(style->placeholder, placeholder, len);
    style->placeholder[len] = 0;

    gt_event_send(input, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_input_style_move_left_pos_cursor(gt_obj_st * input)
{
    _gt_style_input_st * style = input->style;
    int32_t d_pos = style->pos_cursor - 1;
    if( style->pos_cursor > 0 ){
        d_pos = gt_txt_check_char_numb(style->value , &d_pos);
        style->pos_cursor -= d_pos;
        // style->pos_cursor--;
    }
    gt_event_send(input, GT_EVENT_TYPE_DRAW_START, NULL);
}
void gt_input_style_move_right_pos_cursor(gt_obj_st * input)
{
    _gt_style_input_st * style = input->style;
    int32_t d_pos = style->pos_cursor;
    if( style->pos_cursor < strlen(style->value) ){
        d_pos = gt_txt_check_char_numb(style->value , &d_pos);
        style->pos_cursor += d_pos;
        // style->pos_cursor++;
    }
    gt_event_send(input, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_input_style_set_bg_color(gt_obj_st * input, gt_color_t color)
{
	_gt_style_input_st * style = input->style;
    style->bg_color = color;
    gt_event_send(input, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_input_style_set_font_color(gt_obj_st * input, gt_color_t color)
{
    _gt_style_input_st * style = input->style;
    style->font_color = color;
    gt_event_send(input, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_input_style_set_font_family_cn(gt_obj_st * input, gt_family_t family)
{
    _gt_style_input_st * style = input->style;
    style->font_family_cn = family;
}

void gt_input_style_set_font_family_en(gt_obj_st * input, gt_family_t family)
{
    _gt_style_input_st * style = input->style;
    style->font_family_en = family;
}

void gt_input_style_set_font_size(gt_obj_st * input, uint8_t size)
{
    _gt_style_input_st * style = input->style;
    style->font_size = size;
}
void gt_input_style_set_font_gray(gt_obj_st * input, uint8_t gray)
{
    _gt_style_input_st * style = input->style;
    style->font_gray = gray;
}
void gt_input_style_set_font_align(gt_obj_st * input, uint8_t align)
{
    _gt_style_input_st * style = input->style;
    style->font_align = align;
}
void gt_input_style_set_border_width(gt_obj_st * input, gt_size_t width)
{
    _gt_style_input_st * style = input->style;
    style->border_width = width;
    gt_event_send(input, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_input_style_set_border_color(gt_obj_st * input, gt_color_t color)
{
    _gt_style_input_st * style = input->style;
    style->border_color = color;
    gt_event_send(input, GT_EVENT_TYPE_DRAW_START, NULL);
}

/* end ------------------------------------------------------------------*/
