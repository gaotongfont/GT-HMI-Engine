/**
 * @file gt_draw_blend_dir.c
 * @author Feyoung
 * @brief Runtime draw direction control implementation.
 * @version 0.1
 * @date 2026-04-02
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_draw_blend_dir.h"
#include "../../gt_conf.h"

/* static variables -----------------------------------------------------*/
/** 0 = horizontal (default), 1 = vertical */
static int _gt_flush_vertical = 0;

/* global functions / API interface -------------------------------------*/
int gt_draw_blend_is_vertical(void)
{
    return _gt_flush_vertical;
}

void gt_draw_blend_set_vertical(int vertical)
{
    _gt_flush_vertical = vertical ? 1 : 0;
}

void gt_draw_blend_dir_init(void)
{
#if GT_FLUSH_CONVERT_VERTICAL
    _gt_flush_vertical = 1;
#else
    _gt_flush_vertical = 0;
#endif
}

/* end ------------------------------------------------------------------*/
