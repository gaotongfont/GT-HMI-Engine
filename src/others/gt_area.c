/**
 * @file gt_area.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-08-11 09:34:57
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_area.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
bool gt_area_intersect(const gt_area_st * area_back, const gt_area_st * area_fore, gt_area_st * area_res)
{
    if( AREA_X2(area_back) < AREA_X1(area_fore) ){
        return false;
    }

    if( AREA_Y2(area_back) < AREA_Y1(area_fore) ){
        return false;
    }

    if( AREA_X1(area_back) > AREA_X2(area_fore) ){
        return false;
    }

    if( AREA_Y1(area_back) > AREA_Y2(area_fore) ){
        return false;
    }

    // calc area_res pos and width and height
    AREA_X1(area_res) = 0;
    if( AREA_X1(area_fore) < AREA_X1(area_back) ){
        AREA_X1(area_res) = AREA_X1(area_back) - AREA_X1(area_fore);

        if( AREA_X2(area_fore) < AREA_X2(area_back) ){
            AREA_W(area_res) = AREA_X2(area_fore) - AREA_X1(area_back);
        }else{
            AREA_W(area_res) = AREA_W(area_back);
        }
    }else{
        if( AREA_X2(area_fore) < AREA_X2(area_back) ){
            AREA_W(area_res) = AREA_X2(area_fore) - AREA_X1(area_fore);
        }else{
            AREA_W(area_res) = AREA_X2(area_back) - AREA_X1(area_fore);
        }
    }

    AREA_Y1(area_res) = 0;
    if( AREA_Y1(area_fore) < AREA_Y1(area_back) ){
        AREA_Y1(area_res) = AREA_Y1(area_back) - AREA_Y1(area_fore);

        if( AREA_Y2(area_fore) < AREA_Y2(area_back) ){
            AREA_H(area_res) = AREA_Y2(area_fore) - AREA_Y1(area_back);
        }else{
            AREA_H(area_res) = AREA_H(area_back);
        }
    }else{
        if( AREA_Y2(area_fore) < AREA_Y2(area_back) ){
            AREA_H(area_res) = AREA_H(area_fore);
        }else{
            AREA_H(area_res) = AREA_Y2(area_back) - AREA_Y1(area_fore);
        }
    }

    return true;
}

#include "stdio.h"
void gt_area_printf(gt_area_st area)
{
    printf("gui:" "x:%d,y:%d,w:%d,h:%d\n", area.x, area.y, area.w, area.h);
}
/* end ------------------------------------------------------------------*/
