/**
 * @file gt_refr.c
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-08-12 10:04:48
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_refr.h"
#include "gt_refr.h"
#include "../gt_conf.h"
#include "../widgets/gt_obj.h"
#include "../others/gt_log.h"
#include "gt_obj_pos.h"
#include "../hal/gt_hal_disp.h"
#include "gt_disp.h"
#include "../extra/draw/gt_draw_blend.h"
#include "../core/gt_draw.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
volatile uint32_t cnt_refr = 0;
void gt_refr_timer()
{
    gt_disp_st * disp = gt_disp_get_default();
    if( disp == NULL ){
        return;
    }
    gt_area_st * area = NULL;
    while( _gt_disp_refr_check() ){
        cnt_refr++;
        area = _gt_disp_refr_get_area();
        // GT_LOGD(GT_LOG_TAG_GUI, "x:%d,y:%d,w:%d,h:%d", area->x, area->y, area->w, area->h);
        gt_disp_ref_area( area );
        _gt_disp_refr_idx_r_inc();
    }
}


/* end ------------------------------------------------------------------*/
