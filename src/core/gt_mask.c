/**
 * @file gt_mask.c
 * @author Yang
 * @brief
 * @version 0.1
 * @date 2024-11-18 15:02:53
 * @copyright Copyright (c) 2014-2024, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "gt_mask.h"
#include "gt_obj_pos.h"
#include "gt_mem.h"
#include "../others/gt_area.h"
/* private define -------------------------------------------------------*/
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


#define _GT_CIRCLE_MAX_COUNT   1000
#define _CIRCLE_CACHE_AGING(count, r)   count = GT_MIN(count + (r < 16 ? 1 : (r >> 4)), _GT_CIRCLE_MAX_COUNT)
/* private typedef ------------------------------------------------------*/


typedef struct _gt_mask_t {
    void* p_mask;
}_gt_mask_st;


/* static prototypes ----------------------------------------------------*/



/* static variables -----------------------------------------------------*/
static _gt_mask_st _gt_mask_list[_GT_MASK_MAX_NUM] = {0};
static _gt_mask_radius_circle_st _gt_circle_list[_GT_MASK_CIRCLE_MAX_NUM] = {0};


#if 0
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
#endif

/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
static GT_ATTRIBUTE_RAM_TEXT void _gt_circ_init(gt_point_st * c, gt_size_t * tmp, uint16_t radius) {
    c->x = radius;
    c->y = 0;
    *tmp = 1 - radius;
}

static GT_ATTRIBUTE_RAM_TEXT bool _gt_circ_cont(gt_point_st * c) {
    return c->y <= c->x ? true : false;
}

static GT_ATTRIBUTE_RAM_TEXT void _gt_circ_next(gt_point_st * c, gt_size_t * tmp) {

    if(*tmp <= 0) {
        (*tmp) += 2 * c->y + 3; /*Change in decision criterion for y -> y+1*/
    }
    else {
        (*tmp) += 2 * (c->y - c->x) + 5; /*Change for y -> y+1, x -> x-1*/
        c->x--;
    }
    c->y++;
}

static GT_ATTRIBUTE_RAM_TEXT gt_opa_t * _get_next_line(_gt_mask_radius_circle_st * c, gt_size_t y, gt_size_t * len,
                                gt_size_t * x_start) {
    *len = c->opa_start_on_y[y + 1] - c->opa_start_on_y[y];
    *x_start = c->x_start_on_y[y];
    return &c->opa[c->opa_start_on_y[y]];
}

static GT_ATTRIBUTE_RAM_TEXT inline gt_opa_t _gt_mask_mix(gt_opa_t mask_act, gt_opa_t mask_new) {
    if(mask_new >= GT_OPA_MAX) return mask_act;
    if(mask_new <= GT_OPA_MIN) return 0;

    return GT_UDIV255(mask_act * mask_new);// >> 8);
}

static GT_ATTRIBUTE_RAM_TEXT void _gt_circ_calc_aa4(_gt_mask_radius_circle_st* c, uint16_t radius) {
    if(0 == radius) return;
    c->radius = radius;

    if(c->buf) {
        gt_mem_free(c->buf);
        c->buf = NULL;
    }

    uint32_t mask_size = (radius << 1) + 2;
    c->buf = gt_mem_malloc(mask_size * 3);
    if(!c->buf){
        GT_LOGE(GT_LOG_TAG_GUI, "malloc mask buf fail size: %d\r\n", mask_size * 3);
        return ;
    }

    c->opa = c->buf;
    c->opa_start_on_y = (uint16_t*)(c->buf + mask_size);
    c->x_start_on_y = (uint16_t*)(c->buf + (mask_size << 1));

    if(1 == radius){
        c->opa[0] = 180;
        c->opa_start_on_y[0] = 0;
        c->opa_start_on_y[1] = 1;
        c->x_start_on_y[0] = 0;
        return ;
    }

    uint32_t cir_xy_size = (radius + 1) << 3;
    gt_size_t* cir_x = gt_mem_malloc(cir_xy_size);
    if(!cir_x){
        GT_LOGE(GT_LOG_TAG_GUI, "malloc cirx fail size: %d\r\n", cir_xy_size);
        return ;
    }
    gt_size_t* cir_y = &cir_x[(radius + 1) * 2];
    uint32_t y_8th_cnt = 0;
    gt_point_st cp;
    gt_size_t tmp;
    _gt_circ_init(&cp, &tmp, radius * 4);
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
            c->opa[cir_size] = x_fract[0] + x_fract[1] + x_fract[2] + x_fract[3];
            c->opa[cir_size] *= 16;
            cir_size++;
        }
        /*Second line on new x when downscaled*/
        else if(x_int[0] != x_int[1]) {
            cir_x[cir_size] = x_int[0];
            cir_y[cir_size] = y_8th_cnt;
            c->opa[cir_size] = x_fract[0];
            c->opa[cir_size] *= 16;
            cir_size++;

            cir_x[cir_size] = x_int[0] - 1;
            cir_y[cir_size] = y_8th_cnt;
            c->opa[cir_size] = 1 * 4 + x_fract[1] + x_fract[2] + x_fract[3];;
            c->opa[cir_size] *= 16;
            cir_size++;
        }
        /*Third line on new x when downscaled*/
        else if(x_int[0] != x_int[2]) {
            cir_x[cir_size] = x_int[0];
            cir_y[cir_size] = y_8th_cnt;
            c->opa[cir_size] = x_fract[0] + x_fract[1];
            c->opa[cir_size] *= 16;
            cir_size++;

            cir_x[cir_size] = x_int[0] - 1;
            cir_y[cir_size] = y_8th_cnt;
            c->opa[cir_size] = 2 * 4 + x_fract[2] + x_fract[3];;
            c->opa[cir_size] *= 16;
            cir_size++;
        }
        /*Forth line on new x when downscaled*/
        else {
            cir_x[cir_size] = x_int[0];
            cir_y[cir_size] = y_8th_cnt;
            c->opa[cir_size] = x_fract[0] + x_fract[1] + x_fract[2];
            c->opa[cir_size] *= 16;
            cir_size++;

            cir_x[cir_size] = x_int[0] - 1;
            cir_y[cir_size] = y_8th_cnt;
            c->opa[cir_size] = 3 * 4 + x_fract[3];;
            c->opa[cir_size] *= 16;
            cir_size++;
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
        c->opa[cir_size] = tmp_val;
        c->opa[cir_size] *= 16;
        cir_size++;
    }

    /*Build the second octet by mirroring the first*/
    for(i = cir_size - 2; i >= 0; i--, cir_size++) {
        cir_x[cir_size] = cir_y[i];
        cir_y[cir_size] = cir_x[i];
        c->opa[cir_size] = c->opa[i];
    }

    gt_size_t y = 0;
    i = 0;
    c->opa_start_on_y[0] = 0;

    while(i < cir_size) {

        c->opa_start_on_y[y] = i;
        c->x_start_on_y[y] = cir_x[i];
        for(; cir_y[i] == y && i < (int32_t)cir_size; i++) {
            c->x_start_on_y[y] = GT_MIN(c->x_start_on_y[y], cir_x[i]);
        }
        y++;
    }

    gt_mem_free(cir_x);
    cir_x = NULL;

}


static GT_ATTRIBUTE_RAM_TEXT gt_mask_res_et _gt_mask_radius_get(gt_opa_t* mask_buf, \
                                        gt_size_t abs_x, gt_size_t abs_y, uint16_t len, \
                                        gt_mask_radius_st* p)
{
    if(!p){
        return GT_MASK_RES_UNKNOWN;
    }

    bool outer = p->outer;
    int32_t radius = p->radius;
    gt_area_st area;
    gt_area_copy(&area, &p->area);

    gt_size_t x2 = area.x + area.w - 1;
    gt_size_t y2 = area.y + area.h - 1;

    if(outer == false) {
        if((abs_y < area.y || abs_y > y2)) {
            return GT_MASK_RES_TRANSP;
        }
    }
    else {
        if(abs_y < area.y || abs_y > y2) {
            return GT_MASK_RES_FULL_COVER;
        }
    }

    if((abs_x >= area.x + radius && abs_x + len <= x2 - radius) ||
        (abs_y >= area.y + radius && abs_y <= y2 - radius)) {
        if(outer == false) {
            /*Remove the edges*/
            int32_t last = area.x - abs_x;
            if(last > len) return GT_MASK_RES_TRANSP;
            if(last >= 0) {
                gt_memset_0(&mask_buf[0], last);
            }

            int32_t first = x2 - abs_x + 1;
            if(first <= 0) return GT_MASK_RES_TRANSP;
            else if(first < len) {
                gt_memset_0(&mask_buf[first], len - first);
            }
            if(last == 0 && first == len) return GT_MASK_RES_FULL_COVER;
            else return GT_MASK_RES_FULL_COVER;
        }
        else {
            int32_t first = area.x - abs_x;
            if(first < 0) first = 0;
            if(first <= len) {
                int32_t last = x2 - abs_x - first + 1;
                if(first + last > len) last = len - first;
                if(last >= 0) {
                    gt_memset_0(&mask_buf[first], last);
                }
            }
        }
        return GT_MASK_RES_CHANGE;
    }

    int32_t k = area.x - abs_x; /*First relevant coordinate on the of the mask*/
    int32_t w = area.w;
    int32_t h = area.h;
    abs_x -= area.x;
    abs_y -= area.y;

    gt_size_t aa_len;
    gt_size_t x_start;
    gt_size_t cir_y;
    if(abs_y < radius) {
        cir_y = radius - abs_y - 1;
    }
    else {
        cir_y = abs_y - (h - radius);
    }

    if(!p->circle){
        return GT_MASK_RES_UNKNOWN;
    }

    gt_opa_t * aa_opa = _get_next_line(p->circle, cir_y, &aa_len, &x_start);
    gt_size_t cir_x_right = k + w - radius + x_start;
    gt_size_t cir_x_left = k + radius - x_start - 1;
    gt_size_t i;

    if(outer == false) {
        for(i = 0; i < aa_len; i++) {
            gt_opa_t opa = aa_opa[aa_len - i - 1];
            if(cir_x_right + i >= 0 && cir_x_right + i < len) {
                mask_buf[cir_x_right + i] = _GT_DRAW_MASK_MIX(opa, mask_buf[cir_x_right + i]);

            }
            if(cir_x_left - i >= 0 && cir_x_left - i < len) {
                mask_buf[cir_x_left - i] = _GT_DRAW_MASK_MIX(opa, mask_buf[cir_x_left - i]);
            }
        }

        /*Clean the right side*/
        cir_x_right = GT_CLAMP(0, cir_x_right + i, len);
        gt_memset_0(&mask_buf[cir_x_right], len - cir_x_right);

        /*Clean the left side*/
        cir_x_left = GT_CLAMP(0, cir_x_left - aa_len + 1, len);
        gt_memset_0(&mask_buf[0], cir_x_left);
    }
    else {
        for(i = 0; i < aa_len; i++) {
            gt_opa_t opa = 255 - (aa_opa[aa_len - 1 - i]);
            if(cir_x_right + i >= 0 && cir_x_right + i < len) {
                mask_buf[cir_x_right + i] = _GT_DRAW_MASK_MIX(opa, mask_buf[cir_x_right + i]);
            }
            if(cir_x_left - i >= 0 && cir_x_left - i < len) {
                mask_buf[cir_x_left - i] = _GT_DRAW_MASK_MIX(opa, mask_buf[cir_x_left - i]);
            }
        }

        gt_size_t clr_start = GT_CLAMP(0, cir_x_left + 1, len);
        gt_size_t clr_len = GT_CLAMP(0, cir_x_right - clr_start, len - clr_start);
        gt_memset_0(&mask_buf[clr_start], clr_len);
    }

    return GT_MASK_RES_CHANGE;
}

static GT_ATTRIBUTE_RAM_TEXT gt_mask_res_et _gt_mask_line_flat_get(gt_opa_t* mask_buf, \
                                        gt_size_t abs_x, gt_size_t abs_y, uint16_t len, \
                                        gt_mask_line_st* p)
{
    int32_t y_at_x;
    y_at_x = (int32_t)((int32_t)p->yx_steep * abs_x) >> 10;

    if(p->yx_steep > 0) {
        if(y_at_x > abs_y) {
            if(p->inv) { return GT_MASK_RES_FULL_COVER; }
            else { return GT_MASK_RES_TRANSP; }
        }
    }
    else {
        if(y_at_x < abs_y) {
            if(p->inv) { return GT_MASK_RES_FULL_COVER; }
            else { return GT_MASK_RES_TRANSP; }
        }
    }
    y_at_x = (int32_t)((int32_t)p->yx_steep * (abs_x + len)) >> 10;

    if(p->yx_steep > 0) {
        if(y_at_x < abs_y) {
            if(p->inv) { return GT_MASK_RES_TRANSP; }
            else { return GT_MASK_RES_FULL_COVER; }
        }
    }
    else {
        if(y_at_x > abs_y) {
            if(p->inv) { return GT_MASK_RES_TRANSP; }
            else { return GT_MASK_RES_FULL_COVER; }
        }
    }

    int32_t xe;
    if(p->yx_steep > 0) xe = ((abs_y * 256) * p->xy_steep) >> 10;
    else xe = (((abs_y + 1) * 256) * p->xy_steep) >> 10;

    int32_t xei = xe >> 8;
    int32_t xef = xe & 0xFF;

    int32_t px_h;
    if(xef == 0) px_h = 255;
    else px_h = 255 - (((255 - xef) * p->spx) >> 8);
    int32_t k = xei - abs_x;
    gt_opa_t m;

    if(xef) {
        if(k >= 0 && k < len) {
            m = 255 - (((255 - xef) * (255 - px_h)) >> 9);
            if(p->inv) m = 255 - m;
            mask_buf[k] = _GT_DRAW_MASK_MIX(mask_buf[k], m);
        }
        k++;
    }

    while(px_h > p->spx) {
        if(k >= 0 && k < len) {
            m = px_h - (p->spx >> 1);
            if(p->inv) m = 255 - m;
            mask_buf[k] = _GT_DRAW_MASK_MIX(mask_buf[k], m);
        }
        px_h -= p->spx;
        k++;
        if(k >= len) break;
    }

    if(k < len && k >= 0) {
        int32_t x_inters = (px_h * p->xy_steep) >> 10;
        m = (x_inters * px_h) >> 9;
        if(p->yx_steep < 0) m = 255 - m;
        if(p->inv) m = 255 - m;
        mask_buf[k] = _GT_DRAW_MASK_MIX(mask_buf[k], m);
    }

    if(p->inv) {
        k = xei - abs_x;
        if(k > len) {
            return GT_MASK_RES_TRANSP;
        }
        if(k >= 0) {
            gt_memset_0(&mask_buf[0], k);
        }
    }
    else {
        k++;
        if(k < 0) {
            return GT_MASK_RES_TRANSP;
        }
        if(k <= len) {
            gt_memset_0(&mask_buf[k], len - k);
        }
    }

    return GT_MASK_RES_CHANGE;
}


static GT_ATTRIBUTE_RAM_TEXT gt_mask_res_et _gt_mask_line_steep_get(gt_opa_t* mask_buf, \
                                        gt_size_t abs_x, gt_size_t abs_y, uint16_t len, \
                                        gt_mask_line_st* p)
{
    int32_t k;
    int32_t x_at_y;

    x_at_y = (int32_t)((int32_t)p->xy_steep * abs_y) >> 10;
    if(p->xy_steep > 0) x_at_y++;
    if(x_at_y < abs_x) {
        if(p->inv) { return GT_MASK_RES_FULL_COVER; }
        else { return GT_MASK_RES_TRANSP; }
    }

    x_at_y = (int32_t)((int32_t)p->xy_steep * (abs_y)) >> 10;
    if(x_at_y > abs_x + len) {
        if(p->inv) { return GT_MASK_RES_TRANSP; }
        else { return GT_MASK_RES_FULL_COVER; }
    }

    /*X start*/
    int32_t xs = ((abs_y * 256) * p->xy_steep) >> 10;
    int32_t xsi = xs >> 8;
    int32_t xsf = xs & 0xFF;

    /*X end*/
    int32_t xe = (((abs_y + 1) * 256) * p->xy_steep) >> 10;
    int32_t xei = xe >> 8;
    int32_t xef = xe & 0xFF;

    gt_opa_t m;

    k = xsi - abs_x;
    if(xsi != xei && (p->xy_steep < 0 && xsf == 0)) {
        xsf = 0xFF;
        xsi = xei;
        k--;
    }

    if(xsi == xei) {
        if(k >= 0 && k < len) {
            m = (xsf + xef) >> 1;
            if(p->inv) m = 255 - m;
            mask_buf[k] = _GT_DRAW_MASK_MIX(mask_buf[k], m);
        }
        k++;

        if(p->inv) {
            k = xsi - abs_x;
            if(k >= len) {
                return GT_MASK_RES_TRANSP;
            }
            if(k >= 0) gt_memset_0(&mask_buf[0], k);

        }
        else {
            if(k > len) k = len;
            if(k == 0) return GT_MASK_RES_TRANSP;
            else if(k > 0) gt_memset_0(&mask_buf[k],  len - k);
        }
    }
    else {
        int32_t y_inters;
        if(p->xy_steep < 0) {
            y_inters = (xsf * (-p->yx_steep)) >> 10;
            if(k >= 0 && k < len) {
                m = (y_inters * xsf) >> 9;
                if(p->inv) m = 255 - m;
                mask_buf[k] = _GT_DRAW_MASK_MIX(mask_buf[k], m);
            }
            k--;

            int32_t x_inters = ((255 - y_inters) * (-p->xy_steep)) >> 10;

            if(k >= 0 && k < len) {
                m = 255 - (((255 - y_inters) * x_inters) >> 9);
                if(p->inv) m = 255 - m;
                mask_buf[k] = _GT_DRAW_MASK_MIX(mask_buf[k], m);
            }

            k += 2;

            if(p->inv) {
                k = xsi - abs_x - 1;

                if(k > len) k = len;
                else if(k > 0) gt_memset_0(&mask_buf[0],  k);

            }
            else {
                if(k > len) return GT_MASK_RES_FULL_COVER;
                if(k >= 0) gt_memset_0(&mask_buf[k],  len - k);
            }
        }
        else {
            y_inters = ((255 - xsf) * p->yx_steep) >> 10;
            if(k >= 0 && k < len) {
                m = 255 - ((y_inters * (255 - xsf)) >> 9);
                if(p->inv) m = 255 - m;
                mask_buf[k] = _GT_DRAW_MASK_MIX(mask_buf[k], m);
            }

            k++;

            int32_t x_inters = ((255 - y_inters) * p->xy_steep) >> 10;
            if(k >= 0 && k < len) {
                m = ((255 - y_inters) * x_inters) >> 9;
                if(p->inv) m = 255 - m;
                mask_buf[k] = _GT_DRAW_MASK_MIX(mask_buf[k], m);
            }
            k++;

            if(p->inv) {
                k = xsi - abs_x;
                if(k > len)  return GT_MASK_RES_TRANSP;
                if(k >= 0) gt_memset_0(&mask_buf[0],  k);

            }
            else {
                if(k > len) k = len;
                if(k == 0) return GT_MASK_RES_TRANSP;
                else if(k > 0) gt_memset_0(&mask_buf[k],  len - k);
            }
        }
    }

    return GT_MASK_RES_CHANGE;
}

static GT_ATTRIBUTE_RAM_TEXT gt_mask_res_et _gt_mask_line_get(gt_opa_t* mask_buf, \
                                        gt_size_t abs_x, gt_size_t abs_y, uint16_t len, \
                                        gt_mask_line_st* p)
{
    abs_y -= p->origo.y;
    abs_x -= p->origo.x;
    if(p->steep == 0) {
        if(p->flat){
            if(p->side == GT_MASK_LINE_SIDE_LEFT || p->side == GT_MASK_LINE_SIDE_RIGHT){
                return GT_MASK_RES_FULL_COVER;
            }
            else if(p->side == GT_MASK_LINE_SIDE_TOP && abs_y + 1 < 0){
                return GT_MASK_RES_FULL_COVER;
            }
            else if(p->side == GT_MASK_LINE_SIDE_BOTTOM && abs_y > 0){
                return GT_MASK_RES_FULL_COVER;
            }
            else {
                return GT_MASK_RES_TRANSP;
            }
        }
        else{
            if(p->side == GT_MASK_LINE_SIDE_TOP || p->side == GT_MASK_LINE_SIDE_BOTTOM){
                return GT_MASK_RES_FULL_COVER;
            }
            else if(p->side == GT_MASK_LINE_SIDE_RIGHT && abs_x > 0){
                return GT_MASK_RES_FULL_COVER;
            }
            else if(p->side == GT_MASK_LINE_SIDE_LEFT){
                if(abs_x + len < 0) {
                    return GT_MASK_RES_FULL_COVER;
                }
                else{
                    int32_t k = - abs_x;
                    if(k < 0) return GT_MASK_RES_TRANSP;
                    if(k >= 0 && k < len) gt_memset_0(&mask_buf[k], len - k);
                    return  GT_MASK_RES_CHANGE;
                }
            }
            else {
                if(abs_x + len < 0) {
                    return GT_MASK_RES_TRANSP;
                }
                else{
                    int32_t k = - abs_x;
                    if(k < 0) k = 0;
                    if(k >= len) return GT_MASK_RES_TRANSP;
                    else if(k >= 0 && k < len) gt_memset_0(&mask_buf[0], k);
                    return  GT_MASK_RES_CHANGE;
                }
            }
        }
    }

    gt_mask_res_et res;
    if(p->flat) {
        res = _gt_mask_line_flat_get(mask_buf, abs_x, abs_y, len, p);
    }
    else {
        res = _gt_mask_line_steep_get(mask_buf, abs_x, abs_y, len, p);
    }

    return res;
}

static GT_ATTRIBUTE_RAM_TEXT gt_mask_res_et _gt_mask_angle_get(gt_opa_t* mask_buf, \
                                        gt_size_t abs_x, gt_size_t abs_y, uint16_t len, \
                                        gt_mask_angle_st* p)
{
    int32_t rel_y = abs_y - p->vertex_p.y;
    int32_t rel_x = abs_x - p->vertex_p.x;

    if( p->start_angle < 180 && p->end_angle < 180 &&
        p->start_angle != 0  && p->end_angle != 0 &&
        p->start_angle > p->end_angle) {

        if(abs_y < p->vertex_p.y) {
            return GT_MASK_RES_FULL_COVER;
        }

        int32_t end_angle_first = (rel_y * p->end_line.xy_steep) >> 10;
        int32_t start_angle_last = ((rel_y + 1) * p->start_line.xy_steep) >> 10;

        if(p->start_angle > 270 && p->start_angle <= 359 && start_angle_last < 0) start_angle_last = 0;
        else if(p->start_angle > 0 && p->start_angle <= 90 && start_angle_last < 0) start_angle_last = 0;
        else if(p->start_angle > 90 && p->start_angle < 270 && start_angle_last > 0) start_angle_last = 0;

        if(p->end_angle > 270 && p->end_angle <= 359 && start_angle_last < 0) start_angle_last = 0;
        else if(p->end_angle > 0 &&   p->end_angle <= 90 && start_angle_last < 0) start_angle_last = 0;
        else if(p->end_angle > 90 &&  p->end_angle < 270 && start_angle_last > 0) start_angle_last = 0;

        int32_t dist = (end_angle_first - start_angle_last) >> 1;

        gt_mask_res_et res1 = GT_MASK_RES_FULL_COVER;
        gt_mask_res_et res2 = GT_MASK_RES_FULL_COVER;

        int32_t tmp = start_angle_last + dist - rel_x;
        if(tmp > len) { tmp = len; }
        if(tmp > 0) {
            res1 = _gt_mask_line_get(&mask_buf[0], abs_x, abs_y, tmp, &p->start_line);
            if(res1 == GT_MASK_RES_TRANSP) {
                gt_memset_0(&mask_buf[0], tmp);
            }
        }
        if(tmp > len) { tmp = len; }
        if(tmp < 0) { tmp = 0; }

        res2 = _gt_mask_line_get(&mask_buf[tmp], abs_x + tmp, abs_y, len - tmp, &p->end_line);
        if(res2 == GT_MASK_RES_TRANSP) {
            gt_memset_0(&mask_buf[tmp], len - tmp);
        }

        if(res1 == res2) {
            return res1;
        }
        return GT_MASK_RES_CHANGE;
    }
    else if(p->start_angle > 180 && p->end_angle > 180 && p->start_angle > p->end_angle) {

        if(abs_y > p->vertex_p.y) {
            return GT_MASK_RES_FULL_COVER;
        }

        int32_t end_angle_first = (rel_y * p->end_line.xy_steep) >> 10;
        int32_t start_angle_last = ((rel_y + 1) * p->start_line.xy_steep) >> 10;

        if(p->start_angle > 270 && p->start_angle <= 359 && start_angle_last < 0) start_angle_last = 0;
        else if(p->start_angle > 0 && p->start_angle <= 90 && start_angle_last < 0) start_angle_last = 0;
        else if(p->start_angle > 90 && p->start_angle < 270 && start_angle_last > 0) start_angle_last = 0;

        if(p->end_angle > 270 && p->end_angle <= 359 && start_angle_last < 0) start_angle_last = 0;
        else if(p->end_angle > 0 &&   p->end_angle <= 90 && start_angle_last < 0) start_angle_last = 0;
        else if(p->end_angle > 90 &&  p->end_angle < 270 && start_angle_last > 0) start_angle_last = 0;

        int32_t dist = (end_angle_first - start_angle_last) >> 1;

        gt_mask_res_et res1 = GT_MASK_RES_FULL_COVER;
        gt_mask_res_et res2 = GT_MASK_RES_FULL_COVER;

        int32_t tmp = start_angle_last + dist - rel_x;
        if(tmp > len) { tmp = len; }
        if(tmp > 0) {
            res1 = _gt_mask_line_get(&mask_buf[0], abs_x, abs_y, tmp, &p->end_line);
            if(res1 == GT_MASK_RES_TRANSP) {
                gt_memset_0(&mask_buf[0], tmp);
            }
        }

        if(tmp > len) { tmp = len; }
        if(tmp < 0) { tmp = 0; }
        res2 = _gt_mask_line_get(&mask_buf[tmp], abs_x + tmp, abs_y, len - tmp, &p->start_line);
        if(res2 == GT_MASK_RES_TRANSP) {
            gt_memset_0(&mask_buf[tmp], len - tmp);
        }
        if(res1 == res2) {
            return res1;
        }
        return GT_MASK_RES_CHANGE;
    }
    else {
        gt_mask_res_et res1 = GT_MASK_RES_FULL_COVER;
        gt_mask_res_et res2 = GT_MASK_RES_FULL_COVER;

        if(p->start_angle == 180) {
            if(abs_y < p->vertex_p.y) res1 = GT_MASK_RES_FULL_COVER;
            else res1 = GT_MASK_RES_UNKNOWN;
        }
        else if(p->start_angle == 0) {
            if(abs_y < p->vertex_p.y) res1 = GT_MASK_RES_UNKNOWN;
            else res1 = GT_MASK_RES_FULL_COVER;
        }
        else if((p->start_angle < 180 && abs_y < p->vertex_p.y) ||
                (p->start_angle > 180 && abs_y >= p->vertex_p.y)) {
            res1 = GT_MASK_RES_UNKNOWN;
        }
        else  {
            res1 = _gt_mask_line_get(mask_buf, abs_x, abs_y, len, &p->start_line);
        }

        if(p->end_angle == 180) {
            if(abs_y < p->vertex_p.y) res2 = GT_MASK_RES_UNKNOWN;
            else res2 = GT_MASK_RES_FULL_COVER;
        }
        else if(p->end_angle == 0) {
            if(abs_y < p->vertex_p.y) res2 = GT_MASK_RES_FULL_COVER;
            else res2 = GT_MASK_RES_UNKNOWN;
        }
        else if((p->end_angle < 180 && abs_y < p->vertex_p.y) ||
                (p->end_angle > 180 && abs_y >= p->vertex_p.y)) {
            res2 = GT_MASK_RES_UNKNOWN;
        }
        else {
            res2 = _gt_mask_line_get(mask_buf, abs_x, abs_y, len, &p->end_line);
        }

        if(res1 == GT_MASK_RES_TRANSP || res2 == GT_MASK_RES_TRANSP) {
            gt_memset_0(mask_buf, len);
            return GT_MASK_RES_TRANSP;
        }
        else if(res1 == GT_MASK_RES_UNKNOWN && res2 == GT_MASK_RES_UNKNOWN) {
            gt_memset_0(mask_buf, len);
            return GT_MASK_RES_TRANSP;
        }
        else if(res1 == GT_MASK_RES_FULL_COVER &&  res2 == GT_MASK_RES_FULL_COVER) return GT_MASK_RES_FULL_COVER;
        else return GT_MASK_RES_CHANGE;
    }
}
/* global functions / API interface -------------------------------------*/
int16_t gt_mask_add(void* p_mask)
{
    int16_t i = 0;
    for(i = 0; i < _GT_MASK_MAX_NUM; i++) {
        if(_gt_mask_list[i].p_mask == NULL) {
            break;
        }
    }

    if(i >= _GT_MASK_MAX_NUM) {
        return -1;
    }

    _gt_mask_list[i].p_mask = p_mask;
    return i;
}

gt_mask_res_et gt_mask_get(gt_opa_t* mask_buf, gt_size_t x, gt_size_t y, uint16_t len)
{
    bool change = false;
    _gt_mask_dsc_st* p_dsc = NULL;

    _gt_mask_st* list = _gt_mask_list;
    while(list->p_mask){
        p_dsc = list->p_mask;
        gt_mask_res_et res = GT_MASK_RES_FULL_COVER;
        res = p_dsc->mask_get_cb(mask_buf, x, y, len, (void*)list->p_mask);
        if(res == GT_MASK_RES_TRANSP) return res;
        else if(res == GT_MASK_RES_CHANGE) change = true;
        list++;
    }
    return change ? GT_MASK_RES_CHANGE : GT_MASK_RES_FULL_COVER;
}

void gt_mask_remove_idx(int16_t idx)
{
    if(idx < 0 || idx >= _GT_MASK_MAX_NUM){
        return;
    }

    _gt_mask_list[idx].p_mask = NULL;
}

void gt_mask_remove_all(void)
{
    int16_t i = 0;
    for(i = 0; i < _GT_MASK_MAX_NUM; i++) {
        _gt_mask_list[i].p_mask = NULL;
    }
}

void gt_mask_free(void* p_mask)
{
    _gt_mask_dsc_st* p_dsc = p_mask;
    if(GT_MASK_TYPE_RADIUS == p_dsc->type){
        gt_mask_radius_st* p_radius = (gt_mask_radius_st*)p_mask;
        if(p_radius->circle){
            if(p_radius->circle->count < 0){
                gt_mem_free(p_radius->circle->buf);
                p_radius->circle->buf = NULL;
                gt_mem_free(p_radius->circle);
                p_radius->circle = NULL;
            }
            else{
                p_radius->circle->used_cnt--;
            }
        }
    }
}

bool gt_mask_check(const gt_area_st * coords)
{
    if(NULL == coords){
        return _gt_mask_list[0].p_mask ? true : false;
    }

    uint8_t i = 0;
    for(i = 0; i < _GT_MASK_MAX_NUM; ++i){
        _gt_mask_dsc_st* p_dsc = _gt_mask_list[i].p_mask;
        if(NULL == p_dsc) continue;

        if(GT_MASK_TYPE_RADIUS == p_dsc->type){
            // gt_mask_radius_st* p_radius = (gt_mask_radius_st*)_gt_mask_list[i].p_mask;
            // if(p_radius->outer) {
            //     if(!gt_area_is_intersect_screen(coords, &p_radius->area)){ return true; }
            // }else{
            //     if(!gt_area_is_on((gt_area_st*)coords, &p_radius->area)){ return true ; }
            // }
            return true;
        }
        else{
            return true;
        }
    }
    return false;
}

void gt_mask_radius_init(gt_mask_radius_st* p_mask, const gt_area_st* coords, uint16_t radius, bool outer)
{
    uint16_t min = GT_MIN(coords->w, coords->h) >> 1;
    if(radius > min) { radius = min; }

    gt_area_copy(&p_mask->area, coords);
    p_mask->radius = radius;
    p_mask->outer = outer ? 1 : 0;
    p_mask->dsc.mask_get_cb = (gt_mask_get_cb_t)_gt_mask_radius_get;
    p_mask->dsc.type = GT_MASK_TYPE_RADIUS;

    if(radius == 0){
        p_mask->circle = NULL;
    }

    uint32_t i = 0;
    for(i = 0; i < _GT_MASK_CIRCLE_MAX_NUM; ++i) {
        if(_gt_circle_list[i].radius == radius){
            _gt_circle_list[i].used_cnt++;
            _CIRCLE_CACHE_AGING(_gt_circle_list[i].count, radius);
            p_mask->circle = &_gt_circle_list[i];
            return;
        }
    }

    /* If no identical one is found, use the one with the smallest count */
    _gt_mask_radius_circle_st* circle = NULL;
    for(i = 0; i < _GT_MASK_CIRCLE_MAX_NUM; ++i){
        if(0 == _gt_circle_list[i].used_cnt){
            if(!circle){
                circle = &_gt_circle_list[i];
            }else if(_gt_circle_list[i].count < circle->count){
                circle = &_gt_circle_list[i];
            }
        }
    }

    if(!circle){
        circle = gt_mem_malloc(sizeof(_gt_mask_radius_circle_st));
        if(!circle){
            p_mask->circle = NULL;
            GT_LOGE(GT_LOG_TAG_GUI,"gt_mask_radius_init: no memory");
            return;
        }
        GT_ASSERT_MALLOC(circle);
        gt_memset_0(circle, sizeof(_gt_mask_radius_circle_st));
        circle->count = -1;
    }
    else {
        circle->used_cnt++;
        circle->count = 0;
        _CIRCLE_CACHE_AGING(circle->count, radius);
    }

    p_mask->circle = circle;

    _gt_circ_calc_aa4(p_mask->circle, radius);

}

void gt_mask_line_points_init(gt_mask_line_st* p_mask, gt_size_t p1x, gt_size_t p1y, gt_size_t p2x, gt_size_t p2y, gt_mask_line_side_et side)
{
    gt_memset_0(p_mask, sizeof(gt_mask_line_st));

    if(p1y == p2y && side == GT_MASK_LINE_SIDE_BOTTOM) {
        p1y--;
        p2y--;
    }

    if(p1y > p2y) {
        gt_size_t t;
        t = p2x;
        p2x = p1x;
        p1x = t;

        t = p2y;
        p2y = p1y;
        p1y = t;
    }

    p_mask->p1.x = p1x;
    p_mask->p1.y = p1y;
    p_mask->p2.x = p2x;
    p_mask->p2.y = p2y;
    p_mask->side = side;

    p_mask->origo.x = p1x;
    p_mask->origo.y = p1y;
    p_mask->flat = (gt_abs(p2x - p1x) > gt_abs(p2y - p1y)) ? 1 : 0;
    p_mask->yx_steep = 0;
    p_mask->xy_steep = 0;

    p_mask->dsc.mask_get_cb = (gt_mask_get_cb_t)_gt_mask_line_get;
    p_mask->dsc.type = GT_MASK_TYPE_LINE;

    int32_t dx = p2x - p1x;
    int32_t dy = p2y - p1y;

    if(p_mask->flat) {
        int32_t m;

        if(dx) {
            m = (1L << 20) / dx;  /*m is multiplier to normalize y (upscaled by 1024)*/
            p_mask->yx_steep = (m * dy) >> 10;
        }

        if(dy) {
            m = (1L << 20) / dy;  /*m is multiplier to normalize x (upscaled by 1024)*/
            p_mask->xy_steep = (m * dx) >> 10;
        }
        p_mask->steep = p_mask->yx_steep;
    }
    else {
        int32_t m;

        if(dy) {
            m = (1L << 20) / dy;  /*m is multiplier to normalize x (upscaled by 1024)*/
            p_mask->xy_steep = (m * dx) >> 10;
        }

        if(dx) {
            m = (1L << 20) / dx;  /*m is multiplier to normalize x (upscaled by 1024)*/
            p_mask->yx_steep = (m * dy) >> 10;
        }
        p_mask->steep = p_mask->xy_steep;
    }

    if(p_mask->side == GT_MASK_LINE_SIDE_LEFT) p_mask->inv = 0;
    else if(p_mask->side == GT_MASK_LINE_SIDE_RIGHT) p_mask->inv = 1;
    else if(p_mask->side == GT_MASK_LINE_SIDE_TOP) {
        if(p_mask->steep > 0) p_mask->inv = 1;
        else p_mask->inv = 0;
    }
    else if(p_mask->side == GT_MASK_LINE_SIDE_BOTTOM) {
        if(p_mask->steep > 0) p_mask->inv = 0;
        else p_mask->inv = 1;
    }

    p_mask->spx = p_mask->steep >> 2;
    if(p_mask->steep < 0) p_mask->spx = -p_mask->spx;
}

void gt_mask_line_angle_init(gt_mask_line_st* p_mask, gt_size_t p1x, gt_size_t py, int16_t angle, gt_mask_line_side_et side)
{
    if(angle > 180) angle -= 180;

    int32_t p2x, p2y;

    p2x = (gt_sin(angle + 90) >> 5) + p1x;
    p2y = (gt_sin(angle) >> 5) + py;

    gt_mask_line_points_init(p_mask,p1x, py, p2x, p2y, side);
}

void gt_mask_angle_init(gt_mask_angle_st* p_mask, gt_size_t vertex_x, gt_size_t vertex_y, gt_size_t start_angle, gt_size_t end_angle)
{
    gt_mask_line_side_et start_side;
    gt_mask_line_side_et end_side;

    if(start_angle < 0)
        start_angle = 0;
    else if(start_angle > 359)
        start_angle = 359;

    if(end_angle < 0)
        end_angle = 0;
    else if(end_angle > 359)
        end_angle = 359;

    if(end_angle < start_angle) {
        p_mask->delta_deg = 360 - start_angle + end_angle;
    }
    else {
        p_mask->delta_deg = gt_abs(end_angle - start_angle);
    }

    p_mask->start_angle = start_angle;
    p_mask->end_angle = end_angle;
    p_mask->vertex_p.x = vertex_x;
    p_mask->vertex_p.y = vertex_y;
    p_mask->dsc.mask_get_cb = (gt_mask_get_cb_t)_gt_mask_angle_get;
    p_mask->dsc.type = GT_MASK_TYPE_ANGLE;

    GT_ASSERT_MSG(start_angle >= 0 && start_angle <= 360, "4");

    if(start_angle >= 0 && start_angle < 180) {
        start_side = GT_MASK_LINE_SIDE_LEFT;
    }
    else{
        start_side = GT_MASK_LINE_SIDE_RIGHT; /*silence compiler*/
    }

    GT_ASSERT_MSG(end_angle >= 0 && start_angle <= 360, "Unexpected end angle");

    if(end_angle >= 0 && end_angle < 180) {
        end_side = GT_MASK_LINE_SIDE_RIGHT;
    }
    else if(end_angle >= 180 && end_angle < 360) {
        end_side = GT_MASK_LINE_SIDE_LEFT;
    }
    else{
        end_side = GT_MASK_LINE_SIDE_RIGHT; /*silence compiler*/
    }

    gt_mask_line_angle_init(&p_mask->start_line, vertex_x, vertex_y, start_angle, start_side);
    gt_mask_line_angle_init(&p_mask->end_line, vertex_x, vertex_y, end_angle, end_side);
}


/* end of file ----------------------------------------------------------*/


