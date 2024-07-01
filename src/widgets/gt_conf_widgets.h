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

#define GT_CFG_ENABLE_BTN               1
#define GT_CFG_ENABLE_CHECKBOX          1
#define GT_CFG_ENABLE_GROUP             1
#define GT_CFG_ENABLE_IMG               1
#define GT_CFG_ENABLE_INPUT             1
/**the keypad must exist with input widget*/
#define GT_CFG_ENABLE_KEYPAD            1
#define GT_CFG_ENABLE_LABEL             1
#define GT_CFG_ENABLE_LISTVIEW          1
#define GT_CFG_ENABLE_PROGRESS_BAR      1
#define GT_CFG_ENABLE_RADIO             1
#define GT_CFG_ENABLE_SLIDER            1
#define GT_CFG_ENABLE_SWITCH            1
#define GT_CFG_ENABLE_TEXTAREA          1
#define GT_CFG_ENABLE_IMGBTN            1
#define GT_CFG_ENABLE_RECT              1
#define GT_CFG_ENABLE_BARCODE           1
#define GT_CFG_ENABLE_QRCODE            1
#define GT_CFG_ENABLE_ZK_FONT           1
#define GT_CFG_ENABLE_ZK_SPELL          1
#define GT_CFG_ENABLE_LINE              1
#define GT_CFG_ENABLE_PLAYER            1
#define GT_CFG_ENABLE_INPUT_NUMBER      1
#define GT_CFG_ENABLE_CLOCK             1
#define GT_CFG_ENABLE_WORDART           1
#define GT_CFG_ENABLE_VIEW_PAGER        1
#define GT_CFG_ENABLE_ROLLER            1
#define GT_CFG_ENABLE_BTNMAP            1
#define GT_CFG_ENABLE_CHAT              1

#if GT_USE_LAYER_TOP
#define GT_CFG_ENABLE_DIALOG            1
#define GT_CFG_ENABLE_STATUS_BAR        1
#endif

#if GT_USE_GIF
#define GT_CFG_ENABLE_GIF               1
#endif

/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_CONF_WIDGETS_H_
