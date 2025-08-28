/**
 * @file gt_gc.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-06-14 16:56:38
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_gc.h"
#include "stdlib.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/

GT_ATTRIBUTE_RAM_DATA gt_gc_st _gt_global_call = {
    .scr_info = {
        .home_scr = {
            .id = -1,
            .screen = NULL,
            .alive = false,
        }
    },
    ._gt_event_node_header_ll = NULL,
#if GT_USE_EXTRA_FULL_IMG_BUFFER
    .full_img_buffer = {
        .buffer = NULL,
        .len = 0,
#if GT_USE_EXTRA_FULL_IMG_OFFSET
        .offset = 0,
#endif
    },
#endif
#if GT_USE_FILE_HEADER
    .file_header_ctl = {
        .header = NULL,
        .count = 0,
    },
#endif
#if GT_USE_SCREEN_ANIM
    .event_ctl = {
        .enabled = true,
    },
#endif
#if GT_FONT_USE_ASCII_WIDTH_CACHE
    .ascii_width_cache = {
        .option = GT_CFG_DEFAULT_FONT_FAMILY,
    },
#endif
};

/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_gc_st * GT_ATTRIBUTE_RAM_TEXT _get_gc_ctrl(void) {
    return &_gt_global_call;
}




/* global functions / API interface -------------------------------------*/

gt_disp_st * _gt_gc_get_disp(void)
{
    gt_gc_st * gc = _get_gc_ctrl();
    return gc->disp;
}

void _gt_gc_set_disp(gt_disp_st * disp)
{
    gt_gc_st * gc = _get_gc_ctrl();
    gc->disp = disp;
}

_gt_gc_scr_st * _gt_gc_get_scr_info(void)
{
    gt_gc_st * gc = _get_gc_ctrl();
    return &gc->scr_info;
}

uint16_t _gt_gc_get_size(void)
{
    return sizeof(gt_gc_st);
}

#if GT_USE_EXTRA_FULL_IMG_BUFFER
void gt_gc_set_full_img_buffer(uint8_t * buffer, uint32_t len)
{
    gt_gc_st * gc = _get_gc_ctrl();
    gc->full_img_buffer.buffer = buffer;
    gc->full_img_buffer.len = len;
}

#if GT_USE_EXTRA_FULL_IMG_OFFSET
void gt_gc_set_full_img_buffer_offset(uint32_t offset)
{
    gt_gc_st * gc = _get_gc_ctrl();
    gc->full_img_buffer.offset = offset;
}
#endif  /** GT_USE_EXTRA_FULL_IMG_OFFSET */

bool _gt_gc_full_img_buffer_is_valid(void)
{
    gt_gc_st * gc = _get_gc_ctrl();
    return gc->full_img_buffer.buffer && gc->full_img_buffer.len;
}

_gt_gc_full_img_buffer_st * _gt_gc_get_full_img_buffer(void)
{
    gt_gc_st * gc = _get_gc_ctrl();
    return &gc->full_img_buffer;
}
#endif  /** GT_USE_EXTRA_FULL_IMG_BUFFER */

#if GT_USE_EXTRA_FULL_FONT_BUFFER
void gt_gc_set_full_font_buffer(uint8_t * buffer, uint32_t len)
{
    gt_gc_st * gc = _get_gc_ctrl();
    gc->full_font_buffer.ptr = buffer;
    gc->full_font_buffer.len = len;
}

bool _gt_gc_full_font_buffer_is_valid(void)
{
    gt_gc_st * gc = _get_gc_ctrl();
    return gc->full_font_buffer.ptr && gc->full_font_buffer.len;
}

gt_buffer_st * _gt_gc_get_full_font_buffer(void)
{
    gt_gc_st * gc = _get_gc_ctrl();
    return &gc->full_font_buffer;
}
#endif  /** GT_USE_EXTRA_FULL_FONT_BUFFER */

#if GT_USE_EXTRA_FULL_FONT_MASK_BUFFER
void gt_gc_set_full_font_mask_buffer(uint8_t * buffer, uint32_t len)
{
    gt_gc_st * gc = _get_gc_ctrl();
    gc->full_font_mask_buffer.ptr = buffer;
    gc->full_font_mask_buffer.len = len;
}

bool _gt_gc_full_font_mask_buffer_is_valid(void)
{
    gt_gc_st * gc = _get_gc_ctrl();
    return gc->full_font_mask_buffer.ptr && gc->full_font_mask_buffer.len;
}

gt_buffer_st * _gt_gc_get_full_font_mask_buffer(void)
{
    gt_gc_st * gc = _get_gc_ctrl();
    return &gc->full_font_mask_buffer;
}
#endif  /** GT_USE_EXTRA_FULL_FONT_MASK_BUFFER */

#if GT_LAYER_TOP_SCREENSHOT_CACHE
void _gt_gc_set_screenshot_cache(char * cache, uint32_t cache_len)
{
    gt_gc_st * gc = _get_gc_ctrl();
    gc->screenshot_cache.ptr = cache;
    gc->screenshot_cache.len = cache_len;
}

bool _gt_gc_screenshot_cache_is_valid(void)
{
    gt_gc_st * gc = _get_gc_ctrl();
    return gc->screenshot_cache.ptr && gc->screenshot_cache.len;
}

gt_buffer_st * _gt_gc_get_screenshot_cache(void)
{
    gt_gc_st * gc = _get_gc_ctrl();
    return &gc->screenshot_cache;
}
#endif

#if GT_FONT_USE_ASCII_WIDTH_CACHE
/**
 * @brief Get the ASCII font width cache object
 *
 * @return _gt_gc_ascii_width_cache_st
 */
_gt_gc_ascii_width_cache_st * _gt_gc_get_ascii_width_cache(void)
{
    gt_gc_st * gc = _get_gc_ctrl();
    return &gc->ascii_width_cache;
}
#endif
/* end ------------------------------------------------------------------*/
