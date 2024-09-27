/**
 * @file gt_serial_system.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-09-27 15:36:55
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_serial_system.h"

#if GT_USE_SERIAL && GT_USE_BIN_CONVERT
#include "../utils/gt_bin_convert.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/

typedef gt_res_t ( * modified_cb_t)(void * const, uint8_t const * const , uint32_t);

typedef struct {
    uint16_t addr;
    uint16_t len;
    modified_cb_t modified_cb;
}_serial_system_st;


/* static variables -----------------------------------------------------*/
static GT_ATTRIBUTE_LARGE_RAM_ARRAY const _serial_system_st _system_list[] = {
    { 0, 4, (modified_cb_t)NULL },

};


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_res_t gt_serial_system_set_value(uint8_t const * const buffer, uint32_t len)
{
    uint16_t addr = gt_convert_parse_u16(buffer);
    uint32_t tmp_len = len - 2;

    return GT_RES_OK;
}



/* end ------------------------------------------------------------------*/
#endif /** GT_USE_SERIAL && GT_USE_SERIAL_SYSTEM */
