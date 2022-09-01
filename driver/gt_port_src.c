/**
 * @file gt_port_src.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-08-31 10:38:37
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_port_src.h"
#include "../src/others/gt_types.h"
#include "stddef.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/


static const gt_src_st _src_icon_sys[] = {
    {"./1.bmp", 4, 4, 2, 16},
    {"./2.bin", 4, 4, 2, 16},
    {NULL, 0, 0, 0, 0},
};


static const uint8_t arr_data[] = {
    // ./1.bmp
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,

    // ./2.png
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
};


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief Initializes the array mode file management system
 */
void gt_port_src_init(void)
{
    gt_src_init(_src_icon_sys, arr_data, sizeof(arr_data));
}


/* end ------------------------------------------------------------------*/
