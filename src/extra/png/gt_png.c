/**
 * @file gt_png.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-08-10 15:48:26
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_png.h"
#include "../../core/gt_img_decoder.h"

#if GT_USE_PNG
#include "./lodepng.h"
#include "../../others/gt_log.h"
#include "../../core/gt_mem.h"
#include "../../others/gt_types.h"
#include "../../core/gt_fs.h"
#include "../../others/gt_color.h"

/* private define -------------------------------------------------------*/
/**
 * @brief PNG file color order: 0: RGB; 1: BGR
 */
#define _COLOR_BYTE_REVERSE     01


/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static inline bool _is_png_file_ext(const char * path) {
    return strcmp(&path[strlen(path) - 3], "png") ? false : true;
}

static inline uint32_t _get_32bit_int(const char * buffer) {
    return ( (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3] );
}

/**
 * @brief bitmap convert to locate rgb mode
 *
 * @param image
 * @param size
 */
static void _gt_png_color_convert_depth(uint8_t * image, uint8_t * alpha, uint32_t size) {
#if GT_COLOR_DEPTH == 32
    GT_UNUSED(image);
    GT_UNUSED(size);
#elif GT_COLOR_DEPTH == 16
    gt_color32_t * img_argb = (gt_color32_t * )image;
    gt_color_t color;
    uint32_t i,idx=0;
    for (i = 0; i < size; i++) {
#if _COLOR_BYTE_REVERSE
        color = gt_color_make(GT_COLOR_GET_B32(img_argb[i]), GT_COLOR_GET_G32(img_argb[i]), GT_COLOR_GET_R32(img_argb[i]));
#else
        color = gt_color_make(GT_COLOR_GET_R32(img_argb[i]), GT_COLOR_GET_G32(img_argb[i]), GT_COLOR_GET_B32(img_argb[i]));
#endif
        image[idx++] = GT_COLOR_GET(color) & 0xff;
        image[idx++] = GT_COLOR_GET(color) >> 8;
        alpha[i]     = GT_COLOR_GET_A32(img_argb[i]);
    }
#elif GT_COLOR_DEPTH == 8
    gt_color32_t * img_argb = (gt_color32_t * )image;
    gt_color_t color;
    uint32_t i;
    for (i = 0; i < size; i++) {
#if _COLOR_BYTE_REVERSE
        color = gt_color_make(GT_COLOR_GET_B32(img_argb[i]), GT_COLOR_GET_G32(img_argb[i]), GT_COLOR_GET_R32(img_argb[i]));
#else
        color = gt_color_make(GT_COLOR_GET_R32(img_argb[i]), GT_COLOR_GET_G32(img_argb[i]), GT_COLOR_GET_B32(img_argb[i]));
#endif
        image[(i << 1) + 1] = GT_COLOR_GET_A32(img_argb[i]);
        image[i << 1]       = GT_COLOR_GET(color);
    }
#elif GT_COLOR_DEPTH == 1
    gt_color32_t * img_argb = (gt_color32_t * )image;
    uint8_t color;
    uint32_t i;
    for (i = 0; i < size; i++) {
#if _COLOR_BYTE_REVERSE
        color = gt_color_make(GT_COLOR_GET_B32(img_argb[i]), GT_COLOR_GET_G32(img_argb[i]), GT_COLOR_GET_R32(img_argb[i]));
#else
        color = GT_COLOR_GET_R32(img_argb[i]) | GT_COLOR_GET_G32(img_argb[i]) | GT_COLOR_GET_B32(img_argb[i]);
#endif
        image[(i << 1) + 1] = GT_COLOR_GET_A32(img_argb[i]);
        image[i << 1]       = color;
    }
#endif
}

static void _set_png_width_height(_gt_img_info_st * header, gt_size_t width, gt_size_t height) {
    header->w = width;
    header->h = height;
}

static void _set_png_type(_gt_img_info_st * header) {
    header->type = GT_IMG_DECODER_TYPE_PNG;
    header->color_format = GT_IMG_CF_TRUE_COLOR_ALPHA;
}

static void _set_fp_info(gt_fs_fp_st * fp, _gt_img_info_st * header) {
    fp->msg.pic.w = header->w;
    fp->msg.pic.h = header->h;
    fp->msg.pic.is_alpha = 1;   // GT_IMG_TYPE_RGB565_PNG, is not alpha
}

static bool _is_png_file_type(gt_fs_fp_st * fp) {
    const uint8_t magic[] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a};
    uint8_t buffer[8] = {0};
    uint32_t ret_len = 0;

    if (gt_fs_seek(fp, 0, GT_FS_SEEK_SET)) {
        return false;
    }
    if (gt_fs_read(fp, buffer, 8, &ret_len)) {
        return false;
    }
    if (gt_memcmp(buffer, magic, sizeof(magic))) {
        return false;
    }
    return true;
}

static gt_res_t _common_png_info(struct _gt_img_decoder_s * decoder, gt_fs_fp_st * fp, _gt_img_info_st * header) {
    uint8_t tmp_buffer[8] = {0};
    uint32_t ret_len = 0;
    gt_res_t ret = GT_RES_OK;
    if (false == _is_png_file_type(fp)) {
        ret = GT_RES_INV;
        goto err_lb;
    }
    /**
     * png file width and height offset:
     * width: [16 ~ 20]
     * height: [20 ~ 24]
     */
    if (gt_fs_seek(fp, 16, GT_FS_SEEK_SET)) {
        goto err_lb;
    }
    if (gt_fs_read(fp, tmp_buffer, 8, &ret_len)) {
        goto err_lb;
    }

    gt_memset_0(header, sizeof(_gt_img_info_st));

    _set_png_width_height(header, (gt_size_t)_get_32bit_int((char * )&tmp_buffer[0]), (gt_size_t)_get_32bit_int((char * )&tmp_buffer[4]));
    _set_png_type(header);

err_lb:
    gt_fs_close(fp);
    return ret;
}

static gt_res_t _gt_png_info(struct _gt_img_decoder_s * decoder, const void * src, _gt_img_info_st * header) {
    const char * path = (char * )src;
    if (!_is_png_file_ext(path)) {
        return GT_RES_INV;
    }

    gt_fs_fp_st * fp = gt_fs_open(src, GT_FS_MODE_RD);
    if (!fp) {
        return GT_RES_INV;
    }
    return _common_png_info(decoder, fp, header);
}

static gt_res_t _common_png_open(struct _gt_img_decoder_s * decoder, struct _gt_img_dsc_s * dsc, int32_t png_size, uint8_t * png_data) {
    unsigned int png_width = 0, png_height = 0;
    gt_res_t ret = lodepng_decode32(&dsc->img, &png_width, &png_height, png_data, png_size);
    if (ret) {
#ifndef LODEPNG_NO_COMPILE_ERROR_TEXT
        GT_LOGE(GT_LOG_TAG_EXTRA, "err code: %d: %s", ret, lodepng_error_text(ret));
#endif
        goto png_lb;
    }

    dsc->alpha = gt_mem_malloc(png_width * png_height);
    if (!dsc->alpha) {
        goto png_lb;
    }
    _gt_png_color_convert_depth(dsc->img, dsc->alpha, png_width * png_height);

    dsc->header.w = png_width;
    dsc->header.h = png_height;

    return GT_RES_OK;

png_lb:
    if (dsc->img) {
        gt_mem_free(dsc->img);
        dsc->img = NULL;
    }
    return GT_RES_FAIL;
}

static gt_res_t _gt_png_open(struct _gt_img_decoder_s * decoder, struct _gt_img_dsc_s * dsc)
{
    GT_UNUSED(decoder);
    int32_t png_size   = 0;
    uint8_t * png_data = NULL;
    const char * path = (char * )dsc->src;

    if (!_is_png_file_ext(path)) {
        return GT_RES_INV;
    }
    gt_fs_fp_st * fp = gt_fs_open(dsc->src, GT_FS_MODE_RD);
    if (NULL == fp) {
        return GT_RES_INV;
    }
    if (false == _is_png_file_type(fp)) {
        gt_fs_close(fp);
        return GT_RES_INV;
    }

    if (0 != lodepng_load_file(&png_data, (size_t * )&png_size, dsc->src)) {
        goto err_lb;
    }
    gt_res_t ret = _common_png_open(decoder, dsc, png_size, png_data);

err_lb:
    if (png_data) {
        gt_mem_free(png_data);
    }
    gt_fs_close(fp);
    return ret;
}

static gt_res_t _gt_png_read_line(struct _gt_img_dsc_s * dsc,
                                    gt_size_t x, gt_size_t y, gt_size_t len, uint8_t * buffer)
{
    gt_memcpy(buffer, &dsc->img[dsc->header.w *y*sizeof(gt_color_t)], len);

    return GT_RES_OK;
}

static gt_res_t _gt_png_close(struct _gt_img_dsc_s * dsc)
{
    if (dsc->img) {
        gt_mem_free(dsc->img);
        dsc->img = NULL;
    }
    if( dsc->alpha ){
        gt_mem_free(dsc->alpha);
        dsc->alpha = NULL;
    }
    if (dsc->fp) {
        gt_fs_close(dsc->fp);
        dsc->fp = NULL;
    }

    return GT_RES_OK;
}

#if GT_USE_FILE_HEADER
static gt_res_t _gt_png_fh_info(struct _gt_img_decoder_s * decoder, gt_file_header_param_st const * const param, _gt_img_info_st * header) {
    gt_fs_fp_st * fp = gt_fs_fh_open(param, GT_FS_MODE_RD);
    if (!fp) {
        return GT_RES_INV;
    }
    return _common_png_info(decoder, fp, header);
}

gt_res_t _gt_png_fh_open(struct _gt_img_decoder_s * decoder, struct _gt_img_dsc_s * dsc) {
    GT_UNUSED(decoder);
    int32_t png_size   = 0;
    uint8_t * png_data = NULL;
    gt_fs_fp_st * fp = gt_fs_fh_open(dsc->file_header, GT_FS_MODE_RD);
    if (NULL == fp) {
        return GT_RES_INV;
    }
    if (false == _is_png_file_type(fp)) {
        gt_fs_close(fp);
        return GT_RES_INV;
    }

    if (0 != lodepng_load_file_fh(&png_data, (size_t * )&png_size, dsc->file_header)) {
        goto err_lb;
    }
    gt_res_t ret = _common_png_open(decoder, dsc, png_size, png_data);

err_lb:
    if (png_data) {
        gt_mem_free(png_data);
    }
    gt_fs_close(fp);
    return ret;
}
#endif

/* global functions / API interface -------------------------------------*/

void gt_png_init(void)
{
    _gt_img_decoder_st * decoder = gt_img_decoder_create();

    gt_img_decoder_set_info_cb(decoder, _gt_png_info);
    gt_img_decoder_set_open_cb(decoder, _gt_png_open);
    gt_img_decoder_set_read_line_cb(decoder, _gt_png_read_line);
    gt_img_decoder_set_close_cb(decoder, _gt_png_close);

#if GT_USE_FILE_HEADER
    gt_img_decoder_set_fh_info_cb(decoder, _gt_png_fh_info);
    gt_img_decoder_set_fh_open_cb(decoder, _gt_png_fh_open);
#endif

    gt_img_decoder_register(decoder);
}




#endif   /* GT_USE_PNG */

/* end ------------------------------------------------------------------*/
