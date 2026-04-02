/**
 * @file gt_effect_click.h
 * @author Feyoung
 * @brief Generic click feedback effect (opacity dim/restore).
 *        Register on any widget to get visual press feedback.
 *        Zero static RAM — effect data malloc'd per widget, freed on delete.
 * @version 0.2
 * @date 2026-04-02
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_EFFECT_CLICK_H_
#define _GT_EFFECT_CLICK_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../../gt_conf.h"

#if GT_USE_CLICK_EFFECT

#include "../../widgets/gt_obj.h"
#include "../../others/gt_anim.h"

/* define ---------------------------------------------------------------*/
/** Pressed opacity (0~255), default ~70% */
#define GT_EFFECT_CLICK_OPA_DEFAULT     178
/** Restore animation duration [ms] */
#define GT_EFFECT_CLICK_TIME_DEFAULT    120

/* typedef --------------------------------------------------------------*/
/** Opaque handle returned by gt_effect_click_register */
typedef struct _gt_effect_click_s gt_effect_click_st;

/* global functions / API interface -------------------------------------*/

/**
 * @brief Register click opacity feedback on a widget.
 *        Press: instant dim. Release: animated restore.
 *        The returned handle can be used to tune parameters.
 *        Resources are freed automatically when the widget is deleted.
 *
 * @param obj Target widget
 * @return gt_effect_click_st* Handle on success, NULL on failure
 */
gt_effect_click_st * gt_effect_click_register(gt_obj_st * obj);

/**
 * @brief Set pressed opacity value (0~255).
 *
 * @param eff Handle returned by gt_effect_click_register
 * @param opa Pressed opacity [default: GT_EFFECT_CLICK_OPA_DEFAULT]
 */
void gt_effect_click_set_opa(gt_effect_click_st * eff, uint8_t opa);

/**
 * @brief Set restore animation duration in milliseconds.
 *
 * @param eff Handle returned by gt_effect_click_register
 * @param time_ms Duration [default: GT_EFFECT_CLICK_TIME_DEFAULT]
 */
void gt_effect_click_set_time(gt_effect_click_st * eff, uint16_t time_ms);

/**
 * @brief Set animation easing path type.
 *
 * @param eff Handle returned by gt_effect_click_register
 * @param type Path type @ref gt_anim_path_type_em
 */
void gt_effect_click_set_path_type(gt_effect_click_st * eff,
                                   gt_anim_path_type_em type);

#endif /* GT_USE_CLICK_EFFECT */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* _GT_EFFECT_CLICK_H_ */
