/**
 * @file gt_draw_blend_with_argb888.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-06-04 21:31:21
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_draw_blend_with_argb888.h"
#include "stddef.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
/**
 * Mix two colors. Both color can have alpha value.
 * @param bg_color background color
 * @param bg_opa alpha of the background color
 * @param fg_color foreground color
 * @param fg_opa alpha of the foreground color
 * @param res_color the result color
 * @param res_opa the result opacity
 */
static inline void gt_color_mix_with_alpha(gt_color_t bg_color, gt_opa_t bg_opa,
                                            gt_color_t fg_color, gt_opa_t fg_opa,
                                            gt_color_t * res_color, gt_opa_t * res_opa)
{
    /*Pick the foreground if it's fully opaque or the Background is fully transparent*/
    if(fg_opa >= GT_OPA_MAX || bg_opa <= GT_OPA_MIN) {
        res_color->full = fg_color.full;
        *res_opa = fg_opa;
        return ;
    }
    /*Transparent foreground: use the Background*/
    if(fg_opa <= GT_OPA_MIN) {
        res_color->full = bg_color.full;
        *res_opa = bg_opa;
        return ;
    }
    /*Opaque background: use simple mix*/
    if(bg_opa >= GT_OPA_MAX) {
        *res_color = gt_color_mix(fg_color, bg_color, fg_opa);
        *res_opa = GT_OPA_COVER;
        return;
    }

    /*Both colors have alpha. Expensive calculation need to be applied*/
    /*Save the parameters and the result. If they will be asked again don't compute again*/
    static gt_opa_t fg_opa_save       = 0;
    static gt_opa_t bg_opa_save       = 0;
    static gt_color_t fg_color_save   = _GT_COLOR_ZERO_INITIALIZER;
    static gt_color_t bg_color_save   = _GT_COLOR_ZERO_INITIALIZER;
    static gt_color_t res_color_saved = _GT_COLOR_ZERO_INITIALIZER;
    static gt_opa_t res_opa_saved     = 0;

    if (fg_opa != fg_opa_save || bg_opa != bg_opa_save ||
        fg_color.full != fg_color_save.full ||
        bg_color.full != bg_color_save.full) {
        fg_opa_save        = fg_opa;
        bg_opa_save        = bg_opa;
        fg_color_save.full = fg_color.full;
        bg_color_save.full = bg_color.full;
        /** Info: https://en.wikipedia.org/wiki/Alpha_compositing#Analytical_derivation_of_the_over_operator */
        res_opa_saved = 255 - ((uint16_t)((uint16_t)(255 - fg_opa) * (255 - bg_opa)) >> 8);
        GT_ASSERT_EXPR(res_opa_saved != 0);
        gt_opa_t ratio = (uint16_t)((uint16_t)fg_opa * 255) / res_opa_saved;
        res_color_saved = gt_color_mix(fg_color, bg_color, ratio);
    }

    res_color->full = res_color_saved.full;
    *res_opa = res_opa_saved;
}



#if GT_FLUSH_CONVERT_VERTICAL
static inline void _blend_argb888_with_opacity_vertical(_gt_draw_blend_fill_cache_st const * const fc) {
    gt_color_t * src_p = fc->color_src_p;
    gt_opa_t * mask_p = fc->mask_buf_p;
    gt_color_t * dst_p = fc->color_dst_p;
    uint16_t w = fc->area_intersect.w;
    uint16_t h = fc->area_intersect.h;
    uint16_t step_src_line = fc->width_src - w;
    uint16_t step_dst_line = fc->width_buf - w;
    uint16_t x, y;
    uint16_t src_width = fc->width_src;
    gt_opa_t opa = fc->opa;

    if (src_p) {
        if (opa > GT_OPA_MAX) {
            /* be called too much */
            for (y = 0; y < h; ++y) {
                mask_p = fc->mask_buf_p + y;
                src_p = fc->color_src_p + y;
                for (x = 0; x < w; ++x) {
                    *dst_p = gt_color_mix(*src_p, *dst_p, *mask_p);
                    ++dst_p;
                    src_p += src_width;
                    mask_p += src_width;
                }
                dst_p += step_dst_line;
            }
            return;
        }
        // opa < GT_OPA_MAX color mixed
        for (y = 0; y < h; ++y) {
            mask_p = fc->mask_buf_p + y;
            src_p = fc->color_src_p + y;
            for (x = 0; x < w; ++x) {
                *dst_p = gt_color_mix(*src_p, *dst_p, (gt_per_255(*mask_p) * opa) >> 15);
                ++dst_p;
                src_p += src_width;
                mask_p += src_width;
            }
            dst_p += step_dst_line;
        }
        return;
    }
    // draw circle, color_scr_p is NULL
    if (opa > GT_OPA_MAX) {
        for (y = 0; y < h; ++y) {
            mask_p = fc->mask_buf_p + y;
            for (x = 0; x < w; ++x) {
                *dst_p = gt_color_mix(fc->color, *dst_p, *mask_p);
                ++dst_p;
                mask_p += src_width;
            }
            dst_p += step_dst_line;
        }
        return;
    }
    for (y = 0; y < h; ++y) {
        mask_p = fc->mask_buf_p + y;
        for (x = 0; x < w; ++x) {
            *dst_p = gt_color_mix(fc->color, *dst_p, (gt_per_255(*mask_p) * opa) >> 15);
            ++dst_p;
            mask_p += src_width;
        }
        dst_p += step_dst_line;
    }
}
#else   /** 0 == GT_FLUSH_CONVERT_VERTICAL */
static inline void _blend_argb888_with_opacity(_gt_draw_blend_fill_cache_st const * const fc) {
    gt_color_t * src_p = fc->color_src_p;
    gt_opa_t * mask_p = fc->mask_buf_p;
    gt_color_t * dst_p = fc->color_dst_p;
    uint16_t w = fc->area_intersect.w;
    uint16_t h = fc->area_intersect.h;
    uint16_t step_src_line = fc->width_src - w;
    uint16_t step_dst_line = fc->width_buf - w;
    uint16_t x, y;
    gt_opa_t opa = fc->opa;

    if (src_p) {
        if (opa > GT_OPA_MAX) {
            /* be called too much */
            for (y = 0; y < h; ++y) {
                x = 0;
                if ((gt_uintptr_t)&dst_p[0] & 0x1) {
                    *dst_p = gt_color_mix(*src_p, *dst_p, *mask_p);
                    ++dst_p; ++src_p; ++mask_p;
                    ++x;
                }
                for (; x < w - 2; x += 2) {
                    *dst_p = gt_color_mix(*src_p, *dst_p, *mask_p);
                    dst_p[1] = gt_color_mix(src_p[1], dst_p[1], mask_p[1]);
                    dst_p += 2; src_p += 2; mask_p += 2;
                }

                for (; x < w; ++x) {
                    *dst_p = gt_color_mix(*src_p, *dst_p, *mask_p);
                    ++dst_p; ++src_p; ++mask_p;
                }
                dst_p += step_dst_line;
                src_p += step_src_line;
                mask_p += step_src_line;
            }
            return;
        }
        // opa < GT_OPA_MAX color mixed
        for (y = 0; y < h; ++y) {
            x = 0;
            if ((gt_uintptr_t)&dst_p[0] & 0x1) {
                *dst_p = gt_color_mix(*src_p, *dst_p, (gt_per_255(*mask_p) * opa) >> 15);
                ++dst_p; ++src_p; ++mask_p;
                ++x;
            }
            for (; x < w - 2; x += 2) {
                *dst_p = gt_color_mix(*src_p, *dst_p, (gt_per_255(*mask_p) * opa) >> 15);
                dst_p[1] = gt_color_mix(src_p[1], dst_p[1], (gt_per_255(mask_p[1]) * opa) >> 15);
                dst_p += 2; src_p += 2; mask_p += 2;
            }
            for (; x < w; ++x) {
                *dst_p = gt_color_mix(*src_p, *dst_p, (gt_per_255(*mask_p) * opa) >> 15);
                ++dst_p; ++src_p; ++mask_p;
            }
            dst_p += step_dst_line;
            src_p += step_src_line;
            mask_p += step_src_line;
        }
        return;
    }
    // draw circle, color_scr_p is NULL
    if (opa > GT_OPA_MAX) {
        for (y = 0; y < h; ++y) {
            x = 0;
            if ((gt_uintptr_t)&dst_p[0] & 0x1) {
                *dst_p = gt_color_mix(fc->color, *dst_p, *mask_p);
                ++dst_p;
                ++x;
            }
            for (; x < w - 2; x += 2) {
                *dst_p = gt_color_mix(fc->color, *dst_p, *mask_p);
                dst_p[1] = gt_color_mix(fc->color, dst_p[1], mask_p[1]);
                dst_p += 2; mask_p += 2;
            }
            for (; x < w; ++x) {
                *dst_p = gt_color_mix(fc->color, *dst_p, *mask_p);
                ++dst_p; ++mask_p;
            }
            dst_p += step_dst_line;
            mask_p += step_src_line;
        }
        return;
    }
    for (y = 0; y < h; ++y) {
        x = 0;
        if ((gt_uintptr_t)&dst_p[0] & 0x1) {
            *dst_p = gt_color_mix(fc->color, *dst_p, (gt_per_255(*mask_p) * opa) >> 15);
            ++dst_p; ++mask_p;
            ++x;
        }
        for (; x < w - 2; x += 2) {
            *dst_p = gt_color_mix(fc->color, *dst_p, (gt_per_255(*mask_p) * opa) >> 15);
            dst_p[1] = gt_color_mix(fc->color, dst_p[1], (gt_per_255(mask_p[1]) * opa) >> 15);
            dst_p += 2; mask_p += 2;
        }
        for (; x < w; ++x) {
            *dst_p = gt_color_mix(fc->color, *dst_p, (gt_per_255(*mask_p) * opa) >> 15);
            ++dst_p; ++mask_p;
        }
        dst_p += step_dst_line;
        mask_p += step_src_line;
    }
}
#endif  /** GT_FLUSH_CONVERT_VERTICAL */

#if GT_FLUSH_CONVERT_VERTICAL
static inline void _blend_argb888_with_no_opacity_vertical(_gt_draw_blend_fill_cache_st const * const fc) {
    gt_color_t * src_p = fc->color_src_p;
    gt_color_t * dst_p = fc->color_dst_p;
    uint16_t w = fc->area_intersect.w;
    uint16_t h = fc->area_intersect.h;
    uint16_t step_src_line = fc->width_src - w;
    uint16_t step_dst_line = fc->width_buf - w;
    uint16_t x, y, width = w * sizeof(gt_color_t);
    uint16_t src_width = fc->width_src;
    gt_opa_t opa = fc->opa;

    if (opa < GT_OPA_MIN) {
        return ;
    }
    /** fill dsc->dst_buf use opa */
    if (src_p) {
        if (opa < GT_OPA_MAX) {
            for (y = 0; y < h; ++y) {
                src_p = fc->color_src_p + y;
                for (x = 0; x < w; ++x) {
                    *dst_p = gt_color_mix(*src_p, *dst_p, opa);
                    ++dst_p;
                    src_p += src_width;
                }
                dst_p += step_dst_line;
            }
            return;
        }
        for (y = 0; y < h; ++y) {
            src_p = fc->color_src_p + y;
            for (x = 0; x < w; ++x) {
                *dst_p = *src_p;
                ++dst_p;
                src_p += src_width;
            }
            dst_p += step_dst_line;
        }
        return;
    }

    // only fill color by opacity
    if (opa < GT_OPA_MAX) {
        for (y = 0; y < h; ++y) {
            for (x = 0; x < w; ++x) {
                *dst_p = gt_color_mix(fc->color, *dst_p, opa);
                ++dst_p;
            }
            dst_p += step_dst_line;
        }
        return;
    }

#if 16 == GT_COLOR_DEPTH
    uint32_t * ptr32 = NULL;
    uint32_t color32 = (fc->color.full << 16) | fc->color.full;
#endif
    // Direct color overlay, opa >= GT_OPA_MAX
    for (y = 0; y < h; ++y) {
        x = 0;
        if ((gt_uintptr_t)dst_p & 0x1) {
            *dst_p = fc->color;
            ++dst_p;
        }
        for (; x < w - 2; x += 2) {
#if 16 == GT_COLOR_DEPTH
            ptr32 = (uint32_t *)dst_p;
            ptr32[0] = color32;
            dst_p += 2;
#else
            *dst_p = fc->color;
            ++dst_p;
            *dst_p = fc->color;
            ++dst_p;
#endif
        }
        for (; x < w; ++x) {
            *dst_p = fc->color;
            ++dst_p;
        }
        dst_p += step_dst_line;
    }
}
#else   /** 0 == GT_FLUSH_CONVERT_VERTICAL */
static inline void _blend_argb888_with_no_opacity(_gt_draw_blend_fill_cache_st const * const fc) {
    gt_color_t * src_p = fc->color_src_p;
    gt_color_t * dst_p = fc->color_dst_p;
    uint16_t w = fc->area_intersect.w;
    uint16_t h = fc->area_intersect.h;
    uint16_t step_src_line = fc->width_src - w;
    uint16_t step_dst_line = fc->width_buf - w;
    uint16_t x, y, width = w * sizeof(gt_color_t);
    gt_opa_t opa = fc->opa;

    if (opa < GT_OPA_MIN) {
        return ;
    }
    /** fill dsc->dst_buf use opa */
    if (src_p) {
        if (opa < GT_OPA_MAX) {
            for (y = 0; y < h; ++y) {
                x = 0;
                if ((gt_uintptr_t)&dst_p[0] & 0x1) {
                    *dst_p = gt_color_mix(*src_p, *dst_p, opa);
                    ++dst_p; ++src_p;
                    ++x;
                }
                for (; x < w - 2; x += 2) {
                    *dst_p = gt_color_mix(*src_p, *dst_p, opa);
                    dst_p[1] = gt_color_mix(src_p[1], dst_p[1], opa);
                    dst_p += 2; src_p += 2;
                }
                for (; x < w; ++x) {
                    *dst_p = gt_color_mix(*src_p, *dst_p, opa);
                    ++dst_p; ++src_p;
                }
                dst_p += step_dst_line;
                src_p += step_src_line;
            }
            return;
        }
        for (y = 0; y < h; ++y) {
            x = 0;
            if ((gt_uintptr_t)&dst_p[0] & 0x1) {
                *dst_p = *src_p;
                ++dst_p; ++src_p;
                ++x;
            }
            for (; x < w - 2; x += 2) {
                *dst_p = *src_p;
                dst_p[1] = src_p[1];
                dst_p += 2; src_p += 2;
            }
            for (; x < w; ++x) {
                *dst_p = *src_p;
                ++dst_p; ++src_p;
            }
            dst_p += step_dst_line;
            src_p += step_src_line;
        }
        return;
    }

    // only fill color by opacity
    if (opa < GT_OPA_MAX) {
        for (y = 0; y < h; ++y) {
            x = 0;
            if ((gt_uintptr_t)&dst_p[0] & 0x1) {
                *dst_p = gt_color_mix(fc->color, *dst_p, opa);
                ++dst_p;
                ++x;
            }
            for (; x < w - 2; x += 2) {
                *dst_p = gt_color_mix(fc->color, *dst_p, opa);
                dst_p[1] = gt_color_mix(fc->color, dst_p[1], opa);
                dst_p += 2;
            }
            for (; x < w; ++x) {
                *dst_p = gt_color_mix(fc->color, *dst_p, opa);
                ++dst_p;
            }
            dst_p += step_dst_line;
        }
        return;
    }
#if 16 == GT_COLOR_DEPTH
    uint32_t * ptr32 = NULL;
    uint32_t color32 = (fc->color.full << 16) | fc->color.full;
#endif
    // Direct color overlay, opa >= GT_OPA_MAX
    for (y = 0; y < h; ++y) {
        x = 0;
        if ((gt_uintptr_t)&dst_p[0] & 0x1) {
            *dst_p = fc->color;
            ++dst_p;
            ++x;
        }
        for (; x < w - 2; x += 2) {
#if 16 == GT_COLOR_DEPTH
            ptr32 = (uint32_t *)dst_p;
            ptr32[0] = color32;
            dst_p += 2;
#else
            *dst_p = fc->color; ++dst_p;
            *dst_p = fc->color; ++dst_p;
#endif
        }
        for (; x < w; ++x) {
            *dst_p = fc->color;
            ++dst_p;
        }
        dst_p += step_dst_line;
    }
}
#endif  /** GT_FLUSH_CONVERT_VERTICAL */

/* global functions / API interface -------------------------------------*/
void gt_draw_blend_with_argb888_opacity(_gt_draw_blend_fill_cache_st const * const fc)
{
#if GT_FLUSH_CONVERT_VERTICAL
    return _blend_argb888_with_opacity_vertical(fc);
#else
    return _blend_argb888_with_opacity(fc);
#endif
}

void gt_draw_blend_with_argb888_no_opacity(_gt_draw_blend_fill_cache_st const * const fc)
{
#if GT_FLUSH_CONVERT_VERTICAL
    return _blend_argb888_with_no_opacity_vertical(fc);
#else
    return _blend_argb888_with_no_opacity(fc);
#endif
}

/* end ------------------------------------------------------------------*/
