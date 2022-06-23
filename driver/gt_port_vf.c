/**
 * @file gt_port_vf.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-06-22 09:58:01
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_port_vf.h"
#include "../gt.h"
#include "../src/others/gt_log.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static gt_vfs_st virt_file_sys[] ={
	/*name, w, h, addr, size*/
    {".\\main\\chufang.png",48,48,0,0},
    {".\\main\\二维码.png",48,48,4608,0},
    {".\\main\\底层架构.png",48,48,9216,0},
    {".\\check.bmp",48,48,13824,0},
    {".\\check.png",48,48,18432,0},
    {".\\qrcode.png",500,500,23040,0},
    {".\\save.png",48,48,523040,0},
    {".\\分布式.png",48,48,527648,0},
    {".\\发票.png",48,48,532256,0},
    {".\\服务器.png",48,48,536864,0},
    {".\\端口探测.png",32,32,541472,0},
    {NULL,0,0,0,0}
};
static gt_vf_drv_st drv_vf;

/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/


/* global functions / API interface -------------------------------------*/
extern void spi_wr(uint8_t *dw, uint32_t lw, uint8_t *dr, uint32_t lr);

void gt_port_vf_init(){
    gt_vf_init(virt_file_sys);
	drv_vf.rw_cb = spi_wr;
    GT_LOGV(GT_LOG_TAG_GUI, "%p", &drv_vf);
	gt_vf_drv_register(&drv_vf);
}



/* end ------------------------------------------------------------------*/
