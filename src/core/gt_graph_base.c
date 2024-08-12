/**
 * @file gt_graph_base.c
 * @author yongg
 * @brief Low-level rendering interface
 * @version 0.1
 * @date 2022-05-11 15:02:43
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "stdlib.h"
#include "stdbool.h"

#include "gt_graph_base.h"
#include "../hal/gt_hal_disp.h"
#include "gt_mem.h"
#include "../others/gt_math.h"
#include "../others/gt_log.h"
#include "../widgets/gt_obj.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

void gt_graph_init_rect_attr(gt_attr_rect_st * rect_attr)
{
	gt_memset(rect_attr, 0, sizeof(gt_attr_rect_st));
	rect_attr->bg_color = gt_color_black();
	rect_attr->bg_opa = GT_OPA_COVER;
	rect_attr->border_color = gt_color_black();
	rect_attr->border_opa = GT_OPA_COVER;
}


/* end ------------------------------------------------------------------*/
