/**
 * @file gt_hal_disp.h
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-05-13 09:51:55
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_HAL_DISP_H_
#define _GT_HAL_DISP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../widgets/gt_obj.h"
#include "../others/gt_color.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct _gt_drv_disp_s
{
    uint16_t res_hor;
    uint16_t res_ver;

    gt_color_t bcolor;
    gt_color_t fcolor;
    
    void (* flush_cb)(struct _gt_drv_disp_s * drv, gt_area_st * area, gt_color_t * color);
}gt_drv_disp_st;

typedef struct _gt_hal_disp_s
{
    gt_obj_st ** screens;   //Array of screen
    gt_obj_st * scr_act;    //Active screen
    gt_obj_st * scr_pre;    //Preview Screen,Only save one layer
    gt_obj_st * scr_next;   //Next Screen,Only save one layer

    uint16_t cnt_scr;       //Count screens nub
    

}gt_hal_disp_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_HAL_DISP_H_
