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
#include "gt_fs.h"

#include "../font/gt_symbol.h"

#include "gt_img_decoder.h"
/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/

static inline bool _is_align_reverse(uint8_t align) {
    return (align & GT_ALIGN_REVERSE) ? true : false;
}

static inline uint8_t _get_align_type(uint8_t align)
{
    return (align & 0x7F);
}

/* static functions -----------------------------------------------------*/
static void gt_mirror_hor_u8(uint8_t * dst, uint8_t * src, uint32_t w, uint32_t h)
{
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

static void gt_mirror_ver_u8(uint8_t * dst, uint8_t * src, uint32_t w, uint32_t h)
{
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


/* global functions / API interface -------------------------------------*/
void gt_draw_rect(gt_draw_ctx_t * ctx, gt_attr_rect_st * attr, gt_area_st * coords) {
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

void gt_draw_arch(gt_draw_ctx_t * draw_ctx, gt_draw_blend_dsc_st  *blend_dsc,uint8_t *mask_left_up,uint8_t *mask_right_up,const gt_area_st * coords)
{
    gt_size_t x = blend_dsc->mask_area->x;
    uint8_t radius = blend_dsc->mask_area->w;
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

void gt_draw_line(gt_attr_line_st * line_attr, gt_draw_ctx_t * draw_ctx, gt_draw_blend_dsc_st * blend_dsc,uint8_t *mask_left_up,uint8_t *mask_right_up,const gt_area_st * coords)
{
	gt_size_t x1,x2,y1,y2;		//two point get a line

	x1 = line_attr->x_1;
	y1 = line_attr->y_1;

	x2 = line_attr->x_2;
	y2 = line_attr->y_2;

	gt_size_t dx =  abs(x2-x1), sx = x1<x2 ? 1 : -1;
	gt_size_t dy = -abs(y2-y1), sy = y1<y2 ? 1 : -1;
	gt_size_t err = dx+dy, e2; /* error value e_xy */

	for(;;){  /* loop */
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
        gt_draw_arch(draw_ctx,blend_dsc,mask_left_up,mask_right_up,coords);
	}
}

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

void draw_bg(gt_draw_ctx_t * draw_ctx, const gt_attr_rect_st * dsc, const gt_area_st * coords)
{
    if (dsc->bg_opa <= GT_OPA_MIN) return;

    if( !gt_area_is_intersect_screen(&draw_ctx->buf_area, coords) ){
        return;
    }

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
            return ;
        }
        uint8_t * mask_buf = NULL ;
        uint32_t len = coords->w * coords->h , index = 0 , buf_idx = 0;
        char idx_bit = 0;
        mask_buf = gt_mem_malloc(len);
        if(!mask_buf){
            return ;
        }
        blend_dsc.mask_buf = mask_buf;
        blend_dsc.mask_area->w = coords->w;
        blend_dsc.mask_area->h = coords->h;
        // GT_LOGV(GT_LOG_TAG_GUI , "dsc->gray = %d , w = %d , h = %d , buf_size = %d" ,dsc->gray , blend_dsc.mask_area->w , blend_dsc.mask_area->h , len);

        len = len >> 3;
        buf_idx = 0;
        index = 0;
        while( index < len ){

            for(idx_bit = 0 ; idx_bit < 8 ;idx_bit++)
            {
                if( (dsc->data_gray[index] << idx_bit )&0x80 ){
                    mask_buf[buf_idx++] = 0xFF;
                }else{
                    mask_buf[buf_idx++] = 0x00;
                }
            }
            index++;
        }
        gt_draw_blend(draw_ctx , &blend_dsc);

        if(NULL != mask_buf)
        {
            gt_mem_free(mask_buf);
        }
        mask_buf = NULL;
        return ;
    }

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
                return ;
            }
            if (NULL == mask_left_up) {
                gt_mem_free(color_circle_p);
                radius_pre = 0;
                return ;
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
                        return ;
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
}


static uint16_t _gt_gray_and_dot_byte_get(uint8_t* gray , uint8_t width , gt_font_st font , uint8_t ret_style)
{
    uint16_t dot_byte = 0;
    uint8_t font_gray = 1;
    if( (GT_FONT_TYPE_FLAG_DOT_MATRIX_NON_WIDTH) == ret_style)
    {
        dot_byte = (font.info.size + 7) >> 3;
        font_gray = 1;
    }
    else if((GT_FONT_TYPE_FLAG_VEC == ret_style))
    {
        font_gray = font.info.gray;

        if(1 != font_gray){
            dot_byte = (((((font.info.size * font_gray + 15) >> 4) << 4) / font_gray + 7) >> 3) * font_gray;
        }
        else
        {
            dot_byte = (((font.info.size + 15) >> 4) << 4) >> 3;
        }
    }
    else{
        dot_byte = (width + 7) >> 3;
        font_gray = 1;
    }
    *gray = font_gray ;
    return dot_byte;
}


static uint32_t draw_text_convertor_split(const gt_font_st *fonts , uint8_t lan , gt_draw_ctx_t * draw_ctx , gt_draw_blend_dsc_st* blend_dsc,
                                    const gt_attr_font_st * dsc , int font_disp_h , gt_draw_blend_dsc_st* blend_dsc_style , const gt_area_st * coords ,uint8_t flag_ol)
{
    uint32_t idx = 0 , unicode , tmp;
    uint16_t *text = NULL;
    uint8_t *font_buf = NULL;
    text = (uint16_t*)gt_mem_malloc(fonts->len * sizeof(uint16_t));
    if(NULL == text){
        GT_LOGE(GT_LOG_TAG_GUI, "buf malloc failed , size: %d" , fonts->len * sizeof(uint16_t));
        goto _ret_handle;
    }

    gt_font_st _font = {
        .info = fonts->info,
        .utf8 = fonts->utf8,
        .len = fonts->len,
        .res = NULL,
    };

    tmp = 0;
    while (idx < _font.len)
    {
        idx += gt_font_one_char_code_len_get((uint8_t * )&_font.utf8[idx], &unicode, _font.info.encoding);
        text[tmp++] = (uint16_t)unicode;
    }
    uint16_t text_len = tmp;
    uint8_t width , byte_width, font_gray, x_offset;
    uint8_t ret_style = gt_font_type_get(_font.info.style_fl);
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(&_font.info, ret_style, lan, text_len);

    font_buf = (uint8_t*)gt_mem_malloc(font_size_res.font_buff_len);
    if(NULL == font_buf){
        GT_LOGE(GT_LOG_TAG_GUI, "buf malloc failed , size: %d" , font_size_res.font_buff_len);
        goto _ret_handle;
    }
    gt_memset_0(font_buf , font_size_res.font_buff_len);

    gt_area_st area_font_style;
    uint8_t style_space_y = (dsc->style == GT_FONT_STYLE_NONE) ? 0 : ((_font.info.size >> 4) ? (_font.info.size >> 4) : 1);

    font_convertor_st convertor = {
        .fontcode = text ,
        .code_len = text_len ,
        .fontsize = _font.info.size ,
        .fontgray = _font.info.gray ,
        .font_option = _font.info.style_fl ,
        .thick = _font.info.thick_en,
        .data = font_buf ,
        .data_len = font_size_res.font_buff_len ,
        .is_rev = _is_align_reverse(dsc->align) ? 0 : 1,
        .is_vec = ret_style == GT_FONT_TYPE_FLAG_VEC ? 1 : 0 ,
    };

    text_len = gt_font_code_transform(&convertor);

    idx = 0;
    tmp = 0;
    _font.res = &font_buf[font_size_res.font_buff_len - font_size_res.font_per_size];
    while (idx < text_len)
    {
        if(STYLE_EN_HINDI == lan){
            width = gt_font_get_word_width_figure_up(&convertor.data[idx * font_size_res.font_per_size] , 16 , 24 , 1);
            tmp = 1;
        }
        else{
            gt_memset_0(&convertor.data[0] , font_size_res.font_buff_len);
            if(0x20 == convertor.fontcode[idx]){
                width = convertor.fontsize >> 1;
            }
            else{
                tmp = gt_font_convertor_data_get(&convertor , idx);
                width = gt_font_get_word_width_figure_up(&convertor.data[0] , font_size_res.dot_width , convertor.fontsize , convertor.fontgray);
            }
        }
        if(_is_align_reverse(dsc->align)){
            blend_dsc->dst_area->x -= width;
        }

        if(flag_ol){
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
            }
            else{
                // out of range
                if((blend_dsc->dst_area->x + width) > (coords->x + coords->w)){
                    blend_dsc->dst_area->x = coords->x;
                    blend_dsc->dst_area->y += blend_dsc->dst_area->h;
                    if((blend_dsc->dst_area->y + style_space_y + blend_dsc->dst_area->h) > (coords->y + coords->h)){
                        font_disp_h = (coords->y + coords->h) - blend_dsc->dst_area->y;
                        if(font_disp_h < 2){
                            goto _ret_handle;
                        }
                    }
                }
            }
        }
        blend_dsc->dst_area->h = font_disp_h;
        if(STYLE_EN_HINDI == lan){
            byte_width = 2;
            font_gray = 1;
            blend_dsc->dst_area->w = byte_width << 3;
            gt_draw_blend_text(draw_ctx, (const gt_draw_blend_dsc_st*)blend_dsc, _font.info.size, byte_width, font_gray , (const uint8_t*)&convertor.data[idx * font_size_res.font_per_size]);

        }
        else{
            byte_width = (gt_font_get_one_word_width(convertor.fontcode[idx] , &_font) + 7 ) >> 3;
            font_gray = 1;
            byte_width = _gt_gray_and_dot_byte_get(&font_gray , byte_width , _font , ret_style);
            blend_dsc->dst_area->w = byte_width << 3;
            gt_draw_blend_text(draw_ctx, (const gt_draw_blend_dsc_st *)blend_dsc, _font.info.size, byte_width, font_gray , (const uint8_t*)&convertor.data[0]);

        }

        /* use style */

        if( dsc->style == GT_FONT_STYLE_UNDERLINE && font_disp_h == _font.info.size)
        {
            area_font_style = *blend_dsc->dst_area;
            area_font_style.y += _font.info.size + 1;
            area_font_style.h = style_space_y;
            area_font_style.w = width;
            blend_dsc_style->color_fill = dsc->font_color;
            blend_dsc_style->dst_area = &area_font_style;
            gt_draw_blend(draw_ctx, blend_dsc_style);
        }
        if( dsc->style == GT_FONT_STYLE_STRIKETHROUGH && (font_disp_h > ((_font.info.size >> 1) + (style_space_y << 1))))
        {
            area_font_style = *blend_dsc->dst_area;
            area_font_style.y += (_font.info.size >> 1);
            area_font_style.h = style_space_y;
            area_font_style.w = width;
            blend_dsc_style->color_fill = dsc->font_color;
            blend_dsc_style->dst_area = &area_font_style;
            gt_draw_blend(draw_ctx, blend_dsc_style);
        }

        if(_is_align_reverse(dsc->align)){
            blend_dsc->dst_area->x -= dsc->space_x;
        }
        else{
            blend_dsc->dst_area->x += width + dsc->space_x;
        }
        idx += tmp;
    }

_ret_handle:
    if(NULL !=text){
        gt_mem_free(text);
        text = NULL;
    }
    if(NULL != font_buf){
        gt_mem_free(font_buf);
        font_buf = NULL;
    }
    return fonts->len;
}

gt_area_st draw_text_split(gt_draw_ctx_t * draw_ctx, const gt_attr_font_st * dsc, const gt_area_st * coords)
{
    gt_area_st area_font = {
        .x = coords->x , .y = coords->y , .w = dsc->font->info.size, .h = dsc->font->info.size
    };

    if( dsc->font->len == 0 ){
        return area_font;
    }

    gt_font_st _font = {
        .info = dsc->font->info,
        .res = NULL,
    };
    _font.info.gray = dsc->font->info.gray == 0 ? 1 : dsc->font->info.gray;
    uint32_t font_dot_w = (((_font.info.size + 15) >> 4) << 4) * _font.info.gray;
    uint32_t data_len = font_dot_w * font_dot_w >> 3;
    uint32_t mask_len = font_dot_w * font_dot_w;
    /*malloc*/
    uint8_t *data = NULL , *mask_line = NULL , *ret_txt = NULL;
    data = gt_mem_malloc(data_len);
    if(NULL == data){
        GT_LOGE(GT_LOG_TAG_GUI , "data malloc is failed!!! size = %d" , data_len);
        goto _ret_handler;
    }
    mask_line = gt_mem_malloc( _font.info.size << 1 );
    if(NULL == mask_line){
        GT_LOGE(GT_LOG_TAG_GUI , "mask_line malloc is failed!!! size = %d" , _font.info.size << 1);
        goto _ret_handler;
    }
    _font.res = data;
    //
    bool heb_flag = false;
    uint8_t lan;
    _font.utf8 = dsc->font->utf8;
    _font.len = dsc->font->len;
    lan = right_to_left_lan_get(dsc->font->info.style_fl);
    if(lan == STYLE_EN_HEBREW || lan == STYLE_EN_ARABIC){
        ret_txt = gt_mem_malloc(dsc->font->len);
        if(NULL == ret_txt){
            GT_LOGE(GT_LOG_TAG_GUI , "ret_txt malloc is failed!!! size = %d" , dsc->font->len);
            goto _ret_handler;
        }
        heb_flag = gt_right_to_left_handler(&_font , ret_txt , lan);
    }

    //
    gt_draw_blend_dsc_st blend_dsc = {
        .mask_buf   = NULL,
        .dst_area   = &area_font,
        .color_fill = dsc->font_color,
        .opa        = dsc->opa,
    };
    gt_draw_blend_dsc_st blend_dsc_style;
    blend_dsc_style = blend_dsc;
    blend_dsc_style.color_fill  = gt_color_black();
    blend_dsc_style.mask_buf    = mask_line;
    gt_area_st area_font_style;
    gt_memset(mask_line, 0xFF, _font.info.size << 1);
    uint8_t style_space_y = (dsc->style == GT_FONT_STYLE_NONE) ? 0 : ((_font.info.size >> 4) ? (_font.info.size >> 4) : 1);

    //
    uint16_t idx = 0 , idx_2 = 0 , idx_step = 0, idx_len = 0, txt_len = dsc->font->len ;
    uint16_t disp_w = coords->w ;
    uint16_t overlength_offset = ((coords->w / ((_font.info.size >> 1) + dsc->space_x) - 1) >> 1 << 1);
    uint8_t overlength = 0;
    uint32_t  unicode = 0 , ret_w = 0 , lan_len;
    //
    uint8_t *txt = heb_flag ? ret_txt : (uint8_t*)dsc->font->utf8;
    uint8_t* txt_2 = &txt[0];
    //
    bool cov_flag_ol = false;
    uint16_t bidi_len  = 0 , bidi_max = 1 , bidi_pos = 0;
    gt_bidi_st* bidi = NULL;
    bidi = (gt_bidi_st*)gt_mem_malloc(bidi_max * sizeof(gt_bidi_st));
    if(NULL == bidi){
        GT_LOGE(GT_LOG_TAG_GUI , "bidi malloc is failed!!! size = %d" , bidi_max * sizeof(gt_bidi_st));
        goto _ret_handler;
    }

    // align
    area_font.y = coords->y;
    if( (GT_ALIGN_LEFT !=_get_align_type(dsc->align)) &&  (GT_ALIGN_RIGHT !=_get_align_type(dsc->align)) && \
        (GT_ALIGN_CENTER !=_get_align_type(dsc->align)) &&  (GT_ALIGN_NONE !=_get_align_type(dsc->align))
    )
    {
        idx_len = 0;
        while (idx < txt_len)
        {
            if(0x0A == txt[idx]){
                idx_step = 0;
                ++idx;
                goto _compute_line;
            }
            _font.utf8 = &txt[idx];
            _font.len = txt_len - idx;

            idx_step = gt_font_split( &_font , disp_w, coords->w ,dsc->space_x , &ret_w , &lan , &lan_len);

            if(0 == idx_step && 0 == ret_w){
                ++overlength;
                if(disp_w == coords->w){
                    ++overlength;

                }
            }
            else{
                overlength = 0;
            }

            if( disp_w - ret_w > _font.info.size && idx_step != 0 ){
                idx += idx_step;
                disp_w -= ret_w;
                if(idx < txt_len){
                    continue;
                }
            }
        _compute_line:
            idx_len++;
            idx += overlength < 2 ? idx_step : overlength_offset;
            disp_w = coords->w;
            overlength = 0;
        }

        idx_len = idx_len * (_font.info.size + (style_space_y << 1) + dsc->space_y);
        if( (GT_ALIGN_LEFT_MID == _get_align_type(dsc->align)) || (GT_ALIGN_RIGHT_MID == _get_align_type(dsc->align)) || \
            (GT_ALIGN_CENTER_MID == _get_align_type(dsc->align))
        )
        {
            if(idx_len < coords->h){
                area_font.y = coords->y + ((coords->h - idx_len) >> 1);
            }
        }
        else
        {
            if(idx_len < coords->h){
                area_font.y = coords->y + ((coords->h - idx_len));
            }
        }
    }
    //
    if(dsc->start_y ){
        area_font.y = dsc->start_y;
    }

    // disp
    idx = 0;
    idx_len = 0;
    overlength = 0;
    bidi_len = 0;

    uint16_t start_x = dsc->start_x;
    disp_w = coords->w;
    if(start_x){
        disp_w = coords->w - (start_x - coords->x);
        area_font.x = start_x;
    }

    int font_disp_h = _font.info.size;

    while (idx < txt_len)
    {
        // out of range
        if((area_font.y + style_space_y + area_font.h ) > (coords->y + coords->h)){
            font_disp_h = (coords->y + coords->h) - area_font.y;
            if(font_disp_h < 2){
                break;
            }
        }
        // '\n'
        if(0x0A == txt[idx]){
            idx_step = 0;
            ++idx;
            goto _disp_font;
        }

        _font.utf8 = &txt[idx];
        _font.len = txt_len - idx;
        idx_step = gt_font_split( &_font , disp_w, coords->w , dsc->space_x , &ret_w , &lan , &lan_len);

        if(0 == idx_step && 0 == ret_w){
            ++overlength;
            if(disp_w == coords->w){
                ++overlength;
            }
        }
        else{
            overlength = 0;
        }

        if(0 != idx_step){
            disp_w -= ret_w;
        }

        if( disp_w > _font.info.size && idx_step != 0 ){

            if(is_convertor_language(lan)){
                if(bidi_max <= bidi_len){
                    ++bidi_max;
                    bidi = (gt_bidi_st*)gt_mem_realloc( bidi ,bidi_max*sizeof(gt_bidi_st));
                }

                bidi[bidi_len].idx = idx_len;
                bidi[bidi_len].len = idx_step;
                bidi[bidi_len].flag = lan;
                ++bidi_len;
            }

            idx += idx_step;
            idx_len += idx_step;

            if(idx < txt_len){
                continue;
            }
        }

        //
        // if(1 == idx_len && 1 == overlength && 0x20 == txt_2[0]){
        //     // idx += idx_len;
        //     idx_len = 0;
        //     txt_2 = &txt[idx];
        //     bidi_len = 0;
        //     font_disp_h = _font.info.size;
        //     disp_w = coords->w;
        //     continue;
        // }

    _disp_font:
        if(overlength < 2){
            if(is_convertor_language(lan) && (idx < txt_len)){
                if(bidi_max <= bidi_len){
                    ++bidi_max;
                    bidi = (gt_bidi_st*)gt_mem_realloc( bidi ,bidi_max*sizeof(gt_bidi_st));
                }

                bidi[bidi_len].idx = idx_len;
                bidi[bidi_len].len = idx_step;
                bidi[bidi_len].flag = lan;
                ++bidi_len;
            }
            idx_len = (idx < txt_len) ? idx_len + idx_step : idx_len;
        }
        else{
            if(is_convertor_language(lan)){
                if(bidi_max <= bidi_len){
                    ++bidi_max;
                    bidi = (gt_bidi_st*)gt_mem_realloc( bidi ,bidi_max*sizeof(gt_bidi_st));
                }

                bidi[bidi_len].idx = idx_len;
                bidi[bidi_len].len = lan_len;
                bidi[bidi_len].flag = lan;
                ++bidi_len;

                idx_len = lan_len;
                disp_w = 0;
            }
            else{
                idx_len = overlength_offset;
                disp_w = 0;
            }
        }
        // align
        if(!cov_flag_ol){
            switch( _get_align_type(dsc->align) ) {
            case GT_ALIGN_RIGHT_BOTTOM:
            case GT_ALIGN_RIGHT_MID:
            case GT_ALIGN_RIGHT:
                area_font.x = coords->x + disp_w;
                break;
            case GT_ALIGN_CENTER_BOTTOM:
            case GT_ALIGN_CENTER_MID:
            case GT_ALIGN_CENTER:
                area_font.x = coords->x + (disp_w >> 1);
                break;
            case GT_ALIGN_LEFT_BOTTOM:
            case GT_ALIGN_LEFT_MID:
            case GT_ALIGN_LEFT:
            case GT_ALIGN_NONE:
            default:
                area_font.x = coords->x;
                break;
            }
        }

        if(_is_align_reverse(dsc->align) && !cov_flag_ol) {
            area_font.x = (coords->x + coords->w) - (area_font.x - coords->x);
        }

        if(start_x){
            area_font.x = start_x;
            start_x = 0;
        }
        idx_2 = 0;
        bidi_pos = 0;
        area_font.h = font_disp_h;
        while (idx_2 < idx_len)
        {
            gt_memset(_font.res, 0, data_len);

            if(bidi_pos < bidi_len && idx_2 == bidi[bidi_pos].idx){
                _font.utf8 = (char * )&txt_2[idx_2];
                _font.len = bidi[bidi_pos].len;
                draw_text_convertor_split(&_font , bidi[bidi_pos].flag , draw_ctx ,  &blend_dsc, dsc , font_disp_h , &blend_dsc_style , coords, (overlength < 2 ? 0 : 1));
                bidi_pos++;
                goto _disp_next_idx2;
            }

            _font.utf8 = (char * )&txt_2[idx_2];
            _font.len = gt_font_one_char_code_len_get(&txt_2[idx_2], &unicode , _font.info.encoding);

            uint8_t width = gt_font_get_one_word_width(unicode, &_font);

            if(_is_align_reverse(dsc->align)){
                area_font.x -= width;
            }

            if( !gt_symbol_check_by_unicode(unicode) ){

                gt_memset(_font.res, 0, data_len);
                uint16_t ret_style = gt_font_get_string_dot(&_font);

                uint8_t byte_width = (width + 7) >> 3 ;
                uint8_t font_gray = 1;

                byte_width = _gt_gray_and_dot_byte_get(&font_gray , width , _font , ret_style);
                area_font.w = byte_width << 3;
                gt_draw_blend_text(draw_ctx, &blend_dsc, _font.info.size, byte_width, font_gray , (const uint8_t*)_font.res);

            }else{
                const uint8_t * temp = gt_symbol_get_mask_buf(_font.utf8 , _font.info.size);
                if( temp == NULL ){
                    area_font.x += width + dsc->space_x;
                    idx += _font.len;
                    continue;
                }
                blend_dsc.mask_buf = (uint8_t*)temp;
                gt_draw_blend(draw_ctx, &blend_dsc);
                blend_dsc.mask_buf = NULL;
            }

            /* use style */
            if( dsc->style == GT_FONT_STYLE_UNDERLINE && font_disp_h == _font.info.size){
                area_font_style = area_font;
                area_font_style.y += _font.info.size + 1;
                area_font_style.h = style_space_y;
                area_font_style.w = width;
                blend_dsc_style.color_fill = dsc->font_color;
                blend_dsc_style.dst_area = &area_font_style;
                gt_draw_blend(draw_ctx, &blend_dsc_style);
            }
            if( dsc->style == GT_FONT_STYLE_STRIKETHROUGH && (font_disp_h > ((_font.info.size >> 1) + (style_space_y << 1))))
            {
                area_font_style = area_font;
                area_font_style.y += (_font.info.size >> 1);
                area_font_style.h = style_space_y;
                area_font_style.w = width;
                blend_dsc_style.color_fill = dsc->font_color;
                blend_dsc_style.dst_area = &area_font_style;
                gt_draw_blend(draw_ctx, &blend_dsc_style);
            }

            //
            if(_is_align_reverse(dsc->align)){
                area_font.x -= dsc->space_x;
            }
            else{
                area_font.x += width + dsc->space_x;
            }
        _disp_next_idx2:
            idx_2 += _font.len;
        }

        if(is_convertor_language(lan) && overlength >= 2){
            idx += idx_len;
            cov_flag_ol = true;
            if(_is_align_reverse(dsc->align)){
                disp_w = area_font.x - coords->x;
            }
            else{
                disp_w = coords->w - (area_font.x - coords->x);
            }
        }
        else{
            cov_flag_ol = false;
            idx += overlength < 2 ? idx_step : overlength_offset;
            if(idx < txt_len){
                area_font.y += area_font.h + dsc->space_y + (style_space_y << 1);
                area_font.x = coords->x;
            }
            disp_w = coords->w;
        }
        idx_len = 0;
        txt_2 = &txt[idx];
        // overlength = 0;
        bidi_len = 0;
        font_disp_h = _font.info.size;
        area_font.h = _font.info.size;
        area_font.w = _font.info.size;

    }

_ret_handler:
    if(NULL != mask_line){
        gt_mem_free(mask_line);
        mask_line = NULL;
    }
    if(NULL != data){
        gt_mem_free(data);
        data = NULL;
    }
    if(NULL != ret_txt){
        gt_mem_free(ret_txt);
        ret_txt = NULL;
    }
    if(NULL != bidi){
        gt_mem_free(bidi);
        bidi = NULL;
    }
    return area_font;
}


gt_area_st draw_text(gt_draw_ctx_t * draw_ctx, const gt_attr_font_st * dsc, const gt_area_st * coords)
{
    return draw_text_split(draw_ctx , dsc , coords);
}

#if GT_USE_SJPG
static void _filling_by_sjpg_file(gt_draw_ctx_t * draw_ctx, gt_draw_blend_dsc_st * blend_dsc, _gt_img_dsc_st * dsc_img) {
    uint16_t w, h, y = 0;
    w = dsc_img->header.w;
    h = dsc_img->header.h;

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

static void _filling_by_ram_data(gt_draw_ctx_t * draw_ctx, gt_draw_blend_dsc_st * blend_dsc, _gt_img_dsc_st * dsc_img) {
    uint16_t w, h, y = 0;
    w = dsc_img->header.w;
    h = dsc_img->header.h;

    blend_dsc->mask_buf = &dsc_img->alpha[0];
    blend_dsc->dst_buf = (gt_color_t *)&dsc_img->img[0];
    if (NULL == blend_dsc->dst_buf || NULL == blend_dsc->mask_buf) {
        GT_LOGW(GT_LOG_TAG_GUI, "Per-line buffer malloc failed");
        return;
    }

    while(y < h){
        if( gt_area_is_intersect_screen(&draw_ctx->buf_area, blend_dsc->dst_area) ){
            /* blend to bg */
            gt_draw_blend(draw_ctx, blend_dsc);
        }

        blend_dsc->dst_buf += w;
        blend_dsc->mask_buf += w;
        ++blend_dsc->dst_area->y;
        ++blend_dsc->mask_area->y;
        ++y;
    }
}

/**
 * @brief GT-HMI Engine Image format
 *
 * @param draw_ctx
 * @param blend_dsc
 * @param dsc_img
 */
static void _filling_by_default(gt_draw_ctx_t * draw_ctx, gt_draw_blend_dsc_st * blend_dsc, _gt_img_dsc_st * dsc_img) {
    uint32_t offset = 0, len = 0;
    uint32_t addr_start_alpha = 0;
    uint8_t * mask_buf = NULL;
    uint16_t w, h, y = 0;
    w = dsc_img->header.w;
    h = dsc_img->header.h;
    addr_start_alpha = w * h * sizeof(gt_color_t);

    gt_fs_fp_st * fp = dsc_img->fp;
    if (!fp) {
        GT_LOGW(GT_LOG_TAG_GUI, "gt_fs_get_fp failed");
        return;
    }

    len = w * sizeof(gt_color_t);
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

    while(y < h){
        if (gt_area_is_intersect_screen(&draw_ctx->buf_area, blend_dsc->dst_area)) {
            /*read one line color data*/
            gt_fs_read_img_offset(fp, (uint8_t *)blend_dsc->dst_buf, offset, len);
            if(is_mask) {    //png
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

void draw_bg_img(gt_draw_ctx_t * draw_ctx, const gt_attr_rect_st * dsc, gt_area_st * coords)
{
    // if( dsc->bg_img_src == NULL ) return;
    if( dsc->bg_opa < GT_OPA_MIN ) return;

    _gt_img_dsc_st dsc_img = {0};
    if (NULL == dsc->bg_img_src) {
        if (NULL == dsc->ram_img) {
            return;
        }
        dsc_img = *dsc->ram_img;
        // gt_memcpy(&dsc_img, dsc->ram_img, sizeof(_gt_img_dsc_st));
    }
    else if ( GT_RES_FAIL == gt_img_decoder_open(&dsc_img, dsc->bg_img_src)) {
        GT_LOGW(GT_LOG_TAG_DATA, "Open image decoder failed\n");
        return ;
    }
    coords->w = dsc_img.header.w;
    coords->h = dsc_img.header.h;
    _gt_disp_update_max_area(coords, true);

    gt_area_st dst_area = *coords, mask_area = *coords;
    dst_area.h = 1;         /** 每次只填充一行的数据量 */

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


void draw_focus(gt_obj_st* obj , gt_size_t radius)
{
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.bg_opa = obj->opa;
    rect_attr.radius = radius;
    rect_attr.reg.is_fill = 0;
    rect_attr.border_width = 1;
    rect_attr.bg_color = gt_color_focus();
    rect_attr.border_color = gt_color_focus();

    if( obj->focus && GT_ENABLED == obj->focus_dis){
        draw_bg(obj->draw_ctx, &rect_attr, &obj->area);
    }
}

/* end ------------------------------------------------------------------*/
