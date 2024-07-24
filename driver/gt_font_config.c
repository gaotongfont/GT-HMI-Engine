/**
 * @file gt_font_config.c
 * @author Yang (your@email.com)
 * @brief
 * @version 0.1
 * @date 2024-07-19 10:09:10
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_font_config.h"
#include "../gt.h"
#include "../src/gt_conf.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static gt_font_family_st _font_family[] = {
    {16 , {14, 52, 52, 66, 59, 71, 74, 0xFFFF, 0xFFFF, 44, 47}},
    {16 , {157, 175, 175, 167, 165, 169, 177, 0xFFFF, 0xFFFF, 171, 173}},
    {24 , {80, 96, 96, 139, 140, 141, 143, 142, 79, 137, 138}},
    {32 , {80, 96, 96, 139, 140, 141, 143, 142, 0xFFFF, 137, 138}},
};


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_font_config_init(void)
{
    gt_font_family_init(_font_family, sizeof(_font_family) / sizeof(gt_font_family_st));
}


/* end of file ----------------------------------------------------------*/


