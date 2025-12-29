/**
 * @file gt_roll_anim.h
 * @author xiaoliu17
 * @brief
 * @version 0.1
 * @date 2025-08-19
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_ROLL_ANIM_H_
#define _GT_ROLL_ANIM_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/

#include "../others/gt_anim.h"
#include "../core/gt_draw.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/

typedef struct _auto_scroll_s {
    gt_obj_st * parent;
    gt_anim_st * anim;
    gt_area_st area;        // single line auto scroll
    uint32_t total_time;    // scroll total time [ms]
    uint8_t paused : 1;
}_auto_scroll_st;

/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

void _roll_anim_free_auto_ctrl(_auto_scroll_st ** auto_scroll_p);
_auto_scroll_st * _roll_anim_reset_auto_scroll_ctrl(_auto_scroll_st * as);
void _roll_anim_auto_scroll_init(gt_obj_st * obj, _auto_scroll_st * const auto_scroll_p);
_auto_scroll_st * _roll_anim_create_auto_scroll_anim(gt_obj_st * obj, _auto_scroll_st * auto_scroll);
void _roll_anim_first_restart_scroll(_gt_text_style_st text_style, _auto_scroll_st * auto_scroll, gt_area_st box_area, _gt_draw_font_res_st font_res);

/**
 * @brief
 *
 * @param auto_scroll
 * @param paused
 */
void _roll_anim_paused(_auto_scroll_st * auto_scroll, bool paused);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_ROLL_ANIM_H_
