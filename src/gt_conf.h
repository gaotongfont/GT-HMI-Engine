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

/* user: screen width and height config */
#define GT_SCREEN_WIDTH     320
#define GT_SCREEN_HEIGHT    240

/* user: obj contain user_data: 1 yes, 0 no */
#define GT_USE_USER_DATA    1

/* user: color depth: 1(1 byte per pixel), 8(RGB332), 16(RGB565), 32(ARGB8888) */
#define GT_COLOR_DEPTH      16
#define GT_COLOR_16_SWAP    0

/**
 * @brief memory mode. 1: array as memory pool; 0:  c library api.
 */
#define GT_MEM_CUSTOM       0
#if GT_MEM_CUSTOM
    #define GT_MEM_CUSTOM_INCLUDE   "../others/gt_tlsf.h"

    #define GT_MEM_SIZE             (24 * 1024U)    //Byte
#else
    #define GT_MEM_CUSTOM_INCLUDE   "stdlib.h"

    #define GT_MEM_CUSTOM_MALLOC    malloc
    #define GT_MEM_CUSTOM_CALLOC    calloc
    #define GT_MEM_CUSTOM_REALLOC   realloc
    #define GT_MEM_CUSTOM_FREE      free
#endif

/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_CONF_H_
