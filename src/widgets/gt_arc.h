/**
 * @file gt_arc.h
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-11-14 14:53:21
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */
#ifndef _GT_ARC_H_
#define _GT_ARC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_ARC
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "stdarg.h"
#include "stdlib.h"
#include "../core/gt_style.h"

#if GT_USE_FILE_HEADER
#include "../hal/gt_hal_file_header.h"
#endif

#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
#include "../hal/gt_hal_fs.h"
#endif

#include "../core/gt_gradient.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief Create a arc pointer
 *
 * @param parent
 * @return gt_obj_st*
 */
gt_obj_st * gt_arc_create(gt_obj_st * parent);

/**
 * @brief Set the arc pointer rounded
 *
 * @param obj
 * @param rounded
 */
void gt_arc_set_rounded(gt_obj_st* obj, bool rounded);

/**
 * @brief Set the arc pointer active img
 *
 * @param obj
 * @param src
 */
void gt_arc_set_act_img(gt_obj_st * obj, char * src);

/**
 * @brief Set the arc pointer background img
 *
 * @param obj
 * @param src
 */
void gt_arc_set_bg_img(gt_obj_st * obj, char * src);

/**
 * @brief Set the arc pointer knob img
 *
 * @param obj
 * @param src
 */
void gt_arc_set_knob_img(gt_obj_st * obj, char * src);

#if GT_USE_FILE_HEADER
/**
 * @brief Set the arc pointer active img by file header
 *
 * @param obj
 * @param fh
 */
void gt_arc_set_act_by_file_header(gt_obj_st * obj, gt_file_header_param_st * fh);
/**
 * @brief Set the arc pointer background img by file header
 *
 * @param obj
 * @param fh
 */
void gt_arc_set_bg_by_file_header(gt_obj_st * obj, gt_file_header_param_st * fh);

/**
 * @brief Set the knob pointer active img by file header
 *
 * @param obj
 * @param fh
 */
void gt_arc_set_knob_by_file_header(gt_obj_st * obj, gt_file_header_param_st * fh);
#endif

#if GT_USE_DIRECT_ADDR
/**
 * @brief Set the arc pointer active img by direct address
 *
 * @param obj
 * @param addr
 */
void gt_arc_set_act_by_direct_addr(gt_obj_st * obj, gt_addr_t addr);
/**
 * @brief Set the arc pointer background img by direct address
 *
 * @param obj
 * @param addr
 */
void gt_arc_set_bg_by_direct_addr(gt_obj_st * obj, gt_addr_t addr);

/**
 * @brief Set the arc pointer knob img by direct address
 *
 * @param obj
 * @param addr
 */
void gt_arc_set_knob_by_direct_addr(gt_obj_st * obj, gt_addr_t addr);

#endif

#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
/**
 * @brief Set the arc pointer active img by direct address and custom size
 *
 * @param obj
 * @param dac
 */
void gt_arc_set_act_by_custom_size_addr(gt_obj_st * obj, gt_direct_addr_custom_size_st * dac);
/**
 * @brief Set the arc pointer background img by direct address and custom size
 *
 * @param obj
 * @param dac
 */
void gt_arc_set_bg_by_custom_size_addr(gt_obj_st * obj, gt_direct_addr_custom_size_st * dac);

/**
 * @brief Set the arc pointer knob img by direct address and custom size
 *
 * @param obj
 * @param dac
 */
void gt_arc_set_knob_by_custom_size_addr(gt_obj_st * obj, gt_direct_addr_custom_size_st * dac);
#endif

//
/**
 * @brief Set the arc pointer background color
 *
 * @param obj
 * @param color
 */
void gt_arc_set_bg_color(gt_obj_st* obj, gt_color_t color);
/**
 * @brief Set the arc pointer background start angle
 *
 * @param obj
 * @param angle
 */
void gt_arc_set_bg_start_angle(gt_obj_st* obj, uint16_t angle);
/**
 * @brief Set the arc pointer background end angle
 *
 * @param obj
 * @param angle
 */
void gt_arc_set_bg_end_angle(gt_obj_st* obj, uint16_t angle);
/**
 * @brief Set the arc pointer background start and end angle
 *
 * @param obj
 * @param start_angle
 * @param end_angle
 */
void gt_arc_set_bg_start_end_angle(gt_obj_st* obj, uint16_t start_angle, uint16_t end_angle);
/**
 * @brief Set the arc pointer background width
 *
 * @param obj
 * @param width
 */
void gt_arc_set_bg_width(gt_obj_st* obj, uint16_t width);

//
/**
 * @brief Set the arc pointer active color
 *
 * @param obj
 * @param color
 */
void gt_arc_set_act_color(gt_obj_st* obj, gt_color_t color);
/**
 * @brief Set the arc pointer active start angle
 *
 * @param obj
 * @param angle
 */
void gt_arc_set_act_start_angle(gt_obj_st* obj, uint16_t angle);
/**
 * @brief Set the arc pointer active end angle
 *
 * @param obj
 * @param angle
 */
void gt_arc_set_act_end_angle(gt_obj_st* obj, uint16_t angle);
/**
 * @brief Set the arc pointer active start and end angle
 *
 * @param obj
 * @param start_angle
 * @param end_angle
 */
void gt_arc_set_act_start_end_angle(gt_obj_st* obj, uint16_t start_angle, uint16_t end_angle);
/**
 * @brief Set the arc pointer active width
 *
 * @param obj
 * @param width
 */
void gt_arc_set_act_width(gt_obj_st* obj, uint16_t width);

/**
 * @brief Set the arc pointer value range
 *
 * @param obj
 * @param min
 * @param max
 */
void gt_arc_set_value_min_max(gt_obj_st* obj, int16_t min, int16_t max);
/**
 * @brief Set the arc pointer value
 *
 * @param obj
 * @param value
 */
void gt_arc_set_value(gt_obj_st* obj, int16_t value);
/**
 * @brief Set the arc pointer step
 *
 * @param obj
 * @param step
 */
void gt_arc_set_value_step(gt_obj_st* obj, int16_t step);
/**
 * @brief Set the arc pointer to increase @ step
 *
 * @param obj
 */
void gt_arc_set_value_increase(gt_obj_st* obj);
/**
 * @brief Set the arc pointer to decrease @ step
 *
 * @param obj
 */
void gt_arc_set_value_decrease(gt_obj_st* obj);

/**
 * @brief Get the arc pointer value
 *
 * @param obj
 * @return int16_t
 */
int16_t gt_arc_get_value(gt_obj_st* obj);


/**
 * @brief Set the arc pointer knob color
 *
 * @param obj
 * @param color
 */
void gt_arc_set_knob_color(gt_obj_st* obj, gt_color_t color);

/**
 * @brief Set the arc pointer knob width
 *
 * @param obj
 * @param width
 */
void gt_arc_set_knob_width(gt_obj_st* obj, uint16_t width);

/**
 * @brief Set the arc pointer touch disable
 *
 * @param obj
 * @param disable true: disable touch, false: enable touch
 */
void gt_arc_set_touch_disable(gt_obj_st* obj, bool disable);

#if GT_USE_GRADIENT
/**
 * @brief Set angular gradient for the active arc.
 *        Color varies along the arc angle from start to end.
 *        Ownership of @p grad is transferred to the widget; it will be
 *        destroyed automatically when the widget is deleted or replaced.
 *        Pass NULL to remove the gradient and revert to solid act_color.
 *
 * @param obj   Arc widget
 * @param grad  Gradient descriptor created by gt_gradient_dsc_create() /
 *              gt_gradient_dsc_rainbow(), or NULL to clear.
 */
void gt_arc_set_act_gradient(gt_obj_st * obj, gt_gradient_dsc_st * grad);

/**
 * @brief Convenience: set a rainbow gradient on the active arc.
 *
 * @param obj           Arc widget
 * @param steps_per_seg Steps per color segment (>= 2, typical 20~60)
 */
void gt_arc_set_act_gradient_rainbow(gt_obj_st * obj, uint16_t steps_per_seg);

/**
 * @brief Set fixed gradient mode. When enabled, the gradient color palette
 *        is anchored to the background arc's full angle range and stays
 *        stationary. Only the active arc length changes — like a progress
 *        bar revealing a fixed gradient. When disabled (default), the
 *        gradient stretches to fit the current active arc span.
 *
 * @param obj   Arc widget
 * @param fixed true = gradient stays fixed; false = stretch to arc span
 */
void gt_arc_set_act_gradient_fixed(gt_obj_st * obj, bool fixed);
#endif /* GT_USE_GRADIENT */

/**
 * @brief Set inner/outer edge fade width for the active arc.
 *
 * @param obj          Arc widget
 * @param inner_width  Fade width at inner edge in pixels (0 = no fade)
 * @param outer_width  Fade width at outer edge in pixels (0 = no fade)
 */
void gt_arc_set_act_fade(gt_obj_st * obj, uint8_t inner_width, uint8_t outer_width);

/**
 * @brief Set indicator dot at the active arc end point.
 *        The dot is drawn at the midpoint of the arc ring, colored by knob_color.
 *        Call gt_arc_set_knob_color() to set the dot color.
 *
 * @param obj    Arc widget
 * @param width  Dot diameter in pixels (0 = disabled)
 */
void gt_arc_set_knob_dot(gt_obj_st * obj, uint8_t width);

/**
 * @brief Enable or disable background arc drawing.
 *        When disabled, only the active arc and knob are drawn,
 *        making the widget visually transparent (shows whatever is beneath).
 *
 * @param obj      Arc widget
 * @param draw_bg  true = draw background arc (default), false = skip background
 */
void gt_arc_set_draw_bg(gt_obj_st * obj, bool draw_bg);

#endif /*GT_CFG_ENABLE_ARC*/
//
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_ARC_H_

/* end of file ----------------------------------------------------------*/


