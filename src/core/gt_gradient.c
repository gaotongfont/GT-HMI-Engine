/**
 * @file gt_gradient.c
 * @brief Gradient and shadow descriptor implementation.
 * @version 0.1
 * @date 2026-04-08
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_gradient.h"
#include "gt_mem.h"
#include "../others/gt_log.h"

/* private define -------------------------------------------------------*/
#define GT_LOG_TAG_GRAD     "grad"

/* static functions -----------------------------------------------------*/

#if GT_USE_GRADIENT

static bool _gradient_stop_free_cb(void * item) {
    if (item) { gt_mem_free(item); }
    return true;
}

/* global functions / API interface -------------------------------------*/

gt_gradient_dsc_st * gt_gradient_dsc_create(gt_grad_dir_et dir)
{
    gt_gradient_dsc_st * dsc = (gt_gradient_dsc_st *)gt_mem_malloc(sizeof(gt_gradient_dsc_st));
    if (!dsc) {
        GT_LOGW(GT_LOG_TAG_GRAD, "OOM: gradient dsc");
        return NULL;
    }
    dsc->stops = _gt_vector_create(_gradient_stop_free_cb, NULL);
    if (!dsc->stops) {
        gt_mem_free(dsc);
        return NULL;
    }
    dsc->dir  = dir;
    dsc->loop = 0;
    return dsc;
}

void gt_gradient_dsc_destroy(gt_gradient_dsc_st * dsc)
{
    if (!dsc) return;
    if (dsc->stops) { _gt_vector_free(dsc->stops); }
    gt_mem_free(dsc);
}

void gt_gradient_dsc_add_stop(gt_gradient_dsc_st * dsc,
                               gt_color_t start, gt_color_t end,
                               uint16_t steps)
{
    if (!dsc || !dsc->stops) return;
    if (steps < 2) steps = 2;

    gt_gradient_color_st * seg = (gt_gradient_color_st *)gt_mem_malloc(sizeof(gt_gradient_color_st));
    if (!seg) {
        GT_LOGW(GT_LOG_TAG_GRAD, "OOM: gradient stop");
        return;
    }
    seg->start = start;
    seg->end   = end;
    seg->steps = steps;
    _gt_vector_add_item(dsc->stops, seg);
}

void gt_gradient_dsc_set_loop(gt_gradient_dsc_st * dsc, uint8_t loop)
{
    if (!dsc) return;
    dsc->loop = loop ? 1 : 0;
}

gt_gradient_dsc_st * gt_gradient_dsc_rainbow(gt_grad_dir_et dir,
                                              uint16_t steps_per_seg)
{
    if (steps_per_seg < 2) steps_per_seg = 20;

    gt_gradient_dsc_st * dsc = gt_gradient_dsc_create(dir);
    if (!dsc) return NULL;

    /* 6-segment HSV cycle: R→Y→G→C→B→M→R */
    static const uint32_t _rainbow[7] = {
        0xFF0000, /* Red     */
        0xFF8000, /* Orange  */
        0xFFFF00, /* Yellow  */
        0x00FF00, /* Green   */
        0x0000FF, /* Blue    */
        0x8000FF, /* Violet  */
        0xFF0000, /* Red (wrap) */
    };
    for (int i = 0; i < 6; ++i) {
        gt_gradient_dsc_add_stop(dsc,
            gt_color_hex(_rainbow[i]),
            gt_color_hex(_rainbow[i + 1]),
            steps_per_seg);
    }
    dsc->loop = 1;
    return dsc;
}

uint16_t gt_gradient_dsc_total_steps(const gt_gradient_dsc_st * dsc)
{
    if (!dsc || !dsc->stops) return 0;
    gt_size_t cnt = _gt_vector_get_count(dsc->stops);
    uint16_t total = 0;
    for (gt_size_t i = 0; i < cnt; ++i) {
        gt_gradient_color_st * seg = (gt_gradient_color_st *)_gt_vector_get_item(dsc->stops, i);
        if (seg) total += seg->steps;
    }
    return total;
}

gt_color_t gt_gradient_dsc_get_color(const gt_gradient_dsc_st * dsc,
                                      uint16_t step)
{
    if (!dsc || !dsc->stops) return gt_color_black();

    uint16_t total = gt_gradient_dsc_total_steps(dsc);
    if (total == 0) return gt_color_black();

    if (dsc->loop) {
        step = step % total;
    } else {
        if (step >= total) step = total - 1;
    }

    /* Walk segments to find which one owns this step */
    gt_size_t cnt = _gt_vector_get_count(dsc->stops);
    uint16_t offset = 0;
    for (gt_size_t i = 0; i < cnt; ++i) {
        gt_gradient_color_st * seg = (gt_gradient_color_st *)_gt_vector_get_item(dsc->stops, i);
        if (!seg) continue;
        if (step < offset + seg->steps) {
            gt_gradient_color_st local = *seg;
            return gt_gradient_color_get(local, step - offset);
        }
        offset += seg->steps;
    }
    /* Fallback: last color of last segment */
    gt_gradient_color_st * last = (gt_gradient_color_st *)_gt_vector_get_item(dsc->stops, cnt - 1);
    return last ? last->end : gt_color_black();
}

gt_color_t gt_gradient_dsc_get_color_pos(const gt_gradient_dsc_st * dsc,
                                          uint16_t pos)
{
    if (!dsc || !dsc->stops) return gt_color_black();

    gt_size_t cnt = _gt_vector_get_count(dsc->stops);
    if (cnt == 0) return gt_color_black();

    /* Total weight = sum of all segment steps */
    uint32_t total_w = 0;
    for (gt_size_t i = 0; i < cnt; ++i) {
        gt_gradient_color_st * seg = (gt_gradient_color_st *)_gt_vector_get_item(dsc->stops, i);
        if (seg) total_w += seg->steps;
    }
    if (total_w == 0) return gt_color_black();

    /* Map pos [0..65535] → scaled position [0..total_w * 256) */
    /* scaled = pos * total_w / 65535, keep 8 fractional bits */
    uint32_t scaled = ((uint32_t)pos * total_w * 256u) / 65535u;

    if (dsc->loop) {
        scaled = scaled % (total_w * 256u);
    } else {
        uint32_t max_s = total_w * 256u - 1u;
        if (scaled > max_s) scaled = max_s;
    }

    /* Walk segments to find which one owns this position */
    uint32_t seg_start = 0;
    for (gt_size_t i = 0; i < cnt; ++i) {
        gt_gradient_color_st * seg = (gt_gradient_color_st *)_gt_vector_get_item(dsc->stops, i);
        if (!seg) continue;
        uint32_t seg_end = seg_start + (uint32_t)seg->steps * 256u;
        if (scaled < seg_end) {
            /* mix ratio within this segment: 0..255 */
            uint32_t local = scaled - seg_start;          /* 0 .. seg->steps*256-1 */
            uint8_t  mix   = (uint8_t)(local * 255u / ((uint32_t)seg->steps * 256u - 1u));
            return gt_color_mix(seg->end, seg->start, mix);
        }
        seg_start = seg_end;
    }
    /* Clamp to last color */
    gt_gradient_color_st * last = (gt_gradient_color_st *)_gt_vector_get_item(dsc->stops, cnt - 1);
    return last ? last->end : gt_color_black();
}

uint32_t gt_gradient_dsc_get_color24_pos(const gt_gradient_dsc_st * dsc,
                                          uint16_t pos)
{
    if (!dsc || !dsc->stops) return 0;

    gt_size_t cnt = _gt_vector_get_count(dsc->stops);
    if (cnt == 0) return 0;

    uint32_t total_w = 0;
    for (gt_size_t i = 0; i < cnt; ++i) {
        gt_gradient_color_st * seg = (gt_gradient_color_st *)_gt_vector_get_item(dsc->stops, i);
        if (seg) total_w += seg->steps;
    }
    if (total_w == 0) return 0;

    uint32_t scaled = ((uint32_t)pos * total_w * 256u) / 65535u;
    if (dsc->loop) {
        scaled = scaled % (total_w * 256u);
    } else {
        uint32_t max_s = total_w * 256u - 1u;
        if (scaled > max_s) scaled = max_s;
    }

    uint32_t seg_start = 0;
    for (gt_size_t i = 0; i < cnt; ++i) {
        gt_gradient_color_st * seg = (gt_gradient_color_st *)_gt_vector_get_item(dsc->stops, i);
        if (!seg) continue;
        uint32_t seg_end = seg_start + (uint32_t)seg->steps * 256u;
        if (scaled < seg_end) {
            uint32_t local = scaled - seg_start;
            /* mix: 0 = full start, 255 = full end */
            uint32_t mix = local * 255u / ((uint32_t)seg->steps * 256u - 1u);
            /* Expand RGB565 channels to 8-bit and interpolate */
            uint32_t sr = (uint32_t)GT_COLOR_GET_R16(seg->start) * 255u / 31u;
            uint32_t sg = (uint32_t)GT_COLOR_GET_G16(seg->start) * 255u / 63u;
            uint32_t sb = (uint32_t)GT_COLOR_GET_B16(seg->start) * 255u / 31u;
            uint32_t er = (uint32_t)GT_COLOR_GET_R16(seg->end)   * 255u / 31u;
            uint32_t eg = (uint32_t)GT_COLOR_GET_G16(seg->end)   * 255u / 63u;
            uint32_t eb = (uint32_t)GT_COLOR_GET_B16(seg->end)   * 255u / 31u;
            uint32_t r = sr + (er - sr) * mix / 255u;
            uint32_t g = sg + (eg - sg) * mix / 255u;
            uint32_t b = sb + (eb - sb) * mix / 255u;
            return (r << 16) | (g << 8) | b;
        }
        seg_start = seg_end;
    }
    gt_gradient_color_st * last = (gt_gradient_color_st *)_gt_vector_get_item(dsc->stops, cnt - 1);
    if (!last) return 0;
    uint32_t r = (uint32_t)GT_COLOR_GET_R16(last->end) * 255u / 31u;
    uint32_t g = (uint32_t)GT_COLOR_GET_G16(last->end) * 255u / 63u;
    uint32_t b = (uint32_t)GT_COLOR_GET_B16(last->end) * 255u / 31u;
    return (r << 16) | (g << 8) | b;
}

gt_color_t gt_gradient_dsc_get_color_pos_dither(const gt_gradient_dsc_st * dsc,
                                                 uint16_t pos,
                                                 uint8_t  dither_idx)
{
    /* 4-entry Bayer row pattern: offsets in pos space (±half a color step).
     * One RGB565 R/B step ≈ 65535/32 ≈ 2048 pos units.
     * Nudge ±512 (quarter step) — enough to break bands, not enough to shift hue. */
    static const int16_t _bayer4[4] = { -512, 512, 256, -256 };
    int32_t p = (int32_t)pos + _bayer4[dither_idx & 3];
    if (p < 0)     p = 0;
    if (p > 65535) p = 65535;
    return gt_gradient_dsc_get_color_pos(dsc, (uint16_t)p);
}

#endif /* GT_USE_GRADIENT */

/* ------------------------------------------------------------------ */

#if GT_USE_SHADOW

gt_shadow_dsc_st * gt_shadow_dsc_create(gt_shadow_style_et style)
{
    gt_shadow_dsc_st * dsc = (gt_shadow_dsc_st *)gt_mem_malloc(sizeof(gt_shadow_dsc_st));
    if (!dsc) {
        GT_LOGW(GT_LOG_TAG_GRAD, "OOM: shadow dsc");
        return NULL;
    }
    dsc->color = gt_color_make(0x40, 0x40, 0x40);

    switch (style) {
        case GT_SHADOW_STYLE_BOTTOM:
            dsc->offset_x = 0;  dsc->offset_y = 6;
            dsc->spread   = 2;  dsc->opa = GT_OPA_50;
            break;
        case GT_SHADOW_STYLE_AROUND:
            dsc->offset_x = 0;  dsc->offset_y = 0;
            dsc->spread   = 6;  dsc->opa = GT_OPA_40;
            break;
        case GT_SHADOW_STYLE_DEEP:
            dsc->offset_x = 6;  dsc->offset_y = 8;
            dsc->spread   = 4;  dsc->opa = GT_OPA_70;
            break;
        case GT_SHADOW_STYLE_SUBTLE:
            dsc->offset_x = 2;  dsc->offset_y = 2;
            dsc->spread   = 4;  dsc->opa = GT_OPA_30;
            break;
        case GT_SHADOW_STYLE_BOTTOM_RIGHT:
        case GT_SHADOW_STYLE_CUSTOM:
        default:
            dsc->offset_x = 4;  dsc->offset_y = 4;
            dsc->spread   = 0;  dsc->opa = GT_OPA_50;
            break;
    }
    return dsc;
}

void gt_shadow_dsc_destroy(gt_shadow_dsc_st * dsc)
{
    if (dsc) gt_mem_free(dsc);
}

#endif /* GT_USE_SHADOW */

/* end of file ----------------------------------------------------------*/
