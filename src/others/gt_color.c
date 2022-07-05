/**
 * @file gt_color.c
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-05-13 10:23:58
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_color.h"
#include "../widgets/gt_obj.h"
#include "../core/gt_mem.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_color_change(gt_color_t color1, gt_color_t color2){
    uint8_t r,g,b;
}

/**
 * @brief copy color to dst from src
 * 
 * @param dst dst color *
 * @param area_dst dst color pos and size(.x.y dst start pos, .w.h dst size)
 * @param src 
 * @param area_src src color pos and size(.x.y src start pos, .w.h src size)
 * @param w how much width copy from src
 * @param h how much height copy from src
 */
void gt_color_area_copy(gt_color_t *dst, gt_area_st area_dst, gt_color_t * src, gt_area_st area_src, gt_size_t w, gt_size_t h){
    int _h = 0;
    for(_h=0; _h<h; _h++){
        gt_memcpy( &dst[ (_h+area_dst.y) * area_dst.w + area_dst.x ], 
                   &src[ (_h+area_src.y) * area_src.w + area_src.x ], 
                   w * sizeof(gt_color_t) );
    }
}

/* end ------------------------------------------------------------------*/
