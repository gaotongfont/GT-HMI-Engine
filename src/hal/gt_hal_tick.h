/**
 * @file gt_tick.h
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-05-19 17:26:21
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_HAL_TICK_H_
#define _GT_HAL_TICK_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_tick_inc(uint32_t ms);

uint32_t gt_tick_get(void);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_TICK_H_
