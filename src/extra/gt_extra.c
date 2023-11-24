/**
 * @file gt_extra.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-08-17 14:09:08
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_extra.h"

#if GT_USE_PNG
#include "./png/gt_png.h"
#endif

#if GT_USE_SJPG
#include "./jpeg/gt_sjpg.h"
#endif

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief extra function init
 */
void _gt_extra_init(void)
{

#if GT_USE_PNG
    gt_png_init();
#endif

#if GT_USE_SJPG
    gt_sjpg_init();
#endif

}


/* end ------------------------------------------------------------------*/
