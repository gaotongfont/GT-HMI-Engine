/**
 * @file gt_switch.h
 * @author yongg
 * @brief switch controller
 * @version 0.1
 * @date 2022-07-21 14:34:33
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SWITCH_H_
#define _GT_SWITCH_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_SWITCH
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "../core/gt_style.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef enum gt_switch_style_e {
    GT_SWITCH_STYLE_DEFAULT = 0,    /** Focus tag is round style */
    GT_SWITCH_STYLE_RECT,           /** Focus tag is rect style */
    GT_SWITCH_STYLE_AXIS,           /** Thin axis style */
}gt_switch_style_et;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief create switch obj
 *
 * @param parent switch's parent obj
 * @return gt_obj_st* switch obj ptr
 */
gt_obj_st * gt_switch_create(gt_obj_st * parent);

/**
 * @brief active color
 *
 * @param switcher
 * @param color default: 0x13ce66
 */
void gt_switch_set_color_act(gt_obj_st * switcher, gt_color_t color);

/**
 * @brief inactivate color
 *
 * @param switcher
 * @param color default: 0xebeef5
 */
void gt_switch_set_color_ina(gt_obj_st * switcher, gt_color_t color);

/**
 * @brief focus point color
 *
 * @param switcher
 * @param color default: 0xffffff
 */
void gt_switch_set_color_point(gt_obj_st * switcher, gt_color_t color);

/**
 * @brief Set the dividing line color
 *
 * @param switcher
 * @param color Default: 0xdcdfe6
 */
void gt_switch_set_color_divider(gt_obj_st * switcher, gt_color_t color);

/**
 * @brief set switcher style
 *
 * @param switcher
 * @param sw_style
 */
void gt_switch_set_style(gt_obj_st * switcher, gt_switch_style_et sw_style);

/**
 * @brief Set the switcher's dividing line
 *
 * @param switcher
 * @param is_div_line
 */
void gt_switch_set_div_line(gt_obj_st * switcher, bool is_div_line);


#endif  /** GT_CFG_ENABLE_SWITCH */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SWITCH_H_
