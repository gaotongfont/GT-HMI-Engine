/**
 * @file gt_vf.h
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-07-05 19:15:04
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_VF_H_
#define _GT_VF_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "stdint.h"
#include "../others/gt_types.h"

/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
gt_res_t gt_vf_read_img(const char * path, uint8_t * res);
gt_res_t gt_vf_read_img_offset(const char * path, uint8_t * res, uint32_t offset, uint32_t len);
gt_res_t gt_vf_read_img_wh(const char * path, uint16_t * w, uint16_t * h);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_VF_H_
