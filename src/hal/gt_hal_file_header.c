/**
 * @file gt_hal_file_header.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-05-10 14:33:53
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_hal_file_header.h"

#if GT_USE_FILE_HEADER
#include "../core/gt_mem.h"
#include "../others/gt_log.h"
#include "../others/gt_types.h"
#include "../others/gt_gc.h"
#include "stdbool.h"
#include "string.h"



/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

uint16_t gt_file_header_get_instance_size(void)
{
    return sizeof(gt_file_header_st);
}

#if _GT_FILE_HEADER_IMG_SAME_SIZE
void gt_file_header_init(gt_file_header_st const * header_list, uint16_t count, gt_file_header_color_info_st * info)
{
    _gt_file_header_ctl_st * ctl = &_GT_GC_GET_ROOT(file_header_ctl);
    ctl->header = header_list;
    ctl->count = count;
    ctl->info = *info;
}
#else
void gt_file_header_init(gt_file_header_st const * header_list, uint16_t count)
{
    _gt_file_header_ctl_st * ctl = &_GT_GC_GET_ROOT(file_header_ctl);
    ctl->header = header_list;
    ctl->count = count;
}
#endif

uint16_t gt_file_header_get_count(void)
{
    _gt_file_header_ctl_st const * ctl = &_GT_GC_GET_ROOT(file_header_ctl);

    return ctl->count;
}

void gt_file_header_param_init(gt_file_header_param_st * fh)
{
    if (NULL == fh) {
        return ;
    }
    fh->idx = GT_FILE_HEADER_INVALID_IDX;
#if _GT_FILE_HEADER_IMG_PACKAGE
    fh->package_idx = 0;
#endif
}

gt_file_header_st const * gt_file_header_get(gt_file_header_idx_t index)
{
    _gt_file_header_ctl_st const * ctl = &_GT_GC_GET_ROOT(file_header_ctl);
    if (NULL == ctl->header || 0 == ctl->count) {
        return NULL;
    }
    if (index < 0) {
        return NULL;
    }

#if _GT_FILE_HEADER_USE_RAND_INDEX
    gt_file_header_idx_t i;
    for (i = 0; i < ctl->count; i++) {
        if (ctl->header[i].index == index) {
            return &ctl->header[i];
        }
    }
    GT_LOGW(GT_LOG_TAG_FS, "index not found");
    return NULL;
#else
    if (index >= ctl->count) {
        GT_LOGW(GT_LOG_TAG_FS, "index out of range");
        return NULL;
    }
    return &ctl->header[index];
#endif
}

uint32_t gt_file_header_get_img_size(gt_file_header_st const * item)
{
#if _GT_FILE_HEADER_IMG_SAME_SIZE
    _gt_file_header_ctl_st const * ctl = &_GT_GC_GET_ROOT(file_header_ctl);

    return ((ctl->info.alpha ? 1 : 0) + sizeof(gt_color_t)) * ctl->info.width * ctl->info.height;
#else
    return ((item->info.alpha ? 1 : 0) + sizeof(gt_color_t)) * item->info.width * item->info.height;
#endif
}

uint32_t gt_file_header_get_img_offset_by(gt_file_header_st const * item, gt_file_header_param_st const * const fh)
{
    if (NULL == item) {
        return 0;
    }
#if _GT_FILE_HEADER_IMG_PACKAGE
    if (NULL == fh) {
        return 0;
    }
    gt_file_header_idx_t pkg_idx = fh->package_idx;
    if (pkg_idx < 0) {
        GT_LOGW(GT_LOG_TAG_FS, "image index out of package count range");
        pkg_idx = 0;
    }
    if (pkg_idx >= item->package_count) {
        GT_LOGW(GT_LOG_TAG_FS, "image index out of package count range");
        pkg_idx = 0;
    }
    return gt_file_header_get_img_size(item) * pkg_idx + item->address;
#else
    return item->address;
#endif  /** _GT_FILE_HEADER_IMG_PACKAGE */
}


gt_file_header_param_st * gt_file_header_param_check_valid(gt_file_header_param_st * param)
{
    if (NULL == param) {
        return NULL;
    }
    if (param->idx < 0) {
        return NULL;
    }
    return param;
}

#endif  /** GT_USE_FILE_HEADER */
/* end ------------------------------------------------------------------*/
