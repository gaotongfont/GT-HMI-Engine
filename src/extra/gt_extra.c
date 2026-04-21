/**
 * @file gt_extra.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-08-17 14:09:08
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_extra.h"
#include "stddef.h"
#include "../others/gt_gc.h"

#if GT_USE_PNG
#include "./png/gt_png.h"
#endif

#if GT_USE_SJPG
#include "./jpeg/gt_sjpg.h"
#endif

#if GT_USE_SVG
#include "./svg/gt_svg.h"
#endif


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief extra function init
 */
void _gt_extra_init(void)
{
    uint32_t size = 0;
    char * buffer = NULL;
#if GT_LAYER_TOP_SCREENSHOT_CACHE
    if (false == _gt_gc_screenshot_cache_is_valid()) {
        size = GT_SCREEN_WIDTH * GT_SCREEN_HEIGHT * sizeof(gt_color_t);
        buffer = gt_mem_extern_malloc(size);
        if (NULL == buffer) {
            size = 0;
        }
        _gt_gc_set_screenshot_cache(buffer, size);
    }
#endif

#if GT_USE_EXTRA_FULL_IMG_BUFFER
    if (false == _gt_gc_full_img_buffer_is_valid()) {
        gt_gc_set_full_img_buffer(NULL, 0);
    }
#endif

#if GT_USE_EXTRA_FULL_FONT_BUFFER
    if (false == _gt_gc_full_font_buffer_is_valid()) {
        gt_gc_set_full_font_buffer(NULL, 0);
    }
#endif

#if GT_USE_EXTRA_FULL_FONT_MASK_BUFFER
    if (false == _gt_gc_full_font_mask_buffer_is_valid()) {
        gt_gc_set_full_font_mask_buffer(NULL, 0);
    }
#endif

#if GT_USE_PNG
    gt_png_init();
#endif

#if GT_USE_SJPG
    gt_sjpg_init();
#endif

#if GT_USE_SVG
    gt_svg_init();
#endif

}


/* end ------------------------------------------------------------------*/
