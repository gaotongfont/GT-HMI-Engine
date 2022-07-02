/**
 * @file gt_hal_indev.h
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-19 17:33:48
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
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

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef enum _gt_indev_type_e{
    GT_INDEV_TYPE_NONE,        /* nothing inpdev */
    GT_INDEV_TYPE_POINTER,     /* touchpad, mouse */
    GT_INDEV_TYPE_KEYPAD,      /* keypad */
    GT_INDEV_TYPE_BUTTON,      /* button */
}gt_indev_type_et;

typedef enum _gt_indev_state_e{
    GT_INDEV_STATE_RELEASED,        /* released */
    GT_INDEV_STATE_PRESSED,         /* key down */
}gt_indev_state_et;

typedef enum{
    GT_DIR_NONE,
    GT_DIR_UP,
    GT_DIR_RIGHT,
    GT_DIR_DOWN,
    GT_DIR_LEFT,
}gt_dir_et;

typedef struct {
    gt_point_st point; /**< For GT_INDEV_TYPE_POINTER the currently pressed point*/
    uint32_t key;     /**< For GT_INDEV_TYPE_KEYPAD the currently pressed key*/
    uint32_t btn_id;  /**< For GT_INDEV_TYPE_BUTTON the currently pressed button*/
    int16_t enc_diff; /**< For GT_INDEV_TYPE_ENCODER number of steps since the previous read*/

    gt_indev_state_et state; /**< GT_INDEV_STATE_REL or GT_INDEV_STATE_PR*/
	uint8_t continue_reading:1;  /**< If set to true, the read callback is invoked again*/
} gt_indev_data_st;

typedef struct _gt_indev_drv_s
{
    gt_indev_type_et type;
    void (*read_cb)(struct _gt_indev_drv_s * indev_drv, gt_indev_data_st * data);
#if GT_USE_USER_DATA
    void * user_data;
#endif

    gt_size_t limit_timers_long_press;           /* long press timers */
    gt_size_t limit_pixel_gesture_ver;          /* move pixel trigger once event */
    gt_size_t limit_pixel_gesture_hor;          /* move pixel trigger once event */
    gt_size_t limit_pixel_scroll;               /* scroll pixel trigger scroll event */

}gt_indev_drv_st;

typedef struct _gt_indev_proc_s
{
    gt_indev_state_et state;
    union
    {
        struct _point
        {
            gt_point_st point_act;      /* current point of touch input device */
            gt_point_st point_last;     /* last point of touch input device */
            gt_point_st point_temp;     /* used to judge gesture */
            gt_dir_et gesture;          /* gesture of touch input device */

            struct _gt_obj_s * obj_act;     /* The object being pressed */
            struct _gt_obj_s * obj_last;    /* The object of last point */
            struct _gt_obj_s * obj_scroll;  /* The object of gesture */

            gt_size_t count_point;
        }point;

        struct _button
        {
            uint16_t id;
            uint16_t count_keydown;
        }button;

        struct _keypad
        {
            uint16_t id;
        }keypad;

    }data;

}gt_indev_proc_st;


typedef struct _gt_indev_s
{
    gt_indev_drv_st * drv;
    gt_indev_proc_st proc;          /* Save all input device data */

}gt_indev_st;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_indev_drv_init(gt_indev_drv_st * drv);
gt_indev_st * gt_indev_drv_register(gt_indev_drv_st * drv);
gt_indev_st * gt_indev_get_default(void);
gt_indev_st * gt_indev_get_dev_by_nub(uint8_t nub);
uint8_t gt_indev_get_dev_num(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_HAL_INDEV_H_
