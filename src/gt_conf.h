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
#define GT_SCREEN_WIDTH     800
#define GT_SCREEN_HEIGHT    480

/* GT_REFRESH_STYLE type don't change */
#define GT_REFRESH_STYLE_0      0   /* no cache buf, It is suitable for MCU with low refresh requirements and low performance */
#define GT_REFRESH_STYLE_1      1   /* cache 10 lines buf, It is suitable for MCU with low refresh requirements and low performance */
#define GT_REFRESH_STYLE_2      2   /* cache 10+10 lines buf, It is suitable for MCU with high refresh requirements and high performance */
#define GT_REFRESH_STYLE_3      3   /* cache all screen buf, It is suitable for MCU with high refresh requirements and high performance */

/* user: screen refresh style */
#define GT_REFRESH_STYLE        GT_REFRESH_STYLE_1

/* screen refresh flush pre time lines count [default: 10] */
#if GT_REFRESH_STYLE == GT_REFRESH_STYLE_1
#define GT_REFRESH_FLUSH_LINE_PRE_TIME   24
#elif GT_REFRESH_STYLE == GT_REFRESH_STYLE_2
#define GT_REFRESH_FLUSH_LINE_PRE_TIME   10
#elif GT_REFRESH_STYLE == GT_REFRESH_STYLE_3
#define GT_REFRESH_FLUSH_LINE_PRE_TIME   GT_SCREEN_HEIGHT
#else
#define GT_REFRESH_FLUSH_LINE_PRE_TIME   10
#endif

/* user: obj contain user_data: 1 yes, 0 no */
#define GT_USE_USER_DATA    1

/* user: color depth: 1(1 byte per pixel), 8(RGB332), 16(RGB565), 32(ARGB8888) */
#define GT_COLOR_DEPTH      16
#define GT_COLOR_16_SWAP    0

/* user: Timer task handler timer [ms] */
#define GT_TASK_PERIOD_TIME_EVENT   30
#define GT_TASK_PERIOD_TIME_ANIM    30

/* user: virt file device setting */
#define GT_VF_FLASH_SIZE    0x800000    //flash size 8*1024*1024   8M
#define GT_VF_FLASH_START   0x000000    //start addr default 0x0000000

/**
 * @brief memory mode. 1: array as memory pool; 0:  c library api.
 */
#define GT_MEM_CUSTOM       01

#if GT_MEM_CUSTOM
    #define GT_MEM_CUSTOM_INCLUDE   "../others/gt_tlsf.h"

    #define GT_MEM_SIZE             (24 * 1024U)    //Byte

    #define gt_tlsf_assert(_expr)   ( (void)0 )
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


/* default font style and size */
#define GT_CFG_DEFAULT_FONT_STYLE_CN    1
#define GT_CFG_DEFAULT_FONT_STYLE_EN    4
#define GT_CFG_DEFAULT_FONT_SIZE        16

/* default event mask bit num */
#define GT_CFG_DEFAULT_EVENT_MASK_BIT_MAX           16

#define GT_USE_MODE_FLASH       01
#define GT_USE_MODE_SD          01

/**
 * @brief use lodepng
 */
#define GT_USE_PNG              01

/**
 * @brief use examples demo
 */
#define GT_USE_EXAMPLES         01


/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_CONF_H_
