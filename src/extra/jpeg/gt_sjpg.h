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


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
typedef struct gt_jpeg_raw_s {
    /**
     * only rgb pixel data, length = width * height
     * [WARN: Free must by gt_jpeg_free()]
     */
    gt_color_t * buffer;
    uint32_t width;
    uint32_t height;

    gt_fs_file_mem_cb_st mem_ctrl;
}gt_jpeg_raw_st;


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
 * @return gt_jpeg_raw_st [WARN] Free must used @ref gt_jpeg_free()
 */
gt_jpeg_raw_st gt_jpeg_decode(gt_fs_file_st const * const jpeg);

gt_res_t gt_jpeg_free(gt_jpeg_raw_st * jpeg_raw);

#endif /** GT_USE_SJPG */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_SJPG_H_
