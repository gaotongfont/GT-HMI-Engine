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
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class);
static void _deinit_cb(gt_obj_st * obj,const gt_obj_class_st * class);
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
    static bool flag = 0;
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

static void _gt_input_init_widget(gt_obj_st * input) {
    _gt_style_input_st * style = input->style;
    gt_size_t x, y, w, h;           //base attr

    if( 0 == input->area.w || 0 == input->area.h ){
        input->area.w = style->font_size * 8;
        input->area.h = style->font_size + 4;
    }

    w = input->area.w;
    h = input->area.h;

    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.bg_opa = input->opa;
    rect_attr.is_fill = 0;
    rect_attr.border_width = 2;
    rect_attr.radius = 4;
    rect_attr.bg_color = gt_color_black();
    rect_attr.border_color = gt_color_black();

    /* draw base shape */
    draw_bg(input->draw_ctx, &rect_attr, &input->area);

    /* draw font */
    gt_font_st font;
    gt_color_t color_font;
    if( !style->value ){
        font.style_cn = style->font_style_cn;
        font.style_en = style->font_style_en;
        font.utf8 = style->placeholder;
        font.len = strlen(style->placeholder);
        font.size = style->font_size;
        color_font.full = 0x8410;
    }
    // if input value is not null
    else{
        font.style_cn = style->font_style_cn;
        font.style_en = style->font_style_en;
        font.utf8 = style->value;
        font.len = style->pos_cursor;
        font.size = style->font_size;
        color_font = gt_color_hex(0x000000);
    }
    gt_attr_font_st font_attr = {
        .font = &font,
        .align = GT_ALIGN_NONE,
        .space_x = 0,
        .space_y = 0,
        .font_color = color_font,
        .opa = input->opa,
    };

    gt_area_st area_font = input->area;
    area_font.x += 4;
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
        color_font = gt_color_hex(0x000000);

        font_attr.font = &font,
        font_attr.font_color = color_font,

        font_attr.start_x = area_res.x + 4;
        font_attr.start_y = area_res.y;
        draw_text(input->draw_ctx, &font_attr, &area_font);
    }
}

/**
 * @brief obj init input widget cb
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");
    /* code */
    _gt_input_init_widget(obj);
    GT_LOGV(GT_LOG_TAG_GUI, "end init_cb");
}

/**
 * @brief obj deinit cb
 *
 * @param obj
 */
static void _deinit_cb(gt_obj_st * obj, const gt_obj_class_st * class) {
    GT_LOGV(GT_LOG_TAG_GUI, "start deinit_cb");
    /* code */
    GT_LOGV(GT_LOG_TAG_GUI, "end deinit_cb");
}


/**
 * @brief obj event handler cb
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);
    switch(code) {
        case GT_EVENT_TYPE_DRAW_START:
            GT_LOGD(GT_LOG_TAG_GUI, "start draw");
            if( gt_obj_check_scr(obj) ){
                extern void gt_disp_ref_area(gt_area_st * area);
                gt_disp_ref_area(&obj->area);
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
    _gt_style_input_st * style = NULL;

    input->style = gt_mem_malloc(sizeof(_gt_style_input_st));
    style = input->style;

    gt_memset(style, 0, sizeof(_gt_style_input_st));

    gt_input_style_set_placeholder(input, "please input");

    style->font_style_cn    = GT_CFG_DEFAULT_FONT_STYLE_CN;
    style->font_style_en    = GT_CFG_DEFAULT_FONT_STYLE_EN;
    style->font_size        = GT_CFG_DEFAULT_FONT_SIZE;
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
    if( style->placeholder ){
        gt_mem_free(style->value);
    }
    uint16_t len = strlen(value);
    style->value = gt_mem_malloc( len + 1 );
    gt_memcpy(style->value, value, len);
    style->value[len] = 0;

    style->pos_cursor = len;
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
    uint16_t len = strlen(value);
    if( !style->value ){
        style->value = gt_mem_malloc(len+1);
        gt_memcpy(style->value, value, len);
        style->value[len] = 0;
        style->pos_cursor = len;
    }else{
        // uint16_t len_src = strlen(style->value);
        // style->value = gt_mem_realloc(style->value, len + len_src + 1);
        // gt_memcpy(&style->value[len_src], value, len);
        // style->value[len + len_src] = 0;
        gt_txt_ins(style->value, style->pos_cursor, value);
        style->pos_cursor += strlen(value);
    }

    gt_event_send(input, GT_EVENT_TYPE_DRAW_START, NULL);
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
    gt_txt_cut(style->value, style->pos_cursor - 1, style->pos_cursor);
    style->pos_cursor -= 1;
lab_end:
    gt_event_send(input, GT_EVENT_TYPE_DRAW_START, NULL);
}
/**
 * @brief set placeholder value
 *
 * @param input input obj
 * @param placeholder placeholder value
 */
void gt_input_style_set_placeholder(gt_obj_st * input, const char * placeholder)
{
    _gt_style_input_st * style = input->style;
    if( style->placeholder ){
        gt_mem_free(style->placeholder);
    }
    uint16_t len = strlen(placeholder);
    style->placeholder = gt_mem_malloc( len + 1 );
    gt_memcpy(style->placeholder, placeholder, len);
    style->placeholder[len] = 0;
}


void gt_input_style_move_left_pos_cursor(gt_obj_st * input)
{
    _gt_style_input_st * style = input->style;
    if( style->pos_cursor > 0 ){
        style->pos_cursor--;
    }
    gt_event_send(input, GT_EVENT_TYPE_DRAW_START, NULL);
}
void gt_input_style_move_right_pos_cursor(gt_obj_st * input)
{
    _gt_style_input_st * style = input->style;
    if( style->pos_cursor < strlen(style->value) ){
        style->pos_cursor++;
    }
    gt_event_send(input, GT_EVENT_TYPE_DRAW_START, NULL);
}
/* end ------------------------------------------------------------------*/
