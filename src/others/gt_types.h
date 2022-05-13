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
#include "stdio.h"
#include "gt_log.h"

/* define ---------------------------------------------------------------*/
#define _GT_CONCAT3(x, y, z) x ## y ## z
#define GT_CONCAT3(x, y, z) _GT_CONCAT3(x, y, z)


/* typedef --------------------------------------------------------------*/
typedef void * style_widget_t;
typedef enum type_widget_e{
    GT_TYPE_OBJ = 0,        //base obj type
    GT_TYPE_BTN = 1,        //button type
    GT_TYPE_LAB = 2,        //label type
    GT_TYPE_TEXTAREA = 3,   //textarea type
    

    GT_TYPE_TOTAL,          //count total of type
}gt_obj_type_et;

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_TYPES_H_
