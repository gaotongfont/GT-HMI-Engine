/**
 * @file gt_port_indev.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-06-22 14:33:01
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_port_indev.h"
#include "../src/hal/gt_hal_indev.h"
#include "../gt.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static gt_indev_drv_st indev_drv_pointer;
static gt_indev_drv_st indev_drv_button;

/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

extern void read_cb(struct _gt_indev_drv_s * indev_drv, gt_indev_data_st * data);
extern void read_cb_btn(struct _gt_indev_drv_s * indev_drv, gt_indev_data_st * data);

void gt_port_indev_init(void)
{
    /* register a touch screen dev */
	gt_indev_drv_init(&indev_drv_pointer);
	indev_drv_pointer.read_cb = read_cb;
	indev_drv_pointer.type = GT_INDEV_TYPE_POINTER;
	gt_indev_drv_register(&indev_drv_pointer);

	/* register a button dev */
    gt_indev_drv_init(&indev_drv_button);
	indev_drv_button.read_cb = read_cb_btn;
	indev_drv_button.type = GT_INDEV_TYPE_BUTTON;
	gt_indev_drv_register(&indev_drv_button);
}


/* end ------------------------------------------------------------------*/
