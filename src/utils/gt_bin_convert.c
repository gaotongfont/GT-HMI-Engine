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

gt_res_t gt_bin_convert_parse(gt_bin_convert_st const * const bin, gt_bin_item_st const * const list)
{
    uint32_t offset = 0;
    uint16_t i = 0;

    if (NULL == bin) { return GT_RES_FAIL; }
    if (NULL == bin->buffer) { return GT_RES_FAIL; }
    if (0 == bin->byte_len) { return GT_RES_FAIL; }

    for (i = 0; list[i].len; ++i) {
        if (offset > bin->byte_len) { return GT_RES_INV; }
        list[i].parse_cb(bin->tar, bin->buffer + offset, list[i].len);
        offset += list[i].len;
    }
    return GT_RES_OK;
}

gt_res_t gt_bin_convert_print(gt_bin_convert_st const * const bin, gt_bin_item_st const * const list)
{
    uint32_t offset = 0;
    uint16_t i = 0;

    if (NULL == bin) { return GT_RES_FAIL; }
    if (NULL == bin->buffer) { return GT_RES_FAIL; }
    if (0 == bin->byte_len) { return GT_RES_FAIL; }

    for (i = 0; list[i].len; ++i) {
        if (offset > bin->byte_len) { return GT_RES_INV; }
        list[i].print_cb(bin->tar, bin->buffer + offset, list[i].len);
        offset += list[i].len;
    }
    return GT_RES_OK;
}

/* end ------------------------------------------------------------------*/
#endif  /** GT_USE_BIN_CONVERT */
