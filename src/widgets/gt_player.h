/**
 * @file gt_player.h
 * @author Feyoung
 * @brief Slide effect, play content according to the rule
 * @version 0.1
 * @date 2023-05-04 14:22:12
 * @copyright Copyright (c) 2014-2023, Company Genitop. Co., Ltd.
 */
#ifndef _GT_PLAYER_H_
#define _GT_PLAYER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdbool.h"
#include "./gt_conf_widgets.h"
#include "gt_obj.h"
#include "../core/gt_timer.h"

#if GT_CFG_ENABLE_PLAYER
/* define ---------------------------------------------------------------*/

/**
 * @brief The objects displayed use custom display position
 */
#define _GT_PLAYER_USE_CUSTOM_POS       0


/* typedef --------------------------------------------------------------*/
/**
 * @brief The object played by the player
 */
typedef enum _gt_player_type_e {
    GT_PLAYER_TYPE_NONE = 0,     ///< none object, do nothing
    GT_PLAYER_TYPE_IMG,          ///< image object

    GT_PLAYER_TYPE_MAX_COUNT,
}gt_player_type_et;

/**
 * @brief work mode of the player
 */
typedef enum _gt_player_mode_e {
    GT_PLAYER_MODE_NONE = 0,    ///< none mode
    GT_PLAYER_MODE_ONCE,        ///< play it once mode
    GT_PLAYER_MODE_LOOP,        ///< loop mode
    GT_PLAYER_MODE_PLAYBACK,    ///< playback mode

    GT_PLAYER_MODE_MAX_COUNT,
}gt_player_mode_et;

/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_player_create(gt_obj_st * parent);

/**
 * @brief Set the player's type, Must be called before the player run
 *
 * @param obj
 * @param type @ref gt_player_type_et
 */
void gt_player_set_type(gt_obj_st * obj, gt_player_type_et type);

/**
 * @brief Set the player's mode
 *
 * @param obj
 * @param mode @ref gt_player_mode_et
 */
void gt_player_set_mode(gt_obj_st * obj, gt_player_mode_et mode);

/**
 * @brief
 *
 * @param obj
 * @param period 0: stop auto play; otherwise, auto play with the period [ms]
 */
void gt_player_set_auto_play_period(gt_obj_st * obj, uint32_t period);

/**
 * @brief Remove item from player object by item
 *
 * @param obj
 * @param item
 */
void gt_player_remove_item(gt_obj_st * obj, void * item);

/**
 * @brief Remove item from player object by index
 *
 * @param style
 * @param idx
 */
void gt_player_remove_item_by_index(gt_obj_st * obj, uint16_t idx);

#if _GT_PLAYER_USE_CUSTOM_POS
uint16_t gt_player_add_item(gt_obj_st * obj, void * item, uint16_t item_byte_size, gt_area_st * area);
#else
uint16_t gt_player_add_item(gt_obj_st * obj, void * item, uint16_t item_byte_size);
#endif  /** !_GT_PLAYER_USE_CUSTOM_POS */

/**
 * @brief Set player item index
 *
 * @param obj
 * @param index -1: last one; >=0: index number
 */
void gt_player_set_index(gt_obj_st * obj, gt_size_t index);
gt_size_t gt_player_get_index(gt_obj_st * obj);

uint16_t gt_player_get_item_count(gt_obj_st * obj);

/**
 * @brief Get current item percentage of the player
 *
 * @param obj
 * @return float
 */
float gt_player_get_percentage(gt_obj_st * obj);

void gt_player_turn_prev(gt_obj_st * obj);
void gt_player_turn_next(gt_obj_st * obj);

void gt_player_play(gt_obj_st * obj);
void gt_player_stop(gt_obj_st * obj);
void gt_player_toggle(gt_obj_st * obj);

void gt_player_dir_toggle(gt_obj_st * obj);
bool gt_player_dir_is_forward(gt_obj_st * obj);

void gt_player_set_real_time_update(gt_obj_st * obj, bool is_real_time_update);
bool gt_player_is_real_time_update(gt_obj_st * obj);

#endif /** GT_CFG_ENABLE_PLAYER */
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_PLAYER_H_
