/**
 * @file gt_img_decoder.c
 * @author Feyoung
 * @brief The image decoder which need to register the image file type by
 *      extra file type.
 * @version 0.1
 * @date 2022-08-10 19:52:55
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_img_decoder.h"
#include "../others/gt_gc.h"
#include "../others/gt_log.h"
#include "./gt_fs.h"
#include "./gt_mem.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

/**
 * @brief use default constructor for image decoder
 *
 * @param decoder
 * @param src
 * @param header
 * @return gt_res_t
 */
static GT_ATTRIBUTE_RAM_TEXT gt_res_t _gt_img_decoder_built_in_info(struct _gt_img_decoder_s * decoder, const void * src, _gt_img_info_st * header)
{
    GT_UNUSED(decoder);
    gt_fs_fp_st * fp = gt_fs_open(src, GT_FS_MODE_RD);
    if (!fp) {
        return GT_RES_INV;
    }

    header->w = fp->msg.pic.w;
    header->h = fp->msg.pic.h;

    gt_fs_close(fp);

    return GT_RES_OK;
}

/**
 * @brief use default constructor for image decoder
 *
 * @param decoder
 * @param src
 * @param header
 * @return gt_res_t
 */
static GT_ATTRIBUTE_RAM_TEXT gt_res_t _gt_img_decoder_built_in_info_raw(struct _gt_img_decoder_s * decoder, const void * raw_data, uint32_t raw_len, _gt_img_info_st * header)
{
    GT_UNUSED(decoder);
    gt_fs_fp_st * fp = gt_fs_open_raw(raw_data, raw_len, GT_FS_MODE_RD);
    if (!fp) {
        return GT_RES_INV;
    }

    header->w = fp->msg.pic.w;
    header->h = fp->msg.pic.h;

    gt_fs_close(fp);

    return GT_RES_OK;
}

static GT_ATTRIBUTE_RAM_TEXT gt_res_t _gt_img_decoder_built_in_open(struct _gt_img_decoder_s * decoder, struct _gt_img_dsc_s * dsc)
{
    GT_UNUSED(decoder);
    gt_fs_fp_st * fp = NULL;
    if (dsc->src) {
        fp = gt_fs_open(dsc->src, GT_FS_MODE_RD);
    } else if (dsc->raw_p) {
        fp = gt_fs_open_raw(dsc->raw_p, dsc->raw_len, GT_FS_MODE_RD);
    }
    if (!fp) {
        return GT_RES_INV;
    }

    dsc->fp = fp;
    dsc->header.w = fp->msg.pic.w;
    dsc->header.h = fp->msg.pic.h;

    return GT_RES_OK;
}

static GT_ATTRIBUTE_RAM_TEXT gt_res_t _gt_img_decoder_built_in_read_line(struct _gt_img_dsc_s * dsc,
                                                gt_size_t x, gt_size_t y, gt_size_t len, uint8_t * buffer)
{
    gt_fs_fp_st * fp = dsc->fp;
    if (!fp) {
        return GT_RES_INV;
    }
    uint16_t line_pixel = dsc->header.w * y;
    uint32_t offset = ( line_pixel + x ) * sizeof(gt_color_t);

    if (gt_fs_seek(fp, offset, GT_FS_SEEK_SET)) {
        return GT_RES_FAIL;
    }

    uint32_t ret_size = 0;
    if (gt_fs_read(fp, buffer, line_pixel * sizeof(gt_color_t), &ret_size)) {
        return GT_RES_FAIL;
    }

    return GT_RES_OK;
}

static GT_ATTRIBUTE_RAM_TEXT gt_res_t _gt_img_decoder_built_in_close(struct _gt_img_dsc_s * dsc)
{
    gt_fs_close(dsc->fp);

    return GT_RES_OK;
}

#if GT_USE_FILE_HEADER
static GT_ATTRIBUTE_RAM_TEXT gt_res_t _gt_img_decoder_built_in_fh_info(struct _gt_img_decoder_s * decoder, gt_file_header_param_st const * const param, _gt_img_info_st * header) {
    GT_UNUSED(decoder);
    gt_fs_fp_st * fp = gt_fs_fh_open(param, GT_FS_MODE_RD);
    if (!fp) {
        return GT_RES_INV;
    }
    header->w = fp->msg.pic.w;
    header->h = fp->msg.pic.h;

    gt_fs_close(fp);

    return GT_RES_OK;
}

gt_res_t _gt_img_decoder_build_in_fh_open(struct _gt_img_decoder_s * decoder, struct _gt_img_dsc_s * dsc) {
    GT_UNUSED(decoder);
    gt_fs_fp_st * fp = gt_fs_fh_open(dsc->file_header, GT_FS_MODE_RD);
    if (!fp) {
        return GT_RES_INV;
    }
    dsc->fp = fp;
    dsc->header.w = fp->msg.pic.w;
    dsc->header.h = fp->msg.pic.h;

    return GT_RES_OK;
}
#endif

#if GT_USE_DIRECT_ADDR
static GT_ATTRIBUTE_RAM_TEXT gt_res_t _gt_img_decoder_built_in_direct_addr_info(struct _gt_img_decoder_s * decoder, gt_addr_t addr, _gt_img_info_st * header) {
    GT_UNUSED(decoder);
    gt_fs_fp_st * fp = gt_fs_direct_addr_open(addr, GT_FS_MODE_RD);
    if (!fp) {
        return GT_RES_INV;
    }
    header->w = fp->msg.pic.w;
    header->h = fp->msg.pic.h;

    gt_fs_close(fp);

    return GT_RES_OK;
}

gt_res_t _gt_img_decoder_build_in_direct_addr_open(struct _gt_img_decoder_s * decoder, struct _gt_img_dsc_s * dsc) {
    GT_UNUSED(decoder);
    gt_fs_fp_st * fp = gt_fs_direct_addr_open(dsc->addr, GT_FS_MODE_RD);
    if (!fp) {
        return GT_RES_INV;
    }
    dsc->fp = fp;
    dsc->header.w = fp->msg.pic.w;
    dsc->header.h = fp->msg.pic.h;

    return GT_RES_OK;
}
#endif

/* global functions / API interface -------------------------------------*/

void _gt_img_decoder_init(void)
{
    _GT_INIT_LIST_HEAD(&_GT_GC_GET_ROOT(_gt_img_decoder_ll));

    _gt_img_decoder_st * decoder = gt_img_decoder_create();

    gt_img_decoder_set_info_cb(decoder, _gt_img_decoder_built_in_info);
    gt_img_decoder_set_info_raw_cb(decoder, _gt_img_decoder_built_in_info_raw);
    gt_img_decoder_set_open_cb(decoder, _gt_img_decoder_built_in_open);
    gt_img_decoder_set_read_line_cb(decoder, _gt_img_decoder_built_in_read_line);
    gt_img_decoder_set_close_cb(decoder, _gt_img_decoder_built_in_close);

#if GT_USE_FILE_HEADER
    gt_img_decoder_set_fh_info_cb(decoder, _gt_img_decoder_built_in_fh_info);
    gt_img_decoder_set_fh_open_cb(decoder, _gt_img_decoder_build_in_fh_open);
#endif

#if GT_USE_DIRECT_ADDR
    gt_img_decoder_set_direct_addr_info_cb(decoder, _gt_img_decoder_built_in_direct_addr_info);
    gt_img_decoder_set_direct_addr_open_cb(decoder, _gt_img_decoder_build_in_direct_addr_open);
#endif

    gt_img_decoder_register(decoder);
}

_gt_img_decoder_st * gt_img_decoder_create(void)
{
    _gt_img_decoder_st * decoder = (_gt_img_decoder_st * )gt_mem_malloc(sizeof(_gt_img_decoder_st));
    if (!decoder) {
        return NULL;
    }

    gt_memset(decoder, 0, sizeof(_gt_img_decoder_st));

    _GT_INIT_LIST_HEAD(&decoder->list);

    return decoder;
}

void gt_img_decoder_register(_gt_img_decoder_st * decoder)
{
    _gt_list_add(&decoder->list, &_GT_GC_GET_ROOT(_gt_img_decoder_ll));
}

gt_img_decoder_type_em gt_img_decoder_get_type(_gt_img_info_st * header)
{
    return (gt_img_decoder_type_em)header->type;
}

gt_res_t gt_img_decoder_get_info(const char * name, _gt_img_info_st * header)
{
    _gt_img_decoder_st * ptr = NULL;

    _gt_list_for_each_entry(ptr, &_GT_GC_GET_ROOT(_gt_img_decoder_ll), _gt_img_decoder_st, list) {
        if (NULL == ptr->info_cb) {
            continue;
        }
        if (ptr->info_cb(ptr, name, header)) {
            continue;
        }
        break;
    }
    return GT_RES_OK;
}

gt_res_t gt_img_decoder_get_info_raw(const char * raw_data, uint32_t raw_len, _gt_img_info_st * header)
{
    _gt_img_decoder_st * ptr = NULL;

    _gt_list_for_each_entry(ptr, &_GT_GC_GET_ROOT(_gt_img_decoder_ll), _gt_img_decoder_st, list) {
        if (NULL == ptr->info_raw_cb) {
            continue;
        }
        if (ptr->info_raw_cb(ptr, raw_data, raw_len, header)) {
            continue;
        }
        break;
    }
    return GT_RES_OK;
}

gt_res_t gt_img_decoder_open(_gt_img_dsc_st * dsc, const char * const name)
{
    _gt_img_decoder_st * ptr = NULL;
    dsc->decoder = NULL;    /* reset image dsc struct */
    dsc->raw_p = NULL;
    dsc->src = NULL;

    _gt_list_for_each_entry(ptr, &_GT_GC_GET_ROOT(_gt_img_decoder_ll), _gt_img_decoder_st, list) {
        if (NULL == ptr->open_cb || NULL == ptr->info_cb) {
            continue;
        }

        if (ptr->info_cb(ptr, name, &dsc->header)) {
            gt_memset(&dsc->header, 0, sizeof(_gt_img_info_st));
            continue;
        }

        dsc->src = (void * )name;
        if (ptr->open_cb(ptr, dsc)) {
            continue;
        }

        dsc->decoder = ptr;

        break;
    }

    return dsc->decoder ? GT_RES_OK : GT_RES_FAIL;
}

gt_res_t gt_img_decoder_open_raw(_gt_img_dsc_st * dsc, const char * const raw_data, uint32_t raw_len)
{
    _gt_img_decoder_st * ptr = NULL;
    dsc->decoder = NULL;    /* reset image dsc struct */
    dsc->raw_p = NULL;
    dsc->src = NULL;

    _gt_list_for_each_entry(ptr, &_GT_GC_GET_ROOT(_gt_img_decoder_ll), _gt_img_decoder_st, list) {
        if (NULL == ptr->open_cb || NULL == ptr->info_raw_cb) {
            continue;
        }

        if (ptr->info_raw_cb(ptr, raw_data, raw_len, &dsc->header)) {
            gt_memset(&dsc->header, 0, sizeof(_gt_img_info_st));
            continue;
        }

        dsc->raw_p = (char * )raw_data;
        dsc->raw_len = raw_len;
        if (ptr->open_cb(ptr, dsc)) {
            continue;
        }

        dsc->decoder = ptr;

        break;
    }

    return dsc->decoder ? GT_RES_OK : GT_RES_FAIL;
}

gt_res_t gt_img_decoder_read_line(_gt_img_dsc_st * dsc, gt_size_t x, gt_size_t y, gt_size_t len, uint8_t * buf)
{
    GT_CHECK_BACK_VAL(dsc, GT_RES_INV);
    GT_CHECK_BACK_VAL(dsc->decoder, GT_RES_INV);
    GT_CHECK_BACK_VAL(dsc->decoder->read_line_cb, GT_RES_INV);

    if (dsc->decoder->read_line_cb(dsc, x, y, len, buf)) {
        return GT_RES_INV;
    }

    return GT_RES_OK;
}

gt_res_t gt_img_decoder_close(_gt_img_dsc_st * dsc)
{
    GT_CHECK_BACK_VAL(dsc, GT_RES_INV);
    GT_CHECK_BACK_VAL(dsc->decoder, GT_RES_INV);
    GT_CHECK_BACK_VAL(dsc->decoder->close_cb, GT_RES_OK);
    return dsc->decoder->close_cb(dsc);
}

#if GT_USE_FILE_HEADER
gt_res_t gt_img_decoder_fh_get_info(gt_file_header_param_st const * const param, _gt_img_info_st * header)
{
    _gt_img_decoder_st * ptr = NULL;
    if (NULL == param || param->idx < 0) {
        return GT_RES_INV;
    }

    _gt_list_for_each_entry(ptr, &_GT_GC_GET_ROOT(_gt_img_decoder_ll), _gt_img_decoder_st, list) {
        if (NULL == ptr->fh_info_cb) {
            continue;
        }

        if (ptr->fh_info_cb(ptr, param, header)) {
            continue;
        }

        break;
    }

    return GT_RES_OK;
}

gt_res_t gt_img_decoder_fh_open(_gt_img_dsc_st * dsc, gt_file_header_param_st const * const param)
{
    _gt_img_decoder_st * ptr = NULL;
    dsc->decoder = NULL;    /* reset image dsc struct */
    if (NULL == param || param->idx < 0) {
        return GT_RES_INV;
    }

    _gt_list_for_each_entry(ptr, &_GT_GC_GET_ROOT(_gt_img_decoder_ll), _gt_img_decoder_st, list) {
        if (NULL == ptr->fh_open_cb || NULL == ptr->fh_info_cb) {
            continue;
        }

        if (ptr->fh_info_cb(ptr, param, &dsc->header)) {
            gt_memset(&dsc->header, 0, sizeof(_gt_img_info_st));
            continue;
        }

        dsc->file_header = (gt_file_header_param_st * )param;
        if (ptr->fh_open_cb(ptr, dsc)) {
            continue;
        }

        dsc->decoder = ptr;

        break;
    }

    return dsc->decoder ? GT_RES_OK : GT_RES_FAIL;
}

void gt_img_decoder_set_fh_info_cb(_gt_img_decoder_st * decoder, gt_img_decoder_fh_get_info_t fh_info_cb)
{
    decoder->fh_info_cb = fh_info_cb;
}

void gt_img_decoder_set_fh_open_cb(_gt_img_decoder_st * decoder, gt_img_decoder_fh_open_t fh_open_cb)
{
    decoder->fh_open_cb = fh_open_cb;
}
#endif

#if GT_USE_DIRECT_ADDR
gt_res_t gt_img_decoder_direct_addr_get_info(gt_addr_t addr, _gt_img_info_st * header)
{
    _gt_img_decoder_st * ptr = NULL;
    if (gt_hal_is_invalid_addr(addr)) {
        return GT_RES_INV;
    }
    _gt_list_for_each_entry(ptr, &_GT_GC_GET_ROOT(_gt_img_decoder_ll), _gt_img_decoder_st, list) {
        if (NULL == ptr->direct_addr_info_cb) {
            continue;
        }

        if (ptr->direct_addr_info_cb(ptr, addr, header)) {
            continue;
        }

        break;
    }

    return GT_RES_OK;
}

gt_res_t gt_img_decoder_direct_addr_open(_gt_img_dsc_st * dsc, gt_addr_t addr)
{
    _gt_img_decoder_st * ptr = NULL;
    if (gt_hal_is_invalid_addr(addr)) {
        return GT_RES_INV;
    }
    dsc->decoder = NULL;    /* reset image dsc struct */

    _gt_list_for_each_entry(ptr, &_GT_GC_GET_ROOT(_gt_img_decoder_ll), _gt_img_decoder_st, list) {
        if (NULL == ptr->direct_addr_open_cb || NULL == ptr->direct_addr_info_cb) {
            continue;
        }

        if (ptr->direct_addr_info_cb(ptr, addr, &dsc->header)) {
            gt_memset(&dsc->header, 0, sizeof(_gt_img_info_st));
            continue;
        }

        dsc->addr = addr;
        if (ptr->direct_addr_open_cb(ptr, dsc)) {
            continue;
        }

        dsc->decoder = ptr;

        break;
    }

    return dsc->decoder ? GT_RES_OK : GT_RES_FAIL;
}

void gt_img_decoder_set_direct_addr_info_cb(_gt_img_decoder_st * decoder, gt_img_decoder_direct_addr_get_info_t direct_addr_info_cb)
{
    decoder->direct_addr_info_cb = direct_addr_info_cb;
}

void gt_img_decoder_set_direct_addr_open_cb(_gt_img_decoder_st * decoder, gt_img_decoder_direct_addr_open_t direct_addr_open_cb)
{
    decoder->direct_addr_open_cb = direct_addr_open_cb;
}
#endif


#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
gt_res_t gt_img_decoder_custom_size_addr_get_info(gt_direct_addr_custom_size_st * dac, _gt_img_info_st * header)
{
    if (gt_hal_is_invalid_custom_size_addr(dac)) {
        return GT_RES_INV;
    }
    gt_fs_fp_st * fp = gt_fs_custom_size_addr_open(dac, GT_FS_MODE_RD);
    if (!fp) {
        return GT_RES_INV;
    }
    header->w = fp->msg.pic.w;
    header->h = fp->msg.pic.h;

    gt_fs_close(fp);

    return GT_RES_OK;
}

gt_res_t gt_img_decoder_custom_size_addr_open(_gt_img_dsc_st * dsc, gt_direct_addr_custom_size_st * dac)
{
    if (gt_hal_is_invalid_custom_size_addr(dac)) {
        return GT_RES_INV;
    }
    gt_fs_fp_st * fp = gt_fs_custom_size_addr_open(dac, GT_FS_MODE_RD);
    if (!fp) {
        return GT_RES_INV;
    }
    dsc->fp = fp;
    dsc->header.w = fp->msg.pic.w;
    dsc->header.h = fp->msg.pic.h;

    return GT_RES_OK;
}

gt_res_t gt_img_decoder_custom_size_addr_close(_gt_img_dsc_st * dsc)
{
    GT_CHECK_BACK_VAL(dsc, GT_RES_INV);
    if (dsc->fp) {
        gt_fs_close(dsc->fp);
        dsc->fp = NULL;
    }
    return GT_RES_OK;
}
#endif

void gt_img_decoder_set_info_cb(_gt_img_decoder_st * decoder, gt_img_decoder_get_info_t info_cb)
{
    decoder->info_cb = info_cb;
}

void gt_img_decoder_set_info_raw_cb(_gt_img_decoder_st * decoder, gt_img_decoder_get_info_raw_t info_raw_cb)
{
    decoder->info_raw_cb = info_raw_cb;
}

void gt_img_decoder_set_open_cb(_gt_img_decoder_st * decoder, gt_img_decoder_open_t open_cb)
{
    decoder->open_cb = open_cb;
}

void gt_img_decoder_set_read_line_cb(_gt_img_decoder_st * decoder, gt_img_decoder_read_line_t read_line_cb)
{
    decoder->read_line_cb = read_line_cb;
}

void gt_img_decoder_set_close_cb(_gt_img_decoder_st * decoder, gt_img_decoder_close_t close_cb)
{
    decoder->close_cb = close_cb;
}

/* end ------------------------------------------------------------------*/
