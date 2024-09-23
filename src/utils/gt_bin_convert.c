/**
 * @file gt_bin_convert.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-09-23 11:47:54
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_bin_convert.h"

#if GT_USE_BIN_CONVERT

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

gt_bin_res_st gt_bin_convert_parse(gt_bin_convert_st const * const bin, gt_bin_item_st const * const list)
{
    uint32_t offset = 0;
    uint16_t i = 0;
    gt_bin_res_st ret = {
        .res = GT_RES_FAIL,
        .index = 0
    };

    if (NULL == bin) { return ret; }
    if (NULL == bin->buffer) { return ret; }
    if (0 == bin->byte_len) { return ret; }

    for (i = 0; list[i].len; ++i) {
        if (offset > bin->byte_len) {
            ret.res = GT_RES_INV;
            ret.index = i;
            return ret;
        }
        ret.res = list[i].parse_cb(bin->tar, bin->buffer + offset, list[i].len);
        if (GT_RES_OK != ret.res) {
            ret.index = i;
            return ret;
        }
        offset += list[i].len;
    }
    return ret;
}

gt_bin_res_st gt_bin_convert_print(gt_bin_convert_st const * const bin, gt_bin_item_st const * const list)
{
    uint32_t offset = 0;
    uint16_t i = 0;
    gt_bin_res_st ret = {
        .res = GT_RES_FAIL,
        .index = 0
    };

    if (NULL == bin) { return ret; }
    if (NULL == bin->buffer) { return ret; }
    if (0 == bin->byte_len) { return ret; }

    for (i = 0; list[i].len; ++i) {
        if (offset > bin->byte_len) {
            ret.res = GT_RES_INV;
            ret.index = i;
            return ret;
        }
        ret.res = list[i].print_cb(bin->tar, bin->buffer + offset, list[i].len);
        if (GT_RES_OK != ret.res) {
            ret.index = i;
            return ret;
        }
        offset += list[i].len;
    }
    return ret;
}

/* end ------------------------------------------------------------------*/
#endif  /** GT_USE_BIN_CONVERT */
