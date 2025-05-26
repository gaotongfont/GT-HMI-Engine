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
static uint32_t _focus_color = GT_FOCUS_COLOR_SELECT;


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

void gt_color_focus_set(uint32_t color)
{
    _focus_color = color;
}

gt_color_t gt_gradient_color_get(gt_gradient_color_st gradient, uint16_t step)
{
    if(gradient.steps < 2) {
        return gradient.start;
    }

    if(step >= gradient.steps) {
        return gradient.end;
    }

#if 0
    gt_color_t color;
    float ratio = (float)step / (gradient.steps - 1);
    color.ch.red = gradient.start.ch.red + ratio * (gradient.end.ch.red - gradient.start.ch.red);
    color.ch.green = gradient.start.ch.green + ratio * (gradient.end.ch.green - gradient.start.ch.green);
    color.ch.blue = gradient.start.ch.blue + ratio * (gradient.end.ch.blue - gradient.start.ch.blue);
#else
    gt_color_t color;
    uint16_t ratio = step * GT_OPA_COVER / (gradient.steps - 1);
    color = gt_color_mix( gradient.end, gradient.start, ratio);
#endif

    return color;
}

void gt_gradient_color_get_by_vector( gt_color_t * color, _gt_vector_st* vector, uint16_t step)
{
    if(!color || !vector) {
        return ;
    }

    uint16_t count = _gt_vector_get_count(vector);
    uint16_t numb = 0;

    if(1 == count) {
        gt_gradient_color_st * gradient = (gt_gradient_color_st *)_gt_vector_get_item(vector, 0);
        if(!gradient) {
            return ;
        }
        *color = gt_gradient_color_get(*gradient, step);
        return ;
    }

    for(uint16_t i = 0; i < count; i++) {
        gt_gradient_color_st * gradient = (gt_gradient_color_st *)_gt_vector_get_item(vector, i);

        if(!gradient) {
            continue;
        }
        numb += gradient->steps;

        if(step < numb) {
            *color = gt_gradient_color_get(*gradient, (numb - step) % gradient->steps);
            return;
        }

        if((count - 1) == i) {
            step = step - numb;
            numb = 0;
            i = 0;
        }
    }
}

/* end ------------------------------------------------------------------*/
