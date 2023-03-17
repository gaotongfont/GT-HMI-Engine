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



/* static functions -----------------------------------------------------*/
static void gt_mirror_hor_u8(uint8_t * dst, uint8_t * src, uint32_t w, uint32_t h)
{
    int x, y, idx_dst = 0, idx_src = 0;
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
    int x, y, idx_dst = 0, idx_src = 0;
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
static const uint8_t radius1[9] = {0X24,0X69,0X24,0X69,0xFF,0x69,0x24,0x69,0x24};
static const uint8_t radius2[16] = {0X24,0X69,0X69,0X24, 0x69,0x6D,0x6D,0X69, 0X69,0x6D,0x6D,0X69, 0X24,0X69,0X69,0X24};
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
    {0, radius1, 9},
    {1, radius1, 9},
    {2, radius2, 16},
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
    if(radius <= GT_RADIUS_MIN){
        blend_dsc->mask_area->w = (radius == 2) ? 4: 3;
        blend_dsc->mask_area->h = blend_dsc->mask_area->w;
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
	int x1,x2,y1,y2;		//two point get a line

	x1 = line_attr->x_1;
	y1 = line_attr->y_1;

	x2 = line_attr->x_2;
	y2 = line_attr->y_2;

	int dx =  abs(x2-x1), sx = x1<x2 ? 1 : -1;
	int dy = -abs(y2-y1), sy = y1<y2 ? 1 : -1;
	int err = dx+dy, e2; /* error value e_xy */

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

void draw_bg(gt_draw_ctx_t * draw_ctx, const gt_attr_rect_st * dsc, const gt_area_st * coords)
{
    if (dsc->bg_opa <= GT_OPA_MIN) return;

    gt_area_st bg_coords, area_res;
    gt_area_copy(&bg_coords, (gt_area_st *)coords);

    if( !gt_area_intersect_screen(draw_ctx->buf_area, &bg_coords, &area_res) ){
        return;
    }

    static uint8_t radius_pre = 0;
    static gt_color_t * color_circle_p = NULL;
    static uint8_t * mask_left_up = NULL;

    // calc radius
    uint8_t radius = GT_MIN(bg_coords.w >> 1, bg_coords.h >> 1);
    radius = GT_MIN(radius, dsc->radius);
    uint8_t border_width = dsc->border_width;
    uint8_t is_radius_more_than_border = radius > border_width ? true : false;
    // if( border_width > radius && radius ){
    //     border_width = radius;
    // }

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
        if(NULL == mask_buf) {
            GT_LOGE(GT_LOG_TAG_GUI , "mask buf malloc %d err, w: %d, h: %d!", len, coords->w, coords->h);
            return ;
        }
        blend_dsc.mask_buf = mask_buf;
        blend_dsc.mask_area->w = coords->w;
        blend_dsc.mask_area->h = coords->h;
        GT_LOGD(GT_LOG_TAG_GUI , "dsc->gray = %d , w = %d , h = %d , buf_size = %d" ,dsc->gray , blend_dsc.mask_area->w , blend_dsc.mask_area->h , len);

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

        if(mask_buf)
        {
            gt_mem_free(mask_buf);
        }
        mask_buf = NULL;
        return ;
    }

    /* draw circle or draw a line*/
    if(radius > 0 ){
        if (radius <= GT_RADIUS_MIN)   //Get the whole circle info
        {
            if( radius != radius_pre ){
                radius_pre = radius;
                /*free*/
                gt_mem_free(color_circle_p);
                gt_mem_free(mask_left_up);

                /*malloc*/
                color_circle_p = gt_mem_malloc(radius_dat[radius].size);;
                mask_left_up = gt_mem_malloc(radius_dat[radius].size);
            }
            gt_memcpy(mask_left_up, radius_dat[radius].ptr, radius_dat[radius].size);
            blend_dsc.mask_buf = mask_left_up;
        }else {//Get a quarter circle info
            uint32_t mem_size = (radius << 1) * (radius << 1) * sizeof(gt_color_t);
            if( radius != radius_pre ){
                radius_pre = radius;
                /*free*/
                gt_mem_free(color_circle_p);
                gt_mem_free(mask_left_up);

                /*malloc*/
                color_circle_p = gt_mem_malloc(mem_size);
                mask_left_up = gt_mem_malloc(radius * radius);
            }
            if(radius > 12 ){
                gt_attr_circle_st circle_attr = {
                    .fg_color       = gt_color_white(),
                    .bg_color       = gt_color_black(),
                    .radius         = radius,
                    .border_width   = border_width,
                    .border_color   = gt_color_white(),
                    .is_fill        = dsc->is_fill,
                };

                gt_area_st circle_area = {
                    .w = radius << 1,
                    .h = radius << 1
                };
                gt_graph_circle(&circle_attr, &circle_area, color_circle_p);
                // get circle mask
                uint32_t idx = 0, all = circle_area.w * circle_area.h >> 1; // half of circle
                uint8_t * mask = (uint8_t * )color_circle_p;
                uint16_t x, y;
                while (idx < all) {
                    mask[idx] = gt_color_brightness(color_circle_p[idx]);
                    idx++;
                }

                // get circle left mask
                blend_dsc.mask_buf = mask_left_up;
                idx = 0;
                uint32_t idx_circle = 0;
                for (y = 0; y < radius; y++) {
                    for (x = 0; x < radius; x++) {
                        mask_left_up[idx++] = mask[idx_circle++];
                    }
                    idx_circle += radius;
                }
            }else{
                gt_memcpy(mask_left_up, radius_dat[radius].ptr, radius_dat[radius].size);
                blend_dsc.mask_buf = mask_left_up;
                if(false == is_radius_more_than_border){    /* draw circle with border_color*/
                    blend_dsc.color_fill = dsc->border_color;
                    gt_draw_arch(draw_ctx,&blend_dsc,mask_left_up,(uint8_t *)color_circle_p,coords);
                }else { 
                    if(dsc->is_fill == true){   /* draw circle with fill bg_color*/
                        gt_draw_arch(draw_ctx,&blend_dsc,mask_left_up,(uint8_t *)color_circle_p,coords);
                    }
                    if(dsc->border_width != 0){ //need draw border ring with border_color
                        blend_dsc.mask_area->x = coords->x;
                        blend_dsc.mask_area->y = coords->y;
                        blend_dsc.color_fill = dsc->border_color;
                        int radius_hid = radius - dsc->border_width;
                        if( dsc->border_width != 0 && radius_hid >= 3 && dsc->border_width < radius ){
                            uint8_t * mask_hid = gt_mem_malloc( radius_hid * radius_hid );
                            gt_memcpy(mask_hid, radius_dat[radius_hid].ptr, radius_dat[radius_hid].size);

                            int bidx = 0, sidx = 0;
                            int by = radius;
                            for( int sy = radius_hid - 1; sy > 0; sy-- ){
                                bidx = (by * radius) - 1;
                                sidx = (sy * radius_hid) -1;
                                for( int sx = radius_hid-1; sx >= 0; sx--){
                                    // mask_left_up[ bidx-- ] = (255 - mask_hid[sidx--]);
                                    if( mask_hid[sidx--] != 0 ){
                                        mask_left_up[ bidx-- ] = 0;
                                    }
                                }
                                by--;
                            }
                            gt_mem_free(mask_hid);
                        }else if( radius_hid == 2){
                            mask_left_up[ radius * radius - 1 ] = 0;
                            mask_left_up[ (radius-1) * radius - 1 ] = 0;
                            mask_left_up[ (radius-1) * radius - 2 ] = 0;
                            mask_left_up[ (radius-1) * radius + radius - 1 ] = 0;
                            mask_left_up[ (radius-1) * radius + radius - 2 ] = 0;
                        }else if( radius_hid == 1){
                            mask_left_up[ radius * radius - 1 ] = 0;
                        }
                        blend_dsc.mask_buf = mask_left_up;
                        /* draw border */
                        gt_draw_arch(draw_ctx,&blend_dsc,mask_left_up,(uint8_t *)color_circle_p,coords);
                    }
                }
            }
		}
        /* draw circle */
        // gt_draw_arch(draw_ctx,&blend_dsc,mask_left_up,(uint8_t *)color_circle_p,coords);

        if(dsc->is_line) {  /* draw line */
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
        area_mask.w = border_width;
        area_mask.h = coords->h - (radius << 1);
        gt_draw_blend(draw_ctx, &blend_dsc);

        // 4 draw right border
        area_mask.x = (coords->x + coords->w) - border_width;
        gt_draw_blend(draw_ctx, &blend_dsc);
    }

    /* draw rect */
    if (dsc->is_fill) {
        blend_dsc.color_fill = dsc->bg_color;

        // 1:draw up rect
        area_mask.x = coords->x + radius;
        area_mask.y = coords->y + border_width;
        area_mask.w = coords->w - (radius << 1);
		area_mask.h = radius > border_width ? radius - border_width : 0;
        gt_draw_blend(draw_ctx, &blend_dsc);

        // 2:draw center rect
        area_mask.x = coords->x + border_width;
        area_mask.y = coords->y + (radius > border_width ? radius : border_width);
        area_mask.w = coords->w - (border_width<<1);
        area_mask.h = coords->h - (radius > border_width ? (radius << 1) : (border_width << 1));
        gt_draw_blend(draw_ctx, &blend_dsc);

        // 3:draw bottom rect
        area_mask.x = coords->x + radius;
        area_mask.y = area_mask.y + area_mask.h;
        area_mask.w = coords->w - (radius << 1);
        area_mask.h = (coords->y + coords->h - border_width) - area_mask.y;
        gt_draw_blend(draw_ctx, &blend_dsc);
    }
}


gt_area_st draw_text(gt_draw_ctx_t * draw_ctx, const gt_attr_font_st * dsc, const gt_area_st * coords)
{
    static uint8_t * mask_buf = NULL, * data = NULL, font_size_pre = 0, * mask_line = NULL;

    gt_area_st area_font = {
        .x = coords->x, .y = coords->y, .w = dsc->font->size, .h = dsc->font->size,
    };

    if( dsc->font->len == 0 ){
        return area_font;
    }
    gt_size_t w = coords->w, h = coords->h;
    uint8_t * text = (uint8_t * )dsc->font->utf8;
    uint32_t len_text = dsc->font->len, unicode = 0;
    if( font_size_pre != dsc->font->size ){
        font_size_pre = dsc->font->size;
        /*free*/
        gt_mem_free(data);
        gt_mem_free(mask_buf);
        gt_mem_free(mask_line);

        /*malloc*/
        data = gt_mem_malloc(dsc->font->size * dsc->font->size >> 3);
        mask_buf = gt_mem_malloc(dsc->font->size * dsc->font->size);
        mask_line = gt_mem_malloc( dsc->font->size*2 );
    }
    uint32_t idx = 0;
    gt_font_st _font = {
        .res        = data,
        .size       = dsc->font->size,
        .style_cn   = dsc->font->style_cn,
        .style_en   = dsc->font->style_en,
    };


    gt_area_st area_font_style;
    uint16_t width_font = gt_font_get_string_width(dsc->font);

    /*text align*/
    switch( dsc->align ){
        case GT_ALIGN_NONE:
            break;
        case GT_ALIGN_LEFT:
            area_font.y = area_font.y + ((h - dsc->font->size)>>1);
            break;
        case GT_ALIGN_RIGHT:
            area_font.y = area_font.y + ((h - dsc->font->size)>>1);
            if( width_font < w ){
                area_font.x = area_font.x + (w - width_font - 8);
            }
            break;
        case GT_ALIGN_CENTER:
        default:
            if(width_font < w)
            {
                area_font.x = area_font.x + ((w - width_font)>>1);
                area_font.y = area_font.y + ((h - dsc->font->size)>>1);
                break;
            }
            // area_font.x = area_font.x + ((w - width_font)>>1);
            area_font.y = area_font.y + ((h - dsc->font->size)>>1);
            break;
    }

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
    gt_memset(mask_line, 0xFF, dsc->font->size << 1);

    blend_dsc_style = blend_dsc;
    blend_dsc_style.color_fill  = gt_color_black();
    blend_dsc_style.mask_buf    = mask_line;


    gt_area_st area_res;
    int idx_bit = 0,idx_mask_buf = 0;
    uint8_t width;
    uint16_t size_mask = dsc->font->size * dsc->font->size, size_fres = size_mask >> 3, idx_res = 0;

    /* foreach text */
    while( idx < len_text ){
        gt_memset(_font.res, 0, size_fres);

        _font.utf8 = (char * )&text[idx];
        _font.len = _gt_utf8_to_unicode(&text[idx], &unicode);

        if( _font.utf8[0] == '\n' ){
            idx += _font.len;
            goto _lab_next_line;
        }


        width = gt_font_get_one_word_width(unicode, dsc->font);

        if( !gt_area_intersect_screen(draw_ctx->buf_area, &area_font, &area_res) ){
            goto _lab_nex_word;
        }

        if( !gt_symbol_check_by_unicode(unicode) ){
            if (width < dsc->font->size) {
                gt_memset(mask_buf, 0, dsc->font->size * dsc->font->size);
            }
            uint16_t ret_style = gt_font_get_string_dot(&_font);

            // dot to mask_buf
            idx_mask_buf = 0;
            idx_res = 0;
            uint8_t byte_width = \
                (GT_FONT_TYPE_FLAG_DOT_MATRIX_NON_WIDTH == ret_style) ? (dsc->font->size >> 3) : ((width + 7) >> 3);
            for (uint16_t font_y = 0, font_x = 0; font_y < dsc->font->size; font_y++) {
                for (font_x = 0; font_x < byte_width; font_x++) {
                    idx_bit = 7;
                    while (idx_bit >= 0) {
                        mask_buf[idx_mask_buf + (font_x << 3) + (7 - idx_bit)] =  ((_font.res[idx_res] >> idx_bit) & 0x01) ? 0xFF : 0x00;
                        --idx_bit;
                    }
                    ++idx_res;
                }
                idx_mask_buf += dsc->font->size;
            }
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
            area_font_style.y += dsc->font->size + 1;
            area_font_style.h = 2;
            area_font_style.w = width;
            blend_dsc_style.color_fill = gt_color_black();
            blend_dsc_style.dst_area = &area_font_style;
            gt_draw_blend(draw_ctx, &blend_dsc_style);
        }
        if( dsc->style & GT_FONT_STYLE_STRIKETHROUGH ){
            blend_dsc_style.color_fill = gt_color_red();
            area_font_style = area_font;
            area_font_style.y += (dsc->font->size >> 1);
            area_font_style.h = 2;
            area_font_style.w = width;
            blend_dsc_style.dst_area = &area_font_style;
            gt_draw_blend(draw_ctx, &blend_dsc_style);
        }

        _lab_nex_word:
        area_font.x += width + dsc->space_x;
        idx += _font.len;

        /* check whether the boundary is reached */
        if( (area_font.x + area_font.w + dsc->space_x) > (coords->x + coords->w) ){
        _lab_next_line:
            area_font.x = coords->x;
            area_font.y += area_font.h + dsc->space_y;
            if( (area_font.y + area_font.h + dsc->space_y) > (coords->y + coords->h) ){
                break;
            }
        }

    }

    return area_font;
}

void draw_bg_img(gt_draw_ctx_t * draw_ctx, const gt_attr_rect_st * dsc, const gt_area_st * coords)
{
    if( dsc->bg_img_src == NULL ) return;
    if( dsc->bg_opa < GT_OPA_MIN ) return;

    _gt_img_dsc_st dsc_img;
    if ( GT_RES_FAIL == gt_img_decoder_open(&dsc_img, dsc->bg_img_src, 0)) {
        GT_LOGW(GT_LOG_TAG_DATA, "Open image decoder failed\n");
        return ;
    }

    gt_area_st dst_area = *coords, mask_area = *coords;
    dst_area.h = 1;

    gt_draw_blend_dsc_st blend_dsc = {
        .dst_area = &dst_area,
        .mask_area = &mask_area,
        .opa = dsc->bg_opa
    };

    uint32_t addr_start_alpha = 0;
    uint32_t offset = 0, len = 0;
    gt_color_t * color_img_line_p = NULL;
    uint16_t w, h, y = 0;
    uint8_t * mask_buf = NULL;

    w = dsc_img.header.w;
    h = dsc_img.header.h;
    addr_start_alpha = w * h * sizeof(gt_color_t);
    /* img src type */
    switch(dsc_img.header.type) {

        case GT_IMG_DECODER_TYPE_PNG: {
            /* line by line to blend */
            blend_dsc.mask_buf = &dsc_img.alpha[0];
            blend_dsc.dst_buf = (gt_color_t *)&dsc_img.img[0];
            while(y < h){
                if( gt_area_is_intersect_screen(draw_ctx->buf_area, &dst_area) ){
                    /* blend to bg */
                    gt_draw_blend(draw_ctx, &blend_dsc);
                }
                blend_dsc.mask_buf += w;
                blend_dsc.dst_buf += w;
                ++y;
                ++dst_area.y;
                ++mask_area.y;
            }
            break;
        }
        default: {
            // gt format
            gt_fs_fp_st * fp = gt_fs_get_fp();
            if (!fp) { break; }
            uint8_t is_mask = 1 == fp->msg.pic.is_alpha ? 1 : 0;

            len = w * sizeof(gt_color_t);
            color_img_line_p = gt_mem_malloc( len );
            if (!color_img_line_p) { break; }

            if (is_mask) {
                mask_buf = gt_mem_malloc(w);
                if (!mask_buf) {
                    gt_mem_free(color_img_line_p);
                    break;
                }
                blend_dsc.mask_buf = mask_buf;
            } else {
                blend_dsc.mask_buf = NULL;  /* default: bmp */
            }
            blend_dsc.dst_buf = color_img_line_p;

            while(y < h){
                if (gt_area_is_intersect_screen(draw_ctx->buf_area, &dst_area)) {
                    /*read one line color data*/
                    gt_fs_read_img_offset((uint8_t *)color_img_line_p, offset, len);
                    if(is_mask) {    //png
                        gt_fs_read_img_offset((uint8_t *)mask_buf, addr_start_alpha, w);
                        addr_start_alpha += w;
                    }
                    /* blend to bg */
                    gt_draw_blend(draw_ctx, &blend_dsc);
                }
                ++y;
                ++dst_area.y;
                ++mask_area.y;
                offset += len;
            }

            gt_mem_free(color_img_line_p);
            if (is_mask) {
                gt_mem_free(mask_buf);
            }
            break;
        }
    }

    if (dsc_img.decoder) {
        gt_img_decoder_close(&dsc_img);
    }
}

/* end ------------------------------------------------------------------*/
