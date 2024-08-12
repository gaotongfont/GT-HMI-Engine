/**
 * @file gt_area.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-08-11 09:34:57
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_area.h"
#include "../gt_conf.h"
#include "./gt_log.h"


/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/

static inline bool _gt_area_is_outside_area(
    gt_size_t a1_x1, gt_size_t a1_y1, gt_size_t a1_x2, gt_size_t a1_y2,
    gt_size_t a2_x1, gt_size_t a2_y1, gt_size_t a2_x2, gt_size_t a2_y2) {

    if ( a1_x2 < a2_x1 ) { return true; }

    if ( a1_y2 < a2_y1 ) { return true; }

    if ( a1_x1 > a2_x2 ) { return true; }

    if ( a1_y1 > a2_y2 ) { return true; }

    return false;
}


/* global functions / API interface -------------------------------------*/
bool gt_area_is_intersect_screen(const gt_area_st * screen, const gt_area_st * area_fore)
{
    gt_size_t fore_x2 = AREA_X2(area_fore);
    gt_size_t fore_y2 = AREA_Y2(area_fore);

    gt_size_t scr_x2 = AREA_X2(screen);
    gt_size_t scr_y2 = AREA_Y2(screen);

    if (_gt_area_is_outside_area(
        AREA_X1(screen), AREA_Y1(screen), scr_x2, scr_y2,
        AREA_X1(area_fore), AREA_Y1(area_fore), fore_x2, fore_y2)) {
        return false;
    }

    return true;
}

bool gt_area_intersect_screen(const gt_area_st * screen, const gt_area_st * area_fore, gt_area_st * area_res)
{
    gt_size_t fore_x2 = AREA_X2(area_fore);
    gt_size_t fore_y2 = AREA_Y2(area_fore);

    gt_size_t scr_x2 = AREA_X2(screen);
    gt_size_t scr_y2 = AREA_Y2(screen);

    AREA_X1(area_res) = 0;
    AREA_Y1(area_res) = 0;
    AREA_W(area_res)  = 0;
    AREA_H(area_res)  = 0;

    if (_gt_area_is_outside_area(
        AREA_X1(screen), AREA_Y1(screen), scr_x2, scr_y2,
        AREA_X1(area_fore), AREA_Y1(area_fore), fore_x2, fore_y2)) {
            return false;
    }

    // calc area_res pos and width and height
    if( AREA_X1(area_fore) < AREA_X1(screen) ){
        /** fore over the area of the left of screen  */
        AREA_X1(area_res) = AREA_X1(screen) - AREA_X1(area_fore);

        if( fore_x2 < scr_x2 ){
            AREA_W(area_res) = fore_x2 - AREA_X1(screen);
        }else{
            AREA_W(area_res) = AREA_W(screen);
        }
    }else{
        if( fore_x2 < scr_x2 ){
            AREA_W(area_res) = fore_x2 - AREA_X1(area_fore);
        }else{
            AREA_W(area_res) = scr_x2 - AREA_X1(area_fore);
        }
    }

    if( AREA_Y1(area_fore) < AREA_Y1(screen) ){
        /** fore over the area of the top of screen */
        AREA_Y1(area_res) = AREA_Y1(screen) - AREA_Y1(area_fore);

        if( fore_y2 < scr_y2 ){
            AREA_H(area_res) = fore_y2 - AREA_Y1(screen);
        }else{
            AREA_H(area_res) = AREA_H(screen);
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

bool gt_area_cover_screen(gt_area_st const * const area1, gt_area_st const * const area2, gt_area_st * area_res)
{
    gt_size_t a1_x2 = AREA_X2(area1);
    gt_size_t a1_y2 = AREA_Y2(area1);

    gt_size_t a2_x2 = AREA_X2(area2);
    gt_size_t a2_y2 = AREA_Y2(area2);

    AREA_X1(area_res) = 0;
    AREA_Y1(area_res) = 0;
    AREA_W(area_res) = 0;
    AREA_H(area_res) = 0;

    if (_gt_area_is_outside_area(
        AREA_X1(area1), AREA_Y1(area1), a1_x2, a1_y2,
        AREA_X1(area2), AREA_Y1(area2), a2_x2, a2_y2)) {
        return false;
    }
    AREA_X1(area_res) = AREA_X1(area2) > AREA_X1(area1) ? AREA_X1(area2) : AREA_X1(area1);
    AREA_Y1(area_res) = AREA_Y1(area2) > AREA_Y1(area1) ? AREA_Y1(area2) : AREA_Y1(area1);
    AREA_W(area_res) = (AREA_X2(area1) < AREA_X2(area2) ? AREA_X2(area1) : AREA_X2(area2) ) - AREA_X1(area_res);
    AREA_H(area_res) = (AREA_Y2(area1) < AREA_Y2(area2) ? AREA_Y2(area1) : AREA_Y2(area2) ) - AREA_Y1(area_res);
    return true;
}

bool gt_area_is_on(gt_area_st * a1_p, gt_area_st * a2_p)
{
    gt_size_t a1_x2 = AREA_X2(a1_p);
    gt_size_t a1_y2 = AREA_Y2(a1_p);
    gt_size_t a2_x2 = AREA_X2(a2_p);
    gt_size_t a2_y2 = AREA_Y2(a2_p);

    /**
     * a1_p/a2_p
     * |-----------------|
     * |  a2_p/a1_p      |
     * |  |------|       |
     * |  |      |       |
     * |  |------|       |
     * |                 |
     * |-----------------|
     */
    if ( (AREA_X1(a1_p) <= a2_x2) &&  (a1_x2 >= AREA_X1(a2_p)) && (AREA_Y1(a1_p) <= a2_y2) && (a1_y2 >= AREA_Y1(a2_p)) ) {
        return true;
    }
    return false;
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

bool gt_area_check_legal(gt_area_st * area)
{
    if ( !(area->w > GT_SCREEN_WIDTH || area->h > GT_SCREEN_HEIGHT) ) {
        return true;
    }

    area->w = GT_SCREEN_WIDTH;
    area->h = GT_SCREEN_HEIGHT;
    return false;
}

/* end ------------------------------------------------------------------*/
