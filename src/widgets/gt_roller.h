/**
 * @file gt_roller.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2023-11-21 10:47:07
 * @copyright Copyright (c) 2014-2023, Company Genitop. Co., Ltd.
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


/* define ---------------------------------------------------------------*/

/**
 * @brief Roller work mode
 *
 */
typedef enum gt_roller_mode_e {
    GT_ROLLER_MODE_NORMAL   = 0,                /** Limited scrolling range */

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
 * @brief Set the selected item of the roller
 *
 * @param obj Roller object
 * @param index The index of the selected item [Begin with 0]
 * @return gt_res_t Set status
 */
gt_res_t gt_roller_set_selected(gt_obj_st * obj, gt_size_t index);

/**
 * @brief Get the index of the selected item [begin with 0]
 *
 * @param obj Roller object
 * @return gt_size_t -1: failed; other: index of the selected item
 */
gt_size_t gt_roller_get_selected(gt_obj_st * obj);

/**
 * @brief Go previous item
 *
 * @param obj Roller object
 * @return uint8_t The index of new item [begin with 0]
 */
uint8_t gt_roller_go_prev(gt_obj_st * obj);

/**
 * @brief Go next item
 *
 * @param obj Roller object
 * @return uint8_t The index of new item [begin with 0]
 */
uint8_t gt_roller_go_next(gt_obj_st * obj);

/**
 * @brief Get the total count of the roller options
 *
 * @param obj
 * @return uint8_t The count of options [Maximum: 256]
 */
uint8_t gt_roller_get_total_count(gt_obj_st * obj);

#endif  /** GT_CFG_ENABLE_ROLLER */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_ROLLER_H_
