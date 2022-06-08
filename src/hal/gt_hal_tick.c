/**
 * @file gt_hal_tick.c
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-05-19 17:26:34
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_hal_tick.h"
#include "stdint.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static uint32_t __tick;


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_tick_inc(uint32_t ms){
    __tick += ms;
}

uint32_t gt_tick_get(){
    return __tick;
}


/* end ------------------------------------------------------------------*/
