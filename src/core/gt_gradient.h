/**
 * @file gt_gradient.h
 * @brief Gradient and shadow descriptor for GT-HMI-Engine widgets.
 *        Supports horizontal, vertical, diagonal, and radial gradients
 *        with multi-stop color (dual / multi / rainbow), plus a lightweight
 *        3-D shadow effect (offset solid rect + opacity).
 * @version 0.1
 * @date 2026-04-08
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_GRADIENT_H_
#define _GT_GRADIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "../gt_conf.h"
#include "../others/gt_color.h"
#include "../utils/gt_vector.h"

/* define ---------------------------------------------------------------*/
#ifndef GT_USE_GRADIENT
    /** @brief Enable gradient color support. 0 = disabled (saves code size). */
    #define GT_USE_GRADIENT     1
#endif

#ifndef GT_USE_SHADOW
    /** @brief Enable 3-D shadow support. 0 = disabled. */
    #define GT_USE_SHADOW       1
#endif

/* typedef --------------------------------------------------------------*/

#if GT_USE_GRADIENT

/**
 * @brief Gradient direction.
 */
typedef enum gt_grad_dir_e {
    GT_GRAD_DIR_HOR      = 0,   ///< Left → Right
    GT_GRAD_DIR_VER,             ///< Top  → Bottom
    GT_GRAD_DIR_DIAG_TL,         ///< Top-Left → Bottom-Right
    GT_GRAD_DIR_DIAG_TR,         ///< Top-Right → Bottom-Left
    GT_GRAD_DIR_SYM_VER,         ///< Center → Top & Bottom (symmetric vertical)
    GT_GRAD_DIR_SYM_HOR,         ///< Center → Left & Right (symmetric horizontal)
} gt_grad_dir_et;

/**
 * @brief Gradient descriptor.
 *        Stores an ordered list of gt_gradient_color_st segments in a vector.
 *        When loop == 1 the color sequence wraps around (rainbow effect).
 */
typedef struct gt_gradient_dsc_s {
    _gt_vector_st * stops;   ///< Vector of gt_gradient_color_st segments
    gt_grad_dir_et  dir;     ///< Gradient direction
    uint8_t loop : 1;        ///< 1 = loop / wrap (rainbow), 0 = clamp
} gt_gradient_dsc_st;

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a gradient descriptor.
 * @param dir  Gradient direction @ref gt_grad_dir_et
 * @return Heap-allocated descriptor, or NULL on OOM.
 */
gt_gradient_dsc_st * gt_gradient_dsc_create(gt_grad_dir_et dir);

/**
 * @brief Free a gradient descriptor and all its color stops.
 * @param dsc  Descriptor returned by gt_gradient_dsc_create()
 */
void gt_gradient_dsc_destroy(gt_gradient_dsc_st * dsc);

/**
 * @brief Append a two-color segment to the gradient.
 * @param dsc    Target descriptor
 * @param start  Start color of this segment
 * @param end    End color of this segment
 * @param steps  Number of interpolation steps (>= 2)
 */
void gt_gradient_dsc_add_stop(gt_gradient_dsc_st * dsc,
                               gt_color_t start, gt_color_t end,
                               uint16_t steps);

/**
 * @brief Enable looping (rainbow) mode.
 * @param dsc   Target descriptor
 * @param loop  1 = loop, 0 = clamp
 */
void gt_gradient_dsc_set_loop(gt_gradient_dsc_st * dsc, uint8_t loop);

/**
 * @brief Create a pre-built rainbow gradient (6-segment HSV cycle).
 * @param dir           Gradient direction
 * @param steps_per_seg Steps per color segment (>= 2, typical: 20~60)
 * @return Heap-allocated descriptor, or NULL on OOM.
 */
gt_gradient_dsc_st * gt_gradient_dsc_rainbow(gt_grad_dir_et dir,
                                              uint16_t steps_per_seg);

/**
 * @brief Get the total number of steps across all segments.
 * @param dsc  Descriptor
 * @return Total steps, 0 if empty.
 */
uint16_t gt_gradient_dsc_total_steps(const gt_gradient_dsc_st * dsc);

/**
 * @brief Sample the gradient color at a given step.
 *        Handles loop/clamp automatically.
 * @param dsc   Descriptor
 * @param step  Step index (0 .. total_steps-1)
 * @return Interpolated color.
 */
gt_color_t gt_gradient_dsc_get_color(const gt_gradient_dsc_st * dsc,
                                      uint16_t step);

/**
 * @brief Sample the gradient color at a continuous position [0..65535],
 *        returning a 24-bit packed RGB value (0x00RRGGBB).
 *        Interpolation is performed in 8-bit-per-channel space to avoid
 *        RGB565 quantization errors during blending.  Use this variant
 *        when the caller will apply its own dithering (e.g. Floyd-Steinberg).
 * @param dsc  Descriptor
 * @param pos  Position in [0..65535]
 * @return 24-bit color 0x00RRGGBB, each channel 0..255.
 */
uint32_t gt_gradient_dsc_get_color24_pos(const gt_gradient_dsc_st * dsc,
                                          uint16_t pos);

/**
 * @brief Sample the gradient color at a continuous position [0..65535].
 *        pos=0 → gradient start, pos=65535 → gradient end.
 *        Provides sub-step precision: the color is interpolated directly
 *        between the two nearest segment endpoints, eliminating banding
 *        when the widget size differs from the configured step count.
 * @param dsc  Descriptor
 * @param pos  Position in [0..65535] (fixed-point 16-bit fraction of full range)
 * @return Smoothly interpolated color.
 */
gt_color_t gt_gradient_dsc_get_color_pos(const gt_gradient_dsc_st * dsc,
                                          uint16_t pos);

/**
 * @brief Sample the gradient color with Bayer ordered dithering.
 *        Applies a ±1 LSB nudge based on a 4-entry Bayer pattern indexed
 *        by @p dither_idx (0..3), breaking up visible color bands on
 *        RGB565 displays without per-pixel overhead.
 * @param dsc        Descriptor
 * @param pos        Position in [0..65535]
 * @param dither_idx Row or column index mod 4 (0..3)
 * @return Dithered interpolated color.
 */
gt_color_t gt_gradient_dsc_get_color_pos_dither(const gt_gradient_dsc_st * dsc,
                                                 uint16_t pos,
                                                 uint8_t  dither_idx);

#endif /* GT_USE_GRADIENT */

/* ------------------------------------------------------------------ */

#if GT_USE_SHADOW

/**
 * @brief Lightweight 3-D shadow descriptor.
 *        Renders as an offset solid rectangle drawn behind the widget.
 *        No blur — suitable for resource-constrained MCUs.
 * 原始控件区域 (coords):
 *  ┌─────────────────┐
 *  │                 │
 *  │   Widget        │
 *  │   (10,270)      │
 *  │   160×60        │
 *  └─────────────────┘
 *
 *  加上 offset_x=10, offset_y=10, spread=4 后的阴影区域:
 *           ┌─────────────────────┐
 *           │                     │
 *           │   Shadow            │
 *           │   (16,276)          │
 *           │   168×68            │
 *           └─────────────────────┘
 *
 *  叠加效果（阴影在下，控件在上）:
 *  ┌─────────────────┐
 *  │                 │
 *  │   Widget        │──┐
 *  │                 │  │ ← 右侧露出 14px 阴影
 *  └─────────────────┘  │
 *           └───────────┘ ← 下方露出 14px 阴影
 */
typedef struct gt_shadow_dsc_s {
    gt_color_t color;     ///< Shadow fill color
    int8_t     offset_x;  ///< Horizontal offset in pixels (positive = right)
    int8_t     offset_y;  ///< Vertical offset in pixels   (positive = down)
    uint8_t    spread;    ///< Extra expansion radius in pixels (0~16)
    gt_opa_t   opa;       ///< Shadow opacity (GT_OPA_0 ~ GT_OPA_COVER)
} gt_shadow_dsc_st;

/**
 * @brief Preset shadow styles for gt_shadow_dsc_create().
 *
 *  GT_SHADOW_STYLE_BOTTOM_RIGHT  offset(4,4)  spread=0  opa=50%  经典右下 3D 按钮
 *  GT_SHADOW_STYLE_BOTTOM        offset(0,6)  spread=2  opa=50%  卡片底部阴影
 *  GT_SHADOW_STYLE_AROUND        offset(0,0)  spread=6  opa=40%  四周发光 / 浮层
 *  GT_SHADOW_STYLE_DEEP          offset(6,8)  spread=4  opa=70%  深度感强
 *  GT_SHADOW_STYLE_SUBTLE        offset(2,2)  spread=4  opa=30%  微弱扁平阴影
 *  GT_SHADOW_STYLE_CUSTOM        offset(4,4)  spread=0  opa=50%  同 BOTTOM_RIGHT，供用户自行修改
 */
typedef enum gt_shadow_style_e {
    GT_SHADOW_STYLE_BOTTOM_RIGHT = 0,  ///< Classic 3-D button: offset(4,4), spread=0, opa=50%
    GT_SHADOW_STYLE_BOTTOM,            ///< Card bottom shadow: offset(0,6), spread=2, opa=50%
    GT_SHADOW_STYLE_AROUND,            ///< Glow / float layer: offset(0,0), spread=6, opa=40%
    GT_SHADOW_STYLE_DEEP,              ///< Strong depth: offset(6,8), spread=4, opa=70%
    GT_SHADOW_STYLE_SUBTLE,            ///< Flat subtle: offset(2,2), spread=4, opa=30%
    GT_SHADOW_STYLE_CUSTOM,            ///< Same as BOTTOM_RIGHT; caller modifies fields freely
} gt_shadow_style_et;

/* global functions / API interface -------------------------------------*/

/**
 * @brief Create a shadow descriptor from a preset style.
 *        All presets use dark-gray (0x404040) as the shadow color.
 *        Pass GT_SHADOW_STYLE_CUSTOM to get a default descriptor and
 *        then modify individual fields as needed.
 * @param style  Preset style @ref gt_shadow_style_et
 * @return Heap-allocated descriptor, or NULL on OOM.
 */
gt_shadow_dsc_st * gt_shadow_dsc_create(gt_shadow_style_et style);

/**
 * @brief Free a shadow descriptor.
 * @param dsc  Descriptor returned by gt_shadow_dsc_create()
 */
void gt_shadow_dsc_destroy(gt_shadow_dsc_st * dsc);

#endif /* GT_USE_SHADOW */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* _GT_GRADIENT_H_ */

/* end of file ----------------------------------------------------------*/
