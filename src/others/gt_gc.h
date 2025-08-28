/**
 * @file gt_gc.h
 * @author Feyoung
 * @brief The global call
 * @version 0.1
 * @date 2022-06-14 16:56:57
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_GC_H_
#define _GT_GC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_ll.h"
#include "stdint.h"
#include "./gt_types.h"
#include "../gt_conf.h"
#include "../core/gt_timer.h"
#include "../others/gt_list.h"
#include "../others/gt_color.h"
#include "../hal/gt_hal_disp.h"
#include "../hal/gt_hal_file_header.h"
#include "../font/gt_font.h"
#include "../core/gt_disp.h"

/* define ---------------------------------------------------------------*/

#if GT_USE_EXTRA_FULL_IMG_BUFFER

#ifndef GT_USE_EXTRA_FULL_IMG_OFFSET
    /**
     * @brief Using the full image buffer, set the offset of the buffer is valid.
     */
    #define GT_USE_EXTRA_FULL_IMG_OFFSET    0
#endif

/**
 * @brief Malloc or free by user.
 */
typedef struct _gt_gc_full_img_buffer_s {
    uint8_t * buffer;
    uint32_t len;
#if GT_USE_EXTRA_FULL_IMG_OFFSET
    uint32_t offset;
#endif
}_gt_gc_full_img_buffer_st;
#endif

#if GT_USE_SCREEN_ANIM
typedef struct _gt_gc_event_s {
    uint8_t enabled : 1;    /** Disabled when the screen is animating */
    uint8_t reserved : 7;
}_gt_gc_event_st;
#endif

#if GT_FONT_USE_ASCII_WIDTH_CACHE
typedef struct _gt_gc_ascii_width_cache_s {
    uint16_t option;
    uint16_t size;
    uint8_t gray;
    gt_font_width_t ascii_width_cache[_FONT_ASCII_WIDTH_CACHE_COUNT];
}_gt_gc_ascii_width_cache_st;
#endif

typedef struct gt_gc_s {
    struct _gt_list_head _gt_anim_ll;
    struct _gt_list_head _gt_timer_ll;
    struct _gt_list_head _gt_img_decoder_ll;
    struct _gt_event_s * _gt_event_node_header_ll;

    gt_disp_st * disp;
    _gt_gc_scr_st scr_info;

#if GT_USE_EXTRA_FULL_IMG_BUFFER
    _gt_gc_full_img_buffer_st full_img_buffer;
#endif

#if GT_USE_EXTRA_FULL_FONT_BUFFER
    gt_buffer_st full_font_buffer;
#endif

#if GT_USE_EXTRA_FULL_FONT_MASK_BUFFER
    gt_buffer_st full_font_mask_buffer;
#endif

#if GT_USE_FILE_HEADER
    _gt_file_header_ctl_st file_header_ctl;
#endif

#if GT_LAYER_TOP_SCREENSHOT_CACHE
    gt_buffer_st screenshot_cache;
#endif

#if GT_USE_SCREEN_ANIM
    _gt_gc_event_st event_ctl;
#endif
#if GT_FONT_USE_ASCII_WIDTH_CACHE
    _gt_gc_ascii_width_cache_st ascii_width_cache;
#endif
}gt_gc_st;

extern GT_ATTRIBUTE_RAM_DATA gt_gc_st _gt_global_call;

#define _GT_GC_GET_ROOT(_name)  ((&_gt_global_call)->_name)

/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/

static inline int _gt_gc_is_ll_empty(const struct _gt_list_head * head) {
    return _gt_list_empty(head);
}


/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

gt_disp_st * _gt_gc_get_disp(void);
void _gt_gc_set_disp(gt_disp_st * disp);

_gt_gc_scr_st * _gt_gc_get_scr_info(void);

uint16_t _gt_gc_get_size(void);

#if GT_USE_EXTRA_FULL_IMG_BUFFER
/**
 * @brief Set external full image buffer, this buffer will be used by img reading
 *      from flash or SD nand flash, etc. Set with the maximum width and height of
 *      the material.
 *
 * @param buffer color 8 / 565 / 888 buffer, byte
 * @param len buffer length, must be equal or more than
 *          [max_width * max_height * (sizeof(gt_opa_t) + sizeof(gt_color_t))].
 */
void gt_gc_set_full_img_buffer(uint8_t * buffer, uint32_t len);

#if GT_USE_EXTRA_FULL_IMG_OFFSET
/**
 * @brief Using the full image buffer, set the offset of the buffer is valid.
 *
 * @param offset 0[default]: Buffer begin address
 */
void gt_gc_set_full_img_buffer_offset(uint32_t offset);
#endif

bool _gt_gc_full_img_buffer_is_valid(void);

_gt_gc_full_img_buffer_st * _gt_gc_get_full_img_buffer(void);
#endif

#if GT_USE_EXTRA_FULL_FONT_BUFFER
/**
 * @brief Set external full font buffer, this buffer will be used by font getting
 *      data from flash or SD nand flash, etc.
 *
 * @param buffer Also can used gt_gc_set_full_img_buffer()'s buffer as font buffer.
 * @param len Recommend larger than: max_font_size * max_font_size * font_gray * sizeof(gt_color_t).
 */
void gt_gc_set_full_font_buffer(uint8_t * buffer, uint32_t len);

bool _gt_gc_full_font_buffer_is_valid(void);

gt_buffer_st * _gt_gc_get_full_font_buffer(void);
#endif

#if GT_USE_EXTRA_FULL_FONT_MASK_BUFFER
/**
 * @brief Set external full font mask buffer, this buffer will be used by font mask getting
 *      data from flash or SD nand flash, etc.
 *
 * @param buffer [WARN] DO NOT REUSE gt_gc_set_full_font_buffer()'s buffer for store font mask data.
 * @param len Recommend larger than: max_font_size * max_font_size * sizeof(gt_opa_t).
 */
void gt_gc_set_full_font_mask_buffer(uint8_t * buffer, uint32_t len);

bool _gt_gc_full_font_mask_buffer_is_valid(void);

gt_buffer_st * _gt_gc_get_full_font_mask_buffer(void);
#endif

#if GT_LAYER_TOP_SCREENSHOT_CACHE
/**
 * @brief Register screenshot cache buffer to pref top layer full cover draw,
 *      reduce base screen loading time.
 *
 * @param cache
 * @param cache_len full screen cache buffer size
 */
void _gt_gc_set_screenshot_cache(char * cache, uint32_t cache_len);

/**
 * @brief
 *
 * @return true Has init screenshot cache buffer before
 * @return false
 */
bool _gt_gc_screenshot_cache_is_valid(void);

/**
 * @brief Get screenshot cache buffer
 *
 * @return gt_buffer_st*
 */
gt_buffer_st * _gt_gc_get_screenshot_cache(void);
#endif


#if GT_FONT_USE_ASCII_WIDTH_CACHE
/**
 * @brief Get the ASCII font width cache object
 *
 * @return _gt_gc_ascii_width_cache_st *
 */
_gt_gc_ascii_width_cache_st * _gt_gc_get_ascii_width_cache(void);
#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_GC_H_
