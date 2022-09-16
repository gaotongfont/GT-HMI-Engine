/**
 * @file gt_qrcode.h
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-08-24 16:37:06
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_QRCODE_H_
#define _GT_QRCODE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_obj.h"
#include "gt_obj_class.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct _gt_style_qrcode_s
{
    char * value;
}_gt_style_qrcode_st;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief create qrcode obj
 * 
 * @param parent qrcode's parent obj
 * @return gt_obj_st* qrcode obj ptr
 */
gt_obj_st * gt_qrcode_create(gt_obj_st * parent);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_QRCODE_H_
