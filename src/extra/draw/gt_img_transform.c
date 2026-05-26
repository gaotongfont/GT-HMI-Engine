/**
 * @file gt_img_transform.c
 * @brief Fixed-point image rotation + scaling for embedded MCU
 *
 * Algorithm: Inverse mapping with Q15 fixed-point arithmetic
 * Anti-aliasing: Bilinear interpolation (Q8 sub-pixel precision)
 * Memory: Only one line buffer needed (line-by-line output)
 * No FPU required, no malloc for transform matrix
 *
 * --- Pixel sampling ---
 * Nearest-neighbor (old): each dst pixel maps to the nearest src pixel.
 * Bilinear (current): the inverse-mapped src coordinate (sx, sy) is kept
 * in Q8 fixed-point (real * 256).  The four surrounding pixels are blended
 * by their fractional weights:
 *
 *   c00(x0,y0) --- c10(x1,y0)
 *        |               |
 *        +-----(fx)------+   -> top = mix(c10, c00, fx)
 *        |               |
 *   c01(x0,y1) --- c11(x1,y1)
 *        |               |
 *        +-----(fx)------+   -> bot = mix(c11, c01, fx)
 *                 |
 *              (fy) mix(bot, top, fy)  -> result
 *
 * fx/fy are the Q8 fractional parts (0 = fully left/top, 255 = fully right/bottom).
 * gt_color_mix(c1, c2, mix): mix=0 -> c2, mix=255 -> c1.
 * All arithmetic is integer (shifts + adds), no floating point.
 * Cost: ~4x lookups + 2 blends per pixel vs nearest-neighbor.
 */

#include "gt_img_transform.h"
#include "../../core/gt_mem.h"
#include "../../others/gt_log.h"
#include "../../core/gt_draw.h"
#include "../../others/gt_area.h"
#include "../../core/gt_img_decoder.h"
#include "../../core/gt_fs.h"
#include "../../others/gt_math.h"
#include "../../core/gt_mask.h"

#if GT_USE_IMG_TRANSFORM

/* Static line buffers — avoids per-frame malloc/free, safe for single-threaded MCU.
 * Sized for max screen width (GT_SCREEN_WIDTH). */
static gt_color_t _s_out_line[GT_SCREEN_WIDTH];
static gt_opa_t   _s_out_opa[GT_SCREEN_WIDTH];

/* -----------------------------------------------------------------------
 * Bilinear interpolation sample from src_buf (RGB565, w*h pixels).
 * sx_q8, sy_q8: source coordinates in Q8 fixed-point (i.e. real * 256).
 * Returns interpolated gt_color_t. Clamps to image boundary.
 * ----------------------------------------------------------------------- */

/* Fast path: caller guarantees x0 in [0,w-2], y0 in [0,h-2] — no clamp needed */
static inline gt_color_t _bilinear_fast(const gt_color_t * src_buf,
                                        int32_t src_w,
                                        int32_t sx_q8, int32_t sy_q8)
{
    int32_t x0 = sx_q8 >> 8;
    int32_t y0 = sy_q8 >> 8;
    uint8_t fx = (uint8_t)(sx_q8 & 0xFF);
    uint8_t fy = (uint8_t)(sy_q8 & 0xFF);
    const gt_color_t * row0 = src_buf + y0 * src_w + x0;
    const gt_color_t * row1 = row0 + src_w;
    gt_color_t top = gt_color_mix(row0[1], row0[0], fx);
    gt_color_t bot = gt_color_mix(row1[1], row1[0], fx);
    return gt_color_mix(bot, top, fy);
}

/* Safe path: clamps neighbours to image bounds */
static gt_color_t _bilinear_sample(const gt_color_t * src_buf,
                                   int32_t src_w, int32_t src_h,
                                   int32_t sx_q8, int32_t sy_q8)
{
    int32_t x0 = sx_q8 >> 8;
    int32_t y0 = sy_q8 >> 8;
    uint8_t fx = (uint8_t)(sx_q8 & 0xFF);
    uint8_t fy = (uint8_t)(sy_q8 & 0xFF);

    int32_t x1 = x0 + 1; if (x1 >= src_w) x1 = src_w - 1;
    int32_t y1 = y0 + 1; if (y1 >= src_h) y1 = src_h - 1;
    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;

    gt_color_t c00 = src_buf[y0 * src_w + x0];
    gt_color_t c10 = src_buf[y0 * src_w + x1];
    gt_color_t c01 = src_buf[y1 * src_w + x0];
    gt_color_t c11 = src_buf[y1 * src_w + x1];

    gt_color_t top = gt_color_mix(c10, c00, fx);
    gt_color_t bot = gt_color_mix(c11, c01, fx);
    return gt_color_mix(bot, top, fy);
}

/* -----------------------------------------------------------------------
 * Bilinear interpolation for alpha channel (gt_opa_t flat buffer).
 * ----------------------------------------------------------------------- */
static inline gt_opa_t _bilinear_opa_fast(const gt_opa_t * abuf, int32_t src_w,
                                          int32_t sx_q8, int32_t sy_q8)
{
    int32_t x0 = sx_q8 >> 8, y0 = sy_q8 >> 8;
    uint32_t fx = (uint32_t)(sx_q8 & 0xFF);
    uint32_t fy = (uint32_t)(sy_q8 & 0xFF);
    const gt_opa_t * r0 = abuf + y0 * src_w + x0;
    const gt_opa_t * r1 = r0 + src_w;
    uint32_t top = r0[0] * (256 - fx) + r0[1] * fx;
    uint32_t bot = r1[0] * (256 - fx) + r1[1] * fx;
    return (gt_opa_t)(((top * (256 - fy) + bot * fy) + 32768u) >> 16);
}

static inline gt_opa_t _bilinear_opa_safe(const gt_opa_t * abuf,
                                          int32_t src_w, int32_t src_h,
                                          int32_t sx_q8, int32_t sy_q8)
{
    int32_t x0 = sx_q8 >> 8, y0 = sy_q8 >> 8;
    uint32_t fx = (uint32_t)(sx_q8 & 0xFF);
    uint32_t fy = (uint32_t)(sy_q8 & 0xFF);
    int32_t x1 = x0 + 1; if (x1 >= src_w) x1 = src_w - 1;
    int32_t y1 = y0 + 1; if (y1 >= src_h) y1 = src_h - 1;
    if (x0 < 0) x0 = 0; if (y0 < 0) y0 = 0;
    uint32_t top = abuf[y0*src_w+x0]*(256-fx) + abuf[y0*src_w+x1]*fx;
    uint32_t bot = abuf[y1*src_w+x0]*(256-fx) + abuf[y1*src_w+x1]*fx;
    return (gt_opa_t)(((top * (256 - fy) + bot * fy) + 32768u) >> 16);
}

/* -----------------------------------------------------------------------
 * Decode entire source image into a flat color buffer.
 * If the source has a separate alpha channel, also decode it into *out_alpha
 * (caller must free both buffers).  *out_alpha is set to NULL when no alpha.
 * ----------------------------------------------------------------------- */
static gt_color_t * _decode_full_image(_gt_img_dsc_st * dsc_img, gt_opa_t ** out_alpha)
{
    int32_t w = dsc_img->header.w;
    int32_t h = dsc_img->header.h;
    uint32_t pixel_count = (uint32_t)w * h;
    uint32_t row_bytes = pixel_count * sizeof(gt_color_t);

    if (out_alpha) *out_alpha = NULL;

    gt_color_t * buf = (gt_color_t *)gt_mem_malloc(row_bytes);
    if (!buf) {
        GT_LOGW(GT_LOG_TAG_GUI, "transform: decode buf malloc failed (%dx%d)", w, h);
        return NULL;
    }

    /* RAM image in dsc_img->img — may need color format conversion */
    if (dsc_img->img) {
        uint8_t src_cf = dsc_img->header.color_format;
        if (src_cf == 0) src_cf = GT_IMG_CF_RGB565;  /* default: RGB565 */

        /* Decode separate alpha channel if present (C array: color[w*h*src_cf] + alpha[w*h]) */
        if (out_alpha && dsc_img->alpha) {
            gt_opa_t * abuf = (gt_opa_t *)gt_mem_malloc(pixel_count);
            if (abuf) {
                gt_memcpy(abuf, dsc_img->alpha, pixel_count);
                *out_alpha = abuf;
            }
        }

#if GT_COLOR_DEPTH == 32
        if (src_cf == GT_IMG_CF_RGB565) {
            /* RGB565 → ARGB8888 */
            const uint16_t * s = (const uint16_t *)dsc_img->img;
            for (uint32_t i = 0; i < pixel_count; i++) {
                uint16_t p = s[i];
                uint8_t r5 = (p >> 11) & 0x1F;
                uint8_t g6 = (p >> 5)  & 0x3F;
                uint8_t b5 = p & 0x1F;
                buf[i].ch.red   = (r5 << 3) | (r5 >> 2);
                buf[i].ch.green = (g6 << 2) | (g6 >> 4);
                buf[i].ch.blue  = (b5 << 3) | (b5 >> 2);
                buf[i].ch.alpha = 0xFF;
            }
            return buf;
        }
#elif GT_COLOR_DEPTH == 16
        if (src_cf == GT_IMG_CF_RGB565) {
            /* formats match */
            gt_memcpy(buf, dsc_img->img, pixel_count * sizeof(gt_color16_t));
            return buf;
        }
#endif
        /* fallback: direct copy (formats match or unknown) */
        gt_memcpy(buf, dsc_img->img, row_bytes);
        return buf;
    }

    /* Use decoder read_line for all other types (SJPG, built-in file, etc.) */
    uint8_t src_cf = dsc_img->header.color_format;
    if (src_cf == 0) src_cf = GT_IMG_CF_RGB565;
    uint32_t src_row_bytes = (uint32_t)w * gt_img_cf_bpp(src_cf);
    bool need_conv = (gt_img_cf_bpp(src_cf) != sizeof(gt_color_t));

    uint8_t * row_tmp = need_conv ? (uint8_t *)gt_mem_malloc(src_row_bytes) : NULL;
    if (need_conv && !row_tmp) {
        gt_mem_free(buf);
        return NULL;
    }

    for (int32_t y = 0; y < h; y++) {
        gt_color_t * dst_row = buf + y * w;
        if (need_conv) {
            gt_res_t ret = gt_img_decoder_read_line(dsc_img, 0, y,
                                                    (gt_size_t)src_row_bytes, row_tmp);
            if (GT_RES_OK != ret) {
                gt_mem_free(row_tmp);
                gt_mem_free(buf);
                return NULL;
            }
#if GT_COLOR_DEPTH == 32
            if (src_cf == GT_IMG_CF_RGB565) {
                const uint16_t * s = (const uint16_t *)row_tmp;
                for (int32_t i = 0; i < w; i++) {
                    uint16_t p = s[i];
                    uint8_t r5 = (p >> 11) & 0x1F;
                    uint8_t g6 = (p >> 5)  & 0x3F;
                    uint8_t b5 = p & 0x1F;
                    dst_row[i].ch.red   = (r5 << 3) | (r5 >> 2);
                    dst_row[i].ch.green = (g6 << 2) | (g6 >> 4);
                    dst_row[i].ch.blue  = (b5 << 3) | (b5 >> 2);
                    dst_row[i].ch.alpha = 0xFF;
                }
            }
#endif
        } else {
            gt_res_t ret = gt_img_decoder_read_line(dsc_img, 0, y,
                                                    (gt_size_t)src_row_bytes,
                                                    (uint8_t *)dst_row);
            if (GT_RES_OK != ret) {
                gt_mem_free(buf);
                return NULL;
            }
        }
    }
    if (row_tmp) gt_mem_free(row_tmp);
    return buf;
}

/* -----------------------------------------------------------------------
 * Main transform draw function
 * ----------------------------------------------------------------------- */
void gt_img_transform_draw(_gt_draw_ctx_st * draw_ctx,
                           gt_draw_blend_dsc_st * blend_dsc,
                           _gt_img_dsc_st * dsc_img,
                           gt_angle_t angle,
                           gt_zoom_t zoom,
                           int16_t pivot_x,
                           int16_t pivot_y)
{
    int32_t src_w = dsc_img->header.w;
    int32_t src_h = dsc_img->header.h;

    if (src_w <= 0 || src_h <= 0) return;
    if (zoom == 0) return;

    /* Decode entire source image to RAM for random-access sampling */
    gt_opa_t * src_alpha = NULL;
    gt_color_t * src_buf = _decode_full_image(dsc_img, &src_alpha);
    if (!src_buf) return;

    /* Q15 fixed-point cos/sin, pre-divided by zoom for inverse mapping */
    int32_t cos_v = gt_cos_01deg(angle);
    int32_t sin_v = gt_sin_01deg(angle);
    int32_t cos_z = (int32_t)((int64_t)cos_v * 256 / zoom);
    int32_t sin_z = (int32_t)((int64_t)sin_v * 256 / zoom);

    gt_area_st * dst_area = blend_dsc->dst_area;
    int32_t dst_x0 = dst_area->x;
    int32_t dst_y0 = dst_area->y;
    int32_t dst_w  = dst_area->w;
    int32_t dst_h  = dst_area->h;

    gt_color_t * out_line = _s_out_line;
    gt_opa_t   * out_opa  = _s_out_opa;

    gt_opa_t global_opa = blend_dsc->opa >= GT_OPA_MAX ? GT_OPA_COVER : blend_dsc->opa;

    gt_area_st row_area = *dst_area;
    row_area.h = 1;

    for (int32_t dy = 0; dy < dst_h; dy++) {
        row_area.y = dst_y0 + dy;
        if (!gt_area_is_intersect_screen(&draw_ctx->buf_area, &row_area)) {
            continue;
        }

        int32_t ry = dy - pivot_y;
        int32_t sx_q8 = (int32_t)(((int64_t)sin_z * ry) >> 7) + (pivot_x << 8)
                        - (int32_t)(((int64_t)cos_z * pivot_x) >> 7);
        int32_t sy_q8 = (int32_t)(((int64_t)cos_z * ry) >> 7) + (pivot_y << 8)
                        + (int32_t)(((int64_t)sin_z * pivot_x) >> 7);
        int32_t sx_step = cos_z >> 7;
        int32_t sy_step = -(sin_z >> 7);

        /* Safe margin: bilinear needs x0 in [0,w-2], y0 in [0,h-2] for fast path */
        int32_t src_w_safe = src_w - 1;
        int32_t src_h_safe = src_h - 1;

        for (int32_t dx = 0; dx < dst_w; dx++, sx_q8 += sx_step, sy_q8 += sy_step) {
            int32_t sx = sx_q8 >> 8;
            int32_t sy = sy_q8 >> 8;

            if ((uint32_t)sx < (uint32_t)src_w && (uint32_t)sy < (uint32_t)src_h) {
                if (sx < src_w_safe && sy < src_h_safe) {
                    out_line[dx] = _bilinear_fast(src_buf, src_w, sx_q8, sy_q8);
                } else {
                    out_line[dx] = _bilinear_sample(src_buf, src_w, src_h, sx_q8, sy_q8);
                }
                if (src_alpha) {
                    gt_opa_t a = (sx < src_w_safe && sy < src_h_safe)
                        ? _bilinear_opa_fast(src_alpha, src_w, sx_q8, sy_q8)
                        : _bilinear_opa_safe(src_alpha, src_w, src_h, sx_q8, sy_q8);
                    out_opa[dx] = (gt_opa_t)(((uint32_t)a * global_opa) >> 8);
                } else {
                    out_opa[dx] = global_opa;
                }
            } else {
                out_line[dx] = blend_dsc->color_fill;
                out_opa[dx]  = 0;
            }
        }

        gt_draw_blend_dsc_st row_dsc = *blend_dsc;
        row_dsc.dst_area  = &row_area;
        row_dsc.mask_area = &row_area;
        row_dsc.dst_buf   = out_line;
        row_dsc.mask_buf  = out_opa;
        row_dsc.opa       = GT_OPA_COVER;
        gt_draw_blend(draw_ctx, &row_dsc);
    }

    gt_mem_free(src_buf);
    if (src_alpha) gt_mem_free(src_alpha);
}

/* -----------------------------------------------------------------------
 * Decode image source to a flat RGB cache (one-time, for anim reuse)
 * ----------------------------------------------------------------------- */
gt_color_t * gt_img_decode_to_cache(const gt_attr_rect_st * dsc,
                                    uint16_t * out_w, uint16_t * out_h,
                                    bool * out_is_rom, gt_opa_t ** out_alpha)
{
    if (out_is_rom) *out_is_rom = false;
    if (out_alpha) *out_alpha = NULL;

    _gt_img_dsc_st dsc_img = {0};
    gt_res_t ret = GT_RES_FAIL;

    if (dsc->bg_img_src) {
        ret = gt_img_decoder_open(&dsc_img, dsc->bg_img_src);
    } else if (dsc->raw_img) {
        if (dsc->raw_img->raw_p) {
            ret = gt_img_decoder_open_raw(&dsc_img, dsc->raw_img->raw_p, dsc->raw_img->raw_len);
        } else if (GT_IMG_DECODER_TYPE_NONE != dsc->raw_img->header.type || dsc->raw_img->fp) {
            dsc_img = *dsc->raw_img;
            ret = GT_RES_OK;
        }
    }
#if GT_USE_FILE_HEADER
    if (GT_RES_OK != ret) {
        ret = gt_img_decoder_fh_open(&dsc_img, dsc->file_header);
    }
#endif
#if GT_USE_DIRECT_ADDR
    if (GT_RES_OK != ret) {
        ret = gt_img_decoder_direct_addr_open(&dsc_img, dsc->addr);
    }
#endif

    if (GT_RES_OK != ret) {
        GT_LOGW(GT_LOG_TAG_GUI, "decode_to_cache: open failed");
        return NULL;
    }

    *out_w = dsc_img.header.w;
    *out_h = dsc_img.header.h;

    /* ROM/RAM direct pointer: dsc_img.img already points to the pixel data.
     * Return it directly — no malloc, caller must NOT free it. */
    if (dsc_img.img && GT_IMG_DECODER_TYPE_RAM == dsc_img.header.type) {
        if (out_is_rom) *out_is_rom = true;
        /* Alpha channel is stored right after color data — copy it out */
        if (out_alpha && dsc_img.alpha) {
            uint32_t pixel_count = (uint32_t)dsc_img.header.w * dsc_img.header.h;
            gt_opa_t * abuf = (gt_opa_t *)gt_mem_malloc(pixel_count);
            if (abuf) {
                gt_memcpy(abuf, dsc_img.alpha, pixel_count);
                *out_alpha = abuf;
            }
        }
        return (gt_color_t *)dsc_img.img;
    }

    /* For array-type images, fp->buffer holds color+alpha contiguously.
     * Populate dsc_img.img / dsc_img.alpha so _decode_full_image can use them. */
    if (dsc_img.fp && GT_FS_TYPE_ARRAY == dsc_img.fp->type) {
        uint8_t src_cf = dsc_img.header.color_format ? dsc_img.header.color_format : GT_IMG_CF_RGB565;
        uint32_t pixel_count = (uint32_t)dsc_img.header.w * dsc_img.header.h;
        dsc_img.img = (uint8_t *)dsc_img.fp->buffer;
        if (dsc_img.fp->msg.pic.is_alpha) {
            dsc_img.alpha = (gt_opa_t *)((uint8_t *)dsc_img.fp->buffer + pixel_count * gt_img_cf_bpp(src_cf));
        }
    }

    gt_color_t * buf = _decode_full_image(&dsc_img, out_alpha);
    gt_img_decoder_close(&dsc_img);
    return buf;
}

/* -----------------------------------------------------------------------
 * Draw pre-decoded cache with transform — no decoding, safe for anim
 * ----------------------------------------------------------------------- */
void gt_img_transform_draw_cached(struct _gt_draw_ctx_s * draw_ctx,
                                  const gt_area_st * coords,
                                  gt_opt_t opa,
                                  const gt_color_t * src_buf,
                                  const gt_opa_t * alpha_buf,
                                  uint16_t src_w, uint16_t src_h,
                                  gt_angle_t angle, gt_zoom_t zoom,
                                  int16_t pivot_x, int16_t pivot_y,
                                  bool use_nn,
                                  const gt_color_masking_st * masking)
{
    if (!src_buf || src_w == 0 || src_h == 0 || zoom == 0) return;
    if (!draw_ctx->buf) return;

    /* Register masking (radius + edge-blur) if requested */
    gt_size_t _mask_id      = -1;
    gt_size_t _edge_mask_id = -1;
    gt_mask_radius_st    _r_mask    = {0};
    gt_mask_edge_blur_st _edge_blur = {0};
    bool has_masking = (masking != NULL) && (masking->radius != 0 || masking->gap != 0);
    if (has_masking) {
        if (masking->gap != 0) {
            gt_mask_edge_blur_init(&_edge_blur, coords, masking->gap);
            _edge_mask_id = gt_mask_add(&_edge_blur);
        }
        if (masking->radius != 0) {
            gt_mask_radius_init(&_r_mask, coords, masking->radius, GT_MASK_RADIUS_CFG_INNER);
            _mask_id = gt_mask_add(&_r_mask);
        }
    }

    int32_t cos_v = gt_cos_01deg(angle);
    int32_t sin_v = gt_sin_01deg(angle);
    int32_t cos_z = (int32_t)((int64_t)cos_v * 256 / zoom);
    int32_t sin_z = (int32_t)((int64_t)sin_v * 256 / zoom);

    int32_t dst_x0 = coords->x;
    int32_t dst_y0 = coords->y;
    int32_t dst_w  = coords->w;
    int32_t dst_h  = coords->h;

    gt_color_t * out_line = _s_out_line;
    gt_opa_t   * out_opa  = _s_out_opa;

    gt_opa_t global_opa = opa >= GT_OPA_MAX ? GT_OPA_COVER : (gt_opa_t)opa;

    gt_area_st row_area = *coords;
    row_area.h = 1;

    for (int32_t dy = 0; dy < dst_h; dy++) {
        row_area.y = dst_y0 + dy;
        if (!gt_area_is_intersect_screen(&draw_ctx->buf_area, &row_area)) {
            continue;
        }

        int32_t ry = dy - pivot_y;
        int32_t sx_q8 = (int32_t)(((int64_t)sin_z * ry) >> 7) + (pivot_x << 8)
                        - (int32_t)(((int64_t)cos_z * pivot_x) >> 7);
        int32_t sy_q8 = (int32_t)(((int64_t)cos_z * ry) >> 7) + (pivot_y << 8)
                        + (int32_t)(((int64_t)sin_z * pivot_x) >> 7);
        int32_t sx_step = cos_z >> 7;
        int32_t sy_step = -(sin_z >> 7);

        int32_t src_w_safe = (int32_t)src_w - 1;
        int32_t src_h_safe = (int32_t)src_h - 1;

        if (use_nn) {
            /* Nearest-neighbor: fast path for animation */
            for (int32_t dx = 0; dx < dst_w; dx++, sx_q8 += sx_step, sy_q8 += sy_step) {
                int32_t sx = (sx_q8 + 128) >> 8;   /* round to nearest */
                int32_t sy = (sy_q8 + 128) >> 8;
                if ((uint32_t)sx < (uint32_t)src_w && (uint32_t)sy < (uint32_t)src_h) {
                    out_line[dx] = src_buf[sy * src_w + sx];
                    if (alpha_buf) {
                        gt_opa_t a = alpha_buf[sy * src_w + sx];
                        out_opa[dx] = (gt_opa_t)(((uint32_t)a * global_opa) >> 8);
                    } else {
                        out_opa[dx] = global_opa;
                    }
                } else {
                    out_opa[dx] = 0;
                }
            }
        } else {
            /* Bilinear: smooth path for static display */
            for (int32_t dx = 0; dx < dst_w; dx++, sx_q8 += sx_step, sy_q8 += sy_step) {
                int32_t sx = sx_q8 >> 8;
                int32_t sy = sy_q8 >> 8;
                if ((uint32_t)sx < (uint32_t)src_w && (uint32_t)sy < (uint32_t)src_h) {
                    if (sx < src_w_safe && sy < src_h_safe) {
                        out_line[dx] = _bilinear_fast(src_buf, src_w, sx_q8, sy_q8);
                    } else {
                        out_line[dx] = _bilinear_sample(src_buf, src_w, src_h, sx_q8, sy_q8);
                    }
                    if (alpha_buf) {
                        gt_opa_t a = (sx < src_w_safe && sy < src_h_safe)
                            ? _bilinear_opa_fast(alpha_buf, src_w, sx_q8, sy_q8)
                            : _bilinear_opa_safe(alpha_buf, src_w, src_h, sx_q8, sy_q8);
                        out_opa[dx] = (gt_opa_t)(((uint32_t)a * global_opa) >> 8);
                    } else {
                        out_opa[dx] = global_opa;
                    }
                } else {
                    out_opa[dx] = 0;
                }
            }
        }

        gt_draw_blend_dsc_st row_dsc = {
            .dst_area  = &row_area,
            .mask_area = &row_area,
            .dst_buf   = out_line,
            .mask_buf  = out_opa,
            .opa       = GT_OPA_COVER,
        };

        /* Apply registered masks (radius/edge-blur) to the per-row opa buffer */
        if (has_masking) {
            gt_mask_get(out_opa, row_area.x, row_area.y, (uint16_t)dst_w);
        }

        gt_draw_blend(draw_ctx, &row_dsc);
    }

    /* Unregister masks */
    if (has_masking) {
        if (masking->radius != 0) {
            gt_mask_remove_idx(_mask_id);
            gt_mask_free(&_r_mask);
        }
        if (masking->gap != 0) {
            gt_mask_remove_idx(_edge_mask_id);
            gt_mask_free(&_edge_blur);
        }
    }
}

#endif /* GT_USE_IMG_TRANSFORM */
