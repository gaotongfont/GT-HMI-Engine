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
#include "gt_draw_blend_dir.h"
#include "../../core/gt_mem.h"
#include "../../others/gt_math.h"
#include "../../others/gt_log.h"
#include "../../widgets/gt_obj.h"
#include "../../others/gt_area.h"
#include "../../core/gt_draw.h"
#include "../../core/gt_obj_pos.h"
#include "./gt_draw_blend_with_rgb565.h"
#include "./gt_draw_blend_with_argb888.h"

/* private define -------------------------------------------------------*/

#ifndef _GT_DRAW_BLEND_ATTRIBUTE_SRAM_TEXT
    /**
     * @brief Default: GT_ATTRIBUTE_SRAM_TEXT
     */
    #define _GT_DRAW_BLEND_ATTRIBUTE_SRAM_TEXT      GT_ATTRIBUTE_SRAM_TEXT
#endif


/* private typedef ------------------------------------------------------*/
typedef struct _valid_param_s {
    struct _gt_draw_ctx_s * draw_ctx;
    gt_area_st * area_draw;
    gt_area_st * area_dsc;
    gt_area_st * area_intersect;
    gt_point_st * flush_buffer_offset;
}_valid_param_st;

/** Context passed to each gray-depth pixel loop */
typedef struct _blend_text_ctx_s {
    gt_color_t *       color_dst_p;
    const uint8_t *    color_src_p;
    const uint8_t *    res;
    gt_area_st         area_intersect;
    gt_point_st        offset;
    uint16_t           width_buf;
    uint16_t           font_size;
    uint16_t           dot_byte;
    gt_color_t         fill_color;
    gt_opa_t           opa;
    uint32_t           opa_factor;
    int                is_vert;
    int                is_bold;
    int                is_italic;
    int                opa_full;
} _blend_text_ctx_st;


/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static inline gt_point_st _GT_DRAW_BLEND_ATTRIBUTE_SRAM_TEXT
_get_cover_dst_area_and_offset_by(
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

static inline void _blend_text_gray2(const _blend_text_ctx_st * c) {
    gt_color_t * color_dst_p = c->color_dst_p;
    const uint8_t * color_src_p = c->color_src_p;
    gt_size_t row, col;
    uint16_t count, start_count, tmp;
    uint8_t ch;
    gt_color_t tmp_color;

    if (c->is_vert) {
        for (col = c->offset.y; col < c->offset.y + c->area_intersect.h; col++) {
            start_count = c->is_italic ? (c->font_size - c->offset.y - col) >> 1 : 0;
            count = start_count;
            color_src_p = c->offset.x * c->dot_byte + c->res;
            for (row = c->offset.x; row < c->font_size; row++) {
                if ((row + start_count) >= c->offset.x + c->area_intersect.w) break;
                ch = ((color_src_p[col >> 2] >> (6 - ((col & 0x3)<<1))) & 0x03) * 85;
                color_src_p += c->dot_byte;
                tmp = c->opa_full ? ch : (uint16_t)((c->opa_factor * ch) >> 15);
                if (c->is_bold) {
                    if (start_count == count) {
                        tmp_color = gt_color_mix(c->fill_color, color_dst_p[count], tmp);
                        color_dst_p[count] = tmp_color;
                    } else {
                        gt_color_t this_color = color_dst_p[count];
                        color_dst_p[count] = gt_color_mix(c->fill_color, tmp_color, tmp);
                        tmp_color = gt_color_mix(c->fill_color, this_color, tmp);
                    }
                } else {
                    color_dst_p[count] = gt_color_mix(c->fill_color, color_dst_p[count], tmp);
                }
                ++count;
            }
            color_dst_p += c->width_buf;
        }
    } else {
        for (col = 0; col < c->area_intersect.h; col++) {
            start_count = c->is_italic ? (c->font_size - c->offset.y - col) >> 1 : 0;
            count = start_count;
            for (row = c->offset.x; row < c->font_size; row++) {
                if ((row + start_count) >= c->offset.x + c->area_intersect.w) break;
                ch = ((color_src_p[row >> 2] >> (6 - ((row & 0x3)<<1))) & 0x03) * 85;
                tmp = c->opa_full ? ch : (uint16_t)((c->opa_factor * ch) >> 15);
                if (c->is_bold) {
                    if (start_count == count) {
                        tmp_color = gt_color_mix(c->fill_color, color_dst_p[count], tmp);
                        color_dst_p[count] = tmp_color;
                    } else {
                        gt_color_t this_color = color_dst_p[count];
                        color_dst_p[count] = gt_color_mix(c->fill_color, tmp_color, tmp);
                        tmp_color = gt_color_mix(c->fill_color, this_color, tmp);
                    }
                } else {
                    color_dst_p[count] = gt_color_mix(c->fill_color, color_dst_p[count], tmp);
                }
                ++count;
            }
            color_src_p += c->dot_byte;
            color_dst_p += c->width_buf;
        }
    }
}

static inline void _blend_text_gray4(const _blend_text_ctx_st * c) {
    gt_color_t * color_dst_p = c->color_dst_p;
    const uint8_t * color_src_p = c->color_src_p;
    gt_size_t row, col;
    uint16_t count, start_count, tmp;
    uint8_t ch;
    gt_color_t tmp_color;

    if (c->is_vert) {
        for (col = c->offset.y; col < c->offset.y + c->area_intersect.h; col++) {
            start_count = c->is_italic ? (c->font_size - c->offset.y - col) >> 1 : 0;
            count = start_count;
            color_src_p = c->offset.x * c->dot_byte + c->res;
            for (row = c->offset.x; row < c->font_size; row++) {
                if ((row + start_count) >= c->offset.x + c->area_intersect.w) break;
                ch = ((color_src_p[col >> 1] >> (4 - ((col & 0x1)<<2))) & 0x0F) * 17;
                tmp = c->opa_full ? ch : (uint16_t)((c->opa_factor * ch) >> 15);
                if (c->is_bold) {
                    if (start_count == count) {
                        tmp_color = gt_color_mix(c->fill_color, color_dst_p[count], tmp);
                        color_dst_p[count] = tmp_color;
                    } else {
                        gt_color_t this_color = color_dst_p[count];
                        color_dst_p[count] = gt_color_mix(c->fill_color, tmp_color, tmp);
                        tmp_color = gt_color_mix(c->fill_color, this_color, tmp);
                    }
                } else {
                    color_dst_p[count] = gt_color_mix(c->fill_color, color_dst_p[count], tmp);
                }
                color_src_p += c->dot_byte;
                ++count;
            }
            color_dst_p += c->width_buf;
        }
    } else {
        for (col = 0; col < c->area_intersect.h; col++) {
            start_count = c->is_italic ? (c->font_size - c->offset.y - col) >> 1 : 0;
            count = start_count;
            for (row = c->offset.x; row < c->font_size; row++) {
                if ((row + start_count) >= c->offset.x + c->area_intersect.w) break;
                ch = ((color_src_p[row >> 1] >> (4 - ((row & 0x1)<<2))) & 0x0F) * 17;
                tmp = c->opa_full ? ch : (uint16_t)((c->opa_factor * ch) >> 15);
                if (c->is_bold) {
                    if (start_count == count) {
                        tmp_color = gt_color_mix(c->fill_color, color_dst_p[count], tmp);
                        color_dst_p[count] = tmp_color;
                    } else {
                        gt_color_t this_color = color_dst_p[count];
                        color_dst_p[count] = gt_color_mix(c->fill_color, tmp_color, tmp);
                        tmp_color = gt_color_mix(c->fill_color, this_color, tmp);
                    }
                } else {
                    color_dst_p[count] = gt_color_mix(c->fill_color, color_dst_p[count], tmp);
                }
                ++count;
            }
            color_src_p += c->dot_byte;
            color_dst_p += c->width_buf;
        }
    }
}

static inline void _blend_text_gray3(const _blend_text_ctx_st * c) {
    gt_color_t * color_dst_p = c->color_dst_p;
    const uint8_t * color_src_p = c->color_src_p;
    gt_size_t row, col;
    uint16_t count, start_count, tmp, tmp3;
    uint8_t ch;
    gt_color_t tmp_color;

    if (c->is_vert) {
        for (col = c->offset.y; col < c->offset.y + c->area_intersect.h; col++) {
            start_count = c->is_italic ? (c->font_size - c->offset.y - col) >> 1 : 0;
            count = start_count;
            color_src_p = c->offset.x * c->dot_byte + c->res;
            tmp3 = (uint16_t)col * 3;
            for (row = c->offset.x; row < c->font_size; row++) {
                if ((row + start_count) >= c->offset.x + c->area_intersect.w) break;
                if ((tmp3 & 0x7) <= 5) {
                    ch = ((color_src_p[tmp3 >> 3] >> (5 - (tmp3 & 0x7))) & 0x07) * 255 / 7;
                } else {
                    ch = ((((color_src_p[tmp3 >> 3]<<(tmp3 & 0x7))|(color_src_p[(tmp3>>3)+1]>>(8-(tmp3&0x7)))) >> 5) & 0x07) * 255 / 7;
                }
                tmp = c->opa_full ? ch : (uint16_t)((c->opa_factor * ch) >> 15);
                if (c->is_bold) {
                    if (start_count == count) {
                        tmp_color = gt_color_mix(c->fill_color, color_dst_p[count], tmp);
                        color_dst_p[count] = tmp_color;
                    } else {
                        gt_color_t this_color = color_dst_p[count];
                        color_dst_p[count] = gt_color_mix(c->fill_color, tmp_color, tmp);
                        tmp_color = gt_color_mix(c->fill_color, this_color, tmp);
                    }
                } else {
                    color_dst_p[count] = gt_color_mix(c->fill_color, color_dst_p[count], tmp);
                }
                color_src_p += c->dot_byte;
                ++count;
            }
            color_dst_p += c->width_buf;
        }
    } else {
        for (col = 0; col < c->area_intersect.h; col++) {
            start_count = c->is_italic ? (c->font_size - c->offset.y - col) >> 1 : 0;
            count = start_count;
            for (row = c->offset.x; row < c->font_size; row++) {
                if ((row + start_count) >= c->offset.x + c->area_intersect.w) break;
                tmp3 = (uint16_t)row * 3;
                if ((tmp3 & 0x7) <= 5) {
                    ch = ((color_src_p[tmp3 >> 3] >> (5 - (tmp3 & 0x7))) & 0x07) * 255 / 7;
                } else {
                    ch = ((((color_src_p[tmp3 >> 3]<<(tmp3 & 0x7))|(color_src_p[(tmp3>>3)+1]>>(8-(tmp3&0x7)))) >> 5) & 0x07) * 255 / 7;
                }
                tmp = c->opa_full ? ch : (uint16_t)((c->opa_factor * ch) >> 15);
                if (c->is_bold) {
                    if (start_count == count) {
                        tmp_color = gt_color_mix(c->fill_color, color_dst_p[count], tmp);
                        color_dst_p[count] = tmp_color;
                    } else {
                        gt_color_t this_color = color_dst_p[count];
                        color_dst_p[count] = gt_color_mix(c->fill_color, tmp_color, tmp);
                        tmp_color = gt_color_mix(c->fill_color, this_color, tmp);
                    }
                } else {
                    color_dst_p[count] = gt_color_mix(c->fill_color, color_dst_p[count], tmp);
                }
                ++count;
            }
            color_src_p += c->dot_byte;
            color_dst_p += c->width_buf;
        }
    }
}

static inline void _blend_text_gray1(const _blend_text_ctx_st * c) {
    gt_color_t * color_dst_p = c->color_dst_p;
    const uint8_t * color_src_p = c->color_src_p;
    gt_size_t row, col;
    uint16_t count, start_count;
    uint8_t ch;
    gt_color_t tmp_color;

    if (c->is_vert) {
        for (col = c->offset.y; col < c->offset.y + c->area_intersect.h; col++) {
            start_count = c->is_italic ? (c->font_size - c->offset.y - col) >> 1 : 0;
            count = start_count;
            color_src_p = c->offset.x * c->dot_byte + c->res;
            for (row = c->offset.x; row < c->font_size; row++) {
                if ((row + start_count) >= c->offset.x + c->area_intersect.w) break;
                ch = color_src_p[col >> 3] >> (7 - (col & 0x7));
                if (c->is_bold) {
                    if (ch & 0x01) {
                        tmp_color = gt_color_mix(c->fill_color, color_dst_p[count], c->opa);
                        color_dst_p[count] = tmp_color;
                        color_dst_p[count+1] = tmp_color;
                    }
                } else {
                    if (ch & 0x01) {
                        color_dst_p[count] = gt_color_mix(c->fill_color, color_dst_p[count], c->opa);
                    }
                }
                color_src_p += c->dot_byte;
                ++count;
            }
            color_dst_p += c->width_buf;
        }
    } else {
        for (col = 0; col < c->area_intersect.h; col++) {
            start_count = c->is_italic ? (c->font_size - c->offset.y - col) >> 1 : 0;
            count = start_count;
            for (row = c->offset.x; row < c->font_size; row++) {
                if ((row + start_count) >= c->offset.x + c->area_intersect.w) break;
                ch = color_src_p[row >> 3] >> (7 - (row & 0x7));
                if (c->is_bold) {
                    if (ch & 0x01) {
                        tmp_color = gt_color_mix(c->fill_color, color_dst_p[count], c->opa);
                        color_dst_p[count] = tmp_color;
                        color_dst_p[count+1] = tmp_color;
                    }
                } else {
                    if (ch & 0x01) {
                        color_dst_p[count] = gt_color_mix(c->fill_color, color_dst_p[count], c->opa);
                    }
                }
                ++count;
            }
            color_src_p += c->dot_byte;
            color_dst_p += c->width_buf;
        }
    }
}

/* global functions / API interface -------------------------------------*/
/**
 * @brief blend fore_color buf to back_color draw ctx
 *
 * @param draw_ctx draw content
 * @param dsc
 */
void _GT_DRAW_BLEND_ATTRIBUTE_SRAM_TEXT
gt_draw_blend(struct _gt_draw_ctx_s * draw_ctx, const gt_draw_blend_dsc_st * dsc)
{
    if (dsc->opa < GT_OPA_MIN) {
        return;
    }
    if (NULL == draw_ctx->buf) {
        return;
    }

    const int is_vert = gt_draw_blend_is_vertical();
    _gt_draw_blend_fill_cache_st fill_cache = {
        .color = dsc->color_fill,
        .opa = dsc->opa,
    };
    /** area_draw x/y screen offset; w/h need to redraw area, which pos by dst_area */
    gt_area_st area_draw = draw_ctx->buf_area, area_dst = *dsc->dst_area;

    if (is_vert) {
        fill_cache.width_src = area_dst.h;
        fill_cache.width_buf = area_draw.h;
    } else {
        fill_cache.width_src = area_dst.w;
        fill_cache.width_buf = area_draw.w;
    }
    gt_point_st offset = {0};   /** src data buffer offset set */

    if (dsc->view_area) {
        if (draw_ctx->parent_area) {
            gt_area_cover_screen(draw_ctx->parent_area, dsc->view_area, &fill_cache.area_intersect);
            offset = _get_cover_dst_area_and_offset_by(&fill_cache.area_intersect, dsc->dst_area, &area_dst);
        } else {
            offset = _get_cover_dst_area_and_offset_by(dsc->view_area, dsc->dst_area, &area_dst);
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

    if (is_vert) {
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
    }

    offset.x += fill_cache.area_intersect.x;
    offset.y += fill_cache.area_intersect.y;

    uint32_t idx_dst = flush_buffer_offset.y * fill_cache.width_buf + flush_buffer_offset.x;
    uint32_t idx_src = is_vert
        ? (offset.x * fill_cache.width_src + offset.y)
        : (offset.y * fill_cache.width_src + offset.x);

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


void _GT_DRAW_BLEND_ATTRIBUTE_SRAM_TEXT
gt_draw_blend_text(struct _gt_draw_ctx_s * draw_ctx, const gt_draw_blend_dsc_st * dsc,
                        uint16_t font_size, uint16_t dot_byte, uint8_t gray, const uint8_t* res, gt_font_style_st font_style)
{
    if(!draw_ctx || !dsc || !res) {
        return;
    }
    if (dsc->opa < GT_OPA_MIN) {
        return;
    }
    if (NULL == draw_ctx->buf) {
        return;
    }

    /** 绘制区域和显示屏buffer的交集区域 */
    const int is_vert = gt_draw_blend_is_vertical();
    gt_area_st area_intersect = {0};
    /** buf_area: area_disp */
    gt_area_st area_draw = draw_ctx->buf_area, dst_area = *dsc->dst_area;

    if(font_style.italic){
        dst_area.w += font_size >> 1;
    }

    gt_area_st area_dst = dst_area;

    uint16_t width_buf = is_vert ? area_draw.h : area_draw.w;
    gt_point_st offset = {0};   /** src data buffer offset set */

    if (dsc->view_area) {
        if (draw_ctx->parent_area) {
            gt_area_cover_screen(draw_ctx->parent_area, dsc->view_area, &area_intersect);
            offset = _get_cover_dst_area_and_offset_by(&area_intersect, &dst_area, &area_dst);
        } else {
            offset = _get_cover_dst_area_and_offset_by(dsc->view_area, &dst_area, &area_dst);
        }
    } else if (draw_ctx->parent_area) {
        /** Be used when obj->inside true and object has its parent */
        offset = _get_cover_dst_area_and_offset_by(draw_ctx->parent_area, &dst_area, &area_dst);
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

    gt_size_t col_clip = 0;
    if (is_vert) {
        gt_area_st convert = {
            .x = area_intersect.y,
            .y = area_intersect.x,
            .w = area_intersect.h,
            .h = area_intersect.w,
        };
        /* col_clip: how many columns of the character are hidden to the left
         * of the refresh area. Use the original dst_area.x (before view_area
         * clipping) so that partial-refresh cuts through a character are
         * handled correctly. */
        col_clip = area_draw.x > dst_area.x ? (area_draw.x - dst_area.x) : 0;
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
    }

    gt_color_t * color_dst_p = (gt_color_t *)draw_ctx->buf;
    const uint8_t * color_src_p = res;

    offset.x += area_intersect.x;
    if (is_vert) {
        /* col_clip: character-relative column offset for font data in vertical mode */
        offset.y = col_clip;
    } else {
        offset.y += area_intersect.y;
    }

    // from area_intersect cpy to flush_buffer_offset
    uint32_t idx_dst = flush_buffer_offset.y * width_buf + flush_buffer_offset.x;
    uint32_t idx_src = offset.y * dot_byte;

    if (color_dst_p) { color_dst_p += idx_dst; }
    if (color_src_p) { color_src_p += idx_src; }

    _blend_text_ctx_st ctx;
    ctx.color_dst_p  = color_dst_p;
    ctx.color_src_p  = color_src_p;
    ctx.res          = res;
    ctx.area_intersect = area_intersect;
    ctx.offset       = offset;
    ctx.width_buf    = width_buf;
    ctx.font_size    = font_size;
    ctx.dot_byte     = dot_byte;
    ctx.fill_color   = dsc->color_fill;
    ctx.opa          = dsc->opa;
    ctx.is_vert      = is_vert;
    ctx.is_bold      = font_style.bold;
    ctx.is_italic    = font_style.italic;
    ctx.opa_full     = (dsc->opa >= GT_OPA_MAX);
    ctx.opa_factor   = ctx.opa_full ? 0 : gt_per_255(dsc->opa);

    if      (2 == gray) { _blend_text_gray2(&ctx); }
    else if (4 == gray) { _blend_text_gray4(&ctx); }
    else if (3 == gray) { _blend_text_gray3(&ctx); }
    else                { _blend_text_gray1(&ctx); }
}
/* end ------------------------------------------------------------------*/
