/**
 * @file gt_hal_disp.c
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-05-13 09:52:03
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_hal_disp.h"
#include "../core/gt_mem.h"
#include "../others/gt_color.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static gt_disp_st * disp_p;


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/


/* global functions / API interface -------------------------------------*/

/*************************************/
/*                drv               */
/*************************************/
void gt_disp_drv_init(gt_disp_drv_st * drv){
    drv->res_hor = GT_SCREEN_WIDTH;
    drv->res_ver = GT_SCREEN_HEIGHT;
}

void gt_disp_drv_register(gt_disp_drv_st * drv){
    if( disp_p == NULL ){
        disp_p = gt_mem_malloc(sizeof(gt_disp_st));
    }
    if(drv->flush_cb == NULL){
        GT_LOGW(GT_LOG_TAG_GUI,"flush_cb is null");
        return;
    }
    disp_p->drv = drv;
}


/*************************************/
/*                disp               */
/*************************************/
void gt_disp_set_default(gt_disp_st * disp){
    disp_p = disp;
}


gt_disp_st * gt_disp_get_default(){
    return disp_p;
}



/* end ------------------------------------------------------------------*/
