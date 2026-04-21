/**
 * @file gt_svg.h
 * @author Feyoung
 * @brief SVG image decoder based on nanosvg (SVG Tiny 1.2)
 * @version 0.1
 * @date 2026-04-21
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_SVG_H_
#define _GT_SVG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../../gt_conf.h"

#if GT_USE_SVG
#include "../../core/gt_img_decoder.h"

/* global functions / API interface -------------------------------------*/

/** @brief SVG image decoder init, register to decoder list. */
void gt_svg_init(void);

#endif  /* GT_USE_SVG */

#ifdef __cplusplus
}
#endif

#endif  /* _GT_SVG_H_ */
