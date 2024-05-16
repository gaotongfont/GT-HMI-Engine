/**
 * @file gt_conf_widgets.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2022-05-11 16:18:14
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_CONF_WIDGETS_H_
#define _GT_CONF_WIDGETS_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../gt_conf.h"


/* define ---------------------------------------------------------------*/

/**
 * @brief Configure whether the widgets is enabled
 *  such as: gt_template.c/h
 */
#define GT_CFG_ENABLE_TEMPLATE          0

#define GT_CFG_ENABLE_BTN               01
#define GT_CFG_ENABLE_CHECKBOX          01
#define GT_CFG_ENABLE_GROUP             01
#define GT_CFG_ENABLE_IMG               01
#define GT_CFG_ENABLE_INPUT             01
/**the keypad must exist with input widget*/
#define GT_CFG_ENABLE_KEYPAD            01
#define GT_CFG_ENABLE_LABEL             01
#define GT_CFG_ENABLE_LISTVIEW          01
#define GT_CFG_ENABLE_PROGRESS_BAR      01
#define GT_CFG_ENABLE_RADIO             01
#define GT_CFG_ENABLE_SLIDER            01
#define GT_CFG_ENABLE_SWITCH            01
#define GT_CFG_ENABLE_TEXTAREA          01
#define GT_CFG_ENABLE_IMGBTN            01
#define GT_CFG_ENABLE_RECT              01
#define GT_CFG_ENABLE_BARCODE           01
#define GT_CFG_ENABLE_QRCODE            01
#define GT_CFG_ENABLE_ZK_FONT           01
#define GT_CFG_ENABLE_ZK_SPELL          01
#define GT_CFG_ENABLE_LINE              01
#define GT_CFG_ENABLE_PLAYER            01
#define GT_CFG_ENABLE_INPUT_NUMBER      01
#define GT_CFG_ENABLE_CLOCK             01
#define GT_CFG_ENABLE_WORDART           01
#define GT_CFG_ENABLE_VIEW_PAGER        01
#define GT_CFG_ENABLE_ROLLER            01
#define GT_CFG_ENABLE_BTNMAP            01
#define GT_CFG_ENABLE_DIALOG            01
#define GT_CFG_ENABLE_CHAT              01
#define GT_CFG_ENABLE_STATUS_BAR        01

#if GT_USE_GIF
#define GT_CFG_ENABLE_GIF               01
#endif

/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_CONF_WIDGETS_H_
