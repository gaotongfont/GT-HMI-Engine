/**
 * @file gt_conf.h
 * @author Feyoung
 * @brief 
 * @version 0.1
 * @date 2022-05-11 14:43:24
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_CONF_H_
#define _GT_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/






/* define ---------------------------------------------------------------*/

/* obj contain user_data: 1 yes, 0 no */
#define GT_USE_USER_DATA    1

/* color depth: 1(1 byte per pixel), 8(RGB332), 16(RGB565), 32(ARGB8888) */
#define GT_COLOR_DEPTH      16
#define GT_COLOR_16_SWAP    0

/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_CONF_H_
