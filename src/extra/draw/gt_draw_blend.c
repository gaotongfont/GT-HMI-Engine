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

#include "gt_draw_blend.h"
#include "../../core/gt_mem.h"
#include "../../others/gt_math.h"
#include "../../others/gt_log.h"
#include "../../others/gt_effects.h"
#include "../../widgets/gt_obj.h"
#include "../../others/gt_area.h"
#include "../../core/gt_draw.h"
#include "../../core/gt_obj_pos.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief blend fore_color buf to back_color draw ctx
 *
 * @param draw_ctx draw content
 * @param dsc
 */
void gt_draw_blend(struct _gt_draw_ctx_t * draw_ctx, const gt_draw_blend_dsc_st * dsc)
{
    if (dsc->opa <= GT_OPA_MIN) return;

    // if two area intersect
    bool mask = dsc->mask_buf ? true : false;   // use mask blend

    /** 绘制区域和显示屏buffer的交集区域 */
    gt_area_st area_intersect = { 0, 0, 0, 0 };
    gt_area_st area_draw, area_dsc;

    /** 显示屏的显示区域 */
    gt_area_copy(&area_draw, &draw_ctx->buf_area);
    /** 图片当前处理的显示区域 */
    gt_area_copy(&area_dsc, dsc->dst_area);

    /** 当前重绘制起始坐标 */
    gt_point_st area_dst = {
        .x = area_dsc.x < area_draw.x ? 0 : (area_dsc.x - area_draw.x),
        .y = area_dsc.y > area_draw.y ? (area_dsc.y - area_draw.y) : 0,
    };

    if (draw_ctx->valid) {
        /** 界面切换动画绘制区 */
        gt_area_st area_valid;
        if (draw_ctx->valid->is_prev) {
            /** prev screen display */
            gt_area_copy(&area_valid, &draw_ctx->valid->area_prev);
            if (draw_ctx->valid->is_hor) {
                if (draw_ctx->valid->offset_prev.x && area_dst.x < draw_ctx->valid->offset_prev.x) {
                    area_valid.x += draw_ctx->valid->offset_prev.x;
                    area_valid.w -= draw_ctx->valid->offset_prev.x;
                    area_dst.x = draw_ctx->valid->offset_prev.x;
                }
            } else {
                if (draw_ctx->valid->offset_prev.y && area_dst.y < draw_ctx->valid->offset_prev.y) {
                    area_dst.y = draw_ctx->valid->offset_prev.y;
                }
            }
        } else {
            /** new screen display */
            gt_area_copy(&area_valid, &draw_ctx->valid->area_scr);
            if (draw_ctx->valid->is_hor) {
                area_valid.x += draw_ctx->valid->offset_scr.x;
                if (draw_ctx->valid->offset_scr.x && area_dst.x < draw_ctx->valid->offset_scr.x) {
                    area_dst.x = draw_ctx->valid->offset_scr.x;
                }
            } else {
                area_valid.y -= draw_ctx->valid->offset_scr.y;
                if (draw_ctx->valid->offset_scr.y && area_dst.y < draw_ctx->valid->offset_scr.y) {
                    area_dst.y = draw_ctx->valid->offset_scr.y;
                }
            }
        }
        if (!gt_area_intersect_screen(&area_valid, dsc->dst_area, &area_intersect)) {
            return;
        }
    } else {
        /** draw_ctx->buf_area: 图片显示区域; dsc->dst_area: 当前绘制区域 */
        if (!gt_area_intersect_screen(&draw_ctx->buf_area, dsc->dst_area, &area_intersect)) {
            return;
        }
    }
    if (area_intersect.w == 0 || area_intersect.h == 0) {
        return;
    }

    // from area_intersect cpy to area_dst
    uint32_t idx_dst = area_dst.y * area_draw.w + area_dst.x;
    uint32_t idx_src = area_intersect.y * area_dsc.w + area_intersect.x;

    /** 当前重绘制窗口, 相对显示屏的全区域每行相差的像素数目 */
    uint16_t step_dst_line = area_draw.w - area_intersect.w;
    uint16_t step_src_line = area_dsc.w - area_intersect.w;

    gt_color_t * color_dst_p = draw_ctx->buf, * color_src_p = dsc->dst_buf;
    gt_opa_t opa_res;
    uint16_t x, y;
    /**
     * use dsc->opa to blend background
     * -------------------------------------- <- area_draw
     * |                                    |
     * |      ---------------               |
     * |      |             |               |
     * |      |~~~~~~~~~~~~~| <- area_dsc   |
     * |      |             |               |
     * |      |             |               |
     * |      |             |               |
     * |      ---------------               |
     * |                                    |
     * |                                    |
     * --------------------------------------
     *
     */
    if (!mask) {
        // fill dsc->dst_buf use opa
        if (dsc->dst_buf) {
            for (y = 0; y < area_intersect.h; y++) {
                for (x = 0; x < area_intersect.w; x++) {
                    gt_color_mix_with_alpha(
                        color_dst_p[idx_dst], GT_OPA_COVER,
                        color_src_p[idx_src], dsc->opa,
                        &color_dst_p[idx_dst], &opa_res);
                    idx_dst++;
                    idx_src++;
                }
                idx_dst += step_dst_line;
                idx_src += step_src_line;
            }
            return ;
        }
        // fill dsc->color_fill use opa 'rect'
        for (y = 0; y < area_intersect.h; y++) {
            for (x = 0; x < area_intersect.w; x++) {
                gt_color_mix_with_alpha(
                    color_dst_p[idx_dst], GT_OPA_COVER,
                    dsc->color_fill, dsc->opa,
                    &color_dst_p[idx_dst], &opa_res);
                idx_dst++;
            }
            idx_dst += step_dst_line;
        }
        return;
    }

    // use dsc->mask_buf to blend background
    // use dsc->mask_buf fill dsc->dst_buf to draw_ctx->buf
    if (dsc->dst_buf) {
        if (dsc->opa == GT_OPA_COVER) {
            for (y = 0; y < area_intersect.h; y++) {
                for (x = 0; x < area_intersect.w; x++) {
                    gt_color_mix_with_alpha(
                        color_dst_p[idx_dst], GT_OPA_COVER,
                        color_src_p[idx_src], dsc->mask_buf[idx_src],
                        &color_dst_p[idx_dst], &opa_res);
                    idx_dst++;
                    idx_src++;
                }
                idx_dst += step_dst_line;
                idx_src += step_src_line;
            }
            return ;
        }
        for (y = 0; y < area_intersect.h; y++) {
            for (x = 0; x < area_intersect.w; x++) {
                gt_color_mix_with_alpha(
                    color_dst_p[idx_dst], GT_OPA_COVER,
                    color_src_p[idx_src], ((gt_per_255(dsc->mask_buf[idx_src]) * dsc->opa) >> 15),
                    &color_dst_p[idx_dst], &opa_res);
                idx_dst++;
                idx_src++;
            }
            idx_dst += step_dst_line;
            idx_src += step_src_line;
        }
        return ;
    }
    // fill dsc->color_fill
    if (dsc->opa == GT_OPA_COVER) {
        for (y = 0; y < area_intersect.h; y++) {
            for (x = 0; x < area_intersect.w; x++) {
                gt_color_mix_with_alpha(
                    color_dst_p[idx_dst], GT_OPA_COVER,
                    dsc->color_fill, dsc->mask_buf[idx_src],
                    &color_dst_p[idx_dst], &opa_res);
                idx_dst++;
                idx_src++;
            }
            idx_dst += step_dst_line;
            idx_src += step_src_line;
        }
        return ;
    }
    for (y = 0; y < area_intersect.h; y++) {
        for (x = 0; x < area_intersect.w; x++) {
            gt_color_mix_with_alpha(
                color_dst_p[idx_dst], GT_OPA_COVER,
                dsc->color_fill, ((gt_per_255(dsc->mask_buf[idx_src]) * dsc->opa) >> 15),
                &color_dst_p[idx_dst], &opa_res);
            idx_dst++;
            idx_src++;
        }
        idx_dst += step_dst_line;
        idx_src += step_src_line;
    }
}

/* end ------------------------------------------------------------------*/
