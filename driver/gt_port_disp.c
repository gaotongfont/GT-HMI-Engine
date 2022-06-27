/**
 * @file gt_port_disp.c
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-06-22 14:41:30
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_port_disp.h"
#include "../gt.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static gt_disp_drv_st drv;

static gt_color_t bufa[240*90];


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/


extern void _flush_cb(struct _gt_disp_drv_s * drv, gt_area_st * area, gt_color_t * color);

void gt_port_disp_init(){
    gt_disp_drv_init(&drv);
	drv.res_hor = GT_SCREEN_HEIGHT;
	drv.res_ver = GT_SCREEN_HEIGHT;
	drv.flush_cb = _flush_cb;
	drv.rotated = GT_ROTATED_0;

	gt_disp_drv_register(&drv);
	
	gt_disp_graph_buf_init(NULL, NULL, bufa);
}


/* end ------------------------------------------------------------------*/
