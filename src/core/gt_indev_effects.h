/**
 * @file gt_indev_effects.h
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2025-09-10 12:03:26
 * @copyright Copyright (c) 2014-2025, Company Genitop. Co., Ltd.
 */
#ifndef _GT_INDEV_EFFECTS_H_
#define _GT_INDEV_EFFECTS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../widgets/gt_conf_widgets.h"

#if GT_CFG_ENABLE_INDEV_EFFECTS
#include <stdbool.h>
#include <stdint.h>
#include "gt_event.h"
#include "gt_color.h"
/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_indev_effects_show(gt_event_type_et gesture_type, uint32_t distance);
void gt_indev_effects_hide(void);
void gt_indev_effects_set_color(gt_color_t color);
void gt_indev_effects_set_opa(uint8_t opa);



#endif //! GT_CFG_ENABLE_INDEV_EFFECTS
//
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_INDEV_EFFECTS_H_

/* end of file ----------------------------------------------------------*/


