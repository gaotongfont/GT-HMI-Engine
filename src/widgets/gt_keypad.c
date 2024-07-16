/**
 * @file gt_keypad.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-03-04 12:24:51
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_keypad.h"

#if GT_CFG_ENABLE_KEYPAD
#include "gt_btnmap.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "string.h"
#include "../core/gt_graph_base.h"
#include "../core/gt_obj_pos.h"
#include "../font/gt_font.h"
#include "../others/gt_assert.h"
#include "../core/gt_draw.h"
#include "../core/gt_disp.h"
#include "../hal/gt_hal_disp.h"
#include "../font/gt_symbol.h"
#include "../core/gt_obj_scroll.h"
#include "gt_conf_widgets.h"

#include "gt_input.h"


/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_KEYPAD
#define MY_CLASS    &gt_keypad_class


/* private typedef ------------------------------------------------------*/
typedef struct _gt_keypad_s {
    gt_obj_st obj;
    gt_obj_st* btnmap;

    gt_keypad_map_st * map_list;

    gt_color_t bg_color;
    gt_color_t border_color;
    gt_color_t color_ctrl;

    gt_size_t border_width;
    gt_keypad_type_te type;
    gt_keypad_default_style_st def_style;

    uint8_t map_total;
}_gt_keypad_st;

typedef struct _gt_def_style_param_s {
    const gt_keypad_map_st * map_list;

    gt_btnmap_disp_special_btn_cb_t _disp_special_btn_cb;
    gt_btnmap_push_btn_kv_cb_t _push_btn_kv_cb;

    uint32_t bg_color;
    uint32_t border_color;
    uint32_t key_bg_color;
    uint32_t key_border_color;
    uint32_t ctrl_key_bg_color;
    uint32_t ctrl_key_border_color;
    gt_size_t border_width;
    gt_size_t key_border_width;
    gt_size_t ctrl_key_border_width;
    gt_radius_t radius;
    gt_radius_t key_radius;

    uint16_t w;
    uint16_t h;
    uint16_t key_h;

    gt_keypad_default_style_st def_style;

    uint8_t map_total;
    uint8_t key_x_space;
    uint8_t key_y_space;
}_gt_def_style_param_st;
/* static prototypes ----------------------------------------------------*/
static void _init_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);
static void _scrolling_handler(gt_obj_st * obj);
static gt_size_t _gt_get_map_index(gt_keypad_map_st * map_list, uint8_t map_total, gt_keypad_type_te type);


/* static variables -----------------------------------------------------*/
const gt_obj_class_st gt_keypad_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = NULL,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_keypad_st)
};


/* keypad default style -----------------------------------------------------*/
#ifdef _DEF_STYLE_26_KEY

#define _GT_KEYPADS_SPELL_NUMB      (9)
static const char _gt_spell[_GT_KEYPADS_SPELL_NUMB][8] = {
    "spell_0", "spell_1", "spell_2", "spell_3", "spell_4", "spell_5", "spell_6", "spell_7", "spell_8",
};
// h = 34 * 6 + 3 * 7 = 204 + 24 = 225
static const gt_map_st _low_map_default_26_key[] = {
    {".",1}, {",",1}, {"?",1}, {";",1}, {":",1}, {"\'",1}, {"\"",1}, {"@",1}, {"(",1}, {")",1}, {GT_BTNMAP_NEW_LINE,0},
    {"1",1}, {"2",1}, {"3",1}, {"4",1}, {"5",1}, {"6",1}, {"7",1}, {"8",1}, {"9",1}, {"0",1}, {GT_BTNMAP_NEW_LINE,0},
    {"q",1}, {"w",1}, {"e",1}, {"r",1}, {"t",1}, {"y",1}, {"u",1}, {"i",1}, {"o",1}, {"p",1}, {GT_BTNMAP_NEW_LINE,0},
    {NULL,1}, {"a",3}, {"s",3}, {"d",3}, {"f",3}, {"g",3}, {"h",3}, {"j",3}, {"k",3}, {"l",3}, {NULL,1}, {GT_BTNMAP_NEW_LINE,0}, {GT_SYMBOL_LOWER_CASE,5},
    {NULL,1}, {"z",4}, {"x",4}, {"c",4}, {"v",4}, {"b",4}, {"n",4}, {"m",4}, {NULL,1}, {GT_SYMBOL_BACKSPACE,5}, {GT_BTNMAP_NEW_LINE,0},
    {GT_SYMBOL_CH,5}, {GT_SYMBOL_WELL_NUMBER,5}, {GT_SYMBOL_LEFT,4}, {GT_SYMBOL_SPACE,12}, {GT_SYMBOL_RIGHT,4}, {GT_SYMBOL_NEW_LINE,8},
    {NULL,0}
};

static const gt_map_st _up_map_default_26_key[] = {
    {".",1}, {",",1}, {"?",1}, {";",1}, {":",1}, {"\'",1}, {"\"",1}, {"@",1}, {"(",1}, {")",1}, {GT_BTNMAP_NEW_LINE,0},
    {"1",1}, {"2",1}, {"3",1}, {"4",1}, {"5",1}, {"6",1}, {"7",1}, {"8",1}, {"9",1}, {"0",1}, {GT_BTNMAP_NEW_LINE,0},
    {"Q",1}, {"W",1}, {"E",1}, {"R",1}, {"T",1}, {"Y",1}, {"U",1}, {"I",1}, {"O",1}, {"P",1}, {GT_BTNMAP_NEW_LINE,0},
    {NULL,1}, {"A",3}, {"S",3}, {"D",3}, {"F",3}, {"G",3}, {"H",3}, {"J",3}, {"K",3}, {"L",3}, {NULL,1}, {GT_BTNMAP_NEW_LINE,0},
    {GT_SYMBOL_UPPER_CASE,5}, {NULL,1}, {"Z",4}, {"X",4}, {"C",4}, {"V",4}, {"B",4}, {"N",4}, {"M",4}, {NULL,1}, {GT_SYMBOL_BACKSPACE,5}, {GT_BTNMAP_NEW_LINE,0},
    {GT_SYMBOL_CH,5}, {GT_SYMBOL_WELL_NUMBER,5}, {GT_SYMBOL_LEFT,4}, {GT_SYMBOL_SPACE,12}, {GT_SYMBOL_RIGHT,4}, {GT_SYMBOL_NEW_LINE,8},
    {NULL,0},
};

static const gt_map_st _symbol_en_map_default_26_key[] = {
    {"1",1}, {"2",1}, {"3",1}, {"4",1}, {"5",1}, {"6",1}, {"7",1}, {"8",1}, {"9",1}, {"0",1}, {GT_BTNMAP_NEW_LINE,0},
    {"!",1}, {"@",1}, {"#",1}, {"$",1}, {"%",1}, {"^",1}, {"&",1}, {"*",1}, {"(",1}, {")",1}, {GT_BTNMAP_NEW_LINE,0},
    {NULL,1}, {"-",5}, {"+",5}, {"=",5}, {"_",5}, {"{",5}, {"}",5}, {"[",5}, {"]",5}, {"\\",5}, {NULL,1}, {GT_BTNMAP_NEW_LINE,0},
    {NULL,1}, {";",3}, {":",3}, {"\"",3}, {"\'",3}, {"~",3}, {"`",3}, {"/ ",3}, {" |",3}, {NULL,1}, {GT_BTNMAP_NEW_LINE,0},
    {GT_SYMBOL_LOWER_CASE,5}, {NULL,1}, {",",4}, {".",4}, {"?",4}, {"<",4}, {">",4}, {NULL,1}, {GT_SYMBOL_BACKSPACE,5}, {GT_BTNMAP_NEW_LINE,0},
    {GT_SYMBOL_CH,5}, {GT_SYMBOL_WELL_NUMBER,5}, {GT_SYMBOL_LEFT,4}, {GT_SYMBOL_SPACE,12}, {GT_SYMBOL_RIGHT,4}, {GT_SYMBOL_NEW_LINE,8},
    {NULL,},
};

static const gt_map_st _ch_map_default_26_key[] = {
    {_gt_spell[0],1}, {GT_BTNMAP_NEW_LINE,0},
    {GT_SYMBOL_PREV,1}, {_gt_spell[1],1}, {_gt_spell[2],1}, {_gt_spell[3],1}, {_gt_spell[4],1}, {_gt_spell[5],1}, {_gt_spell[6],1}, {_gt_spell[7],1}, {_gt_spell[8],1}, {GT_SYMBOL_NEXT,1}, {GT_BTNMAP_NEW_LINE,0},
    {"q",1}, {"w",1}, {"e",1}, {"r",1}, {"t",1}, {"y",1}, {"u",1}, {"i",1}, {"o",1}, {"p",1}, {GT_BTNMAP_NEW_LINE,0},
    {NULL,1}, {"a",3}, {"s",3}, {"d",3}, {"f",3}, {"g",3}, {"h",3}, {"j",3}, {"k",3}, {"l",3}, {NULL,1}, {GT_BTNMAP_NEW_LINE,0},
    {GT_SYMBOL_LOWER_CASE,5}, {NULL,1}, {"z",4}, {"x",4}, {"c",4}, {"v",4}, {"b",4}, {"n",4}, {"m",4}, {NULL,1}, {GT_SYMBOL_BACKSPACE,5}, {GT_BTNMAP_NEW_LINE,0},
    {GT_SYMBOL_EN,5}, {GT_SYMBOL_WELL_NUMBER,5}, {GT_SYMBOL_LEFT,4}, {GT_SYMBOL_SPACE,12}, {GT_SYMBOL_RIGHT,4}, {GT_SYMBOL_NEW_LINE,8},
    {NULL,0},
};

static const char * _def_26_key_user_icon_key_list[] = {
    GT_SYMBOL_BACKSPACE,
    GT_SYMBOL_NEW_LINE,
    GT_SYMBOL_LOWER_CASE,
    GT_SYMBOL_UPPER_CASE,
    GT_SYMBOL_CH,
    GT_SYMBOL_EN,
    GT_SYMBOL_WELL_NUMBER,
    GT_SYMBOL_LEFT,
    GT_SYMBOL_RIGHT,
    GT_SYMBOL_PREV,
    GT_SYMBOL_NEXT,
    NULL,
};

static const gt_keypad_map_st _list_default_26_key[] = {
    {.type = GT_KEYPAD_TYPE_LOWER, .map = _low_map_default_26_key},
    {.type = GT_KEYPAD_TYPE_UPPER, .map = _up_map_default_26_key},
    {.type = GT_KEYPAD_TYPE_SYMBOL_EN, .map = _symbol_en_map_default_26_key},
    {.type = GT_KEYPAD_TYPE_CH, .map = _ch_map_default_26_key},
};


static gt_size_t _gt_get_spell_index(char const * const kv) {
    gt_size_t idx = 0;

    if (kv == NULL) {
        return -1;
    }

    for (idx = 0; idx < _GT_KEYPADS_SPELL_NUMB; idx++) {
        if(strcmp((const char * )_gt_spell[idx], kv) == 0) {
            return idx;
        }
    }

    return -1;
}

static uint8_t * _gt_spell_get_text(gt_py_input_method_st * py_input_mt,  uint8_t index) {
    if (!py_input_mt) {
        return (uint8_t * )"";
    }

    if (index == 0 && py_input_mt->ascii_numb > 0) {
        return py_input_mt->ascii;
    }
    else if (index > 0 && py_input_mt->chinese_numb > 0) {
        return py_input_mt->chinese[index-1];
    }
    return (uint8_t * )"";
}

static void _def_26_key_push_btn_kv_handler( gt_obj_st * obj,  gt_obj_st * input, const char* const kv) {
    if (!obj || !kv) {
        return ;
    }
    gt_py_input_method_st* py_input_mt = gt_btnmap_get_py_input_method(obj);
    gt_keypad_type_te tmp_keypad_type;
    gt_size_t idx = -1;
    uint16_t type = gt_btnmap_get_map_type(obj);

    if (0 == strcmp(kv, GT_SYMBOL_SPACE)) {
        gt_input_append_value(input, " ");
    }
    else if (0 == strcmp(kv, GT_SYMBOL_BACKSPACE)) {
        if(py_input_mt && GT_KEYPAD_TYPE_CH == type && gt_py_input_method_get_ascii_numb(py_input_mt) > 0){
            gt_py_input_method_pop_ascii(py_input_mt);
            goto _get_chinese;
        }
        gt_input_del_value(input);
    }
    else if (0 == strcmp(kv, GT_SYMBOL_NEW_LINE)) {
        gt_input_append_value(input, "\n");
    }
    else if (0 == strcmp(kv, GT_SYMBOL_UPPER_CASE) || 0 == strcmp(kv, GT_SYMBOL_EN)) {

        idx = _gt_get_map_index((gt_keypad_map_st*)_list_default_26_key, sizeof(_list_default_26_key)/sizeof(gt_keypad_map_st), GT_KEYPAD_TYPE_LOWER);
        if(idx < 0){
            GT_LOGW(GT_LOG_TAG_GUI, "keypad type is not exist!!! def_style = %d, type = %d", GT_KEYPAD_STYLE_26_KEY, GT_KEYPAD_TYPE_LOWER);
            return ;
        }
        gt_btnmap_set_map( obj, (gt_map_st*)_list_default_26_key[idx].map, GT_KEYPAD_TYPE_LOWER);
    }
    else if (0 == strcmp(kv, GT_SYMBOL_LOWER_CASE)) {
        idx = _gt_get_map_index((gt_keypad_map_st*)_list_default_26_key, sizeof(_list_default_26_key)/sizeof(gt_keypad_map_st), GT_KEYPAD_TYPE_UPPER);
        if(idx < 0){
            GT_LOGW(GT_LOG_TAG_GUI, "keypad type is not exist!!! def_style = %d, type = %d", GT_KEYPAD_STYLE_26_KEY, GT_KEYPAD_TYPE_UPPER);
            return ;
        }
        gt_btnmap_set_map( obj, (gt_map_st*)_list_default_26_key[idx].map, GT_KEYPAD_TYPE_UPPER);
    }
    else if (0 == strcmp(kv, GT_SYMBOL_WELL_NUMBER)) {
        tmp_keypad_type = (type != GT_KEYPAD_TYPE_SYMBOL_EN) ? GT_KEYPAD_TYPE_SYMBOL_EN : GT_KEYPAD_TYPE_LOWER;
        idx = _gt_get_map_index((gt_keypad_map_st*)_list_default_26_key, sizeof(_list_default_26_key)/sizeof(gt_keypad_map_st), tmp_keypad_type);
        if (idx < 0) {
            GT_LOGW(GT_LOG_TAG_GUI, "keypad type is not exist!!! def_style = %d, type = %d", GT_KEYPAD_STYLE_26_KEY, tmp_keypad_type);
            return ;
        }
        gt_btnmap_set_map( obj, (gt_map_st*)_list_default_26_key[idx].map, tmp_keypad_type);
    }
    else if (0 == strcmp(kv, GT_SYMBOL_CH)) {
        if (!py_input_mt) {
            GT_LOGW(GT_LOG_TAG_GUI, "Keyboard does not create pinyin input method!!!");
            return ;
        }
        idx = _gt_get_map_index((gt_keypad_map_st*)_list_default_26_key, sizeof(_list_default_26_key)/sizeof(gt_keypad_map_st), GT_KEYPAD_TYPE_CH);
        if (idx < 0) {
            GT_LOGW(GT_LOG_TAG_GUI, "keypad type is not exist!!! def_style = %d, type = %d", GT_KEYPAD_STYLE_26_KEY, GT_KEYPAD_TYPE_CH);
            return ;
        }
        gt_btnmap_set_map( obj, (gt_map_st*)_list_default_26_key[idx].map, GT_KEYPAD_TYPE_CH);
    }
    else if (0 == strcmp(kv, GT_SYMBOL_LEFT)) {
        gt_input_move_left_pos_cursor(input);
    }
    else if (0 == strcmp(kv, GT_SYMBOL_RIGHT)) {
        gt_input_move_right_pos_cursor(input);
    }
    else if (0 == strcmp(kv, GT_SYMBOL_PREV)) {
        if (!py_input_mt || py_input_mt->chinese_numb <= 0) return ;
        gt_py_input_method_last_page(py_input_mt);
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_REDRAW, NULL);
    }
    else if (0 == strcmp(kv, GT_SYMBOL_NEXT)) {
        if(!py_input_mt || py_input_mt->chinese_numb <= 0) return ;
        gt_py_input_method_next_page(py_input_mt);
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_REDRAW, NULL);
    }
    else {
        idx = _gt_get_spell_index(kv);
        if (idx < 0 && GT_KEYPAD_TYPE_CH != type) {
            gt_input_append_value(input, (char*)kv);
            return ;
        }

        if (!py_input_mt) return ;

        if (idx < 0 && GT_KEYPAD_TYPE_CH == type) {
            if (py_input_mt->ascii_numb >= GT_PY_MAX_NUMB) {
                return ;
            }

            gt_py_input_method_push_ascii(py_input_mt, *kv);

_get_chinese:
            if (!gt_py_input_method_get_chinese(py_input_mt)) {
                gt_event_send(obj, GT_EVENT_TYPE_DRAW_REDRAW, NULL);
                return ;
            }

            gt_py_input_method_chinese_data_handler(py_input_mt);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_REDRAW, NULL);
            return ;
        }
        else if (0 == idx) {
            uint8_t *tmp_str = _gt_spell_get_text(py_input_mt, idx);
            gt_input_append_value(input, (char*)tmp_str);
            gt_py_input_method_clean(py_input_mt);
            gt_event_send(obj, GT_EVENT_TYPE_DRAW_REDRAW, NULL);
            return ;
        }

        uint8_t *tmp_str = _gt_spell_get_text(py_input_mt, idx);
        gt_input_append_value(input, (char*)tmp_str);

        gt_py_input_method_select_text(py_input_mt, idx);
        gt_py_input_method_chinese_data_handler(py_input_mt);
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_REDRAW, NULL);
    }
}

static bool _gt_keypad_check_key_is_ctl(char const * const kv) {
    if (kv == NULL) {
        return false;
    }
    gt_size_t idx = 0;
    while (_def_26_key_user_icon_key_list[idx] != NULL) {
        if (strcmp(_def_26_key_user_icon_key_list[idx], kv) == 0) {
            return true;
        }
        idx++;
    }
    return false;
}

static bool _def_26_key_disp_special_btn_handler(gt_obj_st* obj, const char* const kv, gt_attr_font_st* font_attr) {
    if (_gt_keypad_check_key_is_ctl(kv)) {
        return true;
    }

    gt_size_t idx = _gt_get_spell_index(kv);
    if (idx < 0) {
        return false;
    }

    gt_py_input_method_st* py_input_mt = gt_btnmap_get_py_input_method(obj);
    uint8_t *tmp_str = _gt_spell_get_text(py_input_mt, idx);

    font_attr->font->utf8 = (char * )tmp_str;
    font_attr->font->len = strlen((const char * )tmp_str);
    return false;
}

#endif // _DEF_STYLE_26_KEY

#ifdef _DEF_STYLE_OTHER_4x5
// w = 44 * 4 + 3 * 5 = 176 + 15 = 191
// h = 44 * 5 + 3 * 6 = 220 + 18 = 238
static const gt_map_st _low_map_default_tlb[] = {
    {"a",1}, {"b",1}, {"c",1}, {"d",1}, {GT_BTNMAP_NEW_LINE,0},
    {"e",1}, {"f",1}, {"g",1}, {"h",1}, {GT_BTNMAP_NEW_LINE,0},
    {"i",1}, {"j",1}, {"k",1}, {"l",1}, {GT_BTNMAP_NEW_LINE,0},
    {"m",1}, {"n",1}, {"o",1}, {"p",1}, {GT_BTNMAP_NEW_LINE,0},
    {"q",1}, {"r",1}, {"s",1}, {"t",1}, {GT_BTNMAP_NEW_LINE,0},
    {"u",1}, {"v",1}, {"w",1}, {"x",1}, {GT_BTNMAP_NEW_LINE,0},
    {"y",1}, {"z",1}, {NULL,2}, {NULL,0},
};

static const gt_map_st _up_map_default_tlb[] = {
    {"A",1}, {"B",1}, {"C",1}, {"D",1}, {GT_BTNMAP_NEW_LINE,0},
    {"E",1}, {"F",1}, {"G",1}, {"H",1}, {GT_BTNMAP_NEW_LINE,0},
    {"I",1}, {"J",1}, {"K",1}, {"L",1}, {GT_BTNMAP_NEW_LINE,0},
    {"M",1}, {"N",1}, {"O",1}, {"P",1}, {GT_BTNMAP_NEW_LINE,0},
    {"Q",1}, {"R",1}, {"S",1}, {"T",1}, {GT_BTNMAP_NEW_LINE,0},
    {"U",1}, {"V",1}, {"W",1}, {"X",1}, {GT_BTNMAP_NEW_LINE,0},
    {"Y",1}, {"Z",1}, {NULL,2}, {NULL,0},
};

static const gt_map_st _numb_map_default_tlb[] = {
    {"1",1}, {"2",1}, {"3",1}, {"4",1}, {GT_BTNMAP_NEW_LINE,0},
    {"5",1}, {"6",1}, {"7",1}, {"8",1}, {GT_BTNMAP_NEW_LINE,0},
    {"9",1}, {"0",1}, {"@",1}, {"=",1}, {GT_BTNMAP_NEW_LINE,0},
    {"+",1}, {"-",1}, {"*",1}, {"/",1}, {GT_BTNMAP_NEW_LINE,0},
    {",",1}, {".",1}, {";",1}, {"?",1}, {GT_BTNMAP_NEW_LINE,0},
    {"\'",1}, {"\"",1}, {":",1}, {"!",1}, {GT_BTNMAP_NEW_LINE,0},
    {"(",1}, {")",1}, {"<",1}, {">",1}, {GT_BTNMAP_NEW_LINE,0},
    {"[",1}, {"]",1}, {"_",1}, {"~",1}, {GT_BTNMAP_NEW_LINE,0},
    {"\\",1}, {"|",1}, {"`",1}, {"#",1}, {GT_BTNMAP_NEW_LINE,0},
    {"$",1}, {"%",1}, {"^",1}, {"&",1}, {GT_BTNMAP_NEW_LINE,0},
    {"{",1}, {"}",1}, {NULL,2}, {NULL,0},
};

static const gt_keypad_map_st _list_default_tlb[] = {
    {.type = GT_KEYPAD_TYPE_LOWER, .map = _low_map_default_tlb},
    {.type = GT_KEYPAD_TYPE_UPPER, .map = _up_map_default_tlb},
    {.type = GT_KEYPAD_TYPE_NUMBER, .map = _numb_map_default_tlb},
};
#endif // _DEF_STYLE_OTHER_4x5
//
#ifdef _DEF_STYLE_OTHER_11x3
// w = 50 * 11 + 3 * 12 = 550 + 36 = 586
// h = 38 * 3 + 3 * 4 = 114 + 12 = 126
static const gt_map_st _low_map_default_fyb_369[] = {
    {"a",1}, {"b",1}, {"c",1}, {"d",1}, {"e",1}, {"f",1}, {"g",1}, {"h",1}, {"i",1}, {"j",1}, {"k",1}, {GT_BTNMAP_NEW_LINE,0},
    {"l",1}, {"m",1}, {"n",1}, {"o",1}, {"p",1}, {"q",1}, {"r",1}, {"s",1}, {"t",1}, {"u",1}, {"v",1}, {GT_BTNMAP_NEW_LINE,0},
    {"w",1}, {"x",1}, {"y",1}, {"z",1}, {NULL,1}, {NULL,1}, {NULL,1}, {NULL,1}, {NULL,1}, {NULL,1}, {NULL,1}, {NULL,0},
};

static const gt_map_st _up_map_default_fyb_369[] = {
    {"A",1}, {"B",1}, {"C",1}, {"D",1}, {"E",1}, {"F",1}, {"G",1}, {"H",1}, {"I",1}, {"J",1}, {"K",1}, {GT_BTNMAP_NEW_LINE,0},
    {"L",1}, {"M",1}, {"N",1}, {"O",1}, {"P",1}, {"Q",1}, {"R",1}, {"S",1}, {"T",1}, {"U",1}, {"V",1}, {GT_BTNMAP_NEW_LINE,0},
    {"W",1}, {"X",1}, {"Y",1}, {"Z",1}, {NULL,1}, {NULL,1}, {NULL,1}, {NULL,1}, {NULL,1}, {NULL,1}, {NULL,1}, {NULL,0},
};

static const gt_map_st _numb_map_default_fyb_369[] = {
    {"1",1}, {"2",1}, {"3",1}, {"4",1}, {"5",1}, {"6",1}, {"7",1}, {"8",1}, {"9",1}, {"0",1}, {"@",1}, {GT_BTNMAP_NEW_LINE,0},
    {"=",1}, {"+",1}, {"-",1}, {"*",1}, {"/",1}, {",",1}, {".",1}, {";",1}, {"?",1}, {"\'",1}, {"\"",1}, {GT_BTNMAP_NEW_LINE,0},
    {":",1}, {"!",1}, {"(",1}, {")",1}, {"<",1}, {">",1}, {"[",1}, {"]",1}, {"_",1}, {"~",1}, {"\\",1}, {GT_BTNMAP_NEW_LINE,0},
    {"|",1}, {"`",1}, {"#",1}, {"$",1}, {"%",1}, {"^",1}, {"&",1}, {"{",1}, {"}",1}, {NULL,1}, {NULL,1}, {NULL,0},
};

static const gt_keypad_map_st _list_default_fyb_369[] = {
    {.type = GT_KEYPAD_TYPE_LOWER, .map = _low_map_default_fyb_369},
    {.type = GT_KEYPAD_TYPE_UPPER, .map = _up_map_default_fyb_369},
    {.type = GT_KEYPAD_TYPE_NUMBER, .map = _numb_map_default_fyb_369},
};
#endif // _DEF_STYLE_OTHER_11x3

static const _gt_def_style_param_st _def_style_list[] = {
#ifdef _DEF_STYLE_26_KEY
    {
        .def_style = GT_KEYPAD_STYLE_26_KEY,
        .map_list = _list_default_26_key,
        .map_total = sizeof(_list_default_26_key) / sizeof(gt_keypad_map_st),
        .w = 304,
        .h = 225,
        .radius = 6,
        .bg_color = 0xD2D3D7,
        .border_width = 0,
        .border_color = 0xD2D3D7,
        .key_h = 34,
        .key_radius = 6,
        .key_bg_color = 0xFFFFFF,
        .key_border_width = 0,
        .key_border_color = 0xFFFFFF,
        .key_x_space = 3,
        .key_y_space = 3,
        ._disp_special_btn_cb = _def_26_key_disp_special_btn_handler,
        ._push_btn_kv_cb = _def_26_key_push_btn_kv_handler,
        .ctrl_key_bg_color = 0xBAC1CB,
        .ctrl_key_border_width = 0,
        .ctrl_key_border_color = 0xBAC1CB,
    },
#endif // _DEF_STYLE_26_KEY

#ifdef _DEF_STYLE_OTHER_4x5
    {
        .def_style = GT_KEYPAD_STYLE_OTHER_4x5,
        .map_list = _list_default_tlb,
        .map_total = sizeof(_list_default_tlb) / sizeof(gt_keypad_map_st),
        .w = 191,
        .h = 238,
        .radius = 6,
        .bg_color = 0x0A1143,
        .border_width = 0,
        .border_color = 0x0A1143,
        .key_h = 44,
        .key_radius = 6,
        .key_bg_color = 0x183581,
        .key_border_width = 0,
        .key_border_color = 0x183581,
        .key_x_space = 3,
        .key_y_space = 3,
        ._disp_special_btn_cb = NULL,
        ._push_btn_kv_cb = NULL,
        .ctrl_key_bg_color = 0x183581,
        .ctrl_key_border_width = 0,
        .ctrl_key_border_color = 0x183581,
    },
#endif // _DEF_STYLE_OTHER_4x5

#ifdef _DEF_STYLE_OTHER_11x3
    {
        .def_style = GT_KEYPAD_STYLE_OTHER_11x3,
        .map_list = _list_default_fyb_369,
        .map_total = sizeof(_list_default_fyb_369) / sizeof(gt_keypad_map_st),
        .w = 586,
        .h = 126,
        .radius = 6,
        .bg_color = 0x000000,
        .border_width = 0,
        .border_color = 0x000000,
        .key_h = 38,
        .key_radius = 6,
        .key_bg_color = 0x22272C,
        .key_border_width = 0,
        .key_border_color = 0x22272C,
        .key_x_space = 3,
        .key_y_space = 3,
        ._disp_special_btn_cb = NULL,
        ._push_btn_kv_cb = NULL,
        .ctrl_key_bg_color = 0x22272C,
        .ctrl_key_border_width = 0,
        .ctrl_key_border_color = 0x22272C,
    },
#endif // _DEF_STYLE_OTHER_11x3
};

static void _gt_switch_default_style(gt_obj_st * keypad, gt_keypad_default_style_st def_style) {
    _gt_keypad_st* style = (_gt_keypad_st*)keypad;
    style->def_style = def_style;
    if (GT_KEYPAD_STYLE_NONE == style->def_style) {
        return ;
    }
    const _gt_def_style_param_st * item = NULL;
    uint8_t total = sizeof(_def_style_list) / sizeof(_gt_def_style_param_st);
    uint8_t i = 0;
    for(i = 0; i < total; i++) {
        if (style->def_style == _def_style_list[i].def_style) {
            item = &_def_style_list[i];
            break;
        }
    }
    if (NULL == item) {
        return;
    }
    gt_keypad_set_radius(keypad, item->radius);
    gt_keypad_set_color_background(keypad, gt_color_hex(item->bg_color));
    gt_keypad_set_border_width_and_color(keypad, item->border_width, gt_color_hex(item->border_color));
    gt_keypad_set_key_height(keypad, item->key_h);
    gt_keypad_set_key_radius(keypad, item->key_radius);
    gt_keypad_set_key_color_background(keypad, gt_color_hex(item->key_bg_color));
    gt_keypad_set_key_border_width_and_color(keypad, item->key_border_width, gt_color_hex(item->key_border_color));
    gt_keypad_set_key_xy_space(keypad, item->key_x_space, item->key_y_space);
    gt_keypad_set_ctrl_key_color_background(keypad, gt_color_hex(item->ctrl_key_bg_color));
    gt_keypad_set_ctrl_key_border_width_and_color(keypad, item->ctrl_key_border_width, gt_color_hex(item->ctrl_key_border_color));
    gt_obj_set_size(keypad, item->w, item->h);
    // def
    if (GT_KEYPAD_STYLE_26_KEY == style->def_style) {
        gt_keypad_set_key_height_auto_fill(keypad, true);
    } else {
        gt_keypad_set_key_height_auto_fill(keypad, false);
    }

    gt_btnmap_set_disp_special_btn_handler(style->btnmap, item->_disp_special_btn_cb);
    gt_btnmap_set_push_btn_kv_handler(style->btnmap, item->_push_btn_kv_cb);
    gt_keypad_set_map_list(keypad, (gt_keypad_map_st*)item->map_list, item->map_total);
}

static inline void _set_minimum_width(gt_obj_st * keypad) {
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;

    uint16_t minimum_width = gt_btnmap_get_calc_minimum_width(style->btnmap);
    if (keypad->area.w < minimum_width) {
        keypad->area.w = minimum_width;
    }
}

/* macros ---------------------------------------------------------------*/


/* static functions -----------------------------------------------------*/
/**
 * @brief obj init keypad widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    if(obj->area.w == 0 || obj->area.h == 0) return;

    _gt_keypad_st* style = (_gt_keypad_st*)obj;

    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill = 1;
    rect_attr.bg_opa = obj->opa;
    rect_attr.bg_color = style->bg_color;
    rect_attr.border_color = style->border_color;
    rect_attr.border_width = style->border_width;
    rect_attr.radius = obj->radius;
    // draw_bg
    draw_bg(obj->draw_ctx, &rect_attr, &obj->area);

    _set_minimum_width(obj);
    gt_obj_set_size(style->btnmap, obj->area.w, gt_btnmap_get_btn_height_auto_fill(style->btnmap) ? obj->area.h : style->btnmap->area.h);
}

/**
 * @brief obj event handler call back
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et tp = gt_event_get_code(e);

    if (GT_EVENT_TYPE_DRAW_START == tp) {
        gt_disp_invalid_area(obj);
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_END, NULL);
    }
    else if (GT_EVENT_TYPE_INPUT_SCROLL == tp) {
        _scrolling_handler(obj);
    }
    else if (GT_EVENT_TYPE_DRAW_REDRAW == tp) {
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
    }
}

static void _scrolling_handler(gt_obj_st * obj)
{
    gt_size_t bottom = gt_obj_get_limit_bottom(obj);
    uint16_t height = gt_obj_get_childs_max_height(obj);

    if (height < obj->area.h) {
        obj->process_attr.scroll.y = 0;
        return;
    }
    else if (gt_obj_scroll_get_y(obj) > 0) {
        obj->process_attr.scroll.y = 0;
    }
    else if (gt_obj_scroll_get_y(obj) < bottom) {
        obj->process_attr.scroll.y = bottom;
    }
    gt_size_t offset = obj->area.y + gt_obj_scroll_get_y(obj);
    for (gt_size_t i = 0, cnt = obj->cnt_child; i < cnt; i++) {
        gt_obj_set_pos(obj->child[i], gt_obj_get_x(obj->child[i]), offset);
    }
}

static gt_size_t _gt_get_map_index(gt_keypad_map_st * map_list, uint8_t map_total, gt_keypad_type_te type)
{
    gt_size_t i = -1;
    for ( i = 0; i < map_total; i++) {
        if (type != map_list[i].type) {
            continue;
        }

        if (NULL == map_list[i].map) {
            GT_LOGE(GT_LOG_TAG_GUI, "keypad map is NULL!!!");
            return -1;
        }

        return i;
    }
    return -1;
}

static void _gt_keypad_switch_type(gt_obj_st * keypad, gt_keypad_type_te type, bool flag)
{
    _gt_keypad_st* style = (_gt_keypad_st*)keypad;

    if(!flag && style->type == type) return;

    if(NULL == style->map_list){
        GT_LOGE(GT_LOG_TAG_GUI, "keypad map_list is NULL!!!");
        return ;
    }
    gt_size_t idx = _gt_get_map_index(style->map_list, style->map_total, type);
    if (idx < 0) {
        GT_LOGW(GT_LOG_TAG_GUI, "keypad type is not exist!!! def_style = %d, type = %d", style->def_style, style->type);
        return ;
    }
    style->type = type;
    keypad->process_attr.scroll.y = 0;
    gt_obj_set_pos(style->btnmap, keypad->area.x, keypad->area.y);
    gt_btnmap_set_map(style->btnmap, (gt_map_st*)style->map_list[idx].map, style->map_list[idx].type);

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
    if (NULL == obj) {
        return obj;
    }

    obj->radius = 4;
    gt_obj_set_fixed(obj, false);
    gt_obj_set_focus_disabled(obj, GT_DISABLED);

    _gt_keypad_st* style = (_gt_keypad_st*)obj;
    style->btnmap = gt_btnmap_create(obj);
    gt_obj_set_pos(style->btnmap, obj->area.x, obj->area.y);
    gt_obj_set_inside(style->btnmap, true);
    gt_obj_set_fixed(style->btnmap, true);

    gt_keypad_set_default_style(obj, GT_KEYPAD_STYLE_26_KEY);

    return obj;
}

void gt_keypad_set_default_style(gt_obj_st * keypad, gt_keypad_default_style_st def_style)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }

    _gt_switch_default_style(keypad, def_style);
    gt_event_send(keypad, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_keypad_set_type(gt_obj_st * keypad, gt_keypad_type_te type)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }

    _gt_keypad_switch_type(keypad, type, false);
}

void gt_keypad_set_map_list(gt_obj_st * keypad, gt_keypad_map_st* map_list, uint8_t map_num)
{
    if (NULL == map_list) {
        return;
    }
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st* style = (_gt_keypad_st*)keypad;
    style->map_list = map_list;
    style->map_total = map_num;
    _gt_keypad_switch_type(keypad, map_list[0].type, true);
}

void gt_keypad_set_target(gt_obj_st * keypad, gt_obj_st * ta)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    if (NULL == ta) {
        return;
    }
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;
    gt_btnmap_set_input(style->btnmap, ta);
}

void gt_keypad_set_color_background(gt_obj_st * keypad, gt_color_t color)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st* style = (_gt_keypad_st*)keypad;
    style->bg_color = color;
}

void gt_keypad_set_border_width_and_color(gt_obj_st * keypad, gt_size_t width, gt_color_t color)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st* style = (_gt_keypad_st*)keypad;
    style->border_width = width;
    style->border_color = color;
}

void gt_keypad_set_color_ctrl_key(gt_obj_st * keypad, gt_color_t color)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;
    style->color_ctrl = color;
}

void gt_keypad_set_key_color_background(gt_obj_st * keypad, gt_color_t color)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st* style = (_gt_keypad_st*)keypad;
    gt_btnmap_set_color_background(style->btnmap, color);
}

void gt_keypad_set_key_border_width_and_color(gt_obj_st * keypad, gt_size_t width, gt_color_t color)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st* style = (_gt_keypad_st*)keypad;
    gt_btnmap_set_border_width_and_color(style->btnmap, width, color);
}



void gt_keypad_set_radius(gt_obj_st * keypad, uint8_t radius)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    keypad->radius = radius;
    gt_event_send(keypad, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_keypad_set_key_radius(gt_obj_st * keypad, uint8_t radius)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st* style = (_gt_keypad_st*)keypad;
    gt_btnmap_set_radius(style->btnmap, radius);
}

void gt_keypad_set_key_height(gt_obj_st * keypad, uint16_t height)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st* style = (_gt_keypad_st*)keypad;
    gt_btnmap_set_btn_height(style->btnmap, height);
}

void gt_keypad_set_key_height_auto_fill(gt_obj_st* keypad, bool auto_fill)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st* style = (_gt_keypad_st*)keypad;
    gt_btnmap_set_btn_height_auto_fill(style->btnmap, auto_fill);
}

void gt_keypad_set_key_xy_space(gt_obj_st* keypad, uint8_t x_space, uint8_t y_space)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st* style = (_gt_keypad_st*)keypad;
    gt_btnmap_set_btn_xy_space(style->btnmap, x_space, y_space);
}
//
void gt_keypad_set_font_color(gt_obj_st * keypad, gt_color_t color)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;
    gt_btnmap_set_font_color(style->btnmap, color);
}
void gt_keypad_set_font_size(gt_obj_st * keypad, uint8_t size)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;
    gt_btnmap_set_font_size(style->btnmap, size);
}

void gt_keypad_set_font_gray(gt_obj_st * keypad, uint8_t gray)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;
    gt_btnmap_set_font_gray(style->btnmap, gray);
}

void gt_keypad_set_font_family_cn(gt_obj_st * keypad, gt_family_t family)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;
    gt_btnmap_set_font_family_cn(style->btnmap, family);
}
void gt_keypad_set_font_family_en(gt_obj_st * keypad, gt_family_t family)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;
    gt_btnmap_set_font_family_en(style->btnmap, family);
}

void gt_keypad_set_font_family_fl(gt_obj_st * keypad, gt_family_t family)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;
    gt_btnmap_set_font_family_fl(style->btnmap, family);
}
void gt_keypad_set_font_family_numb(gt_obj_st * keypad, gt_family_t family)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;
    gt_btnmap_set_font_family_numb(style->btnmap, family);
}
void gt_keypad_set_font_thick_en(gt_obj_st * keypad, uint8_t thick)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;
    gt_btnmap_set_font_thick_en(style->btnmap, thick);
}

void gt_keypad_set_font_thick_cn(gt_obj_st * keypad, uint8_t thick)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;
    gt_btnmap_set_font_thick_cn(style->btnmap, thick);
}

void gt_keypad_set_ctrl_key_color_background(gt_obj_st * keypad, gt_color_t color)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;
    gt_btnmap_set_special_btn_color_background(style->btnmap, color);
}
void gt_keypad_set_ctrl_key_border_width_and_color(gt_obj_st * keypad, gt_size_t width, gt_color_t color)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;
    gt_btnmap_set_special_btn_border_width_and_color(style->btnmap, width, color);
}
void gt_keypad_set_ctrl_key_font_color(gt_obj_st * keypad, gt_color_t color)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;
    gt_btnmap_set_special_btn_font_color(style->btnmap, color);
}

void gt_keypad_set_py_input_method(gt_obj_st * keypad, gt_py_input_method_st* py_input_method)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;
    gt_btnmap_set_py_input_method(style->btnmap, py_input_method);
}

#endif  /** GT_CFG_ENABLE_KEYPAD */
/* end of file ----------------------------------------------------------*/
