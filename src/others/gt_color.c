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
    gt_color_t const * const end_p = color_arr + len;
    gt_color_t * ptr = color_arr;

    while (ptr < end_p) {
        *ptr = color;
        ++ptr;
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
