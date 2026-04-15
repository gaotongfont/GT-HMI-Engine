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
#include "../core/gt_handler.h"
#include "./gt_mem.h"
#include "./gt_layout.h"


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
    GT_CHECK_BACK(disp);
    if (false == gt_refr_is_enabled(disp)) {
        return;
    }
#if GT_USE_WIDGET_LAYOUT
    gt_layout_flush_dirty();
#endif
    if (_gt_disp_refr_check(disp)) {
#if GT_USE_DISPLAY_PREF_MSG
        uint32_t start = gt_tick_get();
#endif
        area = _gt_disp_refr_get_area(disp);
        GT_CHECK_BACK(area);
        gt_refr_area_align(area);
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
    if (NULL == disp) {
        return;
    }
    gt_area_st reflush_area = disp->area_act;
    gt_refr_area_align(&reflush_area);
    gt_disp_ref_area(&reflush_area);
}

gt_res_t gt_refr_force_flush_thread_safe(uint32_t timeout)
{
    if (false == gt_task_check_idle_timeout(timeout)) {
        return GT_RES_INV;
    }
    gt_refr_force_flush();
    return GT_RES_OK;
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
