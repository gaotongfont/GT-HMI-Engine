/**
 * @file gt_txt.h
 * @author yongg
 * @brief 
 * @version 0.1
 * @date 2022-08-26 14:56:04
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_TXT_H_
#define _GT_TXT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "string.h"
#include "stdint.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
void gt_txt_ins(char * dst, uint32_t pos, char * src);
void gt_txt_cut(char * dst, uint32_t pos_start, uint32_t pos_end);


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_TXT_H_
