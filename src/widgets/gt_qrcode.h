/**
 * @file gt_qrcode.h
 * @author yongg
 * @brief The QRCode implementation
 * @version 0.1
 * @date 2022-08-24 16:37:06
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_QRCODE_H_
#define _GT_QRCODE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_QRCODE
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "gt_conf_widgets.h"

/* define ---------------------------------------------------------------*/

/* typedef --------------------------------------------------------------*/
typedef enum qrcode_version_e {
    GT_FAMILY_QRCODE_VERSION_3 = 3,
    GT_FAMILY_QRCODE_VERSION_4,
    GT_FAMILY_QRCODE_VERSION_5,
    GT_FAMILY_QRCODE_VERSION_6,
    GT_FAMILY_QRCODE_VERSION_7,
    GT_FAMILY_QRCODE_VERSION_8,
    GT_FAMILY_QRCODE_VERSION_9,
    GT_FAMILY_QRCODE_VERSION_10,
    GT_FAMILY_QRCODE_VERSION_11,
    GT_FAMILY_QRCODE_VERSION_12,
    GT_FAMILY_QRCODE_VERSION_13,
    GT_FAMILY_QRCODE_VERSION_14,
    GT_FAMILY_QRCODE_VERSION_15,
    GT_FAMILY_QRCODE_VERSION_16,
    GT_FAMILY_QRCODE_VERSION_17
}gt_qr_code_version_em;



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief create QRCode obj
 *
 * @param parent QRCode's parent obj
 * @return gt_obj_st* QRCode obj ptr
 */
gt_obj_st * gt_qrcode_create(gt_obj_st * parent);

void gt_qrcode_set_version(gt_obj_st * qr_code , gt_qr_code_version_em version);
void gt_qrcode_set_str(gt_obj_st * qr_code , char* str);

void gt_qrcode_set_background(gt_obj_st * qr_code , gt_color_t color);
void gt_qrcode_set_forecolor(gt_obj_st * qr_code , gt_color_t color);


#endif  /** GT_CFG_ENABLE_QRCODE */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_QRCODE_H_
