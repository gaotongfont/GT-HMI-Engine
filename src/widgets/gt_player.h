/**
 * @file gt_player.h
 * @author Feyoung
 * @brief Slide effect, play content according to the rule
 * @version 0.1
 * @date 2023-05-04 14:22:12
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_PLAYER_H_
#define _GT_PLAYER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_PLAYER
#include "stdbool.h"
#include "gt_obj.h"
#include "../core/gt_timer.h"


#if GT_USE_FILE_HEADER
#include "../hal/gt_hal_file_header.h"
#endif

#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
#include "../hal/gt_hal_fs.h"
#endif

/* define ---------------------------------------------------------------*/

#ifndef GT_PLAYER_REPEAT_FINISH_CALLBACK
    /**
     * @brief Using repeat finish callback
     *      [default]: 0
     */
    #define GT_PLAYER_REPEAT_FINISH_CALLBACK    0
#endif

#ifndef GT_PLAYER_USE_RAW_DATA
    /**
     * @brief Using raw data to display image
     *      [default]: 0
     */
    #define GT_PLAYER_USE_RAW_DATA              0
#endif


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
 * @brief Remove all item
 *
 * @param obj
 */
void gt_player_remove_all_items(gt_obj_st * obj);

/**
 * @brief Remove item from player object by index
 *
 * @param style
 * @param idx
 */
void gt_player_remove_item_by_index(gt_obj_st * obj, gt_size_t idx);

/**
 * @brief Add the player project content
 *
 * @param obj
 * @param item The item to be added
 * @param item_byte_size The item memory size in bytes
 * @return gt_size_t -1: failed, The item count of player
 */
gt_size_t gt_player_add_item(gt_obj_st * obj, void * item, uint16_t item_byte_size);

#if GT_PLAYER_USE_RAW_DATA
/**
 * @brief Display the image directly with raw data
 *
 * @param obj player object
 * @param raw Raw data of the image
 */
gt_size_t gt_player_add_item_raw(gt_obj_st * obj, gt_color_img_raw_st * raw);
#endif

#if GT_USE_FILE_HEADER
/**
 * @brief Add the player project content by file header
 *
 * @param obj
 * @param fh idx -1[defalut]: Disabled file header, using img path to open file; >=0: index number, valid value
 *              < 0: invalid value.
 *           package_idx 0[defalut]: The index of element within item, as the first element
 * @return gt_size_t -1: failed, The item count of player
 */
gt_size_t gt_player_add_item_by_file_header(gt_obj_st * obj, gt_file_header_param_st * fh);

/**
 * @brief Add the player project content by file header list
 *
 * @param obj
 * @param fh_array The array of gt_file_header_param_st
 * @param count The count of gt_file_header_param_st[]
 * @return gt_size_t -1: failed, The item count of player
 */
gt_size_t gt_player_add_item_list_by_file_header(gt_obj_st * obj, gt_file_header_param_st const * const fh_array, uint16_t count);
#endif

#if GT_USE_DIRECT_ADDR
/**
 * @brief Add the player project content by direct address
 *
 * @param obj
 * @param addr
 * @return gt_size_t
 */
gt_size_t gt_player_add_item_by_direct_addr(gt_obj_st * obj, gt_addr_t addr);

/**
 * @brief Add the player project content by direct address list
 *
 * @param obj
 * @param addr_array  The array of gt_addr_t
 * @param count The count of gt_addr_t[]
 * @return gt_size_t
 */
gt_size_t gt_player_add_item_list_by_direct_addr(gt_obj_st * obj, gt_addr_t const * const addr_array, uint16_t count);
#endif

#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
/**
 * @brief Add the player project content by custom size direct address
 *
 * @param obj
 * @param dac
 * @return gt_size_t
 */
gt_size_t gt_player_add_item_by_custom_size_addr(gt_obj_st * obj, gt_direct_addr_custom_size_st * dac);

/**
 * @brief Add the player project content by custom size direct address list
 *
 * @param obj
 * @param addr_array  The array of gt_direct_addr_custom_size_st
 * @param count The count of gt_direct_addr_custom_size_st[]
 * @return gt_size_t
 */
gt_size_t gt_player_add_item_list_by_custom_size_addr(gt_obj_st * obj, gt_direct_addr_custom_size_st * addr_array, uint16_t count);
#endif
/**
 * @brief Set player item index
 *
 * @param obj
 * @param index -1: last one; >=0: index number
 */
void gt_player_set_index(gt_obj_st * obj, gt_size_t index);
gt_size_t gt_player_get_index(gt_obj_st * obj);

gt_size_t gt_player_get_item_count(gt_obj_st * obj);

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

/**
 * @brief When GT_PLAYER_MODE_LOOP or GT_PLAYER_MODE_PLAYBACK mode, set
 *      repeat count
 *
 * @param obj
 * @param count -1: infinite loop; otherwise, repeat count
 */
void gt_player_set_repeat_count(gt_obj_st * obj, int32_t count);

#if GT_PLAYER_REPEAT_FINISH_CALLBACK
/**
 * @brief
 *
 * @param obj
 * @param cb
 * @param parms
 */
void gt_player_set_repeat_finish_callback(gt_obj_st * obj, gt_event_cb_t cb, void * parms);
#endif

/**
 * @brief Is forward direction to play
 *
 * @param obj
 * @return true Forward direction
 * @return false Backward direction[default]
 */
bool gt_player_dir_is_forward(gt_obj_st * obj);

/**
 * @brief Set real-time jump mode refresh
 *
 * @param obj
 * @param is_real_time_update true: enabled; false: disabled
 */
void gt_player_set_real_time_update(gt_obj_st * obj, bool is_real_time_update);
bool gt_player_is_real_time_update(gt_obj_st * obj);


#if GT_USE_SERIAL
void gt_player_set_index_offset(gt_obj_st * obj, gt_size_t offset);

#if GT_USE_FILE_HEADER
void gt_player_set_stop_item_by_file_header(gt_obj_st * obj, gt_file_header_param_st* fh);
#endif // GT_USE_FILE_HEADER

#endif // GT_USE_SERIAL

#endif /** GT_CFG_ENABLE_PLAYER */
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_PLAYER_H_
