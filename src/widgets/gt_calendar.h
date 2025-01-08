/**
 * @file gt_calendar.h
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2025-01-02 10:22:15
 * @copyright Copyright (c) 2014-2025, Company Genitop. Co., Ltd.
 */
#ifndef _GT_CALENDAR_H_
#define _GT_CALENDAR_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"
//
#if GT_CFG_ENABLE_CALENDAR
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "stdarg.h"
#include "stdlib.h"
#include "../utils/gt_vector.h"
#include "../core/gt_style.h"
#include "../font/gt_font.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef enum{
    GT_CALENDAR_DAY_TYPE_NORMAL = 0, // normal
    GT_CALENDAR_DAY_TYPE_HOLIDAY,  // day_type
    GT_CALENDAR_DAY_TYPE_WORKDAY,  // workday
}gt_calendar_day_type_et;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief create a calendar widget
 *
 * @param parent
 * @return gt_obj_st*
 */
gt_obj_st * gt_calendar_create(gt_obj_st * parent);
/**
 * @brief Set the calendar to display Chinese dates
 *
 * @param obj
 * @param use_cn true: use Chinese dates; false: use English dates
 */
void gt_calendar_set_cn(gt_obj_st * obj, bool is_cn);
/**
 * @brief set today's date for the calendar
 *
 * @param obj
 * @param year
 * @param month
 * @param day
 */
void gt_calendar_set_today(gt_obj_st * obj, uint16_t year, uint8_t month, uint8_t day);
/**
 * @brief set day type for the calendar
 *
 * @param obj
 * @param year
 * @param month
 * @param day
 * @param day_type @gt_calendar_day_type_et
 */
void gt_calendar_set_day_type(gt_obj_st * obj, uint16_t year, uint8_t month, uint8_t day, gt_calendar_day_type_et day_type);

/**
 * @brief set the border width of the calendar
 *
 * @param obj
 * @param width border width
 */
void gt_calendar_set_border_width(gt_obj_st * obj, uint8_t width);\
/**
 * @brief set the border color of the calendar
 *
 * @param obj
 * @param color border color
 */
void gt_calendar_set_border_color(gt_obj_st * obj, gt_color_t color);

/**
 * @brief set today's display font color for the calende
 *
 * @param obj
 * @param color
 */
void gt_calendar_set_font_color_to_today(gt_obj_st * obj, gt_color_t color);
/**
 * @brief set weekend's display font color for the calende
 *
 * @param obj
 * @param color
 */
void gt_calendar_set_font_color_to_weekend(gt_obj_st * obj, gt_color_t color);
/**
 * @brief set other month's display font color for the calende
 *
 * @param obj
 * @param color
 */
void gt_calendar_set_font_color_to_other_month(gt_obj_st * obj, gt_color_t color);

void gt_calendar_set_font_align(gt_obj_st * obj, gt_align_et align);
void gt_calendar_set_font_color(gt_obj_st * obj, gt_color_t color);
void gt_calendar_set_font_size(gt_obj_st * obj, uint8_t size);
void gt_calendar_set_font_gray(gt_obj_st * obj, uint8_t gray);
void gt_calendar_set_font_thick_en(gt_obj_st * obj, uint8_t thick);
void gt_calendar_set_font_thick_cn(gt_obj_st * obj, uint8_t thick);
void gt_calendar_set_font_style(gt_obj_st * obj, gt_font_style_et font_style);
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
void gt_calendar_set_font_family_cn(gt_obj_st * obj, gt_family_t font_family_cn);
void gt_calendar_set_font_family_en(gt_obj_st * obj, gt_family_t font_family_en);
void gt_calendar_set_font_family_fl(gt_obj_st * obj, gt_family_t font_family_fl);
void gt_calendar_set_font_family_numb(gt_obj_st * obj, gt_family_t font_family_numb);
#else
void gt_calendar_set_font_family(gt_obj_st * obj, gt_family_t font_family);
void gt_calendar_set_font_cjk(gt_obj_st* obj, gt_font_cjk_et cjk);
#endif

#endif  // !GT_CFG_ENABLE_CALENDAR
//
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_CALENDAR_H_

/* end of file ----------------------------------------------------------*/


