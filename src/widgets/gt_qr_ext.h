/**
 * @file gt_qr_ext.h
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2026-01-05 11:09:04
 * @copyright Copyright (c) 2014-2026, Company Genitop. Co., Ltd.
 */
#ifndef _GT_QR_EXT_H_
#define _GT_QR_EXT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_QR_EXT
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "stdarg.h"
#include "stdlib.h"
#include "../utils/gt_vector.h"
#include "../core/gt_style.h"

/* define ---------------------------------------------------------------*/
typedef enum {
    GT_QR_ECC_LEVEL_LOW = 0,
    GT_QR_ECC_LEVEL_MEDIUM,
    GT_QR_ECC_LEVEL_QUARTILE,
    GT_QR_ECC_LEVEL_HIGH,
}gt_qr_ecc_level_te;

typedef enum {
    GT_QR_MASK_AUTO = -1,
    GT_QR_MASK_0 = 0,
    GT_QR_MASK_1,
    GT_QR_MASK_2,
    GT_QR_MASK_3,
    GT_QR_MASK_4,
    GT_QR_MASK_5,
    GT_QR_MASK_6,
    GT_QR_MASK_7,
}gt_qr_mask_te;


/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_obj_st * gt_qr_ext_create(gt_obj_st * parent);

/**
 * @brief 设置二维码内容 set qr content
 *
 * @param obj
 * @param text
 */
void gt_qr_ext_set_text(gt_obj_st * obj, const char * text);
void gt_qr_ext_set_text_by_len(gt_obj_st * obj, const char * text, int len);

/**
 * @brief 设置二维码块大小,默认[4px] set qr module size, default[4px]
 *
 * @param obj
 * @param size
 */
void gt_qr_ext_set_module_size(gt_obj_st * obj, uint8_t size);
/**
 * @brief 设置二维码块颜色,默认[黑色] set qr module color,default[black]
 *
 * @param obj
 * @param color
 */
void gt_qr_ext_set_module_color(gt_obj_st * obj, gt_color_t color);
/**
 * @brief 设置二维码块边距(最小边距),默认[2px], set qr module margin (min margin), default[2px]
 *
 * @param obj
 * @param margin
 */
void gt_qr_ext_set_margin(gt_obj_st * obj, uint8_t margin);
/**
 * @brief 设置二维码版本，默认[40], 范围[2-40] set qr version, default[40], range[2-40]
 *
 * @param obj
 * @param version
 */
void gt_qr_ext_set_version(gt_obj_st * obj, uint8_t version);
/**
 * @brief 设置二维码纠错等级 set qr ecc level
 *
 * @param obj
 * @param ecc_level @ref gt_qr_ecc_level_te default[GT_QR_ECC_LEVEL_HIGH]
 */
void gt_qr_ext_set_ecc_level(gt_obj_st * obj, gt_qr_ecc_level_te ecc_level);
/**
 * @brief 设置二维码掩码 set qr mask
 *
 * @param obj
 * @param mask @ref gt_qr_mask_te default[GT_QR_MASK_0]
 */
void gt_qr_ext_set_mask(gt_obj_st * obj, gt_qr_mask_te mask);


#endif //!GT_CFG_ENABLE_QR_EXT
//
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_QR_EXT_H_

/* end of file ----------------------------------------------------------*/


