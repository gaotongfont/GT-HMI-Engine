/**
 * @file gt_media_player.h
 * @author Feyoung
 * @brief Media player control
 * @version 0.1
 * @date 2024-08-23 11:10:20
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_MEDIA_PLAYER_H_
#define _GT_MEDIA_PLAYER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_MEDIA_PLAYER
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "../others/gt_color.h"
#include "./gt_img.h"
#include "./gt_slider.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
/**
 * @brief When media triggers the play or stop event, the callback function is called
 */
typedef void ( * gt_media_player_handler_cb)(gt_obj_st * obj, void * user_data);


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief Create media player
 *
 * @param parent
 * @return gt_obj_st*
 */
gt_obj_st * gt_media_player_create(gt_obj_st * parent);

/**
 * @brief Set the callback function when the media triggers the play event
 *
 * @param obj
 * @param play_cb Callback function
 * @param user_data
 */
void gt_media_player_set_play_cb(gt_obj_st * obj, gt_media_player_handler_cb play_cb, void * user_data);

/**
 * @brief Set the callback function when the media triggers the stop event
 *
 * @param obj
 * @param stop_cb Callback function
 * @param user_data
 */
void gt_media_player_set_stop_cb(gt_obj_st * obj, gt_media_player_handler_cb stop_cb, void * user_data);

/**
 * @brief Setting frame data
 *
 * @param obj
 * @param raw  Raw data of the frame
 */
void gt_media_player_set_raw(gt_obj_st * obj, gt_color_img_raw_st * raw);

/**
 * @brief Setting media total second time
 *
 * @param obj
 * @param total_time second[s]
 */
void gt_media_player_set_total_time(gt_obj_st * obj, gt_size_t total_time);

/**
 * @brief Set the current second time of the media
 *
 * @param obj
 * @param current_time
 */
void gt_media_player_set_current_time(gt_obj_st * obj, gt_size_t current_time);

/**
 * @brief Get the current second time of the media
 *
 * @param obj
 * @return gt_size_t second
 */
gt_size_t gt_media_player_get_current_time(gt_obj_st * obj);

/**
 * @brief Calling by the slider bar to set the current time
 *
 * @param obj
 * @param value_change_cb Callback function
 * @param user_data
 */
void gt_media_player_set_slider_change_cb(gt_obj_st * obj, gt_event_cb_t value_change_cb, void * user_data);


#endif  /** GT_CFG_ENABLE_MEDIA_PLAYER */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_MEDIA_PLAYER_H_
