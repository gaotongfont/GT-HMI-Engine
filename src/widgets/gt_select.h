/**
 * @file gt_select.h
 * @author Feyoung
 * @brief Combined controls are composed of input box controls and list controls.
 *      Listening GT_EVENT_TYPE_UPDATE_VALUE event to get the selected gt_input_hide_value.
 *      Get the select item text such as:
 *      ```
 *      static void _select_cb(gt_event_st * e) {
 *      	if (e->param) {
 *      	  GT_LOG_A("", "select: [%s]", e->param);
 *      	}
 *      }
 *      ```
 * @version 0.1
 * @date 2024-12-03 15:59:26
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SELECT_H_
#define _GT_SELECT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_SELECT
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "stdarg.h"
#include "stdlib.h"
#include "../core/gt_style.h"
#include "../font/gt_font.h"



/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief Create a select widget, detail @see gt_select.h's "brief" comments
 *
 * @param parent
 * @return gt_obj_st*
 */
gt_obj_st * gt_select_create(gt_obj_st * parent);

void gt_select_add_option(gt_obj_st * obj, const char * text);
void gt_select_clear_all_options(gt_obj_st * obj);
void gt_select_set_option_height(gt_obj_st * obj, uint16_t height);


#endif  /** GT_CFG_ENABLE_SELECT */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SELECT_H_
