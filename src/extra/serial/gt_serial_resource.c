/**
 * @file gt_serial_resource.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-11-11 18:03:08
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_serial_resource.h"

#if GT_USE_SERIAL && GT_USE_BIN_CONVERT
#include "../../core/gt_mem.h"
#include "../../core/gt_fs.h"
#include "../../utils/gt_bin_convert.h"


/* private define -------------------------------------------------------*/

#define _SERIAL_RESOURCE_FILE_COUNT_OFFSET   (5)

#define GT_SERIAL_RESOURCE_FILE_LENGTH      (32)


/* private typedef ------------------------------------------------------*/
typedef struct {
    uint32_t addr;
    uint32_t byte_len;
    uint8_t id;
    uint8_t checksum[16];
}_serial_bin_item_st;

typedef struct {
    uint8_t file_count;
    _serial_bin_item_st * items_list;   /** NULL:  */
}_serial_resource_st;



/* static variables -----------------------------------------------------*/
static _serial_resource_st _serial_res = {
    .file_count = 0,
    .items_list = NULL,
};


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static gt_res_t _get_resource_data_by(uint32_t addr, uint8_t file_count, _serial_bin_item_st * res_list_p) {
    if (NULL == res_list_p) {
        return GT_RES_FAIL;
    }
    uint16_t length = file_count * GT_SERIAL_RESOURCE_FILE_LENGTH;
    uint8_t * buffer = gt_mem_malloc(length);
    if (NULL == buffer) {
        return GT_RES_FAIL;
    }
    gt_fs_read_direct_physical(addr, length, buffer);

    uint8_t * ptr = buffer;
    for (uint16_t k = 0; k < file_count; ++k) {
        res_list_p[k].id = ptr[0];
        res_list_p[k].addr = gt_convert_parse_u32(ptr + 1);
        res_list_p[k].byte_len = gt_convert_parse_u32(ptr + 5);
        gt_memcpy(res_list_p[k].checksum, ptr + 9, 16);
        ptr += GT_SERIAL_RESOURCE_FILE_LENGTH;
    }

    gt_mem_free(buffer);
    buffer = NULL;
    return GT_RES_OK;
}



/* global functions / API interface -------------------------------------*/
gt_res_t gt_serial_resource_init(void)
{
    if (NULL != _serial_res.items_list) {
        return GT_RES_INV;
    }
    uint32_t addr = GT_VF_FLASH_START;
    uint8_t tmp[GT_SERIAL_RESOURCE_FILE_LENGTH] = {0};
    gt_fs_read_direct_physical(addr, 16, tmp);  /** get file count */
    addr += 16;
    _serial_res.file_count = tmp[_SERIAL_RESOURCE_FILE_COUNT_OFFSET];
    _serial_res.items_list = (_serial_bin_item_st *)gt_mem_malloc(_serial_res.file_count * sizeof(_serial_bin_item_st));
    if (NULL == _serial_res.items_list) {
        return GT_RES_FAIL;
    }
    if (GT_RES_OK != _get_resource_data_by(addr, _serial_res.file_count, _serial_res.items_list)) {
        gt_mem_free(_serial_res.items_list);
        _serial_res.items_list = NULL;
        return GT_RES_FAIL;
    }
    GT_LOGV(GT_LOG_TAG_SERIAL, "resource file count: %d", _serial_res.file_count);
    return GT_RES_OK;
}

uint32_t gt_serial_resource_get_addr_by(uint8_t file_id)
{
    if (NULL == _serial_res.items_list) {
        return GT_SERIAL_RESOURCE_INVALID_ADDR;
    }
    for (uint16_t k = 0; k < _serial_res.file_count; ++k) {
        if (file_id == _serial_res.items_list[k].id) {
            return _serial_res.items_list[k].addr;
        }
    }
    return GT_SERIAL_RESOURCE_INVALID_ADDR;
}


/* end ------------------------------------------------------------------*/
#endif  /** GT_USE_SERIAL */
