/**
 * @file gt_draw_blend.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-08-10 19:43:10
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "stdlib.h"
#include "stdbool.h"

#include "../../core/gt_graph_base.h"
#include "../../hal/gt_hal_disp.h"
#include "../../core/gt_mem.h"
#include "../../others/gt_math.h"
#include "../../others/gt_log.h"
#include "../../others/gt_effects.h"
#include "../../widgets/gt_obj.h"
#include "gt_draw_blend.h"
#include "../../others/gt_area.h"
#include "../../core/gt_draw.h"
#include "../../core/gt_obj_pos.h"
#include "../../others/gt_area.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief blend fore_color buf to back_color draw ctx
 *
 * @param draw_ctx draw contentex
 * @param dsc
 */
void gt_draw_blend(gt_draw_ctx_t * draw_ctx, const gt_draw_blend_dsc_st * dsc)
{
    if( dsc->opa <= GT_OPA_MIN ) return;

    // calc intersect area
    gt_area_st area_res;
    if( !gt_area_intersect(draw_ctx->buf_area, dsc->dst_area, &area_res) ){
        return;
    }

    // if two area intersect
    gt_draw_blend_basic(draw_ctx, dsc);
}

void gt_draw_blend_basic(gt_draw_ctx_t * draw_ctx, const gt_draw_blend_dsc_st * dsc)
{
    bool mask = true;   // use mask blend
    if( dsc->mask_buf == NULL ){
        mask = false;
    }

	gt_area_st area_res;
    if( !gt_area_intersect(draw_ctx->buf_area, dsc->dst_area, &area_res) ){
        return;
    }
    if( area_res.w == 0 || area_res.h == 0 ){
        return;
    }
    gt_area_st area_draw, area_dsc;
    gt_area_copy(&area_draw, draw_ctx->buf_area);
    gt_area_copy(&area_dsc, dsc->dst_area);

    gt_area_st area_dst = {
        .x = area_dsc.x - area_draw.x,
        .y = 0,
        .w = area_draw.w,
        .h = area_draw.h
    }, area_src = {
        .x = area_res.x,
        .y = area_res.y,
        .w = area_res.w,
        .h = area_res.h
    };

    // GT_LOGD(GT_LOG_TAG_GUI ,"x:%d,y:%d,w:%d,h:%d\n", area_res.x, area_res.y, area_res.w, area_res.h);

    if( area_dsc.y > area_draw.y ){
        area_dst.y = area_dsc.y - area_draw.y;
    }
    if( area_dst.x < 0 ){
        area_dst.x = 0;
    }

    // from area_src cpy to area_dst
    int idx_dst = area_dst.y * area_dst.w + area_dst.x;
    int idx_src = area_src.y * area_src.w + area_src.x;
    int step_dst_line = area_dst.w - area_src.w;
    int step_src_line = area_dsc.w - area_src.w;

    gt_color_t * color_dst_p = draw_ctx->buf, * color_src_p = dsc->dst_buf;
    gt_opa_t opa_res;
	int x,y;
    // use dsc->opa to blend background
    if( !mask ){
        // fill dsc->dst_buf use opa
        if( dsc->dst_buf ){
            for(y = 0; y < area_src.h; y++){
                for(x = 0; x < area_src.w; x++){
                    gt_color_mix_with_alpha(
                        color_dst_p[idx_dst],   GT_OPA_COVER,
                        color_src_p[idx_src],   dsc->opa,
                        &color_dst_p[idx_dst],  &opa_res);
                    idx_dst++;
                    idx_src++;
                }
                idx_dst += step_dst_line;
                idx_src += step_src_line;
            }
        }
        // fill dsc->color_fill use opa 'rect'
        else{
            for(y = 0; y < area_src.h; y++){
                for(x = 0; x < area_src.w; x++){
                    gt_color_mix_with_alpha(
                        color_dst_p[idx_dst],   GT_OPA_COVER,
                        dsc->color_fill,        dsc->opa,
                        &color_dst_p[idx_dst],  &opa_res);
                    idx_dst++;
                    idx_src++;
                }
                idx_dst += step_dst_line;
                idx_src += step_src_line;
            }
        }
    }
    // use dsc->mask_buff to blend background
    else{
        // use dsc->mask_buf fill dsc->dst_buf to draw_ctx->buf
        if( NULL != dsc->dst_buf ){
            for(y = 0; y < area_src.h; y++){
                for(x = 0; x < area_src.w; x++){
                    gt_color_mix_with_alpha(
                        color_dst_p[idx_dst],   GT_OPA_COVER,
                        color_src_p[idx_src],   dsc->mask_buf[idx_src],
                        &color_dst_p[idx_dst],  &opa_res);
                    idx_dst++;
                    idx_src++;
                }
                idx_dst += step_dst_line;
                idx_src += step_src_line;
            }
        }
        // fill dsc->color_fill
        else{
            for(y = 0; y < area_src.h; y++){
                for(x = 0; x < area_src.w; x++){
                    gt_color_mix_with_alpha(
                        color_dst_p[idx_dst],   GT_OPA_COVER,
                        dsc->color_fill,        dsc->mask_buf[idx_src],
                        &color_dst_p[idx_dst],  &opa_res);
                    idx_dst++;
                    idx_src++;
                }
                idx_dst += step_dst_line;
                idx_src += step_src_line;
            }
        }
    }
}

/* end ------------------------------------------------------------------*/
