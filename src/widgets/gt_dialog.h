/**
 * @file gt_dialog.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-03-04 15:53:41
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_DIALOG_H_
#define _GT_DIALOG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_DIALOG
#include "./gt_obj.h"
#include "../core/gt_style.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct gt_dialog_param_s {
    gt_event_cb_t cancel_cb;    /** cancel button callback */
    void * cancel_cb_user_data; /** cancel button callback parameter */

    gt_event_cb_t confirm_cb;   /** confirm button callback */
    void * confirm_cb_user_data;/** confirm button callback parameter */

    char * title;               /** title */
    char * content;             /** content */
}gt_dialog_param_st;


/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a Empty dialog
 *
 * @param show_close_btn true: show close button, false: hide close button
 * @return gt_obj_st* The dialog object
 */
gt_obj_st * gt_dialog_create(bool show_close_btn);

/**
 * @brief Create a message box dialog
 *
 * @param show_close_btn true: show close button, false: hide close button
 * @param param message box dialog parameter @ref gt_dialog_param_st
 * @return gt_obj_st* The dialog object
 */
gt_obj_st * gt_dialog_create_issue(bool show_close_btn, gt_dialog_param_st const * const param);

/**
 * @brief Begin to show the dialog
 *
 * @param obj The dialog object
 */
void gt_dialog_show(gt_obj_st * obj);

bool gt_dialog_has_showing(void);

/**
 * @brief Set dialog border color
 *
 * @param dialog
 * @param color
 */
void gt_dialog_set_border_color(gt_obj_st * dialog, gt_color_t color);

/**
 * @brief Set dialog border line width
 *
 * @param dialog
 * @param width default: 2
 */
void gt_dialog_set_border_width(gt_obj_st * dialog, uint8_t width);

/**
 * @brief Set dialog background color
 *
 * @param dialog
 * @param color
 */
void gt_dialog_set_bgcolor(gt_obj_st * dialog, gt_color_t color);

/**
 * @brief Set dialog border radius
 *
 * @param dialog
 * @param radius default: 10
 */
void gt_dialog_set_border_radius(gt_obj_st * dialog, uint8_t radius);

/**
 * @brief Click the area outside the control to close the dialog box
 *
 * @param dialog
 * @param auto_hide true[default]: auto hide, false: not auto hide
 */
void gt_dialog_set_outside_auto_hide(gt_obj_st * dialog, bool auto_hide);

/**
 * @brief Set dialog animation time
 *
 * @param dialog
 * @param time default: 250[ms]
 */
void gt_dialog_set_anim_time(gt_obj_st * dialog, uint32_t time);

/* ------------------- Issue mode api ------------------- */

void gt_dialog_set_title_font_color(gt_obj_st * dialog, gt_color_t color);
void gt_dialog_set_title_font_size(gt_obj_st * dialog, uint8_t size);
void gt_dialog_set_title_font_align(gt_obj_st * dialog, gt_align_et align);
void gt_dialog_set_title_font_family_cn(gt_obj_st * dialog, gt_family_t family);
void gt_dialog_set_title_font_family_en(gt_obj_st * dialog, gt_family_t family);
void gt_dialog_set_title_font_family_fl(gt_obj_st * dialog, gt_family_t family);
void gt_dialog_set_title_font_thick_en(gt_obj_st * dialog, uint8_t thick);
void gt_dialog_set_title_font_thick_cn(gt_obj_st * dialog, uint8_t thick);

void gt_dialog_set_content_font_color(gt_obj_st * dialog, gt_color_t color);
void gt_dialog_set_content_font_size(gt_obj_st * dialog, uint8_t size);
void gt_dialog_set_content_font_align(gt_obj_st * dialog, gt_align_et align);
void gt_dialog_set_content_font_family_cn(gt_obj_st * dialog, gt_family_t family);
void gt_dialog_set_content_font_family_en(gt_obj_st * dialog, gt_family_t family);
void gt_dialog_set_content_font_family_fl(gt_obj_st * dialog, gt_family_t family);
void gt_dialog_set_content_font_thick_en(gt_obj_st * dialog, uint8_t thick);
void gt_dialog_set_content_font_thick_cn(gt_obj_st * dialog, uint8_t thick);


#endif  /** GT_CFG_ENABLE_DIALOG */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_DIALOG_H_
