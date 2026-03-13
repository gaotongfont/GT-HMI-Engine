/**
 * @file gt_img_transform.h
 * @brief Image rotation and scaling for embedded MCU (fixed-point, no FPU required)
 *
 * Angle unit: 0.1 degree (0~3599), e.g. 900 = 90 degrees
 * Scale unit: 256 = 1.0x, 128 = 0.5x, 512 = 2.0x
 */
#ifndef _GT_IMG_TRANSFORM_H_
#define _GT_IMG_TRANSFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "../../gt_conf.h"

#if GT_USE_IMG_TRANSFORM

#include "../../core/gt_graph_base.h"
#include "../../core/gt_img_decoder.h"
#include "gt_draw_blend.h"

/* Forward declaration to avoid circular include with gt_draw.h */
struct _gt_draw_ctx_s;

/* Angle: 0~3599 (unit: 0.1 degree) */
typedef uint16_t gt_angle_t;

/* Scale: 256 = 1.0x */
typedef uint16_t gt_zoom_t;

#define GT_ZOOM_NONE    256
#define GT_ANGLE_NONE   0

/**
 * @brief Draw image with rotation and/or scaling
 *
 * @param draw_ctx  Drawing context
 * @param dsc_img   Opened image descriptor (header must be valid)
 * @param blend_dsc Blend descriptor (dst_area = pivot position on screen)
 * @param angle     Rotation angle, unit 0.1 degree (0~3599)
 * @param zoom      Scale factor, 256 = 1.0x
 * @param pivot_x   Pivot x relative to image top-left (pixels)
 * @param pivot_y   Pivot y relative to image top-left (pixels)
 */
void gt_img_transform_draw(struct _gt_draw_ctx_s * draw_ctx,
                           gt_draw_blend_dsc_st * blend_dsc,
                           _gt_img_dsc_st * dsc_img,
                           gt_angle_t angle,
                           gt_zoom_t zoom,
                           int16_t pivot_x,
                           int16_t pivot_y);

/**
 * @brief Decode image source to a flat RGB pixel cache (call once, reuse for anim).
 *        If the source is already a RAM/ROM RGB565 buffer, returns a direct pointer
 *        (no malloc) and sets *out_is_rom = true — do NOT gt_mem_free() it.
 *        Otherwise allocates a new buffer — caller must gt_mem_free() when done.
 *
 * @param dsc        Image attribute descriptor
 * @param out_w      Output: image width in pixels
 * @param out_h      Output: image height in pixels
 * @param out_is_rom Output: true = returned pointer must NOT be freed
 * @return gt_color_t* Pixel buffer, or NULL on failure
 */
gt_color_t * gt_img_decode_to_cache(const gt_attr_rect_st * dsc,
                                    uint16_t * out_w, uint16_t * out_h,
                                    bool * out_is_rom);

/**
 * @brief Draw a pre-decoded pixel cache with rotation and/or scaling.
 *        No image decoding happens — use this in anim callbacks.
 *
 * @param draw_ctx  Drawing context
 * @param coords    Widget area on screen (position + output size)
 * @param opa       Global opacity
 * @param src_buf   Decoded pixel buffer from gt_img_decode_to_cache()
 * @param src_w     Source image width
 * @param src_h     Source image height
 * @param angle     Rotation angle, unit 0.1 degree (0~3599)
 * @param zoom      Scale: 256 = 1.0x
 * @param pivot_x   Pivot x relative to image
 * @param pivot_y   Pivot y relative to image
 */
/**
 * @param use_nn  true = nearest-neighbor (fast, for animation),
 *                false = bilinear (smooth, for static display)
 */
void gt_img_transform_draw_cached(struct _gt_draw_ctx_s * draw_ctx,
                                  const gt_area_st * coords,
                                  gt_opt_t opa,
                                  const gt_color_t * src_buf,
                                  uint16_t src_w, uint16_t src_h,
                                  gt_angle_t angle, gt_zoom_t zoom,
                                  int16_t pivot_x, int16_t pivot_y,
                                  bool use_nn);

#endif /* GT_USE_IMG_TRANSFORM */

#ifdef __cplusplus
}
#endif

#endif /* _GT_IMG_TRANSFORM_H_ */
