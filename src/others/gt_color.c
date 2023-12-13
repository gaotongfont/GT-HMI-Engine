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

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static uint32_t _focus_color = 0x0078D7;


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

void gt_color_area_copy(gt_color_t *dst, gt_area_st area_dst, gt_color_t * src, gt_area_st area_src, gt_size_t w, gt_size_t h){
    uint32_t line = 0;
    uint16_t len = w * sizeof(gt_color_t);
    for (line = 0; line < h; line++) {
        gt_memcpy( &dst[ (line+area_dst.y) * area_dst.w + area_dst.x ],
                   &src[ (line+area_src.y) * area_src.w + area_src.x ],
                   len );
    }
}

void gt_color_fill(gt_color_t * color_arr, int len, gt_color_t color)
{
    int idx = 0;
    while( idx < len ){
        GT_COLOR_SET( color_arr[idx++], GT_COLOR_GET(color) );
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
