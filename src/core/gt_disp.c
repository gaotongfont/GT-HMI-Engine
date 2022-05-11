/**
 * @file gt_disp.c
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-05-11 14:59:50
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "../gt_conf.h"




/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/
typedef struct gt_disp_s
{
    gt_obj_st *src;     //header
}gt_disp_st;

static gt_disp_st *disp;




/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void
gt_disp_set_src(gt_obj_st *src){
    disp->src = src;
}

gt_obj_st *
gt_disp_get_src(){
    return disp->src;
}


/* end ------------------------------------------------------------------*/
