/**
 * @file gt_progress_bar.h
 * @author yongg
 * @brief The progress bar implementation
 * @version 0.1
 * @date 2022-07-22 14:13:28
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_PROGRESS_BAR_H_
#define _GT_PROGRESS_BAR_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_PROGRESS_BAR
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "../core/gt_style.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief create progress_bar obj
 *
 * @param parent progress_bar's parent obj
 * @return gt_obj_st* progress_bar obj ptr
 */
gt_obj_st * gt_progress_bar_create(gt_obj_st * parent);

/**
 * @brief set progress_bar pos
 *
 * @param progress_bar progress_bar obj
 * @param pos pos
 */
void gt_progress_bar_set_pos(gt_obj_st * progress_bar, gt_size_t pos);

/**
 * @brief set progress_bar start and end pos
 *
 * @param progress_bar progress_bar obj
 * @param start start pos
 * @param end end pos
 */
void gt_progress_bar_set_start_end(gt_obj_st * progress_bar, gt_size_t start, gt_size_t end);

/**
 * @brief get progress_bar pos
 *
 * @param progress_bar progress_bar obj
 * @return gt_size_t progress_bar pos
 */
gt_size_t gt_progress_bar_get_pos(gt_obj_st * progress_bar);

/**
 * @brief get progress_bar start pos
 *
 * @param progress_bar progress_bar obj
 * @return gt_size_t progress_bar start pos
 */
gt_size_t gt_progress_bar_get_start(gt_obj_st * progress_bar);

/**
 * @brief get progress_bar end pos
 *
 * @param progress_bar progress_bar obj
 * @return gt_size_t progress_bar end pos
 */
gt_size_t gt_progress_bar_get_end(gt_obj_st * progress_bar);

/**
 * @brief get progress_bar total pos
 *
 * @param progress_bar progress_bar obj
 * @return gt_size_t progress_bar total pos
 */
gt_size_t gt_progress_bar_get_total(gt_obj_st * progress_bar);


void gt_progress_bar_set_color_act(gt_obj_st * progress_bar, gt_color_t color);
void gt_progress_bar_set_color_ina(gt_obj_st * progress_bar, gt_color_t color);

void gt_progress_bar_set_dir(gt_obj_st * progress_bar, gt_bar_dir_et dir);
gt_bar_dir_et gt_progress_bar_get_dir(gt_obj_st * progress_bar);


#endif  /** GT_CFG_ENABLE_PROGRESS_BAR */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_PROGRESS_BAR_H_
