/**
 * @file gt_bin_convert.c
 * @author Feyoung
 * @brief bin hex buffer convert to target object struct or print to hex buffer
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

/* ------------------- convert api ------------------- */

uint64_t  gt_convert_parse_by_len(uint8_t const * const buffer, uint8_t len)
{
    uint32_t val = 0;
    for (uint8_t i = 0; i < len; ++i) {
        val <<= 8;
        val |= buffer[i] & 0xff;
    }
    return val;
}

uint16_t gt_convert_print_by_len(uint8_t * const res, uint64_t val, uint8_t len)
{
    for (uint8_t i = 0; i < len; ++i) {
        res[i] = (val >> ((len - 1 - i) * 8)) & 0xff;
    }
    return len;
}

uint16_t gt_convert_parse_u16(uint8_t const * const buffer)
{
    return (buffer[0] << 8) | buffer[1];
}

uint16_t gt_convert_print_u16(uint8_t * const res, uint16_t val)
{
    res[0] = (val >> 8) & 0xff;
    res[1] = val & 0xff;
    return 2;
}

uint32_t gt_convert_parse_u32(uint8_t const * const buffer)
{
    return (uint32_t)gt_convert_parse_by_len(buffer, 4);
}

uint16_t gt_convert_print_u32(uint8_t * const res, uint32_t val)
{
    return gt_convert_print_by_len(res, val, 4);
}

float gt_convert_parse_float(uint8_t const * const buffer)
{
    uint32_t val = 0;
    for (uint8_t i = 0; i < 4; ++i) {
        val = (val << 8) | buffer[i];
    }
    return *(float*)&val;
}

double gt_convert_parse_double(uint8_t const * const buffer)
{
    uint64_t val = 0;
    for (uint8_t i = 0; i < 8; ++i) {
        val = (val << 8) | buffer[i];
    }
    return *(double*)&val;
}

int16_t gt_convert_parse_i16(uint8_t const * const buffer)
{
    return (int16_t)gt_convert_parse_u16(buffer);
}

int32_t gt_convert_parse_i32(uint8_t const * const buffer)
{
    return (int32_t)gt_convert_parse_u32(buffer);
}

int64_t gt_convert_parse_i64(uint8_t const * const buffer)
{
    uint64_t val = 0;
    for (uint8_t i = 0; i < 8; ++i) {
        val <<= 8;
        val |= buffer[i] & 0xff;
    }
    return (int64_t)val;
}
/* end ------------------------------------------------------------------*/
#endif  /** GT_USE_BIN_CONVERT */
