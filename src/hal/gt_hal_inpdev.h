/**
 * @file gt_hal_inpdev.h
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-05-19 17:33:48
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_HAL_INPDEV_H_
#define _GT_HAL_INPDEV_H_

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
    GT_INDEV_STATE_RELEASED,
    GT_INDEV_STATE_PRESSED,
}gt_indev_state_et;

typedef struct {
    gt_point_st point; /**< For LV_INDEV_TYPE_POINTER the currently pressed point*/
    uint32_t key;     /**< For LV_INDEV_TYPE_KEYPAD the currently pressed key*/
    uint32_t btn_id;  /**< For LV_INDEV_TYPE_BUTTON the currently pressed button*/
    int16_t enc_diff; /**< For LV_INDEV_TYPE_ENCODER number of steps since the previous read*/

    gt_indev_state_et state; /**< LV_INDEV_STATE_REL or LV_INDEV_STATE_PR*/
	uint8_t continue_reading:1;  /**< If set to true, the read callback is invoked again*/
} gt_indev_data_st;

typedef struct _gt_indev_drv_s
{
    gt_indev_type_et type;
    void (*read_cb)(struct _gt_indev_drv_s * indev_drv, gt_indev_data_st * data);
#if GT_USE_USER_DATA
    void * user_data;
#endif
}gt_indev_drv_st;

typedef struct _gt_indev_s
{
    gt_indev_drv_st * drv;

}gt_indev_st;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_indev_drv_init();
gt_indev_st * gt_indev_drv_register(gt_indev_drv_st * drv);
gt_indev_st * gt_indev_get_default();

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_HAL_INPDEV_H_
