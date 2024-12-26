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
#include "../core/gt_disp.h"
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
    gt_size_t fore_x1 = AREA_X1(area_fore);
    gt_size_t fore_y1 = AREA_Y1(area_fore);
    gt_size_t fore_x2 = AREA_X2(area_fore);
    gt_size_t fore_y2 = AREA_Y2(area_fore);

    gt_size_t scr_x1 = AREA_X1(screen);
    gt_size_t scr_y1 = AREA_Y1(screen);
    gt_size_t scr_x2 = AREA_X2(screen);
    gt_size_t scr_y2 = AREA_Y2(screen);

    AREA_X1(area_res) = 0;
    AREA_Y1(area_res) = 0;
    AREA_W(area_res)  = 0;
    AREA_H(area_res)  = 0;

    if (_gt_area_is_outside_area(scr_x1, scr_y1, scr_x2, scr_y2, fore_x1, fore_y1, fore_x2, fore_y2)) {
        return false;
    }

    // Calculate area_res position and width and height
    if (fore_x1 < scr_x1) {
        AREA_X1(area_res) = scr_x1 - fore_x1;
        AREA_W(area_res) = (fore_x2 < scr_x2) ? (fore_x2 - scr_x1) : AREA_W(screen);
    } else {
        AREA_X1(area_res) = 0;
        AREA_W(area_res) = (fore_x2 < scr_x2) ? (fore_x2 - fore_x1) : (scr_x2 - fore_x1);
    }

    if (fore_y1 < scr_y1) {
        AREA_Y1(area_res) = scr_y1 - fore_y1;
        AREA_H(area_res) = (fore_y2 < scr_y2) ? (fore_y2 - scr_y1) : AREA_H(screen);
    } else {
        AREA_Y1(area_res) = 0;
        AREA_H(area_res) = (fore_y2 < scr_y2) ? (fore_y2 - fore_y1) : (scr_y2 - fore_y1);
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
    AREA_W(area_res) = (AREA_X2(area1) < AREA_X2(area2) ? AREA_X2(area1) : AREA_X2(area2)) - AREA_X1(area_res);
    AREA_H(area_res) = (AREA_Y2(area1) < AREA_Y2(area2) ? AREA_Y2(area1) : AREA_Y2(area2)) - AREA_Y1(area_res);
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
    uint16_t hor = gt_disp_get_res_hor(NULL);
    uint16_t ver = gt_disp_get_res_ver(NULL);
    if ( !(area->w > hor || area->h > ver) ) {
        return true;
    }
    area->w = hor;
    area->h = ver;
    return false;
}

bool gt_area_intersect_get(gt_area_st * res_p, const gt_area_st * a1_p, const gt_area_st * a2_p)
{
    gt_size_t x2, y2;
    res_p->x = GT_MAX(AREA_X1(a1_p), AREA_X1(a2_p));
    res_p->y = GT_MAX(AREA_Y1(a1_p), AREA_Y1(a2_p));
    x2 = GT_MIN(AREA_X2(a1_p), AREA_X2(a2_p));
    y2 = GT_MIN(AREA_Y2(a1_p), AREA_Y2(a2_p));
    res_p->w = x2 - res_p->x;
    res_p->h = y2 - res_p->y;

    bool union_ok = true;
    if((AREA_X1(res_p) > x2) || (AREA_Y1(res_p) > y2)) {
        return true;
    }

    return true;
}


void gt_rounded_area_get(int16_t angle, uint16_t radius, uint8_t thickness, gt_area_st * res_area)
{
    const uint8_t ps = 8;
    const uint8_t pa = 127;

    int32_t thick_half = thickness / 2;
    uint8_t thick_corr = (thickness & 0x01) ? 0 : 1;

    int32_t cir_x;
    int32_t cir_y;

    cir_x = ((radius - thick_half) * gt_sin(90 - angle)) >> (GT_MATH_TRIGO_SHIFT - ps);
    cir_y = ((radius - thick_half) * gt_sin(angle)) >> (GT_MATH_TRIGO_SHIFT - ps);

    gt_size_t x2, y2;
    /*Actually the center of the pixel need to be calculated so apply 1/2 px offset*/
    if(cir_x > 0) {
        cir_x = (cir_x - pa) >> ps;
        res_area->x = cir_x - thick_half + thick_corr;
        x2 = cir_x + thick_half;
    }
    else {
        cir_x = (cir_x + pa) >> ps;
        res_area->x = cir_x - thick_half;
        x2 = cir_x + thick_half - thick_corr;
    }

    if(cir_y > 0) {
        cir_y = (cir_y - pa) >> ps;
        res_area->y = cir_y - thick_half + thick_corr;
        y2 = cir_y + thick_half;
    }
    else {
        cir_y = (cir_y + pa) >> ps;
        res_area->y = cir_y - thick_half;
        y2 = cir_y + thick_half - thick_corr;
    }
    res_area->w = x2 - res_area->x + 1;
    res_area->h = y2 - res_area->y + 1;
}
/* end ------------------------------------------------------------------*/
