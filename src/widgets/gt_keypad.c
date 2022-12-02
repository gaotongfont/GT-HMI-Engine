/**
 * @file gt_keypad.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-08-01 14:05:01
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

 /* include --------------------------------------------------------------*/
#include "gt_keypad.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../font/gt_symbol.h"

#include "gt_input.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_KEYPAD
#define MY_CLASS    &gt_keypad_class


#define SPACE_Y 5
#define SPACE_X 5
/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct _gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_keypad_class = {
    ._init_cb = _init_cb,
    ._deinit_cb = _deinit_cb,
    ._event_cb = _event_cb,
    .type = OBJ_TYPE,
    .size_instance = (sizeof(gt_obj_st)),
    .size_style = (sizeof(_gt_style_keypad_st))
};


gt_keypad_map_st map_keypad_mode_lower[] = {
    "1#",5,   "q",4,  "w",4,  "e",4,  "r",4,  "t",4,  "y",4,  "u",4,  "i",4,  "o",4,  "p",4,  GT_SYMBOL_BACKSPACE,7,  "\n",0,
    "ABC",6,  "a",3,  "s",3,  "d",3,  "f",3,  "g",3,  "h",3,  "j",3,  "k",3,  "l",3,  GT_SYMBOL_NEW_LINE,7, "\n",0,
    "_",1,    "-",1,  "z",1,  "x",1,  "c",1,  "v",1,  "b",1,  "n",1,  "m",1,  ".",1,  ",",1,  ":",1,  "\n",0,
    GT_SYMBOL_KEYBOARD,2,  GT_SYMBOL_LEFT,2,  " ",6,  GT_SYMBOL_RIGHT,2,  GT_SYMBOL_OK,2, NULL,0
};

gt_keypad_map_st map_keypad_mode_upper[] = {
    "1#",5,   "Q",4,  "W",4,  "E",4,  "R",4,  "T",4,  "Y",4,  "U",4,  "I",4,  "O",4,  "P",4,  GT_SYMBOL_BACKSPACE,7,  "\n",0,
    "abc",6,  "A",3,  "S",3,  "D",3,  "F",3,  "G",3,  "H",3,  "J",3,  "K",3,  "L",3,  GT_SYMBOL_NEW_LINE,7, "\n",0,
    "_",1,    "-",1,  "Z",1,  "X",1,  "C",1,  "V",1,  "B",1,  "N",1,  "M",1,  ".",1,  ",",1,  ":",1,  "\n",0,
    GT_SYMBOL_KEYBOARD,2,  GT_SYMBOL_LEFT,2,  " ",6,  GT_SYMBOL_RIGHT,2,  GT_SYMBOL_OK,2, NULL,0
};

gt_keypad_map_st map_keypad_mode_number[] = {
    "1",1, "2",1, "3",1, "4",1, "5",1, "6",1, "7",1, "8",1, "9",1, "0",1, GT_SYMBOL_BACKSPACE,2, "\n",0,
    "abc",2, "+",1, "-",1, "/",1, "*",1, "=",1, "%",1, "!",1, "?",1, "#",1, "<",1, ">",1, "\n",0,
    "\\",1,  "@",1, "$",1, "(",1, ")",1, "{",1, "}",1, "[",1, "]",1, ";",1, "\"",1, "'",1, "\n",0,
    GT_SYMBOL_KEYBOARD,2, GT_SYMBOL_LEFT,2, " ",6, GT_SYMBOL_RIGHT,2, GT_SYMBOL_OK,2, NULL,0
};

/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_area_st _gt_keypad_get_key_area(gt_obj_st * keypad, char * kv);
static void _gt_keypad_update_map(gt_obj_st * keypad, gt_keypad_map_st * map);

static char * _gt_keypad_get_click_kv(gt_obj_st * obj)
{
    _gt_style_keypad_st * style = obj->style;
    gt_keypad_map_st * map = style->info.map;
    int idx = 0;
    gt_area_st area_key;
    while ( map[idx].kv != NULL ) {
        area_key = _gt_keypad_get_key_area(obj, map[idx].kv);
        area_key.y -= obj->area.y;
        area_key.x -= obj->area.x;

        if ((area_key.x < obj->process_attr.point.x) &&
            ((area_key.x + area_key.w) > obj->process_attr.point.x) &&
            (area_key.y < obj->process_attr.point.y) &&
            ((area_key.y + area_key.h) > obj->process_attr.point.y)) {
            return map[idx].kv;
        }
        idx++;
    }
    return NULL;
}


static void _gt_keypad_set_kv_selected(gt_obj_st * keypad, char * kv)
{
    extern void _gt_disp_refr_append_area(gt_area_st * area);

    _gt_style_keypad_st * style = keypad->style;
    gt_area_st area;
    char * kv_old = style->kv_selected;
    style->kv_selected = kv;
    if( kv_old != NULL ){
        area = _gt_keypad_get_key_area(keypad, kv_old);
        _gt_disp_refr_append_area(&area);
    }
    area = _gt_keypad_get_key_area(keypad, kv);
    _gt_disp_refr_append_area(&area);
}

static char * _gt_keypad_get_kv_selected(gt_obj_st * keypad)
{
    _gt_style_keypad_st * style = keypad->style;
    return style->kv_selected;
}


/**
 * @brief get abs area of key
 *
 * @param keypad keypad obj
 * @param kv the key keyvalue
 * @return gt_area_st
 */
static gt_area_st _gt_keypad_get_key_area(gt_obj_st * keypad, char * kv)
{
    gt_area_st area_key = {
        .x = 0,.y = 0,.w = 0,.h = 0
    };
    if( NULL == kv ){
        return area_key;
    }
    _gt_style_keypad_st * style = keypad->style;
    gt_keypad_map_st * map_keypad = style->info.map;
    int idx = 0;

    uint16_t ys = keypad->area.y;
    uint8_t h_key = (keypad->area.h / (style->info.cnt_lines+1)) - SPACE_Y;
    uint8_t per_one_width = (keypad->area.w / style->info.per_line[0]);
    uint8_t line = 0;
    uint8_t r, g, b;

    area_key.h = h_key;
    area_key.y = keypad->area.y + SPACE_Y;
    area_key.x = keypad->area.x + SPACE_X;
    while ( map_keypad[idx].kv != NULL ) {

        area_key.w = map_keypad[idx].w * per_one_width - SPACE_X;

        if( map_keypad[idx+1].kv && strcmp(map_keypad[idx+1].kv, "\n") == 0 ){
            if( (area_key.x + area_key.w) > (keypad->area.x + keypad->area.w - SPACE_X) ){
                area_key.w = (keypad->area.x + keypad->area.w - SPACE_X) - area_key.x;
            }
            if( (area_key.x + area_key.w) < (keypad->area.x + keypad->area.w - SPACE_X) ){
                area_key.w += ((keypad->area.x + keypad->area.w - SPACE_X) - (area_key.x + area_key.w));
            }
        }
        if( strcmp(map_keypad[idx].kv, kv) == 0 ){
            return area_key;
        }

        idx++;
        area_key.x += area_key.w + SPACE_X;
        if( map_keypad[idx].kv && strcmp(map_keypad[idx].kv, "\n") == 0 ){
            area_key.x = keypad->area.x + SPACE_X;
            area_key.y += h_key + SPACE_Y;
            line++;
            idx++;
            per_one_width = (keypad->area.w / style->info.per_line[line]);
        }
    }
    gt_memset(&area_key, 0, sizeof(gt_area_st));
    return area_key;
}

static void _gt_keypad_input_event_cb(gt_obj_st * keypad)
{
    GT_ASSERT(keypad);
    _gt_style_keypad_st * style = keypad->style;
    if( style->info.handler_cb != NULL ){
        void (* handler_cb)(gt_obj_st * );
        handler_cb = style->info.handler_cb;
        handler_cb(keypad);
        return;
    }

    if( style->kv_selected == NULL ){
        return;
    }

    if( strcmp(style->kv_selected, "ABC") == 0 ){
        _gt_keypad_update_map(keypad, map_keypad_mode_upper);
    }else if( strcmp(style->kv_selected, "1#") == 0 ){
        _gt_keypad_update_map(keypad, map_keypad_mode_number);
    }else if( strcmp(style->kv_selected, "abc") == 0 ){
        _gt_keypad_update_map(keypad, map_keypad_mode_lower);
    }

    if( style->target == NULL ){
        return;
    }

    if(strcmp(style->kv_selected, GT_SYMBOL_BACKSPACE) == 0 ){
        gt_input_style_del_value(style->target);
    }

    else if( strcmp(style->kv_selected, GT_SYMBOL_LEFT) == 0 ){
        gt_input_style_move_left_pos_cursor(style->target);
    }
    else if( strcmp(style->kv_selected, GT_SYMBOL_RIGHT) == 0 ){
        gt_input_style_move_right_pos_cursor(style->target);
    }
    else if( strcmp(style->kv_selected, GT_SYMBOL_NEW_LINE) == 0 ){
        gt_input_style_append_value(style->target, "\n");
    }else{
        gt_input_style_append_value(style->target, style->kv_selected);
    }
}

static bool _gt_keypad_check_key_is_ctl(char * kv){
    if( kv == NULL ){
        return false;
    }
    char * list[] = {
        GT_SYMBOL_NEW_LINE,
        GT_SYMBOL_KEYBOARD,
        GT_SYMBOL_LEFT,
        GT_SYMBOL_RIGHT,
        GT_SYMBOL_OK,
        GT_SYMBOL_BACKSPACE,
        "ABC",
        "abc",
        "1#",
        NULL,
    };
    int idx = 0;
    while( list[idx] != NULL ){
        if( strcmp(list[idx], kv) == 0 ){
            return true;
        }
        idx++;
    }
    return false;
}

static void _gt_keypad_init_widget(gt_obj_st * keypad)
{
    _gt_style_keypad_st * style = keypad->style;
    int idx = 0, cnt_temp = 0;
    gt_color_t color, * color_widget_p = NULL;

    gt_keypad_map_st * map_keypad = style->info.map;

    gt_font_st font = {
        .style_cn   = style->font_family_cn,
        .style_en   = style->font_family_en,
        .size       = style->font_size,
    };

    gt_attr_rect_st rect_attr;
    rect_attr.is_fill       = 1;
    rect_attr.bg_opa        = keypad->opa;
    rect_attr.border_width  = 0;
    rect_attr.radius        = 4;
    rect_attr.bg_color      = style->color_board;

    draw_bg(keypad->draw_ctx, &rect_attr, &keypad->area);

    gt_attr_font_st font_attr = {
        .font       = &font,
        .space_x    = 0,
        .space_y    = 0,
        .align      = GT_ALIGN_CENTER,
        .font_color = style->font_color,
        .opa        = keypad->opa,
    };

    gt_area_st area_key, area_bg;

    uint16_t ys = keypad->area.y;
    uint8_t h_key = ( (keypad->area.h ) / (style->info.cnt_lines+1)) - SPACE_Y;
    uint8_t per_one_width = (keypad->area.w / style->info.per_line[0]);
    uint8_t line = 0;
    uint8_t r, g, b;

    area_key.h = h_key;
    area_key.y = keypad->area.y + SPACE_Y;
    area_key.x = keypad->area.x + SPACE_X;
    while ( map_keypad[idx].kv != NULL ) {

        rect_attr.bg_color = style->color_key;
        if( _gt_keypad_check_key_is_ctl(map_keypad[idx].kv) ){
            rect_attr.bg_color = style->color_ctrl;
        }

        if ( _gt_keypad_get_kv_selected(keypad) && strcmp( map_keypad[idx].kv, _gt_keypad_get_kv_selected(keypad)) == 0  ){
            r = GT_COLOR_GET_R(rect_attr.bg_color) >> 1;
            g = GT_COLOR_GET_G(rect_attr.bg_color) >> 1;
            b = GT_COLOR_GET_B(rect_attr.bg_color) >> 1;
            GT_COLOR_SET_RGB(rect_attr.bg_color, r, g, b);
        }

        area_key.w = map_keypad[idx].w * per_one_width - SPACE_X;

        if( (map_keypad[idx+1].kv && strcmp(map_keypad[idx+1].kv, "\n") == 0) || map_keypad[idx+1].kv == NULL ){
            if( (area_key.x + area_key.w) > (keypad->area.x + keypad->area.w - SPACE_X) ){
                area_key.w = (keypad->area.x + keypad->area.w - SPACE_X) - area_key.x;
            }
            if( (area_key.x + area_key.w) < (keypad->area.x + keypad->area.w - SPACE_X) ){
                area_key.w += ((keypad->area.x + keypad->area.w - SPACE_X) - (area_key.x + area_key.w));
            }
        }
        draw_bg(keypad->draw_ctx, &rect_attr, &area_key);

        font.utf8 = map_keypad[idx].kv;
        font.len = strlen(map_keypad[idx].kv);

        draw_text(keypad->draw_ctx, &font_attr, &area_key);
        idx++;

        area_key.x += area_key.w + SPACE_X;

        if(map_keypad[idx].kv && strcmp(map_keypad[idx].kv, "\n") == 0 ){
            area_key.x = keypad->area.x + SPACE_X;
            area_key.y += h_key + SPACE_Y;
            line++;
            idx++;
            per_one_width = (keypad->area.w / style->info.per_line[line]);
        }
    }
}

/**
 * @brief obj init keypad widget cb
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj, const gt_obj_class_st * class) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_keypad_init_widget(obj);
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

    _gt_style_keypad_st ** style_p = (_gt_style_keypad_st *)&obj->style;
    if (NULL == *style_p) {
        return ;
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

    static gt_area_st area;
    static char code_pre = 0;
    switch (code) {
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
        if (gt_obj_check_scr(obj)) {
            _gt_keypad_set_kv_selected(obj, _gt_keypad_get_click_kv(obj));
            _gt_keypad_input_event_cb(obj);
        }
        break;

    case GT_EVENT_TYPE_INPUT_SCROLL:
        GT_LOGD(GT_LOG_TAG_GUI, "scroll");
        break;

    case GT_EVENT_TYPE_INPUT_PROCESS:   /* remove clicking style and process click event */
        GT_LOGD(GT_LOG_TAG_GUI, "process");
        /* code here */
        gt_event_send(obj, GT_EVENT_TYPE_INPUT_PROCESSED, NULL);
        break;
    case GT_EVENT_TYPE_INPUT_PROCESS_LOST:
    case GT_EVENT_TYPE_INPUT_PROCESSED: /* click event finish */
        GT_LOGD(GT_LOG_TAG_GUI, "processed");
        if(gt_obj_check_scr(obj)) {
            _gt_keypad_set_kv_selected(obj, NULL);
        }
        break;

    default:
        break;
    }
}

static void _gt_keypad_init_style(gt_obj_st * keypad)
{
    _gt_style_keypad_st * style = (_gt_style_keypad_st * )keypad->style;

    gt_memset(style, 0, sizeof(_gt_style_keypad_st));
    _gt_keypad_update_map(keypad, map_keypad_mode_lower);

    keypad->area.w = GT_SCREEN_WIDTH;
    keypad->area.h = GT_SCREEN_HEIGHT>>1;

    style->font_color      = gt_color_hex(0);
    style->font_family_cn  = GT_CFG_DEFAULT_FONT_FAMILY_CN;
    style->font_family_en  = GT_CFG_DEFAULT_FONT_FAMILY_EN;
    style->font_size       = GT_CFG_DEFAULT_FONT_SIZE;

    style->color_board  = gt_color_hex(0xA8A8A8);
    style->color_key    = gt_color_hex(0xFFFFFF);
    style->color_ctrl   = gt_color_hex(0x9699a1);
}



static void _gt_keypad_update_map(gt_obj_st * keypad, gt_keypad_map_st * map)
{
    _gt_style_keypad_st * style = keypad->style;
    style->info.map = map;
    if( map == map_keypad_mode_lower ){
        style->info.mode = GT_KEYPAD_MODE_LOWER;
    }else if( map == map_keypad_mode_upper ){
        style->info.mode = GT_KEYPAD_MODE_UPPER;
    }else if( map == map_keypad_mode_number ){
        style->info.mode = GT_KEYPAD_MODE_NUMBER;
    }else{
        style->info.mode = GT_KEYPAD_MODE_OTHER;
    }

    int idx = 0;
    uint8_t cnt_lines = 0, cnt_keys = 0, per_all = 0;
    while( map[idx].kv != NULL ){
        cnt_keys++;
        per_all += map[idx].w;
        if( strcmp(map[idx].kv, "\n") == 0 ){
            style->info.per_line[cnt_lines] = per_all;
            cnt_lines++;
            per_all = 0;
        }
        idx++;
    }
    style->info.per_line[cnt_lines] = per_all;
    style->info.cnt_lines = cnt_lines;

    gt_event_send(keypad, GT_EVENT_TYPE_DRAW_START, NULL);
}


/* global functions / API interface -------------------------------------*/

/**
 * @brief create a keypad obj
 *
 * @param parent keypad's parent element
 * @return gt_obj_st* keypad obj
 */
gt_obj_st * gt_keypad_create(gt_obj_st * parent)
{
    gt_obj_st * obj = gt_obj_class_create(MY_CLASS, parent);
    _gt_keypad_init_style(obj);
    return obj;
}

/**
 * @brief set keypad input obj,keypad input character will add to ta
 *
 * @param keypad
 * @param ta
 */
void gt_keypad_style_set_target(gt_obj_st * keypad, gt_obj_st * ta)
{
    _gt_style_keypad_st * style = keypad->style;
    style->target = ta;
    gt_event_send(keypad, GT_EVENT_TYPE_DRAW_START, NULL);
}


/**
 * @brief set keypad of map
 *
 * @param keypad
 * @param map
 */
void gt_keypad_style_set_map(gt_obj_st * keypad,  gt_keypad_map_st * map)
{
    _gt_style_keypad_st * style = keypad->style;
    style->info.map = map;
    _gt_keypad_update_map(keypad, style->info.map);
}

void gt_keypad_style_set_handler_cb(gt_obj_st * keypad, keypad_handler_cb_t cb)
{
    _gt_style_keypad_st * style = keypad->style;
    style->info.handler_cb = cb;
}


void gt_keypad_style_set_color_board(gt_obj_st * keypad, gt_color_t color)
{
    _gt_style_keypad_st * style = keypad->style;
    style->color_board = color;
}
void gt_keypad_style_set_color_key(gt_obj_st * keypad, gt_color_t color)
{
    _gt_style_keypad_st * style = keypad->style;
    style->color_key = color;
}
void gt_keypad_style_set_color_ctrl_key(gt_obj_st * keypad, gt_color_t color)
{
    _gt_style_keypad_st * style = keypad->style;
    style->color_ctrl = color;
}
void gt_keypad_style_set_font_color(gt_obj_st * keypad, gt_color_t color)
{
    _gt_style_keypad_st * style = keypad->style;
    style->font_color = color;
    gt_event_send(keypad, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_keypad_style_set_font_family_cn(gt_obj_st * keypad, gt_family_t family)
{
    _gt_style_keypad_st * style = keypad->style;
    style->font_family_cn = family;
}
void gt_keypad_style_set_font_family_en(gt_obj_st * keypad, gt_family_t family)
{
    _gt_style_keypad_st * style = keypad->style;
    style->font_family_en = family;
}

/* end ------------------------------------------------------------------*/
