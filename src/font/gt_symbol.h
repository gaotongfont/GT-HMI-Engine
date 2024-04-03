/**
 * @file gt_symbol.h
 * @author yongg
 * @brief Common symbols list
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
#define GT_CFG_USE_SYMBOL_ENABLE     1
#define GT_CFG_USE_SYMBOL_16x16	     1
#define GT_CFG_USE_SYMBOL_24x24	     1


#if	GT_CFG_USE_SYMBOL_ENABLE	 == 1
#if !defined GT_SYMBOL_AUDIO
#define GT_SYMBOL_AUDIO           "\xEF\x80\x81" /*61441, 0xF001*/
#endif

#if !defined GT_SYMBOL_VIDEO
#define GT_SYMBOL_VIDEO           "\xEF\x80\x88" /*61448, 0xF008*/
#endif

#if !defined GT_SYMBOL_LIST
#define GT_SYMBOL_LIST            "\xEF\x80\x8B" /*61451, 0xF00B*/
#endif

#if !defined GT_SYMBOL_OK
#define GT_SYMBOL_OK              "\xEF\x80\x8C" /*61452, 0xF00C*/
#endif

#if !defined GT_SYMBOL_CLOSE
#define GT_SYMBOL_CLOSE           "\xEF\x80\x8D" /*61453, 0xF00D*/
#endif

#if !defined GT_SYMBOL_POWER
#define GT_SYMBOL_POWER           "\xEF\x80\x91" /*61457, 0xF011*/
#endif

#if !defined GT_SYMBOL_SETTINGS
#define GT_SYMBOL_SETTINGS        "\xEF\x80\x93" /*61459, 0xF013*/
#endif

#if !defined GT_SYMBOL_HOME
#define GT_SYMBOL_HOME            "\xEF\x80\x95" /*61461, 0xF015*/
#endif

#if !defined GT_SYMBOL_DOWNLOAD
#define GT_SYMBOL_DOWNLOAD        "\xEF\x80\x99" /*61465, 0xF019*/
#endif

#if !defined GT_SYMBOL_DRIVE
#define GT_SYMBOL_DRIVE           "\xEF\x80\x9C" /*61468, 0xF01C*/
#endif

#if !defined GT_SYMBOL_REFRESH
#define GT_SYMBOL_REFRESH         "\xEF\x80\xA1" /*61473, 0xF021*/
#endif

#if !defined GT_SYMBOL_MUTE
#define GT_SYMBOL_MUTE            "\xEF\x80\xA6" /*61478, 0xF026*/
#endif

#if !defined GT_SYMBOL_VOLUME_MID
#define GT_SYMBOL_VOLUME_MID      "\xEF\x80\xA7" /*61479, 0xF027*/
#endif

#if !defined GT_SYMBOL_VOLUME_MAX
#define GT_SYMBOL_VOLUME_MAX      "\xEF\x80\xA8" /*61480, 0xF028*/
#endif

#if !defined GT_SYMBOL_IMAGE
#define GT_SYMBOL_IMAGE           "\xEF\x80\xBE" /*61502, 0xF03E*/
#endif

#if !defined GT_SYMBOL_TINT
#define GT_SYMBOL_TINT            "\xEF\x81\x83" /*61507, 0xF043*/
#endif

#if !defined GT_SYMBOL_PREV
#define GT_SYMBOL_PREV            "\xEF\x81\x88" /*61512, 0xF048*/
#endif

#if !defined GT_SYMBOL_PLAY
#define GT_SYMBOL_PLAY            "\xEF\x81\x8B" /*61515, 0xF04B*/
#endif

#if !defined GT_SYMBOL_PAUSE
#define GT_SYMBOL_PAUSE           "\xEF\x81\x8C" /*61516, 0xF04C*/
#endif

#if !defined GT_SYMBOL_STOP
#define GT_SYMBOL_STOP            "\xEF\x81\x8D" /*61517, 0xF04D*/
#endif

#if !defined GT_SYMBOL_NEXT
#define GT_SYMBOL_NEXT            "\xEF\x81\x91" /*61521, 0xF051*/
#endif

#if !defined GT_SYMBOL_EJECT
#define GT_SYMBOL_EJECT           "\xEF\x81\x92" /*61522, 0xF052*/
#endif

#if !defined GT_SYMBOL_LEFT
#define GT_SYMBOL_LEFT            "\xEF\x81\x93" /*61523, 0xF053*/
#endif

#if !defined GT_SYMBOL_RIGHT
#define GT_SYMBOL_RIGHT           "\xEF\x81\x94" /*61524, 0xF054*/
#endif

#if !defined GT_SYMBOL_PLUS
#define GT_SYMBOL_PLUS            "\xEF\x81\xA7" /*61543, 0xF067*/
#endif

#if !defined GT_SYMBOL_MINUS
#define GT_SYMBOL_MINUS           "\xEF\x81\xA8" /*61544, 0xF068*/
#endif

#if !defined GT_SYMBOL_EYE_OPEN
#define GT_SYMBOL_EYE_OPEN        "\xEF\x81\xAE" /*61550, 0xF06E*/
#endif

#if !defined GT_SYMBOL_EYE_CLOSE
#define GT_SYMBOL_EYE_CLOSE       "\xEF\x81\xB0" /*61552, 0xF070*/
#endif

#if !defined GT_SYMBOL_WARNING
#define GT_SYMBOL_WARNING         "\xEF\x81\xB1" /*61553, 0xF071*/
#endif

#if !defined GT_SYMBOL_SHUFFLE
#define GT_SYMBOL_SHUFFLE         "\xEF\x81\xB4" /*61556, 0xF074*/
#endif

#if !defined GT_SYMBOL_UP
#define GT_SYMBOL_UP              "\xEF\x81\xB7" /*61559, 0xF077*/
#endif

#if !defined GT_SYMBOL_DOWN
#define GT_SYMBOL_DOWN            "\xEF\x81\xB8" /*61560, 0xF078*/
#endif

#if !defined GT_SYMBOL_LOOP
#define GT_SYMBOL_LOOP            "\xEF\x81\xB9" /*61561, 0xF079*/
#endif

#if !defined GT_SYMBOL_DIRECTORY
#define GT_SYMBOL_DIRECTORY       "\xEF\x81\xBB" /*61563, 0xF07B*/
#endif

#if !defined GT_SYMBOL_UPLOAD
#define GT_SYMBOL_UPLOAD          "\xEF\x82\x93" /*61587, 0xF093*/
#endif

#if !defined GT_SYMBOL_CALL
#define GT_SYMBOL_CALL            "\xEF\x82\x95" /*61589, 0xF095*/
#endif

#if !defined GT_SYMBOL_CUT
#define GT_SYMBOL_CUT             "\xEF\x83\x84" /*61636, 0xF0C4*/
#endif

#if !defined GT_SYMBOL_COPY
#define GT_SYMBOL_COPY            "\xEF\x83\x85" /*61637, 0xF0C5*/
#endif

#if !defined GT_SYMBOL_SAVE
#define GT_SYMBOL_SAVE            "\xEF\x83\x87" /*61639, 0xF0C7*/
#endif

#if !defined GT_SYMBOL_BARS
#define GT_SYMBOL_BARS            "\xEF\x83\x89" /*61641, 0xF0C9*/
#endif

#if !defined GT_SYMBOL_ENVELOPE
#define GT_SYMBOL_ENVELOPE        "\xEF\x83\xA0" /*61664, 0xF0E0*/
#endif

#if !defined GT_SYMBOL_CHARGE
#define GT_SYMBOL_CHARGE          "\xEF\x83\xA7" /*61671, 0xF0E7*/
#endif

#if !defined GT_SYMBOL_PASTE
#define GT_SYMBOL_PASTE           "\xEF\x83\xAA" /*61674, 0xF0EA*/
#endif

#if !defined GT_SYMBOL_BELL
#define GT_SYMBOL_BELL            "\xEF\x83\xB3" /*61683, 0xF0F3*/
#endif

#if !defined GT_SYMBOL_KEYBOARD
#define GT_SYMBOL_KEYBOARD        "\xEF\x84\x9C" /*61724, 0xF11C*/
#endif

#if !defined GT_SYMBOL_GPS
#define GT_SYMBOL_GPS             "\xEF\x84\xA4" /*61732, 0xF124*/
#endif

#if !defined GT_SYMBOL_FILE
#define GT_SYMBOL_FILE            "\xEF\x85\x9B" /*61787, 0xF158*/
#endif

#if !defined GT_SYMBOL_WIFI
#define GT_SYMBOL_WIFI            "\xEF\x87\xAB" /*61931, 0xF1EB*/
#endif

#if !defined GT_SYMBOL_BATTERY_FULL
#define GT_SYMBOL_BATTERY_FULL    "\xEF\x89\x80" /*62016, 0xF240*/
#endif

#if !defined GT_SYMBOL_BATTERY_3
#define GT_SYMBOL_BATTERY_3       "\xEF\x89\x81" /*62017, 0xF241*/
#endif

#if !defined GT_SYMBOL_BATTERY_2
#define GT_SYMBOL_BATTERY_2       "\xEF\x89\x82" /*62018, 0xF242*/
#endif

#if !defined GT_SYMBOL_BATTERY_1
#define GT_SYMBOL_BATTERY_1       "\xEF\x89\x83" /*62019, 0xF243*/
#endif

#if !defined GT_SYMBOL_BATTERY_EMPTY
#define GT_SYMBOL_BATTERY_EMPTY   "\xEF\x89\x84" /*62020, 0xF244*/
#endif

#if !defined GT_SYMBOL_USB
#define GT_SYMBOL_USB             "\xEF\x8a\x87" /*62087, 0xF287*/
#endif

#if !defined GT_SYMBOL_BLUETOOTH
#define GT_SYMBOL_BLUETOOTH       "\xEF\x8a\x93" /*62099, 0xF293*/
#endif

#if !defined GT_SYMBOL_TRASH
#define GT_SYMBOL_TRASH           "\xEF\x8B\xAD" /*62189, 0xF2ED*/
#endif

#if !defined GT_SYMBOL_EDIT
#define GT_SYMBOL_EDIT            "\xEF\x8C\x84" /*62212, 0xF304*/
#endif

#if !defined GT_SYMBOL_BACKSPACE
#define GT_SYMBOL_BACKSPACE       "\xEF\x95\x9A" /*62810, 0xF55A*/
#endif

#if !defined GT_SYMBOL_SD_CARD
#define GT_SYMBOL_SD_CARD         "\xEF\x9F\x82" /*63426, 0xF7C2*/
#endif

#if !defined GT_SYMBOL_NEW_LINE
#define GT_SYMBOL_NEW_LINE        "\xEF\xA2\xA2" /*63650, 0xF8A2*/
#endif

#if !defined GT_SYMBOL_EN
#define GT_SYMBOL_EN               "\xEE\x9C\x92" /*59154, 0xE712*/
#endif

#if !defined GT_SYMBOL_CH
#define GT_SYMBOL_CH                "\xEE\x9C\x93" /*59155, 0xE713*/
#endif

#if !defined GT_SYMBOL_SPACE
#define GT_SYMBOL_SPACE             "\xEE\xA1\x91" /*59473, 0xE851*/
#endif

#if !defined GT_SYMBOL_WELL_NUMBER
#define GT_SYMBOL_WELL_NUMBER       "\xEE\xA2\xB0" /*59568, 0xE8B0*/
#endif

#if !defined GT_SYMBOL_LOWER_CASE
#define GT_SYMBOL_LOWER_CASE         "\xEE\x98\xB7" /*59568, 0xE637*/
#endif

#if !defined GT_SYMBOL_UPPER_CASE
#define GT_SYMBOL_UPPER_CASE         "\xEE\x98\xB8" /*59568, 0xE638*/
#endif

#if !defined GT_SYMBOL_VOICE_PLAY
#define GT_SYMBOL_VOICE_PLAY          (3)
#define GT_SYMBOL_VOICE_PLAY_LEFT_1   "\xEE\x9C\x80" /* 0xE700 */
#define GT_SYMBOL_VOICE_PLAY_LEFT_2   "\xEE\x9C\x81" /* 0xE701 */
#define GT_SYMBOL_VOICE_PLAY_LEFT_3   "\xEE\x9C\x82" /* 0xE702 */
#define GT_SYMBOL_VOICE_PLAY_RIGHT_1  "\xEE\x9C\x83" /* 0xE703 */
#define GT_SYMBOL_VOICE_PLAY_RIGHT_2  "\xEE\x9C\x84" /* 0xE704 */
#define GT_SYMBOL_VOICE_PLAY_RIGHT_3  "\xEE\x9C\x85" /* 0xE705 */
#endif

#endif // GT_CFG_USE_SYMBOL_ENABLE
/* typedef --------------------------------------------------------------*/
typedef struct _gt_symbol_s{
    char * utf8;
    const uint8_t * mask_buf;
}_gt_symbol_st;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
bool gt_symbol_check_by_unicode(uint32_t unicode);
const uint8_t * gt_symbol_get_mask_buf(char * utf8 , uint8_t size);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SYMBOL_H_
