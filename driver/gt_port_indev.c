/**
 * @file gt_port_indev.c
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-06-22 14:33:01
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_port_indev.h"
#include "../gt.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static gt_indev_drv_st inde_drv;


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
extern void read_cb(struct _gt_indev_drv_s * indev_drv, gt_indev_data_st * data);

void gt_port_indev_init(){
    gt_indev_drv_init(&inde_drv);
	inde_drv.read_cb = read_cb;
	inde_drv.type = GT_INDEV_TYPE_POINTER;
	gt_indev_drv_register(&inde_drv);
}


/* end ------------------------------------------------------------------*/
