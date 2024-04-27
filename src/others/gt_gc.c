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

gt_gc_st _gt_global_call = {
    .scr_info = {
        .home_scr = {
            .id = -1,
            .screen = NULL,
            .alive = false,
        }
    },
#if GT_USE_EXTRA_FULL_IMG_BUFFER
    .full_img_buffer = {
        .color = NULL,
        .alpha = NULL,
        .len_color = 0,
        .len_alpha = 0,
    },
#endif
};

/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

gt_disp_st * _gt_gc_get_disp(void)
{
    return _gt_global_call.disp;
}

void _gt_gc_set_disp(gt_disp_st * disp)
{
    _gt_global_call.disp = disp;
}

_gt_gc_scr_st * _gt_gc_get_scr_info(void)
{
    return &_gt_global_call.scr_info;
}

uint16_t _gt_gc_get_size(void)
{
    return sizeof(gt_gc_st);
}

#if GT_USE_EXTRA_FULL_IMG_BUFFER
void gt_gc_set_full_img_buffer(gt_color_t * color, uint32_t len_color, gt_opa_t * alpha, uint32_t len_alpha)
{
    _gt_global_call.full_img_buffer.color = color;
    _gt_global_call.full_img_buffer.len_color = len_color;
    _gt_global_call.full_img_buffer.alpha = alpha;
    _gt_global_call.full_img_buffer.len_alpha = len_alpha;
}

_gt_gc_full_img_buffer_st * _gt_gc_get_full_img_buffer(void)
{
    return &_gt_global_call.full_img_buffer;
}
#endif

/* end ------------------------------------------------------------------*/
