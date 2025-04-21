/**
 * @file gt_slider.h
 * @author yongg
 * @brief scroll-bar control
 * @version 0.1
 * @date 2022-07-21 19:53:55
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SLIDER_H_
#define _GT_SLIDER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_SLIDER
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "../core/gt_style.h"
#include "../hal/gt_hal_fs.h"

#if GT_USE_FILE_HEADER
#include "../hal/gt_hal_file_header.h"
#endif

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef enum gt_slider_mode_e {
    GT_SLIDER_MODE_DEFAULT = 0,
    GT_SLIDER_MODE_STEP,
}gt_slider_mode_et;


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

/**
 * @brief Set the active color of the slider
 *
 * @param slider
 * @param color
 */
void gt_slider_set_color_act(gt_obj_st * slider, gt_color_t color);

/**
 * @brief Set the inactive color of the slider
 *
 * @param slider
 * @param color
 */
void gt_slider_set_color_ina(gt_obj_st * slider, gt_color_t color);

void gt_slider_set_dir(gt_obj_st * slider, gt_bar_dir_et dir);
gt_bar_dir_et gt_slider_get_dir(gt_obj_st * slider);


uint16_t gt_slider_get_step(gt_obj_st * slider);
/**
 * @brief Increment/decrement per operation
 *
 * @param slider
 * @param step
 */
void gt_slider_set_step(gt_obj_st * slider, uint16_t step);

/**
 * @brief Set tag image
 *
 * @param slider
 * @param src image path
 */
void gt_slider_set_tag(gt_obj_st * slider, char * src);

/**
 * @brief Display the tag image directly with raw data
 *
 * @param slider slider object
 * @param raw Raw data of the image
 */
void gt_slider_set_tag_raw_data(gt_obj_st * slider, gt_color_img_raw_st * raw);

#if GT_USE_FILE_HEADER
void gt_slider_set_tag_by_file_header(gt_obj_st * slider, gt_file_header_param_st * fh);
#endif

#if GT_USE_DIRECT_ADDR
/**
 * @brief Display the tag image by direct address
 *
 * @param slider
 * @param addr Direct address of the image
 */
void gt_slider_set_tag_by_direct_addr(gt_obj_st * slider, gt_addr_t addr);
#endif

#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
/**
 * @brief Display the tag image by custom size direct address
 *
 * @param slider
 * @param dac Custom size direct address of the image
 */
void gt_slider_set_tag_by_custom_size_addr(gt_obj_st * slider, gt_direct_addr_custom_size_st * dac);
#endif

void gt_slider_set_tag_visible(gt_obj_st * slider, bool visible);
void gt_slider_set_tag_size(gt_obj_st * slider , uint16_t size);


/**
 * @brief Sets the thickness of the slider
 *
 * @param slider
 * @param thickness pixel value
 */
void gt_slider_set_thickness(gt_obj_st * slider , uint16_t thickness);

/**
 * @brief Normal mode or step setting mode
 *
 * @param slider
 * @param mode @ref gt_slider_mode_et
 */
void gt_slider_set_mode(gt_obj_st * slider , gt_slider_mode_et mode);

/**
 * @brief Set AB repeat mode color
 *
 * @param slider
 * @param color
 */
void gt_slider_set_AB_repeat_color(gt_obj_st * slider, gt_color_t color);

/**
 * @brief Three cycles are triggered, the first time point A is recorded,
 *      the second time point B is recorded, and the third time point AB is
 *      cancelled to restore the normal mode.
 *
 *      [Note] AB repeat mode only GT_SLIDER_MODE_DEFAULT mode is valid.
 *
 * @param slider
 */
void gt_slider_set_AB_repeat_remark_point(gt_obj_st * slider);

/**
 * @brief Point AB is cancelled to restore the normal mode.
 *
 * @param slider
 */
void gt_slider_unset_AB_repeat(gt_obj_st * slider);


/**
 * @brief  Set the slider to the touch mode
 *
 * @param slider
 * @param touch_mode  0: default , 1 : pos can only be changed by swiping
 */
void gt_slider_set_touch_mode(gt_obj_st* slider, bool touch_mode);

/**
 * @brief Set the position range of the slider
 *
 * @param slider
 * @param min       min value >= start pos  @ gt_slider_set_start_end
 * @param max       max value <= end pos    @ gt_slider_set_start_end
 *                  max value == min value means the slider is not limited
 */
void gt_slider_set_pos_range(gt_obj_st * slider, gt_size_t min, gt_size_t max);

#endif  /** GT_CFG_ENABLE_SLIDER */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SLIDER_H_
