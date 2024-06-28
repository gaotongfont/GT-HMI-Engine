/**
 * @file gt_symbol.h
 * @author Feyoung
 * @brief Common symbols list GB code and unicode , range:
 *      0xE000 - 0xE03F
 *          |
 *      0xF800 - 0xF83F
 * @version 0.1
 * @date 2022-08-25 17:55:56
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SYMBOL_H_
#define _GT_SYMBOL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"
#include "stdbool.h"


/* define ---------------------------------------------------------------*/
#ifndef _GT_SYMBOL_PRINTF_VALID_CODE
    /**
     * @brief 0[default]: Disabled; 1: Print all valid symbol can be use utf8 code
     */
    #define _GT_SYMBOL_PRINTF_VALID_CODE    0
#endif

#ifndef GT_CFG_USE_SYMBOL_ENABLE
    #define GT_CFG_USE_SYMBOL_ENABLE        1
#endif

#define GT_CFG_USE_SYMBOL_16x16	     1
#define GT_CFG_USE_SYMBOL_24x24	     1


#if	GT_CFG_USE_SYMBOL_ENABLE	 == 1
#if !defined GT_SYMBOL_AUDIO
#define GT_SYMBOL_AUDIO           "\xEE\x80\x80" /*57344, 0xE000*/
#endif

#if !defined GT_SYMBOL_VIDEO
#define GT_SYMBOL_VIDEO           "\xEE\x80\x81" /*57345, 0xE001*/
#endif

#if !defined GT_SYMBOL_LIST
#define GT_SYMBOL_LIST            "\xEE\x80\x82" /*57346, 0xE002*/
#endif

#if !defined GT_SYMBOL_OK
#define GT_SYMBOL_OK              "\xEE\x80\x83" /*57347, 0xE003*/
#endif

#if !defined GT_SYMBOL_CLOSE
#define GT_SYMBOL_CLOSE           "\xEE\x80\x84" /*57348, 0xE004*/
#endif

#if !defined GT_SYMBOL_POWER
#define GT_SYMBOL_POWER           "\xEE\x80\x85" /*57349, 0xE005*/
#endif

#if !defined GT_SYMBOL_SETTINGS
#define GT_SYMBOL_SETTINGS        "\xEE\x80\x86" /*57350, 0xE006*/
#endif

#if !defined GT_SYMBOL_HOME
#define GT_SYMBOL_HOME            "\xEE\x80\x87" /*57351, 0xE007*/
#endif

#if !defined GT_SYMBOL_DOWNLOAD
#define GT_SYMBOL_DOWNLOAD        "\xEE\x80\x88" /*57352, 0xE008*/
#endif

#if !defined GT_SYMBOL_DRIVE
#define GT_SYMBOL_DRIVE           "\xEE\x80\x89" /*57353, 0xE009*/
#endif

#if !defined GT_SYMBOL_REFRESH
#define GT_SYMBOL_REFRESH         "\xEE\x80\x8A" /*57354, 0xE00A*/
#endif

#if !defined GT_SYMBOL_MUTE
#define GT_SYMBOL_MUTE            "\xEE\x80\x8B" /*57355, 0xE00B*/
#endif

#if !defined GT_SYMBOL_VOLUME_MID
#define GT_SYMBOL_VOLUME_MID      "\xEE\x80\x8C" /*57356, 0xE00C*/
#endif

#if !defined GT_SYMBOL_VOLUME_MAX
#define GT_SYMBOL_VOLUME_MAX      "\xEE\x80\x8D" /*57357, 0xE00D*/
#endif

#if !defined GT_SYMBOL_IMAGE
#define GT_SYMBOL_IMAGE           "\xEE\x80\x8E" /*57358, 0xE00E*/
#endif

#if !defined GT_SYMBOL_TINT
#define GT_SYMBOL_TINT            "\xEE\x80\x8F" /*57359, 0xE00F*/
#endif

#if !defined GT_SYMBOL_PREV
#define GT_SYMBOL_PREV            "\xEE\x80\x90" /*57360, 0xE010*/
#endif

#if !defined GT_SYMBOL_PLAY
#define GT_SYMBOL_PLAY            "\xEE\x80\x91" /*57361, 0xE011*/
#endif

#if !defined GT_SYMBOL_PAUSE
#define GT_SYMBOL_PAUSE           "\xEE\x80\x92" /*57362, 0xE012*/
#endif

#if !defined GT_SYMBOL_STOP
#define GT_SYMBOL_STOP            "\xEE\x80\x93" /*57363, 0xE013*/
#endif

#if !defined GT_SYMBOL_NEXT
#define GT_SYMBOL_NEXT            "\xEE\x80\x94" /*57364, 0xE014*/
#endif

#if !defined GT_SYMBOL_EJECT
#define GT_SYMBOL_EJECT           "\xEE\x80\x95" /*57365, 0xE015*/
#endif

#if !defined GT_SYMBOL_LEFT
#define GT_SYMBOL_LEFT            "\xEE\x80\x96" /*57366, 0xE016*/
#endif

#if !defined GT_SYMBOL_RIGHT
#define GT_SYMBOL_RIGHT           "\xEE\x80\x97" /*57367, 0xE017*/
#endif

#if !defined GT_SYMBOL_PLUS
#define GT_SYMBOL_PLUS            "\xEE\x80\x98" /*57368, 0xE018*/
#endif

#if !defined GT_SYMBOL_MINUS
#define GT_SYMBOL_MINUS           "\xEE\x80\x99" /*57369, 0xE019*/
#endif

#if !defined GT_SYMBOL_EYE_OPEN
#define GT_SYMBOL_EYE_OPEN        "\xEE\x80\x9A" /*57370, 0xE01A*/
#endif

#if !defined GT_SYMBOL_EYE_CLOSE
#define GT_SYMBOL_EYE_CLOSE       "\xEE\x80\x9B" /*57371, 0xE01B*/
#endif

#if !defined GT_SYMBOL_WARNING
#define GT_SYMBOL_WARNING         "\xEE\x80\x9C" /*57372, 0xE01C*/
#endif

#if !defined GT_SYMBOL_SHUFFLE
#define GT_SYMBOL_SHUFFLE         "\xEE\x80\x9D" /*57373, 0xE01D*/
#endif

#if !defined GT_SYMBOL_UP
#define GT_SYMBOL_UP              "\xEE\x80\x9F" /*57375, 0xE01F*/
#endif

#if !defined GT_SYMBOL_DOWN
#define GT_SYMBOL_DOWN            "\xEE\x80\xA0" /*57376, 0xE020*/
#endif

#if !defined GT_SYMBOL_LOOP
#define GT_SYMBOL_LOOP            "\xEE\x80\xA1" /*57377, 0xE021*/
#endif

#if !defined GT_SYMBOL_DIRECTORY
#define GT_SYMBOL_DIRECTORY       "\xEE\x80\xA2" /*57378, 0xE022*/
#endif

#if !defined GT_SYMBOL_UPLOAD
#define GT_SYMBOL_UPLOAD          "\xEE\x80\xA3" /*57379, 0xE023*/
#endif

#if !defined GT_SYMBOL_CALL
#define GT_SYMBOL_CALL            "\xEE\x80\xA4" /*57380, 0xE024*/
#endif

#if !defined GT_SYMBOL_CUT
#define GT_SYMBOL_CUT             "\xEE\x80\xA5" /*57381, 0xE025*/
#endif

#if !defined GT_SYMBOL_COPY
#define GT_SYMBOL_COPY            "\xEE\x80\xA6" /*57382, 0xE026*/
#endif

#if !defined GT_SYMBOL_SAVE
#define GT_SYMBOL_SAVE            "\xEE\x80\xA7" /*57383, 0xE027*/
#endif

#if !defined GT_SYMBOL_BARS
#define GT_SYMBOL_BARS            "\xEE\x80\xA8" /*57384, 0xE028*/
#endif

#if !defined GT_SYMBOL_ENVELOPE
#define GT_SYMBOL_ENVELOPE        "\xEE\x80\xA9" /*57385, 0xE029*/
#endif

#if !defined GT_SYMBOL_CHARGE
#define GT_SYMBOL_CHARGE          "\xEE\x80\xAA" /*57386, 0xE02A*/
#endif

#if !defined GT_SYMBOL_PASTE
#define GT_SYMBOL_PASTE           "\xEE\x80\xAB" /*57387, 0xE02B*/
#endif

#if !defined GT_SYMBOL_BELL
#define GT_SYMBOL_BELL            "\xEE\x80\xAC" /*57388, 0xE02C*/
#endif

#if !defined GT_SYMBOL_KEYBOARD
#define GT_SYMBOL_KEYBOARD        "\xEE\x80\xAD" /*57389, 0xE02D*/
#endif

#if !defined GT_SYMBOL_GPS
#define GT_SYMBOL_GPS             "\xEE\x80\xAE" /*57390, 0xE02E*/
#endif

#if !defined GT_SYMBOL_FILE
#define GT_SYMBOL_FILE            "\xEE\x80\xAF" /*57391, 0xE02F*/
#endif

#if !defined GT_SYMBOL_WIFI
#define GT_SYMBOL_WIFI            "\xEE\x80\xB0" /*57392, 0xE030*/
#endif

#if !defined GT_SYMBOL_BATTERY_FULL
#define GT_SYMBOL_BATTERY_FULL    "\xEE\x80\xB1" /*57393, 0xE031*/
#endif

#if !defined GT_SYMBOL_BATTERY_3
#define GT_SYMBOL_BATTERY_3       "\xEE\x80\xB2" /*57394, 0xE032*/
#endif

#if !defined GT_SYMBOL_BATTERY_2
#define GT_SYMBOL_BATTERY_2       "\xEE\x80\xB3" /*57395, 0xE033*/
#endif

#if !defined GT_SYMBOL_BATTERY_1
#define GT_SYMBOL_BATTERY_1       "\xEE\x80\xB4" /*57396, 0xE034*/
#endif

#if !defined GT_SYMBOL_BATTERY_EMPTY
#define GT_SYMBOL_BATTERY_EMPTY   "\xEE\x80\xB5" /*57397, 0xE035*/
#endif

#if !defined GT_SYMBOL_USB
#define GT_SYMBOL_USB             "\xEE\x80\xB6" /*57398, 0xE036*/
#endif

#if !defined GT_SYMBOL_BLUETOOTH
#define GT_SYMBOL_BLUETOOTH       "\xEE\x80\xB7" /*57399, 0xE037*/
#endif

#if !defined GT_SYMBOL_TRASH
#define GT_SYMBOL_TRASH           "\xEE\x80\xB8" /*57400, 0xE038*/
#endif

#if !defined GT_SYMBOL_EDIT
#define GT_SYMBOL_EDIT            "\xEE\x80\xB9" /*57401, 0xE039*/
#endif

#if !defined GT_SYMBOL_BACKSPACE
#define GT_SYMBOL_BACKSPACE       "\xEE\x80\xBA" /*57402, 0xE03A*/
#endif

#if !defined GT_SYMBOL_SD_CARD
#define GT_SYMBOL_SD_CARD         "\xEE\x80\xBB" /*57403, 0xE03B*/
#endif

#if !defined GT_SYMBOL_NEW_LINE
#define GT_SYMBOL_NEW_LINE        "\xEE\x80\xBC" /*57404, 0xE03C*/
#endif

#if !defined GT_SYMBOL_EN
#define GT_SYMBOL_EN               "\xEE\x80\xBD" /*57405, 0xE03D*/
#endif

#if !defined GT_SYMBOL_CH
#define GT_SYMBOL_CH                "\xEE\x80\xBE" /*57406, 0xE03E*/
#endif

#if !defined GT_SYMBOL_SPACE
#define GT_SYMBOL_SPACE             "\xEE\x80\xBF" /*57407, 0xE03F*/
#endif

#if !defined GT_SYMBOL_WELL_NUMBER
#define GT_SYMBOL_WELL_NUMBER       "\xEE\x84\x80" /*57600, 0xE100*/
#endif

#if !defined GT_SYMBOL_LOWER_CASE
#define GT_SYMBOL_LOWER_CASE         "\xEE\x84\x81" /*57601, 0xE101*/
#endif

#if !defined GT_SYMBOL_UPPER_CASE
#define GT_SYMBOL_UPPER_CASE         "\xEE\x84\x82" /*57602, 0xE102*/
#endif

#if !defined GT_SYMBOL_VOICE_PLAY
#define GT_SYMBOL_VOICE_PLAY          (3)
#endif

#if !defined GT_SYMBOL_VOICE_PLAY_LEFT_1
#define GT_SYMBOL_VOICE_PLAY_LEFT_1   "\xEE\x84\x83" /*57603, 0xE103*/
#endif

#if !defined GT_SYMBOL_VOICE_PLAY_LEFT_2
#define GT_SYMBOL_VOICE_PLAY_LEFT_2   "\xEE\x84\x84" /*57604, 0xE104*/
#endif

#if !defined GT_SYMBOL_VOICE_PLAY_LEFT_3
#define GT_SYMBOL_VOICE_PLAY_LEFT_3   "\xEE\x84\x85" /*57605, 0xE105*/
#endif

#if !defined GT_SYMBOL_VOICE_PLAY_RIGHT_1
#define GT_SYMBOL_VOICE_PLAY_RIGHT_1  "\xEE\x84\x86" /*57606, 0xE106*/
#endif

#if !defined GT_SYMBOL_VOICE_PLAY_RIGHT_2
#define GT_SYMBOL_VOICE_PLAY_RIGHT_2  "\xEE\x84\x87" /*57607, 0xE107*/
#endif

#if !defined GT_SYMBOL_VOICE_PLAY_RIGHT_3
#define GT_SYMBOL_VOICE_PLAY_RIGHT_3  "\xEE\x84\x88" /*57608, 0xE108*/
#endif

#endif // GT_CFG_USE_SYMBOL_ENABLE
/* typedef --------------------------------------------------------------*/
typedef struct _gt_symbol_s{
    char * utf8;
    const uint8_t * mask_buf;
}_gt_symbol_st;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
bool gt_symbol_is_valid_range(uint32_t uni_or_gbk);
const uint8_t * gt_symbol_get_mask_buf(char * utf8 , uint8_t size);

#if _GT_SYMBOL_PRINTF_VALID_CODE
/**
 * @brief Print all valid symbol can be use utf8 code, unicode and gbk(gb18030) common valid range:
 *      0xE000 - 0xE03F
 *          |
 *      0xF800 - 0xF83F
 * @returns such as: "\xEE\x80\x80" /*57344, 0xE000*\/
 */
void _gt_symbol_get_all_valid_utf8_code(void);
#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SYMBOL_H_
