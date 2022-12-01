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
bool gt_area_intersect_screen(const gt_area_st * area_scr, const gt_area_st * area_fore, gt_area_st * area_res)
{
    gt_size_t fore_x2 = AREA_X2(area_fore);
    gt_size_t fore_y2 = AREA_Y2(area_fore);

    gt_size_t scr_x2 = AREA_X2(area_scr);
    gt_size_t scr_y2 = AREA_Y2(area_scr);

    if( scr_x2 < AREA_X1(area_fore) ){
        return false;
    }

    if( scr_y2 < AREA_Y1(area_fore) ){
        return false;
    }

    if( AREA_X1(area_scr) > fore_x2 ){
        return false;
    }

    if( AREA_Y1(area_scr) > fore_y2 ){
        return false;
    }

    // calc area_res pos and width and height
    AREA_X1(area_res) = 0;
    if( AREA_X1(area_fore) < AREA_X1(area_scr) ){
        AREA_X1(area_res) = AREA_X1(area_scr) - AREA_X1(area_fore);

        if( fore_x2 < scr_x2 ){
            AREA_W(area_res) = fore_x2 - AREA_X1(area_scr);
        }else{
            AREA_W(area_res) = AREA_W(area_scr);
        }
    }else{
        if( fore_x2 < scr_x2 ){
            AREA_W(area_res) = fore_x2 - AREA_X1(area_fore);
        }else{
            AREA_W(area_res) = scr_x2 - AREA_X1(area_fore);
        }
    }

    AREA_Y1(area_res) = 0;
    if( AREA_Y1(area_fore) < AREA_Y1(area_scr) ){
        AREA_Y1(area_res) = AREA_Y1(area_scr) - AREA_Y1(area_fore);

        if( fore_y2 < scr_y2 ){
            AREA_H(area_res) = fore_y2 - AREA_Y1(area_scr);
        }else{
            AREA_H(area_res) = AREA_H(area_scr);
        }
    }else{
        if( fore_y2 < scr_y2 ){
            AREA_H(area_res) = AREA_H(area_fore);
        }else{
            AREA_H(area_res) = scr_y2 - AREA_Y1(area_fore);
        }
    }

    return true;
}

bool gt_area_is_on(gt_area_st * a1_p, gt_area_st * a2_p)
{
    gt_size_t dis_x = gt_abs(a2_p->x - a1_p->x);
    gt_size_t dis_y = gt_abs(a2_p->y - a1_p->y);
    gt_size_t w_max = GT_MAX(a1_p->w, a2_p->w);
    gt_size_t h_max = GT_MAX(a1_p->h, a2_p->h);

    if (!w_max || !h_max) {
        return false;
    }

    if (dis_x > w_max || dis_y > h_max) {
        return false;
    }
    // printf("area1: (%d, %d, %d, %d), area2: (%d, %d, %d, %d)\n",
    //     a1_p->x, a1_p->y, a1_p->x + a1_p->w, a1_p->y + a1_p->h,
    //     a2_p->x, a2_p->y, a2_p->x + a2_p->w, a2_p->y + a2_p->h);
    return true;
}

void gt_area_join(gt_area_st * res, const gt_area_st * const src, const gt_area_st * const dst)
{
    gt_size_t src_x2 = src->x + src->w;
    gt_size_t src_y2 = src->y + src->h;

    gt_size_t dst_x2 = dst->x + dst->w;
    gt_size_t dst_y2 = dst->y + dst->h;

    gt_size_t min_x = GT_MIN(src->x, dst->x);
    gt_size_t min_y = GT_MIN(src->y, dst->y);
    gt_size_t max_x = GT_MAX(src_x2, dst_x2);
    gt_size_t max_y = GT_MAX(src_y2, dst_y2);

    res->x = min_x;
    res->y = min_y;
    res->w = max_x - min_x;
    res->h = max_y - min_y;
}


#include "stdio.h"
void gt_area_printf(gt_area_st area)
{
    printf("gui:" "x:%d,y:%d,w:%d,h:%d\n", area.x, area.y, area.w, area.h);
}
/* end ------------------------------------------------------------------*/
