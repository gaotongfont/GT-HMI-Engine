/**
 * @file gt_draw.c
 * @author yongg
 * @brief Interface functions for drawing text and images
 * @version 0.1
 * @date 2022-08-09 18:29:24
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

 /* include --------------------------------------------------------------*/
#include "stdlib.h"
#include "stdbool.h"

#include "gt_graph_base.h"
#include "gt_obj_pos.h"
#include "../hal/gt_hal_disp.h"
#include "./gt_disp.h"
#include "gt_mem.h"
#include "../others/gt_math.h"
#include "../others/gt_log.h"
#include "../widgets/gt_obj.h"
#include "gt_draw.h"
#include "../others/gt_color.h"
#include "../others/gt_area.h"
#include "../others/gt_gc.h"
#include "gt_fs.h"

#include "../font/gt_font.h"
#include "../font/gt_symbol.h"
#include "gt_img_decoder.h"

/* private define -------------------------------------------------------*/

#define RADIUS_MAX  16

#ifndef _GT_DRAW_USE_RADIUS_MASK_MIX
    /**
     * @brief Use the mask to mix the color of the radius, or use the minimum value of the two colors mask
     *          0: Use the minimum value of the two colors mask
     *          1: Use the mask to mix the color of the radius
     */
    #define _GT_DRAW_USE_RADIUS_MASK_MIX        0
#endif

#if _GT_DRAW_USE_RADIUS_MASK_MIX
    #define _GT_DRAW_MASK_MIX(opa, mask_val)    (gt_per_255(opa) * mask_val >> 15)
#else
    #define _GT_DRAW_MASK_MIX(opa, mask_val)    GT_MIN(opa, mask_val)
#endif



/* private typedef ------------------------------------------------------*/

/**
 * @brief Single line omit mode
 */
typedef struct {
    char * utf8;
    uint8_t len;
    uint16_t width;
}_omit_info_st;


/* static variables -----------------------------------------------------*/

/** Single line omit mode, such as: "..." */
static const GT_ATTRIBUTE_LARGE_RAM_ARRAY char _utf8_dots[] = {
    0x2e, 0x2e, 0x2e
};
/** Single line omit mode, such as: "。。。" */
static const GT_ATTRIBUTE_LARGE_RAM_ARRAY char _utf8_full_stops[] = {
    0xe3, 0x80, 0x82, 0xe3, 0x80, 0x82, 0xe3, 0x80, 0x82
};

static const GT_ATTRIBUTE_LARGE_RAM_ARRAY uint8_t r_1[] = {
    0xB4,0x2D,0xDC,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0xDC,0x00
};
static const GT_ATTRIBUTE_LARGE_RAM_ARRAY uint8_t r_2[] = {
    0x00,0xE0,0x50,0xE0,0x00,0x00,0x00,0x00,0x02,0x00,0x04,0x00,0x01,0x00,0x00,0x00,0x00,0x00
};
static const GT_ATTRIBUTE_LARGE_RAM_ARRAY uint8_t r_3[] = {
    0x00,0xF0,0x80,0x00,0x80,0xF0,0x00,0x00,0x00,0x00,0x02,0x00,0x03,0x00,0x06,0x00,0x02,0x00,0x02,0x00,0x00,0x00,0x00,0x00
};
static const GT_ATTRIBUTE_LARGE_RAM_ARRAY uint8_t r_4[] = {
    0x00,0xA0,0xF0,0xA0,0x00,0x00,0xAB,0x00,0x90,0x2E,0x00,0x00,0x01,0x00,0x02,0x00,0x03,0x00,0x04,0x00,0x04,0x00,0x03,0x00,0x02,0x00,0x01,0x00,0x00,0x00
};
static const GT_ATTRIBUTE_LARGE_RAM_ARRAY uint8_t r_5[] = {
    0x00,0xB0,0x40,0x80,0x40,0xB0,0x00,0x00,0x90,0x2E,0xBF,0x00,0x00,0x00,0x01,0x00,0x02,0x00,0x03,0x00,0x04,0x00,0x06,0x00,0x05,0x00,0x04,0x00,0x04,0x00,0x03,0x00,0x01,0x00,0x00,0x00
};
static const GT_ATTRIBUTE_LARGE_RAM_ARRAY uint8_t r_6[] = {
    0x00,0xC0,0x60,0x00,0xD0,0x10,0xD0,0x00,0x60,0xC0,0x00,0x00,0x04,0x05,0x00,0x00,0x01,0x00,0x02,0x00,0x03,0x00,0x05,0x00,0x07,0x00,0x0A,0x00,0x06,0x00,0x05,0x00,0x05,0x00,0x04,0x00,0x03,0x00,0x01,0x00,0x00,0x00
};
static const GT_ATTRIBUTE_LARGE_RAM_ARRAY uint8_t r_7[] = {
    0x00,0x00,0xD0,0x80,0x10,0xF0,0x40,0xF0,0x40,0xF0,0x10,0x80,0xD0,0x00,0x00,0x07,0x00,0x00,0x01,0x00,0x03,0x00,0x04,0x00,0x06,0x00,0x08,0x00,0x0A,0x00,0x0D,0x00,0x07,0x00,0x06,0x00,0x06,0x00,0x05,0x00,0x04,0x00,0x03,0x00,0x01,0x00,0x00,0x00
};
static const GT_ATTRIBUTE_LARGE_RAM_ARRAY uint8_t r_8[] = {
    0x00,0x00,0xD0,0x90,0x20,0x80,0xC0,0x80,0x20,0x90,0xD0,0x00,0x00,0x05,0x06,0x07,0x1F,0x00,0x00,0x00,0x01,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x06,0x00,0x07,0x00,0x08,0x00,0x0B,0x00,0x08,0x00,0x07,0x00,0x07,0x00,0x07,0x00,0x06,0x00,0x05,0x00,0x04,0x00,0x01,0x00,0x00,0x00
};
static const GT_ATTRIBUTE_LARGE_RAM_ARRAY uint8_t r_9[] = {
    0x00,0x00,0xE0,0xA0,0x40,0x00,0xC0,0x20,0xF0,0x40,0xF0,0x20,0xC0,0x00,0x40,0xA0,0xE0,0x00,0x00,0x1F,0x00,0x00,0x01,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x07,0x00,0x09,0x00,0x0B,0x00,0x0D,0x00,0x11,0x00,0x09,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x07,0x00,0x06,0x00,0x05,0x00,0x04,0x00,0x01,0x00,0x00,0x00
};
static const GT_ATTRIBUTE_LARGE_RAM_ARRAY uint8_t r_10[] = {
    0x00,0x00,0xE0,0xA0,0x50,0x00,0xE0,0x40,0x80,0x00,0x80,0x40,0xE0,0x00,0x50,0xA0,0xE0,0x00,0x00,0x1F,0x7F,0x7E,0x00,0x00,0x01,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x07,0x00,0x08,0x00,0x09,0x00,0x0B,0x00,0x0D,0x00,0x11,0x00,0x0A,0x00,0x09,0x00,0x09,0x00,0x09,0x00,0x08,0x00,0x08,0x00,0x07,0x00,0x06,0x00,0x04,0x00,0x01,0x00,0x00,0x00
};
static const GT_ATTRIBUTE_LARGE_RAM_ARRAY uint8_t r_11[] = {
    0x00,0x00,0xE0,0xB0,0x60,0x10,0xF0,0x80,0x00,0xD0,0xE0,0xD0,0x00,0x80,0xF0,0x10,0x60,0xB0,0xE0,0x00,0x00,0xBE,0x00,0x00,0x00,0x00,0x01,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x07,0x00,0x08,0x00,0x0A,0x00,0x0B,0x00,0x0C,0x00,0x0F,0x00,0x13,0x00,0x0B,0x00,0x0A,0x00,0x0A,0x00,0x0A,0x00,0x09,0x00,0x09,0x00,0x08,0x00,0x07,0x00,0x06,0x00,0x04,0x00,0x01,0x00,0x00,0x00
};
static const GT_ATTRIBUTE_LARGE_RAM_ARRAY uint8_t r_12[] = {
    0x00,0x00,0xE0,0xB0,0x70,0x20,0xA0,0x10,0xF0,0x40,0x70,0x40,0xF0,0x10,0xA0,0x20,0x70,0xB0,0xE0,0x00,0x00,0x56,0x00,0x00,0x50,0x2D,0x00,0x00,0x01,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x06,0x00,0x07,0x00,0x09,0x00,0x0A,0x00,0x0B,0x00,0x0D,0x00,0x0F,0x00,0x13,0x00,0x0C,0x00,0x0B,0x00,0x0B,0x00,0x0B,0x00,0x0B,0x00,0x0A,0x00,0x09,0x00,0x09,0x00,0x08,0x00,0x06,0x00,0x05,0x00,0x01,0x00,0x00,0x00
};
static const GT_ATTRIBUTE_LARGE_RAM_ARRAY uint8_t r_13[] = {
    0x00,0x00,0xF0,0xC0,0x70,0x20,0x00,0xC0,0x30,0x80,0x00,0xB0,0x10,0xB0,0x00,0x80,0x30,0xC0,0x00,0x20,0x70,0xC0,0xF0,0x00,0x00,0x2D,0xC2,0x00,0x00,0x00,0x01,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x06,0x00,0x08,0x00,0x09,0x00,0x0A,0x00,0x0C,0x00,0x0E,0x00,0x10,0x00,0x12,0x00,0x17,0x00,0x0D,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0B,0x00,0x0B,0x00,0x0A,0x00,0x09,0x00,0x08,0x00,0x07,0x00,0x05,0x00,0x01,0x00,0x00,0x00
};
static const GT_ATTRIBUTE_LARGE_RAM_ARRAY uint8_t r_14[] = {
    0x00,0x00,0xF0,0xC0,0x80,0x30,0x00,0xE0,0x60,0x00,0xC0,0x20,0xF0,0xF0,0xF0,0x20,0xC0,0x00,0x60,0xE0,0x00,0x30,0x80,0xC0,0xF0,0x00,0x00,0x00,0xC0,0x00,0x00,0x00,0x01,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x06,0x00,0x08,0x00,0x09,0x00,0x0B,0x00,0x0D,0x00,0x0E,0x00,0x0F,0x00,0x11,0x00,0x14,0x00,0x19,0x00,0x0E,0x00,0x0D,0x00,0x0D,0x00,0x0D,0x00,0x0D,0x00,0x0C,0x00,0x0C,0x00,0x0B,0x00,0x0A,0x00,0x09,0x00,0x08,0x00,0x07,0x00,0x05,0x00,0x01,0x00,0x00,0x00
};
static const GT_ATTRIBUTE_LARGE_RAM_ARRAY uint8_t r_15[] = {
    0x00,0x00,0xF0,0xC0,0x90,0x50,0x00,0xE0,0x80,0x10,0xE0,0x40,0x80,0xA0,0x80,0x40,0xE0,0x10,0x80,0xE0,0x00,0x50,0x90,0xC0,0xF0,0x00,0x00,0x00,0xC0,0x00,0xB1,0x00,0x00,0x00,0x01,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x06,0x00,0x08,0x00,0x09,0x00,0x0B,0x00,0x0C,0x00,0x0D,0x00,0x0E,0x00,0x0F,0x00,0x11,0x00,0x14,0x00,0x19,0x00,0x0F,0x00,0x0E,0x00,0x0E,0x00,0x0E,0x00,0x0E,0x00,0x0D,0x00,0x0D,0x00,0x0C,0x00,0x0C,0x00,0x0B,0x00,0x0A,0x00,0x09,0x00,0x07,0x00,0x05,0x00,0x01,0x00,0x00,0x00
};
static const GT_ATTRIBUTE_LARGE_RAM_ARRAY uint8_t r_16[] = {
    0x00,0x00,0xC0,0x90,0x50,0x00,0xF0,0x90,0x20,0xF0,0x80,0x00,0xD0,0x10,0xD0,0x20,0xD0,0x10,0xD0,0x00,0x80,0xF0,0x20,0x90,0xF0,0x00,0x50,0x90,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x02,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x07,0x00,0x08,0x00,0x0A,0x00,0x0B,0x00,0x0D,0x00,0x0F,0x00,0x11,0x00,0x13,0x00,0x16,0x00,0x19,0x00,0x1D,0x00,0x10,0x00,0x10,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,0x0E,0x00,0x0E,0x00,0x0D,0x00,0x0D,0x00,0x0C,0x00,0x0B,0x00,0x0A,0x00,0x09,0x00,0x07,0x00,0x05,0x00,0x02,0x00,0x00,0x00
};

static const uint8_t * small_radius_make[RADIUS_MAX + 1] = {
    r_1, r_1, r_2, r_3, r_4, r_5, r_6, r_7, r_8, r_9, r_10, r_11, r_12, r_13, r_14, r_15, r_16
};

/* macros ---------------------------------------------------------------*/
static inline bool _is_align_reverse(gt_align_et align) {
    return (align & GT_ALIGN_REVERSE) ? true : false;
}

static inline uint8_t _get_align_type(gt_align_et align) {
    return (align & 0x7F);
}

/* static functions -----------------------------------------------------*/
static void gt_mirror_hor_u8(uint8_t * dst, uint8_t * src, uint32_t w, uint32_t h) {
    uint32_t x, y, idx_dst = 0, idx_src = 0;
    for (y = 0; y < h; y++) {
        idx_src = (y + 1) * w - 1;
        for (x = 0; x < w; x++) {
            dst[idx_dst] = src[idx_src];
            idx_dst++;
            idx_src--;
        }
    }
}

static void gt_mirror_ver_u8(uint8_t * dst, uint8_t * src, uint32_t w, uint32_t h) {
    uint32_t x, y, idx_dst = 0, idx_src = 0;
    for (y = 0; y < h; y++) {
        idx_src = (h - y - 1) * w;
        for (x = 0; x < w; x++) {
            dst[idx_dst] = src[idx_src];
            idx_dst++;
            idx_src++;
        }
    }
}

static void _gt_circ_init(gt_point_st * c, gt_size_t * tmp, uint16_t radius) {
    c->x = radius;
    c->y = 0;
    *tmp = 1 - radius;
}

static bool _gt_circ_cont(gt_point_st * c) {
    return c->y <= c->x ? true : false;
}

static void _gt_circ_next(gt_point_st * c, gt_size_t * tmp) {
    if(*tmp <= 0) {
        (*tmp) += 2 * c->y + 3; /*Change in decision criterion for y -> y+1*/
    } else {
        (*tmp) += 2 * (c->y - c->x) + 5; /*Change for y -> y+1, x -> x-1*/
        c->x--;
    }
    c->y++;
}

static void _gt_radius_mask_free(gt_radius_mask_st *r_mask, uint16_t radius) {
    if (r_mask->buf && radius > RADIUS_MAX) {
        gt_mem_free(r_mask->buf);
    }

    r_mask->buf = NULL;
    r_mask->opa = NULL;
    r_mask->opa_start = NULL;
    r_mask->x_start = NULL;
}

static bool _gt_get_radius_mask(gt_radius_mask_st *r_mask, uint16_t radius) {
    if(!r_mask) return false;

    if(0 == radius){
        return true;
    }

    const uint32_t mask_size = (radius << 1) + 2;

    if (radius <= RADIUS_MAX) {
        r_mask->buf = (uint8_t * )small_radius_make[radius];
        r_mask->opa = r_mask->buf;
        r_mask->opa_start = (uint16_t * )(r_mask->buf + mask_size);
        r_mask->x_start = (uint16_t * )(r_mask->buf + (mask_size << 1));
        return true;
    }

    if(r_mask->buf) {
        GT_LOGE(GT_LOG_TAG_ASSERT, "radius mask already malloc!");
        gt_mem_free(r_mask->buf);
        r_mask->buf = NULL;
    }

    r_mask->buf = gt_mem_malloc(mask_size * 3);
    if(!r_mask->buf){
        GT_LOGE(GT_LOG_TAG_MEM, "radius malloc fail!!! size: %d", mask_size * 3);
        return false;
    }
    gt_memset_0(r_mask->buf, mask_size * 3);
    r_mask->opa = r_mask->buf;
    r_mask->opa_start = (uint16_t * )(r_mask->buf + mask_size);
    r_mask->x_start = (uint16_t * )(r_mask->buf + (mask_size << 1));

#if 0
    /*Special case, handle manually*/
    if(radius == 1) {
        r_mask->opa[0] = 180;
        r_mask->opa_start[0] = 0;
        r_mask->opa_start[1] = 1;
        r_mask->x_start[0] = 0;
        return true;
    }
#endif

    const uint32_t cir_xy_size = (radius + 1) << 3;
    gt_size_t * cir_x = gt_mem_malloc(cir_xy_size);
    if(!cir_x){
        GT_LOGE(GT_LOG_TAG_MEM, "radius cir_x malloc fail!!! size: %d", cir_xy_size);
        gt_mem_free(r_mask->buf);
        r_mask->buf = NULL;
        return false;
    }
    gt_size_t * cir_y = &cir_x[(radius + 1) << 1];

    uint32_t y_8th_cnt = 0;
    gt_point_st cp;
    gt_size_t tmp;
    _gt_circ_init(&cp, &tmp, radius << 2);    /*Upscale by 4*/
    int32_t i;

    uint32_t x_int[4];
    uint32_t x_fract[4];
    gt_size_t cir_size = 0;
    x_int[0] = cp.x >> 2;
    x_fract[0] = 0;

    /*Calculate an 1/8 circle*/
    while(_gt_circ_cont(&cp)) {
        /*Calculate 4 point of the circle */
        for(i = 0; i < 4; i++) {
            _gt_circ_next(&cp, &tmp);
            if(_gt_circ_cont(&cp) == false) break;
            x_int[i] = cp.x >> 2;
            x_fract[i] = cp.x & 0x3;
        }
        if(i != 4) break;

        /*All lines on the same x when downscaled*/
        if(x_int[0] == x_int[3]) {
            cir_x[cir_size] = x_int[0];
            cir_y[cir_size] = y_8th_cnt;
            r_mask->opa[cir_size] = x_fract[0] + x_fract[1] + x_fract[2] + x_fract[3];
            r_mask->opa[cir_size] <<= 4;
            ++cir_size;
        }
        /*Second line on new x when downscaled*/
        else if(x_int[0] != x_int[1]) {
            cir_x[cir_size] = x_int[0];
            cir_y[cir_size] = y_8th_cnt;
            r_mask->opa[cir_size] = x_fract[0];
            r_mask->opa[cir_size] <<= 4;
            ++cir_size;

            cir_x[cir_size] = x_int[0] - 1;
            cir_y[cir_size] = y_8th_cnt;
            r_mask->opa[cir_size] = 1 * 4 + x_fract[1] + x_fract[2] + x_fract[3];
            r_mask->opa[cir_size] <<= 4;
            ++cir_size;
        }
        /*Third line on new x when downscaled*/
        else if(x_int[0] != x_int[2]) {
            cir_x[cir_size] = x_int[0];
            cir_y[cir_size] = y_8th_cnt;
            r_mask->opa[cir_size] = x_fract[0] + x_fract[1];
            r_mask->opa[cir_size] <<= 4;
            ++cir_size;

            cir_x[cir_size] = x_int[0] - 1;
            cir_y[cir_size] = y_8th_cnt;
            r_mask->opa[cir_size] = 2 * 4 + x_fract[2] + x_fract[3];
            r_mask->opa[cir_size] <<= 4;
            ++cir_size;
        }
        /*Forth line on new x when downscaled*/
        else {
            cir_x[cir_size] = x_int[0];
            cir_y[cir_size] = y_8th_cnt;
            r_mask->opa[cir_size] = x_fract[0] + x_fract[1] + x_fract[2];
            r_mask->opa[cir_size] <<= 4;
            ++cir_size;

            cir_x[cir_size] = x_int[0] - 1;
            cir_y[cir_size] = y_8th_cnt;
            r_mask->opa[cir_size] = 3 * 4 + x_fract[3];
            r_mask->opa[cir_size] <<= 4;
            ++cir_size;
        }

        y_8th_cnt++;
    }

    /*The point on the 1/8 circle is special, calculate it manually*/
    int32_t mid = radius * 723;
    int32_t mid_int = mid >> 10;
    if(cir_x[cir_size - 1] != mid_int || cir_y[cir_size - 1] != mid_int) {
        int32_t tmp_val = mid - (mid_int << 10);
        if(tmp_val <= 512) {
            tmp_val = tmp_val * tmp_val * 2;
            tmp_val = tmp_val >> (10 + 6);
        }
        else {
            tmp_val = 1024 - tmp_val;
            tmp_val = tmp_val * tmp_val * 2;
            tmp_val = tmp_val >> (10 + 6);
            tmp_val = 15 - tmp_val;
        }

        cir_x[cir_size] = mid_int;
        cir_y[cir_size] = mid_int;
        r_mask->opa[cir_size] = tmp_val;
        r_mask->opa[cir_size] <<= 4;
        ++cir_size;
    }

    /*Build the second octet by mirroring the first*/
    for(i = cir_size - 2; i >= 0; i--, cir_size++) {
        cir_x[cir_size] = cir_y[i];
        cir_y[cir_size] = cir_x[i];
        r_mask->opa[cir_size] = r_mask->opa[i];
    }

    gt_size_t y = 0;
    i = 0;
    r_mask->opa_start[0] = 0;
    while(i < cir_size) {
        r_mask->opa_start[y] = i;
        r_mask->x_start[y] = cir_x[i];
        for(; cir_y[i] == y && i < (int32_t)cir_size; i++) {
            r_mask->x_start[y] = GT_MIN(r_mask->x_start[y], cir_x[i]);
        }
        y++;
    }

    gt_mem_free(cir_x);
    cir_x = NULL;
    return true;
}

static gt_opa_t* _gt_get_next_line(const gt_radius_mask_st *r_mask, uint16_t y, uint16_t*len, uint16_t* x_start) {
    *len = r_mask->opa_start[y + 1] - r_mask->opa_start[y];
    *x_start = r_mask->x_start[y];
    return &r_mask->opa[r_mask->opa_start[y]];
}

static void _gt_radius_put_mask_buf(const gt_radius_mask_st *r_mask, uint8_t* mask_buf, gt_opa_t opa, uint16_t radius, gt_radius_dir_et dir) {
    gt_size_t i, col = 0, x_pos = 0;
    uint16_t len = 0, x_start = 0;
    gt_opa_t * aa_opa, tmp_opa;
    uint8_t * buf = mask_buf;

    if (R_DIR_TOP_LEFT ==  dir || R_DIR_TOP_RIGHT == dir) {
        for (col = radius - 1; col >= 0; col--) {
            aa_opa = _gt_get_next_line( r_mask, col, &len, &x_start);
            x_pos = R_DIR_TOP_LEFT == dir ? radius - x_start - 1 : x_start;

            for (i = 0; i < len; i++) {
                tmp_opa = aa_opa[len - i - 1];

                if (R_DIR_TOP_LEFT == dir && (x_pos - i >= 0 && x_pos - i < radius)) {
                    buf[x_pos - i] = (gt_per_255(tmp_opa) * opa) >> 15;
                } else if (R_DIR_TOP_RIGHT == dir && (x_pos + i >= 0 && x_pos + i < radius)) {
                    buf[x_pos + i] = (gt_per_255(tmp_opa) * opa) >> 15;
                }
            }

            if (R_DIR_TOP_LEFT == dir) {
                x_pos = radius - x_start;
                gt_memset(&buf[x_pos], opa, radius - x_pos);
            } else {
                gt_memset(&buf[0], opa, x_start);
            }
            buf += radius;
        }
    } else if (R_DIR_BOTTOM_LEFT ==  dir || R_DIR_BOTTOM_RIGHT == dir) {
        for(col = 0; col < radius; col++) {
            aa_opa = _gt_get_next_line( r_mask, col, &len, &x_start);
            x_pos = R_DIR_BOTTOM_LEFT == dir ? radius - x_start - 1 : x_start;

            for (i = 0; i < len; i++) {
                tmp_opa = aa_opa[len - i - 1];

                if (R_DIR_BOTTOM_LEFT == dir && (x_pos - i >= 0 && x_pos - i < radius)) {
                    buf[x_pos - i] = (gt_per_255(tmp_opa) * opa) >> 15;
                } else if (R_DIR_BOTTOM_RIGHT == dir && (x_pos + i >= 0 && x_pos + i < radius)) {
                    buf[x_pos + i] = (gt_per_255(tmp_opa) * opa) >> 15;
                }
            }

            if (R_DIR_BOTTOM_LEFT == dir) {
                x_pos = radius - x_start;
                gt_memset(&buf[x_pos], opa, radius - x_pos);
            } else {
                gt_memset(&buf[0], opa, x_start);
            }
            buf += radius;
        }
    }
}

#if 0
static bool _gt_draw_radius(_gt_draw_ctx_st * draw_ctx, const gt_attr_rect_st * dsc, const gt_area_st * coords) {
    if(NULL == draw_ctx || NULL == dsc || NULL == coords) return false;

    // calc radius
    uint16_t radius = GT_MIN(coords->w >> 1, coords->h >> 1);
    radius = GT_MIN(radius, dsc->radius);
    uint16_t border_width = dsc->border_width;

    if(0 == radius){
        return true;
    }

    uint8_t *mask_buf = gt_mem_malloc(radius * radius);
    if(!mask_buf){
        GT_LOGE(GT_LOG_TAG_MEM, "draw radius malloc fail!!! size: %d", radius*radius);
        return false;
    }

    gt_area_st area_mask;
    gt_draw_blend_dsc_st blend_dsc = {
        .opa        = dsc->bg_opa,
        .color_fill = dsc->bg_color,
        .mask_area  = &area_mask,
        .dst_area   = &area_mask,
        .mask_buf    = mask_buf,
    };

    gt_radius_mask_st radius_mask={0};
    gt_size_t in_radius = radius - border_width;
    bool flag = false;


    flag = _gt_get_radius_mask( &radius_mask, radius);
    if(!flag){
        goto radius_fail;
    }

    if(border_width > 0){
        blend_dsc.color_fill = dsc->border_color;
        // blend_dsc.opa = dsc->border_opa;
    }

    // top-left
    area_mask.x = coords->x;
    area_mask.y = coords->y;
    area_mask.w = radius;
    area_mask.h = radius;
    gt_memset_0(mask_buf, radius * radius);
    _gt_radius_put_mask_buf( &radius_mask, mask_buf, 0xFF, radius, R_DIR_TOP_LEFT);
    gt_draw_blend(draw_ctx, &blend_dsc);
    // top-right
    area_mask.x = coords->x + coords->w - radius;
    gt_memset_0(mask_buf, radius * radius);
    _gt_radius_put_mask_buf( &radius_mask, mask_buf, 0xFF, radius, R_DIR_TOP_RIGHT);
    gt_draw_blend(draw_ctx, &blend_dsc);
    // bottom-left
    area_mask.x = coords->x;
    area_mask.y = coords->y + coords->h - radius;
    gt_memset_0(mask_buf, radius * radius);
    _gt_radius_put_mask_buf( &radius_mask, mask_buf, 0xFF, radius, R_DIR_BOTTOM_LEFT);
    gt_draw_blend(draw_ctx, &blend_dsc);
    // // bottom-right
    area_mask.x = coords->x + coords->w - radius;
    gt_memset_0(mask_buf, radius * radius);
    _gt_radius_put_mask_buf( &radius_mask, mask_buf, 0xFF, radius, R_DIR_BOTTOM_RIGHT);
    gt_draw_blend(draw_ctx, &blend_dsc);
    //

    _gt_radius_mask_free(&radius_mask, radius);

    flag = false;
    if(border_width > 0 && in_radius > 0){
        flag = _gt_get_radius_mask( &radius_mask, in_radius);
        if(!flag){
            goto radius_fail;
        }
        blend_dsc.color_fill = dsc->bg_color;
        blend_dsc.opa = dsc->bg_opa;

        // top-left
        area_mask.x = coords->x + border_width;
        area_mask.y = coords->y + border_width;
        area_mask.w = in_radius;
        area_mask.h = in_radius;
        gt_memset_0(mask_buf, radius * radius);
        _gt_radius_put_mask_buf( &radius_mask, mask_buf, 0xFF, in_radius, R_DIR_TOP_LEFT);
        gt_draw_blend(draw_ctx, &blend_dsc);
        // top-right
        area_mask.x = coords->x + coords->w - in_radius - border_width;
        gt_memset_0(mask_buf, radius * radius);
        _gt_radius_put_mask_buf( &radius_mask, mask_buf, 0xFF, in_radius, R_DIR_TOP_RIGHT);
        gt_draw_blend(draw_ctx, &blend_dsc);
        // bottom-left
        area_mask.x = coords->x + border_width;
        area_mask.y = coords->y + coords->h - in_radius - border_width;
        gt_memset_0(mask_buf, radius * radius);
        _gt_radius_put_mask_buf( &radius_mask, mask_buf, 0xFF, in_radius, R_DIR_BOTTOM_LEFT);
        gt_draw_blend(draw_ctx, &blend_dsc);
        // bottom-right
        area_mask.x = coords->x + coords->w - in_radius - border_width;
        gt_memset_0(mask_buf, radius * radius);
        _gt_radius_put_mask_buf( &radius_mask, mask_buf, 0xFF, in_radius, R_DIR_BOTTOM_RIGHT);
        gt_draw_blend(draw_ctx, &blend_dsc);

        _gt_radius_mask_free(&radius_mask, in_radius);
    }

    if(mask_buf){
        gt_mem_free(mask_buf);
        mask_buf = NULL;
    }

    return true;

radius_fail:

    if(mask_buf){
        gt_mem_free(mask_buf);
        mask_buf = NULL;
    }

    return false;
}
#endif

static bool _gt_radius_mask_init( gt_radius_mask_st*r_mask, const gt_area_st coords, uint16_t radius, bool outer) {
    if(!r_mask) return false;

    gt_area_copy(&r_mask->area, &coords);

    r_mask->outer = outer;
    r_mask->radius = radius;

    return _gt_get_radius_mask(r_mask, radius);
}

static void _gt_radius_mask_deinit(gt_radius_mask_st*r_mask) {
    if (!r_mask) { return; }

    if (r_mask->buf && r_mask->radius > RADIUS_MAX) {
        gt_mem_free(r_mask->buf);
    }

    r_mask->buf = NULL;
    r_mask->opa = NULL;
    r_mask->opa_start = NULL;
    r_mask->x_start = NULL;
    r_mask->radius = 0;
    r_mask->area.x = 0;
    r_mask->area.y = 0;
    r_mask->area.w = 0;
    r_mask->area.h = 0;
}

static bool _gt_get_radius_mask_buf(gt_opa_t* mask_line, gt_size_t x, gt_size_t y, uint16_t len, gt_radius_mask_st* p) {
    if(!p || !mask_line) { return false; }

    if (false == p->outer) {
        if (y < p->area.y || y >= p->area.y + p->area.h) {
            return false;
        }
    } else {
        if (y < p->area.y || y >= p->area.y + p->area.h) {
            return false;
        }
    }

    if ((x >= p->area.x + p->radius && x + len <= p->area.x + p->area.w - p->radius-1) ||
        (y >= p->area.y + p->radius && y <= p->area.y + p->area.h - p->radius-1)) {
        if (false == p->outer) {
            int32_t last = p->area.x - x;
            if (last > len) { return false; }
            if (last >= 0) {
                gt_memset_0(&mask_line[0], last);
            }

            int32_t first = p->area.x + p->area.w - x;
            if (first < 0) {
                return false;
            } else if (first < len) {
                gt_memset_0(&mask_line[first], len - first);
            }
            return false;
        } else {
            int32_t first = p->area.x - x;
            if (first < 0) { first = 0; }
            if (first <= len) {
                int32_t last = p->area.x + p->area.w - x - first;
                if (first + last > len) { last = len - first; }
                if (last >= 0) {
                    gt_memset_0(&mask_line[first], last);
                }
            }
        }
        return false;
    }

    if(p->radius <= 0) return false;

    gt_size_t k = p->area.x - x;
    x -= p->area.x;
    y -= p->area.y;
    //
    gt_size_t aa_len, x_start, tmp_y;
    if (y < p->radius) {
        tmp_y = p->radius - y - 1;
    } else {
        tmp_y = y - (p->area.h - p->radius);
    }
    gt_opa_t tmp_opa;
    gt_opa_t* aa_opa = _gt_get_next_line(p, tmp_y, (uint16_t * )&aa_len, (uint16_t * )&x_start);
    gt_size_t pos_x_right = k + p->area.w - p->radius + x_start;
    gt_size_t pos_x_left = k + p->radius - x_start - 1;
    gt_size_t i;

    if (false == p->outer) {
        for (i = 0; i < aa_len; i++) {
            tmp_opa = aa_opa[aa_len - i - 1];
            if (pos_x_right + i >= 0 && pos_x_right + i < len) {
                mask_line[pos_x_right + i] = _GT_DRAW_MASK_MIX(tmp_opa, mask_line[pos_x_right + i]);
            }
            if (pos_x_left - i >= 0 && pos_x_left - i < len) {
                mask_line[pos_x_left - i] = _GT_DRAW_MASK_MIX(tmp_opa, mask_line[pos_x_left - i]);
            }
        }

        pos_x_right = GT_MAX(0, (GT_MIN(pos_x_right + aa_len, len)));
        gt_memset_0(&mask_line[pos_x_right], len - pos_x_right);

        pos_x_left = GT_MAX(0, (GT_MIN(pos_x_left - aa_len + 1, len)));
        gt_memset_0(&mask_line[0], pos_x_left);
    } else {
        for (i = 0; i < aa_len; i++) {
            tmp_opa = 255 - aa_opa[aa_len - i - 1];
            if (pos_x_right + i >= 0 && pos_x_right + i < len) {
                mask_line[pos_x_right + i] = _GT_DRAW_MASK_MIX(tmp_opa, mask_line[pos_x_right + i]);
            }
            if (pos_x_left - i >= 0 && pos_x_left - i < len) {
                mask_line[pos_x_left - i] = _GT_DRAW_MASK_MIX(tmp_opa, mask_line[pos_x_left - i]);
            }
        }

        int clr_s = GT_MAX(0, (GT_MIN(pos_x_left + 1, len)));
        int clr_len = GT_MAX(0, (GT_MIN(pos_x_right - clr_s, len - clr_s)));
        gt_memset_0(&mask_line[clr_s], clr_len);
    }

    return true;
}

static uint16_t _gt_gray_and_dot_byte_get(uint8_t* gray, uint8_t width, gt_font_st font, uint8_t ret_style) {
    uint16_t dot_byte = 0;
    uint8_t font_type = ret_style % 10;
    uint8_t font_gray = (ret_style/10) > 0 ? (ret_style/10) : 1;

    if ( (GT_FONT_TYPE_FLAG_DOT_MATRIX_NON_WIDTH) == font_type) {
        dot_byte = ((font.info.size + 7) >> 3) * font_gray;
    }
    else if ((GT_FONT_TYPE_FLAG_VEC == font_type)) {
        font_gray = font.info.gray;

        if (1 != font_gray) {
            dot_byte = (((((font.info.size * font_gray + 15) >> 4) << 4) / font_gray + 7) >> 3) * font_gray;
        } else {
            dot_byte = (((font.info.size + 15) >> 4) << 4) >> 3;
        }
    } else {
        dot_byte = ((width + 7) >> 3) * font_gray;
    }
    *gray = font_gray;
    return dot_byte;
}

static inline uint8_t _get_style_space_y(const gt_attr_font_st * dsc, uint16_t font_size) {
    return (dsc->reg.style == GT_FONT_STYLE_NONE) ? 0 : ((font_size >> 4) ? (font_size >> 4) : 1);
}

static inline void _draw_blend_underline(
    _gt_draw_ctx_st * draw_ctx, const gt_attr_font_st * dsc, gt_draw_blend_dsc_st * blend_dsc_style,
    gt_area_st area_font_style, uint16_t font_size, uint16_t width, uint16_t height) {
    area_font_style.y += font_size + 1;
    area_font_style.w = width;
    area_font_style.h = height;
    blend_dsc_style->color_fill = dsc->font_color;
    blend_dsc_style->dst_area = &area_font_style;
    gt_draw_blend(draw_ctx, blend_dsc_style);
}

static inline void _draw_blend_strikethrough(
    _gt_draw_ctx_st * draw_ctx, const gt_attr_font_st * dsc, gt_draw_blend_dsc_st * blend_dsc_style,
    gt_area_st area_font_style, uint16_t font_size, uint16_t width, uint16_t height) {
    area_font_style.y += (font_size >> 1);
    area_font_style.w = width;
    area_font_style.h = height;
    blend_dsc_style->color_fill = dsc->font_color;
    blend_dsc_style->dst_area = &area_font_style;
    gt_draw_blend(draw_ctx, blend_dsc_style);
}


#if _GT_FONT_ENABLE_CONVERTOR
static _gt_draw_font_res_st _draw_text_convertor_split(
    const gt_font_st * fonts, uint8_t lan,
    _gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st * blend_dsc,
    const gt_attr_font_st * dsc, gt_size_t font_disp_h,
    gt_draw_blend_dsc_st * blend_dsc_style, const gt_area_st * coords, uint8_t flag_ol)
{
    uint32_t idx = 0, uni_or_gbk = 0, tmp = 0;
    _gt_draw_font_res_st ret = { .res = GT_RES_OK };
    uint16_t * text = NULL;
    uint8_t * font_buf = NULL;
    text = (uint16_t * )gt_mem_malloc(fonts->len * sizeof(uint16_t));
    if (NULL == text) {
        GT_LOGE(GT_LOG_TAG_GUI, "buf malloc failed, size: %lu", fonts->len * sizeof(uint16_t));
        goto _ret_handle;
    }
    gt_font_st tmp_font = {
        .info = fonts->info,
        .utf8 = fonts->utf8,
        .len = fonts->len,
        .res = NULL,
    };

    while (idx < tmp_font.len) {
        idx += gt_font_one_char_code_len_get((uint8_t * )&tmp_font.utf8[idx], &uni_or_gbk, tmp_font.info.encoding);
        text[tmp++] = (uint16_t)uni_or_gbk;
    }
    uint16_t text_len = tmp;
    uint8_t width, byte_width, font_gray;

#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
    uint16_t font_option = tmp_font.info.style_fl;
    uint8_t ret_style = gt_font_type_get(tmp_font.info.style_fl);
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(&tmp_font.info, ret_style, lan, text_len);
#else
    uint16_t font_option = gt_font_family_get_option(tmp_font.info.family, lan, tmp_font.info.cjk);
    uint8_t ret_style = gt_font_type_get(font_option);
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(&tmp_font.info, lan, text_len);
#endif

    font_buf = (uint8_t * )gt_mem_malloc(font_size_res.font_buff_len);
    if (NULL == font_buf) {
        GT_LOGE(GT_LOG_TAG_GUI, "buf malloc failed, size: %d", font_size_res.font_buff_len);
        goto _ret_handle;
    }
    gt_memset_0(font_buf, font_size_res.font_buff_len);

    uint8_t style_space_y = _get_style_space_y(dsc, tmp_font.info.size);
    font_convertor_st convertor = {
        .fontcode = text,
        .code_len = text_len,
        .fontsize = tmp_font.info.size,
        .fontgray = tmp_font.info.gray,
        .font_option = font_option,
        .thick = tmp_font.info.thick_en,
        .data = font_buf,
        .data_len = font_size_res.font_buff_len,
        .is_rev = _is_align_reverse(dsc->align) ? 0 : 1,
        .is_vec = ret_style == GT_FONT_TYPE_FLAG_VEC ? 1 : 0,
    };
    if(ret_style != GT_FONT_TYPE_FLAG_VEC){
        convertor.fontgray = (ret_style / 10) ? (ret_style / 10) : 1;
    }


    text_len = gt_font_code_transform(&convertor);

    idx = 0;
    tmp = 0;
    tmp_font.res = &font_buf[font_size_res.font_buff_len - font_size_res.font_per_size];
    while (idx < text_len) {
        if (FONT_LAN_HINDI == lan) {
            width = gt_font_get_word_width_figure_up(&convertor.data[idx * font_size_res.font_per_size],
                                                        16, 24, 1);
            tmp = 1;
        }
        else{
            gt_memset_0(&convertor.data[0], font_size_res.font_buff_len);
            if (0x20 == convertor.fontcode[idx]) {
                width = convertor.fontsize >> 1;
            }
            else{
                tmp = gt_font_convertor_data_get(&convertor, idx);
                width = gt_font_get_word_width_figure_up(&convertor.data[0], font_size_res.dot_width,
                                                            convertor.fontsize, convertor.fontgray);
            }
        }
        if (_is_align_reverse(dsc->align)) {
            blend_dsc->dst_area->x -= width;
        }

        if (flag_ol) {
            if(_is_align_reverse(dsc->align)){
                if((blend_dsc->dst_area->x) < (coords->x)){
                    blend_dsc->dst_area->x = coords->x + coords->w - width;
                    blend_dsc->dst_area->y += blend_dsc->dst_area->h;
                    if((blend_dsc->dst_area->y + style_space_y + blend_dsc->dst_area->h) > (coords->y + coords->h)){
                        font_disp_h = (coords->y + coords->h) - blend_dsc->dst_area->y;
                        if(font_disp_h < 2){
                            goto _ret_handle;
                        }
                    }
                }
            } else {
                // out of range
                if ((blend_dsc->dst_area->x + width) > (coords->x + coords->w)) {
                    blend_dsc->dst_area->x = coords->x;
                    blend_dsc->dst_area->y += blend_dsc->dst_area->h;
                    if ((blend_dsc->dst_area->y + style_space_y + blend_dsc->dst_area->h) > (coords->y + coords->h)) {
                        font_disp_h = (coords->y + coords->h) - blend_dsc->dst_area->y;
                        if (font_disp_h < 2) {
                            goto _ret_handle;
                        }
                    }
                }
            }
        }
        blend_dsc->dst_area->h = font_disp_h;
        if (blend_dsc->dst_area->y + style_space_y + blend_dsc->dst_area->h < blend_dsc->view_area->y ||
            blend_dsc->dst_area->y > (blend_dsc->view_area->y + blend_dsc->view_area->h)) {
            goto _next_word_cvt_lb;
        }
        else if (blend_dsc->dst_area->x + width < blend_dsc->view_area->x ||
            blend_dsc->dst_area->x > (blend_dsc->view_area->x + blend_dsc->view_area->w)) {
            goto _next_word_cvt_lb;
        }
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
        if (dsc->reg.touch_point) {
            if (dsc->touch_point->y < blend_dsc->dst_area->y ||
                dsc->touch_point->y > blend_dsc->dst_area->y + blend_dsc->dst_area->h + style_space_y) {
                goto _next_word_cvt_lb;
            }
            if (dsc->touch_point->x < blend_dsc->dst_area->x ||
                dsc->touch_point->x > blend_dsc->dst_area->x + width) {
                goto _next_word_cvt_lb;
            }
            ret.touch_word.word_p = tmp_font.utf8;
            ret.touch_word.len = tmp_font.len;
            goto _next_word_cvt_lb;
        }
#endif
        font_gray = 1;
        if (FONT_LAN_HINDI == lan) {
            byte_width = 2;
            blend_dsc->dst_area->w = byte_width << 3;
            gt_draw_blend_text(draw_ctx, blend_dsc, tmp_font.info.size, byte_width, font_gray,
                                &convertor.data[idx * font_size_res.font_per_size]);
        } else {
            byte_width = (gt_font_get_one_word_width(convertor.fontcode[idx], &tmp_font) + 7) >> 3;
            byte_width = _gt_gray_and_dot_byte_get(&font_gray, byte_width, tmp_font, ret_style);
            blend_dsc->dst_area->w = byte_width << 3;
            gt_draw_blend_text(draw_ctx, blend_dsc, tmp_font.info.size, byte_width, font_gray, &convertor.data[0]);
        }

        /* use style */
        if (dsc->reg.style == GT_FONT_STYLE_UNDERLINE && font_disp_h == tmp_font.info.size) {
            _draw_blend_underline(draw_ctx, dsc, blend_dsc_style, *blend_dsc->dst_area,
                                    tmp_font.info.size, width, style_space_y);
        }
        if (dsc->reg.style == GT_FONT_STYLE_STRIKETHROUGH && (font_disp_h > ((tmp_font.info.size >> 1) + (style_space_y << 1)))) {
            _draw_blend_strikethrough(draw_ctx, dsc, blend_dsc_style, *blend_dsc->dst_area,
                                        tmp_font.info.size, width, style_space_y);
        }
    _next_word_cvt_lb:
        if (_is_align_reverse(dsc->align)) {
            blend_dsc->dst_area->x -= dsc->space_x;
        } else {
            blend_dsc->dst_area->x += width + dsc->space_x;
        }
        idx += tmp;
    }

_ret_handle:
    if(NULL != font_buf){
        gt_mem_free(font_buf);
        font_buf = NULL;
    }
    if(NULL !=text){
        gt_mem_free(text);
        text = NULL;
    }
    ret.area = *blend_dsc->dst_area;
    return ret;
}
#endif  /** _GT_FONT_ENABLE_CONVERTOR */

#if GT_USE_SJPG
static void _filling_by_sjpg_file(_gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st * blend_dsc, _gt_img_dsc_st * dsc_img) {
    uint16_t w, h, y = 0;
    w = dsc_img->header.w;
    h = dsc_img->header.h;

    blend_dsc->dst_area->h = 1;

    /* line by line to blend */
    uint32_t len = w * sizeof(gt_color_t);
    blend_dsc->dst_buf = gt_mem_malloc(len);
    if (NULL == blend_dsc->dst_buf) {
        GT_LOGW(GT_LOG_TAG_GUI, "Per-line buffer malloc failed");
        return;
    }

    while(y < h){
        if( gt_area_is_intersect_screen(&draw_ctx->buf_area, blend_dsc->dst_area) ){
            /* blend to bg */
            gt_img_decoder_read_line(dsc_img, 0, y, len, (uint8_t *)blend_dsc->dst_buf);
            gt_draw_blend(draw_ctx, blend_dsc);
        }
        ++blend_dsc->dst_area->y;
        ++y;
    }
    gt_mem_free(blend_dsc->dst_buf);
    blend_dsc->dst_buf = NULL;
}
#endif

static void _filling_by_ram_data(_gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st * blend_dsc, _gt_img_dsc_st * dsc_img) {
    blend_dsc->mask_buf = &dsc_img->alpha[0];
    blend_dsc->dst_buf = (gt_color_t *)&dsc_img->img[0];
    if (NULL == blend_dsc->dst_buf && NULL == blend_dsc->mask_buf) {
        GT_LOGW(GT_LOG_TAG_GUI, "Raw data buffer or mask is NULL");
        return;
    }

    gt_draw_blend(draw_ctx, blend_dsc);
    blend_dsc->dst_buf = NULL;
    blend_dsc->mask_buf = NULL;
}

#if GT_USE_EXTRA_FULL_IMG_BUFFER
static gt_res_t _filling_by_default_full_mode(_gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st * blend_dsc, _gt_img_dsc_st * dsc_img) {
    uint32_t len = 0, addr_start_alpha = 0;

    len = dsc_img->header.w * dsc_img->header.h;
    addr_start_alpha = len * sizeof(gt_color_t);

    gt_fs_fp_st * fp = dsc_img->fp;
    uint8_t is_mask = 1 == fp->msg.pic.is_alpha ? 1 : 0;
    if (!fp) {
        GT_LOGW(GT_LOG_TAG_GUI, "gt_fs_get_fp failed");
        return GT_RES_FAIL;
    }

    _gt_gc_full_img_buffer_st * full_img = _gt_gc_get_full_img_buffer();
    if (NULL == full_img->buffer) {
        GT_LOGV(GT_LOG_TAG_GUI, "Extra buffer color or alpha pointer is NULL");
        return GT_RES_INV;
    }
    if (full_img->len < (is_mask ? len : 0) + addr_start_alpha) {
        GT_LOGV(GT_LOG_TAG_GUI, "Extra buffer color or alpha size is not enough to store image data");
        return GT_RES_INV;
    }

    gt_fs_read_img_offset(fp, (uint8_t *)full_img->buffer, 0, (is_mask ? len : 0) + addr_start_alpha);

#if GT_USE_EXTRA_FULL_IMG_OFFSET
    blend_dsc->dst_buf = (gt_color_t * )&full_img->buffer[full_img->offset];
#else
    blend_dsc->dst_buf = (gt_color_t * )full_img->buffer;
#endif

    if (is_mask) {
#if GT_USE_EXTRA_FULL_IMG_OFFSET
        blend_dsc->mask_buf = (gt_opa_t * )&full_img->buffer[addr_start_alpha + full_img->offset];
#else
        blend_dsc->mask_buf = (gt_opa_t * )&full_img->buffer[addr_start_alpha];
#endif
    }

    gt_draw_blend(draw_ctx, blend_dsc);

    if (is_mask) {
        blend_dsc->mask_buf = NULL;
    }
line_lb:
    blend_dsc->dst_buf = NULL;
    return GT_RES_OK;
}
#endif

/**
 * @brief GT-HMI Engine Image format
 *
 * @param draw_ctx
 * @param blend_dsc
 * @param dsc_img
 */
static void _filling_by_default_line_mode(_gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st * blend_dsc, _gt_img_dsc_st * dsc_img) {
    uint32_t offset = 0, len = 0;
    uint32_t addr_start_alpha = 0;
    uint16_t w, h, y = 0;
    w = dsc_img->header.w;
    h = dsc_img->header.h;
    len = w * sizeof(gt_color_t);
    addr_start_alpha = len * h;

    gt_fs_fp_st * fp = dsc_img->fp;
    if (!fp) {
        GT_LOGW(GT_LOG_TAG_GUI, "gt_fs_get_fp failed");
        return;
    }

    blend_dsc->dst_area->h = 1;     /** Read and cover line by line */
    blend_dsc->dst_buf = gt_mem_malloc( len );
    if (!blend_dsc->dst_buf) {
        GT_LOGW(GT_LOG_TAG_GUI, "Per-line buffer malloc failed");
        return;
    }

    uint8_t is_mask = 1 == fp->msg.pic.is_alpha ? 1 : 0;
    if (is_mask) {
        blend_dsc->mask_buf = gt_mem_malloc(w);
        if (!blend_dsc->mask_buf) {
            goto line_lb;
        }
    }

    while (y < h) {
        if (gt_area_is_intersect_screen(&draw_ctx->buf_area, blend_dsc->dst_area)) {
            /*read one line color data*/
            gt_fs_read_img_offset(fp, (uint8_t *)blend_dsc->dst_buf, offset, len);
            if (is_mask) {    //png
                gt_fs_read_img_offset(fp, (uint8_t *)blend_dsc->mask_buf, addr_start_alpha, w);
            }
            /* blend to bg */
            gt_draw_blend(draw_ctx, blend_dsc);
        }
        offset += len;
        addr_start_alpha += w;

        ++blend_dsc->dst_area->y;
        ++blend_dsc->mask_area->y;
        ++y;
    }

    if (blend_dsc->mask_buf) {
        gt_mem_free(blend_dsc->mask_buf);
        blend_dsc->mask_buf = NULL;
    }
line_lb:
    if (blend_dsc->dst_buf) {
        gt_mem_free(blend_dsc->dst_buf);
        blend_dsc->dst_buf = NULL;
    }
}

static inline void _filling_by_default(_gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st * blend_dsc, _gt_img_dsc_st * dsc_img) {
#if GT_USE_EXTRA_FULL_IMG_BUFFER
    if (GT_RES_OK == _filling_by_default_full_mode(draw_ctx, blend_dsc, dsc_img)) {
        return;
    }
#endif
    _filling_by_default_line_mode(draw_ctx, blend_dsc, dsc_img);
}

/**
 * @brief
 *
 * @param align
 * @param x
 * @param width the rest of the width
 * @param string_width The string total width pixel
 * @return gt_size_t
 */
static inline gt_size_t _get_align_start_x(gt_align_et align, gt_size_t x, uint16_t width, uint16_t string_width) {
    gt_align_et type = _get_align_type(align);

    if (GT_ALIGN_CENTER == type || GT_ALIGN_CENTER_MID == type || GT_ALIGN_CENTER_BOTTOM == type) {
        return ((width - string_width) >> 1) + x;
    }
    if (GT_ALIGN_RIGHT == type || GT_ALIGN_RIGHT_MID == type || GT_ALIGN_RIGHT_BOTTOM == type) {
        return (width - string_width) + x;
    }
    return x;
}

static inline bool _is_multi_line(const gt_attr_font_st * dsc) {
    return false == dsc->reg.single_line ? true : false;
}

static gt_size_t _get_align_start_y_by_line(const gt_attr_font_st * dsc, const gt_area_st * coords, gt_font_st * tmp_font, uint8_t * txt, uint16_t line_count) {
    gt_size_t offset_y = coords->y;
    uint16_t line_height = (tmp_font->info.size + (_get_style_space_y(dsc, tmp_font->info.size) << 1) + dsc->space_y) * line_count;
    gt_align_et type = _get_align_type(dsc->align);

    if (line_height >= coords->h) {
        return offset_y;
    }
    if (GT_ALIGN_LEFT_MID == type || GT_ALIGN_RIGHT_MID == type || GT_ALIGN_CENTER_MID == type) {
        offset_y = coords->y + ((coords->h - line_height) >> 1);
    }
    else if (GT_ALIGN_LEFT_BOTTOM == type || GT_ALIGN_RIGHT_BOTTOM == type || GT_ALIGN_CENTER_BOTTOM == type) {
        offset_y = coords->y + ((coords->h - line_height));
    }
    return offset_y;
}

static gt_size_t _get_align_start_y(const gt_attr_font_st * dsc, const gt_area_st * coords, gt_font_st * tmp_font, uint8_t * txt) {
    uint32_t ret_w = 0, lan_len = 0;
    uint16_t line_count = 1, idx = 0, txt_len = dsc->font->len;
    uint16_t idx_step = 0;
    gt_size_t disp_w = coords->w;
    gt_align_et type = _get_align_type(dsc->align);
    uint8_t lan = 0;

    if (dsc->reg.enabled_start) {
        disp_w = coords->w - (dsc->start_x - coords->x);
    }

    if (GT_ALIGN_LEFT != type && GT_ALIGN_RIGHT != type && GT_ALIGN_CENTER != type && GT_ALIGN_NONE != type) {
        while (idx < txt_len) {
            if (0x0A == txt[idx]) {
                idx += 1;
                goto _compute_line;
            }
            tmp_font->utf8 = (char * )&txt[idx];
            tmp_font->len = txt_len - idx;
            idx_step = gt_font_split(tmp_font, disp_w, coords->w, dsc->space_x, &ret_w, &lan, &lan_len);

            disp_w -= ret_w;
            if (disp_w <= 0 || 0 == idx_step) {
                if (ret_w >= coords->w) {
                    /** Over widget's width, forced display */
                    idx += idx_step;
                    goto _compute_line;
                }
                goto _compute_line;
            }
            idx += idx_step;
            continue;
    _compute_line:
            ++line_count;
            disp_w = coords->w;
        }
    }

    return _get_align_start_y_by_line(dsc, coords, tmp_font, txt, line_count);
}

static uint16_t _calc_line_height_pixel(gt_area_st * area_font, const gt_attr_font_st * dsc, uint8_t style_space_y) {
    return area_font->h + dsc->space_y + (style_space_y << 1);
}

static _gt_draw_font_res_st draw_text_multiple_line(_gt_draw_ctx_st * draw_ctx, const gt_attr_font_st * dsc, gt_area_st const * view) {
    const gt_area_st * coords = &dsc->logical_area;
    _gt_draw_font_res_st ret = {0};
    _gt_draw_font_res_st tmp_ret = {0};
    gt_area_st area_font = {
        .x = coords->x, .y = coords->y, .w = dsc->font->info.size, .h = dsc->font->info.size
    };

    if (0 == dsc->font->len) {
        gt_align_et type = _get_align_type(dsc->align);
        if (GT_ALIGN_LEFT_MID == type || GT_ALIGN_RIGHT_MID == type || GT_ALIGN_CENTER_MID == type) {
            if (dsc->font->info.size < coords->h) {
                area_font.y = coords->y + ((coords->h - dsc->font->info.size) >> 1);
            }
        }
        ret.area = area_font;
        return ret;
    }
    gt_font_st tmp_font = {
        .info = dsc->font->info,
        .utf8 = dsc->font->utf8,
        .len = dsc->font->len,
        .res = NULL,
    };

#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
    int8_t offset_y = _gt_font_get_type_group_offset_y(tmp_font.info.style_cn, tmp_font.info.style_en);
#else
    int8_t offset_y = _gt_font_get_type_group_offset_y( gt_font_family_get_option(tmp_font.info.family, FONT_LAN_ASCII , tmp_font.info.cjk), \
                                                        gt_font_family_get_option(tmp_font.info.family, FONT_LAN_CN , tmp_font.info.cjk));
#endif
    tmp_font.info.gray = dsc->font->info.gray == 0 ? 1 : dsc->font->info.gray;
    gt_size_t font_disp_h = tmp_font.info.size;
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(&tmp_font.info, 2, 0, 0);
#else
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(&tmp_font.info, FONT_LAN_UNKNOWN, 0);
#endif
    uint16_t data_len = font_size_res.font_per_size;
    uint8_t * mask_line = NULL;

    tmp_font.res = gt_mem_malloc(data_len);
    if (NULL == tmp_font.res) {
        GT_LOGE(GT_LOG_TAG_GUI, "data malloc is failed!!! size = %d", data_len);
        goto _ret_handler;
    }
    mask_line = gt_mem_malloc( tmp_font.info.size << 1 );
    if (NULL == mask_line) {
        GT_LOGE(GT_LOG_TAG_GUI, "mask_line malloc is failed!!! size = %d", tmp_font.info.size << 1);
        goto _ret_handler;
    }
    gt_memset(mask_line, 0xFF, tmp_font.info.size << 1);

    uint8_t * txt = (uint8_t * )dsc->font->utf8;
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
    uint8_t lan = right_to_left_lan_get(dsc->font->info.style_fl);
#else
    uint8_t lan = right_to_left_lan_get(&tmp_font);
#endif

#if _GT_FONT_ENABLE_CONVERTOR
    uint8_t * ret_txt = NULL;
    if (lan == FONT_LAN_HEBREW || lan == FONT_LAN_ARABIC) {
        ret_txt = gt_mem_malloc(dsc->font->len+1);
        if (NULL == ret_txt) {
            GT_LOGE(GT_LOG_TAG_GUI, "ret_txt malloc is failed!!! size = %d", dsc->font->len+1);
            goto _ret_handler;
        }
        gt_memset_0(ret_txt, dsc->font->len+1);
        if (gt_right_to_left_handler(&tmp_font, ret_txt, lan)) {
            txt = ret_txt;
        }
    }
#endif

    uint8_t * txt_2 = &txt[0];
    gt_draw_blend_dsc_st blend_dsc = {
        .mask_buf   = NULL,
        .dst_area   = &area_font,
        .color_fill = dsc->font_color,
        .opa        = dsc->opa,
        .view_area = (gt_area_st * )view,
    };
    gt_draw_blend_dsc_st blend_dsc_style = blend_dsc;
    blend_dsc_style.color_fill  = gt_color_black();
    blend_dsc_style.mask_buf    = mask_line;
    uint8_t style_space_y = _get_style_space_y(dsc, tmp_font.info.size);
    uint16_t per_line_h = (style_space_y << 1) + dsc->font->info.size + dsc->space_y;

    gt_size_t disp_w = coords->w;
    uint32_t uni_or_gbk = 0, ret_w = 0, lan_len = 0;
    uint16_t idx = 0, idx_2 = 0, idx_step = 0, idx_len = 0, txt_len = dsc->font->len;
    uint16_t over_length_offset = ((coords->w / ((tmp_font.info.size >> 1) + dsc->space_x) - 1) >> 1) << 1;
    uint8_t over_length = 0;

#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
    gt_font_touch_word_st tmp_word = {0};
    gt_font_lan_et prev_sty = FONT_LAN_UNKNOWN;
    gt_font_lan_et cur_sty = FONT_LAN_UNKNOWN;
#endif
    bool cov_flag_ol = false;
    bool start_x_flag = dsc->reg.enabled_start;
    _gt_font_dot_ret_st dot_ret = {0};
    uint16_t line_height = _calc_line_height_pixel(&area_font, dsc, style_space_y);
    ret.size.y = line_height;
#if _GT_FONT_ENABLE_CONVERTOR
    uint16_t bidi_len = 0, bidi_max = 1, bidi_pos = 0;
    gt_bidi_st * bidi = (gt_bidi_st * )gt_mem_malloc(bidi_max * sizeof(gt_bidi_st));
    if (NULL == bidi) {
        GT_LOGE(GT_LOG_TAG_GUI, "bidi malloc is failed!!! size = %lu", bidi_max * sizeof(gt_bidi_st));
        goto _ret_handler;
    }
#endif  /** _GT_FONT_ENABLE_CONVERTOR */
    // align vertical
    area_font.y = _get_align_start_y(dsc, coords, &tmp_font, txt);
    if (dsc->reg.enabled_start) {
        area_font.y = dsc->start_y;
    }
    // disp
    if (start_x_flag) {
        disp_w = coords->w - (dsc->start_x - coords->x);
    }

    while (idx < txt_len) {
        // out of range
        if ( (area_font.y + style_space_y + area_font.h) > (coords->y + coords->h) ) {
            font_disp_h = (coords->y + coords->h) - area_font.y;
            if(font_disp_h < 2){
                break;
            }
        }
        // '\n'
        if (0x0A == txt[idx]) {
            idx_step = 0;
            ++idx;
            goto _disp_font;
        }
        if (area_font.y > view->y + view->h) {
            if (dsc->reg.immediately_return) {
                ret.res = GT_RES_INV;
                goto _ret_handler;
            }
        }
        tmp_font.utf8 = (char * )&txt[idx];
        tmp_font.len = txt_len - idx;
        idx_step = gt_font_split(&tmp_font, disp_w, coords->w, dsc->space_x, &ret_w, &lan, &lan_len);

        if (0 == idx_step && 0 == ret_w) {
            ++over_length;
            if (disp_w == coords->w) {
                ++over_length;
            }
        } else {
            over_length = 0;
        }
        if (0 != idx_step) {
            disp_w -= ret_w;
        }
        if (disp_w > tmp_font.info.size && idx_step != 0) {
#if _GT_FONT_ENABLE_CONVERTOR
            if (_gt_font_is_convertor_language(lan)) {
                if (bidi_max <= bidi_len) {
                    ++bidi_max;
                    bidi = (gt_bidi_st * )gt_mem_realloc(bidi, bidi_max * sizeof(gt_bidi_st));
                    if (NULL == bidi) {
                        GT_LOGE(GT_LOG_TAG_GUI, "bidi realloc is failed!!! size = %lu", bidi_max * sizeof(gt_bidi_st));
                        goto _ret_handler;
                    }
                }
                bidi[bidi_len].idx = idx_len;
                bidi[bidi_len].len = idx_step;
                bidi[bidi_len].flag = lan;
                ++bidi_len;
            }
#endif
            idx += idx_step;
            idx_len += idx_step;

            if (idx < txt_len) {
                continue;
            }
        }
    _disp_font:
        if (over_length < 2) {
#if _GT_FONT_ENABLE_CONVERTOR
            if (_gt_font_is_convertor_language(lan) && (idx < txt_len)) {
                if (bidi_max <= bidi_len) {
                    ++bidi_max;
                    bidi = (gt_bidi_st * )gt_mem_realloc(bidi, bidi_max * sizeof(gt_bidi_st));
                }
                bidi[bidi_len].idx = idx_len;
                bidi[bidi_len].len = idx_step;
                bidi[bidi_len].flag = lan;
                ++bidi_len;
            }
#endif
            idx_len = (idx < txt_len) ? idx_len + idx_step : idx_len;
        } else {
#if _GT_FONT_ENABLE_CONVERTOR
            if (_gt_font_is_convertor_language(lan)) {
                if (bidi_max <= bidi_len) {
                    ++bidi_max;
                    bidi = (gt_bidi_st * )gt_mem_realloc(bidi, bidi_max * sizeof(gt_bidi_st));
                }
                bidi[bidi_len].idx = idx_len;
                bidi[bidi_len].len = lan_len;
                bidi[bidi_len].flag = lan;
                ++bidi_len;

                idx_len = lan_len;
                disp_w = 0;
            } else {
                idx_len = over_length_offset;
                disp_w = 0;
            }
#else
            idx_len = over_length_offset;
            disp_w = 0;
#endif
        }
        /** align to left, center or right */
        if (!cov_flag_ol) {
            area_font.x = _get_align_start_x(dsc->align, coords->x, disp_w, 0);
        }
        if (_is_align_reverse(dsc->align) && !cov_flag_ol) {
            area_font.x = (coords->x + coords->w) - (area_font.x - coords->x);
        }
        if (start_x_flag) {
            area_font.x = dsc->start_x;
            start_x_flag = false;
        }
        idx_2 = 0;
#if _GT_FONT_ENABLE_CONVERTOR
        bidi_pos = 0;
#endif
        area_font.h = font_disp_h;

        if (area_font.y + per_line_h < view->y) {
            area_font.x += ret_w;
            goto next_line_lb;
        }
        else if (area_font.y > view->y + view->h) {
            area_font.x += ret_w;
            goto next_line_lb;
        }
        /** Display current line chars */
        while (idx_2 < idx_len) {
            tmp_font.info.size = dsc->font->info.size;
#if _GT_FONT_ENABLE_CONVERTOR
            if (bidi_pos < bidi_len && idx_2 == bidi[bidi_pos].idx) {
                tmp_font.utf8 = (char * )&txt_2[idx_2];
                tmp_font.len = bidi[bidi_pos].len;
                tmp_ret = _draw_text_convertor_split(&tmp_font, bidi[bidi_pos].flag,
                                            draw_ctx, &blend_dsc, dsc, font_disp_h,
                                            &blend_dsc_style, coords,
                                            over_length < 2 ? 0 : 1);
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
                if (tmp_ret.touch_word.word_p) {
                    ret.touch_word = tmp_ret.touch_word;
                }
#endif
                ++bidi_pos;
                idx_2 += tmp_font.len;
                continue;
            }
#endif  /** _GT_FONT_ENABLE_CONVERTOR */

            gt_memset(tmp_font.res, 0, data_len);
            tmp_font.utf8 = (char * )&txt_2[idx_2];
            tmp_font.len = gt_font_one_char_code_len_get(&txt_2[idx_2], &uni_or_gbk, tmp_font.info.encoding);

            uint8_t width = gt_font_get_one_word_width(uni_or_gbk, &tmp_font);

            if (_is_align_reverse(dsc->align)) {
                area_font.x -= width;
            }
            if (area_font.x + width < view->x || area_font.x > (view->x + view->w)) {
                goto next_word_multi_lb;
            }
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
            if (dsc->reg.touch_point) {
                cur_sty = gt_font_lan_get(uni_or_gbk, tmp_font.info.encoding);
                if (cur_sty != prev_sty) {
                    /** different language remark start directly */
                    tmp_word.word_p = tmp_font.utf8;
                    tmp_word.len = txt_len - idx_2;
                }
                if (gt_font_is_illegal_char(uni_or_gbk)) {
                    /** illegal char waiting to next remark start */
                    tmp_word.word_p = NULL;
                    tmp_word.len = 0;
                } else if (NULL == tmp_word.word_p) {
                    tmp_word.word_p = tmp_font.utf8;
                    tmp_word.len = txt_len - idx_2;
                }
                prev_sty = cur_sty;
                if (dsc->touch_point->y < area_font.y || dsc->touch_point->y > area_font.y + per_line_h) {
                    goto check_next_lf_lb;
                }
                if (dsc->touch_point->x < area_font.x || dsc->touch_point->x > area_font.x + width) {
                    goto check_next_lf_lb;
                }
                if (IS_CN_FONT_LAN(cur_sty) && dsc->reg.single_cn) {
                    ret.touch_word.word_p = tmp_font.utf8;
                    ret.touch_word.len = tmp_font.len;
                } else {
                    ret.touch_word.len = gt_font_get_word_byte_length(tmp_word.word_p, tmp_word.len, tmp_font.info.encoding);
                    ret.touch_word.word_p = tmp_word.word_p;
                }

            check_next_lf_lb:
                gt_font_one_char_code_len_get(&txt_2[idx_2 + tmp_font.len], &uni_or_gbk, tmp_font.info.encoding);
                if ('\n' == uni_or_gbk) {
                    tmp_word.word_p = NULL;
                    tmp_word.len = 0;
                }
                goto next_word_multi_lb;
            }
#endif
            if (!gt_symbol_is_valid_range(uni_or_gbk)) {
                gt_memset(&tmp_font.res[data_len >> 1], 0, data_len >> 1);
                dot_ret = gt_font_get_dot(&tmp_font, uni_or_gbk);

                uint8_t byte_width = (width + 7) >> 3;
                uint8_t font_gray = 1;

                byte_width = _gt_gray_and_dot_byte_get(&font_gray, width, tmp_font, dot_ret.type);
                area_font.w = (byte_width / font_gray) << 3;
                if (FONT_LAN_ASCII == gt_font_lan_get(uni_or_gbk, tmp_font.info.encoding)) {
                    area_font.y += offset_y;
                }
                gt_draw_blend_text(draw_ctx, &blend_dsc, tmp_font.info.size,
                                    byte_width, font_gray, (const uint8_t*)tmp_font.res);
                if (FONT_LAN_ASCII == gt_font_lan_get(uni_or_gbk, tmp_font.info.encoding)) {
                    area_font.y -= offset_y;
                }
            } else {
                uint8_t * temp = (uint8_t*)gt_symbol_get_mask_buf(uni_or_gbk, tmp_font.info.size);
                if( temp == NULL ){
                    area_font.x += width + dsc->space_x;
                    idx_2 += tmp_font.len;
                    continue;
                }
                blend_dsc.mask_buf = temp;
                area_font.w = tmp_font.info.size;
                gt_draw_blend(draw_ctx, &blend_dsc);
                blend_dsc.mask_buf = NULL;
            }

            /* use style */
            if (dsc->reg.style == GT_FONT_STYLE_UNDERLINE && font_disp_h == tmp_font.info.size) {
                _draw_blend_underline(draw_ctx, dsc, &blend_dsc_style, area_font,
                                        tmp_font.info.size, width, style_space_y);
            }
            if (dsc->reg.style == GT_FONT_STYLE_STRIKETHROUGH && (font_disp_h > ((tmp_font.info.size >> 1) + (style_space_y << 1)))) {
                _draw_blend_strikethrough(draw_ctx, dsc, &blend_dsc_style, area_font,
                                            tmp_font.info.size, width, style_space_y);
            }
        next_word_multi_lb:
            if (_is_align_reverse(dsc->align)) {
                area_font.x -= dsc->space_x;
            } else {
                area_font.x += width + dsc->space_x;
            }
            idx_2 += tmp_font.len;
        }

    next_line_lb:
#if _GT_FONT_ENABLE_CONVERTOR
        if (_gt_font_is_convertor_language(lan) && over_length > 1) {
            idx += idx_len;
            cov_flag_ol = true;
            if (_is_align_reverse(dsc->align)) {
                disp_w = area_font.x - coords->x;
            } else {
                disp_w = coords->w - (area_font.x - coords->x);
            }
        } else {
#endif  /** _GT_FONT_ENABLE_CONVERTOR */
            cov_flag_ol = false;
            idx += over_length < 2 ? idx_step : over_length_offset;
            if (idx < txt_len) {
                /** LF has skipped to handler, new line or multi '\n' to handler */
                if (0x0A != txt[idx] || (0x0A == txt[idx] && txt[idx] == txt[idx - 1])) {
                    ret.size.y += line_height;
                    area_font.y += line_height;
                }
            }
            else if (0x0A == txt[txt_len - 1]) {
                ret.size.y += line_height;
                area_font.y += line_height;
                if (_is_align_reverse(dsc->align)) {
                    area_font.x = (coords->x + coords->w) - (area_font.x - coords->x);
                } else {
                    area_font.x = _get_align_start_x(dsc->align, coords->x, disp_w, 0);
                }
            }
            disp_w = coords->w;
#if _GT_FONT_ENABLE_CONVERTOR
        }
        bidi_len = 0;
#endif
        idx_len = 0;
        txt_2 = &txt[idx];
        font_disp_h = dsc->font->info.size;
        area_font.h = font_disp_h;
        area_font.w = dsc->font->info.size;
    }

_ret_handler:
#if _GT_FONT_ENABLE_CONVERTOR
    if (NULL != bidi) {
        gt_mem_free(bidi);
        bidi = NULL;
    }
    if (NULL != ret_txt) {
        gt_mem_free(ret_txt);
        ret_txt = NULL;
    }
#endif
    if (NULL != mask_line) {
        gt_mem_free(mask_line);
        mask_line = NULL;
    }
    if (NULL != tmp_font.res) {
        gt_mem_free(tmp_font.res);
        tmp_font.res = NULL;
    }
    ret.area = area_font;
    return ret;
}

static _omit_info_st _omit_get_font_width(gt_font_st * tmp_font) {
    uint32_t uni_or_gbk = 0;
    _omit_info_st ret = {0};

    tmp_font->utf8 = (char * )&_utf8_dots[0];
    tmp_font->len = gt_font_one_char_code_len_get((uint8_t * )&_utf8_dots[0], &uni_or_gbk, tmp_font->info.encoding);
    ret.width = gt_font_get_one_word_width(uni_or_gbk, tmp_font) * 3;
    if (ret.width) {
        if (ret.width < tmp_font->info.size) {
            ret.width = tmp_font->info.size;
        }
        ret.utf8 = (char * )&_utf8_dots[0];
        ret.len = sizeof(_utf8_dots);
        return ret;
    }

    tmp_font->utf8 = (char * )&_utf8_full_stops[0];
    tmp_font->len = gt_font_one_char_code_len_get((uint8_t * )&_utf8_full_stops[0], &uni_or_gbk, tmp_font->info.encoding);
    ret.width = gt_font_get_one_word_width(uni_or_gbk, tmp_font) * 3;
    if (ret.width < tmp_font->info.size) {
        ret.width = tmp_font->info.size;
    }
    ret.utf8 = (char * )&_utf8_full_stops[0];
    ret.len = sizeof(_utf8_full_stops);
    return ret;
}

static _gt_draw_font_res_st draw_text_single_line(_gt_draw_ctx_st * draw_ctx, const gt_attr_font_st * dsc, const gt_area_st * view) {
    gt_area_st * coords = (gt_area_st * )&dsc->logical_area;
    _gt_draw_font_res_st ret = { .res = GT_RES_OK };
    _gt_draw_font_res_st tmp_ret = {0};
    gt_area_st area_font = {
        .x = coords->x, .y = coords->y, .w = dsc->font->info.size, .h = dsc->font->info.size
    };

    if (0 == dsc->font->len) {
        gt_align_et type = _get_align_type(dsc->align);
        if (GT_ALIGN_LEFT_MID == type || GT_ALIGN_RIGHT_MID == type || GT_ALIGN_CENTER_MID == type) {
            if (dsc->font->info.size < coords->h) {
                area_font.y = coords->y + ((coords->h - dsc->font->info.size) >> 1);
            }
        }
        // return area_font;
        ret.area = area_font;
        ret.size.x = 0xffff;
        ret.size.y = 0;
        return ret;
    }
    gt_font_st tmp_font = {
        .info = dsc->font->info,
        .utf8 = dsc->font->utf8,
        .len = dsc->font->len,
        .res = NULL,
    };
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
    int8_t offset_y = _gt_font_get_type_group_offset_y(tmp_font.info.style_cn, tmp_font.info.style_en);
#else
    int8_t offset_y = _gt_font_get_type_group_offset_y( gt_font_family_get_option(tmp_font.info.family, FONT_LAN_ASCII , tmp_font.info.cjk), \
                                                        gt_font_family_get_option(tmp_font.info.family, FONT_LAN_CN , tmp_font.info.cjk));
#endif
    tmp_font.info.gray = dsc->font->info.gray == 0 ? 1 : dsc->font->info.gray;
    gt_size_t font_disp_h = tmp_font.info.size;
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(&tmp_font.info, 2, 0, 0);
#else
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(&tmp_font.info, FONT_LAN_UNKNOWN, 0);
#endif
    uint16_t data_len = font_size_res.font_per_size;
    uint8_t * mask_line = NULL;

    tmp_font.res = gt_mem_malloc(data_len);
    if (NULL == tmp_font.res) {
        GT_LOGE(GT_LOG_TAG_GUI, "data malloc is failed!!! size = %d", data_len);
        goto ret_handler_lb;
    }
    mask_line = gt_mem_malloc( tmp_font.info.size << 1 );
    if (NULL == mask_line) {
        GT_LOGE(GT_LOG_TAG_GUI, "mask_line malloc is failed!!! size = %d", tmp_font.info.size << 1);
        goto ret_handler_lb;
    }
    gt_memset(mask_line, 0xFF, tmp_font.info.size << 1);

    uint8_t * txt = (uint8_t * )dsc->font->utf8;
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
    uint8_t lan = right_to_left_lan_get(dsc->font->info.style_fl);
#else
    uint8_t lan = right_to_left_lan_get(&tmp_font);
#endif

#if _GT_FONT_ENABLE_CONVERTOR
    uint8_t * ret_txt = NULL;
    if (lan == FONT_LAN_HEBREW || lan == FONT_LAN_ARABIC) {
        ret_txt = gt_mem_malloc(dsc->font->len + 1);
        if (NULL == ret_txt) {
            GT_LOGE(GT_LOG_TAG_GUI, "ret_txt malloc is failed!!! size = %d", dsc->font->len+1);
            goto ret_handler_lb;
        }
        gt_memset_0(ret_txt, dsc->font->len + 1);
        if (gt_right_to_left_handler(&tmp_font, ret_txt, lan)) {
            txt = ret_txt;
        }
    }
#endif

    gt_draw_blend_dsc_st blend_dsc = {
        .mask_buf   = NULL,
        .dst_area   = &area_font,
        .color_fill = dsc->font_color,
        .opa        = dsc->opa,
        .view_area = (gt_area_st * )view,
    };
    gt_draw_blend_dsc_st blend_dsc_style = blend_dsc;
    blend_dsc_style.color_fill  = gt_color_black();
    blend_dsc_style.mask_buf    = mask_line;
    uint8_t style_space_y = _get_style_space_y(dsc, tmp_font.info.size);

#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
    gt_font_touch_word_st tmp_word = {0};
    gt_font_lan_et prev_sty = FONT_LAN_UNKNOWN;
    gt_font_lan_et cur_sty = FONT_LAN_UNKNOWN;
#endif
    gt_size_t string_total_width = 0, view_width = coords->w;
    uint32_t uni_or_gbk = 0, remark_idx = 0, lan_len = 0, ret_w = 0;
    uint16_t idx = 0, idx_len = 0, txt_len = dsc->font->len;
    uint16_t over_length_offset = ((coords->w / ((tmp_font.info.size >> 1) + dsc->space_x) - 1) >> 1) << 1;
    uint8_t over_length = 0;
    _omit_info_st omit_info = {0};
    bool start_x_flag = dsc->reg.enabled_start;
    _gt_font_dot_ret_st dot_ret = {0};
    uint8_t width, byte_width, font_gray;

#if _GT_FONT_ENABLE_CONVERTOR
    uint16_t bidi_len = 0, bidi_max = 1, bidi_pos = 0;
    gt_bidi_st * bidi = (gt_bidi_st * )gt_mem_malloc(bidi_max * sizeof(gt_bidi_st));
    if (NULL == bidi) {
        GT_LOGE(GT_LOG_TAG_GUI, "bidi malloc is failed!!! size = %lu", bidi_max * sizeof(gt_bidi_st));
        goto ret_handler_lb;
    }
#endif
    // align vertical
    area_font.y = _get_align_start_y_by_line(dsc, coords, &tmp_font, txt, 1);
    if (dsc->reg.enabled_start) {
        area_font.y = dsc->start_y;
    }
    // out of range
    if ( (area_font.y + style_space_y + area_font.h) > (coords->y + coords->h) ) {
        font_disp_h = (coords->y + coords->h) - area_font.y;
        if(font_disp_h < 2){
            goto ret_handler_lb;
        }
    }
    if (dsc->reg.omit_line) {
        omit_info = _omit_get_font_width(&tmp_font);
        if (view_width < (tmp_font.info.size >> 1)) {
            /** Area can not display anyone char, hide omit char to display the only one char */
            omit_info.width = 0;
            string_total_width = 0;
            view_width = coords->w;
        }
    }
    while (idx < txt_len) {
        if (0x0A == txt[idx]) {
            ++idx;
            continue;
        }
#if 0
        tmp_font.info.size = dsc->font->info.size;
        tmp_font.utf8 = (char * )&txt[idx];
        tmp_font.len = gt_font_one_char_code_len_get(&txt[idx], &uni_or_gbk, tmp_font.info.encoding);
        width = gt_font_get_one_word_width(uni_or_gbk, &tmp_font);
#else
        tmp_font.utf8 = (char * )&txt[idx];
        tmp_font.len = txt_len - idx;
        tmp_font.len = gt_font_split(&tmp_font, view_width, coords->w, dsc->space_x, &ret_w, &lan, &lan_len);
#endif
        if (0 == tmp_font.len) {
            ++idx;
            continue;
        }

        if (dsc->reg.omit_line) {
            view_width -= ret_w;
            if (0 == remark_idx && view_width < omit_info.width + 1) {
                remark_idx = idx + tmp_font.len;
            }
        }
        if (tmp_font.len) {
            string_total_width += ret_w;
        }
#if _GT_FONT_ENABLE_CONVERTOR
        if (_gt_font_is_convertor_language(lan)) {
            if (bidi_max <= bidi_len) {
                ++bidi_max;
                bidi = (gt_bidi_st * )gt_mem_realloc(bidi, bidi_max * sizeof(gt_bidi_st));
                if (NULL == bidi) {
                    GT_LOGE(GT_LOG_TAG_GUI, "bidi realloc is failed!!! size = %lu", bidi_max * sizeof(gt_bidi_st));
                    goto ret_handler_lb;
                }
            }
            bidi[bidi_len].idx = idx_len;
            bidi[bidi_len].len = tmp_font.len;
            bidi[bidi_len].flag = lan;
            ++bidi_len;
        }
#endif
        idx += tmp_font.len;
        idx_len += tmp_font.len;
    }
    if (dsc->reg.omit_line && view_width < 0) {
        idx_len = remark_idx;
    }
    ret.size.x = string_total_width;
    if (string_total_width > view->w) {
        area_font.x = coords->x;
    } else {
        /** Area can display all content, no need for omit width */
        area_font.x = _get_align_start_x(dsc->align, view->x, view->w, string_total_width);
    }
    if (_is_align_reverse(dsc->align)) {
        area_font.x = (coords->x + coords->w) - (area_font.x - coords->x);
    }
    if (start_x_flag) {
        area_font.x = dsc->start_x;
        start_x_flag = false;
    }

    idx = 0;
    area_font.h = font_disp_h;
    while (idx < idx_len) {
        tmp_font.info.size = dsc->font->info.size;
#if _GT_FONT_ENABLE_CONVERTOR
        if (bidi_pos < bidi_len && idx == bidi[bidi_pos].idx) {
            tmp_font.utf8 = (char * )&txt[idx];
            tmp_font.len = bidi[bidi_pos].len;
            tmp_ret = _draw_text_convertor_split(&tmp_font, bidi[bidi_pos].flag, draw_ctx,
                                        &blend_dsc, dsc, font_disp_h, &blend_dsc_style,
                                        coords, (over_length < 2 ? 0 : 1));
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
            if (tmp_ret.touch_word.word_p) {
                ret.touch_word = tmp_ret.touch_word;
            }
#endif
            ++bidi_pos;
            idx += tmp_font.len;
            continue;
        }
#endif  /** _GT_FONT_ENABLE_CONVERTOR */

        gt_memset(tmp_font.res, 0, data_len);
        tmp_font.utf8 = (char * )&txt[idx];
        tmp_font.len = gt_font_one_char_code_len_get(&txt[idx], &uni_or_gbk, tmp_font.info.encoding);

        width = gt_font_get_one_word_width(uni_or_gbk, &tmp_font);

        if (_is_align_reverse(dsc->align)) {
            area_font.x -= width;
        }

        if (area_font.x + width < view->x || area_font.x > view->x + view->w) {
            goto next_word_lb;
        }
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
        if (dsc->reg.touch_point) {
            cur_sty = gt_font_lan_get(uni_or_gbk, tmp_font.info.encoding);
            if (cur_sty != prev_sty) {
                /** different language remark start directly */
                tmp_word.word_p = tmp_font.utf8;
                tmp_word.len = txt_len - idx;
            }
            if (gt_font_is_illegal_char(uni_or_gbk)) {
                /** illegal char waiting to next remark start */
                tmp_word.word_p = NULL;
                tmp_word.len = 0;
            } else if (NULL == tmp_word.word_p) {
                tmp_word.word_p = tmp_font.utf8;
                tmp_word.len = txt_len - idx;
            }
            prev_sty = cur_sty;
            if (dsc->touch_point->x < area_font.x || dsc->touch_point->x > area_font.x + width) {
                goto check_next_lf_lb;
            }
            if (IS_CN_FONT_LAN(cur_sty) && dsc->reg.single_cn) {
                ret.touch_word.word_p = tmp_font.utf8;
                ret.touch_word.len = tmp_font.len;
            } else {
                ret.touch_word.len = gt_font_get_word_byte_length(tmp_word.word_p, tmp_word.len, tmp_font.info.encoding);
                ret.touch_word.word_p = tmp_word.word_p;
            }

        check_next_lf_lb:
            gt_font_one_char_code_len_get(&txt[idx + tmp_font.len], &uni_or_gbk, tmp_font.info.encoding);
            if ('\n' == uni_or_gbk) {
                tmp_word.word_p = NULL;
                tmp_word.len = 0;
            }
            goto next_word_lb;
        }
#endif
        if( !gt_symbol_is_valid_range(uni_or_gbk) ){
            gt_memset(&tmp_font.res[data_len >> 1], 0, data_len >> 1);
            dot_ret = gt_font_get_dot(&tmp_font, uni_or_gbk);

            byte_width = (width + 7) >> 3;
            font_gray = 1;

            byte_width = _gt_gray_and_dot_byte_get(&font_gray, width, tmp_font, dot_ret.type);
            area_font.w = (byte_width / font_gray) << 3;
            if (FONT_LAN_ASCII == gt_font_lan_get(uni_or_gbk, tmp_font.info.encoding)) {
                area_font.y += offset_y;
            }
            gt_draw_blend_text(draw_ctx, &blend_dsc, tmp_font.info.size,
                                byte_width, font_gray, (const uint8_t*)tmp_font.res);
            if (FONT_LAN_ASCII == gt_font_lan_get(uni_or_gbk, tmp_font.info.encoding)) {
                area_font.y -= offset_y;
            }
        } else {
            blend_dsc.mask_buf = (uint8_t*)gt_symbol_get_mask_buf(uni_or_gbk, tmp_font.info.size);
            if (NULL == blend_dsc.mask_buf) {
                area_font.x += width + dsc->space_x;
                idx += tmp_font.len;
                continue;
            }
            area_font.w = tmp_font.info.size;
            gt_draw_blend(draw_ctx, &blend_dsc);
            blend_dsc.mask_buf = NULL;
        }

        /* use style */
        if (dsc->reg.style == GT_FONT_STYLE_UNDERLINE && font_disp_h == tmp_font.info.size) {
            _draw_blend_underline(draw_ctx, dsc, &blend_dsc_style, area_font,
                                    tmp_font.info.size, width, style_space_y);
        }
        if (dsc->reg.style == GT_FONT_STYLE_STRIKETHROUGH && (font_disp_h > ((tmp_font.info.size >> 1) + (style_space_y << 1)))) {
            _draw_blend_strikethrough(draw_ctx, dsc, &blend_dsc_style, area_font,
                                        tmp_font.info.size, width, style_space_y);
        }
next_word_lb:
        if (_is_align_reverse(dsc->align)) {
            area_font.x -= dsc->space_x;
        } else {
            area_font.x += width + dsc->space_x;
        }
        idx += tmp_font.len;
    }

    if (dsc->reg.omit_line && view_width < omit_info.width && idx_len < txt_len) {
        // Display "..." or "。。。"
        tmp_font.utf8 = omit_info.utf8;
        tmp_font.len = gt_font_one_char_code_len_get((uint8_t * )omit_info.utf8, &uni_or_gbk, tmp_font.info.encoding);
        width = gt_font_get_one_word_width(uni_or_gbk, &tmp_font);

        tmp_font.info.size = dsc->font->info.size;
        dot_ret = gt_font_get_dot(&tmp_font, uni_or_gbk);
        byte_width = (width + 7) >> 3;
        font_gray = 1;

        byte_width = _gt_gray_and_dot_byte_get(&font_gray, width, tmp_font, dot_ret.type);
        area_font.w = (byte_width / font_gray) << 3;
        idx = 0;
        while (idx < omit_info.len)  {
            gt_draw_blend_text(draw_ctx, &blend_dsc, tmp_font.info.size, byte_width,
                                font_gray, (const uint8_t*)tmp_font.res);
            if (_is_align_reverse(dsc->align)) {
                area_font.x -= dsc->space_x;
            } else {
                area_font.x += width + dsc->space_x;
            }
            idx += tmp_font.len;
        }
    }

    area_font.w = tmp_font.info.size;
    area_font.h = tmp_font.info.size;

ret_handler_lb:
#if _GT_FONT_ENABLE_CONVERTOR
    if (NULL != bidi) {
        gt_mem_free(bidi);
        bidi = NULL;
    }
    if (NULL != ret_txt) {
        gt_mem_free(ret_txt);
        ret_txt = NULL;
    }
#endif
    if (NULL != mask_line) {
        gt_mem_free(mask_line);
        mask_line = NULL;
    }
    if (NULL != tmp_font.res) {
        gt_mem_free(tmp_font.res);
        tmp_font.res = NULL;
    }
    ret.area = area_font;
    return ret;
}

static void _draw_dot_matrix_data(_gt_draw_ctx_st * draw_ctx, const gt_attr_rect_st * dsc, const gt_area_st * coords, gt_draw_blend_dsc_st * blend_dsc_p) {
    uint32_t len = coords->w * coords->h, index = 0, buf_idx = 0;
    uint8_t idx_bit = 0;
    uint8_t * mask_buf = gt_mem_malloc(len);
    GT_CHECK_BACK(mask_buf);

    gt_area_st area_gy = {0};
    gt_area_copy(&area_gy, coords);
    blend_dsc_p->mask_buf = mask_buf;
    blend_dsc_p->dst_area = &area_gy;
    blend_dsc_p->mask_area = &area_gy;

    len >>= 3;
    while (index < len) {
        for (idx_bit = 0; idx_bit < 8; idx_bit++) {
            if ((dsc->data_gray[index] << idx_bit) & 0x80) {
                mask_buf[buf_idx] = 0xFF;
            }else{
                mask_buf[buf_idx] = 0x00;
            }
            ++buf_idx;
        }
        ++index;
    }
    gt_draw_blend(draw_ctx, blend_dsc_p);

    gt_mem_free(mask_buf);
    mask_buf = NULL;
    blend_dsc_p->mask_buf = NULL;
}

/* global functions / API interface -------------------------------------*/

void draw_bg(_gt_draw_ctx_st * draw_ctx, const gt_attr_rect_st * dsc, const gt_area_st * coords)
{
    if (dsc->bg_opa <= GT_OPA_MIN) {
        return;
    }
    if (NULL == draw_ctx->buf) {
        return;
    }
    if (0 == coords->w || 0 == coords->h) {
        return;
    }
    if( !gt_area_is_intersect_screen(&draw_ctx->buf_area, coords) ){
        return;
    }

    gt_draw_blend_dsc_st blend_dsc = {0};
    blend_dsc.color_fill = dsc->bg_color;
    blend_dsc.view_area = dsc->limit_area;
    blend_dsc.opa = dsc->bg_opa;
    blend_dsc.mask_buf = NULL;

    // Dot matrix data
    if (dsc->gray != 0) {
        if (!dsc->data_gray) { return; }
        _draw_dot_matrix_data(draw_ctx, dsc, coords, &blend_dsc);
        return ;
    }

    gt_area_st area_border = {0};
    gt_area_copy(&area_border, coords);
    uint16_t r_bd = GT_MIN(dsc->radius, (GT_MIN(area_border.w >> 1, area_border.h >> 1)));

    gt_area_st area_bg = {0};
    area_bg = gt_area_reduce(area_border, dsc->border_width);

    uint16_t r_bg = (r_bd < dsc->border_width) ? 0 : r_bd - dsc->border_width;
    r_bg = GT_MIN(r_bg, (GT_MIN(area_bg.w >> 1, area_bg.h >> 1)));

    // draw bg
    if (dsc->reg.is_fill) {
        if (r_bg > 0) {
            gt_radius_mask_st r_bg_radius = {0};
            if (!_gt_radius_mask_init(&r_bg_radius, area_bg, r_bg, false)) {
                GT_LOGE( GT_LOG_TAG_GUI,"draw bg radius init fail!!! radius = %d", r_bg);
                return ;
            }

            gt_radius_mask_st r_cl_radius = {0};
            if (dsc->base_area) {
                uint16_t r_cl = GT_MIN(dsc->radius, (GT_MIN(dsc->base_area->w >> 1, dsc->base_area->h >> 1)));
                if (!_gt_radius_mask_init(&r_cl_radius, *dsc->base_area, r_cl, false)) {
                    _gt_radius_mask_deinit(&r_bg_radius);
                    GT_LOGE( GT_LOG_TAG_GUI,"draw clipped radius init fail!!! radius = %d", r_cl);
                    return ;
                }
            }

            gt_opa_t* mask_bg_line = gt_mem_malloc(area_bg.w);
            if (NULL == mask_bg_line) {
                if (dsc->base_area) {
                    _gt_radius_mask_deinit(&r_cl_radius);
                }
                _gt_radius_mask_deinit(&r_bg_radius);
                GT_LOGE( GT_LOG_TAG_MEM,"draw bg mask_line malloc is fail!!! size = %d", area_bg.w);
                return ;
            }

            gt_area_st area_line = {0};
            area_line.x = area_bg.x;
            area_line.w = area_bg.w;
            area_line.h = 1;

            blend_dsc.dst_area = &area_line;
            blend_dsc.mask_buf = mask_bg_line;
            blend_dsc.mask_area = &area_line;

            for (gt_size_t i = 0; i < area_bg.h; i++) {
                gt_memset(mask_bg_line, 0xFF, area_bg.w);
                area_line.y = area_bg.y + i;
                _gt_get_radius_mask_buf(mask_bg_line, area_line.x, area_line.y, area_line.w, &r_bg_radius);
                if (dsc->base_area) {
                    _gt_get_radius_mask_buf(mask_bg_line, area_line.x, area_line.y, area_line.w, &r_cl_radius);
                }
                gt_draw_blend(draw_ctx, &blend_dsc);
            }

            if (dsc->base_area) {
                _gt_radius_mask_deinit(&r_cl_radius);
            }
            _gt_radius_mask_deinit(&r_bg_radius);
            gt_mem_free(mask_bg_line);
            mask_bg_line = NULL;
            blend_dsc.mask_buf = NULL;
        } else {
            blend_dsc.dst_area = &area_bg;
            gt_draw_blend(draw_ctx, &blend_dsc);
        }
    }

    if(dsc->border_width <= 0) { return ; }

    // draw board
    gt_area_st area_bd;
    blend_dsc.dst_area = &area_bd;
    blend_dsc.color_fill = dsc->border_color;
    /*
        border opa should be used here, but there was no border opa before,
        so for compatibility, border opa is not used.
    */
    // blend_dsc.opa = dsc->border_opa;
    if (r_bd > 0) {
        gt_radius_mask_st in_r_mask = {0};
        if (!_gt_radius_mask_init(&in_r_mask, area_bg, r_bg, true)) {
            GT_LOGE( GT_LOG_TAG_GUI, "draw board radius init fail!!! radius = %d", r_bg);
            return ;
        }

        gt_radius_mask_st out_r_mask = {0};
        if (!_gt_radius_mask_init(&out_r_mask, area_border, r_bd, false)) {
            _gt_radius_mask_deinit(&in_r_mask);
            GT_LOGE( GT_LOG_TAG_GUI, "draw board radius init fail!!! radius = %d", r_bd);
            return ;
        }

        gt_opa_t* mask_bd_line = gt_mem_malloc(area_border.w);
        if (NULL == mask_bd_line) {
            _gt_radius_mask_deinit(&in_r_mask);
            _gt_radius_mask_deinit(&out_r_mask);
            GT_LOGE( GT_LOG_TAG_MEM, "draw board mask_line malloc is fail!!! size = %d", area_border.w);
            return ;
        }

        area_bd.x = area_border.x;
        area_bd.w = area_border.w;
        area_bd.h = 1;

        blend_dsc.mask_buf = mask_bd_line;
        blend_dsc.mask_area = &area_bd;

        for (gt_size_t i = 0; i < area_border.h; i++) {
            gt_memset(mask_bd_line, 0xFF, area_border.w);
            area_bd.y = area_border.y + i;
            _gt_get_radius_mask_buf(mask_bd_line, area_bd.x, area_bd.y, area_border.w, &in_r_mask);
            _gt_get_radius_mask_buf(mask_bd_line, area_bd.x, area_bd.y, area_bd.w, &out_r_mask);
            gt_draw_blend(draw_ctx, &blend_dsc);
        }
        _gt_radius_mask_deinit(&in_r_mask);
        _gt_radius_mask_deinit(&out_r_mask);
        gt_mem_free(mask_bd_line);
        mask_bd_line = NULL;
        blend_dsc.mask_buf = NULL;
    } else {
        // top
        area_bd.x = area_border.x;
        area_bd.y = area_border.y;
        area_bd.w = area_border.w;
        area_bd.h = dsc->border_width;
        gt_draw_blend(draw_ctx, &blend_dsc);
        // bottom
        area_bd.y = area_border.y + area_border.h - dsc->border_width;
        gt_draw_blend(draw_ctx, &blend_dsc);
        // left
        area_bd.x = area_border.x;
        area_bd.y = area_border.y + dsc->border_width;
        area_bd.w = dsc->border_width;
        area_bd.h = area_border.h - dsc->border_width;
        gt_draw_blend(draw_ctx, &blend_dsc);
        // right
        area_bd.x = area_border.x + area_border.w - dsc->border_width;
        gt_draw_blend(draw_ctx, &blend_dsc);
    }
}

_gt_draw_font_res_st draw_text(_gt_draw_ctx_st * draw_ctx, const gt_attr_font_st * dsc, const gt_area_st * coords)
{
    if (dsc->reg.single_line) {
        return draw_text_single_line(draw_ctx, dsc, coords);
    }
    return draw_text_multiple_line(draw_ctx, dsc, coords);
}

void draw_bg_img(_gt_draw_ctx_st * draw_ctx, const gt_attr_rect_st * dsc, gt_area_st * coords)
{
    if (dsc->bg_opa < GT_OPA_MIN) {
        return;
    }
    if (NULL == draw_ctx->buf) {
        return;
    }

    _gt_img_dsc_st dsc_img = {0};
    if (NULL == dsc->bg_img_src) {
        if (dsc->raw_img) {
            dsc_img = *dsc->raw_img;
        }
#if GT_USE_FILE_HEADER
        else if (GT_RES_FAIL == gt_img_decoder_fh_open(&dsc_img, dsc->file_header)) {
            GT_LOGW(GT_LOG_TAG_DATA, "Open image decoder by file header failed");
            return;
        }
#endif
#if GT_USE_DIRECT_ADDR
        else if (GT_RES_FAIL == gt_img_decoder_direct_addr_open(&dsc_img, dsc->addr)) {
            GT_LOGW(GT_LOG_TAG_DATA, "Open image decoder by direct address failed");
            return;
        }
#endif
        else {
            GT_LOGW(GT_LOG_TAG_DATA, "No image source");
            return;
        }
    }
    else if (GT_RES_FAIL == gt_img_decoder_open(&dsc_img, dsc->bg_img_src)) {
        GT_LOGW(GT_LOG_TAG_DATA, "[%s] Open image decoder failed", dsc->bg_img_src);
        return;
    }
    coords->w = dsc_img.header.w;
    coords->h = dsc_img.header.h;
    _gt_disp_update_max_area(coords, true);

    if (NULL == draw_ctx->buf) {
        goto close_lb;
    }

    gt_area_st dst_area = *coords, mask_area = *coords;
    gt_draw_blend_dsc_st blend_dsc = {
        .dst_buf = NULL,
        .mask_buf = NULL,
        .color_fill = dsc->raw_img ? dsc->raw_img->fill_color : gt_color_black(),
        .dst_area = &dst_area,
        .mask_area = &mask_area,
        // .view_area = dsc->limit_area,   // TODO feat limit area range
        .opa = dsc->bg_opa
    };

    /* img src type */
    switch(dsc_img.header.type) {
#if GT_USE_PNG
        case GT_IMG_DECODER_TYPE_PNG: {
            _filling_by_ram_data(draw_ctx, &blend_dsc, &dsc_img);
            break;
        }
#endif
#if GT_USE_SJPG
        case GT_IMG_DECODER_TYPE_SJPG: {
            _filling_by_sjpg_file(draw_ctx, &blend_dsc, &dsc_img);
            break;
        }
#endif
        case GT_IMG_DECODER_TYPE_RAM: {
            _filling_by_ram_data(draw_ctx, &blend_dsc, &dsc_img);
            break;
        }
        default: {
            _filling_by_default(draw_ctx, &blend_dsc, &dsc_img);
            break;
        }
    }

close_lb:
    gt_img_decoder_close(&dsc_img);
}

static inline bool _is_brush_round(gt_line_st const * const line) {
    return GT_BRUSH_TYPE_ROUND == line->brush;
}

void gt_draw_point(_gt_draw_ctx_st * draw_ctx, gt_attr_point_st const * dsc, gt_area_st const * coords)
{
    if (dsc->line.opa < GT_OPA_MIN) {
        return;
    }
    if (NULL == draw_ctx->buf) {
        return;
    }
    if (!gt_area_is_intersect_screen(&draw_ctx->buf_area, coords)) {
        return;
    }
    gt_radius_mask_st radius_mask = {0};
    uint16_t rd_val = dsc->line.width ? dsc->line.width : 4;
    gt_area_st area_line = { 0, 0, rd_val, rd_val };
    bool is_round = _is_brush_round(&dsc->line);
    if (is_round) {
        if (!_gt_radius_mask_init(&radius_mask, area_line, rd_val >> 1, false)) {
            return;
        }
    }
    gt_area_st * remark_area = draw_ctx->parent_area;
    draw_ctx->parent_area = (gt_area_st * )coords;  /** remark before parent area value, return will reload */

    area_line.x = dsc->pos.x - (rd_val >> 1);     /** move to circle center point */
    area_line.y = dsc->pos.y - (rd_val >> 1);
    area_line.h = 1;

    gt_draw_blend_dsc_st blend_dsc = {
        .dst_area = &area_line,
        .mask_area = &area_line,
        .color_fill = dsc->line.color,
        .opa = dsc->line.opa,
        .mask_buf = gt_mem_malloc(rd_val),
    };
    if (NULL == blend_dsc.mask_buf) {
        goto radius_lb;
    }
    for (gt_size_t i = 0; i < rd_val; ++i) {
        gt_memset(blend_dsc.mask_buf, 0xFF, rd_val);
        if (is_round) {
            _gt_get_radius_mask_buf(blend_dsc.mask_buf, 0, i, area_line.w, &radius_mask);
        }
        ++area_line.y;
        gt_draw_blend(draw_ctx, &blend_dsc);
    }

    gt_mem_free(blend_dsc.mask_buf);
radius_lb:
    _gt_radius_mask_deinit(&radius_mask);
    draw_ctx->parent_area = remark_area;    /** reset to before value */
}

void gt_draw_line(_gt_draw_ctx_st * draw_ctx, gt_attr_line_st const * dsc, const gt_area_st * coords)
{
    if (dsc->line.opa < GT_OPA_MIN) {
        return;
    }
    if (NULL == draw_ctx->buf) {
        return;
    }
    if (!gt_area_is_intersect_screen(&draw_ctx->buf_area, coords)) {
        return;
    }
    gt_radius_mask_st radius_mask = {0};
    uint16_t rd_val = dsc->line.width ? dsc->line.width : 4;
    gt_area_st area_line = { 0, 0, rd_val, rd_val };
    bool is_round = _is_brush_round(&dsc->line);
    if (is_round) {
        if (!_gt_radius_mask_init(&radius_mask, area_line, rd_val >> 1, false)) {
            return;
        }
    }
    gt_area_st * remark_area = draw_ctx->parent_area;
    draw_ctx->parent_area = (gt_area_st * )coords;  /** remark before parent area value, return will reload */

    area_line.x = dsc->start.x - (rd_val >> 1);     /** move to circle center point */
    area_line.y = dsc->start.y - (rd_val >> 1);
    area_line.h = 1;

    gt_draw_blend_dsc_st blend_dsc = {
        .dst_area = &area_line,
        .mask_area = &area_line,
        .color_fill = dsc->line.color,
        .opa = dsc->line.opa,
        .mask_buf = gt_mem_malloc(rd_val),
    };
    if (NULL == blend_dsc.mask_buf) {
        goto radius_lb;
    }
    gt_point_st tmp = { .x = area_line.x, .y = area_line.y };
    gt_point_st end = { .x = dsc->end.x - (rd_val >> 1), .y = dsc->end.y - (rd_val >> 1) };
    gt_point_st diff = {
        .x = end.x == tmp.x ? 0 : (end.x > tmp.x ? 1 : -1),
        .y = end.y == tmp.y ? 0 : (end.y > tmp.y ? 1 : -1)
    };
    gt_point_st start = { .x = tmp.x, .y = tmp.y };
    gt_point_st distance = { .x = end.x - start.x, .y = end.y - start.y };
    uint8_t dir_hor = gt_abs(distance.x) > gt_abs(distance.y) ? 1 : 0;

    do {
        if (is_round) {
            for (gt_size_t i = 0; i < rd_val; ++i) {
                gt_memset(blend_dsc.mask_buf, 0xFF, rd_val);
                _gt_get_radius_mask_buf(blend_dsc.mask_buf, 0, i, area_line.w, &radius_mask);
                area_line.x = tmp.x;
                area_line.y = tmp.y + i;
                gt_draw_blend(draw_ctx, &blend_dsc);
            }
        } else {
            gt_memset(blend_dsc.mask_buf, 0xFF, rd_val);
            area_line.x = tmp.x;
            area_line.y = tmp.y + (rd_val >> 1);
            gt_draw_blend(draw_ctx, &blend_dsc);
        }
        if (0 == diff.x) {
            tmp.y += diff.y;
        } else if (0 == diff.y) {
            tmp.x += diff.x;
        } else {
            if (dir_hor) {
                tmp.x += diff.x;
                tmp.y = (tmp.x - start.x) * distance.y / distance.x + start.y;
            } else {
                tmp.y += diff.y;
                tmp.x = (tmp.y - start.y) * distance.x / distance.y + start.x;
            }
        }
    } while (tmp.y != end.y || tmp.x != end.x);  /** The start or end point are the same, it will draw once. */

    gt_mem_free(blend_dsc.mask_buf);
radius_lb:
    _gt_radius_mask_deinit(&radius_mask);
    draw_ctx->parent_area = remark_area;    /** reset to before value */
}

void gt_draw_catmullrom(_gt_draw_ctx_st * draw_ctx, gt_attr_curve_st const * dsc, const gt_area_st * coords)
{
    if (dsc->line.opa < GT_OPA_MIN) {
        return;
    }
    if (NULL == draw_ctx->buf) {
        return;
    }
    if (!gt_area_is_intersect_screen(&draw_ctx->buf_area, coords)) {
        return;
    }
    gt_radius_mask_st radius_mask = {0};
    uint16_t rd_val = dsc->line.width ? dsc->line.width : 4;
    gt_area_st area_line = { 0, 0, rd_val, rd_val };
    bool is_round = _is_brush_round(&dsc->line);
    if (is_round) {
        if (!_gt_radius_mask_init(&radius_mask, area_line, rd_val >> 1, false)) {
            return;
        }
    }
    gt_area_st * remark_area = draw_ctx->parent_area;
    draw_ctx->parent_area = (gt_area_st * )coords;  /** remark before parent area value, return will reload */

    area_line.x = dsc->p0.x - (rd_val >> 1);     /** move to circle center point */
    area_line.y = dsc->p0.y - (rd_val >> 1);
    area_line.h = 1;

    gt_draw_blend_dsc_st blend_dsc = {
        .dst_area = &area_line,
        .mask_area = &area_line,
        .color_fill = dsc->line.color,
        .opa = dsc->line.opa,
        .mask_buf = gt_mem_malloc(rd_val),
    };
    if (NULL == blend_dsc.mask_buf) {
        goto radius_lb;
    }
    gt_point_f_st remark_prev = { .x = -(1 << ((sizeof(float) << 3) - 2)), .y = -(1 << ((sizeof(float) << 3) - 2)) };
    gt_point_f_st tmp = { .x = area_line.x, .y = area_line.y };
    gt_point_f_st p0 = { .x = tmp.x, .y = tmp.y };
    gt_point_f_st p1 = { .x = dsc->p1.x - (rd_val >> 1), .y = dsc->p1.y - (rd_val >> 1) };
    gt_point_f_st p2 = { .x = dsc->p2.x - (rd_val >> 1), .y = dsc->p2.y - (rd_val >> 1) };
    gt_point_f_st p3 = { .x = dsc->p3.x - (rd_val >> 1), .y = dsc->p3.y - (rd_val >> 1) };
    uint16_t max_hor_range = gt_abs(
        GT_MAX(p0.x, GT_MAX(p1.x, GT_MAX(p2.x, p3.x))) - GT_MIN(p0.x, GT_MIN(p1.x, GT_MIN(p2.x, p3.x)))
    );
    uint16_t max_ver_range = gt_abs(
        GT_MAX(p0.y, GT_MAX(p1.y, GT_MAX(p2.y, p3.y))) - GT_MIN(p0.y, GT_MIN(p1.y, GT_MIN(p2.y, p3.y)))
    );
    for (gt_size_t i = 0, second = 0, cnt = GT_MAX(max_hor_range, max_ver_range) << 1; i < cnt; ++i) {
        tmp = gt_math_catmullrom(1.0 * i / cnt, &p0, &p1, &p2, &p3);
        if (second && gt_abs(remark_prev.x - tmp.x) < 1 && gt_abs(remark_prev.y - tmp.y) < 1) {
            second = 0;
            continue;
        }
        remark_prev = tmp;
        ++second;
        for (gt_size_t i = 0; i < rd_val; ++i) {
            gt_memset(blend_dsc.mask_buf, 0xFF, rd_val);
            if (is_round) {
                _gt_get_radius_mask_buf(blend_dsc.mask_buf, 0, i, area_line.w, &radius_mask);
            }
            area_line.x = (gt_size_t)tmp.x;
            area_line.y = (gt_size_t)tmp.y + i;
            gt_draw_blend(draw_ctx, &blend_dsc);
        }
    }

    gt_mem_free(blend_dsc.mask_buf);
radius_lb:
    _gt_radius_mask_deinit(&radius_mask);
    draw_ctx->parent_area = remark_area;    /** reset to before value */
}

void draw_focus(gt_obj_st* obj, gt_size_t radius)
{
    if (0 == radius) {
        return;
    }
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.bg_opa = obj->opa;
    rect_attr.radius = radius;
    rect_attr.reg.is_fill = 0;
    rect_attr.border_width = 1;
    rect_attr.bg_color = gt_color_focus();
    rect_attr.border_color = gt_color_focus();

    if (obj->focus && GT_ENABLED == obj->focus_dis) {
        draw_bg(obj->draw_ctx, &rect_attr, &obj->area);
    }
}

/* end ------------------------------------------------------------------*/
