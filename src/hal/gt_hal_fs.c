/**
 * @file gt_hal_fs.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-08-30 20:29:41
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_hal_fs.h"
#include "../core/gt_mem.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/

/**
 * @brief file operation structure
 */
static gt_fs_fp_st * _fp = NULL;


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

gt_fs_fp_st * _gt_hal_fp_init(void)
{
    if (!_fp) {
        _fp = gt_mem_malloc(sizeof(gt_fs_fp_st));
    }
    gt_memset_0(_fp, sizeof(gt_fs_fp_st));
    return _fp;
}


/* end ------------------------------------------------------------------*/
