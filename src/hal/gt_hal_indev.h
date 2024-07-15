/**
 * @file gt_hal_indev.h
 * @author yongg
 * @brief Hal layer implementation for input devices
 * @version 0.1
 * @date 2022-05-19 17:33:48
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_HAL_INDEV_H_
#define _GT_HAL_INDEV_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"
#include "../gt_conf.h"
#include "../widgets/gt_obj.h"
#include "../others/gt_anim.h"

/* define ---------------------------------------------------------------*/

/* default gesture repeat */

/* ms repeat trigger once event */
#define GT_CFG_DEFAULT_POINT_LONG_PRESS_TIMERS      300

#define GT_CFG_DEFAULT_POINT_GERSTURE_PIXEL_HOR     (GT_SCREEN_WIDTH >> 4)

#define GT_CFG_DEFAULT_POINT_GERSTURE_PIXEL_VER     (GT_SCREEN_HEIGHT >> 4)

#define GT_CFG_DEFAULT_POINT_SCROLL_PIXEL           3

/** Drag throw slow-down in [%]. Greater value: faster(far)[0] -> slow-down[100] */
#define GT_CFG_DEFAULT_POINT_SCROLL_THROW           10

#define GT_CFG_DEFAULT_POINT_SCROLL_LIMIT           10

/** home gesture default time and pix */
#define GT_CFG_DEFAULT_POINT_HOME_GESTURE_TIME              (500)   //ms
#define GT_CFG_DEFAULT_POINT_HOME_GESTURE_PIXEL             (20)    //px
#define GT_CFG_DEFAULT_AREA_HOME_GESTURE_TOP                (GT_CFG_DEFAULT_POINT_HOME_GESTURE_PIXEL)    //@ref GT_EVENT_TYPE_INPUT_HOME_GESTURE_TOP
#define GT_CFG_DEFAULT_AREA_HOME_GESTURE_BOTTOM             (GT_CFG_DEFAULT_POINT_HOME_GESTURE_PIXEL)    //@ref GT_EVENT_TYPE_INPUT_HOME_GESTURE_BOTTOM
#define GT_CFG_DEFAULT_AREA_HOME_GESTURE_LEFT               (GT_CFG_DEFAULT_POINT_HOME_GESTURE_PIXEL)    //@ref GT_EVENT_TYPE_INPUT_HOME_GESTURE_LEFT
#define GT_CFG_DEFAULT_AREA_HOME_GESTURE_RIGHT              (GT_CFG_DEFAULT_POINT_HOME_GESTURE_PIXEL)    //@ref GT_EVENT_TYPE_INPUT_HOME_GESTURE_RIGHT

/* typedef --------------------------------------------------------------*/

/**
 * @brief input device type enum
 */
typedef enum _gt_indev_type_e {
    GT_INDEV_TYPE_NONE,        ///< nothing input dev
    GT_INDEV_TYPE_POINTER,     ///< touch-pad, mouse
    GT_INDEV_TYPE_KEYPAD,      ///< keypad
    GT_INDEV_TYPE_BUTTON,      ///< button
}gt_indev_type_et;

/**
 * @brief button or touch pad, input status
 */
typedef enum _gt_indev_state_e {
    GT_INDEV_STATE_RELEASED = 0,    ///< released
    GT_INDEV_STATE_PRESSED,         ///< key down
    GT_INDEV_STATE_INVALID,         ///< invalid
}gt_indev_state_et;

/**
 * @brief direction definition enum
 */
typedef enum{
    GT_DIR_NONE = 0,  ///< default

    GT_DIR_UP   ,     ///< upward
    GT_DIR_DOWN ,     ///< downward

    GT_DIR_LEFT ,     ///< left
    GT_DIR_RIGHT,     ///< right
}gt_dir_et;

typedef enum {
    GT_KEY_UP        = 17,  /*0x11*/
    GT_KEY_DOWN      = 18,  /*0x12*/
    GT_KEY_RIGHT     = 19,  /*0x13*/
    GT_KEY_LEFT      = 20,  /*0x14*/
    GT_KEY_ESC       = 27,  /*0x1B*/
    GT_KEY_DEL       = 127, /*0x7F*/
    GT_KEY_BACKSPACE = 8,   /*0x08*/
    GT_KEY_ENTER     = 10,  /*0x0A, '\n'*/
    GT_KEY_NEXT      = 9,   /*0x09, '\t'*/
    GT_KEY_PREV      = 11,  /*0x0B, '*/
    GT_KEY_HOME      = 2,   /*0x02, STX*/
    GT_KEY_END       = 3,   /*0x03, ETX*/

    GT_KEY_NONE      = 0xFFFFFFF,
}gt_key_et;

/**
 * @brief Describes input device related data
 */
typedef struct {
    gt_point_st point; ///< For GT_INDEV_TYPE_POINTER the currently pressed point
    uint32_t key;     ///< For GT_INDEV_TYPE_KEYPAD the currently pressed key
    uint32_t btn_id;  ///< For GT_INDEV_TYPE_BUTTON the currently pressed button
    int16_t enc_diff; ///< For GT_INDEV_TYPE_ENCODER number of steps since the previous read

    gt_indev_state_et state; ///< GT_INDEV_STATE_REL or GT_INDEV_STATE_PR
	uint8_t continue_reading:1;  ///< If set to true, the read callback is invoked again
    uint8_t reserved:7;          ///< Reserved for future use
} gt_indev_data_st;

/**
 * @brief Describes input device and driver
 */
typedef struct _gt_indev_drv_s
{
    gt_indev_type_et type;
    void (*read_cb)(struct _gt_indev_drv_s * indev_drv, gt_indev_data_st * data);

    gt_size_t limit_timers_long_press;          ///< long press timers
    gt_size_t limit_pixel_gesture_ver;          ///< move pixel trigger once event
    gt_size_t limit_pixel_gesture_hor;          ///< move pixel trigger once event
    uint8_t limit_pixel_scroll;               ///< scroll pixel trigger scroll event
    uint8_t limit_scroll_throw;               ///< [%]

}gt_indev_drv_st;

/**
 * @brief input device information
 */
typedef struct _gt_indev_s {
    gt_indev_drv_st * drv;

    uint32_t timestamp_start;
    uint32_t timestamp_long_press;

    gt_indev_state_et state;
    union
    {
        struct _point
        {
            gt_point_st act;            ///< the first point of touch input device
            gt_point_st newly;          ///< last point of touch input device
            gt_point_st last;           ///< used to judge gesture

            gt_point_st scroll_diff;     ///< the distance between two coordinates
            gt_point_st scroll_sum;      ///< the sum of distance
            gt_point_st scroll_throw;    ///< used to judge scroll

            struct gt_obj_s * obj_act;       ///< The object which is touching now
            struct gt_obj_s * obj_target;    ///< The object of first touch
            struct gt_obj_s * obj_origin;    //< The object of first touch
            struct gt_obj_s * obj_scroll;    ///< The object of gesture

            uint16_t gesture : 3;           ///< @ref gt_dir_et gesture of touch input device point
            uint16_t reserved : 13;
        }point;

        struct _button
        {
            uint16_t id;
            uint16_t count_keydown;
        }button;

        struct _keypad
        {
            uint32_t key;
            uint32_t count_keydown;
            struct gt_obj_s * obj_target;    ///< The object of first touch
        }keypad;
    }data;

    gt_anim_st * scroll_throw_anim;
}gt_indev_st;

typedef struct gt_indev_param_st {
    union
    {
        gt_point_st point;
        uint16_t button_id;
        uint32_t keypad_key;
    }param;

    uint8_t enabled : 1;   /** When screen anim used, 1: default set, normal; 0: temporary disability */
    uint8_t reserved : 7;
}gt_indev_param_st;


/* macros ---------------------------------------------------------------*/

static inline void _gt_indev_point_set_value(gt_point_st * point, gt_size_t x, gt_size_t y) {
    point->x = x;
    point->y = y;
}

static inline bool _gt_indev_point_equal(gt_point_st const * const dst, gt_point_st const * const src) {
    if (dst == src) {
        return true;
    }
    if (dst->x != src->x) {
        return false;
    }
    if (dst->y != src->y) {
        return false;
    }
    return true;
}

/* global functions / API interface -------------------------------------*/
/**
 * @brief input device init
 *
 * @param drv input device
 */
void gt_indev_drv_init(gt_indev_drv_st * drv);

/**
 * @brief input device register into core.
 *
 * @param drv input device
 * @return gt_indev_st*
 */
gt_indev_st * gt_indev_drv_register(gt_indev_drv_st * drv);

/**
 * @brief Get default input device
 *
 * @return gt_indev_st*
 */
gt_indev_st * gt_indev_get_default(void);

/**
 * @brief Get input device by number
 *
 * @param idx index of input device
 * @return gt_indev_st*
 */
gt_indev_st * gt_indev_get_dev_by_idx(uint8_t idx);

/**
 * @brief Get input device count
 *
 * @return uint8_t
 */
uint8_t gt_indev_get_dev_count(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_HAL_INDEV_H_
