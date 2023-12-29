/**
 * @file gt_hal_fs.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-08-30 20:29:41
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_hal_fs.h"
#include "../core/gt_mem.h"
#include "../others/gt_log.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

gt_fs_fp_st * _gt_hal_fp_init(void)
{
    gt_fs_fp_st * fp = gt_mem_malloc(sizeof(gt_fs_fp_st));
    if (!fp) {
        GT_LOGE(GT_LOG_TAG_GUI, "malloc fp fail");
        return fp;
    }
    gt_memset_0(fp, sizeof(gt_fs_fp_st));
    return fp;
}


/* end ------------------------------------------------------------------*/
