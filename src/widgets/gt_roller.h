/**
 * @file gt_roller.h
 * @author Feyoung
 * @brief Roller widget When the selection is modified,
 *      the GT_EVENT_TYPE_UPDATE_VALUE event of the current control is fired.
 * @version 0.1
 * @date 2023-11-21 10:47:07
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_ROLLER_H_
#define _GT_ROLLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_ROLLER
#include "gt_obj.h"
#include "../core/gt_style.h"


/* define ---------------------------------------------------------------*/

/**
 * @brief The number of pages of the project template is used to cache infinite churn
 */
#define GT_ROLLER_INFINITE_PAGES_COUNT  7

/**
 * @brief Roller work mode
 */
typedef enum gt_roller_mode_e {
    GT_ROLLER_MODE_NORMAL   = 0,                /** Limited scrolling range */
    GT_ROLLER_MODE_INFINITE,                    /** Infinite scrolling range */

    GT_ROLLER_MODE_MAX_COUNT = (1 << 2),        /** Up to 4 modes */
}gt_roller_mode_em;


/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a roller widget
 *
 * @param parent Parent object
 * @return gt_obj_st* Roller object
 */
gt_obj_st * gt_roller_create(gt_obj_st * parent);

/**
 * @brief Set the options content of the roller, In the middle,
 *      "\n" is used to distinguish items, such as: "A\nB\nC".
 *      [Maximum options item: 256]
 *
 * @param obj Roller object
 * @param options The options content of the roller
 * @param mode Roller work mode  @ref gt_roller_mode_em
 */
void gt_roller_set_options(gt_obj_st * obj, char * options, gt_roller_mode_em mode);

/**
 * @brief Set the number of Roller displays
 *
 * @param obj Roller object
 * @param count [Minimum or default: 3]
 */
void gt_roller_set_display_item_count(gt_obj_st * obj, uint8_t count);

/**
 * @brief Set the selected item of the roller immediately
 *
 * @param obj Roller object
 * @param index The index of the selected item [Begin with 0]
 * @return gt_res_t Set status
 */
gt_res_t gt_roller_set_selected(gt_obj_st * obj, gt_size_t index);

/**
 * @brief Set the selected item of the roller by anim
 *
 * @param obj Roller object
 * @param index The index of the selected item [Begin with 0]
 * @return gt_res_t Set status
 */
gt_res_t gt_roller_set_selected_anim(gt_obj_st * obj, gt_size_t index);

/**
 * @brief Get the index of the selected item [begin with 0]
 *
 * @param obj Roller object
 * @return gt_size_t -1: failed; other: index of the selected item
 */
gt_size_t gt_roller_get_selected(gt_obj_st * obj);

/**
 * @brief Get the text of the selected item
 *
 * @param obj Roller object
 * @param result The text of the selected item
 * @return true Get success, false Get failed
 */
bool gt_roller_get_selected_text(gt_obj_st * obj, char * result);

/**
 * @brief Go previous item
 *
 * @param obj Roller object
 * @return uint16_t The index of new item [begin with 0]
 */
uint16_t gt_roller_go_prev(gt_obj_st * obj);

/**
 * @brief Go next item
 *
 * @param obj Roller object
 * @return uint16_t The index of new item [begin with 0]
 */
uint16_t gt_roller_go_next(gt_obj_st * obj);

/**
 * @brief Get the total count of the roller options
 *
 * @param obj Roller object
 * @return uint8_t The count of options [Maximum: 256]
 */
uint8_t gt_roller_get_total_count(gt_obj_st * obj);

/**
 * @brief Set the line space of the roller
 *
 * @param obj Roller object
 * @param space value
 */
void gt_roller_set_line_space(gt_obj_st * obj, uint8_t space);

void gt_roller_set_font_color(gt_obj_st * obj, gt_color_t color);
void gt_roller_set_font_size(gt_obj_st * obj, uint8_t size);
void gt_roller_set_font_gray(gt_obj_st * obj, uint8_t gray);
void gt_roller_set_font_align(gt_obj_st * obj, gt_align_et align);
void gt_roller_set_font_family_cn(gt_obj_st * obj, gt_family_t family);
void gt_roller_set_font_family_en(gt_obj_st * obj, gt_family_t family);
void gt_roller_set_font_family_fl(gt_obj_st * obj, gt_family_t family);
void gt_roller_set_font_family_numb(gt_obj_st * obj, gt_family_t family);
void gt_roller_set_font_thick_en(gt_obj_st * obj, uint8_t thick);
void gt_roller_set_font_thick_cn(gt_obj_st * obj, uint8_t thick);

#endif  /** GT_CFG_ENABLE_ROLLER */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_ROLLER_H_
