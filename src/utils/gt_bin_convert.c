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
uint32_t gt_bin_get_total_byte_length(gt_bin_item_st const * const list)
{
    uint32_t total = 0;
    uint16_t i = 0;

    for (i = 0; list[i].len; ++i) {
        total += list[i].len;
    }
    return total;
}

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
        if (list[i].parse_cb) {
            ret.res = list[i].parse_cb(bin->tar, bin->buffer + offset, list[i].len);
        }
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
        if (list[i].print_cb) {
            ret.res = list[i].print_cb(bin->tar, bin->buffer + offset, list[i].len);
        }
        if (GT_RES_OK != ret.res) {
            ret.index = i;
            return ret;
        }
        offset += list[i].len;
    }
    return ret;
}

uint16_t gt_convert_parse_short(uint8_t const * const buffer)
{
    uint16_t val = 0;
    for (uint8_t i = 0; i < 2; ++i) {
        val <<= 8;
        val |= buffer[i];
    }
    return val;
}

uint16_t gt_convert_print_short(uint8_t * const res, uint16_t val)
{
    for (uint8_t i = 0; i < 2; ++i) {
        res[i] = (val >> ((1 - i) * 8)) & 0xff;
    }
    return 2;
}


/* end ------------------------------------------------------------------*/
#endif  /** GT_USE_BIN_CONVERT */
