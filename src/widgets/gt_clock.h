/**
 * @file gt_clock.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2023-06-05 15:22:09
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_CLOCK_H_
#define _GT_CLOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_CLOCK
#include "gt_obj.h"
#include "../font/gt_font.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
/**
 * @brief After 24 hours, execute the callback function
 */
typedef void (* gt_clock_next_day_cb)(gt_obj_st * obj, void * user_data);

/**
 * @brief In the countdown timer mode, the callback is executed
 *      when the time reaches 00:00:00.
 *      In clock mode, the callback is executed when the target
 *      time is reached.
 */
typedef void (* gt_clock_alert_cb)(gt_obj_st * obj, void * user_data);

/**
 * @brief Clock working mode [default: GT_CLOCK_MODE_TIME]
 */
typedef enum gt_clock_mode_e {
    GT_CLOCK_MODE_TIME = 0,     /** normal timer mode */
    GT_CLOCK_MODE_COUNTDOWN,    /** countdown timer mode */

    GT_CLOCK_MODE_MAX_COUNT,
}gt_clock_mode_et;

typedef struct gt_clock_time_s {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
}gt_clock_time_st;

/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_clock_create(gt_obj_st * parent);

/**
 * @brief Set the current time
 *
 * @param obj
 * @param hour
 * @param minute
 * @param second
 */
void gt_clock_set_time(gt_obj_st * obj, uint8_t hour, uint8_t minute, uint8_t second);
void gt_clock_set_time_by_timestamp(gt_obj_st * obj, uint32_t timestamp);

/**
 * @brief Set the alert time, which usd to alert callback function
 *
 * @param obj
 * @param hour
 * @param minute
 * @param second
 */
void gt_clock_set_alert_time(gt_obj_st * obj, uint8_t hour, uint8_t minute, uint8_t second);
void gt_clock_set_alert_time_by_timestamp(gt_obj_st * obj, uint32_t timestamp);

/**
 * @brief Get current time
 *
 * @param obj The time widget
 * @param time NULL: only return the timestamp; not NULL: return the time by the struct.
 * @return uint32_t return the timestamp
 */
uint32_t gt_clock_get_time(gt_obj_st * obj, gt_clock_time_st * time);

/**
 * @brief Get the alert time
 *
 * @param obj The time widget
 * @param time NULL: only return the timestamp; not NULL: return the time by the struct.
 * @return uint32_t return the timestamp
 */
uint32_t gt_clock_get_alert_time(gt_obj_st * obj, gt_clock_time_st * time);

/**
 * @brief Switch the state of the clock, This function can use
 *      an external RTC for more accurate time control
 *
 * @param obj
 */
void gt_clock_turn_next_second(gt_obj_st * obj);

void gt_clock_turn_prev_second(gt_obj_st * obj);

/**
 * @brief Set widget working mode, [default: GT_CLOCK_MODE_TIME],
 *      @ref gt_clock_mode_et
 *
 * @param obj
 * @param mode @ref gt_clock_mode_et
 */
void gt_clock_set_mode(gt_obj_st * obj, gt_clock_mode_et mode);
gt_clock_mode_et gt_clock_get_mode(gt_obj_st * obj);

/**
 * @brief Enabled the 12-hour clock, [default: false]
 *      example: "13:25:40", enabled is: "01:25:40", otherwise is: "13:25:40".
 *
 * @param obj
 * @param enabled true: enabled; false: disabled
 */
void gt_clock_set_12_hours_mode(gt_obj_st * obj, bool enabled);

/**
 * @brief Get the 12-hour clock status, [default: false]
 *      example: "13:25:40", enabled is: "01:25:40", otherwise is: "13:25:40".
 *
 * @param obj
 * @return true: enabled; false: disabled
 */
bool gt_clock_get_12_hours_mode(gt_obj_st * obj);

/**
 * @brief It's only valid when the 12-hour clock is on, enabled is display "AM/PM"
 *      behind the time.
 *      [default: false]
 *
 * @param obj
 * @param enabled true: display "AM/PM"; false: not display "AM/PM"
 */
void gt_clock_set_meridiem_mode(gt_obj_st * obj, bool enabled);

/**
 * @brief It's only valid when the 12-hour clock is on, when result is enabled,
 *      it display "AM/PM" behind the time.
 *      [default: false]
 *
 * @param obj
 * @return true Enabled the "AM/PM" display content
 * @return false Disabled
 */
bool gt_clock_get_meridiem_mode(gt_obj_st * obj);

/**
 * @brief Sets the format of the time display, default is "hh:mm:ss"
 *      [HH / hh]: hour
 *      [MM / mm]: minute
 *      [SS / ss]: second
 *      example: "hh|MM" -> "12|00".
 *
 * @param obj
 * @param format format string
 */
void gt_clock_set_format(gt_obj_st * obj, const char * const format);

/**
 * @brief The current callback function is executed 24 hours after the time elapsed
 *
 * @param obj
 * @param next_day_cb The callback function
 * @param user_data The user data
 */
void gt_clock_set_next_day_cb(gt_obj_st * obj, gt_clock_next_day_cb next_day_cb, void * user_data);

/**
 * @brief The current callback function is executed when the timer Arrive
 *      at the set time or countdown timer mode is set to 00:00:00
 *
 * @param obj
 * @param alert_cb The callback function
 * @param user_data The user data
 */
void gt_clock_set_alert_cb(gt_obj_st * obj, gt_clock_alert_cb alert_cb, void * user_data);

/**
 * @brief clear all next day callback function
 *
 * @param obj
 */
void gt_clock_clr_all_next_day_cb(gt_obj_st * obj);

/**
 * @brief clear all alert callback function
 *
 * @param obj
 */
void gt_clock_clr_all_alert_cb(gt_obj_st * obj);

/**
 * @brief Remove the specified next day callback function
 *
 * @param obj
 * @param next_day_cb The callback function
 * @return true The callback function is removed successfully
 * @return false The callback function is not found or delete failed
 */
bool gt_clock_remove_next_day_cb(gt_obj_st * obj, gt_clock_next_day_cb next_day_cb);

/**
 * @brief Remove the specified alert callback function
 *
 * @param obj
 * @param alert_cb The callback function
 * @return true The callback function is removed successfully
 * @return false The callback function is not found or delete failed
 */
bool gt_clock_remove_alert_cb(gt_obj_st * obj, gt_clock_alert_cb alert_cb);

/**
 * @brief Run the clock using the default timer
 *
 * @param obj
 */
void gt_clock_start(gt_obj_st * obj);

/**
 * @brief Stop the clock using the default timer
 *
 * @param obj
 */
void gt_clock_stop(gt_obj_st * obj);

/**
 * @brief Toggle the clock state
 *
 * @param obj
 */
void gt_clock_toggle(gt_obj_st * obj);

void gt_clock_set_font_color(gt_obj_st * obj, gt_color_t color);
void gt_clock_set_font_size(gt_obj_st * obj, uint8_t size);
void gt_clock_set_font_gray(gt_obj_st * obj, uint8_t gray);
void gt_clock_set_font_align(gt_obj_st * obj, gt_align_et align);
void gt_clock_set_font_family_cn(gt_obj_st * obj, gt_family_t family);
void gt_clock_set_font_family_en(gt_obj_st * obj, gt_family_t family);
void gt_clock_set_font_family_fl(gt_obj_st * obj, gt_family_t family);
void gt_clock_set_font_family_numb(gt_obj_st * obj, gt_family_t family);
void gt_clock_set_font_thick_en(gt_obj_st * obj, uint8_t thick);
void gt_clock_set_font_thick_cn(gt_obj_st * obj, uint8_t thick);
void gt_clock_set_space(gt_obj_st * obj, uint8_t space_x, uint8_t space_y);


#endif /** #if GT_CFG_ENABLE_CLOCK */
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_CLOCK_H_
