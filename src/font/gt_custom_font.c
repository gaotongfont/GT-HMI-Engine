/**
 * @file gt_custom_font.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2025-10-14 19:50:31
 * @copyright Copyright (c) 2014-2025, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_custom_font.h"

#if GT_CUSTOM_FONT_ENABLE

#include "../core/gt_mem.h"
#include "../others/gt_log.h"


/* private define -------------------------------------------------------*/
#define _CUSTOM_TAG     "GT_CUSTOM_FONT"
#define _GT_CUSTOM_LIST_DEFAULT_COUNT    (4)


/* private typedef ------------------------------------------------------*/
typedef struct {
    gt_custom_font_st* list;
    uint16_t total;
    uint16_t count;
}_gt_custom_font_list_st;

typedef struct{
    gt_custom_emoji_st* list;
    uint16_t total;
    uint16_t count;
}_gt_custom_emoji_list_st;

typedef struct {
    _gt_custom_font_list_st font_list;
    _gt_custom_emoji_list_st emoji_list;
}_gt_custom_list_st;

/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/

#if defined(GT_FAMILY_Emoji_16x16)

GT_ATTRIBUTE_RAM_TEXT
static int _emoji_def_get_dot_width(struct gt_custom_font_s* font, uint32_t font_code);
GT_ATTRIBUTE_RAM_TEXT
static int _emoji_def_get_width(struct gt_custom_font_s* font, uint32_t font_code);
GT_ATTRIBUTE_RAM_TEXT
static int _emoji_def_get_height(struct gt_custom_font_s* font, uint32_t font_code);
GT_ATTRIBUTE_RAM_TEXT
static int _emoji_def_read_img_font(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer, unsigned char* opa, unsigned int line);
GT_ATTRIBUTE_RAM_TEXT
static bool _emoji_def_is_emoji (struct gt_custom_emoji_s * em, uint32_t font_code);

GT_ATTRIBUTE_RAM_DATA
static gt_custom_font_st _emoji_font_def = {

    .ffs.option = GT_CUSTOM_FONT_OPTION(GT_FAMILY_Emoji_16x16),
    .ffs.func.read_img_font = _emoji_def_read_img_font,
    .ffs.type = GT_FONT_TYPE_FLAG_EMOJI,

    .get_dot_width = _emoji_def_get_dot_width,
    .get_width = _emoji_def_get_width,
    .get_height = _emoji_def_get_height,
};
GT_ATTRIBUTE_RAM_DATA
static gt_custom_emoji_st _emoji_16x16_def = {
    .font = &_emoji_font_def,
    .is_emoji = _emoji_def_is_emoji,
};

GT_ATTRIBUTE_RAM_DATA
static const uint32_t _gt_emoji_unicode_list[] = {
// 表情	Unicode 编码
    0x200D,     //😵‍💫
    0x261D,     //
    0x270A,     //
    0x270B,     //
    0x270C,     //
    0x2764,     //
    0x2B50,     //
    0x1F308,    //🌈
    0x1F31A,    //🌚
    0x1F31C,    //🌜
    0x1F31D,    //🌝
    0x1F31E,    //🌞
    0x1F31F,    //🌟
    0x1F332,    //🌲
    0x1F333,    //🌳
    0x1F334,    //🌴
    0x1F335,    //
    0x1F336,    //🌶
    0x1F339,    //
    0x1F33A,    //
    0x1F340,    //🍀
    0x1F341,    //🍁
    0x1F343,    //🍃
    0x1F345,    //🍅
    0x1F346,    //🍆
    0x1F347,    //🍇
    0x1F348,    //🍈
    0x1F349,    //🍉
    0x1F34A,    //🍊
    0x1F34B,    //🍋
    0x1F34C,    //🍌
    0x1F34D,    //🍍
    0x1F34E,    //🍎
    0x1F350,    //🍐
    0x1F351,    //🍑
    0x1F352,    //🍒
    0x1F353,    //🍓
    0x1F381,    //🎁
    0x1F383,    //🎃
    0x1F384,    //🎄
    0x1F385,    //🎅
    0x1F388,    //🎈
    0x1F389,    //🎉
    0x1F38A,    //🎊
    0x1F42D,    //🐭
    0x1F430,    //🐰
    0x1F436,    //🐶
    0x1F439,    //🐹
    0x1F43B,    //🐻
    0x1F43C,    //🐼
    0x1F43E,    //🐾
    0x1F446,    //👆
    0x1F447,    //👇
    0x1F448,    //👈
    0x1F449,    //👉
    0x1F44B,    //👋
    0x1F44C,    //👌
    0x1F44D,    //👍
    0x1F44F,    //👏
    0x1F48B,    //💋
    0x1F494,    //💔
    0x1F498,    //💘
    0x1F49D,    //💝
    0x1F4A9,    //💩
    0x1F4AA,    //💪
    0x1F4E8,    //📨
    0x1F4E9,    //📩
    0x1F600,    //😀
    0x1F602,    //😂
    0x1F605,    //😅
    0x1F607,    //😇
    0x1F60A,    //😊
    0x1F60D,    //😍
    0x1F60E,    //😎
    0x1F613,    //😓
    0x1F616,    //😖
    0x1F619,    //😙
    0x1F61C,    //😜
    0x1F620,    //😠
    0x1F621,    //😡
    0x1F622,    //😢
    0x1F623,    //😣
    0x1F624,    //😤
    0x1F625,    //😥
    0x1F62D,    //😭
    0x1F62E,    //😮
    0x1F62F,    //😯
    0x1F631,    //😱
    0x1F632,    //😲
    0x1F634,    //😴
    0x1F635,    //😵
    0x1F637,    //😷
    0x1F638,    //😸
    0x1F639,    //😹
    0x1F63A,    //😺
    0x1F63B,    //😻
    0x1F642,    //🙂
    0x1F64C,    //🙌
    0x1F64F,    //🙏
    0x1F910,    //🤐
    0x1F914,    //🤔
    0x1F916,    //🤖
    0x1F917,    //🤗
    0x1F92B,    //🤫
    0x1F92D,    //🤭
    0x1F951,    //🥑
    0x1F954,    //🥔
    0x1F955,    //🥕
    0x1F95D,    //🥝
    0x1F970,    //🥰
    0x1F971,    //🥱
    0x1F973,    //🥳
    0x1F979,    //🥹
    0x1F97A,    //🥺
    0x1F98A,    //🦊
    0x1F9E7,    //🧧
    0x1F9E8,    //🧨
    0x1FAF0,    //
    0x1FAF5,    //
};
#endif


GT_ATTRIBUTE_RAM_DATA
static _gt_custom_list_st _custom_list = {0};
/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
#if defined(GT_FAMILY_Emoji_16x16)
GT_ATTRIBUTE_RAM_TEXT
static int _emoji_def_read_img_font(unsigned long fontcode, unsigned int font_option, unsigned char* pbuffer, unsigned char* opa, unsigned int line)
{
    (void)font_option;
    return GT_Emoji(fontcode, GT_FAMILY_Emoji_16x16, pbuffer, opa, line);
}
GT_ATTRIBUTE_RAM_TEXT
static int _emoji_def_get_dot_width(struct gt_custom_font_s* font, uint32_t font_code)
{
    return GT_Get_Font_Dot_Width(font_code, GT_FAMILY_Emoji_16x16, 16, 1);
}
GT_ATTRIBUTE_RAM_TEXT
static int _emoji_def_get_width(struct gt_custom_font_s* font, uint32_t font_code)
{
    return GT_Get_Font_Width(font_code, GT_FAMILY_Emoji_16x16, 16 , 1 , NULL);
}
GT_ATTRIBUTE_RAM_TEXT
static int _emoji_def_get_height(struct gt_custom_font_s* font, uint32_t font_code)
{
    return GT_Get_Font_Height(font_code, GT_FAMILY_Emoji_16x16, 16, 1);
}

GT_ATTRIBUTE_RAM_TEXT
static bool _emoji_def_is_emoji (struct gt_custom_emoji_s * em, uint32_t font_code)
{
    uint32_t count = sizeof(_gt_emoji_unicode_list) / sizeof(_gt_emoji_unicode_list[0]);
    if (-1 != gt_binary_search_from_u32(_gt_emoji_unicode_list, count, font_code)) {
        return true;
    }
    return false;
}

#endif

/* global functions / API interface -------------------------------------*/
GT_ATTRIBUTE_RAM_TEXT
void gt_custom_font_init(void)
{
#if defined(GT_FAMILY_Emoji_16x16)
    gt_custom_register_emoji(&_emoji_16x16_def);
#endif
}

GT_ATTRIBUTE_RAM_TEXT
void gt_custom_register_font(gt_custom_font_st* font)
{
    GT_CHECK_BACK(font);

    int len = sizeof(gt_custom_font_st);
    uint16_t i = 0;

    for(i = 0; i < _custom_list.font_list.count; ++i) {
        if(0 == gt_memcmp(&_custom_list.font_list.list[i], font, sizeof(gt_custom_font_st))) {
            return;
        }
    }

    if(_custom_list.font_list.count >= _custom_list.font_list.total) {
        len = (_custom_list.font_list.total + _GT_CUSTOM_LIST_DEFAULT_COUNT) * sizeof(gt_custom_font_st);
        if(_custom_list.font_list.list) {
            _custom_list.font_list.list = gt_mem_realloc(_custom_list.font_list.list, len);
        }
        else {
            _custom_list.font_list.list = gt_mem_malloc(len);
        }

        if(!_custom_list.font_list.list) {
            GT_LOGE(_CUSTOM_TAG, "register font failed, malloc failed!! len: %d", len);
            return;
        }
        gt_memset(_custom_list.font_list.list + _custom_list.font_list.total, 0, sizeof(gt_custom_font_st) * _GT_CUSTOM_LIST_DEFAULT_COUNT);
        _custom_list.font_list.total += _GT_CUSTOM_LIST_DEFAULT_COUNT;
    }

    gt_memcpy(&_custom_list.font_list.list[_custom_list.font_list.count], font, sizeof(gt_custom_font_st));
    ++_custom_list.font_list.count;
}

GT_ATTRIBUTE_RAM_TEXT
void gt_custom_register_emoji(gt_custom_emoji_st* emj)
{
    GT_CHECK_BACK(emj);

    int len = sizeof(gt_custom_emoji_st);
    uint16_t i = 0;

    for(i = 0; i < _custom_list.emoji_list.count; ++i) {
        if(0 == gt_memcmp(&_custom_list.emoji_list.list[i], emj, sizeof(gt_custom_emoji_st))) {
            return;
        }
    }

    if(_custom_list.emoji_list.count >= _custom_list.emoji_list.total) {
        len = (_custom_list.emoji_list.total + _GT_CUSTOM_LIST_DEFAULT_COUNT) * sizeof(gt_custom_emoji_st);
        if(_custom_list.emoji_list.list) {
            _custom_list.emoji_list.list = gt_mem_realloc(_custom_list.emoji_list.list, len);
        }
        else {
            _custom_list.emoji_list.list = gt_mem_malloc(len);
        }

        if(!_custom_list.emoji_list.list) {
            GT_LOGE(_CUSTOM_TAG, "register emoji failed, malloc failed!! len: %d", len);
            return;
        }
        gt_memset(_custom_list.emoji_list.list + _custom_list.emoji_list.total, 0, sizeof(gt_custom_emoji_st) * _GT_CUSTOM_LIST_DEFAULT_COUNT);
        _custom_list.emoji_list.total += _GT_CUSTOM_LIST_DEFAULT_COUNT;
    }

    gt_memcpy(&_custom_list.emoji_list.list[_custom_list.emoji_list.count], emj, sizeof(gt_custom_emoji_st));
    ++_custom_list.emoji_list.count;
}

GT_ATTRIBUTE_RAM_TEXT
bool gt_custom_is_emoji(uint32_t option, uint32_t font_code)
{
    uint16_t i = 0;
    for(i = 0; i < _custom_list.emoji_list.count; ++i) {
        if(!_custom_list.emoji_list.list[i].font) {
            continue;
        }
        if(_custom_list.emoji_list.list[i].font->ffs.option == option) {
            return _custom_list.emoji_list.list[i].is_emoji ? _custom_list.emoji_list.list[i].is_emoji( &_custom_list.emoji_list.list[i] ,font_code) : false;
        }
    }
    return false;
}

GT_ATTRIBUTE_RAM_TEXT
gt_custom_font_st* gt_custom_get_font(uint32_t option)
{
    uint16_t i = 0;
    for(i = 0; i < _custom_list.font_list.count; ++i) {
        if(_custom_list.font_list.list[i].ffs.option == option) {
            return &_custom_list.font_list.list[i];
        }
    }

    for(i = 0; i < _custom_list.emoji_list.count; ++i) {
        if(!_custom_list.emoji_list.list[i].font) {
            continue;
        }
        if(_custom_list.emoji_list.list[i].font->ffs.option == option) {
            return _custom_list.emoji_list.list[i].font;
        }
    }
    return NULL;
}


/* end of file ----------------------------------------------------------*/
#endif //!GT_CUSTOM_FONT_ENABLE

