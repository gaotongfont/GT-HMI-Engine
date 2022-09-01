/**
 * @file gt_draw.c
 * @author yongg
 * @brief
 * @version 0.1
 * @date 2022-08-09 18:29:24
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */

 /* include --------------------------------------------------------------*/
#include "stdlib.h"
#include "stdbool.h"

#include "gt_graph_base.h"
#include "../hal/gt_hal_disp.h"
#include "gt_mem.h"
#include "../others/gt_math.h"
#include "../others/gt_log.h"
#include "../others/gt_effects.h"
#include "../widgets/gt_obj.h"
#include "gt_draw.h"
#include "../../extra/draw/gt_draw_blend.h"
#include "../others/gt_color.h"
#include "../others/gt_area.h"
#include "gt_vf.h"

#include "gt_symbol.h"
/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/


/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
void gt_mirror_hor_u8(uint8_t * dst, uint8_t * src, uint32_t w, uint32_t h)
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

void gt_mirror_ver_u8(uint8_t * dst, uint8_t * src, uint32_t w, uint32_t h)
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

    //   draw_bg(draw_ctx, dsc, coords);
        // draw_bg_img(draw_ctx, dsc, coords);

        // draw_border(draw_ctx, dsc, coords);

        // draw_outline(draw_ctx, dsc, coords);

}



/*  static func --------------------------------------------------------*/
/**
 * @brief draw rect backgrounp
 * 
 * @param draw_ctx 
 * @param dsc 
 * @param coords 
 */
void draw_bg(gt_draw_ctx_t * draw_ctx, const gt_attr_rect_st * dsc, const gt_area_st * coords)
{
    if (dsc->bg_opa <= GT_OPA_MIN) return;

    gt_area_st bg_coords, area_res;
    gt_area_copy(&bg_coords, coords);

    if( !gt_area_intersect(draw_ctx->buf_area, &bg_coords, &area_res) ){
        return;
    }
    
    static uint8_t radius_pre = 0;
    static gt_color_t * color_circle_p = NULL;
    static uint8_t * mask_left_up = NULL;

    // calc radius
    uint8_t radius = GT_MIN(bg_coords.w >> 1, bg_coords.h >> 1);
    radius = GT_MIN(radius, dsc->radius);
    uint8_t border_width = dsc->border_width;

    if( border_width > radius && radius ){
        border_width = radius;
    }

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

    if (radius <= GT_RADIUS_MIN) {
        radius = 0;
    }
    /* draw circle */
    if (radius > GT_RADIUS_MIN) {
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
        uint8_t * mask = color_circle_p;
        uint8_t x, y;
        int idx = 0, all = circle_area.w * circle_area.h;
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

        if (dsc->bg_opa > GT_OPA_MIN) {
            idx = 0;
            all = radius * radius;
            while (idx < all) {
                mask_left_up[idx] = mask_left_up[idx] * ((float)dsc->bg_opa / 255.0);
                idx++;
            }
        }

        // 1:draw left-up arch        
        gt_draw_blend(draw_ctx, &blend_dsc);

        // 2:draw right_up arch
        uint8_t * mask_right_up = color_circle_p;
        gt_mirror_hor_u8(mask_right_up, mask_left_up, radius, radius);

        area_mask.x = (coords->x + coords->w) - radius;
        blend_dsc.mask_buf = mask_right_up;
        gt_draw_blend(draw_ctx, &blend_dsc);

        // 3:draw right_down arch
        uint8_t * mask_right_down = mask_left_up;
        gt_mirror_ver_u8(mask_right_down, mask_right_up, radius, radius);

        area_mask.y = (coords->y + coords->h) - radius;
        blend_dsc.mask_buf = mask_right_down;
        gt_draw_blend(draw_ctx, &blend_dsc);

        // 4:draw right_up arch
        uint8_t * mask_left_down = mask_right_up;
        gt_mirror_hor_u8(mask_left_down, mask_right_down, radius, radius);

        area_mask.x = coords->x;
        blend_dsc.mask_buf = mask_left_down;
        gt_draw_blend(draw_ctx, &blend_dsc);

    }

    /* draw border */
    if (border_width > 0) {
        blend_dsc.mask_buf      = NULL;
        blend_dsc.dst_buf       = NULL;
        blend_dsc.color_fill    = dsc->border_color;
        
        // 1:draw up border
        area_mask.x = coords->x + radius;
        area_mask.y = coords->y;
        area_mask.w = coords->w - (radius << 1);
        area_mask.h = border_width;
        gt_draw_blend(draw_ctx, &blend_dsc);

        // 2:draw bottom border
        area_mask.x = coords->x + radius;
        area_mask.y = (coords->y + coords->h) - border_width;
        area_mask.w = coords->w - (radius << 1);
        area_mask.h = border_width;
        gt_draw_blend(draw_ctx, &blend_dsc);

        // 3 draw left border
        area_mask.x = coords->x;
        area_mask.y = coords->y + radius;
        area_mask.w = border_width;
        area_mask.h = coords->h - (radius << 1);
        gt_draw_blend(draw_ctx, &blend_dsc);

        // 4 draw right border
        area_mask.x = (coords->x + coords->w) - border_width;
        area_mask.y = coords->y + radius;
        area_mask.w = border_width;
        area_mask.h = coords->h - (radius << 1);
        gt_draw_blend(draw_ctx, &blend_dsc);
    }
    
    /* draw rect */
    if (dsc->is_fill) {
        blend_dsc.mask_buf = NULL;
        blend_dsc.dst_buf = NULL;
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
    int w = coords->w, h = coords->h, x = coords->x, y = coords->y;
    uint8_t * text = dsc->font->utf8;
    uint32_t len_text = dsc->font->len, unicode = 0;
    if( font_size_pre != dsc->font->size ){
        font_size_pre = dsc->font->size;
        /*free*/
        gt_mem_free(data);
        gt_mem_free(mask_buf);
        gt_mem_free(mask_line);

        /*malloc*/
        data = gt_mem_malloc(dsc->font->size * dsc->font->size / 8);
        mask_buf = gt_mem_malloc(dsc->font->size * dsc->font->size);
        mask_line = gt_mem_malloc( dsc->font->size*2 );
    }
    uint8_t * temp = NULL;
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
            area_font.x = area_font.x + ((w - width_font)>>1);
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
        .opa        = GT_OPA_COVER,
    };
    
    
    gt_memset(mask_line, 0xFF, dsc->font->size*2);

    blend_dsc_style = blend_dsc;
    blend_dsc_style.color_fill  = gt_color_black();
    blend_dsc_style.mask_buf    = mask_line;

    
    gt_area_st area_res;
    int idx_bit = 0,idx_mask_buf = 0;
    uint8_t width;
    uint16_t size_mask = dsc->font->size * dsc->font->size, size_fres = size_mask / 8, idx_res = 0;
    
    /* foreach text */
    while( idx < len_text ){
        gt_memset(_font.res, 0, size_fres);

        _font.utf8 = &text[idx];
        _font.len = utf8_to_unicode(&text[idx], &unicode);
                

        if( _font.utf8[0] == '\n' ){
            idx += _font.len;
            goto _lab_next_line;
        }
        

        width = gt_font_get_one_word_width(unicode, dsc->font->size);

        if( !gt_area_intersect(draw_ctx->buf_area, &area_font, &area_res) ){            
            goto _lab_nex_word;
        }

        if( !gt_symbol_check_by_unicode(unicode) ){
           
            gt_font_get_string_dot(&_font);
            
            // dot to mask_buf
            idx_mask_buf = 0;
            idx_res = 0;
            while( idx_res < size_fres ){
                idx_bit = 7;
                while( idx_bit >= 0 ){
                    if( (_font.res[idx_res] >> idx_bit)&0x1 ){
                        mask_buf[idx_mask_buf++] = 0xFF;
                    }else{
                        mask_buf[idx_mask_buf++] = 0x00;
                    }
                    idx_bit--;
                }
                idx_res++;
            }
        }else{
            temp = gt_symbol_get_mask_buf(_font.utf8);
            if( temp == NULL ){
                area_font.x += width + dsc->space_x;
                idx += _font.len;
                continue;
            }
            gt_memcpy(mask_buf, temp, 256);
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

    // gt_mem_free(data);
    // gt_mem_free(mask_buf);
    // gt_mem_free(mask_line);

    return area_font;
}

enum{
    GT_IMG_TYPE_FILE_PNG,
    GT_IMG_TYPE_RGB565_PNG,
    GT_IMG_TYPE_RGBA8888_PNG,

    GT_IMG_TYPE_FILE_BMP,
    GT_IMG_TYPE_RGB565_BMP,
    GT_IMG_TYPE_RGBA8888_BMP,
};


void draw_bg_img(gt_draw_ctx_t * draw_ctx, const gt_attr_rect_st * dsc, const gt_area_st * coords)
{
    if( dsc->bg_img_src == NULL ) return;
    if( dsc->bg_opa < GT_OPA_MIN ) return;

    gt_area_st dst_area, mask_area;
    dst_area = *coords;
    dst_area.h = 1;
    mask_area = dst_area;

    gt_draw_blend_dsc_st blend_dsc = {
        .dst_area = &dst_area,
        .mask_area = &mask_area,
        .opa = GT_OPA_COVER
    };
    uint16_t w,h, y = 0;
    
    /* img src type */
    // gt_img_get_type(dsc->bg_img_src);
    switch(GT_IMG_TYPE_RGB565_PNG){
        case GT_IMG_TYPE_RGB565_PNG:
            if( gt_vf_read_img_wh(dsc->bg_img_src, &w, &h) == GT_RES_FAIL){
                break;
            }
            gt_area_st area_res;
            gt_color_t * color_img_line_p = gt_mem_malloc( w * sizeof(gt_color_t) );
            uint8_t * mask_buf = gt_mem_malloc( w );
            uint16_t idx = 0;
            
            /* line by line to blend */
            while(y < h){
                if( !gt_area_intersect(draw_ctx->buf_area, &dst_area, &area_res) ){
                    y++;
                    dst_area.y++;
                    continue;
                }
                dst_area.y++;
                mask_area.y++;
                /* read img one line */
                gt_vf_read_img_offset(dsc->bg_img_src, color_img_line_p, y * w * sizeof(gt_color_t), w * sizeof(gt_color_t));

                /* change color to mask buf */
                idx = 0;
                while(idx < w){
                    if( color_img_line_p[idx].full ){
                        mask_buf[idx] = 0xFF;
                    }else{
                        mask_buf[idx] = 0x00;
                    }
                    idx++;
                } 
                blend_dsc.mask_buf = NULL;//mask_buf;
                blend_dsc.dst_buf = color_img_line_p;

                /* blend to bg */
                gt_draw_blend(draw_ctx, &blend_dsc);
                y++;
            }
            
            gt_mem_free(color_img_line_p);
            gt_mem_free(mask_buf);
            break;
        default:
            break;
    }
    
    
}

/* end ------------------------------------------------------------------*/
