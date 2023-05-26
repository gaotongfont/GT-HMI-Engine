/**
 * @file gt_slider.h
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
#include "../core/gt_style.h"
/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief create slider obj
 *
 * @param parent slider's parent obj
 * @return gt_obj_st* slider obj ptr
 */
gt_obj_st * gt_slider_create(gt_obj_st * parent);

/**
 * @brief set slider pos
 *
 * @param slider slider obj ptr
 * @param pos pos between start and end
 */
void gt_slider_set_pos(gt_obj_st * slider, gt_size_t pos);

/**
 * @brief set slider start and end pos
 *
 * @param slider slider obj
 * @param start start pos
 * @param end end pos
 */
void gt_slider_set_start_end(gt_obj_st * slider, gt_size_t start, gt_size_t end);

/**
 * @brief get slider pos
 *
 * @param slider slider obj ptr
 * @return gt_size_t slider pos
 */
gt_size_t gt_slider_get_pos(gt_obj_st * slider);

/**
 * @brief get slider start pos
 *
 * @param slider slider obj ptr
 * @return gt_size_t slider start pos
 */
gt_size_t gt_slider_get_start(gt_obj_st * slider);

/**
 * @brief get slider end pos
 *
 * @param slider slider obj ptr
 * @return gt_size_t slider end pos
 */
gt_size_t gt_slider_get_end(gt_obj_st * slider);

/**
 * @brief get slider start-end pos
 *
 * @param slider slider obj ptr
 * @return gt_size_t slider start-end distance
 */
gt_size_t gt_slider_get_total(gt_obj_st * slider);

void gt_slider_set_color_act(gt_obj_st * slider, gt_color_t color);
void gt_slider_set_color_ina(gt_obj_st * slider, gt_color_t color);

void gt_slider_set_dir(gt_obj_st * slider, gt_bar_dir_et dir);
gt_bar_dir_et gt_slider_get_dir(gt_obj_st * slider);


uint8_t gt_slider_get_step(gt_obj_st * slider);
void gt_slider_set_step(gt_obj_st * slider, uint8_t step);
void gt_slider_set_tag(gt_obj_st * slider, char * src);
void gt_slider_set_tag_visible(gt_obj_st * slider, bool visible);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SCROLLBAR_H_
