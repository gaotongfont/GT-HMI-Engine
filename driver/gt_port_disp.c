/**
 * @file gt_port_disp.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-06-22 14:41:30
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_port_disp.h"
#include "../gt.h"
#include "../src/gt_conf.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static gt_disp_drv_st drv;

#if(GT_REFRESH_STYLE == GT_REFRESH_STYLE_0)

#elif(GT_REFRESH_STYLE == GT_REFRESH_STYLE_1)
	static gt_color_t buf1[GT_SCREEN_WIDTH * GT_REFRESH_FLUSH_LINE_PRE_TIME];
#elif(GT_REFRESH_STYLE == GT_REFRESH_STYLE_2)
	static gt_color_t buf1[GT_SCREEN_WIDTH * GT_REFRESH_FLUSH_LINE_PRE_TIME];
	static gt_color_t buf2[GT_SCREEN_WIDTH * GT_REFRESH_FLUSH_LINE_PRE_TIME];
#elif(GT_REFRESH_STYLE == GT_REFRESH_STYLE_3)
	static gt_color_t buf_all[GT_SCREEN_WIDTH * GT_SCREEN_HEIGHT];
#endif

/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/


extern void _flush_cb(struct _gt_disp_drv_s * drv, gt_area_st * area, gt_color_t * color);

void gt_port_disp_init(void)
{
    gt_disp_drv_init(&drv);
	drv.res_hor = GT_SCREEN_WIDTH;
	drv.res_ver = GT_SCREEN_HEIGHT;
	drv.flush_cb = _flush_cb;
	drv.rotated = GT_ROTATED_0;

#if(GT_REFRESH_STYLE == GT_REFRESH_STYLE_0)

#elif(GT_REFRESH_STYLE == GT_REFRESH_STYLE_1)
	gt_disp_graph_buf_init(buf1, NULL, NULL);
#elif(GT_REFRESH_STYLE == GT_REFRESH_STYLE_2)
	gt_disp_graph_buf_init(buf1, buf2, NULL);
#elif(GT_REFRESH_STYLE == GT_REFRESH_STYLE_3)
	gt_disp_graph_buf_init(NULL, NULL, buf_all);
#endif

	gt_disp_drv_register(&drv);
}


/* end ------------------------------------------------------------------*/
