/**
 * @file gt_img_cache.c
 * @author Feyoung
 * @brief The image cache function, which can be used to cache the image data.
 * @version 0.1
 * @date 2024-10-17 10:26:47
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_img_cache.h"

#if GT_USE_IMG_CACHE
#include "../core/gt_fs.h"
#include "../core/gt_img_decoder.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_res_t _get_image_raw_data(_gt_img_dsc_st * dsc_img, gt_color_img_raw_st * raw, uint8_t * cache_buffer, uint32_t byte_len) {
    uint32_t len = 0, addr_start_alpha = 0, total_len = 0;
    uint8_t is_mask = 1 == dsc_img->fp->msg.pic.is_alpha ? 1 : 0;

    len = dsc_img->header.w * dsc_img->header.h;
    addr_start_alpha = len * sizeof(gt_color_t);
    total_len = (is_mask ? len : 0) + addr_start_alpha;

    if (byte_len < total_len) {
        GT_LOGV(GT_LOG_TAG_GUI, "Cache buffer color or alpha size is not enough to store image data");
        return GT_RES_FAIL;
    }
    if (GT_FS_RES_OK != gt_fs_read_img_offset(dsc_img->fp, cache_buffer, 0, total_len)) {
        GT_LOGW(GT_LOG_TAG_GUI, "Read image data failed");
        return GT_RES_FAIL;
    }

    raw->buffer = (gt_color_t * )cache_buffer;
    if (is_mask) {
        raw->opa = (gt_opa_t * )&cache_buffer[addr_start_alpha];
    }
    raw->width = dsc_img->header.w;
    raw->height = dsc_img->header.h;

    return GT_RES_OK;
}


/* global functions / API interface -------------------------------------*/
gt_color_img_raw_st gt_img_cache_get_raw(char * src, uint8_t * cache_buffer, uint32_t byte_len)
{
    gt_color_img_raw_st raw = {0};
    _gt_img_dsc_st dsc_img = {0};

    if (NULL == cache_buffer || 0 == byte_len) {
        GT_LOGV(GT_LOG_TAG_GUI, "Image cache buffer is NULL");
        return raw;
    }
    if (GT_RES_OK != gt_img_decoder_open(&dsc_img, src)) {
        GT_LOGW(GT_LOG_TAG_DATA, "[%s] Cache open image decoder failed", src);
        return raw;
    }

    _get_image_raw_data(&dsc_img, &raw, cache_buffer, byte_len);

    if (GT_RES_FAIL == gt_img_decoder_close(&dsc_img)) {
        GT_LOGW(GT_LOG_TAG_DATA, "[%s] Cache close image decoder failed", src);
    }
    return raw;
}

#if GT_USE_FILE_HEADER
gt_color_img_raw_st gt_img_cache_fh_get_raw(gt_file_header_param_st * fh, uint8_t * cache_buffer, uint32_t byte_len)
{
    gt_color_img_raw_st raw = {0};
    _gt_img_dsc_st dsc_img = {0};

    if (NULL == cache_buffer || 0 == byte_len) {
        GT_LOGV(GT_LOG_TAG_GUI, "Image cache buffer is NULL");
        return raw;
    }
    if (GT_RES_OK != gt_img_decoder_fh_open(&dsc_img, fh)) {
        GT_LOGW(GT_LOG_TAG_DATA, "[%d] File header cache open image decoder failed", fh->idx);
        return raw;
    }

    _get_image_raw_data(&dsc_img, &raw, cache_buffer, byte_len);

    if (GT_RES_FAIL == gt_img_decoder_close(&dsc_img)) {
        GT_LOGW(GT_LOG_TAG_DATA, "[%d] File header cache close image decoder failed", fh->idx);
    }
    return raw;
}
#endif

#if GT_USE_DIRECT_ADDR
gt_color_img_raw_st gt_img_cache_addr_get_raw(gt_addr_t addr, uint8_t * cache_buffer, uint32_t byte_len)
{
    gt_color_img_raw_st raw = {0};
    _gt_img_dsc_st dsc_img = {0};

    if (NULL == cache_buffer || 0 == byte_len) {
        GT_LOGV(GT_LOG_TAG_GUI, "Image cache buffer is NULL");
        return raw;
    }
    if (GT_RES_OK != gt_img_decoder_direct_addr_open(&dsc_img, addr)) {
        GT_LOGW(GT_LOG_TAG_DATA, "[0x%x] Address image cache open image decoder failed", addr);
        return raw;
    }

    _get_image_raw_data(&dsc_img, &raw, cache_buffer, byte_len);

    if (GT_RES_FAIL == gt_img_decoder_close(&dsc_img)) {
        GT_LOGW(GT_LOG_TAG_DATA, "[0x%x] Cache close image decoder failed", addr);
    }
    return raw;
}
#endif

/* end ------------------------------------------------------------------*/
#endif  /** GT_USE_IMG_CACHE */
