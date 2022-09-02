/**
 * @file gt_img_decoder.c
 * @author Feyoung
 * @brief default
 * @version 0.1
 * @date 2022-08-10 19:52:55
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
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
static gt_res_t _gt_img_decoder_built_in_info(struct _gt_img_decoder_s * decoder, const void * src, _gt_img_info_st * header)
{
    gt_fs_fp_st * fp = gt_fs_open(src, GT_FS_MODE_RD);
    if (!fp) {
        return GT_RES_INV;
    }

    header->w = fp->msg.pic.w;
    header->h = fp->msg.pic.h;

    return GT_RES_OK;
}

static gt_res_t _gt_img_decoder_built_in_open(struct _gt_img_decoder_s * decoder, struct _gt_img_dsc_s * dsc)
{
    GT_UNUSED(decoder);

    gt_fs_fp_st * fp = gt_fs_open(dsc->src, GT_FS_MODE_RD);
    if (!fp) {
        return GT_RES_INV;
    }

    dsc->header.w = fp->msg.pic.w;
    dsc->header.h = fp->msg.pic.h;

    return GT_RES_OK;
}

static gt_res_t _gt_img_decoder_built_in_read_line(struct _gt_img_decoder_s * decoder, struct _gt_img_dsc_s * dsc,
                                                gt_size_t x, gt_size_t y, gt_size_t len, uint8_t * buffer)
{
    gt_fs_fp_st * fp = gt_fs_get_fp();
    if (!fp) {
        return GT_RES_INV;
    }
    uint16_t line_pixel = (dsc->header.w * dsc->header.h) * y;
    uint32_t offset = ( line_pixel + x ) * sizeof(gt_color_t);

    if (gt_fs_seek(offset, GT_FS_SEEK_SET)) {
        return GT_RES_FAIL;
    }

    uint32_t ret_size = 0;
    if (gt_fs_read(buffer, line_pixel * sizeof(gt_color_t), &ret_size)) {
        return GT_RES_FAIL;
    }

    return GT_RES_OK;
}

static gt_res_t _gt_img_decoder_built_in_close(struct _gt_img_decoder_s * decoder, struct _gt_img_dsc_s * dsc)
{
    GT_UNUSED(decoder);
    GT_UNUSED(dsc);

    gt_fs_close();

    return GT_RES_OK;
}



/* global functions / API interface -------------------------------------*/

/**
 * @brief init decoder collection
 */
void _gt_img_decoder_init(void)
{
    GT_INIT_LIST_HEAD(&_GT_GC_GET_ROOT(_gt_img_decoder_ll));

    _gt_img_decoder_st * decoder = gt_img_decoder_create();
    GT_CHECK(decoder, (void)0);

    decoder->info_cb = _gt_img_decoder_built_in_info;
    gt_img_decoder_set_info_cb(decoder, _gt_img_decoder_built_in_info);
    gt_img_decoder_set_open_cb(decoder, _gt_img_decoder_built_in_open);
    gt_img_decoder_set_read_line_cb(decoder, _gt_img_decoder_built_in_read_line);
    gt_img_decoder_set_close_cb(decoder, _gt_img_decoder_built_in_close);

    gt_img_decoder_register(decoder);
}

/**
 * @brief create a gt_img decoder object
 *
 * @return _gt_img_decoder_st* decoder malloc memory pointer
 */
_gt_img_decoder_st * gt_img_decoder_create(void)
{
    _gt_img_decoder_st * decoder = (_gt_img_decoder_st * )gt_mem_malloc(sizeof(_gt_img_decoder_st));
    GT_CHECK(decoder, NULL);

    gt_memset(decoder, 0, sizeof(_gt_img_decoder_st));

    GT_INIT_LIST_HEAD(&decoder->list);

    return decoder;
}

void gt_img_decoder_register(_gt_img_decoder_st * decoder)
{
    gt_list_add(&decoder->list, &_GT_GC_GET_ROOT(_gt_img_decoder_ll));
}

gt_res_t gt_img_decoder_get_info(const char * name, _gt_img_info_st * header)
{
    _gt_img_decoder_st * ptr = NULL;

    gt_list_for_each_entry(ptr, &_GT_GC_GET_ROOT(_gt_img_decoder_ll), list) {
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

gt_res_t gt_img_decoder_open(_gt_img_dsc_st * dsc, const char * const name, gt_size_t frame_id)
{
    _gt_img_decoder_st * ptr = NULL;

    gt_list_for_each_entry(ptr, &_GT_GC_GET_ROOT(_gt_img_decoder_ll), list) {
        if (NULL == ptr->open_cb || NULL == ptr->info_cb) {
            continue;
        }

        if (ptr->info_cb(ptr, name, &dsc->header)) {
            continue;
        }

        if (ptr->open_cb(ptr, dsc)) {
            continue;
        }

        gt_memset_0(&dsc->header, sizeof(_gt_img_info_st));

        dsc->decoder = ptr;
        dsc->frame_id = frame_id;
        dsc->src = (void * )name;
        dsc->msg = NULL;

        break;
    }

    return GT_RES_OK;
}

gt_res_t gt_img_decoder_read_line(_gt_img_dsc_st * dsc, gt_size_t x, gt_size_t y, gt_size_t len, uint8_t * buf)
{
    if (NULL == dsc->decoder->read_line_cb) {
        return GT_RES_INV;
    }

    if (dsc->decoder->read_line_cb(dsc->decoder, dsc, x, y, len, buf)) {
        return GT_RES_INV;
    }

    return GT_RES_OK;
}

gt_res_t gt_img_decoder_close(struct _gt_img_decoder_s * decoder, _gt_img_dsc_st * dsc)
{
    if (NULL == dsc->decoder->close_cb) {
        return GT_RES_OK;
    }
    return dsc->decoder->close_cb(decoder, dsc);
}

void gt_img_decoder_set_info_cb(_gt_img_decoder_st * decoder, gt_img_decoder_get_info_t info_cb)
{
    decoder->info_cb = info_cb;
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
