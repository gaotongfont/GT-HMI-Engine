/**
 * @file gt_input.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-07-20 13:37:26
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
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
#include "../core/gt_disp.h"
#include "../others/gt_txt.h"
#include "../hal/gt_hal_indev.h"
#include "../core/gt_indev.h"
/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_INPUT
#define MY_CLASS    &gt_input_class

/* private typedef ------------------------------------------------------*/
typedef struct _gt_input_s
{
    char * value;
    char * placeholder;

    uint16_t pos_selected_start;
    uint16_t pos_selected_end;
    uint16_t pos_cursor;

    gt_point_st point_pos;
    gt_size_t border_width;
    gt_color_t border_color;

    gt_color_t bg_color;
    gt_color_t font_color;

    gt_font_info_st font_info;

    uint8_t     font_align;
    uint8_t     space_x;
    uint8_t     space_y;
}_gt_input_st;


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_input_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_input_st)
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static void _gt_input_cursor_anim(gt_obj_st * input , const gt_area_st* box_area)
{
    // static bool flag = 0;
    _gt_input_st * style = input->style;
    int tmp;
    gt_area_st area = {
        .x = style->point_pos.x,
        .y = style->point_pos.y,
        .w = 2,
        .h = style->font_info.size
    };

    tmp = ((area.y + area.h) - (box_area->y + box_area->h));

    if(tmp > 0){
        tmp = style->font_info.size - tmp;
        if(tmp < (area.h-2)){
            return ;
        }
        area.h = tmp;
    }

    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill = 1;
    rect_attr.bg_color = gt_color_hex(0x666666);

    draw_bg(input->draw_ctx, &rect_attr, &area);
}

static inline void _gt_input_init_widget(gt_obj_st * input) {
    _gt_input_st * style = input->style;

    if( 0 == input->area.w || 0 == input->area.h ){
        input->area.w = style->font_info.size * 8;
        input->area.h = style->font_info.size + 16;
    }


    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.bg_opa = input->opa;
    rect_attr.reg.is_fill = 1;
    rect_attr.border_width = style->border_width;
    rect_attr.radius = (0 == style->border_width) ? 0 : 4;
    rect_attr.bg_color = style->bg_color;
    rect_attr.border_color = style->border_color;

    /* draw base shape */
    gt_area_st box_area = gt_area_reduce(input->area , REDUCE_DEFAULT);
    draw_bg(input->draw_ctx, &rect_attr, &box_area);

    /* draw font */
    gt_font_st font;
    gt_color_t color_font = {0};
    if( !style->value ){
        font.utf8 = style->placeholder;
        font.len = strlen(style->placeholder);
        color_font = gt_color_hex(0x808080);
    }
    // if input value is not null
    else{
        font.utf8 = style->value;
        font.len = style->pos_cursor;
        color_font = style->font_color;

    }
    font.info = style->font_info;
    font.info.thick_en = style->font_info.thick_en == 0 ? style->font_info.size + 6: style->font_info.thick_en;
    font.info.thick_cn = style->font_info.thick_cn == 0 ? style->font_info.size + 6: style->font_info.thick_cn;
    font.info.encoding = gt_project_encoding_get();

    gt_attr_font_st font_attr = {
        .font = &font,
        .align = style->font_align,
        .space_x = style->space_x,
        .space_y = style->space_y,
        .font_color = color_font,
        .opa = input->opa,
    };

    //
    gt_area_st area_font = gt_area_reduce(box_area , style->border_width + 2);
    gt_area_st area_res = draw_text(input->draw_ctx, &font_attr, &area_font);

    /*draw cursor*/
    style->point_pos.x = area_res.x;
    style->point_pos.y = area_res.y;
    // gt_graph_init_rect_attr(&rect_attr);
    // rect_attr.reg.is_fill = 1;
    // rect_attr.bg_color = gt_color_hex(0x666666);
    // area_res.w = 2;
    // area_res.h = style->font_info.size;
    // draw_bg(input->draw_ctx, &rect_attr, &area_res);
    if( style->value ){
        _gt_input_cursor_anim(input , &area_font);
    }

    /*draw text after cursor*/
    if( style->value && style->pos_cursor != strlen(style->value) ){
        font.utf8 = &style->value[style->pos_cursor];
        font.len = strlen(&style->value[style->pos_cursor]);
        font.info.size = style->font_info.size;
        color_font = style->font_color;

        font_attr.font = &font,
        font_attr.font_color = color_font,

        font_attr.start_x = area_res.x + style->border_width + 2;
        font_attr.start_y = area_res.y;
        draw_text(input->draw_ctx, &font_attr, &area_font);
    }

    draw_focus(input , rect_attr.radius);
}

static void _input_key_handler( gt_obj_st* obj, uint32_t key)
{
    switch (key)
    {
    case GT_KEY_UP:
        break;
    case GT_KEY_DOWN:
        break;
    case GT_KEY_RIGHT:
        gt_input_move_right_pos_cursor(obj);
        break;
    case GT_KEY_LEFT:
        gt_input_move_left_pos_cursor(obj);
        break;
    case GT_KEY_ESC:
        break;
    case GT_KEY_DEL:
        break;
    case GT_KEY_BACKSPACE:
        gt_input_del_value(obj);
        break;
    case GT_KEY_ENTER:
        gt_input_append_value(obj ,"\n");
        break;
    case GT_KEY_NEXT:
        break;
    case GT_KEY_PREV:
        break;
    case GT_KEY_HOME:
        break;
    case GT_KEY_END:
        break;
    case GT_KEY_NONE:
        break;
    default:
        if(key >= 0x20 && key <= 0x7E){
            gt_input_append_char(obj , key);
        }
        break;
    }
}

/**
 * @brief obj init input widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_input_init_widget(obj);
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

    _gt_input_st ** style_p = (_gt_input_st ** )&obj->style;
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
 * @brief obj event handler call back
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);
    uint32_t key = gt_indev_get_key();

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
            break;
        case GT_EVENT_TYPE_INPUT_KEY:
            GT_LOGV(GT_LOG_TAG_GUI, "input key");
            _input_key_handler(obj, key);
            break;
        default:
            break;
    }
}


static void _gt_input_init_style(gt_obj_st * input)
{
    _gt_input_st * style = (_gt_input_st * )input->style;

    gt_memset(style, 0, sizeof(_gt_input_st));

    gt_input_set_placeholder(input, "please input");

    style->font_info.style_cn = GT_CFG_DEFAULT_FONT_FAMILY_CN;
    style->font_info.style_en = GT_CFG_DEFAULT_FONT_FAMILY_EN;
    style->font_info.style_fl    = GT_CFG_DEFAULT_FONT_FAMILY_FL;
    style->font_info.style_numb  = GT_CFG_DEFAULT_FONT_FAMILY_NUMB;
    style->font_info.size      = GT_CFG_DEFAULT_FONT_SIZE;
    style->font_color     = gt_color_black();
    style->border_width   = 2;
    style->border_color   = gt_color_black();
	style->bg_color 	  = gt_color_hex(0xffffff);
    style->font_info.gray      = 1;
    style->font_align     = GT_ALIGN_NONE;
    style->font_info.thick_en       = 0;
    style->font_info.thick_cn       = 0;
    style->space_x        = 0;
    style->space_y        = 0;
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
void gt_input_set_value(gt_obj_st * input, const char * fmt, ...)
{
    char buffer[8] = {0};
    va_list args;
    va_list args2;
    va_start(args, fmt);
    va_copy(args2, args);

    _gt_input_st * style = input->style;
    if( style->value ){
        gt_mem_free(style->value);
    }
    uint16_t size = (NULL == fmt) ? 0 : (vsnprintf(buffer, sizeof(buffer), fmt, args) + 1);
    style->value = gt_mem_malloc(size);
    vsnprintf(style->value, size, fmt, args2);

    style->pos_cursor = size - 1;
    gt_event_send(input, GT_EVENT_TYPE_DRAW_START, NULL);

free_lb:
    va_end(args2);
    va_end(args);
}

char * gt_input_get_value(gt_obj_st * input)
{
    _gt_input_st * style = input->style;
    return style->value;
}

/**
 * @brief append value to input obj value
 *
 * @param input input obj
 * @param value append data value
 */
void gt_input_append_value(gt_obj_st * input, char * value)
{
    _gt_input_st * style = input->style;
    uint16_t len = value == NULL ? 0 : strlen(value);
    if( !style->value ){
        style->value = gt_mem_malloc(len+1);
        gt_memcpy(style->value, value, len);
        style->value[len] = 0;
        style->pos_cursor = len;
    }else{
        style->value = gt_txt_ins(style->value, style->pos_cursor, value);
        style->pos_cursor += value == NULL ? 0 : strlen(value);
    }

    gt_event_send(input, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_input_append_char(gt_obj_st * input, char chr)
{
    char tmp[2];
    tmp[0] = chr;
    tmp[1] = '\0';
    gt_input_append_value(input , tmp);
}


void gt_input_append_value_encoding(gt_obj_st * input, char * value , uint8_t encoding)
{
    if(encoding == gt_project_encoding_get())
    {
        gt_input_append_value(input , value);
        return ;
    }

#if GT_CFG_ENABLE_ZK_FONT == 1
    uint16_t len = value == NULL ? 0 : strlen(value);
    uint16_t idx = 0;
    char *tmp_str = gt_mem_malloc(len + 1);

    while(idx < len)
    {
        gt_memset_0(tmp_str, len+1);
        if(encoding == GT_ENCODING_UTF8){
            idx += gt_encoding_table_one_char((uint8_t * )&value[idx], (uint8_t * )tmp_str , UTF8_2_GB);
        }
        else if(encoding == GT_ENCODING_GB){
            idx += gt_encoding_table_one_char((uint8_t * )&value[idx], (uint8_t * )tmp_str , GB_2_UTF8);
        }
        tmp_str[strlen(tmp_str)] = '\0';
        gt_input_append_value(input , tmp_str);
    }

    gt_mem_free(tmp_str);
    tmp_str = NULL;
#endif
}

void gt_input_del_value(gt_obj_st * input)
{
    _gt_input_st * style = input->style;
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

void gt_input_set_placeholder(gt_obj_st * input, const char * placeholder)
{
    _gt_input_st * style = input->style;
    if( style->placeholder ){
        gt_mem_free(style->placeholder);
    }
    uint16_t len = strlen(placeholder);
    style->placeholder = gt_mem_malloc( len + 1 );
    gt_memcpy(style->placeholder, placeholder, len);
    style->placeholder[len] = 0;

    gt_event_send(input, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_input_move_left_pos_cursor(gt_obj_st * input)
{
    _gt_input_st * style = input->style;
    int32_t d_pos = style->pos_cursor - 1;
    if( style->pos_cursor > 0 ){
        d_pos = gt_txt_check_char_numb(style->value , &d_pos);
        style->pos_cursor -= d_pos;
        // style->pos_cursor--;
    }
    gt_event_send(input, GT_EVENT_TYPE_DRAW_START, NULL);
}
void gt_input_move_right_pos_cursor(gt_obj_st * input)
{
    _gt_input_st * style = input->style;
    int32_t d_pos = style->pos_cursor;
    if( style->pos_cursor < strlen(style->value) ){
        d_pos = gt_txt_check_char_numb(style->value , &d_pos);
        style->pos_cursor += d_pos;
        // style->pos_cursor++;
    }
    gt_event_send(input, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_input_set_bg_color(gt_obj_st * input, gt_color_t color)
{
	_gt_input_st * style = input->style;
    style->bg_color = color;
    gt_event_send(input, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_input_set_font_color(gt_obj_st * input, gt_color_t color)
{
    _gt_input_st * style = input->style;
    style->font_color = color;
    gt_event_send(input, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_input_set_font_family_cn(gt_obj_st * input, gt_family_t family)
{
    _gt_input_st * style = input->style;
    style->font_info.style_cn = family;
}

void gt_input_set_font_family_en(gt_obj_st * input, gt_family_t family)
{
    _gt_input_st * style = input->style;
    style->font_info.style_en = family;
}

void gt_input_set_font_family_fl(gt_obj_st * input, gt_family_t family)
{
    _gt_input_st * style = input->style;
    style->font_info.style_fl = family;
}

void gt_input_set_font_family_numb(gt_obj_st * input, gt_family_t family)
{
    _gt_input_st * style = input->style;
    style->font_info.style_numb = family;
}

void gt_input_set_font_size(gt_obj_st * input, uint8_t size)
{
    _gt_input_st * style = input->style;
    style->font_info.size = size;
}
void gt_input_set_font_gray(gt_obj_st * input, uint8_t gray)
{
    _gt_input_st * style = input->style;
    style->font_info.gray = gray;
}
void gt_input_set_font_align(gt_obj_st * input, uint8_t align)
{
    _gt_input_st * style = input->style;
    style->font_align = align;
}
void gt_input_set_border_width(gt_obj_st * input, gt_size_t width)
{
    _gt_input_st * style = input->style;
    style->border_width = width;
    gt_event_send(input, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_input_set_border_color(gt_obj_st * input, gt_color_t color)
{
    _gt_input_st * style = input->style;
    style->border_color = color;
    gt_event_send(input, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_input_set_font_thick_en(gt_obj_st * input, uint8_t thick)
{
    _gt_input_st * style = (_gt_input_st * )input->style;
    style->font_info.thick_en = thick;
}
void gt_input_set_font_thick_cn(gt_obj_st * input, uint8_t thick)
{
    _gt_input_st * style = (_gt_input_st * )input->style;
    style->font_info.thick_cn = thick;
}
void gt_input_set_space(gt_obj_st * input, uint8_t space_x, uint8_t space_y)
{
    _gt_input_st * style = input->style;
    style->space_x = space_x;
    style->space_y = space_y;
}

/* end ------------------------------------------------------------------*/
