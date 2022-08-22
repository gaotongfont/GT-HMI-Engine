/**
 * @file gt_draw_blend.h
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-08-10 19:43:01
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_DRAW_BLEND_H_
#define _GT_DRAW_BLEND_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdlib.h"
#include "stdbool.h"

#include "../../core/gt_graph_base.h"
#include "../../hal/gt_hal_disp.h"
#include "../../others/gt_math.h"
#include "../../others/gt_log.h"
#include "../../others/gt_effects.h"
#include "../../widgets/gt_obj.h"
#include "gt_draw_blend.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct _gt_draw_blend_dsc_s{
    gt_area_st * dst_area;
    gt_color_t * dst_buf;

    gt_opa_t * mask_buf;
    gt_color_t color_fill;

    gt_area_st * mask_area;
    gt_opt_t opa;
}gt_draw_blend_dsc_st;

struct _gt_draw_ctx_t;
/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_draw_blend(struct _gt_draw_ctx_t * draw_ctx, const gt_draw_blend_dsc_st * dsc);
void gt_draw_blend_basic(struct _gt_draw_ctx_t * draw_ctx, const gt_draw_blend_dsc_st * dsc);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_DRAW_BLEND_H_
