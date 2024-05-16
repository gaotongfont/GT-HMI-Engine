/**
 * @file gt_barcode.h
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2022-12-09 13:40:04
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_BARCODE_H_
#define _GT_BARCODE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_conf_widgets.h"

#if GT_CFG_ENABLE_BARCODE
#include "gt_obj.h"
#include "gt_obj_class.h"
#include "gt_conf_widgets.h"


#ifdef GT_CONFIG_GUI_DRIVER_LIB
    #include "../../driver/gt_gui_driver.h"
#endif /* GT_CONFIG_GUI_DRIVER_LIB */

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief create a barcode obj
 *
 * @param parent barcode's parent element
 * @return gt_obj_st* barcode obj
 */
gt_obj_st * gt_barcode_create(gt_obj_st * parent);

void gt_barcode_set_code_text(gt_obj_st * barcode , char* code);
void gt_barcode_set_type(gt_obj_st * barcode , gt_family_t family);
void gt_barcode_set_hri_type(gt_obj_st * barcode , gt_family_t family);
void gt_barcode_set_mode_w(gt_obj_st * barcode , uint8_t value);
void gt_barcode_set_mode_h(gt_obj_st * barcode , uint8_t value);
void gt_barcode_set_upc_e_sys_code(gt_obj_st * barcode , uint8_t value);


#endif  /** GT_CFG_ENABLE_BARCODE */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_BARCODE_H_

/* end of file ----------------------------------------------------------*/


