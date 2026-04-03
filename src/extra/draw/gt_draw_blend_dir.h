/**
 * @file gt_draw_blend_dir.h
 * @author Feyoung
 * @brief Runtime draw direction control: switch between horizontal and vertical
 *        flush layout without recompiling.
 *        Replaces compile-time GT_FLUSH_CONVERT_VERTICAL with a runtime flag
 *        and a function-pointer dispatch table.
 * @version 0.1
 * @date 2026-04-02
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_DRAW_BLEND_DIR_H_
#define _GT_DRAW_BLEND_DIR_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../../gt_conf.h"

/* global functions / API interface -------------------------------------*/

/**
 * @brief Query current flush direction.
 * @return 1 if vertical, 0 if horizontal.
 */
int gt_draw_blend_is_vertical(void);

/**
 * @brief Switch the runtime flush direction and update function pointers.
 *        Safe to call at any time between frames.
 * @param vertical  1 = vertical layout, 0 = horizontal layout
 */
void gt_draw_blend_set_vertical(int vertical);

/**
 * @brief Initialize the dispatch table from the compile-time default
 *        (GT_FLUSH_CONVERT_VERTICAL). Called once during engine init.
 */
void gt_draw_blend_dir_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* _GT_DRAW_BLEND_DIR_H_ */
