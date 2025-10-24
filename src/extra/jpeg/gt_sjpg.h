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
#include "../../core/gt_fs.h"
#include "../../others/gt_types.h"
#include "../../others/gt_color.h"
#include "../../core/gt_img_decoder.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief jpg / sjpg image decoder init, register to list.
 */
void gt_sjpg_init(void);

/**
 * @brief decode jpeg image to raw data
 *      [WARN: Only adjust rgb565 color depth now]
 *
 * @param jpeg
 * @return gt_img_raw_st [WARN] Free must used @ref gt_img_raw_free()
 */
gt_img_raw_st gt_jpeg_decode(gt_fs_file_st const * const jpeg);


#endif /** GT_USE_SJPG */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SJPG_H_
