/**
 * @file gt_spectrum.h
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2025-05-16 14:43:41
 * @copyright Copyright (c) 2014-2025, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SPECTRUM_H_
#define _GT_SPECTRUM_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"
//
#if GT_CFG_ENABLE_SPECTRUM
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "../core/gt_style.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef enum
{
    GT_SPECTRUM_TYPE_CIRCLE = 0,    // circle
    GT_SPECTRUM_TYPE_RECT,          // rect
    GT_SPECTRUM_TYPE_RECT_UD,       // rect up-down
} gt_spectrum_type_et;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief create spectrum widget
 *
 * @param parent parent widget
 * @return gt_obj_st*
 */
gt_obj_st * gt_spectrum_create(gt_obj_st * parent);

/**
 * @brief set spectrum type
 *
 * @param obj
 * @param type spectrum type @ gt_spectrum_type_et
 */
void gt_spectrum_set_type(gt_obj_st * obj, gt_spectrum_type_et type);

/**
 * @brief set spectrum period
 *
 * @param obj
 * @param period period [ms]
 */
void gt_spectrum_set_period(gt_obj_st * obj, uint32_t period);

/**
 * @brief play spectrum
 *
 * @param obj
 */
void gt_spectrum_play(gt_obj_st * obj);

/**
 * @brief stop spectrum
 *
 * @param obj
 */
void gt_spectrum_stop(gt_obj_st * obj);

/**
 * @brief set spectrum number
 *
 * @param obj
 * @param numb number of spectrum
 * @param gap gap between spectrum
 */
void gt_spectrum_set_numb(gt_obj_st * obj, uint16_t numb, uint16_t gap);

/**
 * @brief set spectrum range
 *
 * @param obj
 * @param min min value
 * @param max max value
 * @param steps steps
 */
void gt_spectrum_set_range(gt_obj_st * obj, uint16_t min, uint16_t max, uint16_t steps);

/**
 * @brief set spectrum gradient color
 *
 * @param obj
 * @param start start color
 * @param end end color
 * @param steps steps
 */
void gt_spectrum_set_gradient_color(gt_obj_st * obj, gt_color_t start, gt_color_t end, uint16_t steps);


#endif // !GT_CFG_ENABLE_SPECTRUM
//
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SPECTRUM_H_

/* end of file ----------------------------------------------------------*/


