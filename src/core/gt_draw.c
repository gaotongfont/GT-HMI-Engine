/**
 * @file gt_draw.c
 * @author yongg
 * @brief Interface functions for drawing text and images
 * @version 0.1
 * @date 2022-08-09 18:29:24
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
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

    gt_area_st bg_coords, area_res;
    gt_area_copy(&bg_coords, (gt_area_st *)coords);

    if( !gt_area_intersect_screen(&draw_ctx->buf_area, &bg_coords, &area_res) ){
        return;
    }

    static uint16_t radius_pre = 0;
    static gt_color_t * color_circle_p = NULL;
    static uint8_t * mask_left_up = NULL;

    // calc radius
    uint16_t radius = GT_MIN(bg_coords.w >> 1, bg_coords.h >> 1);
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
        GT_LOGV(GT_LOG_TAG_GUI , "dsc->gray = %d , w = %d , h = %d , buf_size = %d" ,dsc->gray , blend_dsc.mask_area->w , blend_dsc.mask_area->h , len);

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


static void _push_mask_buf(unsigned char *mask_buf , unsigned char *font_buf , uint8_t font_size , uint8_t font_gray ,uint8_t byte_width)
{
    gt_size_t idx_bit = 0, idx_mask_buf = 0 , idx_res = 0;
    uint16_t font_y = 0, font_x = 0;
    uint8_t tmp_c = 0;

    if(2 == font_gray || 4 == font_gray){
        for ( font_y = 0; font_y < font_size; font_y++) {
            for (font_x = 0; font_x < byte_width; font_x++) {
                for( idx_bit = 0 ; idx_bit < 8 ; idx_bit++){
                    if(0 == (idx_bit % (8 / font_gray) )){
                        tmp_c = font_buf[idx_res++];
                    }
                    mask_buf[idx_mask_buf + (font_x << 3) + idx_bit] =  (tmp_c >> (8 - font_gray))  * ((0xFF/((0x01 << font_gray)-1)));
                    tmp_c <<= font_gray;
                }
            }
            idx_mask_buf += font_size;
        }
    }
    else if(3 == font_gray) {
        for ( font_y = 0; font_y < font_size; font_y++) {
            for (font_x = 0; font_x < byte_width; font_x++) {
                tmp_c = font_buf[idx_res];
                for( idx_bit = 0 ; idx_bit < 8 ; idx_bit++){
                    if(2 == idx_bit % font_gray){
                        tmp_c = (((tmp_c >> ((8 - font_gray) + (idx_bit >> 1))) << (idx_bit >> 1)) | ((font_buf[++idx_res] >> (8 - (idx_bit >> 1))))) << (8 - font_gray);
                    }
                    mask_buf[idx_mask_buf + (font_x << 3) + idx_bit] =  (tmp_c >> (8 - font_gray))  * ((0xFF/((0x01 << font_gray)-1)));
                    tmp_c <<= font_gray;
                    if(2 == idx_bit % font_gray){
                        tmp_c = (font_buf[idx_res] << (idx_bit >> 1));
                    }
                }
                ++idx_res;
            }
            idx_mask_buf += font_size;
        }
    }
    else{
        for ( font_y = 0; font_y < font_size; font_y++) {
            for (font_x = 0; font_x < byte_width; font_x++) {
                idx_bit = 7;
                while (idx_bit >= 0) {
                    mask_buf[idx_mask_buf + (font_x << 3) + (7 - idx_bit)] =  ((font_buf[idx_res] >> idx_bit) & 0x01) ? 0xFF : 0x00;
                    --idx_bit;
                }
                ++idx_res;
            }
            idx_mask_buf += font_size;
        }
    }
}

static void _gt_gray_and_dot_width_get(uint8_t* gray , uint8_t* dot_width , gt_font_st font , uint8_t ret_style)
{
    uint8_t byte_width = *dot_width;
    uint8_t font_gray = *gray;
    if( (GT_FONT_TYPE_FLAG_DOT_MATRIX_NON_WIDTH) == ret_style)
    {
        byte_width = (font.size + 7) >> 3;
    }
    else if((GT_FONT_TYPE_FLAG_VEC == ret_style))
    {
        font_gray = font.gray;
        if(1 != font_gray){
            byte_width = ((((font.size * font_gray + 15) >> 4) << 4) / font_gray + 7) >> 3;
        }
        else
        {
            byte_width = (((font.size + 15) >> 4) << 4) >> 3;
        }
    }
    *gray = font_gray ;
    *dot_width = byte_width;
}


static uint32_t _draw_text_convertor(gt_draw_ctx_t * draw_ctx , gt_draw_blend_dsc_st* blend_dsc, const gt_font_st fonts ,\
                                    uint32_t index  , const gt_attr_font_st * dsc, const gt_area_st * coords , gt_size_t start_x)
{
    uint32_t len = 0 , idx = 0 , i , font_dot_w = 0;
    uint32_t unicode , text_len = 0;
    uint16_t *text = NULL;
    gt_font_style_en_cn_et language = 0;
    gt_font_st _font = {
        .size = fonts.size,
        .gray = fonts.gray,
        .style_cn = fonts.style_cn,
        .style_en = fonts.style_en,
        .style_numb = fonts.style_numb,
        .encoding = fonts.encoding,
        .thick_cn = fonts.thick_cn,
        .thick_en = fonts.thick_en,
        .res = NULL,
    };
    uint8_t style_space_y = (_font.size >> 4) ? (_font.size >> 4) : 1;

    i = (NULL == dsc->font->utf8) ? 0 : strlen(dsc->font->utf8);
    if(i < index)
    {
        goto _ret_handle;
    }
    _font.utf8 = &(dsc->font->utf8[index]);
    _font.len = strlen(_font.utf8);
    //
    idx = gt_font_one_char_code_len_get((uint8_t * )&_font.utf8[idx], &unicode, _font.encoding);
    language = _gt_is_style_cn_or_en(unicode, _font.encoding);
    text_len = 1;
    while(idx < _font.len)
    {
        i = gt_font_one_char_code_len_get((uint8_t * )&_font.utf8[idx], &unicode, _font.encoding);
        if(unicode == 0x20 && (language == STYLE_EN_ARABIC)){

        }
        else if(language != _gt_is_style_cn_or_en(unicode, _font.encoding)){
            break;
        }
        idx += i;
        text_len++;
    }
    len = idx;
    //
    text = (uint16_t*)gt_mem_malloc(text_len * sizeof(uint16_t));
    if(NULL == text){
        GT_LOGE(GT_LOG_TAG_GUI, "text malloc failed , size: %d", text_len * sizeof(uint16_t));
        goto _ret_handle;
    }
    idx = 0;
    i = 0;
    while(i < text_len)
    {
        idx += gt_font_one_char_code_len_get((uint8_t * )&_font.utf8[idx], &unicode, _font.encoding);
        text[i] = (uint16_t)unicode;
        ++i;
    }

    uint8_t ret_style = gt_font_type_get(_font.style_en);
    uint8_t width , byte_width, font_gray, x_offset;
    uint32_t one_font_size = 0;
    uint8_t* font_buf = NULL;
    uint32_t font_buf_len = 0;
    if(ret_style == GT_FONT_TYPE_FLAG_VEC){
        font_dot_w = (((_font.size * _font.gray + 15) >> 4) << 4);
    }
    else{
        font_dot_w = (((_font.size + 7) >> 3) << 3);
    }
    //
    if(STYLE_EN_HINDI == language){
        one_font_size = 48;
    }
    else{
        one_font_size = font_dot_w * font_dot_w >> 3;
        if(ret_style == GT_FONT_TYPE_FLAG_DOT_MATRIX_NON_WIDTH)
        {
            one_font_size += 2;
        }
    }
    //
    if(STYLE_EN_HINDI == language){
        font_buf_len = one_font_size * (text_len+1);
    }
    else{
        font_buf_len = one_font_size * 3;
    }
    if(_font.gray != 1){
        font_dot_w = ((font_dot_w / _font.gray) + 7 ) >> 3 << 3;
    }
    // GT_LOGD(GT_LOG_TAG_GUI , "font_dot_w = %d , fontsize = %d , fontgray = %d" , font_dot_w , _font.size  , _font.gray );
    // GT_LOGD(GT_LOG_TAG_GUI , "one_font_size = %d , font_buf_len = %d text_len = %d" , one_font_size , font_buf_len ,text_len);

    font_buf = (uint8_t*)gt_mem_malloc(font_buf_len);
    if(NULL == font_buf){
        GT_LOGE(GT_LOG_TAG_GUI, "buf malloc failed , size: %d" , font_buf_len);
        goto _ret_handle;
    }
    gt_memset_0(font_buf , font_buf_len);

    font_convertor_st convertor = {
        .fontcode = text ,
        .code_len = text_len ,
        .fontsize = _font.size ,
        .fontgray = _font.gray ,
        .font_option = _font.style_en ,
        .thick = _font.thick_en,
        .data = font_buf ,
        .data_len = font_buf_len ,
        .is_rev = _is_align_reverse(dsc->align) ? 0 : 1,
        .is_vec = ret_style == GT_FONT_TYPE_FLAG_VEC ? 1 : 0 ,
    };

    text_len = gt_font_code_transform(&convertor);

    idx = 0;
    i = 1;
    _font.res = &font_buf[font_buf_len - one_font_size];
    while(idx < text_len)
    {
        if(STYLE_EN_HINDI == language){
            width = gt_font_get_word_width_figure_up(&convertor.data[idx * one_font_size] , 16 , 24 , 1);
            i = 1;
        }
        else
        {
            gt_memset_0(&convertor.data[0] , font_buf_len);

            if(0x20 == convertor.fontcode[idx]){
                width = convertor.fontsize >> 1;
            }
            else{
                i = gt_font_convertor_data_get(&convertor , idx);

                width = gt_font_get_word_width_figure_up(&convertor.data[0] , font_dot_w , convertor.fontsize , convertor.fontgray);
            }
        }

        if(_is_align_reverse(dsc->align)) {
            blend_dsc->dst_area->x -= width + dsc->space_x;
            if( blend_dsc->dst_area->x < coords->x ){
                blend_dsc->dst_area->x = start_x - width;
                blend_dsc->dst_area->y += blend_dsc->dst_area->h + dsc->space_y + (style_space_y << 1);
                if( (blend_dsc->dst_area->y + blend_dsc->dst_area->h + dsc->space_y + style_space_y) > (coords->y + coords->h) ){
                    break;
                }
            }
        }

        if( !gt_area_is_intersect_screen(&draw_ctx->buf_area, blend_dsc->dst_area) ){
            goto _lab_nex_word;
        }

        if(STYLE_EN_HINDI == language){
            byte_width = 2 ;
            font_gray = 1;
            gt_memset_0(blend_dsc->mask_buf, _font.size * _font.size);
            _push_mask_buf(blend_dsc->mask_buf, &convertor.data[idx * one_font_size], _font.size, font_gray, byte_width);
        }
        else{
            byte_width = (gt_font_get_one_word_width(convertor.fontcode[idx] , &_font) + 7 ) >> 3;
            font_gray = 1;
            _gt_gray_and_dot_width_get(&font_gray , &byte_width , _font , ret_style);
            gt_memset_0(blend_dsc->mask_buf, _font.size * _font.size);
            _push_mask_buf(blend_dsc->mask_buf, &convertor.data[0], _font.size, font_gray, byte_width);
        }
        // GT_LOGD( GT_LOG_TAG_GUI , "[%d]width = %d byte_width = %d , font_gray = %d , _font.size = %d" , idx , width , byte_width , font_gray , _font.size );

        gt_draw_blend(draw_ctx, blend_dsc);

_lab_nex_word:
        if(!_is_align_reverse(dsc->align)) {
            blend_dsc->dst_area->x += width + dsc->space_x;
        }

        idx += i;

        if(!_is_align_reverse(dsc->align)) {
            width = 0;
            if(GT_ALIGN_RIGHT == (dsc->align & 0x7F)){
                width = gt_font_get_one_word_width(convertor.fontcode[idx], &_font);
            }
            /* check whether the boundary is reached */
            if( (blend_dsc->dst_area->x + (width <= 0 ? blend_dsc->dst_area->w : width) + dsc->space_x) > (coords->x + coords->w) ){
_lab_next_line:
                // blend_dsc->dst_area->x = coords->x;
                blend_dsc->dst_area->x = start_x;
                blend_dsc->dst_area->y += blend_dsc->dst_area->h + dsc->space_y + (style_space_y << 1);
                if( (blend_dsc->dst_area->y + blend_dsc->dst_area->h + dsc->space_y + style_space_y) > (coords->y + coords->h) ){
                    break;
                }
            }
        }
    }

_ret_handle:
    if(NULL != text){
        gt_mem_free(text);
        text = NULL;
    }

    if(NULL != font_buf){
        gt_mem_free(font_buf);
        font_buf = NULL;
        _font.res = NULL;
    }

    return len;
}

gt_area_st draw_text(gt_draw_ctx_t * draw_ctx, const gt_attr_font_st * dsc, const gt_area_st * coords)
{
    static uint32_t font_size_pre = 0;
    uint8_t * mask_buf = NULL, * data = NULL, * mask_line = NULL;
    gt_size_t start_x = 0;
    gt_area_st area_font = {
        .x = coords->x, .y = coords->y, .w = dsc->font->size, .h = dsc->font->size,
    };

    if( dsc->font->len == 0 ){
        return area_font;
    }

    uint32_t idx = 0;
    gt_font_st _font = {
        .utf8       = dsc->font->utf8,
        .size       = dsc->font->size,
        .style_cn   = dsc->font->style_cn,
        .style_en   = dsc->font->style_en,
        .style_numb = dsc->font->style_numb,
        .encoding   = dsc->font->encoding,
        .res        = NULL,
        .thick_cn   = dsc->font->thick_cn,
        .thick_en   = dsc->font->thick_en,
    };
    _font.gray = dsc->font->gray == 0 ? 1 : dsc->font->gray;

    uint32_t font_dot_w = (((_font.size + 15) >> 4) << 4) * _font.gray;
    uint32_t len_text = dsc->font->len, unicode = 0;
    gt_size_t w = coords->w, h = coords->h;
    uint8_t style_space_y = (_font.size >> 4) ? (_font.size >> 4) : 1;
    uint8_t * text = (uint8_t * )dsc->font->utf8;

#if 0
    if( font_size_pre != font_dot_w){
        font_size_pre = font_dot_w;
        /*free*/
        gt_mem_free(data);
        gt_mem_free(mask_buf);
        gt_mem_free(mask_line);

        /*malloc*/
        data = gt_mem_malloc(font_dot_w * font_dot_w >> 3); // (dsc->font->size * dsc->font->size >> 3);
        if(NULL == data){
            goto _ret_handler;
        }
        mask_buf = gt_mem_malloc(font_dot_w * font_dot_w);
        if(NULL == mask_buf){
            goto _ret_handler;
        }
        mask_line = gt_mem_malloc( _font.size*2 );
        if(NULL == mask_line){
            goto _ret_handler;
        }
    }
#else
    /*malloc*/
    data = gt_mem_malloc(font_dot_w * font_dot_w >> 3); // (dsc->font->size * dsc->font->size >> 3);
    if(NULL == data){
        goto _ret_handler;
    }
    mask_buf = gt_mem_malloc(font_dot_w * font_dot_w);
    if(NULL == mask_buf){
        goto _ret_handler;
    }
    mask_line = gt_mem_malloc( _font.size << 1 );
    if(NULL == mask_line){
        goto _ret_handler;
    }
#endif
    _font.res = data;
    gt_memset(_font.res, 0, (font_dot_w * font_dot_w) >>3);

    gt_area_st area_font_style;

    _font.len = strlen(_font.utf8);
    uint16_t width_font = gt_font_get_string_width(&_font);

    /*text align*/
    switch( dsc->align & 0x7F ) {
        case GT_ALIGN_RIGHT_BOTTOM:
            area_font.y = area_font.y + ((h - _font.size) >> 1);
        case GT_ALIGN_RIGHT_MID:
            area_font.y = area_font.y + ((h - _font.size) >> 1);
        case GT_ALIGN_RIGHT:
            if( width_font < w ){
                area_font.x = area_font.x + (w - width_font - 8);
            }
            break;
        case GT_ALIGN_CENTER_BOTTOM:
            area_font.y = area_font.y + ((h - _font.size) >> 1);
        case GT_ALIGN_CENTER_MID:
            area_font.y = area_font.y + ((h - _font.size) >> 1);
        case GT_ALIGN_CENTER:
            if(width_font < w) {
                area_font.x = area_font.x + ((w - width_font) >> 1);
            }
            break;
        case GT_ALIGN_LEFT_BOTTOM:
            area_font.y = area_font.y + ((h - _font.size) >> 1);
        case GT_ALIGN_LEFT_MID:
            area_font.y = area_font.y + ((h - _font.size) >> 1);
            break;
        case GT_ALIGN_LEFT:
        case GT_ALIGN_NONE:
        default:
            break;
    }

    if(_is_align_reverse(dsc->align)) {
        area_font.x = (coords->x + coords->w) - (area_font.x - coords->x);// - area_font.w;
    }

    start_x = area_font.x;

    if( dsc->start_x || dsc->start_y ){
        area_font.x = dsc->start_x;
        area_font.y = dsc->start_y;
    }

    gt_draw_blend_dsc_st blend_dsc_style;
    gt_draw_blend_dsc_st blend_dsc = {
        .mask_buf   = mask_buf,
        .dst_area   = &area_font,
        .color_fill = dsc->font_color,
        .opa        = dsc->opa,
    };
    gt_memset(mask_line, 0xFF, _font.size << 1);

    blend_dsc_style = blend_dsc;
    blend_dsc_style.color_fill  = gt_color_black();
    blend_dsc_style.mask_buf    = mask_line;


    gt_area_st area_res;
    uint8_t width;
    uint16_t size_mask = font_dot_w * font_dot_w, size_fres = size_mask >> 3, idx_res = 0;

    /* foreach text */
    while( idx < len_text ){
        gt_memset(_font.res, 0, size_fres);

        _font.utf8 = (char * )&text[idx];
        // _font.len = _gt_utf8_to_unicode(&text[idx], &unicode);
        _font.len = gt_font_one_char_code_len_get(&text[idx], &unicode , _font.encoding);
        if( _font.utf8[0] == '\n' ){
            idx += _font.len;
            goto _lab_next_line;
        }

        // convertor
        if(gt_style_en_data_is_convertor(unicode , _font.encoding)){

            idx += _draw_text_convertor(draw_ctx , &blend_dsc , _font , idx , dsc , coords , start_x);
            continue;
        }

        width = gt_font_get_one_word_width(unicode, &_font);

        if(_is_align_reverse(dsc->align)) {
            area_font.x -= width + dsc->space_x;
            if( area_font.x < coords->x ){
                area_font.x = start_x - width;
                area_font.y += area_font.h + dsc->space_y + (style_space_y << 1);
                if( (area_font.y + area_font.h + dsc->space_y + style_space_y) > (coords->y + coords->h) ){
                    break;
                }
            }
        }

        if( !gt_area_intersect_screen(&draw_ctx->buf_area, &area_font, &area_res) ){
            goto _lab_nex_word;
        }

        if( !gt_symbol_check_by_unicode(unicode) ){
            if (width < _font.size) {
                gt_memset(mask_buf, 0, font_dot_w * font_dot_w);
            }
            gt_memset(_font.res, 0, size_fres);
            uint16_t ret_style = gt_font_get_string_dot(&_font);

            // dot to mask_buf
            uint8_t byte_width = (width + 7) >> 3 ;
            uint8_t font_gray = 1;

            _gt_gray_and_dot_width_get(&font_gray , &byte_width , _font , ret_style);
            _push_mask_buf(mask_buf , _font.res , _font.size , font_gray , byte_width);

        }else{
            const uint8_t * temp = gt_symbol_get_mask_buf(_font.utf8 , _font.size);
            if( temp == NULL ){
                area_font.x += width + dsc->space_x;
                idx += _font.len;
                continue;
            }
            gt_memcpy(mask_buf, temp, _font.size*_font.size);
        }

        gt_draw_blend(draw_ctx, &blend_dsc);

        /* use style */
        if( dsc->style & GT_FONT_STYLE_UNDERLINE ){
            area_font_style = area_font;
            area_font_style.y += _font.size + 1;
            area_font_style.h = style_space_y;
            area_font_style.w = width;
            blend_dsc_style.color_fill = dsc->font_color;
            blend_dsc_style.dst_area = &area_font_style;
            gt_draw_blend(draw_ctx, &blend_dsc_style);
        }
        if( dsc->style & GT_FONT_STYLE_STRIKETHROUGH ){
            area_font_style = area_font;
            area_font_style.y += (_font.size >> 1);
            area_font_style.h = style_space_y;
            area_font_style.w = width;
            blend_dsc_style.color_fill = dsc->font_color;
            blend_dsc_style.dst_area = &area_font_style;
            gt_draw_blend(draw_ctx, &blend_dsc_style);
        }

        _lab_nex_word:
        if(!_is_align_reverse(dsc->align)) {
            area_font.x += width + dsc->space_x;
        }
        idx += _font.len;

        /* check whether the boundary is reached */
        if(!_is_align_reverse(dsc->align)) {
            width = 0;
            if(GT_ALIGN_RIGHT == (dsc->align & 0x7F)){
                gt_font_one_char_code_len_get(&text[idx], &unicode , _font.encoding);
                width = gt_font_get_one_word_width(unicode, &_font);
            }
            if( (area_font.x + (width <= 0 ? area_font.w : width) + dsc->space_x) > (coords->x + coords->w) ){
            _lab_next_line:
                // area_font.x = coords->x;
                area_font.x = start_x;
                area_font.y += area_font.h + dsc->space_y + (style_space_y << 1);
                if( (area_font.y + area_font.h + dsc->space_y + style_space_y) > (coords->y + coords->h) ){
                    break;
                }
            }
        }
    }

_ret_handler:
    if(NULL != mask_line){
        gt_mem_free(mask_line);
        mask_line = NULL;
    }
    if(NULL != mask_buf){
        gt_mem_free(mask_buf);
        mask_buf = NULL;
    }
    if(NULL != data){
        gt_mem_free(data);
        data = NULL;
    }
    return area_font;
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
