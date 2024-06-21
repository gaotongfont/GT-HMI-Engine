/**
 * @file gt_effects.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-07-05 18:59:35
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_effects.h"
#include "stdlib.h"
#include "stdbool.h"
#include "gt_log.h"
#include "../core/gt_graph_base.h"

/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
#if 0
/**
 * @brief Horizontal gray gradient
 *
 * @param buffer
 * @param area the display area
 * @param coord beginning coord point
 * @param step vertical direction variables
 * @return uint8_t gray level
 */
static uint8_t _gt_set_trans_gray_hor(gt_color_t * buffer, gt_area_st * area, gt_point_st * coord, gt_size_t step)
{
    if (step + coord-> x > area->w || step + coord-> x < 0) {
        return 0;
    }

    if (!step) {
        return 0;
    }

    gt_color_t from = GT_COLOR_GET_OBJ(buffer, coord->x, coord->y, area->w, NULL);
    gt_color_t to   = GT_COLOR_GET_OBJ(buffer, step + coord->x, coord->y, area->w, NULL);
    gt_color_t * ptr;

    gt_size_t i, cnt = 0;
    gt_size_t div = abs(step);

    if (step > 0) {
        for (i = 0; i < step; i++) {
            ptr = &GT_COLOR_GET_OBJ(buffer, coord->x + i, coord->y, area->w, area->h);
            gt_graph_get_gray(ptr, from, to, div, cnt);
            ++cnt;
        }
    } else {
        cnt = div;
        coord->x += step;
        for (i = 0; i < div; i++) {
            ptr = &GT_COLOR_GET_OBJ(buffer, coord->x + i + 1, coord->y, area->w, area->h);
            gt_graph_get_gray(ptr, from, to, div, cnt);
            --cnt;
        }
        coord->x -= step;
    }

    return cnt;
}

/**
 * @brief Vertical gray gradient
 *
 * @param buffer
 * @param area the display area
 * @param coord beginning coord point
 * @param step vertical direction variables
 * @return uint8_t gray level
 */
static uint8_t _gt_set_trans_gray_ver(gt_color_t * buffer, gt_area_st * area, gt_point_st * coord, gt_size_t step)
{
    if (step + coord-> y > area->h || step + coord-> y < 0) {
        return 0;
    }

    if (!step) {
        return 0;
    }

    gt_color_t from = GT_COLOR_GET_OBJ(buffer, coord->x, coord->y, area->w, NULL);
    gt_color_t to   = GT_COLOR_GET_OBJ(buffer, coord->x, step + coord->y, area->w, NULL);
    gt_color_t * ptr;

    gt_size_t i, cnt = 0;
    gt_size_t div = abs(step);

    if (step > 0) {
        for (i = 0; i < div; i++) {
            ++cnt;
            ptr = &GT_COLOR_GET_OBJ(buffer, coord->x, coord->y + i, area->w, area->h);
            gt_graph_get_gray(ptr, from, to, div, cnt);
        }
    } else {
        cnt = div;
        coord->y += step;
        for (i = 0; i < div; i++) {
            --cnt;
            ptr = &GT_COLOR_GET_OBJ(buffer, coord->x, coord->y + i + 1, area->w, area->h);
            gt_graph_get_gray(ptr, from, to, div, cnt);
        }
        coord->y -= step;
    }

    return cnt;
}


/* global functions / API interface -------------------------------------*/

/**
 * @brief coord value difference to achieve grayscale change
 *  ![only for horizontal or vertical]
 *
 * @param buffer fill color buffer
 * @param area using width and height
 * @param coord current coord point
 * @param last last coord point
 * @return uint8_t gray level
 */
uint8_t gt_anti_aliasing_div(gt_color_t * buffer,gt_area_st * area, gt_point_st * coord, gt_point_st * last)
{
    int16_t     diff_x = coord->x - last->x, diff_y = coord->y - last->y;
    int16_t     step   = 0;
    uint16_t    w      = area->w;
    bool        is_hor = false;
    gt_point_st * dst  = NULL;

    if (abs(diff_x) > abs(diff_y)) {
        is_hor = true;
        if (GT_COLOR_GET_BUF(buffer, coord->x, last->y, w, NULL) == GT_COLOR_GET_BUF(buffer, coord->x, coord->y, w, NULL)) {
            dst  = coord;
            step = -diff_x;
        } else {
            dst  = last;
            step = diff_x;
        }
    } else {
        if (GT_COLOR_GET_BUF(buffer, last->x, coord->y, w, NULL) == GT_COLOR_GET_BUF(buffer, coord->x, coord->y, w, NULL)) {
            dst  = coord;
            step = -diff_y;
        } else {
            dst  = last;
            step = diff_y;
        }
    }

    return is_hor ? _gt_set_trans_gray_hor(buffer, area, dst, step) : _gt_set_trans_gray_ver(buffer, area, dst, step);
}
#endif

/* end ------------------------------------------------------------------*/
