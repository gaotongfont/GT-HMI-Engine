/**
 * @file gt_sjpg.h
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2023-10-23 15:47:07
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SJPG_H_
#define _GT_SJPG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../../gt_conf.h"

#if GT_USE_SJPG
#include "../../others/gt_types.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief jpg / sjpg image decoder init, register to list.
 */
void gt_sjpg_init(void);


#endif /** GT_USE_SJPG */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SJPG_H_
