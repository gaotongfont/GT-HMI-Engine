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
#include "../../others/gt_math.h"

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
 * Decode entire source image into a flat RGB buffer via read_line.
 * Returns malloc'd buffer (caller must free), or NULL on failure.
 * ----------------------------------------------------------------------- */
static gt_color_t * _decode_full_image(_gt_img_dsc_st * dsc_img)
{
    int32_t w = dsc_img->header.w;
    int32_t h = dsc_img->header.h;
    uint32_t row_bytes = (uint32_t)w * sizeof(gt_color_t);

    gt_color_t * buf = (gt_color_t *)gt_mem_malloc(row_bytes * (uint32_t)h);
    if (!buf) {
        GT_LOGW(GT_LOG_TAG_GUI, "transform: decode buf malloc failed (%dx%d)", w, h);
        return NULL;
    }

    /* RAM image already in dsc_img->img — just copy */
    if (dsc_img->img) {
        gt_memcpy(buf, dsc_img->img, row_bytes * (uint32_t)h);
        return buf;
    }

    /* Use decoder read_line for all other types (SJPG, built-in file, etc.) */
    for (int32_t y = 0; y < h; y++) {
        gt_res_t ret = gt_img_decoder_read_line(dsc_img, 0, y,
                                                (gt_size_t)row_bytes,
                                                (uint8_t *)(buf + y * w));
        if (GT_RES_OK != ret) {
            GT_LOGW(GT_LOG_TAG_GUI, "transform: read_line failed at y=%d", y);
            gt_mem_free(buf);
            return NULL;
        }
    }
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
    gt_color_t * src_buf = _decode_full_image(dsc_img);
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

        /* Row-start src coords (Q8), incremented per column by (cos_z>>7, -sin_z>>7) */
        int32_t ry = dy - pivot_y;
        int32_t sx_q8 = (int32_t)(((int64_t)sin_z * ry) >> 7) + (pivot_x << 8);
        int32_t sy_q8 = (int32_t)(((int64_t)cos_z * ry) >> 7) + (pivot_y << 8);
        /* Per-column step: d(sx_q8)/d(dx) = cos_z>>7, d(sy_q8)/d(dx) = -sin_z>>7 */
        int32_t sx_step = cos_z >> 7;
        int32_t sy_step = -(sin_z >> 7);
        /* Correction for truncation error in step (keep full precision at row start) */
        int32_t sx_q8_start = sx_q8 - (int32_t)(((int64_t)cos_z * pivot_x) >> 7);
        int32_t sy_q8_start = sy_q8 + (int32_t)(((int64_t)sin_z * pivot_x) >> 7);
        sx_q8 = sx_q8_start;
        sy_q8 = sy_q8_start;

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
                out_opa[dx] = global_opa;
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
}

/* -----------------------------------------------------------------------
 * Decode image source to a flat RGB cache (one-time, for anim reuse)
 * ----------------------------------------------------------------------- */
gt_color_t * gt_img_decode_to_cache(const gt_attr_rect_st * dsc,
                                    uint16_t * out_w, uint16_t * out_h,
                                    bool * out_is_rom)
{
    if (out_is_rom) *out_is_rom = false;

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
        return (gt_color_t *)dsc_img.img;
    }

    gt_color_t * buf = _decode_full_image(&dsc_img);
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
                                  uint16_t src_w, uint16_t src_h,
                                  gt_angle_t angle, gt_zoom_t zoom,
                                  int16_t pivot_x, int16_t pivot_y,
                                  bool use_nn)
{
    if (!src_buf || src_w == 0 || src_h == 0 || zoom == 0) return;
    if (!draw_ctx->buf) return;

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
                    out_opa[dx]  = global_opa;
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
                    out_opa[dx] = global_opa;
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
        gt_draw_blend(draw_ctx, &row_dsc);
    }
}

#endif /* GT_USE_IMG_TRANSFORM */
