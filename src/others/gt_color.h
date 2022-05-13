/**
 * @file gt_color.h
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-05-13 10:13:45
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_COLOR_H_
#define _GT_COLOR_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"
#include "../gt_conf.h"
#include "gt_types.h"

/* define ---------------------------------------------------------------*/

#if GT_COLOR_DEPTH == 1
#define GT_COLOR_SIZE 8
#elif GT_COLOR_DEPTH == 8
#define GT_COLOR_SIZE 8
#elif GT_COLOR_DEPTH == 16
#define GT_COLOR_SIZE 16
#elif GT_COLOR_DEPTH == 32
#define GT_COLOR_SIZE 32
#else
#error "Invalid GT_COLOR_DEPTH in lv_conf.h! Set it to 1, 8, 16 or 32!"
#endif

/* typedef --------------------------------------------------------------*/
typedef union {
    uint8_t full; /*must be declared first to set all bits of byte via initializer list*/
    union {
        uint8_t blue : 1;
        uint8_t green : 1;
        uint8_t red : 1;
    } ch;
} gt_color1_t;

typedef union {
    struct {
        uint8_t blue : 2;
        uint8_t green : 3;
        uint8_t red : 3;
    } ch;
    uint8_t full;
} gt_color8_t;

typedef union {
    struct {
#if GT_COLOR_16_SWAP == 0
        uint16_t blue : 5;
        uint16_t green : 6;
        uint16_t red : 5;
#else
        uint16_t green_h : 3;
        uint16_t red : 5;
        uint16_t blue : 5;
        uint16_t green_l : 3;
#endif
    } ch;
    uint16_t full;
} gt_color16_t;

typedef union {
    struct {
        uint8_t blue;
        uint8_t green;
        uint8_t red;
        uint8_t alpha;
    } ch;
    uint32_t full;
} gt_color32_t;

typedef GT_CONCAT3(uint, GT_COLOR_SIZE, _t) gt_color_int_t;
typedef GT_CONCAT3(gt_color, GT_COLOR_DEPTH, _t) gt_color_t;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_COLOR_H_
