/**
 * @file gt_conf.h
 * @author Feyoung
 * @brief Configuration infomation for the GT GUI
 * @version 0.1
 * @date 2022-05-11 14:43:24
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_CONF_H_
#define _GT_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/



/* define ---------------------------------------------------------------*/

/* user: screen width and height config */
#ifndef GT_SCREEN_WIDTH
    #define GT_SCREEN_WIDTH     800
#endif
#ifndef GT_SCREEN_HEIGHT
    #define GT_SCREEN_HEIGHT    480
#endif

/* GT_REFRESH_STYLE type don't change */
#define GT_REFRESH_STYLE_0      0   /* no cache buf, It is suitable for MCU with low refresh requirements and low performance */
#define GT_REFRESH_STYLE_1      1   /* cache 10 lines buf, It is suitable for MCU with low refresh requirements and low performance */
#define GT_REFRESH_STYLE_2      2   /* cache 10+10 lines buf, It is suitable for MCU with high refresh requirements and high performance */
#define GT_REFRESH_STYLE_3      3   /* cache all screen buf, It is suitable for MCU with high refresh requirements and high performance */

/* user: screen refresh style */
#define GT_REFRESH_STYLE        GT_REFRESH_STYLE_3

/* screen refresh flush pre time lines count [default: 10] */
#if GT_REFRESH_STYLE == GT_REFRESH_STYLE_1
#define GT_REFRESH_FLUSH_LINE_PRE_TIME   40
#elif GT_REFRESH_STYLE == GT_REFRESH_STYLE_2
#define GT_REFRESH_FLUSH_LINE_PRE_TIME   10
#elif GT_REFRESH_STYLE == GT_REFRESH_STYLE_3
#define GT_REFRESH_FLUSH_LINE_PRE_TIME   GT_SCREEN_HEIGHT
#else
#define GT_REFRESH_FLUSH_LINE_PRE_TIME   10
#endif

/* user: refr max area */
#define _GT_REFR_AREA_MAX   32

/* user: color depth: 1(1 byte per pixel), 8(RGB332), 16(RGB565), 32(ARGB8888) */
#ifndef GT_COLOR_DEPTH
    #define GT_COLOR_DEPTH          16
#endif
#ifndef GT_COLOR_16_SWAP
    #define GT_COLOR_16_SWAP        0
#endif

#ifndef GT_FLUSH_CONVERT_VERTICAL
    /**
     * @brief Set the screen refresh direction, 0: horizontal, 1: vertical
     * [Default: 0]
     */
    #define GT_FLUSH_CONVERT_VERTICAL   0
#endif

/* user: Timer task handler timer [ms] */
#define GT_TASK_PERIOD_TIME_INDEV   10
#define GT_TASK_PERIOD_TIME_EVENT   10
#define GT_TASK_PERIOD_TIME_ANIM    10
#define GT_TASK_PERIOD_TIME_REFR    10

/** Free object memory after N ms timer */
#define GT_TASK_PERIOD_TIME_DESTROY 10

/* user: virt file device setting */
#ifndef GT_VF_FLASH_SIZE
    //flash size 16*1024*1024   16M
    #define GT_VF_FLASH_SIZE        0x1000000
#endif
#ifndef GT_VF_FLASH_START
    //start addr default 0x0000000
    #define GT_VF_FLASH_START       0x000000
#endif

/**
 * @brief memory mode. 1: array as memory pool; 0:  c library api.
 */
#ifndef GT_MEM_CUSTOM
    #define GT_MEM_CUSTOM           01
#endif

#if GT_MEM_CUSTOM
    #define GT_MEM_CUSTOM_INCLUDE   "../others/gt_tlsf.h"

    #define GT_MEM_SIZE             (24 * 1024U)    //Byte

    #define gt_tlsf_assert(_expr)   ( (void)0 )
#else
    #define GT_MEM_CUSTOM_INCLUDE   "stdlib.h"

    #define GT_MEM_CUSTOM_MALLOC        malloc
    #define GT_MEM_CUSTOM_CALLOC        calloc
    #define GT_MEM_CUSTOM_REALLOC       realloc
    #define GT_MEM_CUSTOM_FREE          free
    #define GT_MEM_CUSTOM_BLOCK_SIZE    NULL
#endif



/* Compiler prefix for a big array declaration in RAM */
#ifndef GT_ATTRIBUTE_LARGE_RAM_ARRAY
    #define GT_ATTRIBUTE_LARGE_RAM_ARRAY
#endif


/* ui default style */
#define GT_STYLE_UI_SIMPLE     0
#define GT_STYLE_UI_DARK       1
#define GT_STYLE_UI_BRIGHT     2

#define GT_CFG_DEFAULT_POINT_SCROLL_PIXEL_INVALID   80

#ifndef GT_BOOTING_INFO_MSG
    /**
     * @brief Print Engine information when booting, 0: no display, 1: display
     * [Default: 0]
     */
    #define GT_BOOTING_INFO_MSG     0
#endif

#ifndef GT_USE_LAYER_TOP
    /**
     * @brief Can be used to set the top layer, the top layer
     *      is always displayed on the screen.
     *      Such as: dialog, pop-up window, status bar, etc.
     *      [Default: 1]
     */
    #define GT_USE_LAYER_TOP    01
#endif

#ifndef GT_USE_SCREEN_ANIM
    /**
     * @brief Set the screen animation effect, the screen animation
     *      [default: 1] 0: only GT_SCR_ANIM_TYPE_NONE can be used.
     * @ref gt_scr_anim_type_et
     */
    #define GT_USE_SCREEN_ANIM  01
#endif

#ifndef GT_USE_EXTRA_FULL_IMG_BUFFER
    /**
     * @brief Set with the maximum width and height of the material,
     *      read all the data of a single image material from flash at once.
     *      If the storage space is less than the image data, default
     *      use line fill mode. Default: 0.
     * @see ./src/extra/gt_extra.c -> gt_gc_set_full_img_buffer()
     */
    #define GT_USE_EXTRA_FULL_IMG_BUFFER    0
#endif

/* default font style and size */
#define GT_CFG_DEFAULT_FONT_FAMILY_CN       0xFFFE
#define GT_CFG_DEFAULT_FONT_FAMILY_EN       0xFFFF
#define GT_CFG_DEFAULT_FONT_FAMILY_FL       GT_CFG_DEFAULT_FONT_FAMILY_EN
#define GT_CFG_DEFAULT_FONT_FAMILY_NUMB     GT_CFG_DEFAULT_FONT_FAMILY_EN
#define GT_CFG_DEFAULT_FONT_SIZE            16

#ifndef GT_USE_MODE_SRC
    #define GT_USE_MODE_SRC             01
#endif
#ifndef GT_USE_MODE_FLASH
    #define GT_USE_MODE_FLASH           01
#endif
#ifndef GT_USE_MODE_SD
    #define GT_USE_MODE_SD              0
#endif

#if GT_USE_MODE_FLASH || GT_USE_MODE_SD
    #ifndef GT_USE_FILE_HEADER
        /**
         * @brief Enabled img file header function @ref gt_file_header_st, such as:
         *      { index[2], count[2], address[4], width[2], height[2], reg[4] }
         *      reg: colorDepth, alpha, etc.
         *      More details see @ref gt_hal_file_header.h
         *
         * [Default: 0]
         */
        #define GT_USE_FILE_HEADER      0
    #endif
#endif

#ifndef GT_USE_FOLDER_SYSTEM
    /**
     * @brief Enabled file system function such as:
     *       dir_open_cb() / dir_read_cb() / dir_close_cb()
     * [Default: 0]
     */
    #define GT_USE_FOLDER_SYSTEM        0
#endif

#ifndef GT_USE_FS_NAME_BY_INDEX
    /**
     * @brief Enabled file system get name by index of list
     * [Default: 0]
     */
    #define GT_USE_FS_NAME_BY_INDEX     0
#endif

/** display refresh time */
#define GT_USE_DISPLAY_PREF_MSG         0

/** display destroy object time */
#define GT_USE_DISPLAY_PREF_DESTROY     0

/** display idle time */
#define GT_USE_DISPLAY_PREF_IDLE        0

/** @brief use lodepng */
#define GT_USE_PNG              01
/** @brief use TJpgDec */
#define GT_USE_SJPG             01
/** @brief use gifDec */
#define GT_USE_GIF              01

/**
 * @brief use examples demo
 */
#define GT_USE_EXAMPLES             01

#define GT_CONFIG_GUI_DRIVER_LIB    01

#ifndef _GT_USE_TEST
    /**
     * @brief Use the test module
     */
    #define _GT_USE_TEST       0
#endif
/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_CONF_H_
