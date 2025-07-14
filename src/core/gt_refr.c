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
#include "./gt_mem.h"


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
#if GT_REFR_AREA_ALIGN_HOR || GT_REFR_AREA_ALIGN_VER
    uint16_t align_val = 0;
#endif
    GT_CHECK_BACK(disp);
    if (false == gt_refr_is_enabled(disp)) {
        return;
    }
    if (_gt_disp_refr_check(disp)) {
#if GT_USE_DISPLAY_PREF_MSG
        uint32_t start = gt_tick_get();
#endif
        area = _gt_disp_refr_get_area(disp);
        GT_CHECK_BACK(area);
#if GT_REFR_AREA_ALIGN_HOR
        align_val = gt_abs(area->x % GT_REFR_AREA_ALIGN_HOR_PIXEL);
        if (align_val) {
            area->x -= align_val;
            area->w = ((uint16_t)(align_val + GT_REFR_AREA_ALIGN_HOR_PIXEL + area->w) / GT_REFR_AREA_ALIGN_HOR_PIXEL) * GT_REFR_AREA_ALIGN_HOR_PIXEL;
        }
 #endif

#if GT_REFR_AREA_ALIGN_VER
        align_val = gt_abs(area->y % GT_REFR_AREA_ALIGN_VER_PIXEL);
        if (align_val) {
            area->y -= align_val;
            area->h = ((uint16_t)(align_val + GT_REFR_AREA_ALIGN_VER_PIXEL + area->h) / GT_REFR_AREA_ALIGN_VER_PIXEL) * GT_REFR_AREA_ALIGN_VER_PIXEL;
        }
#endif
        gt_disp_ref_area(area);
        _gt_disp_refr_area_pop(disp);

#if GT_USE_DISPLAY_PREF_MSG
        if( (gt_tick_get() - start) > 1 ) {
            GT_LOGD(GT_LOG_TAG_GUI, "refr timer: %d ms", gt_tick_get() - start );
        }
#endif
    } else {
        // gt_disp_invalid_area(NULL);
    }
#if (defined(_GT_PORT_SIMULATOR_ENVS) && defined(USE_MEM_REAL_USED_REMARK)) || 0
    gt_mem_check_used();
#endif
}

void gt_refr_force_flush(void)
{
    gt_disp_st * disp = gt_disp_get_default();
#if GT_REFR_AREA_ALIGN_HOR || GT_REFR_AREA_ALIGN_VER
    uint16_t align_val = 0;
#endif
    if (NULL == disp) {
        return;
    }
    gt_area_st reflush_area = disp->area_act;
#if GT_REFR_AREA_ALIGN_HOR
        align_val = gt_abs(reflush_area.x % GT_REFR_AREA_ALIGN_HOR_PIXEL);
        if (align_val) {
            reflush_area.x -= align_val;
            reflush_area.w = ((uint16_t)(align_val + GT_REFR_AREA_ALIGN_HOR_PIXEL + reflush_area.w) / GT_REFR_AREA_ALIGN_HOR_PIXEL) * GT_REFR_AREA_ALIGN_HOR_PIXEL;
        }
 #endif

#if GT_REFR_AREA_ALIGN_VER
        align_val = gt_abs(reflush_area.y % GT_REFR_AREA_ALIGN_VER_PIXEL);
        if (align_val) {
            reflush_area.y -= align_val;
            reflush_area.h = ((uint16_t)(align_val + GT_REFR_AREA_ALIGN_VER_PIXEL + reflush_area.h) / GT_REFR_AREA_ALIGN_VER_PIXEL) * GT_REFR_AREA_ALIGN_VER_PIXEL;
        }
#endif
    gt_disp_ref_area(&reflush_area);
}

void gt_refr_set_enabled(bool enabled)
{
    gt_disp_st * disp = gt_disp_get_default();
    disp->enabled = enabled;
}

bool gt_refr_is_enabled(gt_disp_st * disp)
{
    if (NULL == disp) {
        disp = gt_disp_get_default();
    }
    return disp->enabled;
}

/* end ------------------------------------------------------------------*/
