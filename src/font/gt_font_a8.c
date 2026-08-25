#include "./gt_font_ascii.h"
#include <stddef.h>

int gt_font_a8_metrics(const uint8_t *data, uint16_t width, uint16_t height,
                       uint16_t stride, uint8_t alpha_threshold,
                       uint16_t *left, uint16_t *ink_width)
{
    uint16_t min_x = width;
    uint16_t max_x = 0;

    if (NULL == data || 0 == width || 0 == height || stride < width) {
        return 0;
    }

    for (uint16_t y = 0; y < height; ++y) {
        for (uint16_t x = 0; x < width; ++x) {
            if (data[y * stride + x] > alpha_threshold) {
                min_x = min_x < x ? min_x : x;
                max_x = max_x > x ? max_x : x;
            }
        }
    }

    if (min_x == width) {
        return 0;
    }
    if (left) {
        *left = min_x;
    }
    if (ink_width) {
        *ink_width = max_x - min_x + 1;
    }
    return 1;
}
