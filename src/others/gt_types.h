/**
 * @file gt_types.h
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-05-12 16:27:54
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_TYPES_H_
#define _GT_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"
#include "stdbool.h"

/* define ---------------------------------------------------------------*/

// If __UINTPTR_MAX__ or UINTPTR_MAX are available, use them to determine arch size
#if defined(__UINTPTR_MAX__) && __UINTPTR_MAX__ > 0xFFFFFFFF
#define GT_ARCH_64

#elif defined(UINTPTR_MAX) && UINTPTR_MAX > 0xFFFFFFFF
#define GT_ARCH_64

// Otherwise use compiler-dependent means to determine arch size
#elif defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined (__aarch64__)
#define GT_ARCH_64

#endif

/* typedef --------------------------------------------------------------*/
typedef void * style_widget_t;

typedef int16_t gt_size_t;

typedef enum type_widget_e{
    GT_TYPE_OBJ        = 0,      //base obj type
    GT_TYPE_BTN        = 1,      //button type
    GT_TYPE_IMG        = 2,      //img type
    GT_TYPE_LAB        = 3,      //label type
    GT_TYPE_TEXTAREA   = 4,      //textarea type
    GT_TYPE_CANVAS     = 5,      //canvas
    GT_TYPE_OPTION     = 6,      //option
    GT_TYPE_GROUP      = 7,      //group
    GT_TYPE_CHECKBOX   = 8,      //checkbox
    GT_TYPE_RADIO      = 9,      //radio box
    GT_TYPE_INPUT      = 10,     //input
    GT_TYPE_SWITCH     = 11,     //switch
    GT_TYPE_SCROLLBAR  = 12,     //scroll bar
    GT_TYPE_PROCESSBAR = 13,     //progress bar
    GT_TYPE_KEYPAD     = 14,     //keypad

    GT_TYPE_TOTAL,          //count total of type
}gt_obj_type_et;

typedef enum{
    GT_RES_FAIL = -1,
    GT_RES_OK   = 0,
    GT_RES_INV  = 1,
}gt_res_t;

typedef struct _gt_area_s {
    gt_size_t x;
    gt_size_t y;
    uint16_t w;
    uint16_t h;
}gt_area_st;


typedef uint8_t gt_opt_t;

/* macros ---------------------------------------------------------------*/

#define _GT_CONCAT3(x, y, z) x ## y ## z
#define GT_CONCAT3(x, y, z) _GT_CONCAT3(x, y, z)

#define _GT_CONCAT(x, y) x ## y
#define GT_CONCAT(x, y) _GT_CONCAT(x, y)

#define GT_UNUSED(_val) ((void)_val)

/* global functions / API interface -------------------------------------*/

static inline uint32_t gt_area_get_size(gt_area_st area){ return (area.w  * area.h); }

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_TYPES_H_
