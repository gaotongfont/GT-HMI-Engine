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
#include "../others/gt_effects.h"
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
static const char _utf8_dots[] = { 0x2e, 0x2e, 0x2e };
/** Single line omit mode, such as: "。。。" */
static const char _utf8_full_stops[] = { 0xe3, 0x80, 0x82, 0xe3, 0x80, 0x82, 0xe3, 0x80, 0x82 };

static const uint8_t r_1[] = {0xB4,0x2D,0xDC,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0xDC,0x00,};
static const uint8_t r_2[] = {0x00,0xE0,0x50,0xE0,0x00,0x00,0x00,0x00,0x02,0x00,0x04,0x00,0x01,0x00,0x00,0x00,0x00,0x00,};
static const uint8_t r_3[] = {0x00,0xF0,0x80,0x00,0x80,0xF0,0x00,0x00,0x00,0x00,0x02,0x00,0x03,0x00,0x06,0x00,0x02,0x00,0x02,0x00,0x00,0x00,0x00,0x00,};
static const uint8_t r_4[] = {0x00,0xA0,0xF0,0xA0,0x00,0x00,0xAB,0x00,0x90,0x2E,0x00,0x00,0x01,0x00,0x02,0x00,0x03,0x00,0x04,0x00,0x04,0x00,0x03,0x00,0x02,0x00,0x01,0x00,0x00,0x00,};
static const uint8_t r_5[] = {0x00,0xB0,0x40,0x80,0x40,0xB0,0x00,0x00,0x90,0x2E,0xBF,0x00,0x00,0x00,0x01,0x00,0x02,0x00,0x03,0x00,0x04,0x00,0x06,0x00,0x05,0x00,0x04,0x00,0x04,0x00,0x03,0x00,0x01,0x00,0x00,0x00,};
static const uint8_t r_6[] = {0x00,0xC0,0x60,0x00,0xD0,0x10,0xD0,0x00,0x60,0xC0,0x00,0x00,0x04,0x05,0x00,0x00,0x01,0x00,0x02,0x00,0x03,0x00,0x05,0x00,0x07,0x00,0x0A,0x00,0x06,0x00,0x05,0x00,0x05,0x00,0x04,0x00,0x03,0x00,0x01,0x00,0x00,0x00,};
static const uint8_t r_7[] = {0x00,0x00,0xD0,0x80,0x10,0xF0,0x40,0xF0,0x40,0xF0,0x10,0x80,0xD0,0x00,0x00,0x07,0x00,0x00,0x01,0x00,0x03,0x00,0x04,0x00,0x06,0x00,0x08,0x00,0x0A,0x00,0x0D,0x00,0x07,0x00,0x06,0x00,0x06,0x00,0x05,0x00,0x04,0x00,0x03,0x00,0x01,0x00,0x00,0x00,};
static const uint8_t r_8[] = {0x00,0x00,0xD0,0x90,0x20,0x80,0xC0,0x80,0x20,0x90,0xD0,0x00,0x00,0x05,0x06,0x07,0x1F,0x00,0x00,0x00,0x01,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x06,0x00,0x07,0x00,0x08,0x00,0x0B,0x00,0x08,0x00,0x07,0x00,0x07,0x00,0x07,0x00,0x06,0x00,0x05,0x00,0x04,0x00,0x01,0x00,0x00,0x00,};
static const uint8_t r_9[] = {0x00,0x00,0xE0,0xA0,0x40,0x00,0xC0,0x20,0xF0,0x40,0xF0,0x20,0xC0,0x00,0x40,0xA0,0xE0,0x00,0x00,0x1F,0x00,0x00,0x01,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x07,0x00,0x09,0x00,0x0B,0x00,0x0D,0x00,0x11,0x00,0x09,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x07,0x00,0x06,0x00,0x05,0x00,0x04,0x00,0x01,0x00,0x00,0x00,};
static const uint8_t r_10[] = {0x00,0x00,0xE0,0xA0,0x50,0x00,0xE0,0x40,0x80,0x00,0x80,0x40,0xE0,0x00,0x50,0xA0,0xE0,0x00,0x00,0x1F,0x7F,0x7E,0x00,0x00,0x01,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x07,0x00,0x08,0x00,0x09,0x00,0x0B,0x00,0x0D,0x00,0x11,0x00,0x0A,0x00,0x09,0x00,0x09,0x00,0x09,0x00,0x08,0x00,0x08,0x00,0x07,0x00,0x06,0x00,0x04,0x00,0x01,0x00,0x00,0x00,};
static const uint8_t r_11[] = {0x00,0x00,0xE0,0xB0,0x60,0x10,0xF0,0x80,0x00,0xD0,0xE0,0xD0,0x00,0x80,0xF0,0x10,0x60,0xB0,0xE0,0x00,0x00,0xBE,0x00,0x00,0x00,0x00,0x01,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x07,0x00,0x08,0x00,0x0A,0x00,0x0B,0x00,0x0C,0x00,0x0F,0x00,0x13,0x00,0x0B,0x00,0x0A,0x00,0x0A,0x00,0x0A,0x00,0x09,0x00,0x09,0x00,0x08,0x00,0x07,0x00,0x06,0x00,0x04,0x00,0x01,0x00,0x00,0x00,};
static const uint8_t r_12[] = {0x00,0x00,0xE0,0xB0,0x70,0x20,0xA0,0x10,0xF0,0x40,0x70,0x40,0xF0,0x10,0xA0,0x20,0x70,0xB0,0xE0,0x00,0x00,0x56,0x00,0x00,0x50,0x2D,0x00,0x00,0x01,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x06,0x00,0x07,0x00,0x09,0x00,0x0A,0x00,0x0B,0x00,0x0D,0x00,0x0F,0x00,0x13,0x00,0x0C,0x00,0x0B,0x00,0x0B,0x00,0x0B,0x00,0x0B,0x00,0x0A,0x00,0x09,0x00,0x09,0x00,0x08,0x00,0x06,0x00,0x05,0x00,0x01,0x00,0x00,0x00,};
static const uint8_t r_13[] = {0x00,0x00,0xF0,0xC0,0x70,0x20,0x00,0xC0,0x30,0x80,0x00,0xB0,0x10,0xB0,0x00,0x80,0x30,0xC0,0x00,0x20,0x70,0xC0,0xF0,0x00,0x00,0x2D,0xC2,0x00,0x00,0x00,0x01,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x06,0x00,0x08,0x00,0x09,0x00,0x0A,0x00,0x0C,0x00,0x0E,0x00,0x10,0x00,0x12,0x00,0x17,0x00,0x0D,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0B,0x00,0x0B,0x00,0x0A,0x00,0x09,0x00,0x08,0x00,0x07,0x00,0x05,0x00,0x01,0x00,0x00,0x00,};
static const uint8_t r_14[] = {0x00,0x00,0xF0,0xC0,0x80,0x30,0x00,0xE0,0x60,0x00,0xC0,0x20,0xF0,0xF0,0xF0,0x20,0xC0,0x00,0x60,0xE0,0x00,0x30,0x80,0xC0,0xF0,0x00,0x00,0x00,0xC0,0x00,0x00,0x00,0x01,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x06,0x00,0x08,0x00,0x09,0x00,0x0B,0x00,0x0D,0x00,0x0E,0x00,0x0F,0x00,0x11,0x00,0x14,0x00,0x19,0x00,0x0E,0x00,0x0D,0x00,0x0D,0x00,0x0D,0x00,0x0D,0x00,0x0C,0x00,0x0C,0x00,0x0B,0x00,0x0A,0x00,0x09,0x00,0x08,0x00,0x07,0x00,0x05,0x00,0x01,0x00,0x00,0x00,};
static const uint8_t r_15[] = {0x00,0x00,0xF0,0xC0,0x90,0x50,0x00,0xE0,0x80,0x10,0xE0,0x40,0x80,0xA0,0x80,0x40,0xE0,0x10,0x80,0xE0,0x00,0x50,0x90,0xC0,0xF0,0x00,0x00,0x00,0xC0,0x00,0xB1,0x00,0x00,0x00,0x01,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x06,0x00,0x08,0x00,0x09,0x00,0x0B,0x00,0x0C,0x00,0x0D,0x00,0x0E,0x00,0x0F,0x00,0x11,0x00,0x14,0x00,0x19,0x00,0x0F,0x00,0x0E,0x00,0x0E,0x00,0x0E,0x00,0x0E,0x00,0x0D,0x00,0x0D,0x00,0x0C,0x00,0x0C,0x00,0x0B,0x00,0x0A,0x00,0x09,0x00,0x07,0x00,0x05,0x00,0x01,0x00,0x00,0x00,};
static const uint8_t r_16[] = {0x00,0x00,0xC0,0x90,0x50,0x00,0xF0,0x90,0x20,0xF0,0x80,0x00,0xD0,0x10,0xD0,0x20,0xD0,0x10,0xD0,0x00,0x80,0xF0,0x20,0x90,0xF0,0x00,0x50,0x90,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x02,0x00,0x03,0x00,0x04,0x00,0x05,0x00,0x07,0x00,0x08,0x00,0x0A,0x00,0x0B,0x00,0x0D,0x00,0x0F,0x00,0x11,0x00,0x13,0x00,0x16,0x00,0x19,0x00,0x1D,0x00,0x10,0x00,0x10,0x00,0x0F,0x00,0x0F,0x00,0x0F,0x00,0x0E,0x00,0x0E,0x00,0x0D,0x00,0x0D,0x00,0x0C,0x00,0x0B,0x00,0x0A,0x00,0x09,0x00,0x07,0x00,0x05,0x00,0x02,0x00,0x00,0x00,};


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

static const uint8_t * small_radius_make[RADIUS_MAX + 1] = {
    r_1, r_1, r_2, r_3, r_4, r_5, r_6, r_7, r_8, r_9, r_10, r_11, r_12, r_13, r_14, r_15, r_16
};

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

static bool _gt_get_radius_mask( gt_radius_mask_st *r_mask, uint16_t radius) {
    if(!r_mask) return false;

    if(0 == radius){
        return true;
    }

    const uint32_t mask_size = (radius << 1) + 2;

    if(radius <= RADIUS_MAX){
        r_mask->buf = (uint8_t * )small_radius_make[radius];
        r_mask->opa = r_mask->buf;
        r_mask->opa_start = (uint16_t * )(r_mask->buf + mask_size);
        r_mask->x_start = (uint16_t * )(r_mask->buf + mask_size*2);
        return true;
    }

    if(r_mask->buf) {
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
    r_mask->x_start = (uint16_t * )(r_mask->buf + mask_size*2);

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

    return _gt_get_radius_mask( r_mask, radius);
}

static void _gt_radius_mask_deinit(gt_radius_mask_st*r_mask) {
    if(!r_mask) return;

    if(r_mask->buf && r_mask->radius > RADIUS_MAX){
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
    if(!p || !mask_line) return false;

    if(false == p->outer){
        if(y < p->area.y || y >= p->area.y + p->area.h){
            return false;
        }
    }
    else{
        if(y < p->area.y || y >= p->area.y + p->area.h){
            return false;
        }
    }

    if((x >= p->area.x + p->radius && x + len <= p->area.x + p->area.w - p->radius-1)
        || (y >= p->area.y + p->radius && y <= p->area.y + p->area.h - p->radius-1)
    ){
        if(false == p->outer){
            int32_t last = p->area.x - x;
            if(last > len) return false;
            if(last >= 0){
                gt_memset_0(&mask_line[0], last);
            }

            int32_t first = p->area.x + p->area.w - x;
            if(first < 0) return false;
            else if(first < len){
                gt_memset_0(&mask_line[first], len - first);
            }
            return false;
        }
        else{
            int32_t first = p->area.x - x;
            if(first < 0) first = 0;
            if(first <= len) {
                int32_t last = p->area.x + p->area.w - x - first;
                if(first + last > len) last = len - first;
                if(last >= 0){
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
    gt_opa_t* aa_opa = _gt_get_next_line(p, tmp_y, &aa_len, &x_start);
    gt_size_t pos_x_right = k + p->area.w - p->radius + x_start;
    gt_size_t pos_x_left = k + p->radius - x_start - 1;
    gt_size_t i;

    if (false == p->outer) {
        for (i = 0; i < aa_len; i++) {
            tmp_opa = aa_opa[aa_len - i - 1];
            if (pos_x_right + i >= 0 && pos_x_right + i < len) {
                mask_line[pos_x_right + i] = GT_MIN(tmp_opa, mask_line[pos_x_right + i]); //(gt_per_255(tmp_opa) * mask_line[pos_x_right + i]) >> 15;
            }
            if (pos_x_left - i >= 0 && pos_x_left - i < len) {
                mask_line[pos_x_left - i] = GT_MIN(tmp_opa, mask_line[pos_x_left - i]); //(gt_per_255(tmp_opa) * mask_line[pos_x_right - i]) >> 15;
            }
        }

        pos_x_right = GT_MAX(0, (GT_MIN(pos_x_right + aa_len, len)));
        gt_memset_0(&mask_line[pos_x_right], len -pos_x_right );

        pos_x_left = GT_MAX(0, (GT_MIN(pos_x_left - aa_len + 1, len)));
        gt_memset_0(&mask_line[0], pos_x_left);
    } else {
        for (i = 0; i < aa_len; i++) {
            tmp_opa = 255 - aa_opa[aa_len - i - 1];
            if (pos_x_right + i >= 0 && pos_x_right + i < len) {
                mask_line[pos_x_right + i] = GT_MIN(tmp_opa, mask_line[pos_x_right + i]); //(gt_per_255(tmp_opa) * mask_line[pos_x_right + i]) >> 15;
            }
            if (pos_x_left - i >= 0 && pos_x_left - i < len) {
                mask_line[pos_x_left - i] = GT_MIN(tmp_opa, mask_line[pos_x_left - i]); //(gt_per_255(tmp_opa) * mask_line[pos_x_left - i]) >> 15;
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

static _gt_draw_font_res_st _draw_text_convertor_split(
    const gt_font_st * fonts, uint8_t lan,
    _gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st * blend_dsc,
    const gt_attr_font_st * dsc, gt_size_t font_disp_h,
    gt_draw_blend_dsc_st * blend_dsc_style, const gt_area_st * coords, uint8_t flag_ol)
{
    uint32_t idx = 0, uni_or_gbk = 0, tmp = 0;
    _gt_draw_font_res_st ret = {0};
    uint16_t * text = NULL;
    uint8_t * font_buf = NULL;
    text = (uint16_t * )gt_mem_malloc(fonts->len * sizeof(uint16_t));
    if (NULL == text) {
        GT_LOGE(GT_LOG_TAG_GUI, "buf malloc failed, size: %d", fonts->len * sizeof(uint16_t));
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
    uint8_t ret_style = gt_font_type_get(tmp_font.info.style_fl);
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(&tmp_font.info, ret_style, lan, text_len);

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
        .font_option = tmp_font.info.style_fl,
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
        if (STYLE_EN_HINDI == lan) {
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
        if (blend_dsc->dst_area->y + style_space_y + blend_dsc->dst_area->h < blend_dsc->font_limit_area->y ||
            blend_dsc->dst_area->y > (blend_dsc->font_limit_area->y + blend_dsc->font_limit_area->h)) {
            goto _next_word_cvt_lb;
        }
        else if (blend_dsc->dst_area->x + width < blend_dsc->font_limit_area->x ||
            blend_dsc->dst_area->x > (blend_dsc->font_limit_area->x + blend_dsc->font_limit_area->w)) {
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
        if (STYLE_EN_HINDI == lan) {
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
    if (NULL == blend_dsc->dst_buf) {
        GT_LOGW(GT_LOG_TAG_GUI, "Raw data buffer is NULL");
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

    if (is_mask) {
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
            tmp_font->utf8 = &txt[idx];
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
    int8_t offset_y = _gt_font_get_type_group_offset_y(tmp_font.info.style_cn, tmp_font.info.style_en);

    tmp_font.info.gray = dsc->font->info.gray == 0 ? 1 : dsc->font->info.gray;
    gt_size_t font_disp_h = tmp_font.info.size;
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(&tmp_font.info, 2, 0, 0);
    uint16_t data_len = font_size_res.font_per_size;
    uint8_t * mask_line = NULL, * ret_txt = NULL;

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
    uint8_t lan = right_to_left_lan_get(dsc->font->info.style_fl);
    if (lan == STYLE_EN_HEBREW || lan == STYLE_EN_ARABIC) {
        ret_txt = gt_mem_malloc(dsc->font->len);
        if (NULL == ret_txt) {
            GT_LOGE(GT_LOG_TAG_GUI, "ret_txt malloc is failed!!! size = %d", dsc->font->len);
            goto _ret_handler;
        }
        if (gt_right_to_left_handler(&tmp_font, ret_txt, lan)) {
            txt = ret_txt;
        }
    }

    uint8_t * txt_2 = &txt[0];
    gt_draw_blend_dsc_st blend_dsc = {
        .mask_buf   = NULL,
        .dst_area   = &area_font,
        .color_fill = dsc->font_color,
        .opa        = dsc->opa,
        .font_limit_area = (gt_area_st * )view,
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
    gt_font_style_en_cn_et prev_sty = STYLE_UNKNOWN;
    gt_font_style_en_cn_et cur_sty = STYLE_UNKNOWN;
#endif
    bool cov_flag_ol = false;
    bool start_x_flag = dsc->reg.enabled_start;
    uint16_t bidi_len = 0, bidi_max = 1, bidi_pos = 0;
    gt_bidi_st * bidi = (gt_bidi_st * )gt_mem_malloc(bidi_max * sizeof(gt_bidi_st));
    _gt_font_dot_ret_st dot_ret = {0};
    if (NULL == bidi) {
        GT_LOGE(GT_LOG_TAG_GUI, "bidi malloc is failed!!! size = %d", bidi_max * sizeof(gt_bidi_st));
        goto _ret_handler;
    }
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
        tmp_font.utf8 = &txt[idx];
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
            if (_gt_font_is_convertor_language(lan)) {
                if (bidi_max <= bidi_len) {
                    ++bidi_max;
                    bidi = (gt_bidi_st * )gt_mem_realloc(bidi, bidi_max * sizeof(gt_bidi_st));
                }
                bidi[bidi_len].idx = idx_len;
                bidi[bidi_len].len = idx_step;
                bidi[bidi_len].flag = lan;
                ++bidi_len;
            }
            idx += idx_step;
            idx_len += idx_step;

            if (idx < txt_len) {
                continue;
            }
        }

    _disp_font:
        if (_gt_font_is_convertor_language(lan)) {
            if (bidi_max <= bidi_len) {
                ++bidi_max;
                bidi = (gt_bidi_st * )gt_mem_realloc(bidi, bidi_max * sizeof(gt_bidi_st));
            }
            bidi[bidi_len].idx = idx_len;
            bidi[bidi_len].flag = lan;

            if (over_length < 2 && idx < txt_len) {
                bidi[bidi_len].len = idx_step;
                idx_len = idx_len + idx_step;
            } else {
                bidi[bidi_len].len = lan_len;
                idx_len = lan_len;
                disp_w = 0;
            }
            ++bidi_len;
        } else if (over_length < 2) {
            idx_len = (idx < txt_len) ? idx_len + idx_step : idx_len;
        } else {
            idx_len = over_length_offset;
            disp_w = 0;
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
        bidi_pos = 0;
        area_font.h = font_disp_h;
        while (idx_2 < idx_len) {
            if (bidi_pos < bidi_len && idx_2 == bidi[bidi_pos].idx) {
                tmp_font.utf8 = (char * )&txt_2[idx_2];
                tmp_font.len = bidi[bidi_pos].len;
                tmp_ret = _draw_text_convertor_split(&tmp_font, bidi[bidi_pos].flag,
                                            draw_ctx, &blend_dsc, dsc, font_disp_h,
                                            &blend_dsc_style, coords,
                                            (over_length < 2 ? 0 : 1));
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
                if (tmp_ret.touch_word.word_p) {
                    ret.touch_word = tmp_ret.touch_word;
                }
#endif
                ++bidi_pos;
                idx_2 += tmp_font.len;
                continue;
            }

            gt_memset(tmp_font.res, 0, data_len);
            tmp_font.info.size = dsc->font->info.size;
            tmp_font.utf8 = (char * )&txt_2[idx_2];
            tmp_font.len = gt_font_one_char_code_len_get(&txt_2[idx_2], &uni_or_gbk, tmp_font.info.encoding);

            uint8_t width = gt_font_get_one_word_width(uni_or_gbk, &tmp_font);

            if (_is_align_reverse(dsc->align)) {
                area_font.x -= width;
            }

            if (area_font.y + per_line_h < view->y || area_font.y > (view->y + view->h)) {
                goto next_word_multi_lb;
            }
            else if (area_font.x + width < view->x || area_font.x > (view->x + view->w)) {
                goto next_word_multi_lb;
            }
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
            if (dsc->reg.touch_point) {
                cur_sty = _gt_is_style_cn_or_en(uni_or_gbk, tmp_font.info.encoding);
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
                if (STYLE_CN == cur_sty && dsc->reg.single_cn) {
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
                if (STYLE_EN_ASCII == _gt_is_style_cn_or_en(uni_or_gbk, tmp_font.info.encoding)) {
                    area_font.y += offset_y;
                }
                gt_draw_blend_text(draw_ctx, &blend_dsc, tmp_font.info.size,
                                    byte_width, font_gray, (const uint8_t*)tmp_font.res);
                if (STYLE_EN_ASCII == _gt_is_style_cn_or_en(uni_or_gbk, tmp_font.info.encoding)) {
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

        if (_gt_font_is_convertor_language(lan) && over_length > 1) {
            idx += idx_len;
            cov_flag_ol = true;
            if (_is_align_reverse(dsc->align)) {
                disp_w = area_font.x - coords->x;
            } else {
                disp_w = coords->w - (area_font.x - coords->x);
            }
        } else {
            cov_flag_ol = false;
            idx += over_length < 2 ? idx_step : over_length_offset;
            if (idx < txt_len) {
                area_font.y += area_font.h + dsc->space_y + (style_space_y << 1);
            }
            else if (0x0A == txt[txt_len - 1]) {
                area_font.y += area_font.h + dsc->space_y + (style_space_y << 1);
                if (_is_align_reverse(dsc->align)) {
                    area_font.x = (coords->x + coords->w) - (area_font.x - coords->x);
                } else {
                    area_font.x = _get_align_start_x(dsc->align, coords->x, disp_w, 0);
                }
            }
            disp_w = coords->w;
        }
        idx_len = 0;
        txt_2 = &txt[idx];
        bidi_len = 0;
        font_disp_h = tmp_font.info.size;
        area_font.h = tmp_font.info.size;
        area_font.w = tmp_font.info.size;
    }

_ret_handler:
    if (NULL != bidi) {
        gt_mem_free(bidi);
        bidi = NULL;
    }
    if (NULL != ret_txt) {
        gt_mem_free(ret_txt);
        ret_txt = NULL;
    }
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
    tmp_font->len = gt_font_one_char_code_len_get((char * )&_utf8_dots[0], &uni_or_gbk, tmp_font->info.encoding);
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
    tmp_font->len = gt_font_one_char_code_len_get((char * )&_utf8_full_stops[0], &uni_or_gbk, tmp_font->info.encoding);
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
    int8_t offset_y = _gt_font_get_type_group_offset_y(tmp_font.info.style_cn, tmp_font.info.style_en);

    tmp_font.info.gray = dsc->font->info.gray == 0 ? 1 : dsc->font->info.gray;
    gt_size_t font_disp_h = tmp_font.info.size;
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(&tmp_font.info, 2, 0, 0);
    uint16_t data_len = font_size_res.font_per_size;
    uint8_t * mask_line = NULL, * ret_txt = NULL;

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
    uint8_t lan = right_to_left_lan_get(dsc->font->info.style_fl);
    if (lan == STYLE_EN_HEBREW || lan == STYLE_EN_ARABIC) {
        ret_txt = gt_mem_malloc(dsc->font->len);
        if (NULL == ret_txt) {
            GT_LOGE(GT_LOG_TAG_GUI, "ret_txt malloc is failed!!! size = %d", dsc->font->len);
            goto ret_handler_lb;
        }
        if (gt_right_to_left_handler(&tmp_font, ret_txt, lan)) {
            txt = ret_txt;
        }
    }

    gt_draw_blend_dsc_st blend_dsc = {
        .mask_buf   = NULL,
        .dst_area   = &area_font,
        .color_fill = dsc->font_color,
        .opa        = dsc->opa,
        .font_limit_area = (gt_area_st * )view,
    };
    gt_draw_blend_dsc_st blend_dsc_style = blend_dsc;
    blend_dsc_style.color_fill  = gt_color_black();
    blend_dsc_style.mask_buf    = mask_line;
    uint8_t style_space_y = _get_style_space_y(dsc, tmp_font.info.size);

#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
    gt_font_touch_word_st tmp_word = {0};
    gt_font_style_en_cn_et prev_sty = STYLE_UNKNOWN;
    gt_font_style_en_cn_et cur_sty = STYLE_UNKNOWN;
#endif
    gt_size_t string_total_width = 0, view_width = coords->w;
    uint32_t uni_or_gbk = 0, ret_w = 0, lan_len = 0;
    uint16_t idx = 0, idx_step = 0, idx_len = 0, txt_len = dsc->font->len;
    uint16_t over_length_offset = ((coords->w / ((tmp_font.info.size >> 1) + dsc->space_x) - 1) >> 1) << 1;
    uint8_t over_length = 0;
    _omit_info_st omit_info = {0};
    bool start_x_flag = dsc->reg.enabled_start;
    uint16_t bidi_len = 0, bidi_max = 1, bidi_pos = 0;
    gt_bidi_st * bidi = (gt_bidi_st * )gt_mem_malloc(bidi_max * sizeof(gt_bidi_st));
    if (NULL == bidi) {
        GT_LOGE(GT_LOG_TAG_GUI, "bidi malloc is failed!!! size = %d", bidi_max * sizeof(gt_bidi_st));
        goto ret_handler_lb;
    }
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
        string_total_width += omit_info.width;
        view_width -= omit_info.width;
        if (view_width < tmp_font.info.size >> 1) {
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
        tmp_font.utf8 = &txt[idx];
        tmp_font.len = txt_len - idx;
        idx_step = gt_font_split(&tmp_font, view_width, coords->w, dsc->space_x, &ret_w, &lan, &lan_len);

        if (0 == idx_step || 0 == ret_w) {
            ++idx;
            continue;
        }

        if (dsc->reg.omit_line) {
            if (view_width <= ret_w) {
                break;
            }
            view_width -= ret_w;
        }
        if (idx_step) {
            string_total_width += ret_w;
        }
        if (_gt_font_is_convertor_language(lan)) {
            if (bidi_max <= bidi_len) {
                ++bidi_max;
                bidi = (gt_bidi_st * )gt_mem_realloc(bidi, bidi_max * sizeof(gt_bidi_st));
            }
            bidi[bidi_len].idx = idx_len;
            bidi[bidi_len].len = idx_step;
            bidi[bidi_len].flag = lan;
            ++bidi_len;
        }
        idx += idx_step;
        idx_len += idx_step;
    }
    ret.size.x = string_total_width;
    if (string_total_width > view->w) {
        area_font.x = coords->x;
    } else {
        /** Area can display all content, no need for omit width */
        string_total_width -= omit_info.width;
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
    _gt_font_dot_ret_st dot_ret = {0};
    uint8_t width, byte_width, font_gray;
    while (idx < idx_len) {
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

        gt_memset(tmp_font.res, 0, data_len);
        tmp_font.info.size = dsc->font->info.size;
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
            cur_sty = _gt_is_style_cn_or_en(uni_or_gbk, tmp_font.info.encoding);
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
            if (STYLE_CN == cur_sty && dsc->reg.single_cn) {
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
            if (STYLE_EN_ASCII == _gt_is_style_cn_or_en(uni_or_gbk, tmp_font.info.encoding)) {
                area_font.y += offset_y;
            }
            gt_draw_blend_text(draw_ctx, &blend_dsc, tmp_font.info.size,
                                byte_width, font_gray, (const uint8_t*)tmp_font.res);
            if (STYLE_EN_ASCII == _gt_is_style_cn_or_en(uni_or_gbk, tmp_font.info.encoding)) {
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

    if (dsc->reg.omit_line && idx_len < txt_len) {
        // Display "..." or "。。。"
        tmp_font.utf8 = omit_info.utf8;
        tmp_font.len = gt_font_one_char_code_len_get(omit_info.utf8, &uni_or_gbk, tmp_font.info.encoding);
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
    if (NULL != bidi) {
        gt_mem_free(bidi);
        bidi = NULL;
    }
    if (NULL != ret_txt) {
        gt_mem_free(ret_txt);
        ret_txt = NULL;
    }
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

/* global functions / API interface -------------------------------------*/

#if 0
void gt_draw_rect(_gt_draw_ctx_st * ctx, gt_attr_rect_st * attr, gt_area_st * coords) {
#if GT_DRAW_COMPLEX
    draw_shadow(draw_ctx, dsc, coords);
#endif

    // draw_bg(draw_ctx, dsc, coords);
    // draw_bg_img(draw_ctx, dsc, coords);
    // draw_border(draw_ctx, dsc, coords);

    // draw_outline(draw_ctx, dsc, coords);
}
static const uint8_t radius1[1] = {0XBD};
static const uint8_t radius2[4] = {0X24,0X69,0X69,0xFF};
static const uint8_t radius3[9] = {0X00,0X24,0X69,0X24,0X6D,0X6D,0X69,0X6D,0X6D};
static const uint8_t radius4[16] = {0X00,0X00,0X49,0XAF,0X00,0X6D,0XFF,0XFF,0X49,0XFF,0XFF,0xFF,0XAF,0XFF,0XFF,0XFF};
static const uint8_t radius5[25] = {0X00,0X00,0X49,0XB6,0XFF,0X00,0X92,0XFF,0XFF,0XFF,0X49,0XFF,0XFF,0XFF,0XFF,0XB6,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF};
static const uint8_t radius6[36] = {0X00,0X00,0X00,0X6D,0XDB,0XFF,0X00,0X00,0XDB,0XFF,0XFF,0XFF,0X00,0XDB,0XFF,0XFF,0XFF,0XFF,0X6D,0XFF,0XFF,0XFF,0XFF,0XFF,0XDB,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF};
static const uint8_t radius7[49] = {0X00,0X00,0X00,0X00,0X92,0XDB,0XFF,0X00,0X00,0X49,0XFF,0XFF,0XFF,0XFF,0X00,0X49,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X92,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XDB,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF};
static const uint8_t radius8[64] = {0X00,0X00,0X00,0X00,0X24,0X92,0XDB,0XFF,0X00,0X00,0X00,0X92,0XFF,0XFF,0XFF,0XFF,0X00,0X00,0XB6,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X92,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X24,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X92,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XDB,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF};
static const uint8_t radius9[81] = {0X00,0X00,0X00,0X00,0X00,0X49,0X92,0XDB,0XFF,0X00,0X00,0X00,0X24,0XB6,0XFF,0XFF,0XFF,0XFF,0X00,0X00,0X24,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X24,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0XDB,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X49,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X92,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XDB,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF};
static const uint8_t radius10[100] = {0X00,0X00,0X00,0X00,0X00,0X00,0X49,0XB6,0XDB,0XFF,0X00,0X00,0X00,0X00,0X49,0XDB,0XFF,0XFF,0XFF,0XFF,0X00,0X00,0X00,0X92,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X00,0X92,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X49,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X49,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XB6,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF};
static const uint8_t radius11[121] = {0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X6D,0XB6,0XFF,0XFF,0X00,0X00,0X00,0X00,0X00,0X6D,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X00,0X00,0X24,0XDB,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X00,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X00,0XDB,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X6D,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X6D,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XB6,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF};
static const uint8_t radius12[144] = {0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X6D,0XB6,0XFF,0XFF,0X00,0X00,0X00,0X00,0X00,0X00,0XB6,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X00,0X00,0X00,0X49,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X00,0X00,0X6D,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X00,0X49,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0XB6,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0X6D,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XB6,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF};
typedef struct _gt_radius_dat
{
    /* data */
    uint8_t radius;
    const uint8_t * ptr;
    uint8_t size;
}_gt_radius_dat;

_gt_radius_dat radius_dat[] = {
    {0, radius1, 1},
    {1, radius1, 1},
    {2, radius2, 4},
    {3, radius3, 9},
    {4, radius4, 16},
    {5, radius5, 25},
    {6, radius6, 36},
    {7, radius7, 49},
    {8, radius8, 64},
    {9, radius9, 81},
    {10, radius10, 100},
    {11, radius11, 121},
    {12, radius12, 144}
};
#endif

void gt_draw_arch(_gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st  *blend_dsc,uint8_t *mask_left_up,uint8_t *mask_right_up,const gt_area_st * coords)
{
    gt_size_t x = blend_dsc->mask_area->x;
    gt_radius_t radius = blend_dsc->mask_area->w;
    uint8_t * mask_right_down = mask_left_up;
    uint8_t * mask_left_down = mask_right_up;

    // draw whole circle
    if(radius < GT_RADIUS_MIN){ //data not need flip
        gt_draw_blend(draw_ctx, blend_dsc);
        blend_dsc->mask_area->x += coords->w - radius;
        gt_draw_blend(draw_ctx, blend_dsc);
        blend_dsc->mask_area->y += coords->h - radius;
        gt_draw_blend(draw_ctx, blend_dsc);
        blend_dsc->mask_area->x = x;
        gt_draw_blend(draw_ctx, blend_dsc);
        return;
    }
    // 1:draw left-up arch
    gt_draw_blend(draw_ctx, blend_dsc);

    // 2:draw right_up arch
    gt_mirror_hor_u8(mask_right_up, mask_left_up, radius, radius);
    blend_dsc->mask_area->x += coords->w - radius;
    blend_dsc->mask_buf = mask_right_up;
    gt_draw_blend(draw_ctx, blend_dsc);

    // 3:draw right_down arch
    gt_mirror_ver_u8(mask_right_down, mask_right_up, radius, radius);
    blend_dsc->mask_area->y += coords->h - radius;
    blend_dsc->mask_buf = mask_right_down;
    gt_draw_blend(draw_ctx, blend_dsc);

    // 4:draw left_down arch
    gt_mirror_hor_u8(mask_left_down, mask_right_down, radius, radius);
    blend_dsc->mask_area->x = x;
    blend_dsc->mask_buf = mask_left_down;
    gt_draw_blend(draw_ctx, blend_dsc);

    //Recovery Data
    gt_mirror_ver_u8(mask_left_up, mask_left_down, radius, radius);
    blend_dsc->mask_buf = mask_left_up;
}

void gt_draw_line(gt_attr_line_st * line_attr, _gt_draw_ctx_st * draw_ctx,
                    gt_draw_blend_dsc_st * blend_dsc,
                    uint8_t *mask_left_up, uint8_t *mask_right_up,
                    const gt_area_st * coords)
{
	gt_size_t x1,x2,y1,y2;		//two point get a line

	x1 = line_attr->x_1;
	y1 = line_attr->y_1;

	x2 = line_attr->x_2;
	y2 = line_attr->y_2;

	gt_size_t dx =  abs(x2-x1), sx = x1<x2 ? 1 : -1;
	gt_size_t dy = -abs(y2-y1), sy = y1<y2 ? 1 : -1;
	gt_size_t err = dx+dy, e2; /* error value e_xy */

	while (1) {  /* loop */
		if (x1==x2 && y1==y2){
			break;
		}
		e2 = err << 1;

		if (e2 >= dy) { /* e_xy+e_x > 0 */
			err += dy;
			x1 += sx;
		}
		if (e2 <= dx) {/* e_xy+e_y < 0 */
			err += dx;
			y1 += sy;
		}
        blend_dsc->mask_area->x = x1;
        blend_dsc->mask_area->y = y1;
        gt_draw_arch(draw_ctx, blend_dsc, mask_left_up, mask_right_up, coords);
	}
}

#if 0
static void get_graph_circle_ring_buff(uint16_t radius,uint16_t border_width,uint8_t * mask_left_up,uint8_t * mask_temp){
    gt_size_t radius_hid = radius - border_width;
    if( radius_hid >= 3){
        uint8_t * mask_hid = gt_mem_malloc( radius_hid * radius_hid );
        gt_memcpy(mask_hid, radius_dat[radius_hid].ptr, radius_dat[radius_hid].size);

        gt_size_t bidx = 0, sidx = 0;
        gt_size_t by = radius;
        for( gt_size_t sy = radius_hid - 1; sy > 0; sy-- ){
            bidx = (by * radius) - 1;
            sidx = (sy * radius_hid) -1;
            for( gt_size_t sx = radius_hid-1; sx >= 0; sx--){
                if( mask_hid[sidx] != 0 ){
                    mask_temp[sidx + radius_hid] = mask_left_up[bidx];
                    mask_left_up[ bidx-- ] = 0;
                    sidx--;
                }
            }
            by--;
        }
        gt_mem_free(mask_hid);
    }else if( radius_hid == 2){
        mask_temp[3] = mask_left_up[ radius * radius - 1];
        mask_temp[2] = mask_left_up[ radius * radius - 2];
        mask_temp[1] = mask_left_up[ (radius-1) * radius - 1];
        mask_temp[0] = mask_left_up[ (radius-1) * radius - 2];
        mask_left_up[ radius * radius - 1 ] = 0;
        mask_left_up[ radius * radius - 2 ] = 0;
        mask_left_up[ (radius-1) * radius - 1 ] = 0;
        mask_left_up[ (radius-1) * radius - 2 ] = 0;
    }else if( radius_hid == 1){
        mask_temp[0] = mask_left_up[ radius * radius - 1 ];
        mask_left_up[ radius * radius - 1 ] = 0;
    }
}
#endif

void draw_bg(_gt_draw_ctx_st * draw_ctx, const gt_attr_rect_st * dsc, const gt_area_st * coords)
{
    if (dsc->bg_opa <= GT_OPA_MIN) return;

    if( !gt_area_is_intersect_screen(&draw_ctx->buf_area, coords) ){
        return;
    }

    gt_draw_blend_dsc_st blend_dsc = {0};
    blend_dsc.color_fill = dsc->bg_color;
    blend_dsc.opa = dsc->bg_opa;
    blend_dsc.mask_buf = NULL;

    // Dot matrix data
    if(dsc->gray != 0) {
        if(!dsc->data_gray) {
            return ;
        }
        uint8_t * mask_buf = NULL;
        uint32_t len = coords->w * coords->h, index = 0, buf_idx = 0;
        char idx_bit = 0;
        mask_buf = gt_mem_malloc(len);
        if (!mask_buf) {
            return ;
        }

        gt_area_st area_gy = {0};
        gt_area_copy(&area_gy, coords);

        blend_dsc.mask_buf = mask_buf;
        blend_dsc.dst_area = &area_gy;
        blend_dsc.mask_area = &area_gy;

        len = len >> 3;
        buf_idx = 0;
        index = 0;
        while ( index < len ) {
            for (idx_bit = 0; idx_bit < 8; idx_bit++) {
                if( (dsc->data_gray[index] << idx_bit )&0x80 ){
                    mask_buf[buf_idx++] = 0xFF;
                }else{
                    mask_buf[buf_idx++] = 0x00;
                }
            }
            index++;
        }
        gt_draw_blend(draw_ctx, &blend_dsc);

        if(NULL != mask_buf) {
            gt_mem_free(mask_buf);
        }
        mask_buf = NULL;
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
            if(!_gt_radius_mask_init(&r_bg_radius, area_bg, r_bg, false)){
                GT_LOGE( GT_LOG_TAG_GUI,"draw bg radius init fail!!! radius = %d", r_bg);
                return ;
            }

            gt_radius_mask_st r_cl_radius = {0};
            if (dsc->base_area) {
                uint16_t r_cl = GT_MIN(dsc->radius, (GT_MIN(dsc->base_area->w >> 1, dsc->base_area->h >> 1)));
                if(!_gt_radius_mask_init(&r_cl_radius, *dsc->base_area, r_cl, false)){
                    _gt_radius_mask_deinit(&r_bg_radius);
                    GT_LOGE( GT_LOG_TAG_GUI,"draw clipped radius init fail!!! radius = %d", r_cl);
                    return ;
                }
            }

            gt_opa_t* mask_bg_line = gt_mem_malloc(area_bg.w);
            if (NULL == mask_bg_line) {
                if(dsc->base_area) {
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

        for(gt_size_t i = 0; i < area_border.h; i++){
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


#if 0
    static uint16_t radius_pre = 0;
    static gt_color_t * color_circle_p = NULL;
    static uint8_t * mask_left_up = NULL;

    // calc radius
    uint16_t radius = GT_MIN(coords->w >> 1, coords->h >> 1);
    radius = GT_MIN(radius, dsc->radius);
    uint16_t border_width = dsc->border_width;
    uint8_t is_radius_more_than_border = radius > border_width ? true : false;

    gt_area_st area_mask = {
        .x = coords->x,
        .y = coords->y,
        .w = radius,
        .h = radius,
    };

    gt_draw_blend_dsc_st blend_dsc = {
        .opa        = dsc->bg_opa,
        .color_fill = dsc->bg_color,
        .mask_area  = &area_mask,
        .dst_area   = &area_mask
    };


    // Dot matrix data
    if(dsc->gray != 0)
    {
        if(!dsc->data_gray)
        {
            return;
        }
        uint8_t * mask_buf = NULL;
        uint32_t len = coords->w * coords->h, index = 0, buf_idx = 0;
        char idx_bit = 0;
        mask_buf = gt_mem_malloc(len);
        if(!mask_buf){
            return;
        }
        blend_dsc.mask_buf = mask_buf;
        blend_dsc.mask_area->w = coords->w;
        blend_dsc.mask_area->h = coords->h;
        // GT_LOGV(GT_LOG_TAG_GUI, "dsc->gray = %d, w = %d, h = %d, buf_size = %d",dsc->gray, blend_dsc.mask_area->w, blend_dsc.mask_area->h, len);

        len = len >> 3;
        buf_idx = 0;
        index = 0;
        while( index < len ){

            for(idx_bit = 0; idx_bit < 8; idx_bit++) {
                if( (dsc->data_gray[index] << idx_bit )&0x80 ){
                    mask_buf[buf_idx++] = 0xFF;
                }else{
                    mask_buf[buf_idx++] = 0x00;
                }
            }
            index++;
        }
        gt_draw_blend(draw_ctx, &blend_dsc);

        if(NULL != mask_buf)
        {
            gt_mem_free(mask_buf);
        }
        mask_buf = NULL;
        return;
    }

#if 0
    /* draw circle or draw a line*/
    if(radius > 0 ){
        uint32_t mem_size = (radius << 1) * (radius << 1) * sizeof(gt_color_t);
        if( radius != radius_pre ){
            radius_pre = radius;
            /*free*/
            if(NULL != color_circle_p){
                gt_mem_free(color_circle_p);
                color_circle_p = NULL;
            }
            if(NULL != mask_left_up){
                gt_mem_free(mask_left_up);
                mask_left_up = NULL;
            }
            /*malloc*/
            color_circle_p = gt_mem_malloc(mem_size);
            mask_left_up = gt_mem_malloc(radius * radius);
            if (NULL == color_circle_p) {
                gt_mem_free(mask_left_up);
                radius_pre = 0;
                return;
            }
            if (NULL == mask_left_up) {
                gt_mem_free(color_circle_p);
                radius_pre = 0;
                return;
            }
        }
        if(radius > 12 ){
            gt_attr_circle_st circle_attr = {
                .fg_color       = gt_color_white(),
                .bg_color       = gt_color_black(),
                .radius         = radius,
                .border_width   = border_width,
                .border_color   = gt_color_white(),
                .reg.is_fill    = dsc->reg.is_fill,
            };

            gt_area_st circle_area = {
                .w = radius << 1,
                .h = radius << 1
            };
            uint32_t idx = 0, idx_circle = 0;
            uint16_t x, y;
            uint8_t need_draw_circle = false;
            if(false == is_radius_more_than_border){    /* draw circle with border_color*/
                blend_dsc.color_fill = dsc->border_color;
                circle_attr.reg.is_fill = true;
                need_draw_circle = true;
            }else{
                if(dsc->reg.is_fill == true){   /* draw circle with fill bg_color*/
                    circle_attr.reg.is_fill = true;
                    need_draw_circle = true;
                }
                if(border_width != 0){ //need draw border ring with border_color
                    if(need_draw_circle == true){   //have
                        // get circle mask
                        gt_graph_circle(&circle_attr, &circle_area, color_circle_p);
                        // get circle left mask
                        for (y = 0; y < radius; y++) {
                            for (x = 0; x < radius; x++) {
                                mask_left_up[idx++] = gt_color_brightness(color_circle_p[idx_circle++]);
                            }
                            idx_circle += radius;
                        }
                        blend_dsc.mask_buf = mask_left_up;
                        gt_draw_arch(draw_ctx,&blend_dsc,mask_left_up,(uint8_t *)color_circle_p,coords);
                        need_draw_circle = false;
                    }

                    blend_dsc.mask_area->x = coords->x;
                    blend_dsc.mask_area->y = coords->y;
                    blend_dsc.color_fill = dsc->border_color;
                    circle_attr.reg.is_fill = false;
                    need_draw_circle = true;
                }
            }
            if (true == need_draw_circle){
                // get circle mask
                gt_graph_circle(&circle_attr, &circle_area, color_circle_p);
                // get circle left mask
                idx = 0;
                idx_circle = 0;
                for (y = 0; y < radius; y++) {
                    for (x = 0; x < radius; x++) {
                        mask_left_up[idx++] = gt_color_brightness(color_circle_p[idx_circle++]);
                    }
                    idx_circle += radius;
                }
                blend_dsc.mask_buf = mask_left_up;
                gt_draw_arch(draw_ctx,&blend_dsc,mask_left_up,(uint8_t *)color_circle_p,coords);
                need_draw_circle = false;
            }
        }else{
            gt_memcpy(mask_left_up, radius_dat[radius].ptr, radius_dat[radius].size);
            blend_dsc.mask_buf = mask_left_up;
            if(false == is_radius_more_than_border){    /* draw all circle with border_color*/
                blend_dsc.color_fill = dsc->border_color;
                gt_draw_arch(draw_ctx,&blend_dsc,mask_left_up,(uint8_t *)color_circle_p,coords);
            }else if(border_width == 0 && dsc->reg.is_fill == true){     /* draw all circle with fill bg_color*/
                gt_draw_arch(draw_ctx,&blend_dsc,mask_left_up,(uint8_t *)color_circle_p,coords);
            }else {
                if(border_width != 0){ /* draw border ring with border_color */
                    blend_dsc.color_fill = dsc->border_color;
                    uint16_t radius_hid = radius - border_width;
                    uint8_t * mask_temp = gt_mem_malloc(radius_hid * radius_hid);
                    if(NULL == mask_temp){
                        return;
                    }
                    get_graph_circle_ring_buff(radius,border_width,mask_left_up,mask_temp);
                    gt_draw_arch(draw_ctx,&blend_dsc,mask_left_up,(uint8_t *)color_circle_p,coords);

                    if(dsc->reg.is_fill == true){   /* draw circle remove ring with fill bg_color*/
                        blend_dsc.mask_area->x = coords->x + border_width;
                        blend_dsc.mask_area->y = coords->y + border_width;
                        blend_dsc.mask_area->w = radius_hid;
                        blend_dsc.mask_area->h = radius_hid;
                        blend_dsc.mask_buf = mask_temp;
                        blend_dsc.color_fill = dsc->bg_color;
                        gt_area_st area_mask_temp = {
                            .x = blend_dsc.mask_area->x,
                            .y = blend_dsc.mask_area->y,
                            .w = coords->w - (border_width << 1),
                            .h = coords->h - (border_width << 1),
                        };
                        gt_draw_arch(draw_ctx,&blend_dsc,mask_temp,(uint8_t *)color_circle_p,&area_mask_temp);
                    }
                    gt_mem_free(mask_temp);
                }
            }
        }

        if(dsc->reg.is_line) {  /* draw line */
            gt_draw_line(dsc->line,draw_ctx, &blend_dsc,mask_left_up,(uint8_t *)color_circle_p,coords);
        }
    }


#endif

    if(radius > 0)
    {
        if(!_gt_draw_radius( draw_ctx, dsc, coords )){
            radius = 0;
        }
    }

    blend_dsc.mask_buf      = NULL;
    blend_dsc.dst_buf       = NULL;
    /* draw border */
    if (border_width > 0) {
        blend_dsc.color_fill    = dsc->border_color;

        // 1:draw up border
        area_mask.x = coords->x + radius;
        area_mask.y = coords->y;
        area_mask.w = coords->w - (radius << 1);
        area_mask.h = border_width;
        gt_draw_blend(draw_ctx, &blend_dsc);

        // 2:draw bottom border
        area_mask.y = (coords->y + coords->h) - border_width;
        gt_draw_blend(draw_ctx, &blend_dsc);

        // 3 draw left border
        area_mask.x = coords->x;
        area_mask.y = coords->y + radius;
        area_mask.w = (false == is_radius_more_than_border) ? radius : border_width;
        area_mask.h = coords->h - (radius << 1);
        gt_draw_blend(draw_ctx, &blend_dsc);

        // 4 draw right border
        area_mask.x = (coords->x + coords->w) - (false == is_radius_more_than_border ? radius : border_width);
        gt_draw_blend(draw_ctx, &blend_dsc);

        if(false == is_radius_more_than_border){
            area_mask.x = coords->x + radius;
            area_mask.y = coords->y + border_width;
            area_mask.w = border_width - radius;
            area_mask.h = coords->h - (border_width << 1);
            gt_draw_blend(draw_ctx, &blend_dsc);

            area_mask.x = (coords->x + coords->w) - border_width;
            gt_draw_blend(draw_ctx, &blend_dsc);
        }

    }

    /* draw rect */
    if (dsc->reg.is_fill) {
        blend_dsc.color_fill = dsc->bg_color;

        if(true == is_radius_more_than_border) {
            // 1:draw up rect
            area_mask.x = coords->x + radius;
            area_mask.y = coords->y + border_width;
            area_mask.w = coords->w - (radius << 1);
            area_mask.h = radius - border_width;
            gt_draw_blend(draw_ctx, &blend_dsc);

            // 2:draw bottom rect
            area_mask.y = coords->y + coords->h - radius;
            gt_draw_blend(draw_ctx, &blend_dsc);
        }

        // 3:draw center rect
        area_mask.x = coords->x + border_width;
        area_mask.y = coords->y + (true == is_radius_more_than_border ? radius : border_width);
        area_mask.w = coords->w - (border_width<<1);
        area_mask.h = coords->h - (true == is_radius_more_than_border ? (radius << 1) : (border_width << 1));
        gt_draw_blend(draw_ctx, &blend_dsc);
    }
#endif

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
    if( dsc->bg_opa < GT_OPA_MIN ) return;

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
    }
    else if (GT_RES_FAIL == gt_img_decoder_open(&dsc_img, dsc->bg_img_src)) {
        GT_LOGW(GT_LOG_TAG_DATA, "[%s] Open image decoder failed", dsc->bg_img_src);
        return;
    }
    coords->w = dsc_img.header.w;
    coords->h = dsc_img.header.h;
    _gt_disp_update_max_area(coords, true);

    gt_area_st dst_area = *coords, mask_area = *coords;
    gt_draw_blend_dsc_st blend_dsc = {
        .dst_buf = NULL,
        .mask_buf = NULL,
        .dst_area = &dst_area,
        .mask_area = &mask_area,
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

    if (dsc_img.decoder) {
        gt_img_decoder_close(&dsc_img);
    }
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
