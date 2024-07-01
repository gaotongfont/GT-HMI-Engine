/**
 * @file gt_draw_blend.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-08-10 19:43:10
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
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
#include "./gt_draw_blend_with_rgb565.h"
#include "./gt_draw_blend_with_argb888.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/
typedef struct _valid_param_s {
    struct _gt_draw_ctx_s * draw_ctx;
    gt_area_st * area_draw;
    gt_area_st * area_dsc;
    gt_area_st * area_intersect;
    gt_point_st * flush_buffer_offset;
}_valid_param_st;


/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
#if 0
static inline void _fill_no_opacity(
    gt_color_t * color_dst_p, gt_area_st const * const area_intersect,
    uint16_t step_dst_line, gt_color_t color_fill, gt_opa_t opa,
    gt_color_t const * const color_src_p, uint16_t src_width) {

    gt_color_t * src_p = (gt_color_t *)color_src_p;
    gt_color_t * dst_p = color_dst_p;
    uint16_t w = area_intersect->w;
    uint16_t h = area_intersect->h;
    uint16_t step_src_line = src_width - w;
    uint16_t x, y, width = w * sizeof(gt_color_t);

    if (opa < GT_OPA_MIN) {
        return ;
    }
    /** fill dsc->dst_buf use opa */
    if (src_p) {
        if (opa < GT_OPA_MAX) {
            for (y = 0; y < h; y++) {
#if GT_FLUSH_CONVERT_VERTICAL
                src_p = (gt_color_t * )color_src_p + y;
#endif
                for (x = 0; x < w; x++) {
                    *dst_p = gt_color_mix(*src_p, *dst_p, opa);
                    ++dst_p;
#if GT_FLUSH_CONVERT_VERTICAL
                    src_p += src_width;
#else
                    ++src_p;
#endif
                }
                dst_p += step_dst_line;
#if !GT_FLUSH_CONVERT_VERTICAL
                src_p += step_src_line;
#endif
            }
        } else {
            for (y = 0; y < h; y++) {
#if GT_FLUSH_CONVERT_VERTICAL
                src_p = (gt_color_t * )color_src_p + y;
#endif
                for (x = 0; x < w; x++) {
                    *dst_p = *src_p;
                    ++dst_p;
#if GT_FLUSH_CONVERT_VERTICAL
                    src_p += src_width;
#else
                    ++src_p;
#endif
                }
                dst_p += step_dst_line;
#if !GT_FLUSH_CONVERT_VERTICAL
                src_p += step_src_line;
#endif
            }
        }
        return;
    }

    // only fill color by opacity
    if (opa < GT_OPA_MAX) {
        for (y = 0; y < h; y++) {
            for (x = 0; x < w; x++) {
                *dst_p = gt_color_mix(color_fill, *dst_p, opa);
                ++dst_p;
            }
            dst_p += step_dst_line;
        }
        return;
    }

    // Direct color overlay, opa >= GT_OPA_MAX
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            *dst_p = color_fill;
            ++dst_p;
        }
        dst_p += step_dst_line;
    }
}

static inline void _fill_opacity(
    gt_color_t * color_dst_p, gt_area_st * area_intersect,
    uint16_t step_dst_line, gt_color_t color_fill, gt_opa_t opa, gt_opa_t const * const mask,
    gt_color_t const * const color_src_p, uint16_t src_width) {

    gt_color_t * src_p = (gt_color_t *)color_src_p;
    gt_opa_t * mask_p = (gt_opa_t *)mask;
    gt_color_t * dst_p = color_dst_p;
    uint16_t w = area_intersect->w;
    uint16_t h = area_intersect->h;
    uint16_t step_src_line = src_width - w;
    uint16_t x, y;

    gt_color_t dst_save_c = gt_color_black();
    gt_color_t src_save_c = gt_color_black();
    gt_color_t res_save_c = gt_color_black();
    gt_opa_t opa_save = GT_OPA_0;
    gt_opa_t opa_tmp = GT_OPA_0;

    if (src_p) {
        if (opa > GT_OPA_MAX) {
            /* be called too much */
            for (y = 0; y < h; y++) {
#if GT_FLUSH_CONVERT_VERTICAL
                mask_p = (gt_opa_t * )mask + y;
                src_p = (gt_color_t * )color_src_p + y;
#endif
                for (x = 0; x < w; x++) {
                    opa_tmp = *mask_p;
#if GT_FLUSH_CONVERT_VERTICAL
                    mask_p += src_width;
#else
                    ++mask_p;
#endif
                    if (opa_tmp > GT_OPA_MAX) {
                        *dst_p = *src_p;
#if GT_FLUSH_CONVERT_VERTICAL
                        src_p += src_width;
#else
                        ++src_p;
#endif
                        ++dst_p;
                        continue;
                    }
                    else if (opa_tmp < GT_OPA_MIN) {
#if GT_FLUSH_CONVERT_VERTICAL
                        src_p += src_width;
#else
                        ++src_p;
#endif
                        ++dst_p;
                        continue;
                    }
                    if (src_save_c.full != (*src_p).full ||
                        dst_save_c.full != (*dst_p).full ||
                        opa_save != opa_tmp) {
                        src_save_c = *src_p;
                        dst_save_c = *dst_p;
                        opa_save = opa_tmp;
                        res_save_c = gt_color_mix(*src_p, *dst_p, opa_tmp);
                    }
                    *dst_p = res_save_c;
                    ++dst_p;
#if GT_FLUSH_CONVERT_VERTICAL
                    src_p += src_width;
#else
                    ++src_p;
#endif
                }
                dst_p += step_dst_line;
#if !GT_FLUSH_CONVERT_VERTICAL
                src_p += step_src_line;
                mask_p += step_src_line;
#endif
            }
            return;
        }
        // opa < GT_OPA_MAX color mixed
        for (y = 0; y < h; y++) {
#if GT_FLUSH_CONVERT_VERTICAL
            mask_p = (gt_opa_t * )mask + y;
            src_p = (gt_color_t * )color_src_p + y;
#endif
            for (x = 0; x < w; x++) {
                opa_tmp = (gt_per_255(*mask_p) * opa) >> 15;
#if GT_FLUSH_CONVERT_VERTICAL
                mask_p += src_width;
#else
                ++mask_p;
#endif
                if (opa_tmp > GT_OPA_MAX) {
                    *dst_p = *src_p;
                    ++dst_p;
#if GT_FLUSH_CONVERT_VERTICAL
                    src_p += src_width;
#else
                    ++src_p;
#endif
                    continue;
                }
                else if (opa_tmp < GT_OPA_MIN) {
                    ++dst_p;
#if GT_FLUSH_CONVERT_VERTICAL
                    src_p += src_width;
#else
                    ++src_p;
#endif
                    continue;
                }
                if (src_save_c.full != (*src_p).full ||
                    dst_save_c.full != (*dst_p).full ||
                    opa_save != opa_tmp) {
                    src_save_c = *src_p;
                    dst_save_c = *dst_p;
                    opa_save = opa_tmp;
                    res_save_c = gt_color_mix(*src_p, *dst_p, opa_tmp);
                }
                *dst_p = res_save_c;
                ++dst_p;
#if GT_FLUSH_CONVERT_VERTICAL
                src_p += src_width;
#else
                ++src_p;
#endif
            }
            dst_p += step_dst_line;
#if !GT_FLUSH_CONVERT_VERTICAL
            src_p += step_src_line;
            mask_p += step_src_line;
#endif
        }
        return;
    }
    // draw circle, color_scr_p is NULL
    if (opa > GT_OPA_MAX) {
        for (y = 0; y < h; y++) {
#if GT_FLUSH_CONVERT_VERTICAL
            mask_p = (gt_opa_t * )mask + y;
#endif
            for (x = 0; x < w; x++) {
                opa_tmp = *mask_p;
#if GT_FLUSH_CONVERT_VERTICAL
                mask_p += src_width;
#else
                ++mask_p;
#endif
                if (opa_tmp > GT_OPA_MAX) {
                    *dst_p = color_fill;
                    ++dst_p;
                    continue;
                }
                else if (opa_tmp < GT_OPA_MIN) {
                    ++dst_p;
                    continue;
                }
                if (dst_save_c.full != (*dst_p).full ||
                    opa_save != opa_tmp) {
                    dst_save_c = *dst_p;
                    opa_save = opa_tmp;
                    res_save_c = gt_color_mix(color_fill, *dst_p, opa_tmp);
                }
                *dst_p = res_save_c;
                ++dst_p;
            }
            dst_p += step_dst_line;
#if !GT_FLUSH_CONVERT_VERTICAL
            mask_p += step_src_line;
#endif
        }
        return;
    }
    for (y = 0; y < h; y++) {
#if GT_FLUSH_CONVERT_VERTICAL
        mask_p = (gt_opa_t * )mask + y;
#endif
        for (x = 0; x < w; x++) {
            opa_tmp = (gt_per_255(*mask_p) * opa) >> 15;
#if GT_FLUSH_CONVERT_VERTICAL
            mask_p += src_width;
#else
            ++mask_p;
#endif
            if (opa_tmp > GT_OPA_MAX) {
                *dst_p = color_fill;
                ++dst_p;
                continue;
            }
            else if (opa_tmp < GT_OPA_MIN) {
                ++dst_p;
                continue;
            }
            if (dst_save_c.full != (*dst_p).full ||
                opa_save != opa_tmp) {
                dst_save_c = *dst_p;
                opa_save = opa_tmp;
                res_save_c = gt_color_mix(color_fill, *dst_p, opa_tmp);
            }
            *dst_p = res_save_c;
            ++dst_p;
        }
        dst_p += step_dst_line;
#if !GT_FLUSH_CONVERT_VERTICAL
        mask_p += step_src_line;
#endif
    }
}
#endif

static inline gt_point_st _get_cover_dst_area_and_offset_by(
    gt_area_st const * const parent_or_limit, gt_area_st const * const dst_area, gt_area_st * ret
) {
    gt_point_st offset = {0};

    gt_area_cover_screen(parent_or_limit, dst_area, ret);

    if (parent_or_limit->x == ret->x && dst_area->x < ret->x) {
        /** widget over left of parent display area */
        offset.x = ret->x - dst_area->x;
    }
    if (parent_or_limit->y == ret->y && dst_area->y < ret->y) {
        /** widget over top of parent display area */
        offset.y = ret->y - dst_area->y;
    }
    return offset;
}

/* global functions / API interface -------------------------------------*/
/**
 * @brief blend fore_color buf to back_color draw ctx
 *
 * @param draw_ctx draw content
 * @param dsc
 */
void gt_draw_blend(struct _gt_draw_ctx_s * draw_ctx, const gt_draw_blend_dsc_st * dsc)
{
    if (dsc->opa < GT_OPA_MIN) return;

    _gt_draw_blend_fill_cache_st fill_cache = {
        .color = dsc->color_fill,
        .opa = dsc->opa,
    };
    /** area_draw x/y screen offset; w/h need to redraw area, which pos by dst_area */
    gt_area_st area_draw = draw_ctx->buf_area, area_dst = *dsc->dst_area;

#if GT_FLUSH_CONVERT_VERTICAL
    fill_cache.width_src = area_dst.h;
    fill_cache.width_buf = area_draw.h;
#else
    fill_cache.width_src = area_dst.w;
    fill_cache.width_buf = area_draw.w;
#endif
    gt_point_st offset = {0};   /** src data buffer offset set */

    if (dsc->font_limit_area) {
        if (draw_ctx->parent_area) {
            gt_area_cover_screen(draw_ctx->parent_area, dsc->font_limit_area, &fill_cache.area_intersect);
            offset = _get_cover_dst_area_and_offset_by(&fill_cache.area_intersect, dsc->dst_area, &area_dst);
        } else {
            offset = _get_cover_dst_area_and_offset_by(dsc->font_limit_area, dsc->dst_area, &area_dst);
        }
    } else if (draw_ctx->parent_area) {
        /** Be used when obj->inside true and object has its parent */
        offset = _get_cover_dst_area_and_offset_by(draw_ctx->parent_area, dsc->dst_area, &area_dst);
    }

    /** 当前重绘制起始坐标 */
    gt_point_st flush_buffer_offset = {
        .x = area_dst.x < area_draw.x ? 0 : (area_dst.x - area_draw.x),
        .y = area_dst.y > area_draw.y ? (area_dst.y - area_draw.y) : 0,
    };

#if GT_USE_SCREEN_ANIM
    if (draw_ctx->valid && false == draw_ctx->valid->layer_top) {
        if (draw_ctx->valid->is_hor) {
            area_draw.w = draw_ctx->valid->area_clip.w;
        }
        flush_buffer_offset.x += draw_ctx->valid->area_clip.x;
        flush_buffer_offset.y += draw_ctx->valid->area_clip.y;
    }
#endif

    if (!gt_area_intersect_screen(&area_draw, &area_dst, &fill_cache.area_intersect)) {
        return;
    }

#if GT_FLUSH_CONVERT_VERTICAL
    gt_area_st convert = {
        .x = fill_cache.area_intersect.y,
        .y = fill_cache.area_intersect.x,
        .w = fill_cache.area_intersect.h,
        .h = fill_cache.area_intersect.w,
    };
    fill_cache.area_intersect = convert;

    gt_point_st convert_offset = {
        .x = flush_buffer_offset.y,
        .y = flush_buffer_offset.x,
    };
    flush_buffer_offset = convert_offset;

    gt_point_st convert_parent = {
        .x = offset.y,
        .y = offset.x,
    };
    offset = convert_parent;
#endif

    offset.x += fill_cache.area_intersect.x;
    offset.y += fill_cache.area_intersect.y;

    // from area_intersect cpy to flush_buffer_offset
    uint32_t idx_dst = flush_buffer_offset.y * fill_cache.width_buf + flush_buffer_offset.x;
#if GT_FLUSH_CONVERT_VERTICAL
    uint32_t idx_src = offset.x * fill_cache.width_src + offset.y;
#else
    uint32_t idx_src = offset.y * fill_cache.width_src + offset.x;
#endif

    if (0 == fill_cache.area_intersect.w || 0 == fill_cache.area_intersect.h) {
        return;
    }

    if (draw_ctx->buf) { fill_cache.color_dst_p = (gt_color_t * )draw_ctx->buf + idx_dst; }
    if (dsc->dst_buf)  { fill_cache.color_src_p = (gt_color_t * )dsc->dst_buf + idx_src; }
    if (dsc->mask_buf) { fill_cache.mask_buf_p = (gt_opa_t * )dsc->mask_buf + idx_src; }

    /**
     * use dsc->opa to blend background
     * -------------------------------------- <- area_draw
     * |                                    |
     * |      ---------------               |
     * |      |             |               |
     * |      |~~~~~~~~~~~~~| <- area_dst   |
     * |      |             |               |
     * |      |             |               |
     * |      |             |               |
     * |      ---------------               |
     * |                                    |
     * |                                    |
     * --------------------------------------
     *
     * 当前重绘制窗口, 相对显示屏的全区域每行相差的像素数目
     *  uint16_t step_dst_line = area_draw.w - area_intersect.w;
     *  uint16_t step_src_line = width_dsc - area_intersect.w;
     */
    if (dsc->mask_buf) {
#if 32 == GT_COLOR_DEPTH
        gt_draw_blend_with_argb888_opacity(&fill_cache);
#else
        gt_draw_blend_with_rgb565_opacity(&fill_cache);
#endif
    } else {
#if 32 == GT_COLOR_DEPTH
        gt_draw_blend_with_argb888_no_opacity(&fill_cache);
#else
        gt_draw_blend_with_rgb565_no_opacity(&fill_cache);
#endif
    }
}


void gt_draw_blend_text(struct _gt_draw_ctx_s * draw_ctx, const gt_draw_blend_dsc_st * dsc,
                        uint16_t font_size, uint16_t dot_byte, uint8_t gray, const uint8_t* res)
{
    if(!draw_ctx || !dsc || !res) return;

    if (dsc->opa < GT_OPA_MIN) return;

    /** 绘制区域和显示屏buffer的交集区域 */
    gt_area_st area_intersect = {0};
    /** buf_area: area_disp */
    gt_area_st area_draw = draw_ctx->buf_area, area_dst = *dsc->dst_area;

#if GT_FLUSH_CONVERT_VERTICAL
    uint16_t width_buf = area_draw.h;
#else
    uint16_t width_buf = area_draw.w;
#endif
    gt_point_st offset = {0};   /** src data buffer offset set */

    if (dsc->font_limit_area) {
        if (draw_ctx->parent_area) {
            gt_area_cover_screen(draw_ctx->parent_area, dsc->font_limit_area, &area_intersect);
            offset = _get_cover_dst_area_and_offset_by(&area_intersect, dsc->dst_area, &area_dst);
        } else {
            offset = _get_cover_dst_area_and_offset_by(dsc->font_limit_area, dsc->dst_area, &area_dst);
        }
    } else if (draw_ctx->parent_area) {
        /** Be used when obj->inside true and object has its parent */
        offset = _get_cover_dst_area_and_offset_by(draw_ctx->parent_area, dsc->dst_area, &area_dst);
    }

    /** 当前重绘制起始坐标 */
    gt_point_st flush_buffer_offset = {
        .x = area_dst.x < area_draw.x ? 0 : (area_dst.x - area_draw.x),
        .y = area_dst.y > area_draw.y ? (area_dst.y - area_draw.y) : 0,
    };

#if GT_USE_SCREEN_ANIM
    if (draw_ctx->valid && false == draw_ctx->valid->layer_top) {
        if (draw_ctx->valid->is_hor) {
            area_draw.w = draw_ctx->valid->area_clip.w;
        }
        flush_buffer_offset.x += draw_ctx->valid->area_clip.x;
        flush_buffer_offset.y += draw_ctx->valid->area_clip.y;
    }
#endif

    if (!gt_area_intersect_screen(&area_draw, &area_dst, &area_intersect)) {
        return;
    }

    if (0 == area_intersect.w || 0 == area_intersect.h) {
        return;
    }

#if GT_FLUSH_CONVERT_VERTICAL
    gt_area_st convert = {
        .x = area_intersect.y,
        .y = area_intersect.x,
        .w = area_intersect.h,
        .h = area_intersect.w,
    };
    area_intersect = convert;

    gt_point_st convert_offset = {
        .x = flush_buffer_offset.y,
        .y = flush_buffer_offset.x,
    };
    flush_buffer_offset = convert_offset;

    gt_point_st convert_parent = {
        .x = offset.y,
        .y = offset.x,
    };
    offset = convert_parent;
#endif

    gt_color_t * color_dst_p = (gt_color_t *)draw_ctx->buf;
    const uint8_t * color_src_p = res;

    offset.x += area_intersect.x;
    offset.y += area_intersect.y;

    // from area_intersect cpy to flush_buffer_offset
    uint32_t idx_dst = flush_buffer_offset.y * width_buf + flush_buffer_offset.x;
    uint32_t idx_src = offset.y * dot_byte;

    if (color_dst_p) { color_dst_p += idx_dst; }
    if (color_src_p) { color_src_p += idx_src; }

    uint16_t count = 0, tmp = 0;
    gt_size_t row, col;
    uint8_t ch = 0;

    if (2 == gray) {
        for (col = 0; col < area_intersect.h; col++) {
            count = 0;
#if GT_FLUSH_CONVERT_VERTICAL
            color_src_p = offset.x * dot_byte + res;
#endif
            for (row = offset.x; row < font_size; row++) {
                if (row >= offset.x + area_intersect.w) break;

#if GT_FLUSH_CONVERT_VERTICAL
                ch = ((color_src_p[col >> 2] >> (6 - ((col & 0x3)<<1))) & 0x03) * 255 / 3;
                color_src_p += dot_byte;
#else
                ch = ((color_src_p[row >> 2] >> (6 - ((row & 0x3)<<1))) & 0x03) * 255 / 3;
#endif
                tmp = (gt_per_255(ch) * dsc->opa) >> 15;
                color_dst_p[count] = gt_color_mix(dsc->color_fill, color_dst_p[count], tmp);
                ++count;
            }
#if !GT_FLUSH_CONVERT_VERTICAL
            color_src_p += dot_byte;
#endif
            color_dst_p += width_buf;
        }
    } else if (4 == gray) {
        for (col = 0; col < area_intersect.h; col++) {
            count = 0;
#if GT_FLUSH_CONVERT_VERTICAL
            color_src_p = offset.x * dot_byte + res;
#endif
            for (row = offset.x; row < font_size; row++) {
                if (row >= offset.x + area_intersect.w) break;

#if GT_FLUSH_CONVERT_VERTICAL
                ch = ((color_src_p[col >> 1] >> (4 - ((col & 0x1)<<2))) & 0x0F) * 255 / 15;
                color_src_p += dot_byte;
#else
                ch = ((color_src_p[row >> 1] >> (4 - ((row & 0x1)<<2))) & 0x0F) * 255 / 15;
#endif
                tmp = (gt_per_255(ch) * dsc->opa) >> 15;
                color_dst_p[count] = gt_color_mix(dsc->color_fill, color_dst_p[count], tmp);
                ++count;
            }
#if !GT_FLUSH_CONVERT_VERTICAL
            color_src_p += dot_byte;
#endif
            color_dst_p += width_buf;
        }
    } else if (3 == gray) {
        for (col = 0; col < area_intersect.h; col++) {
            count = 0;
#if GT_FLUSH_CONVERT_VERTICAL
            color_src_p = offset.x * dot_byte + res;
#endif
            for (row = offset.x; row < font_size; row++) {
                if(row >= offset.x + area_intersect.w) break;

#if GT_FLUSH_CONVERT_VERTICAL
                tmp = col * 3;
#else
                tmp = row * 3;
#endif
                if ((tmp & 0x7) <= 5) {
                    ch = ((color_src_p[tmp >> 3] >> (5 - (tmp & 0x7))) & 0x07) * 255 / 7;
                } else {
                    ch = ((((color_src_p[tmp >> 3]<<(tmp & 0x7))|(color_src_p[(tmp >> 3)+1]>>(8-(tmp & 0x7)))) >> 5) & 0x07) * 255 / 7;
                }
                tmp = (gt_per_255(ch) * dsc->opa) >> 15;
                color_dst_p[count] = gt_color_mix(dsc->color_fill, color_dst_p[count], tmp);

#if GT_FLUSH_CONVERT_VERTICAL
                color_src_p += dot_byte;
#endif
                ++count;
            }
#if !GT_FLUSH_CONVERT_VERTICAL
            color_src_p += dot_byte;
#endif
            color_dst_p += width_buf;
        }
    } else {
        for (col = 0; col < area_intersect.h; col++) {
            count = 0;
#if GT_FLUSH_CONVERT_VERTICAL
            color_src_p = offset.x * dot_byte + res;
#endif
            for (row = offset.x; row < font_size; row++) {
                if (row >= offset.x + area_intersect.w) break;
#if GT_FLUSH_CONVERT_VERTICAL
                ch = color_src_p[col >> 3] >> (7 - (col & 0x7));
                color_src_p += dot_byte;
#else
                ch = color_src_p[row >> 3] >> (7 - (row & 0x7));
#endif
                if (ch & 0x01) {
                    color_dst_p[count] = gt_color_mix(dsc->color_fill, color_dst_p[count], dsc->opa);
                }
                ++count;
            }
#if !GT_FLUSH_CONVERT_VERTICAL
            color_src_p += dot_byte;
#endif
            color_dst_p += width_buf;
        }
    }

}
/* end ------------------------------------------------------------------*/
