/**
 * @file gt_draw_blend.h
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-08-10 19:43:01
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_DRAW_BLEND_H_
#define _GT_DRAW_BLEND_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../../core/gt_graph_base.h"
#include "../../hal/gt_hal_disp.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct _gt_draw_blend_dsc_s{
    gt_area_st  * dst_area;  /** dst area, draw temp cache */
    gt_color_t  * dst_buf;   /** blend dst color */

    gt_area_st  * mask_area; /** alpha data display area */
    gt_opa_t    * mask_buf;  /** opa / alpha buffer */
    gt_color_t  color_fill;  /*blend color*/

    gt_opt_t    opa;         /** value of the overall opacity */
}gt_draw_blend_dsc_st;

struct _gt_draw_ctx_t;
/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_draw_blend(struct _gt_draw_ctx_t * draw_ctx, const gt_draw_blend_dsc_st * dsc);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_DRAW_BLEND_H_
