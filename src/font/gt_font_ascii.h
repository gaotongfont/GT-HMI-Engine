/**
 * @file gt_font_ascii.h
 * @brief Built-in ASCII bitmap font resources.
 */
#ifndef _GT_FONT_ASCII_H_
#define _GT_FONT_ASCII_H_

#include <stdint.h>

#ifndef GT_CFG_USE_FONT_16_ASCII
#define GT_CFG_USE_FONT_16_ASCII 1
#endif

#ifndef GT_CFG_USE_FONT_64_ASCII
#define GT_CFG_USE_FONT_64_ASCII 1
#endif

/** Built-in 16-pixel ASCII bitmap data, indexed from character 0x20. */
extern const uint8_t gt_font_ascii_16[];

/**
 * Calculate the non-empty bounds of an A8 glyph bitmap.
 *
 * @param data            Glyph bitmap, one alpha byte per pixel.
 * @param width           Bitmap width in pixels.
 * @param height          Bitmap height in pixels.
 * @param stride          Number of bytes between two rows.
 * @param alpha_threshold Pixels above this alpha value belong to the ink.
 * @param left            Optional output for the left ink offset.
 * @param ink_width       Optional output for the ink width.
 * @return 1 when ink was found, otherwise 0.
 */
int gt_font_a8_metrics(const uint8_t *data, uint16_t width, uint16_t height,
                       uint16_t stride, uint8_t alpha_threshold,
                       uint16_t *left, uint16_t *ink_width);

#endif
