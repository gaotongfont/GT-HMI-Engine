/**
 * @file gt_svg.c
 * @author Feyoung
 * @brief SVG image decoder based on nanosvg + nanosvgrast (SVG Tiny 1.2)
 * @version 0.1
 * @date 2026-04-21
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_svg.h"

#if GT_USE_SVG

#define NANOSVG_IMPLEMENTATION
#define NANOSVGRAST_IMPLEMENTATION
#include "./nanosvg.h"
#include "./nanosvgrast.h"

#include "../../core/gt_mem.h"
#include "../../core/gt_fs.h"
#include "../../others/gt_log.h"
#include "../../others/gt_color.h"

/* private define -------------------------------------------------------*/
#define _SVG_DPI    96.0f

/* private typedef ------------------------------------------------------*/
typedef struct {
    uint8_t lock : 1;
} _svg_ctrl_st;

/* static variables -----------------------------------------------------*/
static _svg_ctrl_st _svg_ctrl = { .lock = 0 };

/* static functions -----------------------------------------------------*/
static inline bool _is_svg_lock(void)   { return _svg_ctrl.lock; }
static inline void _svg_lock(void)      { _svg_ctrl.lock = 1; }
static inline void _svg_unlock(void)    { _svg_ctrl.lock = 0; }

static inline bool _is_svg_ext(const char * path) {
    size_t len = strlen(path);
    return (len >= 4 && strcmp(&path[len - 4], ".svg") == 0) ? true : false;
}

/** Convert RGBA8888 (nanosvg output) to gt_color_t + alpha array.
 *  Forward iteration: write dst[i*2] before reading src[i*4], safe in-place. */
static void _rgba_to_gt_color(uint8_t * rgba, gt_opa_t * alpha, uint32_t pixel_count) {
#if GT_COLOR_DEPTH == 32
    for (uint32_t i = 0; i < pixel_count; i++) {
        alpha[i]        = rgba[i * 4 + 3];
        rgba[i * 4 + 3] = 0xFF;
    }
#else
    uint32_t wi = 0;
    for (uint32_t i = 0; i < pixel_count; i++) {
        uint8_t r = rgba[i * 4 + 0];
        uint8_t g = rgba[i * 4 + 1];
        uint8_t b = rgba[i * 4 + 2];
        alpha[i]  = rgba[i * 4 + 3];
        gt_color_t c = gt_color_make(r, g, b);
        rgba[wi++] = (uint8_t)(c.full & 0xFF);
        rgba[wi++] = (uint8_t)(c.full >> 8);
    }
#endif
}

/**
 * Rasterize parsed NSVGimage into dsc->img / dsc->alpha.
 * rgba_buf: W*H*4 temporary buffer (freed before return).
 */
static gt_res_t _rasterize(struct _gt_img_dsc_s * dsc, NSVGimage * svg) {
    uint32_t w = (uint32_t)svg->width;
    uint32_t h = (uint32_t)svg->height;
    uint32_t pixel_count = w * h;

    uint8_t * rgba = (uint8_t *)gt_mem_extern_malloc(pixel_count * 4);
    if (!rgba) { goto fail; }

    NSVGrasterizer * rast = nsvgCreateRasterizer();
    if (!rast) { gt_mem_extern_free(rgba); goto fail; }

    nsvgRasterize(rast, svg, 0, 0, 1.0f, rgba, (int)w, (int)h, (int)(w * 4));
    nsvgDeleteRasterizer(rast);

    dsc->alpha = (gt_opa_t *)gt_mem_extern_malloc(pixel_count);
    if (!dsc->alpha) { gt_mem_extern_free(rgba); goto fail; }

    /* reuse rgba buffer in-place: RGB565 shrinks, so iterate reverse */
    dsc->img = rgba;
    _rgba_to_gt_color(rgba, dsc->alpha, pixel_count);

    dsc->header.w = w;
    dsc->header.h = h;
    dsc->header.type = GT_IMG_DECODER_TYPE_SVG;
    dsc->header.color_format = GT_IMG_CF_TRUE_COLOR_ALPHA;
    nsvgDelete(svg);
    return GT_RES_OK;

fail:
    nsvgDelete(svg);
    return GT_RES_FAIL;
}

/* ---- decoder callbacks ---- */

/** Parse SVG from a gt_fs_fp_st whose buffer points to raw SVG text */
static NSVGimage * _svg_parse_from_fp(gt_fs_fp_st * fp) {
    uint32_t len = fp->end - fp->start;
    char * buf = (char *)gt_mem_extern_malloc(len + 1);
    if (!buf) { return NULL; }
    gt_memcpy(buf, fp->buffer, len);
    buf[len] = '\0';
    NSVGimage * svg = nsvgParse(buf, "px", _SVG_DPI);
    gt_mem_extern_free(buf);
    return svg;
}

static gt_res_t _gt_svg_info(struct _gt_img_decoder_s * decoder,
                              const void * src, _gt_img_info_st * header) {
    const char * path = (const char *)src;
    if (!_is_svg_ext(path)) { return GT_RES_INV; }

    gt_fs_fp_st * fp = gt_fs_open(path, GT_FS_MODE_RD);
    if (!fp) { return GT_RES_INV; }

    NSVGimage * svg = _svg_parse_from_fp(fp);
    gt_fs_close(fp);
    if (!svg) { return GT_RES_INV; }

    gt_memset_0(header, sizeof(_gt_img_info_st));
    header->w = (uint32_t)svg->width;
    header->h = (uint32_t)svg->height;
    header->type = GT_IMG_DECODER_TYPE_SVG;
    header->color_format = GT_IMG_CF_TRUE_COLOR_ALPHA;
    nsvgDelete(svg);
    return GT_RES_OK;
}

static gt_res_t _gt_svg_info_raw(struct _gt_img_decoder_s * decoder,
                                  const void * raw_data, uint32_t raw_len,
                                  _gt_img_info_st * header) {
    /* nanosvg modifies the string in-place, so copy first */
    char * buf = (char *)gt_mem_extern_malloc(raw_len + 1);
    if (!buf) { return GT_RES_INV; }
    gt_memcpy(buf, raw_data, raw_len);
    buf[raw_len] = '\0';

    NSVGimage * svg = nsvgParse(buf, "px", _SVG_DPI);
    gt_mem_extern_free(buf);
    if (!svg) { return GT_RES_INV; }

    gt_memset_0(header, sizeof(_gt_img_info_st));
    header->w = (uint32_t)svg->width;
    header->h = (uint32_t)svg->height;
    header->type = GT_IMG_DECODER_TYPE_SVG;
    header->color_format = GT_IMG_CF_TRUE_COLOR_ALPHA;
    nsvgDelete(svg);
    return GT_RES_OK;
}

static gt_res_t _gt_svg_open(struct _gt_img_decoder_s * decoder,
                              struct _gt_img_dsc_s * dsc) {
    if (_is_svg_lock()) { return GT_RES_INV; }
    _svg_lock();

    NSVGimage * svg = NULL;
    const char * path = (const char *)dsc->src;

    if (path && _is_svg_ext(path)) {
        gt_fs_fp_st * fp = gt_fs_open(path, GT_FS_MODE_RD);
        if (fp) {
            svg = _svg_parse_from_fp(fp);
            gt_fs_close(fp);
        }
    } else if (dsc->raw_p && dsc->raw_len > 0) {
        char * buf = (char *)gt_mem_extern_malloc(dsc->raw_len + 1);
        if (!buf) { _svg_unlock(); return GT_RES_INV; }
        gt_memcpy(buf, dsc->raw_p, dsc->raw_len);
        buf[dsc->raw_len] = '\0';
        svg = nsvgParse(buf, "px", _SVG_DPI);
        gt_mem_extern_free(buf);
    }

    if (!svg) { _svg_unlock(); return GT_RES_INV; }

    gt_res_t ret = _rasterize(dsc, svg);  /* svg freed inside */
    if (ret != GT_RES_OK) { _svg_unlock(); }
    return ret;
}

static gt_res_t _gt_svg_read_line(struct _gt_img_dsc_s * dsc,
                                   gt_size_t x, gt_size_t y,
                                   gt_size_t len, uint8_t * buffer) {
    gt_memcpy(buffer, &dsc->img[dsc->header.w * y * sizeof(gt_color_t)], len);
    return GT_RES_OK;
}

static gt_res_t _gt_svg_close(struct _gt_img_dsc_s * dsc) {
    _svg_unlock();
    if (dsc->img)   { gt_mem_extern_free(dsc->img);   dsc->img   = NULL; }
    if (dsc->alpha) { gt_mem_extern_free(dsc->alpha);  dsc->alpha = NULL; }
    if (dsc->fp)    { gt_fs_close(dsc->fp);            dsc->fp    = NULL; }
    return GT_RES_OK;
}

/* global functions / API interface -------------------------------------*/

void gt_svg_init(void) {
    _gt_img_decoder_st * decoder = gt_img_decoder_create();
    gt_img_decoder_set_info_cb(decoder, _gt_svg_info);
    gt_img_decoder_set_info_raw_cb(decoder, _gt_svg_info_raw);
    gt_img_decoder_set_open_cb(decoder, _gt_svg_open);
    gt_img_decoder_set_read_line_cb(decoder, _gt_svg_read_line);
    gt_img_decoder_set_close_cb(decoder, _gt_svg_close);
    gt_img_decoder_register(decoder);
}

#endif  /* GT_USE_SVG */

/* end ------------------------------------------------------------------*/
