/**
 * @file gt_color.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-13 10:23:58
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_color.h"
#include "../widgets/gt_obj.h"
#include "../core/gt_mem.h"
#include "./gt_log.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static uint32_t _focus_color = 0x0078D7;


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

void gt_color_fill(gt_color_t * color_arr, uint32_t len, gt_color_t color)
{
    gt_color_t * ptr = color_arr;
    gt_color_t const * end_p = color_arr + len - 2;
#if 16 == GT_COLOR_DEPTH
    uint32_t * ptr32 = NULL;
    uint32_t color32 = (color.full << 16) | color.full;
#endif

    if ((gt_uintptr_t)ptr & 0x3) {
        *ptr = color; ++ptr;
    }
    while (ptr < end_p) {
#if 16 == GT_COLOR_DEPTH
        ptr32 = (uint32_t *)ptr;
        ptr32[0] = color32;
#else
        ptr[0] = color;
        ptr[1] = color;
#endif
        ptr += 2;
    }

    end_p = color_arr + len;
    while (ptr < end_p) {
        *ptr = color; ++ptr;
    }
}

gt_color_t gt_color_focus(void)
{
    return gt_color_hex(_focus_color);
}

void gt_color_focus_set(uint32_t col)
{
    _focus_color = col;
}

/* end ------------------------------------------------------------------*/
