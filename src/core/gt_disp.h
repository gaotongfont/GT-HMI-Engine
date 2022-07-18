/**
 * @file gt_disp.h
 * @author yongg (your@email.com)
 * @brief
 * @version 0.1
 * @date 2022-05-11 15:05:29
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_DISP_H_
#define _GT_DISP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../widgets/gt_obj.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef enum{
    GT_DIR_VER,
    GT_DIR_HOR,
}gt_dir_e;

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_load_scr(gt_obj_st * scr);

void gt_disp_set_scr(gt_obj_st * scr);
gt_obj_st * gt_disp_get_scr(void);

void gt_disp_load_scr(gt_obj_st * scr);

gt_area_st * gt_disp_get_area_act(void);

void gt_disp_set_area_act(gt_area_st * area);

void gt_disp_scroll_area_act(gt_dir_e dir, int16_t dist);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_DISP_H_
