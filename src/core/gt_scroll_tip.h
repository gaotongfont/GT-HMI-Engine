/**
 * @file gt_scroll_tip.h
 * @author Feyoung
 * @brief Draw scroll tip bar
 * @version 0.1
 * @date 2025-12-18
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SCROLL_TIP_H_
#define _GT_SCROLL_TIP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../others/gt_types.h"
#include "../widgets/gt_obj.h"



/* define ---------------------------------------------------------------*/
#ifndef GT_SCROLL_TIP_BAR_AUTO_HIDE_DELAY_MS
    /**
     * @brief Delay n ms to hide scroll tip bar
     */
    #define GT_SCROLL_TIP_BAR_AUTO_HIDE_DELAY_MS        500
#endif

#ifndef GT_SCROLL_TIP_BAR_AUTO_HIDE_TRANSITION_MS
    /**
     * @brief scroll tip bar transition n ms period from 60% -> 0%
     */
    #define GT_SCROLL_TIP_BAR_AUTO_HIDE_TRANSITION_MS   100
#endif


/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief [WARN] Calling by core!
 *      draw scroll tip bar when obj->scroll_tip is true.
 *
 * @param obj
 * @return gt_res_t
 */
gt_res_t gt_scroll_tip_draw(gt_obj_st * obj);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SCROLL_TIP_H_
