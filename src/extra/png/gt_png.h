/**
 * @file gt_png.h
 * @author Feyoung
 * @brief png implementation
 * @version 0.1
 * @date 2022-08-10 15:48:21
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_PNG_H_
#define _GT_PNG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../../gt_conf.h"

#if GT_USE_PNG
#include "../../core/gt_fs.h"
#include "../../core/gt_img_decoder.h"
#include "../../others/gt_color.h"

/* define ---------------------------------------------------------------*/
#define LODEPNG_NO_COMPILE_ENCODER
// #define LODEPNG_NO_COMPILE_ERROR_TEXT

/* typedef --------------------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

/**
 * @brief PNG image decoder init, register to list.
 */
void gt_png_init(void);

/**
 * @brief decode png image to raw data and alpha data
 *
 * @param png
 *         png.mem_ctrl unused
 * @return gt_png_raw_st [WARN] Free must used @ref gt_img_raw_free()
 */
gt_img_raw_st gt_png_decode(gt_fs_file_st const * const png);


#endif   /* GT_USE_PNG */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_PNG_H_
