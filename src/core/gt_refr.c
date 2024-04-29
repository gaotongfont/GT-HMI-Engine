/**
 * @file gt_refr.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-08-12 10:04:48
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_refr.h"
#include "gt_refr.h"
#include "../gt_conf.h"
#include "../widgets/gt_obj.h"
#include "../others/gt_log.h"
#include "../others/gt_area.h"
#include "gt_obj_pos.h"
#include "../hal/gt_hal_disp.h"
#include "../hal/gt_hal_tick.h"
#include "gt_disp.h"
#include "../extra/draw/gt_draw_blend.h"
#include "../core/gt_draw.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_refr_timer(void)
{
    gt_disp_st * disp = gt_disp_get_default();
    gt_area_st * area = NULL;
    if( disp == NULL ){
        return ;
    }
    if( _gt_disp_refr_check(disp) ){
#if GT_USE_DISPLAY_PREF_MSG
        uint32_t start = gt_tick_get();
#endif

        area = _gt_disp_refr_get_area(disp);
        if ( !area ) {
            return ;
        }

        gt_disp_ref_area( area );
        _gt_disp_refr_area_pop(disp);

#if GT_USE_DISPLAY_PREF_MSG
        if( (gt_tick_get() - start) > 1 ) {
            GT_LOGD(GT_LOG_TAG_GUI, "refr timer: %d ms", gt_tick_get() - start );
        }
#endif
    } else {
        // gt_disp_invalid_area(NULL);
    }
}


/* end ------------------------------------------------------------------*/
