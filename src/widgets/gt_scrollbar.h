/**
 * @file gt_scrollbar.h
 * @author yongg
 * @brief scroll-bar control
 * @version 0.1
 * @date 2022-07-21 19:53:55
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SCROLLBAR_H_
#define _GT_SCROLLBAR_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct _gt_style_scrollbar_s
{
    gt_color_t color_act;
    gt_color_t color_ina;

    gt_color_t bg_color;

    gt_size_t start;
    gt_size_t end;
    gt_size_t pos;

    uint8_t dir;    //0 hor, 1 ver
    uint8_t step;
}_gt_style_scrollbar_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief create scrollbar obj
 *
 * @param parent scrollbar's parent obj
 * @return gt_obj_st* scrollbar obj ptr
 */
gt_obj_st * gt_scrollbar_create(gt_obj_st * parent);

/**
 * @brief set scrollbar pos
 *
 * @param scrollbar scrollbar obj ptr
 * @param pos pos between start and end
 */
void gt_scrollbar_style_set_pos(gt_obj_st * scrollbar, gt_size_t pos);

/**
 * @brief set scrollbar start and end pos
 *
 * @param scrollbar scrollbar obj
 * @param start start pos
 * @param end end pos
 */
void gt_scrollbar_style_set_start_end(gt_obj_st * scrollbar, gt_size_t start, gt_size_t end);

/**
 * @brief get scrollbar pos
 *
 * @param scrollbar scrollbar obj ptr
 * @return gt_size_t scrollbar pos
 */
gt_size_t gt_scrollbar_style_get_pos(gt_obj_st * scrollbar);

/**
 * @brief get scrollbar start pos
 *
 * @param scrollbar scrollbar obj ptr
 * @return gt_size_t scrollbar start pos
 */
gt_size_t gt_scrollbar_style_get_start(gt_obj_st * scrollbar);

/**
 * @brief get scrollbar end pos
 *
 * @param scrollbar scrollbar obj ptr
 * @return gt_size_t scrollbar end pos
 */
gt_size_t gt_scrollbar_style_get_end(gt_obj_st * scrollbar);

/**
 * @brief get scrollbar start-end pos
 *
 * @param scrollbar scrollbar obj ptr
 * @return gt_size_t scrollbar start-end distance
 */
gt_size_t gt_scrollbar_style_get_total(gt_obj_st * scrollbar);

void gt_scrollbar_style_set_color_act(gt_obj_st * scrollbar, gt_color_t color);
void gt_scrollbar_style_set_color_ina(gt_obj_st * scrollbar, gt_color_t color);
void gt_scrollbar_style_set_color_bg(gt_obj_st * scrollbar, gt_color_t color);



void gt_scrollbar_style_set_dir(gt_obj_st * scrollbar, uint8_t dir);
uint8_t gt_scrollbar_style_get_dir(gt_obj_st * scrollbar);

void gt_scrollbar_style_set_step(gt_obj_st * scrollbar, uint8_t step);
uint8_t gt_scrollbar_style_get_step(gt_obj_st * scrollbar);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SCROLLBAR_H_
