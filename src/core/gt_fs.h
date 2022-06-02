/**
 * @file gt_fs.h
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-06-01 14:07:18
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_FS_H_
#define _GT_FS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef enum _gt_fs_res_s
{
    GT_FS_RES_OK,
    GT_FS_RES_ERR,
    GT_FS_RES_NOT_EXIST,
    GT_FS_RES_OUT_OF_MEM,
    GT_FS_RES_NOT_INIT,
}gt_fs_res_et;


typedef struct _gt_fs_virt_s
{
    char * path;
    uint32_t size;
}gt_fs_virt_st;

typedef struct _gt_fs_drv_s
{   
    char letter;
    void * (*open_cb)(struct _gt_fs_drv_s * drv, char * path);
    gt_fs_res_et (*close_cb)(struct _gt_fs_drv_s * drv, void * fp);
    gt_fs_res_et (*read_cb)(struct _gt_fs_drv_s * drv, void * fp, void * buf, uint32_t len, uint32_t *rlen);
    gt_fs_res_et (*write_cb)(struct _gt_fs_drv_s * drv, void * fp, void * buf, uint32_t len, uint32_t *rlen);
}gt_fs_drv_st;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_FS_H_
