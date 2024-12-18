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
    #define GT_COLOR_DEPTH      16
#endif
#ifndef GT_COLOR_16_SWAP
    #define GT_COLOR_16_SWAP    0
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
#define GT_TASK_PERIOD_TIME_SERIAL  20

/** Free object memory after N ms timer */
#define GT_TASK_PERIOD_TIME_DESTROY 10

/* user: virt file device setting */
#ifndef GT_VF_FLASH_SIZE
    //HMI-chip or flash size 16*1024*1024   16M
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

#ifndef GT_ATTRIBUTE_RAM_DATA
    #define GT_ATTRIBUTE_RAM_DATA   GT_ATTRIBUTE_LARGE_RAM_ARRAY
#endif

#ifndef GT_ATTRIBUTE_RAM_TEXT
    #define GT_ATTRIBUTE_RAM_TEXT
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

#ifndef GT_USE_IMG_CACHE
    /**
     * @brief Enabled use a separate image cache to store meta data,
     *      @ref gt_img_cache.h, get raw object to set widget image data.
     *      [default: 0]
     */
    #define GT_USE_IMG_CACHE            0
#endif

#ifndef GT_USE_WIDGET_LAYOUT
    /**
     * @brief Set the widget layout function:
     *      0: no layout function, improve performance
     *      1: layout function
     * [Default: 1]
     */
    #define GT_USE_WIDGET_LAYOUT        01
#endif

#ifndef GT_FONT_FAMILY_OLD_ENABLE
    /**
     * @brief use old font family
     *
     */
    #define GT_FONT_FAMILY_OLD_ENABLE   01
#endif

/* default font style and size */
#define GT_CFG_DEFAULT_FONT_FAMILY          (0xFFFF)
#define GT_CFG_DEFAULT_FONT_SIZE            (16)

#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
#define GT_CFG_DEFAULT_FONT_FAMILY_CN       0xFFFE
#define GT_CFG_DEFAULT_FONT_FAMILY_EN       GT_CFG_DEFAULT_FONT_FAMILY
#define GT_CFG_DEFAULT_FONT_FAMILY_FL       GT_CFG_DEFAULT_FONT_FAMILY
#define GT_CFG_DEFAULT_FONT_FAMILY_NUMB     GT_CFG_DEFAULT_FONT_FAMILY
#endif

#ifndef GT_FONT_USE_ASCII_WIDTH_CACHE
    /**
     * @brief 1: Cache the width of the ASCII font, 0: Do not cache.
     *      Almost 94 characters, (94 * _FONT_ASCII_WIDTH_CACHE_BYTE_SIZE) bytes
     *      [default: 01]
     */
    #define GT_FONT_USE_ASCII_WIDTH_CACHE     01
#endif

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
    #ifndef GT_USE_DIRECT_ADDR
        /**
         * @brief Enabled direct address function, range such as:
         *        GT_VF_FLASH_START -> GT_VF_FLASH_START + GT_VF_FLASH_SIZE
         */
        #define GT_USE_DIRECT_ADDR      0
    #endif
    #ifndef GT_USE_DIRECT_ADDR_CUSTOM_SIZE
        /**
         * @brief Enabled direct address custom size function, such as:
         *      { addr, w, h, is_alpha }
         * [Default: 0]
         */
        #define GT_USE_DIRECT_ADDR_CUSTOM_SIZE   0
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

#ifndef GT_USE_CUSTOM_TOUCH_EXPAND_SIZE
    /**
     * @brief Enabled custom touch expand size, such as:
     *      widget size: 20x20,  expand size: (5, 10), touch size: 30x40.
     * [Default: 0] widget default display size
     */
    #define GT_USE_CUSTOM_TOUCH_EXPAND_SIZE     0
#endif

/** display refresh time */
#define GT_USE_DISPLAY_PREF_MSG         0

/** display destroy object time */
#define GT_USE_DISPLAY_PREF_DESTROY     0

/** display idle time */
#define GT_USE_DISPLAY_PREF_IDLE        0

#ifndef GT_USE_SERIAL
    /**
     * @brief Enabled serial communications function @ref gt_serial.h
     *      [default: 0]
     */
    #define GT_USE_SERIAL       0
#endif

#if GT_USE_SERIAL
    #ifndef GT_SERIAL_USE_TIMER_RECV_UNPACK
        /**
         * @brief Use client recv timer to unpack serial data, timer period
         *      @ref GT_TASK_PERIOD_TIME_SERIAL, callback function @ref
         *      _serial_client_recv_timer_handler_cb().
         *      [default: 1]
         */
        #define GT_SERIAL_USE_TIMER_RECV_UNPACK 1
    #endif

    #ifndef GT_SERIAL_MASTER_CACHE_SIZE
        /**
         * @brief The size of the send cache cycle buffer
         */
        #define GT_SERIAL_MASTER_CACHE_SIZE     512
    #endif

    #ifndef GT_SERIAL_CLIENT_CACHE_SIZE
        /**
         * @brief The size of the recv cache cycle buffer
         */
        #define GT_SERIAL_CLIENT_CACHE_SIZE     512
    #endif

    #ifndef GT_SERIAL_PACK_CACHE_SIZE
        /**
         * @brief The size of the pack data cache for each time
         */
        #define GT_SERIAL_PACK_CACHE_SIZE       512
    #endif

    #ifndef GT_SERIAL_UNPACK_CACHE_SIZE
        /**
         * @brief The size of the unpack data cache for each time
         */
        #define GT_SERIAL_UNPACK_CACHE_SIZE     512
    #endif

    #ifndef GT_SERIAL_HEADER_BYTE
        /**
         * @brief The serial communication header byte
         *      [Warn] Range: 1 ~ 4 bytes
         */
        #define GT_SERIAL_HEADER_BYTE           { 0x5A, 0xA5 }
    #endif

    #ifndef GT_SERIAL_WIDTH_BYTE_LENGTH
        /**
         * @brief The serial communication width byte length to store recv valid byte data
         *      [default: 1]
         */
        #define GT_SERIAL_WIDTH_BYTE_LENGTH     1
    #endif

    #ifndef GT_SERIAL_USE_AUTO_PACK
        /**
         * @brief Set auto pack, such as: [0x83, 0x00, 0x10, 0x04],
         *      result: [0x5a, 0xa5, 0x06, 0x83, 0x00, 0x10, 0x04, 0x25, 0xa3]
         *      [default: 1]
         */
        #define GT_SERIAL_USE_AUTO_PACK         1
    #endif

    #ifndef GT_SERIAL_GET_ONLY_VALID_DATA
        /**
         * @brief Only valid byte data is kept, remove headers, length, and crc16 byte data;
         *      otherwise, keep all data.
         *      such as: 0x5a, 0xa5, 0x06, 0x83, 0x00, 0x10, 0x04, 0x25, 0xa3
         *      result:  0x83, 0x00, 0x10, 0x04
         *      [default: 1]
         */
        #define GT_SERIAL_GET_ONLY_VALID_DATA   1
    #endif

    #undef GT_QUEUE_USE_PRE_CHECK_VALID
    /**
     * @brief [Warn] Force Use pre-check data by callback function, such as: check uart data is valid.
     *      Must be used with GT_USE_SERIAL.
     */
    #define GT_QUEUE_USE_PRE_CHECK_VALID        1

    #undef GT_USE_CRC
    /**
     * @brief [Warn] Force enabled CRC function @ref gt_crc.h
     *      Must be used with GT_USE_SERIAL.
     */
    #define GT_USE_CRC          1

    #undef GT_USE_BIN_CONVERT
    /**
     * @brief [Warn] Force enabled bin convert function @ref gt_bin_convert.h
     *      Must be used with GT_USE_SERIAL.
     */
    #define GT_USE_BIN_CONVERT  1

    #undef GT_INDEV_SIMULATE_POINTER
    /**
     * @brief [Warn] Force enabled simulate pointer input device
     */
    #define GT_INDEV_SIMULATE_POINTER       1

    #undef GT_USE_FILE_HEADER
    /**
     * @brief [Warn] Force enabled img file header function @ref gt_file_header_st
     */
    #define GT_USE_FILE_HEADER              1

    #undef GT_FONT_FAMILY_OLD_ENABLE
    /**
     * @brief [Warn] Force use new font family
     */
    #define GT_FONT_FAMILY_OLD_ENABLE       0
#endif  /** GT_USE_SERIAL */

#ifndef GT_USE_BIN_CONVERT
    /**
     * @brief Enabled bin convert function @ref gt_bin_convert.h
     *      [default: 0]
     */
    #define GT_USE_BIN_CONVERT  0
#endif

#ifndef GT_USE_CRC
    /**
     * @brief Enabled CRC function @ref gt_crc.h
     * [default: 0]
     */
    #define GT_USE_CRC          0
#endif

#ifndef GT_USE_PNG
    /**
     * @brief use lodepng
     * [default: 1]
     */
    #define GT_USE_PNG          01
#endif

#ifndef GT_USE_SJPG
    /**
     * @brief use TJpgDec
     * [default: 1]
     */
    #define GT_USE_SJPG         01
#endif

#ifndef GT_USE_GIF
    /**
     * @brief use gifDec
     * [default: 1]
     */
    #define GT_USE_GIF          01
#endif

#ifndef GT_USE_MD4C
    /**
     * @brief use md4c markdown parser library
     * [default: 1]
     */
    #define GT_USE_MD4C         01
#endif

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

#ifndef GT_USE_UD_LR_TO_CONTROL_FOCUS_EN
    /**
     * @brief Use up and down left to control focus
     *      [default: 0] using next or prev logical
     */
    #define GT_USE_UD_LR_TO_CONTROL_FOCUS_EN     0
#endif


/**
 * @brief Set the focus color
 *
 */
#ifndef GT_FOCUS_COLOR_SELECT
    #define GT_FOCUS_COLOR_SELECT   0x0078D7
#endif

#ifndef GT_FOCUS_COLOR_LOCK
    #define GT_FOCUS_COLOR_LOCK     0xFF0000
#endif

/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_CONF_H_
