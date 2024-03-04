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


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/

gt_gc_st _gt_global_call = {
    0
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

uint16_t _gt_gc_get_size(void)
{
    return sizeof(gt_gc_st);
}

/* end ------------------------------------------------------------------*/
