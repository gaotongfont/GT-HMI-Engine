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

extern const uint8_t gt_font_ascii_16[];
int gt_font_a8_metrics(const uint8_t *data, uint16_t width, uint16_t height,
                       uint16_t stride, uint8_t alpha_threshold,
                       uint16_t *left, uint16_t *ink_width);

#endif
