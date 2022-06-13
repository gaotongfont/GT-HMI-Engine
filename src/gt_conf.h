/**
 * @file gt_conf.h
 * @author Feyoung
 * @brief 
 * @version 0.1
 * @date 2022-05-11 14:43:24
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_CONF_H_
#define _GT_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/






/* define ---------------------------------------------------------------*/

/* user: screen width and height config */
#define GT_SCREEN_WIDTH     320
#define GT_SCREEN_HEIGHT    240

/* GT_REFRESH_STYLE type don't change */
#define GT_REFRESH_STYLE_0      0   /* no cache buf, It is suitable for MCU with low refresh requirements and low performance */
#define GT_REFRESH_STYLE_1      1   /* cache 10 lines buf, It is suitable for MCU with low refresh requirements and low performance */
#define GT_REFRESH_STYLE_2      2   /* cache 10+10 lines buf, It is suitable for MCU with high refresh requirements and high performance */
#define GT_REFRESH_STYLE_3      3   /* cache all screen buf,It is suitable for MCU with high refresh requirements and high performance */

/* user: screen refresh style */
#define GT_REFRESH_STYLE        GT_REFRESH_STYLE_3

/* user: obj contain user_data: 1 yes, 0 no */
#define GT_USE_USER_DATA    1

/* user: color depth: 1(1 byte per pixel), 8(RGB332), 16(RGB565), 32(ARGB8888) */
#define GT_COLOR_DEPTH      16
#define GT_COLOR_16_SWAP    0

/* user: event handler timer */
#define GT_EVENT_TIMER_PER  30

/* user: enable widgets */
#define GT_ENABLE_WIDGET_BTN            1
#define GT_ENABLE_WIDGET_TEXTAREA       1
#define GT_ENABLE_WIDGET_LABEL          1
#define GT_ENABLE_WIDGET_INPUT          1
#define GT_ENABLE_WIDGET_IMG            1

/**
 * @brief memory mode. 1: array as memory pool; 0:  c library api.
 */
#define GT_MEM_CUSTOM       0

#if GT_MEM_CUSTOM
    #define GT_MEM_CUSTOM_INCLUDE   "../others/gt_tlsf.h"

    #define GT_MEM_SIZE             (24 * 1024U)    //Byte
#else
    #define GT_MEM_CUSTOM_INCLUDE   "stdlib.h"

    #define GT_MEM_CUSTOM_MALLOC    malloc
    #define GT_MEM_CUSTOM_CALLOC    calloc
    #define GT_MEM_CUSTOM_REALLOC   realloc
    #define GT_MEM_CUSTOM_FREE      free
#endif



/* Compiler prefix for a big array declaration in RAM */
#define GT_ATTRIBUTE_LARGE_RAM_ARRAY


/* ui default style */
#define GT_STYLE_UI_SIMPLE     0
#define GT_STYLE_UI_DARK       1
#define GT_STYLE_UI_BRIGHT     2

/* default gesture repeat */
#define GT_CFG_DEFAULT_POINT_LONG_PRESS_TIMERS      15  /* 30ms*15=450ms repeat trigger once event */
#define GT_CFG_DEFAULT_POINT_GERSTURE_PIXEL_HOR     (GT_SCREEN_WIDTH/3)
#define GT_CFG_DEFAULT_POINT_GERSTURE_PIXEL_VER     (GT_SCREEN_HEIGHT/3)
#define GT_CFG_DEFAULT_POINT_SCROLL_PIXEL           (20)

#define GT_CFG_DEFAULT_BUTTON_

#define GT_CFG_DEFAULT_KEYPAD_

/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_CONF_H_
