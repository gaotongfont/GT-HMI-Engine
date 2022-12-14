/**
 * @file gt_qrcode.h
 * @author yongg
 * @brief The QRCode implementation
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
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_QRCODE == 1
/* define ---------------------------------------------------------------*/

/* typedef --------------------------------------------------------------*/
enum qrcode_version_e
{
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
};

typedef struct _gt_style_qrcode_s
{
    uint8_t * str;
    uint8_t ec_level;
    uint8_t mask_patt;
    gt_family_t version;
    uint16_t str_len;
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

void gt_qrcode_style_set_version(gt_obj_st * qr_code , gt_family_t family);
void gt_qrcode_style_set_str(gt_obj_st * qr_code , char* str);




#endif
/* */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_QRCODE_H_
