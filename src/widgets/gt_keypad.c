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
#include "../core/gt_disp.h"
#include "../font/gt_symbol.h"
#include "gt_conf_widgets.h"

#include "gt_input.h"

/* private define -------------------------------------------------------*/
#define OBJ_TYPE    GT_TYPE_KEYPAD
#define MY_CLASS    &gt_keypad_class


#define SPACE_Y 5
#define SPACE_X 5

#define _GT_KEYPAD_TAG_SPACE        " "
#define _GT_KEYPAD_TAG_NEW_LINE     "\n"
#define _GT_KEYPAD_TAG_UPPERCASE    "ABC"
#define _GT_KEYPAD_TAG_DOWNCASE     "abc"
#define _GT_KEYPAD_TAG_NUMBER       "#+="

#define _GT_KEYPADS_ICON_SIZE       (24)
#define _GT_KEYPADS_SPELL_NUMB      (9)

/* private typedef ------------------------------------------------------*/
typedef struct _gt_keypad_info_s
{
    keypad_handler_cb_t handler_cb;
    struct _gt_keypad_map_s * map;

    /* data */
    uint8_t mode;       /*this mode can set GT_KEYPAD_MODE_XX*/
    uint8_t cnt_lines;  /*record how many lines of the keypad*/
    uint8_t per_line[32];/*record the per of the total width of a line */
}_gt_keypad_info_st;


typedef struct _gt_keypad_s
{
    gt_obj_st * target;
    char *      kv_selected;
    gt_color_t  color_board;  //board color
    gt_color_t  color_key;  //key color
    gt_color_t  color_ctrl;  //ctrl key color

    gt_color_t  font_color;  //font color
    gt_family_t font_family_cn;
    gt_family_t font_family_en;
    gt_family_t font_family_numb;
    uint8_t     font_size;
    uint8_t     font_gray;
    uint8_t     font_align;
    uint8_t     thick_en;
    uint8_t     thick_cn;
    uint8_t     space_x;
    uint8_t     space_y;
    _gt_keypad_info_st info;
}_gt_keypad_st;

typedef struct {
    const char * key;
    const gt_keypad_map_st * map;
    const uint8_t mode;     /* keypad mode */
}_gt_keypad_kv_list_st;

/* static variables -----------------------------------------------------*/
#define _GT_SPELL_ASCII_MAX_NUMB (8)
#define _GT_SPELL_CHINESE_MAX_NUMB (8)
static uint8_t _gt_spell_ascii_numb = 0;
static uint8_t _gt_spell_chinese_numb = 0;
static uint8_t _gt_spell_ascii[_GT_SPELL_ASCII_MAX_NUMB];
static uint8_t _gt_spell_chinese[_GT_SPELL_CHINESE_MAX_NUMB][_GT_SPELL_CHINESE_MAX_NUMB];

#if ((GT_CFG_ENABLE_ZK_SPELL == 1)&&(GT_CFG_ENABLE_ZK_FONT == 1))
    static py_info_st *_gt_spell_info = NULL;
#endif

static const char _gt_spell[_GT_KEYPADS_SPELL_NUMB][8] = {
    "spell_0" , "spell_1" , "spell_2" , "spell_3" , "spell_4" , "spell_5" , "spell_6" , "spell_7" , "spell_8",
};

enum _gt_keypad_mode_e {
    GT_KEYPAD_MODE_LOWER = 0,
    GT_KEYPAD_MODE_UPPER,
    GT_KEYPAD_MODE_NUMBER,
    GT_KEYPAD_MODE_SPELL,
    GT_KEYPAD_MODE_OTHER,

    GT_KEYPAD_MODE_TOTAL_SIZE,
};

static void _init_cb(gt_obj_st * obj);
static void _deinit_cb(gt_obj_st * obj);
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e);

const gt_obj_class_st gt_keypad_class = {
    ._init_cb      = _init_cb,
    ._deinit_cb    = _deinit_cb,
    ._event_cb     = _event_cb,
    .type          = OBJ_TYPE,
    .size_style    = sizeof(_gt_keypad_st)
};


GT_ATTRIBUTE_LARGE_RAM_ARRAY static const gt_keypad_map_st _map_keypad_mode_lower[] = {
    _gt_spell[0],1,  _GT_KEYPAD_TAG_NEW_LINE,0,
    "1",5,  "2",4,  "3",4,  "4",4, "5",4,  "6",4, "7",4,  "8",4,  "9",4,  "0",5, _GT_KEYPAD_TAG_NEW_LINE,0,
    "q",5,  "w",4,  "e",4,  "r",4,  "t",4,  "y",4,  "u",4,  "i",4,  "o",4,  "p",5,  _GT_KEYPAD_TAG_NEW_LINE,0,
    _GT_KEYPAD_TAG_NUMBER,5,  "a",4,  "s",4,  "d",4,  "f",4,  "g",4,  "h",4,  "j",4,  "k",4,  "l",5,  _GT_KEYPAD_TAG_NEW_LINE,0,
    _GT_KEYPAD_TAG_UPPERCASE,7,  "z",4,  "x",4,  "c",4,  "v",4,  "b",4,  "n",4,  "m",4, GT_SYMBOL_BACKSPACE,7,  _GT_KEYPAD_TAG_NEW_LINE,0,
    GT_SYMBOL_KEYBOARD,7,  GT_SYMBOL_LEFT,6,  GT_SYMBOL_EN,10,  GT_SYMBOL_RIGHT,6,  GT_SYMBOL_OK,6, GT_SYMBOL_NEW_LINE,7, NULL,0
};

GT_ATTRIBUTE_LARGE_RAM_ARRAY static const gt_keypad_map_st _map_keypad_mode_upper[] = {
    _gt_spell[0],1,  _GT_KEYPAD_TAG_NEW_LINE,0,
    "1",5,  "2",4,  "3",4,  "4",4, "5",4,  "6",4, "7",4,  "8",4,  "9",4,  "0",5, _GT_KEYPAD_TAG_NEW_LINE,0,
    "Q",5,  "W",4,  "E",4,  "R",4,  "T",4,  "Y",4,  "U",4,  "I",4,  "O",4,  "P",5,  _GT_KEYPAD_TAG_NEW_LINE,0,
    _GT_KEYPAD_TAG_NUMBER,5,  "A",4,  "S",4,  "D",4,  "F",4,  "G",4,  "H",4,  "J",4,  "K",4,  "L",5,  _GT_KEYPAD_TAG_NEW_LINE,0,
    _GT_KEYPAD_TAG_DOWNCASE,7,  "Z",4,  "X",4,  "C",4,  "V",4,  "B",4,  "N",4,  "M",4, GT_SYMBOL_BACKSPACE,7,  _GT_KEYPAD_TAG_NEW_LINE,0,
    GT_SYMBOL_KEYBOARD,7,  GT_SYMBOL_LEFT,6,  GT_SYMBOL_EN,10,  GT_SYMBOL_RIGHT,6,  GT_SYMBOL_OK,6, GT_SYMBOL_NEW_LINE,7, NULL,0
};

GT_ATTRIBUTE_LARGE_RAM_ARRAY static const gt_keypad_map_st _map_keypad_mode_number[] = {
    _gt_spell[0],1,  _GT_KEYPAD_TAG_NEW_LINE,0,
    "!",5,  "@",4,  "#",4,  "$",4,  "%",4,  "^",4,  "&",4,  "*",4,  "(",4,  ")",5,  _GT_KEYPAD_TAG_NEW_LINE,0,
    "-",5,  "+",4,  "=",4,  "_",4, "{",4,  "}",4, "[",4,  "]",4,  "\\",5,  _GT_KEYPAD_TAG_NEW_LINE,0,
    _GT_KEYPAD_TAG_NUMBER,5,  ";",4,  ":",4,  "\"",4,  "\'",4,  "~",4,  "`",4,  "/ ",4,  " |",5,  _GT_KEYPAD_TAG_NEW_LINE,0,
    _GT_KEYPAD_TAG_UPPERCASE,8,  ",",6,  ".",6,  "?",6,  "<",6,  ">",6,  GT_SYMBOL_BACKSPACE,8,  _GT_KEYPAD_TAG_NEW_LINE,0,
    GT_SYMBOL_KEYBOARD,7,  GT_SYMBOL_LEFT,6,  GT_SYMBOL_EN,10,  GT_SYMBOL_RIGHT,6,  GT_SYMBOL_OK,6,  GT_SYMBOL_NEW_LINE,7,  NULL,0
};

GT_ATTRIBUTE_LARGE_RAM_ARRAY static const gt_keypad_map_st _map_keypad_mode_spell[] = {
    _gt_spell[0],1,  _GT_KEYPAD_TAG_NEW_LINE,0,
    GT_SYMBOL_PREV,5,  _gt_spell[1],4,  _gt_spell[2],4,  _gt_spell[3],4,  _gt_spell[4],4,  _gt_spell[5],4,  _gt_spell[6],4,  _gt_spell[7],4,  _gt_spell[8],4,  GT_SYMBOL_NEXT,5, _GT_KEYPAD_TAG_NEW_LINE,0,
    "q",5,  "w",4,  "e",4,  "r",4,  "t",4,  "y",4,  "u",4,  "i",4,  "o",4,  "p",5,  _GT_KEYPAD_TAG_NEW_LINE,0,
    _GT_KEYPAD_TAG_NUMBER,5,  "a",4,  "s",4,  "d",4,  "f",4,  "g",4,  "h",4,  "j",4,  "k",4,  "l",5,  _GT_KEYPAD_TAG_NEW_LINE,0,
    _GT_KEYPAD_TAG_UPPERCASE,7,  "z",4,  "x",4,  "c",4,  "v",4,  "b",4,  "n",4,  "m",4, GT_SYMBOL_BACKSPACE,7,  _GT_KEYPAD_TAG_NEW_LINE,0,
    GT_SYMBOL_KEYBOARD,7,  GT_SYMBOL_LEFT,6,  GT_SYMBOL_SPELL,10,  GT_SYMBOL_RIGHT,6,  GT_SYMBOL_OK,6,  GT_SYMBOL_NEW_LINE,7,  NULL,0
};

GT_ATTRIBUTE_LARGE_RAM_ARRAY static const char * _gt_keypad_ctrl_key_list[] = {
    GT_SYMBOL_NEW_LINE,
    GT_SYMBOL_KEYBOARD,
    GT_SYMBOL_LEFT,
    GT_SYMBOL_RIGHT,
    GT_SYMBOL_OK,
    GT_SYMBOL_BACKSPACE,
    _GT_KEYPAD_TAG_UPPERCASE,
    _GT_KEYPAD_TAG_DOWNCASE,
    _GT_KEYPAD_TAG_NUMBER,
    GT_SYMBOL_NEXT ,
    GT_SYMBOL_PREV ,
    NULL,
};

GT_ATTRIBUTE_LARGE_RAM_ARRAY static const char * _gt_keypad_icon_key_list[] = {

    GT_SYMBOL_NEW_LINE ,
    GT_SYMBOL_EN ,
    GT_SYMBOL_SPELL ,
    GT_SYMBOL_BACKSPACE ,
    GT_SYMBOL_LEFT ,
    GT_SYMBOL_RIGHT ,
    GT_SYMBOL_NEXT ,
    GT_SYMBOL_PREV ,
    GT_SYMBOL_KEYBOARD ,
    GT_SYMBOL_OK ,
    NULL,
};


GT_ATTRIBUTE_LARGE_RAM_ARRAY static const _gt_keypad_kv_list_st _gt_keypad_mode_key_mapping_list[] = {
    { _GT_KEYPAD_TAG_UPPERCASE, _map_keypad_mode_upper, GT_KEYPAD_MODE_UPPER },
    { _GT_KEYPAD_TAG_DOWNCASE,  _map_keypad_mode_lower, GT_KEYPAD_MODE_LOWER },
    { _GT_KEYPAD_TAG_NUMBER,    _map_keypad_mode_number, GT_KEYPAD_MODE_NUMBER },
    { GT_SYMBOL_KEYBOARD, _map_keypad_mode_spell, GT_KEYPAD_MODE_SPELL },
};

/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_area_st _gt_keypad_get_key_area(gt_obj_st * keypad, char const * const kv);
static void _gt_keypad_update_map(gt_obj_st * keypad, gt_keypad_map_st const * const map);

static uint8_t* _gt_spell_get_text( uint8_t index , uint8_t mode)
{
    if(index == 0)
    {
        if((_gt_spell_ascii_numb > 0) && (GT_KEYPAD_MODE_SPELL == mode))
        {
            return _gt_spell_ascii;
        }
        return (uint8_t * )"";
    }

    if((_gt_spell_chinese_numb > 0 ) && (GT_KEYPAD_MODE_SPELL == mode))
    {
        return _gt_spell_chinese[index-1];
    }
    return (uint8_t * )"";
}

static int _gt_get_spell_index(char const * const kv ,  uint8_t mode)
{
    int idx = 0;

    if( kv == NULL){
        return -1;
    }

    for ( idx = 0; idx < _GT_KEYPADS_SPELL_NUMB; idx++)
    {
        if( strcmp((const char * )_gt_spell[idx], kv) == 0 ){
            return idx;
        }
    }

    return -1;
}

static uint8_t* _gt_keypad_get_spell_text(char const * const kv ,  uint8_t mode)
{
    if( kv == NULL){
        return NULL;
    }

    int idx = -1;

    idx = _gt_get_spell_index(kv , mode);

    if(idx < 0)
    {
        return NULL;
    }
#if ((GT_CFG_ENABLE_ZK_SPELL == 1)&&(GT_CFG_ENABLE_ZK_FONT == 1))
    return _gt_spell_get_text(idx , mode);
#else
    return (uint8_t * )"";
#endif

}

static char const * const _gt_keypad_get_click_kv(gt_obj_st * obj)
{
    _gt_keypad_st * style = obj->style;
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


static void _gt_keypad_set_kv_selected(gt_obj_st * keypad, char const * const kv)
{
    extern void _gt_disp_refr_append_area(gt_area_st * area);

    _gt_keypad_st * style = keypad->style;
    gt_area_st area;
    char * kv_old = style->kv_selected;
    style->kv_selected = (char * )kv;
    if( kv_old != kv ){
        area = _gt_keypad_get_key_area(keypad, kv_old);
        _gt_disp_refr_append_area(&area);
    }
    area = _gt_keypad_get_key_area(keypad, kv);
    _gt_disp_refr_append_area(&area);
}

static char * _gt_keypad_get_kv_selected(gt_obj_st * keypad)
{
    _gt_keypad_st * style = keypad->style;
    return style->kv_selected;
}


/**
 * @brief get abs area of key
 *
 * @param keypad keypad obj
 * @param kv the key keyvalue
 * @return gt_area_st
 */
static gt_area_st _gt_keypad_get_key_area(gt_obj_st * keypad, char const * const kv)
{
    gt_area_st area_key = {
        .x = 0,.y = 0,.w = 0,.h = 0
    };
    if( NULL == kv ){
        return area_key;
    }
    _gt_keypad_st * style = keypad->style;
    gt_keypad_map_st * map_keypad = style->info.map;
    int idx = 0;

    uint8_t h_key = ((keypad->area.h - SPACE_Y)/ (style->info.cnt_lines+1)) - SPACE_Y;
    uint8_t per_one_width = (keypad->area.w / style->info.per_line[0]);
    uint8_t line = 0;

    area_key.h = h_key;
    area_key.y = keypad->area.y + SPACE_Y;
    area_key.x = keypad->area.x + SPACE_X;
    while ( map_keypad[idx].kv != NULL ) {

        area_key.w = map_keypad[idx].w * per_one_width - SPACE_X;

        if( map_keypad[idx+1].kv && strcmp(map_keypad[idx+1].kv, _GT_KEYPAD_TAG_NEW_LINE) == 0 ){
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
        if( map_keypad[idx].kv && strcmp(map_keypad[idx].kv, _GT_KEYPAD_TAG_NEW_LINE) == 0 ){
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

/**
 * @brief The keypad mapping control button is pressed and lifted to perform effect switching
 *
 * @param keypad
 * @param update_map true: handle update keypad map; false: get matching state
 * @return true  match ctrl key
 * @return false no match ctrl key
 */
static bool _gt_keypad_is_switch_key_mapping_pressed(gt_obj_st * keypad, const bool update_map)
{
    _gt_keypad_st * style = keypad->style;
    _gt_keypad_kv_list_st const * list_p = _gt_keypad_mode_key_mapping_list;
    uint16_t idx = 0, len = sizeof(_gt_keypad_mode_key_mapping_list) / sizeof(_gt_keypad_kv_list_st);
    bool ret = false;

    if (NULL == style->kv_selected) {
        return ret;
    }
    while(idx < len) {
        if ( 0 == strcmp(style->kv_selected, list_p[idx].key) ) {
            if (update_map) {
                _gt_keypad_update_map(keypad, list_p[idx].map);
            }
            ret = true;
            break;
        }
        ++idx;
    }
    return ret;
}

static uint8_t _gt_keypad_change_mode(gt_obj_st * keypad, gt_keypad_map_st const * const map)
{
    _gt_keypad_st * style = keypad->style;
    _gt_keypad_kv_list_st const * list_p = _gt_keypad_mode_key_mapping_list;
    uint16_t idx = 0, len = sizeof(_gt_keypad_mode_key_mapping_list) / sizeof(_gt_keypad_kv_list_st);

    style->info.mode = GT_KEYPAD_MODE_OTHER;

    while(idx < len) {
        if (map == list_p[idx].map) {
            style->info.mode = list_p[idx].mode;
            break;
        }
        ++idx;
    }

    return style->info.mode;
}

static bool _gt_keypad_check_key_is_ctl(char const * const kv){
    if( kv == NULL ){
        return false;
    }
    int idx = 0;
    while( _gt_keypad_ctrl_key_list[idx] != NULL ){
        if( strcmp(_gt_keypad_ctrl_key_list[idx], kv) == 0 ){
            return true;
        }
        idx++;
    }
    return false;
}

static bool _gt_keypad_check_key_is_icon(char const * const kv){
    if( kv == NULL ){
        return false;
    }
    int idx = 0;
    while( _gt_keypad_icon_key_list[idx] != NULL ){
        if( strcmp(_gt_keypad_icon_key_list[idx], kv) == 0 ){
            return true;
        }
        idx++;
    }
    return false;
}

static void _gt_keypad_spell_event_cb(gt_obj_st * keypad)
{
    _gt_keypad_st * style = keypad->style;
    uint8_t i = 0 , len , *tmp_str = NULL;
    uint32_t unicode ;

    if( style->info.handler_cb != NULL ){
        style->info.handler_cb(keypad);
        return;
    }

    if( style->kv_selected == NULL ){
        return;
    }

    if (_gt_keypad_is_switch_key_mapping_pressed(keypad, false)) {
        return;
    }

    if(style->target == NULL){
        return;
    }


    if( strcmp(style->kv_selected, GT_SYMBOL_EN) == 0 ||
        strcmp(style->kv_selected, GT_SYMBOL_SPELL) == 0)
    {
        gt_input_append_value(style->target, _GT_KEYPAD_TAG_SPACE);
    }
    else if(strcmp(style->kv_selected, GT_SYMBOL_BACKSPACE) == 0 ){
        if(_gt_spell_ascii_numb > 0){
#if ((GT_CFG_ENABLE_ZK_SPELL == 1)&&(GT_CFG_ENABLE_ZK_FONT == 1))
            _gt_spell_ascii[--_gt_spell_ascii_numb] = '\0';
            goto SPELL_GET;
#endif
        }
        else{
            gt_input_del_value(style->target);
        }
    }
    else if( strcmp(style->kv_selected, GT_SYMBOL_LEFT) == 0 ){
        gt_input_move_left_pos_cursor(style->target);
    }
    else if( strcmp(style->kv_selected, GT_SYMBOL_RIGHT) == 0 ){
        gt_input_move_right_pos_cursor(style->target);
    }
    else if( strcmp(style->kv_selected, GT_SYMBOL_NEW_LINE) == 0 ){
        gt_input_append_value(style->target, _GT_KEYPAD_TAG_NEW_LINE);
    }
    else if( strcmp(style->kv_selected, GT_SYMBOL_PREV) == 0 ){
#if ((GT_CFG_ENABLE_ZK_SPELL == 1)&&(GT_CFG_ENABLE_ZK_FONT == 1))
        if(gt_pinyin_last_page(_gt_spell_info)){
            goto  SPELL_CODE_CONV;
        }
#endif
    }
    else if( strcmp(style->kv_selected, GT_SYMBOL_NEXT) == 0 ){
#if ((GT_CFG_ENABLE_ZK_SPELL == 1)&&(GT_CFG_ENABLE_ZK_FONT == 1))
        if(gt_pinyin_next_page(_gt_spell_info)){
            goto  SPELL_CODE_CONV;
        }
#endif
    }
    else{
        if (!_gt_keypad_check_key_is_ctl(style->kv_selected)) {

            if((strcmp(style->kv_selected, (const char * )_gt_spell[0]) == 0))
            {
                return ;
            }

            tmp_str = _gt_keypad_get_spell_text(style->kv_selected , style->info.mode);
            if(tmp_str)
            {
                if(GT_ENCODING_UTF8 == gt_project_encoding_get())
                {
                    gt_input_append_value(style->target, (char * )tmp_str);
                }
                else
                {
                    gt_input_append_value_encoding(style->target, (char * )tmp_str , GT_ENCODING_UTF8);
                }

#if ((GT_CFG_ENABLE_ZK_SPELL == 1)&&(GT_CFG_ENABLE_ZK_FONT == 1))
                int tmp_index = _gt_get_spell_index(style->kv_selected , style->info.mode);
                if(tmp_index <= 0 || tmp_index > _gt_spell_chinese_numb)
                {
                    return ;
                }
                uint8_t tmp_text[2];
                gt_pinyin_select_text(_gt_spell_info , tmp_index , tmp_text);
                _gt_spell_ascii_numb = _gt_spell_info->content->ascii_numb;

                goto  SPELL_CODE_CONV;
#endif
            }

#if ((GT_CFG_ENABLE_ZK_SPELL == 1)&&(GT_CFG_ENABLE_ZK_FONT == 1))
            // 拼音输入法
            if(_gt_spell_ascii_numb >= _GT_SPELL_ASCII_MAX_NUMB - 1)
            {
                return ;
            }


            _gt_spell_ascii[_gt_spell_ascii_numb++] = *style->kv_selected;
            _gt_spell_ascii[_gt_spell_ascii_numb] = '\0';

SPELL_GET:
            //
            if(!gt_pinyin_full_keyboard_get(_gt_spell_ascii))
            {
                _gt_spell_chinese_numb = _gt_spell_info->content->chinese_numb;
                gt_event_send(keypad, GT_EVENT_TYPE_DRAW_REDRAW, NULL);
                return ;
            }

SPELL_CODE_CONV:
            _gt_spell_chinese_numb = _gt_spell_info->content->chinese_numb;
            gt_memset_0(_gt_spell_chinese , _GT_SPELL_CHINESE_MAX_NUMB * _GT_SPELL_CHINESE_MAX_NUMB);
            for(i = 0 ; i < _gt_spell_chinese_numb ; i++)
            {

                unicode = GBKToUnicode((((uint32_t)_gt_spell_info->content->chinese[2*i]) << 8) |_gt_spell_info->content->chinese[2*i+1]);
                len = gt_unicode_to_utf8(_gt_spell_chinese[i] , unicode);
                _gt_spell_chinese[i][len]='\0';

            }
            gt_event_send(keypad, GT_EVENT_TYPE_DRAW_REDRAW, NULL);
#endif
        }
    }

}

static _gt_keypad_st * _get_checked_style(gt_obj_st * keypad) {
    _gt_keypad_st * style = keypad->style;
    if( style->info.handler_cb != NULL ){
        style->info.handler_cb(keypad);
        return NULL;
    }

    if( style->kv_selected == NULL ){
        return NULL;
    }

    if (_gt_keypad_is_switch_key_mapping_pressed(keypad, false)) {
        return NULL;
    }

    if(style->target == NULL) {
        return NULL;
    }

    if(GT_KEYPAD_MODE_SPELL ==  style->info.mode) {
        _gt_keypad_spell_event_cb(keypad);
        return NULL;
    }
    return style;
}

static void _gt_keypad_input_event_continue_cb(gt_obj_st * keypad) {
    _gt_keypad_st * style = _get_checked_style(keypad);
    if (NULL == style) {
        return;
    }
    if( strcmp(style->kv_selected, GT_SYMBOL_EN) == 0 ||
        strcmp(style->kv_selected, GT_SYMBOL_SPELL) == 0)
    {
        gt_input_append_value(style->target, _GT_KEYPAD_TAG_SPACE);
    }
    else if(strcmp(style->kv_selected, GT_SYMBOL_BACKSPACE) == 0 ){
        gt_input_del_value(style->target);
    }
    else if( strcmp(style->kv_selected, GT_SYMBOL_LEFT) == 0 ){
        gt_input_move_left_pos_cursor(style->target);
    }
    else if( strcmp(style->kv_selected, GT_SYMBOL_RIGHT) == 0 ){
        gt_input_move_right_pos_cursor(style->target);
    }
    else if( strcmp(style->kv_selected, GT_SYMBOL_NEW_LINE) == 0 ){
        gt_input_append_value(style->target, _GT_KEYPAD_TAG_NEW_LINE);
    }
}

static void _gt_keypad_input_event_cb(gt_obj_st * keypad) {
#if 01
    _gt_keypad_st * style = _get_checked_style(keypad);
    if (NULL == style) {
        return;
    }
#else
    _gt_keypad_st * style = keypad->style;
    if( style->info.handler_cb != NULL ){
        style->info.handler_cb(keypad);
        return;
    }

    if( style->kv_selected == NULL ){
        return;
    }

    if (_gt_keypad_is_switch_key_mapping_pressed(keypad, false)) {
        return;
    }

    if(style->target == NULL) {
        return;
    }

    if(GT_KEYPAD_MODE_SPELL ==  style->info.mode) {
        _gt_keypad_spell_event_cb(keypad);
        return ;
    }
#endif

    if( strcmp(style->kv_selected, GT_SYMBOL_EN) == 0 ||
        strcmp(style->kv_selected, GT_SYMBOL_SPELL) == 0)
    {
        gt_input_append_value(style->target, _GT_KEYPAD_TAG_SPACE);
    }
    else if(strcmp(style->kv_selected, GT_SYMBOL_BACKSPACE) == 0 ){
        gt_input_del_value(style->target);
    }
    else if( strcmp(style->kv_selected, GT_SYMBOL_LEFT) == 0 ){
        gt_input_move_left_pos_cursor(style->target);
    }
    else if( strcmp(style->kv_selected, GT_SYMBOL_RIGHT) == 0 ){
        gt_input_move_right_pos_cursor(style->target);
    }
    else if( strcmp(style->kv_selected, GT_SYMBOL_NEW_LINE) == 0 ){
        gt_input_append_value(style->target, _GT_KEYPAD_TAG_NEW_LINE);
    }
    else{
        if (!_gt_keypad_check_key_is_ctl(style->kv_selected)) {
            uint8_t * tmp_str = _gt_keypad_get_spell_text(style->kv_selected , style->info.mode);
            if(tmp_str)
            {
                gt_input_append_value(style->target, (char * )tmp_str);
                return ;
            }
            gt_input_append_value(style->target, style->kv_selected);
        }
    }
}

static inline void _gt_keypad_init_widget(gt_obj_st * keypad)
{
    _gt_keypad_st * style = keypad->style;
    int idx = 0;
    uint8_t* tmp_str = NULL;

    gt_keypad_map_st * map_keypad = style->info.map;

    gt_font_st font = {
        .style_cn = style->font_family_cn,
        .style_en = style->font_family_en,
        .style_numb = style->font_family_numb,
        .size     = style->font_size,
        .gray     = style->font_gray,
        .res      = NULL,
    };
    font.thick_en = style->thick_en == 0 ? style->font_size + 6: style->thick_en;
    font.thick_cn = style->thick_cn == 0 ? style->font_size + 6: style->thick_cn;
    font.encoding = GT_ENCODING_UTF8;

    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.reg.is_fill  = 1;
    rect_attr.bg_opa       = keypad->opa;
    rect_attr.border_width = 0;
    rect_attr.radius       = 4;
    rect_attr.bg_color     = style->color_board;

    draw_bg(keypad->draw_ctx, &rect_attr, &keypad->area);

    gt_attr_font_st font_attr = {
        .font       = &font,
        .space_x    = style->space_x,
        .space_y    = style->space_y,
        .align      = style->font_align,
        .font_color = style->font_color,
        .opa        = keypad->opa,
    };

    gt_area_st area_key;

    uint8_t h_key = ( (keypad->area.h - SPACE_Y) / (style->info.cnt_lines+1)) - SPACE_Y;
    uint8_t per_one_width = (keypad->area.w / style->info.per_line[0]);
    uint8_t line = 0;
    uint8_t r, g, b;

    area_key.h = h_key;
    area_key.y = keypad->area.y + SPACE_Y;
    area_key.x = keypad->area.x + SPACE_X;
    while ( map_keypad[idx].kv != NULL ) {

        rect_attr.bg_color = style->color_key;
        font_attr.font->size = style->font_size;
        if( _gt_keypad_check_key_is_ctl(map_keypad[idx].kv) ){
            rect_attr.bg_color = style->color_ctrl;
        }

        if(_gt_keypad_check_key_is_icon(map_keypad[idx].kv)){
            font_attr.font->size = _GT_KEYPADS_ICON_SIZE;
        }

        if ( _gt_keypad_get_kv_selected(keypad) && strcmp( map_keypad[idx].kv, _gt_keypad_get_kv_selected(keypad)) == 0  ){
            r = GT_COLOR_GET_R(rect_attr.bg_color) >> 1;
            g = GT_COLOR_GET_G(rect_attr.bg_color) >> 1;
            b = GT_COLOR_GET_B(rect_attr.bg_color) >> 1;
            GT_COLOR_SET_RGB(rect_attr.bg_color, r, g, b);
        }

        area_key.w = map_keypad[idx].w * per_one_width - SPACE_X;

        if( (map_keypad[idx+1].kv && strcmp(map_keypad[idx+1].kv, _GT_KEYPAD_TAG_NEW_LINE) == 0) || map_keypad[idx+1].kv == NULL ){
            if( (area_key.x + area_key.w) > (keypad->area.x + keypad->area.w - SPACE_X) ){
                area_key.w = (keypad->area.x + keypad->area.w - SPACE_X) - area_key.x;
            }
            if( (area_key.x + area_key.w) < (keypad->area.x + keypad->area.w - SPACE_X) ){
                area_key.w += ((keypad->area.x + keypad->area.w - SPACE_X) - (area_key.x + area_key.w));
            }
        }

        draw_bg(keypad->draw_ctx, &rect_attr, &area_key);

        font.utf8 = (char * )map_keypad[idx].kv;
        font.len = strlen(map_keypad[idx].kv);

        tmp_str = _gt_keypad_get_spell_text(map_keypad[idx].kv , style->info.mode);

        if(tmp_str)
        {
            font.utf8 = (char * )tmp_str;
            font.len = strlen((const char * )tmp_str);
        }

        draw_text(keypad->draw_ctx, &font_attr, &area_key);

        idx++;

        area_key.x += area_key.w + SPACE_X;

        if(map_keypad[idx].kv && strcmp(map_keypad[idx].kv, _GT_KEYPAD_TAG_NEW_LINE) == 0 ){
            area_key.x = keypad->area.x + SPACE_X;
            area_key.y += h_key + SPACE_Y;
            line++;
            idx++;
            per_one_width = (keypad->area.w / style->info.per_line[line]);
        }
    }

}

/**
 * @brief obj init keypad widget call back
 *
 * @param obj
 */
static void _init_cb(gt_obj_st * obj) {
    GT_LOGV(GT_LOG_TAG_GUI, "start init_cb");

    _gt_keypad_init_widget(obj);
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

    _gt_keypad_st ** style_p = (_gt_keypad_st ** )&obj->style;
    if (NULL == *style_p) {
        return ;
    }

    gt_mem_free(*style_p);
    *style_p = NULL;

#if ((GT_CFG_ENABLE_ZK_SPELL == 1)&&(GT_CFG_ENABLE_ZK_FONT == 1))

    if(_gt_spell_info && _gt_spell_info->py_cache)
    {
        gt_mem_free(_gt_spell_info->py_cache);
        _gt_spell_info->py_cache = NULL;
    }

    if(_gt_spell_info && _gt_spell_info->content)
    {
        gt_mem_free(_gt_spell_info->content);
        _gt_spell_info->content = NULL;
    }

    if(_gt_spell_info)
    {
        gt_mem_free(_gt_spell_info);
        _gt_spell_info = NULL;
    }
#endif

}


/**
 * @brief obj event handler call back
 *
 * @param obj
 * @param e event
 */
static void _event_cb(struct gt_obj_s * obj, gt_event_st * e) {
    gt_event_type_et code = gt_event_get_code(e);

    switch (code) {
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
        GT_LOGV(GT_LOG_TAG_GUI, "PRESSING");
        if (gt_obj_check_scr(obj)) {
            _gt_keypad_set_kv_selected(obj, _gt_keypad_get_click_kv(obj));
            _gt_keypad_input_event_continue_cb(obj);
        }
        break;
    case GT_EVENT_TYPE_INPUT_PRESSED:
        GT_LOGV(GT_LOG_TAG_GUI, "PRESSED");
        if (gt_obj_check_scr(obj)) {
            _gt_keypad_set_kv_selected(obj, _gt_keypad_get_click_kv(obj));
            _gt_keypad_input_event_cb(obj);
        }
        break;
    case GT_EVENT_TYPE_DRAW_REDRAW: {
        gt_event_send(obj, GT_EVENT_TYPE_DRAW_START, NULL);
        break;
    }
    case GT_EVENT_TYPE_INPUT_SCROLL:
        GT_LOGV(GT_LOG_TAG_GUI, "scroll");
        break;

    case GT_EVENT_TYPE_INPUT_PROCESS_LOST:
    case GT_EVENT_TYPE_INPUT_RELEASED: /* click event finish */
        GT_LOGV(GT_LOG_TAG_GUI, "processed");
        if(gt_obj_check_scr(obj)) {
            _gt_keypad_is_switch_key_mapping_pressed(obj, true);
            _gt_keypad_set_kv_selected(obj, NULL);
        }
        break;

    default:
        break;
    }
}

static void _gt_keypad_init_style(gt_obj_st * keypad)
{
    _gt_keypad_st * style = (_gt_keypad_st * )keypad->style;

    gt_memset(style, 0, sizeof(_gt_keypad_st));
    _gt_keypad_update_map(keypad, _map_keypad_mode_lower);

    keypad->focus_dis = GT_DISABLED;
    keypad->area.w = GT_SCREEN_WIDTH > KEYPAD_W ? KEYPAD_W : GT_SCREEN_WIDTH;
    keypad->area.h = GT_SCREEN_HEIGHT > KEYPAD_H ? KEYPAD_H : GT_SCREEN_HEIGHT;

    style->font_color      = gt_color_hex(0);
    style->font_family_cn  = GT_CFG_DEFAULT_FONT_FAMILY_CN;
    style->font_family_en  = GT_CFG_DEFAULT_FONT_FAMILY_EN;
    style->font_family_numb  = GT_CFG_DEFAULT_FONT_FAMILY_NUMB;
    style->font_size       = GT_CFG_DEFAULT_FONT_SIZE;

    style->color_board  = gt_color_hex(0x242424);
    style->color_key    = gt_color_hex(0x646464);
    style->color_ctrl   = gt_color_hex(0x3E3E3E);
    style->font_gray    = 1;
    style->font_align   = GT_ALIGN_CENTER_MID;
    style->thick_en     = 0;
    style->thick_cn     = 0;
    style->space_x      = 0;
    style->space_y      = 0;

    _gt_spell_ascii_numb = 0;
    _gt_spell_chinese_numb = 0;
    gt_memset_0(_gt_spell_ascii , _GT_SPELL_ASCII_MAX_NUMB);

#if ((GT_CFG_ENABLE_ZK_SPELL == 1)&&(GT_CFG_ENABLE_ZK_FONT == 1))
    // 初始化拼音输入法
    _gt_spell_info = (py_info_st*)gt_mem_malloc(sizeof(py_info_st));
    if(NULL == _gt_spell_info)
    {
        GT_LOGE(GT_LOG_TAG_GUI , "spell info malloc err!");
        return ;
    }
    _gt_spell_info->content = (py_content_st*)gt_mem_malloc(sizeof(py_content_st));
    if(NULL == _gt_spell_info->content)
    {
        GT_LOGE(GT_LOG_TAG_GUI , "spell info content malloc err!");
        return ;
    }
    _gt_spell_info->py_cache = (unsigned char*)gt_mem_malloc(1218);
    if(NULL == _gt_spell_info->py_cache)
    {
        GT_LOGE(GT_LOG_TAG_GUI , "spell info py_cache malloc err!");
        return ;
    }
    gt_pinyin_init(_gt_spell_info);
#endif

}

static void _gt_keypad_update_map(gt_obj_st * keypad, gt_keypad_map_st const * map)
{
    _gt_keypad_st * style = keypad->style;
    if(style->info.map == map)
    {
        return ;
    }
    style->info.map = (struct _gt_keypad_map_s * )map;
    _gt_keypad_change_mode(keypad, map);

    uint16_t idx = 0;
    uint8_t cnt_lines = 0, cnt_keys = 0, per_all = 0;
    while( map[idx].kv != NULL ){
        cnt_keys++;
        per_all += map[idx].w;
        if( strcmp(map[idx].kv, _GT_KEYPAD_TAG_NEW_LINE) == 0 ){
            style->info.per_line[cnt_lines] = per_all;
            cnt_lines++;
            per_all = 0;
        }
        idx++;
    }
    style->info.per_line[cnt_lines] = per_all;
    style->info.cnt_lines = cnt_lines;

    _gt_spell_ascii_numb = 0;
    _gt_spell_chinese_numb = 0;

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
void gt_keypad_set_target(gt_obj_st * keypad, gt_obj_st * ta)
{
    _gt_keypad_st * style = keypad->style;
    style->target = ta;
    gt_event_send(keypad, GT_EVENT_TYPE_DRAW_START, NULL);
}


/**
 * @brief set keypad of map
 *
 * @param keypad
 * @param map
 */
void gt_keypad_set_map(gt_obj_st * keypad,  gt_keypad_map_st * map)
{
    _gt_keypad_st * style = keypad->style;
    style->info.map = map;
    _gt_keypad_update_map(keypad, style->info.map);
}

void gt_keypad_set_handler_cb(gt_obj_st * keypad, keypad_handler_cb_t callback)
{
    _gt_keypad_st * style = keypad->style;
    style->info.handler_cb = callback;
}


void gt_keypad_set_color_board(gt_obj_st * keypad, gt_color_t color)
{
    _gt_keypad_st * style = keypad->style;
    style->color_board = color;
}
void gt_keypad_set_color_key(gt_obj_st * keypad, gt_color_t color)
{
    _gt_keypad_st * style = keypad->style;
    style->color_key = color;
}
void gt_keypad_set_color_ctrl_key(gt_obj_st * keypad, gt_color_t color)
{
    _gt_keypad_st * style = keypad->style;
    style->color_ctrl = color;
}
void gt_keypad_set_font_color(gt_obj_st * keypad, gt_color_t color)
{
    _gt_keypad_st * style = keypad->style;
    style->font_color = color;
    gt_event_send(keypad, GT_EVENT_TYPE_DRAW_START, NULL);
}

void gt_keypad_set_font_family_cn(gt_obj_st * keypad, gt_family_t family)
{
    _gt_keypad_st * style = keypad->style;
    style->font_family_cn = family;
}
void gt_keypad_set_font_family_en(gt_obj_st * keypad, gt_family_t family)
{
    _gt_keypad_st * style = keypad->style;
    style->font_family_en = family;
}

void gt_keypad_set_font_family_numb(gt_obj_st * keypad, gt_family_t family)
{
    _gt_keypad_st * style = keypad->style;
    style->font_family_numb = family;
}

void gt_keypad_set_font_size(gt_obj_st * keypad, uint8_t size)
{
    _gt_keypad_st * style = keypad->style;
    style->font_size = size;
}
void gt_keypad_set_font_gray(gt_obj_st * keypad, uint8_t gray)
{
    _gt_keypad_st * style = keypad->style;
    style->font_gray = gray;
}
void gt_keypad_set_font_align(gt_obj_st * keypad, uint8_t align)
{
    _gt_keypad_st * style = keypad->style;
    style->font_align = align;
}
void gt_keypad_set_font_thick_en(gt_obj_st * keypad, uint8_t thick)
{
    _gt_keypad_st * style = (_gt_keypad_st * )keypad->style;
    style->thick_en = thick;
}
void gt_keypad_set_font_thick_cn(gt_obj_st * keypad, uint8_t thick)
{
    _gt_keypad_st * style = (_gt_keypad_st * )keypad->style;
    style->thick_cn = thick;
}
void gt_keypad_set_space(gt_obj_st * keypad, uint8_t space_x, uint8_t space_y)
{
    _gt_keypad_st * style = (_gt_keypad_st * )keypad->style;
    style->space_x = space_x;
    style->space_y = space_y;
}

/* end ------------------------------------------------------------------*/
