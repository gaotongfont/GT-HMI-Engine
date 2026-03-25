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
#include "../core/gt_style.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_KEYPAD
#define MY_CLASS    &gt_keypad_class

//
#define _GT_KEYPADS_SPELL_NUMB      (9)
/* private typedef ------------------------------------------------------*/
typedef enum {
    _CMD_NONE = 0,
    _CMD_BACKSPACE,
    _CMD_ENTER,
    _CMD_LOWER_CASE,
    _CMD_UPPER_CASE,
    _CMD_CH,
    _CMD_EN,
    _CMD_WELL_NUMBER,
    _CMD_LEFT,
    _CMD_RIGHT,
    _CMD_PREV,
    _CMD_NEXT,
}_cmd_type;

typedef struct {
    const char* const kv;
    _cmd_type cmd;
}_cmd_map_st;

typedef struct _gt_keypad_s {
    gt_obj_st base;
    gt_obj_st* btnmap;

    gt_keypad_map_st * map_list;

    gt_keypad_enter_cb_t enter_cb;

    gt_color_t border_color;
    gt_color_t color_ctrl;

    gt_size_t border_width;
    gt_keypad_type_te type;
    gt_keypad_default_style_st def_style;

    uint8_t map_total;

    uint8_t auto_hide : 1;
}_gt_keypad_st;

typedef struct _gt_def_style_param_s {
    const gt_keypad_map_st * map_list;

    gt_btnmap_show_special_btn_cb_t _show_special_btn_cb;
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

//
static void _key_push_btn_kv_cb(gt_obj_st * obj, gt_obj_st * input, const char* const kv);
static bool _show_special_btn_cb(gt_obj_st* obj, const char* const kv, gt_attr_font_st* font_attr);
/* static variables -----------------------------------------------------*/
static GT_ATTRIBUTE_RAM_DATA const gt_obj_class_st gt_keypad_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = (_gt_deinit_cb_t)NULL,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_keypad_st)
};

static GT_ATTRIBUTE_RAM_DATA const _cmd_map_st _cmd_map_list[] = {
    {GT_SYMBOL_BACKSPACE, _CMD_BACKSPACE},
    {GT_SYMBOL_NEW_LINE,  _CMD_ENTER},
    {GT_SYMBOL_LOWER_CASE,_CMD_LOWER_CASE},
    {GT_SYMBOL_UPPER_CASE,_CMD_UPPER_CASE},
    {GT_SYMBOL_CH,        _CMD_CH},
    {GT_SYMBOL_EN,        _CMD_EN},
    {GT_SYMBOL_WELL_NUMBER,_CMD_WELL_NUMBER},
    {GT_SYMBOL_LEFT,      _CMD_LEFT},
    {GT_SYMBOL_RIGHT,     _CMD_RIGHT},
    {GT_SYMBOL_PREV,      _CMD_PREV},
    {GT_SYMBOL_NEXT,      _CMD_NEXT},

    //
    {NULL,_CMD_NONE}
};
static GT_ATTRIBUTE_RAM_DATA const char _gt_spell[_GT_KEYPADS_SPELL_NUMB][8] = {
    "spell_0", "spell_1", "spell_2", "spell_3", "spell_4", "spell_5", "spell_6", "spell_7", "spell_8",
};
/* keypad default style -----------------------------------------------------*/

#ifdef _DEF_STYLE_26_KEY
// h = 34 * 6 + 3 * 7 = 204 + 24 = 225
static GT_ATTRIBUTE_RAM_DATA const gt_map_st _low_map_default_26_key[] = {
    // {".",1}, {",",1}, {"?",1}, {";",1}, {":",1}, {"\'",1}, {"\"",1}, {"@",1}, {"(",1}, {")",1}, {GT_BTNMAP_NEW_LINE,0},
    {"1",1}, {"2",1}, {"3",1}, {"4",1}, {"5",1}, {"6",1}, {"7",1}, {"8",1}, {"9",1}, {"0",1}, {GT_BTNMAP_NEW_LINE,0},
    {"q",1}, {"w",1}, {"e",1}, {"r",1}, {"t",1}, {"y",1}, {"u",1}, {"i",1}, {"o",1}, {"p",1}, {GT_BTNMAP_NEW_LINE,0},
    {NULL,1}, {"a",3}, {"s",3}, {"d",3}, {"f",3}, {"g",3}, {"h",3}, {"j",3}, {"k",3}, {"l",3}, {NULL,1}, {GT_BTNMAP_NEW_LINE,0},
    {GT_SYMBOL_LOWER_CASE,5},{NULL,1}, {"z",4}, {"x",4}, {"c",4}, {"v",4}, {"b",4}, {"n",4}, {"m",4}, {NULL,1}, {GT_SYMBOL_BACKSPACE,5}, {GT_BTNMAP_NEW_LINE,0},
    {GT_SYMBOL_CH,5}, {GT_SYMBOL_WELL_NUMBER,5}, {GT_SYMBOL_LEFT,4}, {GT_SYMBOL_SPACE,12}, {GT_SYMBOL_RIGHT,4}, {GT_SYMBOL_NEW_LINE,8},
    {NULL,0}
};

static GT_ATTRIBUTE_RAM_DATA const gt_map_st _up_map_default_26_key[] = {
    // {".",1}, {",",1}, {"?",1}, {";",1}, {":",1}, {"\'",1}, {"\"",1}, {"@",1}, {"(",1}, {")",1}, {GT_BTNMAP_NEW_LINE,0},
    {"1",1}, {"2",1}, {"3",1}, {"4",1}, {"5",1}, {"6",1}, {"7",1}, {"8",1}, {"9",1}, {"0",1}, {GT_BTNMAP_NEW_LINE,0},
    {"Q",1}, {"W",1}, {"E",1}, {"R",1}, {"T",1}, {"Y",1}, {"U",1}, {"I",1}, {"O",1}, {"P",1}, {GT_BTNMAP_NEW_LINE,0},
    {NULL,1}, {"A",3}, {"S",3}, {"D",3}, {"F",3}, {"G",3}, {"H",3}, {"J",3}, {"K",3}, {"L",3}, {NULL,1}, {GT_BTNMAP_NEW_LINE,0},
    {GT_SYMBOL_UPPER_CASE,5}, {NULL,1}, {"Z",4}, {"X",4}, {"C",4}, {"V",4}, {"B",4}, {"N",4}, {"M",4}, {NULL,1}, {GT_SYMBOL_BACKSPACE,5}, {GT_BTNMAP_NEW_LINE,0},
    {GT_SYMBOL_CH,5}, {GT_SYMBOL_WELL_NUMBER,5}, {GT_SYMBOL_LEFT,4}, {GT_SYMBOL_SPACE,12}, {GT_SYMBOL_RIGHT,4}, {GT_SYMBOL_NEW_LINE,8},
    {NULL,0},
};

static GT_ATTRIBUTE_RAM_DATA const gt_map_st _symbol_en_map_default_26_key[] = {
    // {"1",1}, {"2",1}, {"3",1}, {"4",1}, {"5",1}, {"6",1}, {"7",1}, {"8",1}, {"9",1}, {"0",1}, {GT_BTNMAP_NEW_LINE,0},
    {"!",1}, {"@",1}, {"#",1}, {"$",1}, {"%",1}, {"^",1}, {"&",1}, {"*",1}, {"(",1}, {")",1}, {GT_BTNMAP_NEW_LINE,0},
    {NULL,1}, {"-",5}, {"+",5}, {"=",5}, {"_",5}, {"{",5}, {"}",5}, {"[",5}, {"]",5}, {"\\",5}, {NULL,1}, {GT_BTNMAP_NEW_LINE,0},
    {NULL,1}, {";",3}, {":",3}, {"\"",3}, {"\'",3}, {"~",3}, {"`",3}, {"/ ",3}, {" |",3}, {NULL,1}, {GT_BTNMAP_NEW_LINE,0},
    {GT_SYMBOL_LOWER_CASE,5}, {NULL,1}, {",",4}, {".",4}, {"?",4}, {"<",4}, {">",4}, {NULL,1}, {GT_SYMBOL_BACKSPACE,5}, {GT_BTNMAP_NEW_LINE,0},
    {GT_SYMBOL_CH,5}, {GT_SYMBOL_WELL_NUMBER,5}, {GT_SYMBOL_LEFT,4}, {GT_SYMBOL_SPACE,12}, {GT_SYMBOL_RIGHT,4}, {GT_SYMBOL_NEW_LINE,8},
    {NULL,},
};

static GT_ATTRIBUTE_RAM_DATA const gt_map_st _ch_map_default_26_key[] = {
    // {_gt_spell[0],1}, {GT_BTNMAP_NEW_LINE,0},
    {GT_SYMBOL_PREV,1}, {_gt_spell[1],1}, {_gt_spell[2],1}, {_gt_spell[3],1}, {_gt_spell[4],1}, {_gt_spell[5],1}, {_gt_spell[6],1}, {_gt_spell[7],1}, {_gt_spell[8],1}, {GT_SYMBOL_NEXT,1}, {GT_BTNMAP_NEW_LINE,0},
    {"q",1}, {"w",1}, {"e",1}, {"r",1}, {"t",1}, {"y",1}, {"u",1}, {"i",1}, {"o",1}, {"p",1}, {GT_BTNMAP_NEW_LINE,0},
    {NULL,1}, {"a",3}, {"s",3}, {"d",3}, {"f",3}, {"g",3}, {"h",3}, {"j",3}, {"k",3}, {"l",3}, {NULL,1}, {GT_BTNMAP_NEW_LINE,0},
    {GT_SYMBOL_LOWER_CASE,5}, {NULL,1}, {"z",4}, {"x",4}, {"c",4}, {"v",4}, {"b",4}, {"n",4}, {"m",4}, {NULL,1}, {GT_SYMBOL_BACKSPACE,5}, {GT_BTNMAP_NEW_LINE,0},
    {GT_SYMBOL_EN,5}, {GT_SYMBOL_WELL_NUMBER,5}, {GT_SYMBOL_LEFT,4}, {GT_SYMBOL_SPACE,12}, {GT_SYMBOL_RIGHT,4}, {GT_SYMBOL_NEW_LINE,8},
    {NULL,0},
};

static GT_ATTRIBUTE_RAM_DATA const gt_keypad_map_st _list_default_26_key[] = {
    {.type = GT_KEYPAD_TYPE_LOWER, .map = _low_map_default_26_key},
    {.type = GT_KEYPAD_TYPE_UPPER, .map = _up_map_default_26_key},
    {.type = GT_KEYPAD_TYPE_SYMBOL_EN, .map = _symbol_en_map_default_26_key},
    {.type = GT_KEYPAD_TYPE_CH, .map = _ch_map_default_26_key},
};

#endif // _DEF_STYLE_26_KEY

//
#ifdef _DEF_STYLE_NUMBER
static GT_ATTRIBUTE_RAM_DATA const gt_map_st _numb_map_default_number[] = {
    {"1",1}, {"2",1}, {"3",1}, {GT_BTNMAP_NEW_LINE,0},
    {"4",1}, {"5",1}, {"6",1}, {GT_BTNMAP_NEW_LINE,0},
    {"7",1}, {"8",1}, {"9",1}, {GT_BTNMAP_NEW_LINE,0},
    {GT_SYMBOL_BACKSPACE,1}, {"0",1}, {GT_SYMBOL_NEW_LINE,1}, {NULL,0},
};

static GT_ATTRIBUTE_RAM_DATA const gt_keypad_map_st _list_default_number[] = {
    {.type = GT_KEYPAD_TYPE_NUMBER, .map = _numb_map_default_number},
};

#endif // _DEF_STYLE_NUMBER

#ifdef _DEF_STYLE_OTHER_4x5
// w = 44 * 4 + 3 * 5 = 176 + 15 = 191
// h = 44 * 5 + 3 * 6 = 220 + 18 = 238
static GT_ATTRIBUTE_RAM_DATA const gt_map_st _low_map_default_tlb[] = {
    {"a",1}, {"b",1}, {"c",1}, {"d",1}, {GT_BTNMAP_NEW_LINE,0},
    {"e",1}, {"f",1}, {"g",1}, {"h",1}, {GT_BTNMAP_NEW_LINE,0},
    {"i",1}, {"j",1}, {"k",1}, {"l",1}, {GT_BTNMAP_NEW_LINE,0},
    {"m",1}, {"n",1}, {"o",1}, {"p",1}, {GT_BTNMAP_NEW_LINE,0},
    {"q",1}, {"r",1}, {"s",1}, {"t",1}, {GT_BTNMAP_NEW_LINE,0},
    {"u",1}, {"v",1}, {"w",1}, {"x",1}, {GT_BTNMAP_NEW_LINE,0},
    {"y",1}, {"z",1}, {NULL,2}, {NULL,0},
};

static GT_ATTRIBUTE_RAM_DATA const gt_map_st _up_map_default_tlb[] = {
    {"A",1}, {"B",1}, {"C",1}, {"D",1}, {GT_BTNMAP_NEW_LINE,0},
    {"E",1}, {"F",1}, {"G",1}, {"H",1}, {GT_BTNMAP_NEW_LINE,0},
    {"I",1}, {"J",1}, {"K",1}, {"L",1}, {GT_BTNMAP_NEW_LINE,0},
    {"M",1}, {"N",1}, {"O",1}, {"P",1}, {GT_BTNMAP_NEW_LINE,0},
    {"Q",1}, {"R",1}, {"S",1}, {"T",1}, {GT_BTNMAP_NEW_LINE,0},
    {"U",1}, {"V",1}, {"W",1}, {"X",1}, {GT_BTNMAP_NEW_LINE,0},
    {"Y",1}, {"Z",1}, {NULL,2}, {NULL,0},
};

static GT_ATTRIBUTE_RAM_DATA const gt_map_st _numb_map_default_tlb[] = {
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

static GT_ATTRIBUTE_RAM_DATA const gt_keypad_map_st _list_default_tlb[] = {
    {.type = GT_KEYPAD_TYPE_LOWER, .map = _low_map_default_tlb},
    {.type = GT_KEYPAD_TYPE_UPPER, .map = _up_map_default_tlb},
    {.type = GT_KEYPAD_TYPE_NUMBER, .map = _numb_map_default_tlb},
};
#endif // _DEF_STYLE_OTHER_4x5
//
#ifdef _DEF_STYLE_OTHER_11x3
// w = 50 * 11 + 3 * 12 = 550 + 36 = 586
// h = 38 * 3 + 3 * 4 = 114 + 12 = 126
static GT_ATTRIBUTE_RAM_DATA const gt_map_st _low_map_default_fyb_369[] = {
    {"a",1}, {"b",1}, {"c",1}, {"d",1}, {"e",1}, {"f",1}, {"g",1}, {"h",1}, {"i",1}, {"j",1}, {"k",1}, {GT_BTNMAP_NEW_LINE,0},
    {"l",1}, {"m",1}, {"n",1}, {"o",1}, {"p",1}, {"q",1}, {"r",1}, {"s",1}, {"t",1}, {"u",1}, {"v",1}, {GT_BTNMAP_NEW_LINE,0},
    {"w",1}, {"x",1}, {"y",1}, {"z",1}, {NULL,1}, {NULL,1}, {NULL,1}, {NULL,1}, {NULL,1}, {NULL,1}, {NULL,1}, {NULL,0},
};

static GT_ATTRIBUTE_RAM_DATA const gt_map_st _up_map_default_fyb_369[] = {
    {"A",1}, {"B",1}, {"C",1}, {"D",1}, {"E",1}, {"F",1}, {"G",1}, {"H",1}, {"I",1}, {"J",1}, {"K",1}, {GT_BTNMAP_NEW_LINE,0},
    {"L",1}, {"M",1}, {"N",1}, {"O",1}, {"P",1}, {"Q",1}, {"R",1}, {"S",1}, {"T",1}, {"U",1}, {"V",1}, {GT_BTNMAP_NEW_LINE,0},
    {"W",1}, {"X",1}, {"Y",1}, {"Z",1}, {NULL,1}, {NULL,1}, {NULL,1}, {NULL,1}, {NULL,1}, {NULL,1}, {NULL,1}, {NULL,0},
};

static GT_ATTRIBUTE_RAM_DATA const gt_map_st _numb_map_default_fyb_369[] = {
    {"1",1}, {"2",1}, {"3",1}, {"4",1}, {"5",1}, {"6",1}, {"7",1}, {"8",1}, {"9",1}, {"0",1}, {"@",1}, {GT_BTNMAP_NEW_LINE,0},
    {"=",1}, {"+",1}, {"-",1}, {"*",1}, {"/",1}, {",",1}, {".",1}, {";",1}, {"?",1}, {"\'",1}, {"\"",1}, {GT_BTNMAP_NEW_LINE,0},
    {":",1}, {"!",1}, {"(",1}, {")",1}, {"<",1}, {">",1}, {"[",1}, {"]",1}, {"_",1}, {"~",1}, {"\\",1}, {GT_BTNMAP_NEW_LINE,0},
    {"|",1}, {"`",1}, {"#",1}, {"$",1}, {"%",1}, {"^",1}, {"&",1}, {"{",1}, {"}",1}, {NULL,1}, {NULL,1}, {NULL,0},
};

static GT_ATTRIBUTE_RAM_DATA const gt_keypad_map_st _list_default_fyb_369[] = {
    {.type = GT_KEYPAD_TYPE_LOWER, .map = _low_map_default_fyb_369},
    {.type = GT_KEYPAD_TYPE_UPPER, .map = _up_map_default_fyb_369},
    {.type = GT_KEYPAD_TYPE_NUMBER, .map = _numb_map_default_fyb_369},
};
#endif // _DEF_STYLE_OTHER_11x3

static GT_ATTRIBUTE_RAM_DATA const _gt_def_style_param_st _def_style_list[] = {
#ifdef _DEF_STYLE_26_KEY
    {
        .def_style = GT_KEYPAD_STYLE_26_KEY,
        .map_list = _list_default_26_key,
        .map_total = sizeof(_list_default_26_key) / sizeof(gt_keypad_map_st),
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
        ._show_special_btn_cb = _show_special_btn_cb,
        ._push_btn_kv_cb = _key_push_btn_kv_cb,
        .ctrl_key_bg_color = 0xBAC1CB,
        .ctrl_key_border_width = 0,
        .ctrl_key_border_color = 0xBAC1CB,
    },
#endif // _DEF_STYLE_26_KEY

#ifdef _DEF_STYLE_NUMBER
    {
        .def_style = GT_KEYPAD_STYLE_NUMBER,
        .map_list = _list_default_number,
        .map_total = sizeof(_list_default_number) / sizeof(gt_keypad_map_st),
        .radius = 6,
        .bg_color = 0x242424,
        .border_width = 0,
        .border_color = 0x242424,
        .key_h = 44,
        .key_radius = 6,
        .key_bg_color = 0x646464,
        .key_border_width = 0,
        .key_border_color = 0x646464,
        .key_x_space = 3,
        .key_y_space = 6,
        ._show_special_btn_cb = _show_special_btn_cb,
        ._push_btn_kv_cb = _key_push_btn_kv_cb,
        .ctrl_key_bg_color = 0x3E3E3E,
        .ctrl_key_border_width = 0,
        .ctrl_key_border_color = 0x3E3E3E,
    },
#endif // _DEF_STYLE_NUMBER

#ifdef _DEF_STYLE_OTHER_4x5
    {
        .def_style = GT_KEYPAD_STYLE_OTHER_4x5,
        .map_list = _list_default_tlb,
        .map_total = sizeof(_list_default_tlb) / sizeof(gt_keypad_map_st),
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
        ._show_special_btn_cb = NULL,
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
        ._show_special_btn_cb = NULL,
        ._push_btn_kv_cb = NULL,
        .ctrl_key_bg_color = 0x22272C,
        .ctrl_key_border_width = 0,
        .ctrl_key_border_color = 0x22272C,
    },
#endif // _DEF_STYLE_OTHER_11x3
};

/* macros ---------------------------------------------------------------*/


/* static functions -----------------------------------------------------*/
static _cmd_type _get_cmd_type(const char* const kv)
{
    if(NULL == kv) {
        return _CMD_NONE;
    }

    uint16_t count = sizeof(_cmd_map_list) / sizeof(_cmd_map_st);

    for (uint16_t i = 0; i < count; i++) {
        const char* _cmd = _cmd_map_list[i].kv;
        if(NULL == _cmd) {
            return _CMD_NONE;
        }

        int kv_len = strlen(kv);
        int cmd_len = strlen(_cmd_map_list[i].kv);

        if(kv_len != cmd_len) {
            continue;
        }

        if (strcmp(kv, _cmd) == 0) {
            return _cmd_map_list[i].cmd;
        }
    }
    return _CMD_NONE;
}

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

static void _py_input_method_get_chinese(gt_py_input_method_st* py_input_mt)
{
    if (!gt_py_input_method_get_chinese(py_input_mt)) {
        return ;
    }

    gt_py_input_method_chinese_data_handler(py_input_mt);
}

static void _cmd_backspace_handler(gt_obj_st * obj, gt_obj_st * input, const char* const kv)
{
    uint16_t type = gt_btnmap_get_map_type(obj);
    gt_py_input_method_st* py_input_mt = gt_btnmap_get_py_input_method(obj);

    gt_input_del_value(input);

    if( py_input_mt
        && GT_KEYPAD_TYPE_CH == type
        && gt_py_input_method_get_ascii_numb(py_input_mt) > 0
    ) {
        gt_py_input_method_pop_ascii(py_input_mt);
        _py_input_method_get_chinese(py_input_mt);
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_REDRAW, NULL);
    }
}

static void _cmd_enter_handler(gt_obj_st * obj, gt_obj_st * input, const char* const kv)
{
    gt_obj_st* parent = gt_obj_get_parent(obj);
    if(false == gt_obj_is_type(parent, OBJ_TYPE)) {
        return ;
    }

    _gt_keypad_st* kp = (_gt_keypad_st*)parent;
    if(kp->enter_cb) {
        kp->enter_cb();
    }
}

static void _cmd_lower_case_handler(gt_obj_st * obj)
{
    gt_obj_st* parent = gt_obj_get_parent(obj);
    if(false == gt_obj_is_type(parent, OBJ_TYPE)) {
        return ;
    }
    gt_keypad_set_type(parent, GT_KEYPAD_TYPE_LOWER);
}

static void _cmd_upper_case_handler(gt_obj_st * obj)
{
    gt_obj_st* parent = gt_obj_get_parent(obj);
    if(false == gt_obj_is_type(parent, OBJ_TYPE)) {
        return ;
    }
    gt_keypad_set_type(parent, GT_KEYPAD_TYPE_UPPER);
}

static void _cmd_ch_handler(gt_obj_st * obj, gt_obj_st * input, const char* const kv)
{
    gt_obj_st* parent = gt_obj_get_parent(obj);
    if(false == gt_obj_is_type(parent, OBJ_TYPE)) {
        return ;
    }
    gt_py_input_method_st* py_input_mt = gt_btnmap_get_py_input_method(obj);
    if(py_input_mt) {
        gt_py_input_method_clean(py_input_mt);
    }

    gt_keypad_set_type(parent, GT_KEYPAD_TYPE_CH);
}

static void _cmd_en_handler(gt_obj_st * obj)
{
    gt_obj_st* parent = gt_obj_get_parent(obj);
    if(false == gt_obj_is_type(parent, OBJ_TYPE)) {
        return ;
    }

    gt_keypad_set_type(parent, GT_KEYPAD_TYPE_LOWER);
}

static void _cmd_well_number_handler(gt_obj_st * obj)
{
    gt_obj_st* parent = gt_obj_get_parent(obj);
    if(false == gt_obj_is_type(parent, OBJ_TYPE)) {
        return ;
    }

    uint16_t type = gt_btnmap_get_map_type(obj);
    gt_keypad_type_te tmp_type = (type != GT_KEYPAD_TYPE_SYMBOL_EN) ? GT_KEYPAD_TYPE_SYMBOL_EN : GT_KEYPAD_TYPE_LOWER;
    gt_keypad_set_type(parent, tmp_type);
}

static void _cmd_left_handler(gt_obj_st * obj, gt_obj_st * input, const char* const kv)
{
    uint16_t type = gt_btnmap_get_map_type(obj);
    gt_py_input_method_st* py_input_mt = gt_btnmap_get_py_input_method(obj);

    if(GT_KEYPAD_TYPE_CH == type && py_input_mt->ascii_numb > 0) {
        return ;
    }

    gt_input_move_left_pos_cursor(input);
}

static void _cmd_right_handler(gt_obj_st * obj, gt_obj_st * input, const char* const kv)
{
    uint16_t type = gt_btnmap_get_map_type(obj);
    gt_py_input_method_st* py_input_mt = gt_btnmap_get_py_input_method(obj);

    if(GT_KEYPAD_TYPE_CH == type && py_input_mt->ascii_numb > 0) {
        return ;
    }

    gt_input_move_right_pos_cursor(input);
}

static void _cmd_prev_handler(gt_obj_st * obj, gt_obj_st * input, const char* const kv)
{
    uint16_t type = gt_btnmap_get_map_type(obj);
    gt_py_input_method_st* py_input_mt = gt_btnmap_get_py_input_method(obj);

    if (!py_input_mt || py_input_mt->chinese_numb <= 0) {
        return;
    }

    gt_py_input_method_last_page(py_input_mt);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_REDRAW, NULL);
}

static void _cmd_next_handler(gt_obj_st * obj, gt_obj_st * input, const char* const kv)
{
    uint16_t type = gt_btnmap_get_map_type(obj);
    gt_py_input_method_st* py_input_mt = gt_btnmap_get_py_input_method(obj);

    if (!py_input_mt || py_input_mt->chinese_numb <= 0) {
        return;
    }

    gt_py_input_method_next_page(py_input_mt);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_REDRAW, NULL);
}

static void _cmd_default_handler(gt_obj_st * obj, gt_obj_st * input, const char* const kv)
{
    uint16_t type = gt_btnmap_get_map_type(obj);
    gt_py_input_method_st* py_input_mt = gt_btnmap_get_py_input_method(obj);
    gt_size_t idx = _gt_get_spell_index(kv);

    if(GT_KEYPAD_TYPE_CH != type) {
        gt_input_append_value(input, (char*)kv);
        return ;
    }

    if(!py_input_mt) {
        return ;
    }

    if(idx < 0) {
        if(py_input_mt->ascii_numb >= GT_PY_ASCII_RMS_COUNT) {
            return ;
        }
        gt_input_append_value(input, (char*)kv);
        gt_py_input_method_push_ascii(py_input_mt, *kv);
        _py_input_method_get_chinese(py_input_mt);
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_REDRAW, NULL);
        return ;
    }
    else if(0 == idx){
        gt_py_input_method_clean(py_input_mt);
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_REDRAW, NULL);
        return ;
    }

    // idx > 0
    uint8_t *tmp_str = _gt_spell_get_text(py_input_mt, idx);
    for(uint8_t i = 0; i < py_input_mt->ascii_numb; ++i) {
        gt_input_del_value(input);
    }
    gt_input_append_value(input, (char*)tmp_str);

    if(0 == py_input_mt->ascii_numb && 0 == py_input_mt->chinese_numb) {
        return ;
    }

    gt_py_input_method_select_text(py_input_mt, idx);
    gt_py_input_method_chinese_data_handler(py_input_mt);
    gt_event_send(obj, GT_EVENT_TYPE_DRAW_REDRAW, NULL);
}

static void _key_push_btn_kv_cb(gt_obj_st * obj, gt_obj_st * input, const char* const kv) {
    if(!obj || !kv) {
        return ;
    }

    int len = strlen(kv);
    if(strlen(GT_SYMBOL_SPACE) == len && 0 == strcmp(kv, GT_SYMBOL_SPACE)) {
        gt_input_append_value(input, " ");
        return ;
    }

    _cmd_type cmd = _get_cmd_type(kv);

    switch (cmd) {
        case _CMD_BACKSPACE:
            _cmd_backspace_handler(obj, input, kv);
            break;
        case _CMD_ENTER:
            _cmd_enter_handler(obj, input, kv);
            break;
        case _CMD_LOWER_CASE:
            _cmd_upper_case_handler(obj);
            break;
        case _CMD_UPPER_CASE:
            _cmd_lower_case_handler(obj);
            break;
        case _CMD_CH:
            _cmd_ch_handler(obj, input, kv);
            break;
        case _CMD_EN:
            _cmd_en_handler(obj);
            break;
        case _CMD_WELL_NUMBER:
            _cmd_well_number_handler(obj);
            break;
        case _CMD_LEFT:
            _cmd_left_handler(obj, input, kv);
            break;
        case _CMD_RIGHT:
            _cmd_right_handler(obj, input, kv);
            break;
        case _CMD_PREV:
            _cmd_prev_handler(obj, input, kv);
            break;
        case _CMD_NEXT:
            _cmd_next_handler(obj, input, kv);
            break;
        default:
            _cmd_default_handler(obj, input, kv);
            break;
    }

}

static bool _show_special_btn_cb(gt_obj_st* obj, const char* const kv, gt_attr_font_st* font_attr)
{
    _cmd_type cmd_type = _get_cmd_type(kv);
    if(_CMD_NONE != cmd_type) {
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

static GT_ATTRIBUTE_RAM_TEXT void _gt_switch_default_style(gt_obj_st * keypad, gt_keypad_default_style_st def_style) {
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

    // def
    if( GT_KEYPAD_STYLE_OTHER_4x5 == style->def_style
        || GT_KEYPAD_STYLE_OTHER_11x3 == style->def_style
    ){
        gt_keypad_set_key_height_auto_fill(keypad, false);
    }
    else {
        gt_keypad_set_key_height_auto_fill(keypad, true);
    }

    gt_btnmap_set_disp_special_btn_handler(style->btnmap, item->_show_special_btn_cb);
    gt_btnmap_set_push_btn_kv_handler(style->btnmap, item->_push_btn_kv_cb);
    gt_keypad_set_map_list(keypad, (gt_keypad_map_st*)item->map_list, item->map_total);
}

static GT_ATTRIBUTE_RAM_TEXT inline void _set_minimum_width(gt_obj_st * keypad) {
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;

    uint16_t minimum_width = gt_btnmap_get_calc_minimum_width(style->btnmap);
    if (keypad->area.w < minimum_width) {
        keypad->area.w = minimum_width;
    }
}

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
    rect_attr.bg_color = obj->bgcolor;
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
    _gt_obj_scroll_internal(obj, 0, 0);
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
    keypad->process_attr.scroll_prev.y = 0;
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

void gt_keypad_set_auto_hide(gt_obj_st * keypad, bool auto_hide)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st* style = (_gt_keypad_st*)keypad;
    style->auto_hide = auto_hide ? 1 : 0;
}

bool gt_keypad_get_auto_hide(gt_obj_st * keypad)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return false;
    }
    _gt_keypad_st* style = (_gt_keypad_st*)keypad;
    return style->auto_hide;
}

void gt_keypad_hide(gt_obj_st * keypad)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st* style = (_gt_keypad_st*)keypad;
    if (false == style->auto_hide) {
        return;
    }
    gt_obj_set_visible(keypad, false);
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
    if(map_num > 0){
        _gt_keypad_switch_type(keypad, map_list[0].type, true);
    }
}

void gt_keypad_set_target(gt_obj_st * keypad, gt_obj_st * tar_input)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    if (NULL == tar_input) {
        return;
    }
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;
    gt_btnmap_set_input(style->btnmap, tar_input);
    gt_input_hooking_keypad(tar_input, keypad);
}

gt_obj_st * gt_keypad_get_target(gt_obj_st * keypad)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return NULL;
    }
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;
    return gt_btnmap_get_input(style->btnmap);
}

void gt_keypad_set_color_background(gt_obj_st * keypad, gt_color_t color)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    gt_obj_set_bgcolor(keypad, color);
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
    gt_obj_set_radius(keypad, radius);
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
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
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
#else
void gt_keypad_set_font_family(gt_obj_st * keypad, gt_family_t family)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;
    gt_btnmap_set_font_family(style->btnmap, family);
}
void gt_keypad_set_font_cjk(gt_obj_st* keypad, gt_font_cjk_et cjk)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;
    gt_btnmap_set_font_cjk(style->btnmap, cjk);
}

#endif

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

void gt_keypad_set_font_style(gt_obj_st * keypad, gt_font_style_et font_style)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;
    gt_btnmap_set_font_style(style->btnmap, font_style);
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

gt_py_input_method_st* gt_keypad_get_py_input_method(gt_obj_st * keypad)
{
    if( false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return NULL;
    }
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;
    return gt_btnmap_get_py_input_method(style->btnmap);
}

void gt_keypad_set_enter_callback(gt_obj_st * keypad, gt_keypad_enter_cb_t cb)
{
    if (false == gt_obj_is_type(keypad, OBJ_TYPE)) {
        return;
    }
    _gt_keypad_st * style = (_gt_keypad_st * )keypad;
    style->enter_cb = cb;
}

#endif  /** GT_CFG_ENABLE_KEYPAD */
/* end of file ----------------------------------------------------------*/
