/**
 * @file gt_indev.h
 * @author yongg
 * @brief input device handler
 * @version 0.1
 * @date 2022-06-06 14:06:12
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_INDEV_H_
#define _GT_INDEV_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../hal/gt_hal_indev.h"
#include "./gt_timer.h"


/* define ---------------------------------------------------------------*/
#ifndef GT_INDEV_HOME_GESTURE_HOR_MAX_DISTANCE
    /**
     * @brief hor max distance of home gesture
     *      [default: 1/8 screen width]
     */
    #define GT_INDEV_HOME_GESTURE_HOR_MAX_DISTANCE      (GT_SCREEN_WIDTH >> 3)
#endif

#if GT_USE_TOUCH_QUEUE
#ifndef GT_TOUCH_QUEUE_MAX_PENDING
    /**
     * @brief Max pending events to drain from queue per frame.
     *      [default: 32]
     */
    #define GT_TOUCH_QUEUE_MAX_PENDING      32
#endif

/**
 * @brief A single touch event from the driver's event thread.
 *        Compact: 12 bytes per entry.
 */
typedef struct _gt_touch_event_s {
    int16_t x;              ///< Physical X coordinate (before rotation transform)
    int16_t y;              ///< Physical Y coordinate
    uint8_t finger_id;      ///< Driver-provided touch/finger identifier, 255 = mouse/emu
    uint8_t state;          ///< @ref gt_indev_state_et (PRESSED/RELEASED, MOTION = PRESSED)
    uint8_t reserved[2];    ///< Align to 12 bytes
} gt_touch_event_st;

/**
 * @brief Lock-free SPSC ring buffer for touch events.
 *        Producer: driver/SDL thread. Consumer: engine indev timer.
 */
typedef struct _gt_touch_queue_s {
    gt_touch_event_st buffer[GT_TOUCH_QUEUE_SIZE];
    volatile uint8_t write_idx;   ///< Only producer writes
    volatile uint8_t read_idx;    ///< Only consumer writes
} gt_touch_queue_st;

#endif  /* GT_USE_TOUCH_QUEUE */



/* typedef --------------------------------------------------------------*/\
#if GT_USE_UD_LR_TO_CONTROL_FOCUS_EN
typedef enum{
    GT_FOCUS_DIR_RIGHT = 0,
    GT_FOCUS_DIR_LEFT,
    GT_FOCUS_DIR_DOWN,
    GT_FOCUS_DIR_UP,
}gt_focus_dir_et;
#endif

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief Get indev button key value
 *
 * @return uint32_t
 */
uint32_t gt_indev_get_key(void);

/**
 * @brief Get physical point value
 *
 * @return gt_point_st
 */
gt_point_st gt_indev_get_point(void);

/**
 * @brief get click first point
 *
 * @param indev input device
 * @param point click first point
 */
void gt_indev_get_point_act(const gt_indev_st * indev, gt_point_st * point);

/**
 * @brief get clicking newly point
 *
 * @param indev input device
 * @param point clicking newly point
 */
void gt_indev_get_point_newly(const gt_indev_st * indev, gt_point_st * point);

/**
 * @brief Get the gesture of the input device
 *
 * @param indev input device
 * @return gt_dir_et Direction of gesture
 */
gt_dir_et gt_indev_get_gesture(const gt_indev_st * indev);

/**
 * @brief Input processing handle
 */
void gt_indev_handler(struct _gt_timer_s * timer);

/**
 * @brief disabled input device
 *
 * @param disabled 0: disabled, 1: enabled [default: 1]
 */
void gt_indev_set_enabled(bool enabled);

/**
 * @brief
 *
 * @return true
 * @return false
 */
bool gt_indev_is_enabled(void);

/**
 * @brief reset all info when screen load new one.
 *
 * @return gt_res_t
 */
gt_res_t _gt_indev_reset_status(void);

/**
 * @brief check widget is illegal status, and reset to normal status.
 *      such as: rebound effect.
 *
 * @param screen
 * @return gt_res_t
 */
gt_res_t _gt_indev_check_reset_widgets_illegal_status(gt_obj_st * screen);

bool _gt_indev_remove_want_delate_target(gt_obj_st * target);

#if GT_USE_UD_LR_TO_CONTROL_FOCUS_EN

/**
 * @brief Release focus lock
 *
 */
void gt_indev_release_focus_lock(void);
#endif


#if GT_INDEV_SIMULATE_POINTER
void gt_indev_simulate_handler(gt_indev_state_et state, gt_size_t x, gt_size_t y);
#endif

#if GT_USE_TOUCH_QUEUE
/**
 * @brief Initialize the touch event queue. Must be called once during
 *        system init, before any touch drivers push events.
 */
void gt_indev_touch_queue_init(void);

/**
 * @brief Push a touch event into the async queue.
 *        Thread-safe for a single producer (driver/SDL thread).
 *        If the queue is full, the event is silently dropped.
 *
 * @param x         physical X coordinate
 * @param y         physical Y coordinate
 * @param state     GT_INDEV_STATE_PRESSED / RELEASED
 * @param finger_id driver-provided touch identifier (0..254, 255=mouse)
 */
void gt_indev_touch_queue_push(int16_t x, int16_t y, gt_indev_state_et state, uint8_t finger_id);

/**
 * @brief Flush and reset the async touch queue.
 *        Useful when the consumer falls behind or screen transitions.
 */
void gt_indev_touch_queue_reset(void);
#endif  /* GT_USE_TOUCH_QUEUE */

#if GT_USE_MULTI_TOUCH
/**
 * @brief Get the current number of active touch points.
 *
 * @return uint8_t 0 = no touch, 1 = single, 2+ = multi-touch
 */
uint8_t gt_indev_get_active_touch_count(void);

/**
 * @brief Get the pinch gesture midpoint (logical screen coordinates).
 *        Only meaningful during PINCH_UPDATE events.
 *
 * @return gt_point_st midpoint between the two fingers
 */
gt_point_st gt_indev_get_pinch_center(void);
#endif  /* GT_USE_MULTI_TOUCH */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_INDEV_H_
