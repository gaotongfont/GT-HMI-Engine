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
typedef enum {
    GT_ROTATED_0,
    GT_ROTATED_90,
    GT_ROTATED_180,
    GT_ROTATED_270
}gt_rotated_et;

typedef struct _gt_disp_drv_s
{
    uint16_t res_hor;
    uint16_t res_ver;

    gt_color_t bcolor;
    gt_color_t fcolor;
    
    void (* flush_cb)(struct _gt_disp_drv_s * drv, gt_area_st * area, gt_color_t * color);

    uint16_t rotated: 2;
}gt_disp_drv_st;

typedef struct _gt_disp_s
{
    gt_disp_drv_st * drv;

    gt_obj_st ** screens;       //Array of screen
    gt_obj_st *  scr_act;       //Active screen
    gt_obj_st *  scr_pre;       //Preview Screen,Only save one layer
    gt_obj_st *  scr_next;      //Next Screen,Only save one layer

    uint16_t cnt_scr;           //Count screens nub
    uint16_t status;            //

    gt_area_st area_act;        //Location of physical screen on virtual screen
}gt_disp_st;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_disp_drv_init(gt_disp_drv_st * disp_drv);
void gt_disp_drv_deinit(void);

void gt_disp_drv_register(gt_disp_drv_st *drv);
void gt_disp_set_default(gt_disp_st * disp);

gt_disp_st * gt_disp_get_default(void);


uint16_t gt_disp_get_res_hor(gt_disp_st * disp);
uint16_t gt_disp_get_res_ver(gt_disp_st * disp);

void gt_disp_graph_buf_init(gt_color_t *buf1, gt_color_t *buf2, gt_color_t *bufa);
gt_color_t * gt_disp_graph_get_buf1(void);
gt_color_t * gt_disp_graph_get_buf2(void);
gt_color_t * gt_disp_graph_get_bufa(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_HAL_DISP_H_
