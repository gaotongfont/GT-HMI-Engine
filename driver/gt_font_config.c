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
static const gt_font_family_st* _font_family = NULL;


/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_font_config_init(void)
{
    gt_font_family_init(_font_family, 0);
}


/* end of file ----------------------------------------------------------*/


