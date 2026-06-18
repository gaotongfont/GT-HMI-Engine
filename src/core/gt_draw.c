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
#include "../gt_conf.h"

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
#include "gt_mask.h"
#include "../extra/draw/gt_img_transform.h"
#include "gt_gradient.h"

#include "../extra/draw/gt_draw_blend_dir.h"

/* private define -------------------------------------------------------*/

#define RADIUS_MAX  16

#ifndef _GT_DRAW_ATTRIBUTE_SRAM_TEXT
    #define _GT_DRAW_ATTRIBUTE_SRAM_TEXT           GT_ATTRIBUTE_SRAM_TEXT
#endif

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

#ifndef _GT_DRAW_STATIC_LINE_BUFFER
    /**
     * @brief [default: 0] Use the static buffer to draw the line,
     *      the buffer size over width * sizeof(gt_color_t).
     */
    #define _GT_DRAW_STATIC_LINE_BUFFER         0
#endif

#if _GT_DRAW_STATIC_LINE_BUFFER
    #ifndef _GT_DRAW_STATIC_LINE_MAX_WIDTH
        /**
         * @brief
         */
        #define _GT_DRAW_STATIC_LINE_MAX_WIDTH  GT_SCREEN_WIDTH
    #endif
# endif


/* private typedef ------------------------------------------------------*/

/**
 * @brief Single line omit mode
 */
typedef struct {
    char * utf8;
    uint8_t len;
    uint16_t width;
}_omit_info_st;

typedef struct {
    const gt_area_st* coords;
    const gt_point_st* center;
    gt_attr_rect_st* rect_dsc;
    uint16_t radius;
    uint16_t width;
    uint16_t start_angle;
    uint16_t end_angle;
    uint16_t start_quarter;
    uint16_t end_quarter;
}_quarter_circle_st;

typedef struct {
    uint16_t line_count;
    gt_size_t offset_y;
}_text_offset_line_st;

/* static variables -----------------------------------------------------*/

/** Single line omit mode, such as: "..." */
static const GT_ATTRIBUTE_LARGE_RAM_ARRAY char _utf8_dots[] = {
    0x2e, 0x2e, 0x2e
};
/** Single line omit mode, such as: "。。。" */
static const GT_ATTRIBUTE_LARGE_RAM_ARRAY char _utf8_full_stops[] = {
    0xe3, 0x80, 0x82, 0xe3, 0x80, 0x82, 0xe3, 0x80, 0x82
};

#if _GT_DRAW_STATIC_LINE_BUFFER
static GT_ATTRIBUTE_RAM_DATA gt_color_t _static_line_color_buffer[_GT_DRAW_STATIC_LINE_MAX_WIDTH];
static GT_ATTRIBUTE_RAM_DATA gt_opa_t _static_line_opa_buffer[_GT_DRAW_STATIC_LINE_MAX_WIDTH];
#endif

/* macros ---------------------------------------------------------------*/
static inline bool _is_align_reverse(gt_align_et align) {
    return (align & GT_ALIGN_REVERSE) ? true : false;
}

static inline uint8_t _get_align_type(gt_align_et align) {
    return (align & 0x7F);
}

/* static functions -----------------------------------------------------*/
static GT_ATTRIBUTE_RAM_TEXT void _draw_quarter_circle_0(_gt_draw_ctx_st * draw_ctx , _quarter_circle_st* q) {
    gt_area_st area;
    gt_size_t x1, y1, x2, y2;

    if(q->start_quarter == 0 && q->end_quarter == 0 && q->start_angle < q->end_angle) {
        /*Small arc here*/
        y1 = q->center->y + ((gt_sin(q->start_angle) * (q->radius - q->width)) >> GT_MATH_TRIGO_SHIFT);
        x2 = q->center->x + ((gt_sin(q->start_angle + 90) * (q->radius)) >> GT_MATH_TRIGO_SHIFT);

        y2 = q->center->y + ((gt_sin(q->end_angle) * q->radius) >> GT_MATH_TRIGO_SHIFT);
        x1 = q->center->x + ((gt_sin(q->end_angle + 90) * (q->radius - q->width)) >> GT_MATH_TRIGO_SHIFT);

        area.x = x1;
        area.y = y1;
        area.w = x2 - x1 + 1;
        area.h = y2 - y1 + 1;
        gt_area_st inte_area;
        if(gt_area_intersect_get(&inte_area, &area, q->coords)){
            draw_bg(draw_ctx, q->rect_dsc, &inte_area);
        }

    }
    else if(q->start_quarter == 0 || q->end_quarter == 0) {
        /*Start and/or end arcs here*/
        if(q->start_quarter == 0) {
            x1 = q->center->x;
            y2 = q->center->y + q->radius;

            y1 = q->center->y + ((gt_sin(q->start_angle) * (q->radius - q->width)) >> GT_MATH_TRIGO_SHIFT);
            x2 = q->center->x + ((gt_sin(q->start_angle + 90) * (q->radius)) >> GT_MATH_TRIGO_SHIFT);

            area.x = x1;
            area.y = y1;
            area.w = x2 - x1 + 1;
            area.h = y2 - y1 + 1;
            gt_area_st inte_area;
        if(gt_area_intersect_get(&inte_area, &area, q->coords)){
            draw_bg(draw_ctx, q->rect_dsc, &inte_area);
        }

        }
        if(q->end_quarter == 0) {
            x2 = q->center->x + q->radius;
            y1 = q->center->y;

            y2 = q->center->y + ((gt_sin(q->end_angle) * q->radius) >> GT_MATH_TRIGO_SHIFT);
            x1 = q->center->x + ((gt_sin(q->end_angle + 90) * (q->radius - q->width)) >> GT_MATH_TRIGO_SHIFT);

            area.x = x1;
            area.y = y1;
            area.w = x2 - x1 + 1;
            area.h = y2 - y1 + 1;
            gt_area_st inte_area;
        if(gt_area_intersect_get(&inte_area, &area, q->coords)){
            draw_bg(draw_ctx, q->rect_dsc, &inte_area);
        }
        }
    }
    else if((q->start_quarter == q->end_quarter && q->start_quarter != 0 && q->end_angle < q->start_angle) ||
            (q->start_quarter == 2 && q->end_quarter == 1) ||
            (q->start_quarter == 3 && q->end_quarter == 2) ||
            (q->start_quarter == 3 && q->end_quarter == 1)) {
        /*Arc crosses here*/
        x1 = q->center->x;
        y1 = q->center->y;
        x2 = q->center->x + q->radius;
        y2 = q->center->y + q->radius;

        area.x = x1;
        area.y = y1;
        area.w = x2 - x1 + 1;
        area.h = y2 - y1 + 1;
        gt_area_st inte_area;
        if(gt_area_intersect_get(&inte_area, &area, q->coords)){
            draw_bg(draw_ctx, q->rect_dsc, &inte_area);
        }
    }
}

static GT_ATTRIBUTE_RAM_TEXT void _draw_quarter_circle_1(_gt_draw_ctx_st * draw_ctx , _quarter_circle_st* q) {
    gt_area_st area;
    gt_size_t x1, y1, x2, y2;
    if(q->start_quarter == 1 && q->end_quarter == 1 && q->start_angle < q->end_angle) {
        /*Small arc here*/
        y2 = q->center->y + ((gt_sin(q->start_angle) * (q->radius)) >> GT_MATH_TRIGO_SHIFT);
        x2 = q->center->x + ((gt_sin(q->start_angle + 90) * (q->radius - q->width)) >> GT_MATH_TRIGO_SHIFT);

        y1 = q->center->y + ((gt_sin(q->end_angle) * (q->radius - q->width)) >> GT_MATH_TRIGO_SHIFT);
        x1 = q->center->x + ((gt_sin(q->end_angle + 90) * (q->radius)) >> GT_MATH_TRIGO_SHIFT);

        area.x = x1;
        area.y = y1;
        area.w = x2 - x1 + 1;
        area.h = y2 - y1 + 1;
        gt_area_st inte_area;
        if(gt_area_intersect_get(&inte_area, &area, q->coords)){
            draw_bg(draw_ctx, q->rect_dsc, &inte_area);
        }
    }
    else if(q->start_quarter == 1 || q->end_quarter == 1) {
        /*Start and/or end arcs here*/
        if(q->start_quarter == 1) {
            x1 = q->center->x - q->radius;
            y1 = q->center->y;

            y2 = q->center->y + ((gt_sin(q->start_angle) * (q->radius)) >> GT_MATH_TRIGO_SHIFT);
            x2 = q->center->x + ((gt_sin(q->start_angle + 90) * (q->radius - q->width)) >> GT_MATH_TRIGO_SHIFT);

            area.x = x1;
            area.y = y1;
            area.w = x2 - x1 + 1;
            area.h = y2 - y1 + 1;
            gt_area_st inte_area;
        if(gt_area_intersect_get(&inte_area, &area, q->coords)){
            draw_bg(draw_ctx, q->rect_dsc, &inte_area);
        }
        }
        if(q->end_quarter == 1) {
            x2 = q->center->x - 1;
            y2 = q->center->y + q->radius;

            y1 = q->center->y + ((gt_sin(q->end_angle) * (q->radius - q->width)) >> GT_MATH_TRIGO_SHIFT);
            x1 = q->center->x + ((gt_sin(q->end_angle + 90) * (q->radius)) >> GT_MATH_TRIGO_SHIFT);

            area.x = x1;
            area.y = y1;
            area.w = x2 - x1 + 1;
            area.h = y2 - y1 + 1;
            gt_area_st inte_area;
        if(gt_area_intersect_get(&inte_area, &area, q->coords)){
            draw_bg(draw_ctx, q->rect_dsc, &inte_area);
        }
        }
    }
    else if((q->start_quarter == q->end_quarter && q->start_quarter != 1 && q->end_angle < q->start_angle) ||
            (q->start_quarter == 0 && q->end_quarter == 2) ||
            (q->start_quarter == 0 && q->end_quarter == 3) ||
            (q->start_quarter == 3 && q->end_quarter == 2)) {
        /*Arc crosses here*/
        x1 = q->center->x - q->radius;
        y1 = q->center->y;
        x2 = q->center->x - 1;
        y2 = q->center->y + q->radius;

        area.x = x1;
        area.y = y1;
        area.w = x2 - x1 + 1;
        area.h = y2 - y1 + 1;
        gt_area_st inte_area;
        if(gt_area_intersect_get(&inte_area, &area, q->coords)){
            draw_bg(draw_ctx, q->rect_dsc, &inte_area);
        }
    }
}

static GT_ATTRIBUTE_RAM_TEXT void _draw_quarter_circle_2(_gt_draw_ctx_st * draw_ctx , _quarter_circle_st* q) {
    gt_area_st area;
    gt_size_t x1, y1, x2, y2;
    if(q->start_quarter == 2 && q->end_quarter == 2 && q->start_angle < q->end_angle) {
        /*Small arc here*/
        x1 = q->center->x + ((gt_sin(q->start_angle + 90) * (q->radius)) >> GT_MATH_TRIGO_SHIFT);
        y2 = q->center->y + ((gt_sin(q->start_angle) * (q->radius - q->width)) >> GT_MATH_TRIGO_SHIFT);

        y1 = q->center->y + ((gt_sin(q->end_angle) * q->radius) >> GT_MATH_TRIGO_SHIFT);
        x2 = q->center->x + ((gt_sin(q->end_angle + 90) * (q->radius - q->width)) >> GT_MATH_TRIGO_SHIFT);

        area.x = x1;
        area.y = y1;
        area.w = x2 - x1 + 1;
        area.h = y2 - y1 + 1;
        gt_area_st inte_area;
        if(gt_area_intersect_get(&inte_area, &area, q->coords)){
            draw_bg(draw_ctx, q->rect_dsc, &inte_area);
        }
    }
    else if(q->start_quarter == 2 || q->end_quarter == 2) {
        /*Start and/or end arcs here*/
        if(q->start_quarter == 2) {
            x2 = q->center->x - 1;
            y1 = q->center->y - q->radius;

            x1 = q->center->x + ((gt_sin(q->start_angle + 90) * (q->radius)) >> GT_MATH_TRIGO_SHIFT);
            y2 = q->center->y + ((gt_sin(q->start_angle) * (q->radius - q->width)) >> GT_MATH_TRIGO_SHIFT);

            area.x = x1;
            area.y = y1;
            area.w = x2 - x1 + 1;
            area.h = y2 - y1 + 1;
            gt_area_st inte_area;
        if(gt_area_intersect_get(&inte_area, &area, q->coords)){
            draw_bg(draw_ctx, q->rect_dsc, &inte_area);
        }
        }
        if(q->end_quarter == 2) {
            x1 = q->center->x - q->radius;
            y2 = q->center->y - 1;

            x2 = q->center->x + ((gt_sin(q->end_angle + 90) * (q->radius - q->width)) >> GT_MATH_TRIGO_SHIFT);
            y1 = q->center->y + ((gt_sin(q->end_angle) * (q->radius)) >> GT_MATH_TRIGO_SHIFT);

            area.x = x1;
            area.y = y1;
            area.w = x2 - x1 + 1;
            area.h = y2 - y1 + 1;
            gt_area_st inte_area;
        if(gt_area_intersect_get(&inte_area, &area, q->coords)){
            draw_bg(draw_ctx, q->rect_dsc, &inte_area);
        }
        }
    }
    else if((q->start_quarter == q->end_quarter && q->start_quarter != 2 && q->end_angle < q->start_angle) ||
            (q->start_quarter == 0 && q->end_quarter == 3) ||
            (q->start_quarter == 1 && q->end_quarter == 3) ||
            (q->start_quarter == 1 && q->end_quarter == 0)) {
        /*Arc crosses here*/
        x1 = q->center->x - q->radius;
        y1 = q->center->y - q->radius;
        x2 = q->center->x - 1;
        y2 = q->center->y - 1;

        area.x = x1;
        area.y = y1;
        area.w = x2 - x1 + 1;
        area.h = y2 - y1 + 1;
        gt_area_st inte_area;
        if(gt_area_intersect_get(&inte_area, &area, q->coords)){
            draw_bg(draw_ctx, q->rect_dsc, &inte_area);
        }
    }
}

static GT_ATTRIBUTE_RAM_TEXT void _draw_quarter_circle_3(_gt_draw_ctx_st * draw_ctx , _quarter_circle_st* q) {
    gt_area_st area;
    gt_size_t x1, y1, x2, y2;
    if(q->start_quarter == 3 && q->end_quarter == 3 && q->start_angle < q->end_angle) {
        /*Small arc here*/
        x1 = q->center->x + ((gt_sin(q->start_angle + 90) * (q->radius - q->width)) >> GT_MATH_TRIGO_SHIFT);
        y1 = q->center->y + ((gt_sin(q->start_angle) * (q->radius)) >> GT_MATH_TRIGO_SHIFT);

        x2 = q->center->x + ((gt_sin(q->end_angle + 90) * (q->radius)) >> GT_MATH_TRIGO_SHIFT);
        y2 = q->center->y + ((gt_sin(q->end_angle) * (q->radius - q->width)) >> GT_MATH_TRIGO_SHIFT);

        area.x = x1;
        area.y = y1;
        area.w = x2 - x1 + 1;
        area.h = y2 - y1 + 1;
        gt_area_st inte_area;
        if(gt_area_intersect_get(&inte_area, &area, q->coords)){
            draw_bg(draw_ctx, q->rect_dsc, &inte_area);
        }
    }
    else if(q->start_quarter == 3 || q->end_quarter == 3) {
        /*Start and/or end arcs here*/
        if(q->start_quarter == 3) {
            x2 = q->center->x + q->radius;
            y2 = q->center->y - 1;

            x1 = q->center->x + ((gt_sin(q->start_angle + 90) * (q->radius - q->width)) >> GT_MATH_TRIGO_SHIFT);
            y1 = q->center->y + ((gt_sin(q->start_angle) * (q->radius)) >> GT_MATH_TRIGO_SHIFT);

            area.x = x1;
            area.y = y1;
            area.w = x2 - x1 + 1;
            area.h = y2 - y1 + 1;
            gt_area_st inte_area;
        if(gt_area_intersect_get(&inte_area, &area, q->coords)){
            draw_bg(draw_ctx, q->rect_dsc, &inte_area);
        }
        }
        if(q->end_quarter == 3) {
            x1 = q->center->x;
            y1 = q->center->y - q->radius;

            x2 = q->center->x + ((gt_sin(q->end_angle + 90) * (q->radius)) >> GT_MATH_TRIGO_SHIFT);
            y2 = q->center->y + ((gt_sin(q->end_angle) * (q->radius - q->width)) >> GT_MATH_TRIGO_SHIFT);

            area.x = x1;
            area.y = y1;
            area.w = x2 - x1 + 1;
            area.h = y2 - y1 + 1;
            gt_area_st inte_area;
        if(gt_area_intersect_get(&inte_area, &area, q->coords)){
            draw_bg(draw_ctx, q->rect_dsc, &inte_area);
        }
        }
    }
    else if((q->start_quarter == q->end_quarter && q->start_quarter != 3 && q->end_angle < q->start_angle) ||
            (q->start_quarter == 2 && q->end_quarter == 0) ||
            (q->start_quarter == 1 && q->end_quarter == 0) ||
            (q->start_quarter == 2 && q->end_quarter == 1)) {
        /*Arc crosses here*/
        x1 = q->center->x;
        y1 = q->center->y - q->radius;
        x2 = q->center->x + q->radius;
        y2 = q->center->y - 1;

        area.x = x1;
        area.y = y1;
        area.w = x2 - x1 + 1;
        area.h = y2 - y1 + 1;
        gt_area_st inte_area;
        if(gt_area_intersect_get(&inte_area, &area, q->coords)){
            draw_bg(draw_ctx, q->rect_dsc, &inte_area);
        }
    }
}

static uint16_t GT_ATTRIBUTE_RAM_TEXT
_gt_gray_and_dot_byte_get(uint8_t* gray, gt_font_size_t width, gt_font_st * font, uint8_t ret_style, uint32_t unicode) {
    uint16_t dot_byte = 0;
    uint8_t font_type = ret_style % 10;
    uint8_t font_gray = (ret_style/10) > 0 ? (ret_style/10) : 1;

    dot_byte = gt_font_get_one_word_dot_width(unicode, font);
    if (dot_byte){

        if (GT_FONT_TYPE_FLAG_VEC == font_type){
            font_gray = font->info.gray;
        }

        dot_byte = ((dot_byte + 7) >> 3) * font_gray;
        *gray = font_gray;
        return dot_byte;
    }

    if ( (GT_FONT_TYPE_FLAG_DOT_MATRIX_NON_WIDTH) == font_type) {
        dot_byte = ((font->info.size + 7) >> 3) * font_gray;
    }
    else if ((GT_FONT_TYPE_FLAG_VEC == font_type)) {
        font_gray = font->info.gray;

        if (1 != font_gray) {
            dot_byte = (((((font->info.size * font_gray + 15) >> 4) << 4) / font_gray + 7) >> 3) * font_gray;
        } else {
            dot_byte = (((font->info.size + 15) >> 4) << 4) >> 3;
        }
    } else {
        dot_byte = ((width + 7) >> 3) * font_gray;
    }
    *gray = font_gray;

    return dot_byte;
}

/** @brief Get effective display font size, taking scale_size into account */
static GT_ATTRIBUTE_RAM_TEXT inline gt_font_size_t _get_effective_font_size(const gt_font_info_st* font_info) {
    const gt_font_size_t scale_size = font_info->scale_size;
    return (scale_size > 0 && scale_size != font_info->size) ? scale_size : font_info->size;
}

static GT_ATTRIBUTE_RAM_TEXT inline uint8_t _get_style_space_y(const gt_font_info_st* font_info) {

    if(font_info->style.reg.underline || font_info->style.reg.strikethrough) {
        gt_font_size_t eff = _get_effective_font_size(font_info);
        return (eff >> 4) ? (eff >> 4) : 1;
    }
    return 0;
}

static GT_ATTRIBUTE_RAM_TEXT inline uint8_t _get_style_bold_offset_x(const gt_font_info_st* font_info)
{
    if (!font_info->style.reg.bold) return 0;
    gt_font_size_t eff = _get_effective_font_size(font_info);
    if (eff == font_info->size) return 1;
    return (uint8_t)((uint16_t)eff / font_info->size);
}

static GT_ATTRIBUTE_RAM_TEXT inline uint8_t _get_style_space_x( uint8_t org_space_x , const gt_font_info_st* font_info)
{
    return org_space_x + _get_style_bold_offset_x(font_info);
}

static GT_ATTRIBUTE_RAM_TEXT inline void _draw_blend_style_line(
    _gt_draw_ctx_st * draw_ctx, const gt_attr_font_st * dsc, gt_draw_blend_dsc_st * blend_dsc_style,
    gt_area_st area_font_style, const gt_font_info_st* font_info, uint16_t width, uint16_t height,
    gt_size_t y_offset) {
    area_font_style.y += y_offset;
    area_font_style.w = width + _get_style_bold_offset_x(font_info);
    area_font_style.h = height;
    if (font_info->style.reg.italic) {
        area_font_style.w += _get_effective_font_size(font_info) >> 1;
    }
    blend_dsc_style->color_fill = dsc->font_color;
    blend_dsc_style->dst_area = &area_font_style;
    gt_draw_blend(draw_ctx, blend_dsc_style);
}

static GT_ATTRIBUTE_RAM_TEXT inline void _draw_blend_underline(
    _gt_draw_ctx_st * draw_ctx, const gt_attr_font_st * dsc, gt_draw_blend_dsc_st * blend_dsc_style,
    gt_area_st area_font_style, const gt_font_info_st* font_info, uint16_t width, uint16_t height) {
    _draw_blend_style_line(draw_ctx, dsc, blend_dsc_style, area_font_style, font_info, width, height,
                           _get_effective_font_size(font_info) + 1);
}

static GT_ATTRIBUTE_RAM_TEXT inline void _draw_blend_strikethrough(
    _gt_draw_ctx_st * draw_ctx, const gt_attr_font_st * dsc, gt_draw_blend_dsc_st * blend_dsc_style,
    gt_area_st area_font_style, const gt_font_info_st* font_info, uint16_t width, uint16_t height) {
    _draw_blend_style_line(draw_ctx, dsc, blend_dsc_style, area_font_style, font_info, width, height,
                           _get_effective_font_size(font_info) >> 1);
}

/** @brief 推进字符 x 坐标到下一个字符位置 */
static inline void GT_ATTRIBUTE_RAM_TEXT
_advance_x(gt_size_t * x, gt_font_size_t width, uint8_t space_x, bool align_reverse) {
    if (align_reverse) {
        *x -= space_x;
    } else {
        *x += width + space_x;
    }
}

/**
 * @brief 渲染单个 symbol 字形
 * @return true  渲染成功；false  mask 为 NULL，调用方应跳过本字符
 */
static GT_ATTRIBUTE_RAM_TEXT bool _draw_one_symbol(
    _gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st * blend_dsc,
    gt_font_st * tmp_font, gt_area_st * area_font,
    uint32_t uni_or_gbk, uint16_t data_len, gt_size_t font_disp_h)
{
    uint8_t symbol_size = tmp_font->info.size;
    blend_dsc->mask_buf = (uint8_t*)gt_symbol_get_mask_buf(uni_or_gbk, tmp_font->info.size, &symbol_size);
    if (symbol_size != tmp_font->info.size) {
        gt_memset(tmp_font->res, 0, data_len);
    }
    if (NULL == blend_dsc->mask_buf) {
        return false;
    }
    gt_size_t off = (tmp_font->info.size - symbol_size) >> 1;
    area_font->y += off;
    area_font->w = symbol_size;
    area_font->h = symbol_size;
    gt_draw_blend(draw_ctx, blend_dsc);
    area_font->h = font_disp_h;
    area_font->y -= off;
    blend_dsc->mask_buf = NULL;
    return true;
}

/** @brief 渲染下划线和删除线装饰（has_ul/has_st 在循环外预计算，避免每字符访问位域） */
static GT_ATTRIBUTE_RAM_TEXT inline void _draw_char_style(
    _gt_draw_ctx_st * draw_ctx, const gt_attr_font_st * dsc,
    gt_draw_blend_dsc_st * blend_dsc_style, gt_area_st area_font,
    const gt_font_info_st * font_info, uint16_t width,
    uint8_t style_space_y, gt_size_t font_disp_h,
    bool has_ul, bool has_st)
{
    gt_font_size_t eff_fs = _get_effective_font_size(font_info);
    if (has_ul && font_disp_h == eff_fs) {
        _draw_blend_underline(draw_ctx, dsc, blend_dsc_style, area_font, font_info, width, style_space_y);
    }
    if (has_st && (font_disp_h > ((eff_fs >> 1) + (style_space_y << 1)))) {
        _draw_blend_strikethrough(draw_ctx, dsc, blend_dsc_style, area_font, font_info, width, style_space_y);
    }
}

/**
 * @brief 字体渲染资源（res 点阵缓冲 + mask_line）分配结果
 */
typedef struct {
    uint8_t * res;       /**< 点阵数据缓冲，NULL 表示分配失败 */
    uint8_t * mask_line; /**< emoji alpha 行缓冲，NULL 表示分配失败 */
#if GT_USE_EXTRA_FULL_FONT_BUFFER
    bool is_extend_font_valid;
#endif
#if GT_USE_EXTRA_FULL_FONT_MASK_BUFFER
    bool is_extend_font_mask_valid;
#endif
} _draw_font_buf_st;

/**
 * @brief 分配字体渲染所需的 res 缓冲和 mask_line 缓冲
 * @param font_size  字体像素尺寸（用于 mask_line 大小）
 * @param data_len   res 缓冲字节数
 * @return 分配结果，res/mask_line 为 NULL 表示对应分配失败
 */
static GT_ATTRIBUTE_RAM_TEXT _draw_font_buf_st _draw_font_buf_alloc(gt_font_size_t font_size, uint16_t data_len)
{
    _draw_font_buf_st r = {0};
#if GT_USE_EXTRA_FULL_FONT_BUFFER
    gt_buffer_st * gc_font_p = _gt_gc_get_full_font_buffer();
    r.is_extend_font_valid = _gt_gc_full_font_buffer_is_valid() && (data_len < gc_font_p->len);
    r.res = r.is_extend_font_valid ? gc_font_p->ptr : gt_mem_malloc(data_len);
#else
    r.res = gt_mem_malloc(data_len + 128);
#endif
    if (NULL == r.res) {
        GT_LOGE(GT_LOG_TAG_GUI, "data malloc is failed!!! size = %d", data_len);
        return r;
    }
#if GT_USE_EXTRA_FULL_FONT_MASK_BUFFER
    gt_buffer_st * gc_font_mask_p = _gt_gc_get_full_font_mask_buffer();
    r.is_extend_font_mask_valid = _gt_gc_full_font_mask_buffer_is_valid() &&
                                  ((font_size << 1) < gc_font_mask_p->len);
    r.mask_line = r.is_extend_font_mask_valid ? gc_font_mask_p->ptr : gt_mem_malloc(font_size << 1);
#else
    r.mask_line = gt_mem_malloc(font_size << 1);
#endif
    if (NULL == r.mask_line) {
        GT_LOGE(GT_LOG_TAG_GUI, "mask_line malloc is failed!!! size = %d", font_size << 1);
    }
    return r;
}

/**
 * @brief 释放 _draw_font_buf_alloc 分配的缓冲（尊重扩展缓冲标志）
 */
static GT_ATTRIBUTE_RAM_TEXT inline void _draw_font_buf_free(_draw_font_buf_st * b)
{
#if GT_USE_EXTRA_FULL_FONT_MASK_BUFFER
    if (!b->is_extend_font_mask_valid && b->mask_line) {
#else
    if (b->mask_line) {
#endif
        gt_mem_free(b->mask_line);
    }
    b->mask_line = NULL;
#if GT_USE_EXTRA_FULL_FONT_BUFFER
    if (!b->is_extend_font_valid && b->res) {
#else
    if (b->res) {
#endif
        gt_mem_free(b->res);
    }
    b->res = NULL;
}

/**
 * @brief Read one pixel value (gray bits) from packed dot-matrix data.
 *        Supports gray = 1 / 2 / 4.
 *
 * @param src      packed source buffer
 * @param dot_byte bytes per row in src
 * @param x        column index (pixel unit)
 * @param y        row index
 * @param gray     bits per pixel: 1 / 2 / 4
 * @return pixel value in range [0, (1<<gray)-1]
 */
static inline uint8_t _font_dot_read(const uint8_t * src, uint16_t dot_byte,
                                     uint16_t x, uint16_t y, uint8_t gray)
{
    uint32_t bit_idx = ((uint32_t)y * dot_byte * 8) + (uint32_t)x * gray;
    uint8_t byte_val = src[bit_idx >> 3];
    uint8_t bit_off  = (uint8_t)(bit_idx & 7);
    uint8_t mask     = (uint8_t)((1u << gray) - 1u);
    return (byte_val >> (8u - gray - bit_off)) & mask;
}

/**
 * @brief Write one pixel value (gray bits) into packed dot-matrix data.
 */
static inline void _font_dot_write(uint8_t * dst, uint16_t dot_byte,
                                   uint16_t x, uint16_t y, uint8_t gray, uint8_t val)
{
    uint32_t bit_idx = ((uint32_t)y * dot_byte * 8) + (uint32_t)x * gray;
    uint8_t * p      = &dst[bit_idx >> 3];
    uint8_t bit_off  = (uint8_t)(bit_idx & 7);
    uint8_t mask     = (uint8_t)((1u << gray) - 1u);
    *p = (*p & ~(mask << (8u - gray - bit_off))) | ((val & mask) << (8u - gray - bit_off));
}

/**
 * @brief Scale font dot-matrix using nearest-neighbor (gray=1 only).
 *
 * @param src       source dot-matrix buffer
 * @param src_size  source glyph size (pixels, square)
 * @param src_byte  bytes per row in src (dot_byte)
 * @param dst       destination buffer (caller ensures size)
 * @param dst_size  target glyph size (pixels)
 * @param dst_byte  bytes per row in dst
 */
static void GT_ATTRIBUTE_RAM_TEXT
_font_scale_nearest(const uint8_t * src, uint16_t src_w, uint16_t src_h, uint16_t src_byte,
                    uint8_t * dst, uint16_t dst_w, uint16_t dst_h, uint16_t dst_byte)
{
    uint16_t dx, dy;
    for (dy = 0; dy < dst_h; ++dy) {
        uint16_t sy = (uint16_t)((uint32_t)dy * src_h / dst_h);
        for (dx = 0; dx < dst_w; ++dx) {
            uint16_t sx = (uint16_t)((uint32_t)dx * src_w / dst_w);
            uint8_t v = _font_dot_read(src, src_byte, sx, sy, 1);
            _font_dot_write(dst, dst_byte, dx, dy, 1, v);
        }
    }
}

/**
 * @brief Scale font dot-matrix using bilinear interpolation (gray = 2 / 4).
 *        gray=1 is handled by _font_scale_nearest.
 *        Supports non-square glyphs (src_w may differ from src_h).
 */
static void GT_ATTRIBUTE_RAM_TEXT
_font_scale_bilinear(const uint8_t * src, uint16_t src_w, uint16_t src_h, uint16_t src_byte,
                     uint8_t * dst, uint16_t dst_w, uint16_t dst_h, uint16_t dst_byte, uint8_t gray)
{
    const uint8_t max_val = (uint8_t)((1u << gray) - 1u);
    /* Q16 step per axis */
    const uint32_t step_x = (dst_w > 1) ? (((uint32_t)(src_w - 1) << 16) / (dst_w - 1)) : 0;
    const uint32_t step_y = (dst_h > 1) ? (((uint32_t)(src_h - 1) << 16) / (dst_h - 1)) : 0;
    const uint32_t expand = 255u / max_val;
    uint16_t dx, dy;
    uint32_t fy = 0;
    for (dy = 0; dy < dst_h; ++dy, fy += step_y) {
        uint16_t sy0 = (uint16_t)(fy >> 16);
        uint16_t sy1 = (sy0 + 1 < src_h) ? sy0 + 1 : sy0;
        uint32_t wy1 = fy & 0xFFFFu;
        uint32_t wy0 = 0x10000u - wy1;
        uint32_t fx = 0;
        for (dx = 0; dx < dst_w; ++dx, fx += step_x) {
            uint16_t sx0 = (uint16_t)(fx >> 16);
            uint16_t sx1 = (sx0 + 1 < src_w) ? sx0 + 1 : sx0;
            uint32_t wx1 = fx & 0xFFFFu;
            uint32_t wx0 = 0x10000u - wx1;
            uint32_t p00 = _font_dot_read(src, src_byte, sx0, sy0, gray) * expand;
            uint32_t p10 = _font_dot_read(src, src_byte, sx1, sy0, gray) * expand;
            uint32_t p01 = _font_dot_read(src, src_byte, sx0, sy1, gray) * expand;
            uint32_t p11 = _font_dot_read(src, src_byte, sx1, sy1, gray) * expand;
            uint32_t v255 = (((p00 * wx0 + p10 * wx1) >> 16) * wy0 +
                             ((p01 * wx0 + p11 * wx1) >> 16) * wy1) >> 16;
            uint8_t v = (uint8_t)(v255 * max_val / 255u);
            _font_dot_write(dst, dst_byte, dx, dy, gray, v);
        }
    }
}

/**
 * @brief Apply software scaling to font glyph buffer.
 *        For gray=1 (dot-matrix), converts to 4-bit grayscale via bilinear interpolation
 *        to achieve smooth edges (no binarization).
 *
 * @param font      font structure (font->res will be updated to scaled buffer)
 * @param gray      input gray depth (1/2/4)
 * @param out_gray  [out] output gray depth (may change from 1→4 for dot-matrix)
 * @param src_byte  bytes per row in original buffer
 * @param src_size  source glyph size (height in pixels)
 * @param dst_size  target glyph size (height in pixels)
 * @return scaled dst_byte (0 on failure)
 */
static uint16_t GT_ATTRIBUTE_RAM_TEXT
_font_scale_apply(gt_font_st * font, uint8_t gray, uint8_t * out_gray,
                  uint16_t src_byte, uint16_t src_size, uint16_t dst_size)
{
    /* derive actual pixel dimensions */
    uint16_t src_w    = (uint16_t)((src_byte / gray) << 3);  /* pixel width (byte-aligned) */
    uint16_t src_h    = src_size;
    uint16_t dst_w    = (uint16_t)((uint32_t)src_w * dst_size / src_size);
    uint16_t dst_h    = dst_size;

    if (1 == gray) {
        /* 点阵字体平滑缩放：转4bit灰度 → 双线性插值 → 直接输出4bit（不二值化） */
        uint16_t gray4_byte = (uint16_t)(((src_w << 2) + 7u) >> 3);  /* 4bit: 2 pixels/byte */
        uint16_t gray4_size = gray4_byte * src_h;
        uint8_t * gray4_src = (uint8_t *)gt_mem_malloc(gray4_size);

        if (gray4_src) {
            gt_memset_0(gray4_src, gray4_size);

            /* Step 1: 1bit → 4bit (0→0, 1→15) */
            for (uint16_t y = 0; y < src_h; y++) {
                for (uint16_t x = 0; x < src_w; x++) {
                    uint8_t bit = _font_dot_read(font->res, src_byte, x, y, 1);
                    uint8_t val = bit ? 0x0F : 0x00;
                    _font_dot_write(gray4_src, gray4_byte, x, y, 4, val);
                }
            }

            /* Step 2: 双线性插值到目标尺寸（输出4bit灰度） */
            uint16_t dst_byte = (uint16_t)(((dst_w << 2) + 7u) >> 3);  /* 4bit byte width */
            uint16_t dst_need = dst_byte * dst_h;
            uint8_t * dst_buf = (uint8_t *)gt_mem_malloc(dst_need);

            if (dst_buf) {
                gt_memset_0(dst_buf, dst_need);
                _font_scale_bilinear(gray4_src, src_w, src_h, gray4_byte,
                                     dst_buf, dst_w, dst_h, dst_byte, 4);

                gt_mem_free(gray4_src);

                /* 更新输出灰度为4bit */
                *out_gray = 4;

                /* caller saves old font->res and frees it; we only update the pointer */
                font->res = dst_buf;
                return dst_byte;
            }

            gt_mem_free(gray4_src);
        }

        /* 内存不足，回退最近邻（保持1bit） */
        uint16_t dst_byte = (uint16_t)(((dst_w * gray + 7u) >> 3));
        uint16_t dst_need = dst_byte * dst_h;
        uint8_t * dst_buf = (uint8_t *)gt_mem_malloc(dst_need);
        if (NULL == dst_buf) { return 0; }
        gt_memset_0(dst_buf, dst_need);

        _font_scale_nearest(font->res, src_w, src_h, src_byte,
                            dst_buf, dst_w, dst_h, dst_byte);

        font->res = dst_buf;
        *out_gray = 1;  /* 回退路径保持1bit */
        return dst_byte;

    } else {
        /* 灰度字体直接双线性插值 */
        uint16_t dst_byte = (uint16_t)(((dst_w * gray + 7u) >> 3));
        uint16_t dst_need = dst_byte * dst_h;
        uint8_t * dst_buf = (uint8_t *)gt_mem_malloc(dst_need);
        if (NULL == dst_buf) { return 0; }
        gt_memset_0(dst_buf, dst_need);

        _font_scale_bilinear(font->res, src_w, src_h, src_byte,
                             dst_buf, dst_w, dst_h, dst_byte, gray);

        font->res = dst_buf;
        *out_gray = gray;  /* 保持原灰度 */
        return dst_byte;
    }
}

/**
 * @brief 渲染省略号字符序列（draw_text_single_line 末尾使用）
 */
static GT_ATTRIBUTE_RAM_TEXT void _draw_omit_chars(
    _gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st * blend_dsc,
    gt_font_st * tmp_font, gt_area_st * area_font,
    const _omit_info_st * omit_info,
    uint8_t space_x, bool align_reverse)
{
    uint32_t uni_or_gbk = 0;
    tmp_font->utf8 = omit_info->utf8;
    tmp_font->len = gt_font_one_char_code_len_get((uint8_t*)omit_info->utf8, &uni_or_gbk, tmp_font->info.encoding);
    _gt_font_dot_ret_st dot_ret = gt_font_get_dot(tmp_font, uni_or_gbk);
    gt_font_size_t width = gt_font_get_one_word_width_by_option(uni_or_gbk, tmp_font, dot_ret.option);
    uint8_t font_gray = 1;
    uint16_t byte_width = _gt_gray_and_dot_byte_get(&font_gray, width, tmp_font, dot_ret.type, uni_or_gbk);
    area_font->w = (byte_width / font_gray) << 3;
    if (tmp_font->info.style.reg.bold) {
        area_font->w += _get_style_bold_offset_x((const gt_font_info_st*)&tmp_font->info);
    }
    const gt_point_st off_point = gt_font_offset_get(dot_ret.option, uni_or_gbk);
    const bool has_off = (off_point.x | off_point.y) != 0;
    uint16_t idx = 0;
    while (idx < omit_info->len) {
        if (has_off) {
            area_font->x += off_point.x;
            area_font->y += off_point.y;
        }
        gt_draw_blend_text(draw_ctx, blend_dsc, tmp_font->info.size, byte_width,
                           font_gray, (const uint8_t*)tmp_font->res, tmp_font->info.style.reg);
        if (has_off) {
            area_font->x -= off_point.x;
            area_font->y -= off_point.y;
        }
        _advance_x(&area_font->x, width, space_x, align_reverse);
        idx += tmp_font->len;
    }
}

/**
 * @brief 渲染单个普通字形（非 symbol），处理 emoji 和点阵两种情况
 *
 * @param draw_ctx            绘制上下文
 * @param blend_dsc           混合描述符（view_area 用于 emoji limit_area）
 * @param mask_line           emoji alpha 缓冲
 * @param tmp_font            当前字体（含 res 点阵数据）
 * @param dot_ret             字形查找结果
 * @param area_font           当前字符绘制区域（会被临时修改后还原）
 * @param offset_y            ASCII 字体组偏移
 * @param width               字符像素宽度
 * @param uni_or_gbk          当前字符编码
 * @param dsc                 字体属性描述符
 * @param apply_height_center 是否对矮字形做垂直居中（多行模式为 true，单行为 false）
 */
static GT_ATTRIBUTE_RAM_TEXT void _draw_one_char_glyph(
    _gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st * blend_dsc,
    uint8_t * mask_line, gt_font_st * tmp_font,
    _gt_font_dot_ret_st dot_ret, gt_area_st * area_font,
    int8_t offset_y, gt_font_size_t * p_width,
    uint32_t uni_or_gbk, const gt_attr_font_st * dsc,
    bool apply_height_center)
{
    if (GT_FONT_TYPE_FLAG_EMOJI == dot_ret.type) {
        gt_area_st emoji_area = *area_font;
        emoji_area.h = 1;

        _gt_img_dsc_st emoji_raw = {0};
        emoji_raw.img   = (uint8_t*)tmp_font->res;
        emoji_raw.alpha = mask_line;
        emoji_raw.header.w = *p_width;
        emoji_raw.header.h = 1;
        emoji_raw.header.type = GT_IMG_DECODER_TYPE_RAM;
        emoji_raw.header.color_format = GT_IMG_CF_RGB888;

        gt_attr_rect_st dsc_emoji = {0};
        dsc_emoji.bg_opa    = dsc->opa;
        dsc_emoji.raw_img   = &emoji_raw;
        dsc_emoji.limit_area = (gt_area_st*)blend_dsc->view_area;

        gt_font_size_t emoji_h = gt_font_get_one_word_height(uni_or_gbk, tmp_font);
        emoji_area.y += ((tmp_font->info.size - emoji_h) >> 1);

        for (gt_font_size_t i = 0; i < emoji_h; ++i) {
            int emj_ret = gt_font_get_emoji_dot(uni_or_gbk, dot_ret.option, emoji_raw.img, emoji_raw.alpha, i);
            if (emj_ret <= 0) {
                GT_LOGD(GT_LOG_TAG_GUI, "get emoji dot failed!!! ret: %d", emj_ret);
                break;
            }
            draw_bg_img(draw_ctx, &dsc_emoji, &emoji_area);
            ++emoji_area.y;
        }
        gt_memset(mask_line, 0xFF, tmp_font->info.size << 1);
    } else {
        uint8_t font_gray = 1;
        const gt_font_size_t info_size = area_font->h; //tmp_font->info.size;
        const gt_font_style_st style_reg = tmp_font->info.style.reg;
        gt_font_size_t font_height = gt_font_get_one_word_height_by_option(uni_or_gbk, tmp_font, dot_ret.option);
        uint16_t byte_width = _gt_gray_and_dot_byte_get(&font_gray, *p_width, tmp_font, dot_ret.type, uni_or_gbk);
        area_font->w = (byte_width / font_gray) << 3;

        /* --- software scaling --- */
        const gt_font_size_t scale_size = tmp_font->info.scale_size;
        gt_font_size_t render_size = info_size;
        uint8_t * scaled_res = NULL;          /* non-NULL: scaled buffer to free after blend */
        uint8_t * font_buf_res_orig = tmp_font->res;  /* original res to restore after blend */
        if (scale_size > 0 && scale_size != info_size) {
            uint16_t scaled_byte = _font_scale_apply(tmp_font, font_gray, &font_gray, byte_width, info_size, scale_size);
            if (scaled_byte > 0) {
                scaled_res   = tmp_font->res;  /* new buffer allocated by _font_scale_apply */
                byte_width   = scaled_byte;
                render_size  = scale_size;
                /* 使用更新后的 font_gray 计算宽度（点阵可能变为4bit） */
                area_font->w = (uint16_t)((uint32_t)byte_width * 8u / font_gray);
                area_font->h = scale_size;
                /* scale advance width proportionally so cursor advances correctly */
                *p_width = (gt_font_size_t)((uint32_t)(*p_width) * scale_size / info_size);
                font_height = (gt_font_size_t)((uint32_t)font_height * scale_size / info_size);
            } else {
                tmp_font->res = font_buf_res_orig;  /* restore on failure */
            }
        }
        /* --- end scaling --- */

        const bool is_ascii = (uni_or_gbk < 0x80);
        const bool do_scale_offsets = (scale_size > 0 && scale_size != info_size);
        int8_t eff_offset_y = offset_y;
        if (do_scale_offsets) {
            eff_offset_y = (int8_t)((int32_t)offset_y * scale_size / info_size);
        }
        if (is_ascii) { area_font->y += eff_offset_y; }
        if (style_reg.bold) {
            area_font->w += _get_style_bold_offset_x((const gt_font_info_st*)&tmp_font->info);
        }

        int8_t height_off = apply_height_center ? (int8_t)((render_size - font_height) >> 1) : 0;
        area_font->y += height_off;

        gt_point_st off_point = gt_font_offset_get(dot_ret.option, uni_or_gbk);
        if (do_scale_offsets) {
            off_point.x = (int16_t)((int32_t)off_point.x * scale_size / info_size);
            off_point.y = (int16_t)((int32_t)off_point.y * scale_size / info_size);
        }
        if (off_point.x | off_point.y) {
            area_font->x += off_point.x;
            area_font->y += off_point.y;
            gt_draw_blend_text(draw_ctx, blend_dsc, render_size,
                                byte_width, font_gray, (const uint8_t*)tmp_font->res, style_reg);
            area_font->x -= off_point.x;
            area_font->y -= off_point.y;
        } else {
            gt_draw_blend_text(draw_ctx, blend_dsc, render_size,
                                byte_width, font_gray, (const uint8_t*)tmp_font->res, style_reg);
        }

        area_font->y -= height_off;
        if (is_ascii) { area_font->y -= eff_offset_y; }
        /* free scaled buffer and restore original res pointer */
        if (scaled_res) {
            gt_mem_free(scaled_res);
            tmp_font->res = font_buf_res_orig;
        }
        /* restore area_font->h for next character */
        area_font->h = info_size;
    }
}

#if _GT_FONT_ENABLE_CONVERTOR
static GT_ATTRIBUTE_RAM_TEXT _gt_draw_font_res_st _draw_text_convertor_split(
    gt_font_st * fonts, uint8_t lan,
    _gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st * blend_dsc,
    const gt_attr_font_st * dsc, gt_size_t font_disp_h,
    gt_draw_blend_dsc_st * blend_dsc_style, const gt_area_st * coords, uint8_t flag_ol) {
    uint32_t idx = 0, uni_or_gbk = 0, tmp = 0;
    _gt_draw_font_res_st ret = { .res = GT_RES_OK };
    uint16_t * text = NULL;
    uint8_t * font_buf = NULL;
    const bool align_reverse = _is_align_reverse(dsc->align);
    text = (uint16_t * )gt_mem_malloc(fonts->len * sizeof(uint16_t));
    if (NULL == text) {
        GT_LOGE(GT_LOG_TAG_GUI, "buf malloc failed, size: %lu", fonts->len * sizeof(uint16_t));
        goto _null_handle;
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
    gt_font_size_t width; uint16_t byte_width; uint8_t font_gray;

#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
    uint16_t font_option = tmp_font.info.style_fl;
    uint8_t ret_style = gt_font_type_get(tmp_font.info.style_fl);
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(&tmp_font.info, ret_style, lan, text_len);
#else
    uint16_t font_option = gt_font_family_get_option(tmp_font.info.family, lan, tmp_font.info.cjk);
    uint8_t ret_style = gt_font_type_get(font_option);
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(&tmp_font.info, lan, text_len);
#endif

    font_buf = (uint8_t * )gt_mem_malloc(font_size_res.font_buff_len + 128);
    if (NULL == font_buf) {
        GT_LOGE(GT_LOG_TAG_GUI, "buf malloc failed, size: %d", font_size_res.font_buff_len);
        goto _buf_handle;
    }
    gt_memset_0(font_buf, font_size_res.font_buff_len);

    uint8_t style_space_y = _get_style_space_y((const gt_font_info_st*)&tmp_font.info);
    const bool cvt_has_ul = tmp_font.info.style.reg.underline;
    const bool cvt_has_st = tmp_font.info.style.reg.strikethrough;
    font_convertor_st convertor = {
        .fontcode = text,
        .code_len = text_len,
        .fontsize = tmp_font.info.size,
        .fontgray = tmp_font.info.gray,
        .font_option = font_option,
        .thick = tmp_font.info.thick_en,
        .data_p = font_buf,
        .data_len = font_size_res.font_buff_len,
        .is_rev = align_reverse ? 0 : 1,
        .is_vec = ret_style == GT_FONT_TYPE_FLAG_VEC ? 1 : 0,
    };
    if(ret_style != GT_FONT_TYPE_FLAG_VEC){
        convertor.fontgray = (ret_style / 10) ? (ret_style / 10) : 1;
    }

    uint8_t space_x = _get_style_space_x(dsc->space_x , (const gt_font_info_st*)&tmp_font.info);

    text_len = gt_font_code_transform(&convertor);

    idx = 0;
    tmp = 0;
    const int32_t cvt_coords_y_bottom = (int32_t)coords->y + coords->h;
    const int32_t cvt_view_y_bottom   = (int32_t)blend_dsc->view_area->y + blend_dsc->view_area->h;
    const gt_size_t cvt_view_x_right    = blend_dsc->view_area->x + blend_dsc->view_area->w;
    const gt_font_size_t cvt_font_size = tmp_font.info.size;
    const gt_font_style_st cvt_style_reg = tmp_font.info.style.reg;
    tmp_font.res = &font_buf[font_size_res.font_buff_len - font_size_res.font_per_size];
    while (idx < text_len) {
        if (FONT_LAN_HINDI == lan) {
            width = gt_font_get_word_width_figure_up(&convertor.data_p[idx * font_size_res.font_per_size],
                                                        16, 24, 1);
            tmp = 1;
        }
        else{
            gt_memset_0(&convertor.data_p[0], font_size_res.font_per_size);
            if (0x20 == convertor.fontcode[idx]) {
                width = convertor.fontsize >> 1;
            }
            else{
                tmp = gt_font_convertor_data_get(&convertor, idx);
                width = gt_font_get_word_width_figure_up(&convertor.data_p[0], font_size_res.dot_width,
                                                            convertor.fontsize, convertor.fontgray);
            }
        }
        if (align_reverse) {
            blend_dsc->dst_area->x -= width;
        }

        if (flag_ol) {
            if(align_reverse){
                if((blend_dsc->dst_area->x) < (coords->x)){
                    blend_dsc->dst_area->x = coords->x + coords->w - width;
                    blend_dsc->dst_area->y += blend_dsc->dst_area->h;
                    if((blend_dsc->dst_area->y + style_space_y + blend_dsc->dst_area->h) > cvt_coords_y_bottom){
                        font_disp_h = cvt_coords_y_bottom - blend_dsc->dst_area->y;
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
                    if ((blend_dsc->dst_area->y + style_space_y + blend_dsc->dst_area->h) > cvt_coords_y_bottom) {
                        font_disp_h = cvt_coords_y_bottom - blend_dsc->dst_area->y;
                        if (font_disp_h < 2) {
                            goto _ret_handle;
                        }
                    }
                }
            }
        }
        blend_dsc->dst_area->h = font_disp_h;
        if (blend_dsc->dst_area->y + style_space_y + blend_dsc->dst_area->h < blend_dsc->view_area->y ||
            blend_dsc->dst_area->y > cvt_view_y_bottom) {
            goto _next_word_cvt_lb;
        }
        else if (blend_dsc->dst_area->x + width < blend_dsc->view_area->x ||
            blend_dsc->dst_area->x > cvt_view_x_right) {
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
        const uint8_t * cvt_data_p;
        if (FONT_LAN_HINDI == lan) {
            byte_width = 2;
            cvt_data_p = &convertor.data_p[idx * font_size_res.font_per_size];
        } else {
            byte_width = (width + 7) >> 3;
            byte_width = _gt_gray_and_dot_byte_get(&font_gray, byte_width, &tmp_font, ret_style, uni_or_gbk);
            cvt_data_p = &convertor.data_p[0];
        }
        blend_dsc->dst_area->w = byte_width << 3;
        if (cvt_style_reg.bold) {
            blend_dsc->dst_area->w += _get_style_bold_offset_x((const gt_font_info_st*)&tmp_font.info);
        }
        gt_point_st off_point = gt_font_offset_get(font_option, uni_or_gbk);
        if (off_point.x | off_point.y) {
            blend_dsc->dst_area->x += off_point.x;
            blend_dsc->dst_area->y += off_point.y;
            gt_draw_blend_text(draw_ctx, blend_dsc, cvt_font_size, byte_width, font_gray,
                                cvt_data_p, cvt_style_reg);
            blend_dsc->dst_area->x -= off_point.x;
            blend_dsc->dst_area->y -= off_point.y;
        } else {
            gt_draw_blend_text(draw_ctx, blend_dsc, cvt_font_size, byte_width, font_gray,
                                cvt_data_p, cvt_style_reg);
        }

        /* use style */
        _draw_char_style(draw_ctx, dsc, blend_dsc_style, *blend_dsc->dst_area,
                         (const gt_font_info_st*)&tmp_font.info, width, style_space_y, font_disp_h,
                         cvt_has_ul, cvt_has_st);
    _next_word_cvt_lb:
        _advance_x(&blend_dsc->dst_area->x, width, space_x, align_reverse);
        idx += tmp;
    }

_ret_handle:
    if(NULL != font_buf){
        gt_mem_free(font_buf);
        font_buf = NULL;
    }
_buf_handle:
    if(NULL !=text){
        gt_mem_free(text);
        text = NULL;
    }
_null_handle:
    ret.abs_pos.x = blend_dsc->dst_area->x;
    ret.abs_pos.y = blend_dsc->dst_area->y;
    return ret;
}
#endif  /** _GT_FONT_ENABLE_CONVERTOR */

#if GT_USE_SJPG
static void GT_ATTRIBUTE_RAM_TEXT
_filling_by_sjpg_file(_gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st * blend_dsc, _gt_img_dsc_st * dsc_img) {
    gt_area_st * dst_ap = blend_dsc->dst_area;
    uint16_t w, h, y = 0;
    w = dsc_img->header.w;
    h = dsc_img->header.h;

    blend_dsc->dst_area->h = 1;

    /* line by line to blend */
    uint32_t len = w * sizeof(gt_color_t);
#if _GT_DRAW_STATIC_LINE_BUFFER
    blend_dsc->dst_buf = _static_line_color_buffer;
#else
    blend_dsc->dst_buf = gt_mem_malloc(len);
    if (NULL == blend_dsc->dst_buf) {
        GT_LOGW(GT_LOG_TAG_GUI, "Per-line buffer malloc failed, size: %d, %d", w, h);
        return;
    }
#endif

    bool mask_check = gt_mask_check(dst_ap);
    gt_opa_t opa = blend_dsc->opa >= GT_OPA_MAX ? GT_OPA_COVER : blend_dsc->opa;
#if _GT_DRAW_STATIC_LINE_BUFFER
        blend_dsc->mask_buf = _static_line_opa_buffer;
#else
    if(mask_check){
        blend_dsc->mask_buf = gt_mem_malloc(dst_ap->w);
        if(NULL == blend_dsc->mask_buf){
            GT_LOGW(GT_LOG_TAG_GUI, "Per-line mask buffer malloc failed, size: %d, %d", w, h);
            gt_mem_free(blend_dsc->dst_buf);
            blend_dsc->dst_buf = NULL;
            return;
        }
    }
#endif

    while(y < h){
        if( gt_area_is_intersect_screen(&draw_ctx->buf_area, blend_dsc->dst_area) ){
            if(mask_check){
                gt_memset(blend_dsc->mask_buf, opa, dst_ap->w);
                gt_mask_get(blend_dsc->mask_buf, dst_ap->x, dst_ap->y, dst_ap->w);
            }
            /* blend to bg */
            gt_img_decoder_read_line(dsc_img, 0, y, len, (uint8_t *)blend_dsc->dst_buf);
            gt_draw_blend(draw_ctx, blend_dsc);
        }
        ++blend_dsc->dst_area->y;
        ++y;
    }
#if _GT_DRAW_STATIC_LINE_BUFFER
    blend_dsc->mask_buf = NULL;
    blend_dsc->dst_buf = NULL;
#else
    if(mask_check){
        gt_mem_free(blend_dsc->mask_buf);
        blend_dsc->mask_buf = NULL;
    }
    gt_mem_free(blend_dsc->dst_buf);
    blend_dsc->dst_buf = NULL;
#endif
}
#endif

/**
 * @brief Color format conversion: convert source pixel data to gt_color_t format.
 *
 * When the image's source color format (from C array / flash resource) differs
 * from the engine's GT_COLOR_DEPTH, we convert to a temp buffer before blending.
 * The fast path (src_cf == GT_COLOR_DEPTH) has zero overhead — no conversion.
 */

/* ---- RGB565 pixel decode helpers ---- */
/* GT_IMG_CF_RGB565:      standard layout [R4:R0 G5:G3][G2:G0 B4:B0] little-endian uint16
 *                        i.e. uint16 bits: R[15:11] G[10:5] B[4:0]
 * GT_IMG_CF_RGB565_SWAP: byte-swapped for SPI LCD, uint16 stored as [G2:G0 R4:R0][B4:B0 G5:G3]
 *                        need to swap bytes before extracting fields */
static inline void _rgb565_extract(uint16_t p, uint8_t *r5, uint8_t *g6, uint8_t *b5) {
    *r5 = (p >> 11) & 0x1F;
    *g6 = (p >> 5)  & 0x3F;
    *b5 =  p        & 0x1F;
}
static inline void _rgb565_swap_extract(uint16_t p, uint8_t *r5, uint8_t *g6, uint8_t *b5) {
    /* swap bytes then extract as standard RGB565 */
    uint16_t s = (uint16_t)((p << 8) | (p >> 8));
    *r5 = (s >> 11) & 0x1F;
    *g6 = (s >> 5)  & 0x3F;
    *b5 =  s        & 0x1F;
}

/* ---- source RGB565 (standard) → target ---- */
#if GT_COLOR_DEPTH == 32
static inline void _cf_2_to_32(const uint8_t *src, gt_color_t *dst, uint32_t count) {
    const uint16_t *s = (const uint16_t *)src;
    for (uint32_t i = 0; i < count; i++) {
        uint8_t r5, g6, b5;
        _rgb565_extract(s[i], &r5, &g6, &b5);
        dst[i].ch.red   = (r5 << 3) | (r5 >> 2);
        dst[i].ch.green = (g6 << 2) | (g6 >> 4);
        dst[i].ch.blue  = (b5 << 3) | (b5 >> 2);
        dst[i].ch.alpha = 0xFF;
    }
}
#endif
static inline void _cf_2_to_8(const uint8_t *src, gt_color_t *dst, uint32_t count) {
    const uint16_t *s = (const uint16_t *)src;
    for (uint32_t i = 0; i < count; i++) {
        uint8_t r5, g6, b5;
        _rgb565_extract(s[i], &r5, &g6, &b5);
        dst[i].ch.red   = r5 >> 2;
        dst[i].ch.green = g6 >> 3;
        dst[i].ch.blue  = b5 >> 3;
    }
}
static inline void _cf_2_to_1(const uint8_t *src, gt_color_t *dst, uint32_t count) {
    const uint16_t *s = (const uint16_t *)src;
    for (uint32_t i = 0; i < count; i++) {
        uint8_t r5, g6, b5;
        _rgb565_extract(s[i], &r5, &g6, &b5);
        /* expand to 8-bit before applying luma coefficients */
        uint8_t r8 = (r5 << 3) | (r5 >> 2);
        uint8_t g8 = (g6 << 2) | (g6 >> 4);
        uint8_t b8 = (b5 << 3) | (b5 >> 2);
        uint8_t lum = (r8 * 77 + g8 * 150 + b8 * 29) >> 8;
        dst[i].full = (lum >= 128) ? 0x07 : 0x00;
    }
}

/* ---- source RGB565_SWAP (SPI LCD byte-swapped) → target ---- */
#if GT_COLOR_DEPTH == 32
static inline void _cf_2s_to_32(const uint8_t *src, gt_color_t *dst, uint32_t count) {
    const uint16_t *s = (const uint16_t *)src;
    for (uint32_t i = 0; i < count; i++) {
        uint8_t r5, g6, b5;
        _rgb565_swap_extract(s[i], &r5, &g6, &b5);
        dst[i].ch.red   = (r5 << 3) | (r5 >> 2);
        dst[i].ch.green = (g6 << 2) | (g6 >> 4);
        dst[i].ch.blue  = (b5 << 3) | (b5 >> 2);
        dst[i].ch.alpha = 0xFF;
    }
}
#endif
static inline void _cf_2s_to_16(const uint8_t *src, gt_color_t *dst, uint32_t count) {
    const uint16_t *s = (const uint16_t *)src;
#if GT_COLOR_16_SWAP == 1
    /* gt_color16_t is also stored byte-swapped, so RGB565_SWAP matches natively */
    gt_memcpy(dst, src, count * sizeof(uint16_t));
    (void)s;
#else
    /* swap each pixel's bytes to restore standard RGB565 bit layout */
    for (uint32_t i = 0; i < count; i++) {
        dst[i].full = (uint16_t)((s[i] << 8) | (s[i] >> 8));
    }
#endif
}
static inline void _cf_2s_to_8(const uint8_t *src, gt_color_t *dst, uint32_t count) {
    const uint16_t *s = (const uint16_t *)src;
    for (uint32_t i = 0; i < count; i++) {
        uint8_t r5, g6, b5;
        _rgb565_swap_extract(s[i], &r5, &g6, &b5);
        dst[i].ch.red   = r5 >> 2;
        dst[i].ch.green = g6 >> 3;
        dst[i].ch.blue  = b5 >> 3;
    }
}
static inline void _cf_2s_to_1(const uint8_t *src, gt_color_t *dst, uint32_t count) {
    const uint16_t *s = (const uint16_t *)src;
    for (uint32_t i = 0; i < count; i++) {
        uint8_t r5, g6, b5;
        _rgb565_swap_extract(s[i], &r5, &g6, &b5);
        uint8_t r8 = (r5 << 3) | (r5 >> 2);
        uint8_t g8 = (g6 << 2) | (g6 >> 4);
        uint8_t b8 = (b5 << 3) | (b5 >> 2);
        uint8_t lum = (r8 * 77 + g8 * 150 + b8 * 29) >> 8;
        dst[i].full = (lum >= 128) ? 0x07 : 0x00;
    }
}

/* ---- source RGB888 (BGR byte order) → target ---- */
#if GT_COLOR_DEPTH == 32
static inline void _cf_3_to_32(const uint8_t *src, gt_color_t *dst, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        dst[i].ch.blue  = src[i * 3 + 0];
        dst[i].ch.green = src[i * 3 + 1];
        dst[i].ch.red   = src[i * 3 + 2];
        dst[i].ch.alpha = 0xFF;
    }
}
#endif
static inline void _cf_3_to_16(const uint8_t *src, gt_color_t *dst, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        const uint8_t *p = &src[i * 3];
        uint8_t b = p[0], g = p[1], r = p[2];
#if GT_COLOR_16_SWAP == 0
        dst[i].ch.red   = r >> 3;
        dst[i].ch.green = g >> 2;
        dst[i].ch.blue  = b >> 3;
#else
        dst[i].ch.green_h = (g >> 5) & 0x7;
        dst[i].ch.red     = r >> 3;
        dst[i].ch.blue    = b >> 3;
        dst[i].ch.green_l = g & 0x7;
#endif
    }
}
static inline void _cf_3_to_8(const uint8_t *src, gt_color_t *dst, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        dst[i].ch.red   = src[i * 3 + 2] >> 5;
        dst[i].ch.green = src[i * 3 + 1] >> 5;
        dst[i].ch.blue  = src[i * 3 + 0] >> 6;
    }
}
static inline void _cf_3_to_1(const uint8_t *src, gt_color_t *dst, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        const uint8_t *p = &src[i * 3];
        uint8_t lum = (p[2] * 77 + p[1] * 150 + p[0] * 29) >> 8;
        dst[i].full = (lum >= 128) ? 0x07 : 0x00;
    }
}

/* ---- source BGRA8888 → target (byte layout: B G R A, matches gt_color32_t) ---- */
static inline void _cf_4_to_16(const uint8_t *src, gt_color_t *dst, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        const uint8_t *p = &src[i * 4];
        uint8_t r = p[2], g = p[1], b = p[0];
#if GT_COLOR_16_SWAP == 0
        dst[i].ch.red   = r >> 3;
        dst[i].ch.green = g >> 2;
        dst[i].ch.blue  = b >> 3;
#else
        dst[i].ch.green_h = (g >> 5) & 0x7;
        dst[i].ch.red     = r >> 3;
        dst[i].ch.blue    = b >> 3;
        dst[i].ch.green_l = g & 0x7;
#endif
    }
}
static inline void _cf_4_to_8(const uint8_t *src, gt_color_t *dst, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        const uint8_t *p = &src[i * 4];
        dst[i].ch.red   = p[2] >> 5;
        dst[i].ch.green = p[1] >> 5;
        dst[i].ch.blue  = p[0] >> 6;
    }
}
static inline void _cf_4_to_1(const uint8_t *src, gt_color_t *dst, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        const uint8_t *p = &src[i * 4];
        uint8_t lum = (p[2] * 77 + p[1] * 150 + p[0] * 29) >> 8;
        dst[i].full = (lum >= 128) ? 0x07 : 0x00;
    }
}

/* ---- source RGBA8888 → target (byte layout: R G B A) ---- */
#if GT_COLOR_DEPTH == 32
static inline void _cf_rgba_to_32(const uint8_t *src, gt_color_t *dst, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        const uint8_t *p = &src[i * 4];
        dst[i].ch.red   = p[0];
        dst[i].ch.green = p[1];
        dst[i].ch.blue  = p[2];
        dst[i].ch.alpha = p[3];
    }
}
#endif
static inline void _cf_rgba_to_16(const uint8_t *src, gt_color_t *dst, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        const uint8_t *p = &src[i * 4];
        uint8_t r = p[0], g = p[1], b = p[2];
#if GT_COLOR_16_SWAP == 0
        dst[i].ch.red   = r >> 3;
        dst[i].ch.green = g >> 2;
        dst[i].ch.blue  = b >> 3;
#else
        dst[i].ch.green_h = (g >> 5) & 0x7;
        dst[i].ch.red     = r >> 3;
        dst[i].ch.blue    = b >> 3;
        dst[i].ch.green_l = g & 0x7;
#endif
    }
}
static inline void _cf_rgba_to_8(const uint8_t *src, gt_color_t *dst, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        const uint8_t *p = &src[i * 4];
        dst[i].ch.red   = p[0] >> 5;
        dst[i].ch.green = p[1] >> 5;
        dst[i].ch.blue  = p[2] >> 6;
    }
}
static inline void _cf_rgba_to_1(const uint8_t *src, gt_color_t *dst, uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        const uint8_t *p = &src[i * 4];
        uint8_t lum = (p[0] * 77 + p[1] * 150 + p[2] * 29) >> 8;
        dst[i].full = (lum >= 128) ? 0x07 : 0x00;
    }
}

/* ---- dispatcher: returns a convert function, or NULL if formats match ---- */
typedef void (*cf_convert_fn)(const uint8_t *src, gt_color_t *dst, uint32_t count);

static inline cf_convert_fn _get_convert_fn(uint8_t src_cf, uint8_t dst_cf) {
    /* RGB565_SWAP always needs conversion even when dst is depth=16,
     * because the byte order still differs from gt_color16_t native layout. */
    if (src_cf == GT_IMG_CF_RGB565_SWAP) {
#if GT_COLOR_DEPTH == 32
        return (cf_convert_fn)_cf_2s_to_32;
#elif GT_COLOR_DEPTH == 16
        return (cf_convert_fn)_cf_2s_to_16;
#elif GT_COLOR_DEPTH == 8
        return (cf_convert_fn)_cf_2s_to_8;
#elif GT_COLOR_DEPTH == 1
        return (cf_convert_fn)_cf_2s_to_1;
#endif
    }
    if (src_cf == dst_cf) return NULL;
    switch (src_cf) {
    case GT_IMG_CF_RGB565:  /* 2 */
#if GT_COLOR_DEPTH == 32
        return (cf_convert_fn)_cf_2_to_32;
#elif GT_COLOR_DEPTH == 8
        return (cf_convert_fn)_cf_2_to_8;
#elif GT_COLOR_DEPTH == 1
        return (cf_convert_fn)_cf_2_to_1;
#endif
        break;
    case GT_IMG_CF_RGB888:  /* 3 */
#if GT_COLOR_DEPTH == 32
        return (cf_convert_fn)_cf_3_to_32;
#elif GT_COLOR_DEPTH == 16
        return (cf_convert_fn)_cf_3_to_16;
#elif GT_COLOR_DEPTH == 8
        return (cf_convert_fn)_cf_3_to_8;
#elif GT_COLOR_DEPTH == 1
        return (cf_convert_fn)_cf_3_to_1;
#endif
        break;
    case GT_IMG_CF_BGRA8888:  /* 4, byte layout: B G R A */
#if GT_COLOR_DEPTH == 32
        /* gt_color32_t is also BGRA — data matches natively, no conversion needed */
        return NULL;
#elif GT_COLOR_DEPTH == 16
        return (cf_convert_fn)_cf_4_to_16;
#elif GT_COLOR_DEPTH == 8
        return (cf_convert_fn)_cf_4_to_8;
#elif GT_COLOR_DEPTH == 1
        return (cf_convert_fn)_cf_4_to_1;
#endif
        break;
    case GT_IMG_CF_RGBA8888:  /* 5, byte layout: R G B A */
#if GT_COLOR_DEPTH == 32
        return (cf_convert_fn)_cf_rgba_to_32;
#elif GT_COLOR_DEPTH == 16
        return (cf_convert_fn)_cf_rgba_to_16;
#elif GT_COLOR_DEPTH == 8
        return (cf_convert_fn)_cf_rgba_to_8;
#elif GT_COLOR_DEPTH == 1
        return (cf_convert_fn)_cf_rgba_to_1;
#endif
        break;
    }
    return NULL; /* unsupported or matching */
}

/** Returns true when the format stores an alpha byte per pixel (interleaved). */
static inline bool _cf_has_embedded_alpha(uint8_t cf) {
    return cf == GT_IMG_CF_BGRA8888 || cf == GT_IMG_CF_RGBA8888;
}


/**
 * @brief Infer source color format from pixel data size.
 * @return GT_IMG_CF_RGB565, GT_IMG_CF_RGB888, GT_IMG_CF_BGRA8888, or GT_IMG_CF_UNKNOWN
 */
static inline uint8_t _guess_img_cf(_gt_img_dsc_st * dsc_img) {
    uint8_t cf = dsc_img->header.color_format;
    if (cf != GT_IMG_CF_UNKNOWN) return cf;
    /* C array images store color (RGB565) and alpha separately.
     * alpha != NULL means separate alpha channel, NOT interleaved ARGB8888. */
    return GT_IMG_CF_RGB565;
}

/**
 * @brief In vertical flush mode the blend engine reads src in column-major
 *        order (stride = dst_area->h).  Raw/array image data is always
 *        row-major, so a full-image blend produces garbage when w != h.
 *        This helper splits any such blend into one-row-at-a-time calls so
 *        the stride seen by the blend engine is always 1.
 *
 * @param color_base  First pixel of the image (row-major, RGB565)
 * @param alpha_base  Per-pixel opacity array, or NULL if none
 * @param w           Image width in pixels
 * @param h           Image height in pixels
 */
static GT_ATTRIBUTE_RAM_TEXT void _blend_rowwise_in_vert(
    _gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st * blend_dsc,
    const gt_color_t * color_base, const gt_opa_t * alpha_base,
    uint16_t w, uint16_t h)
{
    uint16_t y = 0;
    blend_dsc->dst_area->h = 1;
    while (y < h) {
        if (gt_area_is_intersect_screen(&draw_ctx->buf_area, blend_dsc->dst_area)) {
            blend_dsc->dst_buf  = color_base ? (gt_color_t *)(color_base + (uint32_t)y * w) : NULL;
            blend_dsc->mask_buf = alpha_base ? (gt_opa_t *)(alpha_base + (uint32_t)y * w) : NULL;
            gt_draw_blend(draw_ctx, blend_dsc);
        }
        ++blend_dsc->dst_area->y;
        ++blend_dsc->mask_area->y;
        ++y;
    }
    blend_dsc->dst_buf  = NULL;
    blend_dsc->mask_buf = NULL;
}

static void GT_ATTRIBUTE_RAM_TEXT
_filling_by_ram_data(_gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st * blend_dsc, _gt_img_dsc_st * dsc_img) {
    uint16_t w = dsc_img->header.w;
    uint16_t h = dsc_img->header.h;
    uint32_t pixel_count = (uint32_t)w * h;

    /* Infer source color format */
    uint8_t src_cf = _guess_img_cf(dsc_img);
    uint8_t dst_cf = GT_COLOR_DEPTH;

    /* If formats differ, convert to temp buffer first */
    cf_convert_fn convert = _get_convert_fn(src_cf, dst_cf);
    gt_color_t *color_buf = NULL;
    uint8_t *alpha_buf = NULL;
    bool alpha_buf_owned = false;

    /* When src format embeds alpha (BGRA8888/RGBA8888) and no separate alpha segment
     * is provided, extract the alpha channel into a per-pixel buffer so the engine
     * can blend transparently instead of treating every pixel as fully opaque. */
    bool need_extract_alpha = _cf_has_embedded_alpha(src_cf) && (dsc_img->alpha == NULL);

    if (convert) {
        uint32_t color_bytes = pixel_count * sizeof(gt_color_t);
        uint32_t alpha_bytes = (dsc_img->alpha || need_extract_alpha) ? pixel_count * sizeof(gt_opa_t) : 0;
        color_buf = gt_mem_malloc(color_bytes + alpha_bytes);
        if (!color_buf) {
            GT_LOGE(GT_LOG_TAG_GUI, "Color conversion buffer malloc failed");
            return;
        }
        convert(dsc_img->img, color_buf, pixel_count);
        if (dsc_img->alpha) {
            alpha_buf = (uint8_t *)(color_buf + pixel_count);
            gt_memcpy(alpha_buf, dsc_img->alpha, alpha_bytes);
        } else if (need_extract_alpha) {
            /* Extract alpha from interleaved source; alpha is always the last byte (index 3) */
            alpha_buf = (uint8_t *)(color_buf + pixel_count);
            const uint8_t *src = (const uint8_t *)dsc_img->img;
            for (uint32_t i = 0; i < pixel_count; i++) {
                alpha_buf[i] = src[i * 4 + 3];
            }
        }
    } else {
        /* Fast path: formats match, use original data directly */
        color_buf = (gt_color_t *)dsc_img->img;
        alpha_buf = dsc_img->alpha;
        /* depth=32 BGRA/RGBA: noop convert but alpha still needs extraction */
        if (need_extract_alpha) {
            uint8_t *extracted = gt_mem_malloc(pixel_count * sizeof(gt_opa_t));
            if (extracted) {
                const uint8_t *src = (const uint8_t *)dsc_img->img;
                for (uint32_t i = 0; i < pixel_count; i++) {
                    extracted[i] = src[i * 4 + 3];
                }
                alpha_buf = extracted;
                alpha_buf_owned = true;
            }
        }
    }

    bool check_mask = gt_mask_check(blend_dsc->mask_area);
    if(check_mask){
        gt_opa_t* alpha = (gt_opa_t *)alpha_buf;
        uint16_t y = 0;
        uint32_t offset = 0;
        uint32_t len = w * sizeof(gt_opa_t);

        blend_dsc->mask_buf = gt_mem_malloc(w);
        if (!blend_dsc->mask_buf) {
            GT_LOGE(GT_LOG_TAG_GUI, "Mask buffer malloc failed, size: %d, %d", w, h);
            if (alpha_buf_owned) gt_mem_free(alpha_buf);
            if (convert) gt_mem_free(color_buf);
            return;
        }

        blend_dsc->dst_buf = color_buf;
        gt_opa_t opa = blend_dsc->opa >= GT_OPA_MAX ? GT_OPA_COVER: blend_dsc->opa;
        blend_dsc->dst_area->h = 1;
        while (y < h) {
            if (gt_area_is_intersect_screen(&draw_ctx->buf_area, blend_dsc->dst_area)) {
                if (alpha) {
                    gt_memcpy(blend_dsc->mask_buf, &alpha[offset], w);
                } else {
                    gt_memset(blend_dsc->mask_buf, opa, w);
                }
                gt_mask_get(blend_dsc->mask_buf, blend_dsc->mask_area->x, blend_dsc->mask_area->y, w);
                gt_draw_blend(draw_ctx, blend_dsc);
            }
            blend_dsc->dst_buf += len;
            offset += w;
            ++blend_dsc->dst_area->y;
            ++blend_dsc->mask_area->y;
            ++y;
        }

        gt_mem_free(blend_dsc->mask_buf);
        blend_dsc->mask_buf = NULL;
        blend_dsc->dst_buf = NULL;
        if (alpha_buf_owned) gt_mem_free(alpha_buf);
        if (convert) gt_mem_free(color_buf);
        return;
    }

    if (gt_draw_blend_is_vertical() && h > 1) {
        _blend_rowwise_in_vert(draw_ctx, blend_dsc,
            color_buf, (gt_opa_t *)alpha_buf, w, h);
        if (alpha_buf_owned) gt_mem_free(alpha_buf);
        if (convert) gt_mem_free(color_buf);
        return;
    }

    blend_dsc->dst_buf = color_buf;
    blend_dsc->mask_buf = (gt_opa_t *)alpha_buf;
    gt_draw_blend(draw_ctx, blend_dsc);
    blend_dsc->dst_buf = NULL;
    blend_dsc->mask_buf = NULL;
    if (alpha_buf_owned) gt_mem_free(alpha_buf);
    if (convert) gt_mem_free(color_buf);
}

#if GT_USE_EXTRA_FULL_IMG_BUFFER
static gt_res_t GT_ATTRIBUTE_RAM_TEXT
_filling_by_default_full_mode(_gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st * blend_dsc, _gt_img_dsc_st * dsc_img) {
    uint16_t w = dsc_img->header.w;
    uint16_t h = dsc_img->header.h;
    uint32_t pixel_count = (uint32_t)w * h;

    gt_fs_fp_st * fp = dsc_img->fp;
    if (!fp) {
        GT_LOGW(GT_LOG_TAG_GUI, "gt_fs_get_fp failed");
        return GT_RES_FAIL;
    }

    /* Determine source color format */
    uint8_t src_cf = dsc_img->header.color_format;
    if (src_cf == 0) src_cf = fp->msg.pic.color_format;
    if (src_cf == 0) src_cf = GT_IMG_CF_RGB565;
    uint8_t src_bpp = gt_img_cf_bpp(src_cf);
    if (src_bpp == 0) src_bpp = 2;
    uint8_t dst_cf = GT_COLOR_DEPTH;
    cf_convert_fn convert = _get_convert_fn(src_cf, dst_cf);

    uint8_t is_mask = 1 == fp->msg.pic.is_alpha ? 1 : 0;
    bool check_mask = gt_mask_check(blend_dsc->mask_area);

    _gt_gc_full_img_buffer_st * full_img = _gt_gc_get_full_img_buffer();
    if (NULL == full_img->buffer) {
        GT_LOGV(GT_LOG_TAG_GUI, "Extra buffer color or alpha pointer is NULL");
        return GT_RES_INV;
    }

    /* Calculate actual read size and converted size */
    uint32_t color_read_size = pixel_count * src_bpp;
    uint32_t alpha_read_size = is_mask ? pixel_count : 0;
    uint32_t color_converted_size = pixel_count * sizeof(gt_color_t);
    uint32_t total_need = color_read_size + alpha_read_size;

    /* Check if full image buffer can hold converted data */
    uint32_t blend_size = color_converted_size + (is_mask || check_mask ? pixel_count : 0);
    if (full_img->len < blend_size) {
        GT_LOGV(GT_LOG_TAG_GUI, "Extra buffer color or alpha size is not enough to store image data");
        return GT_RES_INV;
    }

    /* Read raw data from file */
    gt_fs_read_img_offset(fp, (uint8_t *)full_img->buffer, 0, total_need);

    if (convert) {
        /* Convert color data to gt_color_t format */
        convert((uint8_t *)full_img->buffer, (gt_color_t *)full_img->buffer, pixel_count);
        if (is_mask) {
            /* Alpha data is after color_read_size bytes in raw, copy to after converted color */
            uint8_t *src_alpha = (uint8_t *)full_img->buffer + color_read_size;
            uint8_t *dst_alpha = (uint8_t *)full_img->buffer + color_converted_size;
            gt_memcpy(dst_alpha, src_alpha, pixel_count);
        }
    }

#if GT_USE_EXTRA_FULL_IMG_OFFSET
    blend_dsc->dst_buf = (gt_color_t * )&full_img->buffer[full_img->offset];
#else
    blend_dsc->dst_buf = (gt_color_t * )full_img->buffer;
#endif

    if (is_mask || check_mask) {
#if GT_USE_EXTRA_FULL_IMG_OFFSET
        blend_dsc->mask_buf = (gt_opa_t * )&full_img->buffer[color_converted_size + full_img->offset];
#else
        blend_dsc->mask_buf = (gt_opa_t * )&full_img->buffer[color_converted_size];
#endif
    }

    if(check_mask) {
        gt_opa_t opa = blend_dsc->opa >= GT_OPA_MAX ? GT_OPA_COVER: blend_dsc->opa;
        uint8_t *opa_buf = blend_dsc->mask_buf;
        for(uint32_t i = 0; i < h; ++i) {
            if(!is_mask) {
                gt_memset(opa_buf, opa, w);
            }
            gt_mask_get(opa_buf, blend_dsc->mask_area->x, blend_dsc->mask_area->y + i, w);
            opa_buf += w;
        }
    }

    if (gt_draw_blend_is_vertical() && h > 1) {
        _blend_rowwise_in_vert(draw_ctx, blend_dsc,
            (const gt_color_t *)blend_dsc->dst_buf,
            (is_mask || check_mask) ? (const gt_opa_t *)blend_dsc->mask_buf : NULL,
            w, h);
        goto line_lb;
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
static void GT_ATTRIBUTE_RAM_TEXT
_filling_by_default_line_mode(_gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st * blend_dsc, _gt_img_dsc_st * dsc_img) {
    uint16_t w, h, y = 0;
    w = dsc_img->header.w;
    h = dsc_img->header.h;

    gt_fs_fp_st * fp = dsc_img->fp;
    if (!fp) {
        GT_LOGW(GT_LOG_TAG_GUI, "gt_fs_get_fp failed");
        return;
    }

    /* Determine source color format and conversion */
    uint8_t src_cf = dsc_img->header.color_format;
    if (src_cf == 0) src_cf = fp->msg.pic.color_format;
    if (src_cf == 0) src_cf = GT_IMG_CF_RGB565; /* fallback */
    uint8_t src_bpp = gt_img_cf_bpp(src_cf);
    if (src_bpp == 0) src_bpp = sizeof(gt_color_t);
    uint8_t dst_cf = GT_COLOR_DEPTH;
    cf_convert_fn convert = _get_convert_fn(src_cf, dst_cf);

    uint32_t color_offset = 0;
    uint32_t alpha_offset = (uint32_t)w * h * src_bpp;
    uint32_t color_len = w * src_bpp;
    uint32_t blend_len = w * sizeof(gt_color_t);

    blend_dsc->dst_area->h = 1;

    /* Allocate buffers: raw source line + converted color line */
#if _GT_DRAW_STATIC_LINE_BUFFER
    uint8_t *raw_buf = (uint8_t *)_static_line_color_buffer;
    blend_dsc->dst_buf = (gt_color_t *)_static_line_opa_buffer;
#else
    uint8_t *raw_buf = gt_mem_malloc(color_len > blend_len ? color_len : blend_len);
    if (!raw_buf) {
        GT_LOGW(GT_LOG_TAG_GUI, "Per-line buffer malloc failed, size: %d, %d", w, h);
        return;
    }
    if (!convert) {
        blend_dsc->dst_buf = (gt_color_t *)raw_buf;
    } else {
        blend_dsc->dst_buf = gt_mem_malloc(blend_len);
        if (!blend_dsc->dst_buf) {
            gt_mem_free(raw_buf);
            GT_LOGW(GT_LOG_TAG_GUI, "Converted line buffer malloc failed");
            return;
        }
    }
#endif

    gt_opa_t opa = blend_dsc->opa >= GT_OPA_MAX ? GT_OPA_COVER: blend_dsc->opa;
    bool check_mask = gt_mask_check(blend_dsc->mask_area);
    uint8_t is_mask = 1 == fp->msg.pic.is_alpha ? 1 : 0;
    /* For interleaved-alpha formats without a separate alpha segment, extract per row */
    bool extract_alpha = !is_mask && _cf_has_embedded_alpha(src_cf);

    if (is_mask || check_mask || extract_alpha) {
#if _GT_DRAW_STATIC_LINE_BUFFER
        blend_dsc->mask_buf = (uint8_t *)(_static_line_opa_buffer + blend_len);
#else
        blend_dsc->mask_buf = gt_mem_malloc(w);
        if (!blend_dsc->mask_buf) {
            goto line_lb;
        }
#endif
    }

    while (y < h) {
        if (gt_area_is_intersect_screen(&draw_ctx->buf_area, blend_dsc->dst_area)) {
            /* Read one line color data from source */
            gt_fs_read_img_offset(fp, raw_buf, color_offset, color_len);

            /* Convert if formats differ */
            if (convert) {
                convert(raw_buf, (gt_color_t *)blend_dsc->dst_buf, w);
            } else if (!convert && (void *)blend_dsc->dst_buf != (void *)raw_buf) {
                gt_memcpy(blend_dsc->dst_buf, raw_buf, color_len);
            }

            if(!is_mask && !extract_alpha && check_mask){
                gt_memset(blend_dsc->mask_buf, opa, w);
            }

            if (is_mask) {
                gt_fs_read_img_offset(fp, (uint8_t *)blend_dsc->mask_buf, alpha_offset, w);
            } else if (extract_alpha) {
                /* Extract alpha byte (index 3) from each interleaved BGRA/RGBA pixel */
                const uint8_t *px = raw_buf;
                gt_opa_t *dst_a = (gt_opa_t *)blend_dsc->mask_buf;
                for (uint16_t xi = 0; xi < w; xi++, px += 4) {
                    dst_a[xi] = (gt_opa_t)((uint16_t)px[3] * opa / 255u);
                }
            }

            if(check_mask) {
                gt_mask_get(blend_dsc->mask_buf, blend_dsc->mask_area->x, blend_dsc->mask_area->y, w);
            }
            /* blend to bg */
            gt_draw_blend(draw_ctx, blend_dsc);
        }
        color_offset += color_len;
        alpha_offset += w;

        ++blend_dsc->dst_area->y;
        ++blend_dsc->mask_area->y;
        ++y;
    }

#if _GT_DRAW_STATIC_LINE_BUFFER
    blend_dsc->mask_buf = NULL;
    blend_dsc->dst_buf = NULL;
#else
    if (blend_dsc->mask_buf) {
        gt_mem_free(blend_dsc->mask_buf);
        blend_dsc->mask_buf = NULL;
    }
line_lb:
    if (blend_dsc->dst_buf && (void *)blend_dsc->dst_buf != (void *)raw_buf) {
        gt_mem_free(blend_dsc->dst_buf);
        blend_dsc->dst_buf = NULL;
    }
    if (raw_buf) {
        gt_mem_free(raw_buf);
    }
#endif
}

static inline void GT_ATTRIBUTE_RAM_TEXT
_filling_by_default(_gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st * blend_dsc, _gt_img_dsc_st * dsc_img) {
    gt_fs_fp_st * fp = dsc_img->fp;

    if (!fp) {
        GT_LOGW(GT_LOG_TAG_GUI, "gt_fs_get_fp failed");
        return;
    }

    bool check_mask = gt_mask_check(blend_dsc->mask_area);

    if(GT_FS_TYPE_ARRAY != fp->type || check_mask){
#if GT_USE_EXTRA_FULL_IMG_BUFFER
        if (GT_RES_OK == _filling_by_default_full_mode(draw_ctx, blend_dsc, dsc_img)) {
            return;
        }
#endif
        _filling_by_default_line_mode(draw_ctx, blend_dsc, dsc_img);
        return;
    }

    /* src BPP: use color_format field (2/3/4 = bytes/pixel), fallback to 2 (RGB565) */
    uint8_t src_cf = dsc_img->header.color_format;
    if (src_cf == 0) src_cf = fp->msg.pic.color_format;
    if (src_cf == 0) src_cf = GT_IMG_CF_RGB565;
    uint8_t dst_cf = GT_COLOR_DEPTH;
    uint8_t is_mask = fp->msg.pic.is_alpha ? 1 : 0;

    /* alpha data immediately follows color data; offset uses actual src BPP */
    uint32_t pixel_count = (uint32_t)dsc_img->header.w * dsc_img->header.h;
    uint32_t addr_start_alpha = pixel_count * gt_img_cf_bpp(src_cf);

    /* set dsc_img fields so _filling_by_ram_data can use them */
    dsc_img->img   = (uint8_t *)fp->buffer;
    dsc_img->alpha = is_mask ? (gt_opa_t *)((uint8_t *)fp->buffer + addr_start_alpha) : NULL;
    dsc_img->header.color_format = src_cf;

    if (src_cf != dst_cf) {
        /* formats differ: delegate to conversion path */
        _filling_by_ram_data(draw_ctx, blend_dsc, dsc_img);
        return;
    }

    /* fast path: formats match, use buffer directly */
    blend_dsc->dst_buf  = (gt_color_t *)fp->buffer;
    blend_dsc->mask_buf = is_mask ? (gt_opa_t *)((uint8_t *)fp->buffer + addr_start_alpha) : NULL;

    if(NULL == blend_dsc->dst_buf){
        GT_LOGW(GT_LOG_TAG_GUI, "Extra buffer color pointer is NULL");
        return;
    }
    if (gt_draw_blend_is_vertical() && dsc_img->header.h > 1) {
        _blend_rowwise_in_vert(draw_ctx, blend_dsc,
            (const gt_color_t *)blend_dsc->dst_buf,
            is_mask ? (const gt_opa_t *)blend_dsc->mask_buf : NULL,
            dsc_img->header.w, dsc_img->header.h);
        return;
    }
    gt_draw_blend(draw_ctx, blend_dsc);
    blend_dsc->dst_buf = NULL;
    blend_dsc->mask_buf = NULL;
}

static GT_ATTRIBUTE_RAM_TEXT inline bool _is_align_center(gt_align_et align) {
    return GT_ALIGN_CENTER == align || GT_ALIGN_CENTER_MID == align || GT_ALIGN_CENTER_BOTTOM == align;
}

static GT_ATTRIBUTE_RAM_TEXT inline gt_align_et _is_verial_mid_or_bottom(gt_align_et align) {
    return GT_ALIGN_LEFT_MID == align || GT_ALIGN_LEFT_BOTTOM == align ||
        GT_ALIGN_CENTER_MID == align || GT_ALIGN_RIGHT_MID == align ||
        GT_ALIGN_CENTER_BOTTOM == align || GT_ALIGN_RIGHT_BOTTOM == align;
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
static GT_ATTRIBUTE_RAM_TEXT inline gt_size_t _get_align_start_x(gt_align_et align, gt_size_t x,
    uint16_t width, uint16_t string_width) {
    gt_align_et type = _get_align_type(align);

    if (_is_align_center(type)) {
        return ((width - string_width) >> 1) + x;
    }
    if (GT_ALIGN_RIGHT == type || GT_ALIGN_RIGHT_MID == type || GT_ALIGN_RIGHT_BOTTOM == type) {
        return (width - string_width) + x;
    }
    return x;
}

/**
 * @brief 计算行首 x 坐标（含对齐、reverse、start_x、首行缩进）
 * @param dsc           字体属性
 * @param coords        控件区域
 * @param disp_w        当前行可用宽度
 * @param align_reverse 是否右到左对齐
 * @param start_x_flag  是否使用 dsc->start_x 直接定位
 * @param is_first_line 是否首行（需要缩进）
 * @param font_info     字体信息（用于缩进宽度计算）
 * @return 行首 x 坐标
 */
static GT_ATTRIBUTE_RAM_TEXT inline gt_size_t _calc_line_start_x(
    const gt_attr_font_st * dsc, const gt_area_st * coords,
    gt_size_t disp_w, bool align_reverse,
    bool start_x_flag, bool is_first_line,
    const gt_font_info_st * font_info)
{
    if (start_x_flag) { return dsc->start_x; }
    gt_size_t x = _get_align_start_x(dsc->align, coords->x, disp_w, 0);
    if (align_reverse) {
        x = (coords->x + coords->w) - (x - coords->x);
    }
    if (is_first_line) {
        gt_size_t indent = gt_font_get_indent_width(font_info, dsc->indent);
        x = align_reverse ? x - indent : x + indent;
    }
    return x;
}

static gt_size_t GT_ATTRIBUTE_RAM_TEXT
_get_align_start_y_by_line(const gt_attr_font_st * dsc,
    const gt_area_st * coords, gt_font_st * tmp_font, uint8_t * txt, uint16_t line_count) {
    gt_size_t offset_y = coords->y;
    uint16_t line_height = (_get_effective_font_size((const gt_font_info_st*)&tmp_font->info) + (_get_style_space_y((const gt_font_info_st*)&tmp_font->info) << 1) + dsc->space_y) * line_count;
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

static _text_offset_line_st GT_ATTRIBUTE_RAM_TEXT
_get_align_start_y(const gt_attr_font_st * dsc,
    const gt_area_st * coords, gt_font_st * tmp_font, uint8_t * txt, uint16_t txt_len) {
    _text_offset_line_st ret = {
        .line_count = 1,
    };
    gt_split_line_ret_st s_line_ret = {0};
    gt_size_t disp_w = coords->w;
    gt_align_et type = _get_align_type(dsc->align);
    uint16_t indent = 0;

    if (dsc->reg.enabled_start) {
        disp_w = coords->w - (dsc->start_x - coords->x);
    } else {
        indent = dsc->indent;
    }

    if (_is_verial_mid_or_bottom(type)) {
        /** calc total line count to calc position offset */
        gt_font_split_line_st sp_line = {
            .text = (char * )txt,
            .len = txt_len,
            .max_w = coords->w,
            .start_w = disp_w,
            .space = _get_style_space_x(dsc->space_x, (const gt_font_info_st *)&tmp_font->info),
            .indent = indent,
        };

        s_line_ret = gt_font_split_line_numb(&tmp_font->info, &sp_line);
        ret.line_count = s_line_ret.line_numb;
    }

    ret.offset_y = _get_align_start_y_by_line(dsc, coords, tmp_font, txt, ret.line_count);
    return ret;
}

static _gt_draw_font_res_st GT_ATTRIBUTE_RAM_TEXT
_empty_string_handler(const gt_attr_font_st * dsc) {
    const gt_area_st * coords = &dsc->logical_area;
    gt_align_et type = _get_align_type(dsc->align);
    _gt_draw_font_res_st ret = {
        .res = GT_RES_OK,
        .abs_pos = {
            .x = coords->x, .y = coords->y
        },
        .size = {
            .x = dsc->reg.single_line ? 0xffff : 0,
            .y = 0
        },
    };
    if (GT_ALIGN_LEFT_MID == type || GT_ALIGN_RIGHT_MID == type || GT_ALIGN_CENTER_MID == type) {
        if (dsc->font->info.size < coords->h) {
            ret.abs_pos.y = coords->y + ((coords->h - dsc->font->info.size) >> 1);
        }
    }
    return ret;
}

/**
 * @brief
 *
 * @param tmp_font
 * @return int8_t offset y value
 */
static int8_t inline GT_ATTRIBUTE_RAM_TEXT
_font_get_group_offset_by_type(gt_font_st const * const tmp_font_p) {
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
    return _gt_font_get_type_group_offset_y(tmp_font_p->info.style_cn, tmp_font_p->info.style_en);
#else
    return _gt_font_get_type_group_offset_y(
        gt_font_family_get_option(tmp_font_p->info.family, FONT_LAN_ASCII, tmp_font_p->info.cjk), \
        gt_font_family_get_option(tmp_font_p->info.family, FONT_LAN_CN, tmp_font_p->info.cjk));
#endif
}

/* ---------- shared font-render context ---------- */
typedef struct {
    gt_font_st        tmp_font;
    uint8_t *         mask_line;
    uint8_t *         txt;
    uint16_t          txt_len;
    uint16_t          data_len;
    uint8_t           lan;
    int8_t            offset_y;
    _draw_font_buf_st font_buf;
#if _GT_FONT_ENABLE_CONVERTOR
    uint8_t *         ret_txt;
#endif
} _draw_text_ctx_st;

/* Returns: 0=ok, -1=res alloc fail, -2=mask alloc fail, -3=bidi txt alloc fail */
static int8_t _draw_text_ctx_init(_draw_text_ctx_st * ctx, const gt_attr_font_st * dsc) {
    ctx->tmp_font.info  = dsc->font->info;
    ctx->tmp_font.utf8  = dsc->font->utf8;
    ctx->tmp_font.len   = dsc->font->len;
    ctx->tmp_font.res   = NULL;
    ctx->tmp_font.info.gray = dsc->font->info.gray == 0 ? 1 : dsc->font->info.gray;
    ctx->offset_y = _font_get_group_offset_by_type(&ctx->tmp_font);
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(&ctx->tmp_font.info, 2, 0, 0);
#else
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(&ctx->tmp_font.info, FONT_LAN_UNKNOWN, 0);
#endif
    ctx->data_len = font_size_res.font_per_size;
    ctx->font_buf = _draw_font_buf_alloc(ctx->tmp_font.info.size, ctx->data_len);
    ctx->tmp_font.res = ctx->font_buf.res;
    ctx->mask_line    = ctx->font_buf.mask_line;
    if (NULL == ctx->tmp_font.res) { return -1; }
    if (NULL == ctx->mask_line)    { return -2; }
    gt_memset(ctx->mask_line, 0xFF, ctx->tmp_font.info.size << 1);

    ctx->txt     = (uint8_t *)dsc->font->utf8;
    ctx->txt_len = dsc->font->len;
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
    ctx->lan = right_to_left_lan_get(dsc->font->info.style_fl);
#else
    ctx->lan = right_to_left_lan_get(&ctx->tmp_font);
#endif

#if _GT_FONT_ENABLE_CONVERTOR
    ctx->ret_txt = NULL;
    if (ctx->lan == FONT_LAN_HEBREW || ctx->lan == FONT_LAN_ARABIC) {
        ctx->ret_txt = gt_mem_malloc(ctx->txt_len + 1);
        if (NULL == ctx->ret_txt) { return -3; }
        gt_memset_0(ctx->ret_txt, ctx->txt_len + 1);
        if (gt_right_to_left_handler(&ctx->tmp_font, ctx->ret_txt, ctx->lan)) {
            ctx->txt = ctx->ret_txt;
        }
    }
#endif
    return 0;
}

static void _draw_text_ctx_free(_draw_text_ctx_st * ctx) {
#if _GT_FONT_ENABLE_CONVERTOR
    if (ctx->ret_txt) { gt_mem_free(ctx->ret_txt); ctx->ret_txt = NULL; }
#endif
    ctx->font_buf.mask_line = ctx->mask_line;
    ctx->font_buf.res       = ctx->tmp_font.res;
    _draw_font_buf_free(&ctx->font_buf);
    ctx->mask_line    = NULL;
    ctx->tmp_font.res = NULL;
}

static _gt_draw_font_res_st _GT_DRAW_ATTRIBUTE_SRAM_TEXT
draw_text_multiple_line(_gt_draw_ctx_st * draw_ctx, const gt_attr_font_st * dsc, gt_area_st const * view) {
    const gt_area_st * coords = &dsc->logical_area;
    const gt_font_size_t font_size = dsc->font->info.size;
    const gt_size_t coords_w = coords->w;
    _gt_draw_font_res_st ret = { .res = GT_RES_OK };
    gt_area_st area_font = {
        .x = coords->x, .y = coords->y, .w = font_size, .h = font_size
    };
    _draw_text_ctx_st _ctx;
    {
        int8_t _init_ret = _draw_text_ctx_init(&_ctx, dsc);
        if (_init_ret == -1) { goto _res_handler; }
        if (_init_ret == -2) { goto _mask_handler; }
#if _GT_FONT_ENABLE_CONVERTOR
        if (_init_ret == -3) {
            GT_LOGE(GT_LOG_TAG_GUI, "ret_txt malloc is failed!!!");
            goto _txt_handler;
        }
#endif
    }
    gt_font_st * tmp_font_p = &_ctx.tmp_font;
    uint8_t * const mask_line  = _ctx.mask_line;
    uint8_t * txt              = _ctx.txt;
    const uint16_t txt_len     = _ctx.txt_len;
    const uint16_t data_len    = _ctx.data_len;
    const uint8_t  lan         = _ctx.lan;
    const int8_t   offset_y    = _ctx.offset_y;
    gt_size_t font_disp_h      = tmp_font_p->info.size;
    #define tmp_font  (*tmp_font_p)
    uint8_t space_x = _get_style_space_x(dsc->space_x, (const gt_font_info_st *)&tmp_font.info);
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

    uint8_t style_space_y = _get_style_space_y((const gt_font_info_st*)&tmp_font.info);
    const gt_font_size_t eff_font_size = _get_effective_font_size((const gt_font_info_st*)&tmp_font.info);
    uint16_t per_line_h = (style_space_y << 1) + eff_font_size + dsc->space_y;
    const bool ml_has_ul = dsc->font->info.style.reg.underline;
    const bool ml_has_st = dsc->font->info.style.reg.strikethrough;

    gt_size_t disp_w = coords_w;
    uint32_t uni_or_gbk = 0, ret_w = 0;
    uint16_t idx = 0, idx_2 = 0, idx_len = 0;
    uint8_t over_length = 0;
    uint16_t prev_rendered_size = data_len;
    int32_t idx_step = 0;
    const bool align_reverse = _is_align_reverse(dsc->align);

#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
    gt_font_touch_word_st tmp_word = {0};
    gt_font_lan_et prev_sty = FONT_LAN_UNKNOWN;
    gt_font_lan_et cur_sty = FONT_LAN_UNKNOWN;
#endif
    bool cov_flag_ol = false;
    bool start_x_flag = dsc->reg.enabled_start;
    bool is_first_line = true;
    _gt_font_dot_ret_st dot_ret = {0};
    uint16_t line_height = per_line_h;
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
    _text_offset_line_st off_line = _get_align_start_y(dsc, coords, &tmp_font, txt, txt_len);
    area_font.y = off_line.offset_y;
    if (dsc->reg.enabled_start) {
        area_font.y = dsc->start_y;
    }
    // disp
    if (start_x_flag) {
        /** have direct position do not indent */
        disp_w = coords_w - (dsc->start_x - coords->x);
        is_first_line = false;
    } else if (1 == off_line.line_count && _is_align_center(dsc->align)) {
        /** center single line do not indent */
        is_first_line = false;
    } else if (is_first_line) {
        disp_w -= gt_font_get_indent_width(&dsc->font->info, dsc->indent);
    }

    const int32_t coords_y_bottom = (int32_t)coords->y + coords->h;
    const int32_t view_y_bottom   = (int32_t)view->y + view->h;
    const gt_size_t view_x_right    = view->x + view->w;

    while (idx < txt_len) {
        // out of range
        if ( (area_font.y + style_space_y + eff_font_size) > coords_y_bottom ) {
            font_disp_h = coords_y_bottom - area_font.y;
            if(font_disp_h < 2){
                break;
            }
        }
        if(0x0A == txt[idx]){
            ++idx;
            goto _disp_font;
        }
        if (area_font.y > view_y_bottom) {
            if (dsc->reg.immediately_return) {
                ret.res = GT_RES_INV;
                goto _ret_handler;
            }
        }
        tmp_font.utf8 = (char * )&txt[idx];
        tmp_font.len = txt_len - idx;

        ret_w = coords_w - disp_w;
#if _GT_FONT_ENABLE_CONVERTOR
        idx_step = gt_font_split_line_str(&tmp_font, coords_w, space_x, &ret_w,
                                            &bidi, &bidi_len, &bidi_max, &over_length,
                                            is_first_line);
#else
        idx_step = gt_font_split_line_str(&tmp_font, coords_w, space_x, &ret_w,
                                            NULL, NULL, NULL, &over_length,
                                            is_first_line);
#endif
        if(idx_step < 0){
            goto _ret_handler;
        }
        else if(idx_step == 0 && coords_w != disp_w){
            if(ret_w == 0 ) {
                if(coords_w >= eff_font_size) {
                    goto _disp_font;
                }

                goto _ret_handler;

            }
            ++idx_len;
            ++idx;
            goto _disp_font;
        }
        disp_w -= ret_w;
        idx += idx_step;
        idx_len += idx_step;
        if (disp_w < 0) { disp_w = 0; }

        if(0 == idx_len && 0 == idx_step) {
            ++idx;
            idx_len = 1;
        }
        // printf("[%d] idx_len: %d, idx_step: %d\n", idx, idx_len, idx_step);
_disp_font:
        /** align to left, center or right */
        if (!cov_flag_ol) {
            area_font.x = _calc_line_start_x(dsc, coords, disp_w, align_reverse,
                                              start_x_flag, is_first_line,
                                              (const gt_font_info_st*)&tmp_font.info);
            if (start_x_flag) { start_x_flag = false; }
            if (is_first_line) { is_first_line = false; }
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
        else if (area_font.y > view_y_bottom) {
            area_font.x += ret_w;
            goto next_line_lb;
        }
        if (ret.size_show_limit.x < ret_w) {
            ret.size_show_limit.x = ret_w;
        }
        ret.size_show_limit.y += line_height;
        if (ret.size_show_limit.y > view->h) {
            ret.size_show_limit.y = view->h;
        }
        if (dsc->reg.only_calc_display_info) { goto next_line_lb; }
        if (dsc->opa < GT_OPA_MIN) { goto next_line_lb; }

        /** ------------- Display current line chars ------------- */
        tmp_font.info.size = font_size;
        while (idx_2 < idx_len) {
#if _GT_FONT_ENABLE_CONVERTOR
            if (bidi_pos < bidi_len && idx_2 == bidi[bidi_pos].idx) {
                tmp_font.utf8 = (char * )&txt_2[idx_2];
                tmp_font.len = bidi[bidi_pos].len;
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
                _gt_draw_font_res_st tmp_ret =
#endif
                _draw_text_convertor_split(&tmp_font, bidi[bidi_pos].flag,
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

            tmp_font.utf8 = (char * )&txt_2[idx_2];
            tmp_font.len = gt_font_one_char_code_len_get(&txt_2[idx_2], &uni_or_gbk, tmp_font.info.encoding);
            const bool is_symbol_ml = gt_symbol_is_valid_range(uni_or_gbk);
            gt_font_size_t width = is_symbol_ml ? tmp_font.info.size
                                         : gt_font_get_one_word_width(uni_or_gbk, &tmp_font);

            if (align_reverse) {
                area_font.x -= width;
            }
            if (area_font.x + width < view->x || area_font.x > view_x_right) {
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
            if (!is_symbol_ml) {
                dot_ret = gt_font_get_dot(&tmp_font, uni_or_gbk);
                if (dot_ret.len < prev_rendered_size) {
                    gt_memset(&tmp_font.res[dot_ret.len], 0, prev_rendered_size - dot_ret.len);
                }
                if (dot_ret.len > 0) { prev_rendered_size = dot_ret.len; }
                _draw_one_char_glyph(draw_ctx, &blend_dsc, mask_line, &tmp_font,
                                     dot_ret, &area_font, offset_y,
                                     &width, uni_or_gbk, dsc, true);
            } else {
                if (!_draw_one_symbol(draw_ctx, &blend_dsc, &tmp_font, &area_font,
                                      uni_or_gbk, data_len, font_disp_h)) {
                    area_font.x += width + space_x;
                    idx_2 += tmp_font.len;
                    continue;
                }
            }

            /* use style */
            _draw_char_style(draw_ctx, dsc, &blend_dsc_style, area_font,
                             (const gt_font_info_st*)&tmp_font.info, width, style_space_y, font_disp_h,
                             ml_has_ul, ml_has_st);
        next_word_multi_lb:
            _advance_x(&area_font.x, width, space_x, align_reverse);
            idx_2 += tmp_font.len;
        }

    next_line_lb:
#if _GT_FONT_ENABLE_CONVERTOR
        // if (_gt_font_is_convertor_language(lan) && over_length > 1) {
        if(bidi_len > 0 && over_length > 1){
            cov_flag_ol = true;
            if (align_reverse) {
                disp_w = area_font.x - coords->x;
            } else {
                disp_w = coords_w - (area_font.x - coords->x);
            }
        } else {
#endif  /** _GT_FONT_ENABLE_CONVERTOR */
            cov_flag_ol = false;
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
                if (align_reverse) {
                    area_font.x = (coords->x + coords_w) - (area_font.x - coords->x);
                } else {
                    area_font.x = _get_align_start_x(dsc->align, coords->x, disp_w, 0);
                }
            }
            disp_w = coords_w;
#if _GT_FONT_ENABLE_CONVERTOR
        }
        bidi_len = 0;
#endif
        if (0x0A == txt[idx - 1]) {
            is_first_line = true;
            disp_w -= gt_font_get_indent_width(&tmp_font.info, dsc->indent);
        } /** need to indent */
        idx_len = 0;
        txt_2 = &txt[idx];
        font_disp_h = font_size;
        area_font.h = font_disp_h;
        area_font.w = font_size;
    }

    if(tmp_font.info.style.reg.italic){
        area_font.x += eff_font_size >> 1;
    }

_ret_handler:
#if _GT_FONT_ENABLE_CONVERTOR
    if (NULL != bidi) { gt_mem_free(bidi); bidi = NULL; }
#endif
_txt_handler:
    #undef tmp_font
    _draw_text_ctx_free(&_ctx);
_mask_handler:
_res_handler:
    ret.abs_pos.x = area_font.x;
    ret.abs_pos.y = area_font.y;
    return ret;
}

static _omit_info_st GT_ATTRIBUTE_RAM_TEXT
_omit_get_font_width(gt_font_st * tmp_font) {
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

static _gt_draw_font_res_st _GT_DRAW_ATTRIBUTE_SRAM_TEXT
draw_text_single_line(_gt_draw_ctx_st * draw_ctx, const gt_attr_font_st * dsc, const gt_area_st * view) {
    gt_area_st * coords = (gt_area_st * )&dsc->logical_area;
    const gt_font_size_t font_size = dsc->font->info.size;
    const gt_font_size_t eff_font_size_sl = _get_effective_font_size((const gt_font_info_st*)&dsc->font->info);
    const gt_size_t coords_w = coords->w;
    _gt_draw_font_res_st ret = {
        .res = GT_RES_OK,
        .size_show_limit = {
            .x = 0, .y = eff_font_size_sl,
        },
    };
    gt_area_st area_font = {
        .x = coords->x, .y = coords->y, .w = font_size, .h = font_size
    };
    _draw_text_ctx_st _ctx;
    {
        int8_t _init_ret = _draw_text_ctx_init(&_ctx, dsc);
        if (_init_ret == -1) { goto direct_handler_lb; }
        if (_init_ret == -2) { goto mask_handler_lb; }
#if _GT_FONT_ENABLE_CONVERTOR
        if (_init_ret == -3) {
            GT_LOGE(GT_LOG_TAG_GUI, "ret_txt malloc is failed!!!");
            goto txt_handler_lb;
        }
#endif
    }
    gt_font_st * tmp_font_p = &_ctx.tmp_font;
    uint8_t * const mask_line  = _ctx.mask_line;
    uint8_t * txt              = _ctx.txt;
    const uint16_t data_len    = _ctx.data_len;
    const uint8_t  lan         = _ctx.lan;
    const int8_t   offset_y    = _ctx.offset_y;
    gt_size_t font_disp_h      = tmp_font_p->info.size;
    #define tmp_font  (*tmp_font_p)

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
    uint8_t style_space_y = _get_style_space_y((const gt_font_info_st*)&tmp_font.info);
    uint8_t space_x = _get_style_space_x(dsc->space_x, (const gt_font_info_st *)&tmp_font.info);
    const bool sl_has_ul = dsc->font->info.style.reg.underline;
    const bool sl_has_st = dsc->font->info.style.reg.strikethrough;
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
    gt_font_touch_word_st tmp_word = {0};
    gt_font_lan_et prev_sty = FONT_LAN_UNKNOWN;
    gt_font_lan_et cur_sty = FONT_LAN_UNKNOWN;
#endif
    gt_size_t string_total_width = 0, view_width = coords_w;
    uint32_t uni_or_gbk = 0, remark_idx = 0, lan_len = 0;
    uint16_t idx = 0, idx_len = 0, txt_len = dsc->font->len;
    uint8_t over_length = 0;
    uint16_t prev_rendered_size = data_len;
    _omit_info_st omit_info = {0};
    bool start_x_flag = dsc->reg.enabled_start;
    bool is_omit_display = false;
    _gt_font_dot_ret_st dot_ret = {0};
    gt_font_size_t width; uint16_t byte_width; uint8_t font_gray;
    const bool align_reverse = _is_align_reverse(dsc->align);

#if _GT_FONT_ENABLE_CONVERTOR
    uint16_t bidi_len = 0, bidi_max = 1, bidi_pos = 0;
    gt_bidi_st * bidi = (gt_bidi_st * )gt_mem_malloc(bidi_max * sizeof(gt_bidi_st));
    if (NULL == bidi) {
        GT_LOGE(GT_LOG_TAG_GUI, "bidi malloc is failed!!! size = %lu", bidi_max * sizeof(gt_bidi_st));
        goto bidi_handler_lb;
    }
#endif
    // align vertical
    area_font.y = _get_align_start_y_by_line(dsc, coords, &tmp_font, txt, 1);
    if (dsc->reg.enabled_start) {
        area_font.y = dsc->start_y;
    }
    const int32_t coords_y_bottom_sl = (int32_t)coords->y + coords->h;
    const gt_size_t view_x_right_sl    = view->x + view->w;
    // out of range
    if ( (area_font.y + style_space_y + eff_font_size_sl) > coords_y_bottom_sl ) {
        font_disp_h = coords_y_bottom_sl - area_font.y;
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
            view_width = coords_w;
        }
    }
    tmp_font.info.size = font_size;
    while (idx < txt_len) {
        if (0x0A == txt[idx]) {
            ++idx;
            ++idx_len;
            if (dsc->reg.omit_line && view_width >= 0) {
                ret.size_show_limit.x = string_total_width;
                is_omit_display = false;
            }
            continue;
        }
        tmp_font.utf8 = (char * )&txt[idx];
        tmp_font.len = gt_font_one_char_code_len_get(&txt[idx], &uni_or_gbk, tmp_font.info.encoding);
        width = gt_symbol_is_valid_range(uni_or_gbk) ? tmp_font.info.size
                                                      : gt_font_get_one_word_width(uni_or_gbk, &tmp_font);
        if (0 == tmp_font.len) {
            ++idx;
            continue;
        }

        if (dsc->reg.omit_line) {
            view_width -= width + space_x;
            if (idx + tmp_font.len >= txt_len) {
                /** all display string */
                if (view_width >= 0) {
                    ret.size_show_limit.x = string_total_width + width + space_x;
                    is_omit_display = false;
                }
                else{
                    is_omit_display = true;
                }
            }
            else if ( view_width >= omit_info.width) {
                remark_idx = idx + tmp_font.len;
                /** Over display string widget's width */
                ret.size_show_limit.x = string_total_width + width + space_x + omit_info.width;
                is_omit_display = true;
            } else if (view_width <= 0) {
                if (tmp_font.len) {
                    string_total_width += width + space_x;
                }
                break;
            }
        }
        if (tmp_font.len) {
            string_total_width += width + space_x;
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
    if (dsc->reg.omit_line && is_omit_display) {
        if (view_width <= 0) { idx_len = remark_idx; }
        if (0 == ret.size_show_limit.x) { ret.size_show_limit.x = string_total_width; }
        ret.size.x = ret.size_show_limit.x;
    } else {
        ret.size.x = string_total_width;
    }
    if (dsc->reg.only_calc_display_info) { goto ret_handler_lb; }
    /** prepare display area */
    if (string_total_width > view->w) {
        area_font.x = coords->x;
    } else {
        /** Area can display all content, no need for omit width */
        area_font.x = _get_align_start_x(dsc->align, view->x, view->w, string_total_width);
    }
    if (align_reverse) {
        area_font.x = (coords->x + coords_w) - (area_font.x - coords->x);
    }
    if (start_x_flag) {
        area_font.x = dsc->start_x;
        start_x_flag = false;
    }

    idx = 0;
    area_font.h = font_disp_h;
    tmp_font.info.size = font_size;
    if (dsc->opa >= GT_OPA_MIN)
    while (idx < idx_len) {
#if _GT_FONT_ENABLE_CONVERTOR
        if (bidi_pos < bidi_len && idx == bidi[bidi_pos].idx) {
            tmp_font.utf8 = (char * )&txt[idx];
            tmp_font.len = bidi[bidi_pos].len;
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
            _gt_draw_font_res_st tmp_ret =
#endif
            _draw_text_convertor_split(&tmp_font, bidi[bidi_pos].flag, draw_ctx,
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

        tmp_font.utf8 = (char * )&txt[idx];
        tmp_font.len = gt_font_one_char_code_len_get(&txt[idx], &uni_or_gbk, tmp_font.info.encoding);
        if (0x0A == uni_or_gbk) {
            ++idx;
            continue;
        }
        const bool is_symbol_sl = gt_symbol_is_valid_range(uni_or_gbk);
        width = is_symbol_sl ? tmp_font.info.size
                             : gt_font_get_one_word_width(uni_or_gbk, &tmp_font);

        if (align_reverse) {
            area_font.x -= width;
        }
        if (area_font.x + width < view->x || area_font.x > view_x_right_sl) {
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
        if( !is_symbol_sl ){
            dot_ret = gt_font_get_dot(&tmp_font, uni_or_gbk);
            if (dot_ret.len < prev_rendered_size) {
                gt_memset(&tmp_font.res[dot_ret.len], 0, prev_rendered_size - dot_ret.len);
            }
            if (dot_ret.len > 0) { prev_rendered_size = dot_ret.len; }
            _draw_one_char_glyph(draw_ctx, &blend_dsc, mask_line, &tmp_font,
                                 dot_ret, &area_font, offset_y,
                                 &width, uni_or_gbk, dsc, false);
        } else {
            if (!_draw_one_symbol(draw_ctx, &blend_dsc, &tmp_font, &area_font,
                                  uni_or_gbk, data_len, font_disp_h)) {
                area_font.x += width + space_x;
                idx += tmp_font.len;
                continue;
            }
        }

        /* use style */
        _draw_char_style(draw_ctx, dsc, &blend_dsc_style, area_font,
                         (const gt_font_info_st*)&tmp_font.info, width, style_space_y, font_disp_h,
                         sl_has_ul, sl_has_st);
next_word_lb:
        _advance_x(&area_font.x, width, space_x, align_reverse);
        idx += tmp_font.len;
    }

    if (dsc->reg.omit_line && is_omit_display) {
        _draw_omit_chars(draw_ctx, &blend_dsc, &tmp_font, &area_font,
                         &omit_info, space_x, align_reverse);
    }

    area_font.w = tmp_font.info.size;
    area_font.h = tmp_font.info.size;

    if(tmp_font.info.style.reg.italic){
        area_font.x += eff_font_size_sl >> 1;
    }

ret_handler_lb:
#if _GT_FONT_ENABLE_CONVERTOR
    if (NULL != bidi) { gt_mem_free(bidi); bidi = NULL; }
bidi_handler_lb:
#endif
txt_handler_lb:
    #undef tmp_font
    _draw_text_ctx_free(&_ctx);
mask_handler_lb:
direct_handler_lb:
    ret.abs_pos.x = area_font.x;
    ret.abs_pos.y = area_font.y;
    return ret;
}

static GT_ATTRIBUTE_RAM_TEXT void _draw_dot_matrix_data(_gt_draw_ctx_st * draw_ctx, const gt_attr_rect_st * dsc,
    const gt_area_st * coords, gt_draw_blend_dsc_st * blend_dsc_p) {
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

static inline bool _is_brush_round(gt_line_st const * const line) {
    return GT_BRUSH_TYPE_ROUND == line->brush;
}

static void draw_dot_matrix(_gt_draw_ctx_st * draw_ctx, const gt_attr_rect_st * dsc, const gt_area_st * coords) {
    if(0 == dsc->gray) return;
    if (!dsc->data_gray)  return;

    gt_draw_blend_dsc_st blend_dsc = {0};
    blend_dsc.color_fill = dsc->bg_color;
    blend_dsc.view_area = dsc->limit_area;
    blend_dsc.opa = dsc->bg_opa;
    blend_dsc.mask_buf = NULL;

    _draw_dot_matrix_data(draw_ctx, dsc, coords, &blend_dsc);
}

static void _GT_DRAW_ATTRIBUTE_SRAM_TEXT
_draw_bg_fill_solid(_gt_draw_ctx_st * draw_ctx, const gt_attr_rect_st * dsc, const gt_area_st * coords, uint16_t border_radius) {
    gt_draw_blend_dsc_st blend_dsc = {0};
    blend_dsc.color_fill = dsc->bg_color;
    blend_dsc.view_area = dsc->limit_area;
    blend_dsc.opa = dsc->bg_opa;
    blend_dsc.mask_buf = NULL;

    gt_area_st bg_area = {0};
    bg_area = gt_area_reduce(*coords, dsc->border_width);
    uint16_t bg_radius = (border_radius < dsc->border_width) ? 0 : border_radius - dsc->border_width;
    bg_radius = GT_MIN(bg_radius, (GT_MIN(bg_area.w >> 1, bg_area.h >> 1)));

    bool mask_check = gt_mask_check(&bg_area);

    if(0 == bg_radius && !mask_check){
        blend_dsc.dst_area = &bg_area;
        gt_draw_blend(draw_ctx, &blend_dsc);
        return ;
    }

    gt_opa_t* mask_line_buf = gt_mem_malloc(bg_area.w);
    if (NULL == mask_line_buf) {
        GT_LOGE(GT_LOG_TAG_GUI, "mask_line_buf is NULL!");
        goto free_lb;
    }
    int16_t bg_mask_id = -1;
    gt_mask_radius_st bg_mask_radius = {0};

    if(bg_radius > 0){
        gt_mask_radius_cfg_st bg_cfg = { .outer = 0, .corner_mask = dsc->corner_mask };
        gt_mask_radius_init(&bg_mask_radius, &bg_area, bg_radius, bg_cfg);
        bg_mask_id = gt_mask_add(&bg_mask_radius);
    }

    int16_t cl_mask_id = -1;
    uint16_t cl_radius = 0;
    gt_mask_radius_st cl_mask_radius = {0};
    if (dsc->base_area){
        uint16_t r = dsc->clip_radius > 0 ? dsc->clip_radius : dsc->radius;
        cl_radius = GT_MIN(r, (GT_MIN(dsc->base_area->w >> 1, dsc->base_area->h >> 1)));
        /* Also clamp to the actual drawing area's half short-side so the clip mask
         * never exceeds the bar rectangle being drawn. This fixes over-clipping when
         * the bar is shorter than the track radius in either direction
         * (horizontal: small val_w; vertical: small val_h).
         * Do NOT clamp cl_radius to bg_area dimensions here: the clip mask
         * is applied to base_area (the full track), not bg_area (the val bar).
         * Clamping to bg_area would shrink the clip radius when val is narrow,
         * making the blue bar appear with a straight edge instead of following
         * the track's rounded corner shape. */
        if(cl_radius > 0){
            gt_mask_radius_init(&cl_mask_radius, dsc->base_area, cl_radius, GT_MASK_RADIUS_CFG_INNER);
            cl_mask_id = gt_mask_add(&cl_mask_radius);
        }
    }

    gt_opa_t mask_opa = dsc->bg_opa >= GT_OPA_MAX ? GT_OPA_COVER : dsc->bg_opa;
    gt_area_st line_area = {0};
    line_area.x = bg_area.x;
    line_area.w = bg_area.w;
    line_area.h = 1;
    blend_dsc.dst_area = &line_area;
    blend_dsc.mask_area = &line_area;

    /* 无额外 mask 时，圆角矩形中间段可合并为一次矩形 blend */
    if(!mask_check && bg_radius > 0 && bg_area.h > (int16_t)(bg_radius << 1)){
        gt_size_t mid_y_start = bg_area.y + bg_radius;
        gt_size_t mid_y_end   = bg_area.y + bg_area.h - bg_radius;
        uint16_t i = 0;
        /* 顶部圆角区域 */
        for (i = 0; i < bg_radius; ++i) {
            line_area.y = bg_area.y + i;
            gt_memset(mask_line_buf, mask_opa, line_area.w);
            gt_mask_res_et res = gt_mask_get(mask_line_buf, line_area.x, line_area.y, line_area.w);
            if(res == GT_MASK_RES_TRANSP) continue;
            blend_dsc.mask_buf = (res == GT_MASK_RES_FULL_COVER) ? NULL : mask_line_buf;
            gt_draw_blend(draw_ctx, &blend_dsc);
        }
        /* 中间矩形段 */
        gt_area_st mid_area = bg_area;
        mid_area.y = mid_y_start;
        mid_area.h = mid_y_end - mid_y_start;
        blend_dsc.dst_area  = &mid_area;
        blend_dsc.mask_area = NULL;
        if (mask_opa >= GT_OPA_MAX) {
            /* 完全不透明：无需 mask，一次 blend 覆盖整个中间段 */
            blend_dsc.mask_buf = NULL;
            gt_draw_blend(draw_ctx, &blend_dsc);
        } else {
            /* 半透明：逐行处理，保持与圆角区域一致的透明度行为 */
            blend_dsc.dst_area  = &line_area;
            blend_dsc.mask_area = &line_area;
            for(i = bg_radius; i < bg_area.h - bg_radius; ++i){
                line_area.y = bg_area.y + i;
                gt_memset(mask_line_buf, mask_opa, line_area.w);
                blend_dsc.mask_buf = mask_line_buf;
                gt_draw_blend(draw_ctx, &blend_dsc);
            }
        }
        /* 底部圆角区域 */
        blend_dsc.dst_area  = &line_area;
        blend_dsc.mask_area = &line_area;
        for (i = bg_area.h - bg_radius; i < bg_area.h; ++i) {
            line_area.y = bg_area.y + i;
            gt_memset(mask_line_buf, mask_opa, line_area.w);
            gt_mask_res_et res = gt_mask_get(mask_line_buf, line_area.x, line_area.y, line_area.w);
            if(res == GT_MASK_RES_TRANSP) continue;
            blend_dsc.mask_buf = (res == GT_MASK_RES_FULL_COVER) ? NULL : mask_line_buf;
            gt_draw_blend(draw_ctx, &blend_dsc);
        }
    } else {
        uint16_t i = 0;
        for (i = 0; i < bg_area.h; ++i) {
            line_area.y = bg_area.y + i;
            gt_memset(mask_line_buf, mask_opa, line_area.w);
            gt_mask_res_et res = gt_mask_get(mask_line_buf, line_area.x, line_area.y, line_area.w);
            if(res == GT_MASK_RES_TRANSP) continue;
            blend_dsc.mask_buf = (res == GT_MASK_RES_FULL_COVER) ? NULL : mask_line_buf;
            gt_draw_blend(draw_ctx, &blend_dsc);
        }
    }

    gt_mem_free(mask_line_buf);
    mask_line_buf = NULL;
free_lb:
    blend_dsc.mask_buf = NULL;

    if(bg_radius > 0){
        gt_mask_remove_idx(bg_mask_id);
        gt_mask_free(&bg_mask_radius);
    }

    if (dsc->base_area && cl_radius > 0){
        gt_mask_remove_idx(cl_mask_id);
        gt_mask_free(&cl_mask_radius);
    }
}

#if GT_USE_GRADIENT
static void _GT_DRAW_ATTRIBUTE_SRAM_TEXT
_draw_bg_fill_gradient(_gt_draw_ctx_st * draw_ctx, const gt_attr_rect_st * dsc, const gt_area_st * coords, uint16_t border_radius) {
    if (!(dsc->grad && dsc->grad->stops && _gt_vector_get_count(dsc->grad->stops) > 0)) return;

    gt_area_st bg_area = gt_area_reduce(*coords, dsc->border_width);
        uint16_t bg_radius = (border_radius < dsc->border_width) ? 0 : border_radius - dsc->border_width;
        bg_radius = GT_MIN(bg_radius, (GT_MIN(bg_area.w >> 1, bg_area.h >> 1)));

        uint16_t total = gt_gradient_dsc_total_steps(dsc->grad);
        if (total < 2) total = 2;

        gt_draw_blend_dsc_st blend_dsc = {0};
        blend_dsc.view_area = dsc->limit_area;
        blend_dsc.opa       = dsc->bg_opa;

        int16_t bg_mask_id = -1;
        gt_mask_radius_st bg_mask_radius = {0};
        if (bg_radius > 0) {
            gt_mask_radius_cfg_st bg_cfg = { .outer = 0, .corner_mask = dsc->corner_mask };
            gt_mask_radius_init(&bg_mask_radius, &bg_area, bg_radius, bg_cfg);
            bg_mask_id = gt_mask_add(&bg_mask_radius);
        }

        int16_t cl_mask_id = -1;
        uint16_t cl_radius = 0;
        gt_mask_radius_st cl_mask_radius = {0};
        if (dsc->base_area) {
            uint16_t r = dsc->clip_radius > 0 ? dsc->clip_radius : dsc->radius;
            cl_radius = GT_MIN(r, (GT_MIN(dsc->base_area->w >> 1, dsc->base_area->h >> 1)));
            if (cl_radius > 0) {
                gt_mask_radius_init(&cl_mask_radius, dsc->base_area, cl_radius, GT_MASK_RADIUS_CFG_INNER);
                cl_mask_id = gt_mask_add(&cl_mask_radius);
            }
        }

        bool need_mask = (bg_radius > 0) || gt_mask_check(&bg_area);
        gt_opa_t * mask_buf = need_mask
            ? (gt_opa_t *)gt_mem_malloc(GT_MAX(bg_area.w, bg_area.h))
            : NULL;
        gt_opa_t mask_opa = dsc->bg_opa >= GT_OPA_MAX ? GT_OPA_COVER : dsc->bg_opa;

        /* Row color buffer for Floyd-Steinberg dithering */
        gt_color_t * color_row_buf = (gt_color_t *)gt_mem_malloc(bg_area.w * sizeof(gt_color_t));

        /* Floyd-Steinberg error buffers: two rows × width × 3 channels (R,G,B), int16_t.
         * err_cur[col*3+ch] = accumulated error for current row pixel (col, ch).
         * err_nxt[col*3+ch] = accumulated error for next row pixel.
         * Errors are in 8-bit space (×16 fixed-point to keep fractions). */
        int16_t * fs_err_cur = NULL;
        int16_t * fs_err_nxt = NULL;
        if (color_row_buf) {
            fs_err_cur = (int16_t *)gt_mem_malloc(bg_area.w * 3 * sizeof(int16_t));
            fs_err_nxt = (int16_t *)gt_mem_malloc(bg_area.w * 3 * sizeof(int16_t));
            if (fs_err_cur) gt_memset(fs_err_cur, 0, bg_area.w * 3 * sizeof(int16_t));
            if (fs_err_nxt) gt_memset(fs_err_nxt, 0, bg_area.w * 3 * sizeof(int16_t));
        }

        /* Helper macro: clamp int32 to [0..255] */
        #define _FS_CLAMP8(v)  ((v) < 0 ? 0 : ((v) > 255 ? 255 : (v)))

        /* Quantize one 8-bit channel to n bits and return quantized 8-bit value.
         * n=5 → multiply by 255/31; n=6 → multiply by 255/63. */
        #define _FS_QUANT5(v8) (((v8) >> 3) * (255 / 31))
        #define _FS_QUANT6(v8) (((v8) >> 2) * (255 / 63))

        /* Spread error e (×16 fixed-point) to neighbour slot */
        #define _FS_ADD(arr, idx, e) do { if ((idx) >= 0 && (idx) < (int32_t)(bg_area.w * 3)) (arr)[(idx)] += (e); } while(0)

        /* Fill color_row_buf with Floyd-Steinberg dithered colors for one row.
         * ideal_r/g/b are arrays of bg_area.w uint8_t values (ideal 8-bit colors).
         * fs_err_cur carries incoming errors; fs_err_nxt accumulates outgoing errors. */
        #define _FS_DITHER_ROW(ideal_r_fn, ideal_g_fn, ideal_b_fn)                         \
        do {                                                                                 \
            for (uint16_t _c = 0; _c < (uint16_t)bg_area.w; ++_c) {                        \
                int32_t _r8 = _FS_CLAMP8((ideal_r_fn) + (fs_err_cur ? (fs_err_cur[_c*3+0] >> 4) : 0)); \
                int32_t _g8 = _FS_CLAMP8((ideal_g_fn) + (fs_err_cur ? (fs_err_cur[_c*3+1] >> 4) : 0)); \
                int32_t _b8 = _FS_CLAMP8((ideal_b_fn) + (fs_err_cur ? (fs_err_cur[_c*3+2] >> 4) : 0)); \
                int32_t _qr = _FS_QUANT5(_r8);                                              \
                int32_t _qg = _FS_QUANT6(_g8);                                              \
                int32_t _qb = _FS_QUANT5(_b8);                                              \
                GT_COLOR_SET_R(color_row_buf[_c], _r8 >> (GT_COLOR_DEPTH == 32 ? 0 : (GT_COLOR_DEPTH == 16 ? 3 : (GT_COLOR_DEPTH == 8 ? 5 : 7)))); \
                GT_COLOR_SET_G(color_row_buf[_c], _g8 >> (GT_COLOR_DEPTH == 32 ? 0 : (GT_COLOR_DEPTH == 16 ? 2 : (GT_COLOR_DEPTH == 8 ? 5 : 7)))); \
                GT_COLOR_SET_B(color_row_buf[_c], _b8 >> (GT_COLOR_DEPTH == 32 ? 0 : (GT_COLOR_DEPTH == 16 ? 3 : (GT_COLOR_DEPTH == 8 ? 6 : 7)))); \
                int32_t _er = (_r8 - _qr) << 4;                                             \
                int32_t _eg = (_g8 - _qg) << 4;                                             \
                int32_t _eb = (_b8 - _qb) << 4;                                             \
                if (fs_err_cur) {                                                            \
                    _FS_ADD(fs_err_cur, (int32_t)(_c+1)*3+0, (_er*7)>>4);                   \
                    _FS_ADD(fs_err_cur, (int32_t)(_c+1)*3+1, (_eg*7)>>4);                   \
                    _FS_ADD(fs_err_cur, (int32_t)(_c+1)*3+2, (_eb*7)>>4);                   \
                }                                                                            \
                if (fs_err_nxt) {                                                            \
                    _FS_ADD(fs_err_nxt, (int32_t)(_c-1)*3+0, (_er*3)>>4);                   \
                    _FS_ADD(fs_err_nxt, (int32_t)(_c-1)*3+1, (_eg*3)>>4);                   \
                    _FS_ADD(fs_err_nxt, (int32_t)(_c-1)*3+2, (_eb*3)>>4);                   \
                    _FS_ADD(fs_err_nxt, (int32_t)(_c  )*3+0, (_er*5)>>4);                   \
                    _FS_ADD(fs_err_nxt, (int32_t)(_c  )*3+1, (_eg*5)>>4);                   \
                    _FS_ADD(fs_err_nxt, (int32_t)(_c  )*3+2, (_eb*5)>>4);                   \
                    _FS_ADD(fs_err_nxt, (int32_t)(_c+1)*3+0, (_er*1)>>4);                   \
                    _FS_ADD(fs_err_nxt, (int32_t)(_c+1)*3+1, (_eg*1)>>4);                   \
                    _FS_ADD(fs_err_nxt, (int32_t)(_c+1)*3+2, (_eb*1)>>4);                   \
                }                                                                            \
            }                                                                                \
        } while(0)

        /* Swap error rows and clear next row */
        #define _FS_SWAP_ROWS() do {                                                         \
            int16_t * _tmp = fs_err_cur; fs_err_cur = fs_err_nxt; fs_err_nxt = _tmp;        \
            if (fs_err_nxt) gt_memset(fs_err_nxt, 0, bg_area.w * 3 * sizeof(int16_t));     \
        } while(0)

        /* Macro: dither one column pixel (per-column ideal color variant).
         * Reads/writes fs_err_cur[_c*3+ch] and fs_err_nxt[_c*3+ch].
         * Writes color_row_buf[_c]. Must be used inside a loop with uint16_t _c. */
        #define _FS_DITHER_COL_PIXEL(ideal_c24) do { \
            int32_t _r8 = _FS_CLAMP8((int32_t)(((ideal_c24) >> 16) & 0xFF) + (fs_err_cur[_c*3+0] >> 4)); \
            int32_t _g8 = _FS_CLAMP8((int32_t)(((ideal_c24) >>  8) & 0xFF) + (fs_err_cur[_c*3+1] >> 4)); \
            int32_t _b8 = _FS_CLAMP8((int32_t)( (ideal_c24)        & 0xFF) + (fs_err_cur[_c*3+2] >> 4)); \
            int32_t _qr = _FS_QUANT5(_r8); \
            int32_t _qg = _FS_QUANT6(_g8); \
            int32_t _qb = _FS_QUANT5(_b8); \
            GT_COLOR_SET_R(color_row_buf[_c], _r8 >> (GT_COLOR_DEPTH == 32 ? 0 : (GT_COLOR_DEPTH == 16 ? 3 : (GT_COLOR_DEPTH == 8 ? 5 : 7)))); \
            GT_COLOR_SET_G(color_row_buf[_c], _g8 >> (GT_COLOR_DEPTH == 32 ? 0 : (GT_COLOR_DEPTH == 16 ? 2 : (GT_COLOR_DEPTH == 8 ? 5 : 7)))); \
            GT_COLOR_SET_B(color_row_buf[_c], _b8 >> (GT_COLOR_DEPTH == 32 ? 0 : (GT_COLOR_DEPTH == 16 ? 3 : (GT_COLOR_DEPTH == 8 ? 6 : 7)))); \
            int32_t _er = (_r8 - _qr) << 4; \
            int32_t _eg = (_g8 - _qg) << 4; \
            int32_t _eb = (_b8 - _qb) << 4; \
            _FS_ADD(fs_err_cur, (int32_t)(_c+1)*3+0, (_er*7)>>4); \
            _FS_ADD(fs_err_cur, (int32_t)(_c+1)*3+1, (_eg*7)>>4); \
            _FS_ADD(fs_err_cur, (int32_t)(_c+1)*3+2, (_eb*7)>>4); \
            _FS_ADD(fs_err_nxt, (int32_t)(_c-1)*3+0, (_er*3)>>4); \
            _FS_ADD(fs_err_nxt, (int32_t)(_c-1)*3+1, (_eg*3)>>4); \
            _FS_ADD(fs_err_nxt, (int32_t)(_c-1)*3+2, (_eb*3)>>4); \
            _FS_ADD(fs_err_nxt, (int32_t)(_c  )*3+0, (_er*5)>>4); \
            _FS_ADD(fs_err_nxt, (int32_t)(_c  )*3+1, (_eg*5)>>4); \
            _FS_ADD(fs_err_nxt, (int32_t)(_c  )*3+2, (_eb*5)>>4); \
            _FS_ADD(fs_err_nxt, (int32_t)(_c+1)*3+0, (_er*1)>>4); \
            _FS_ADD(fs_err_nxt, (int32_t)(_c+1)*3+1, (_eg*1)>>4); \
            _FS_ADD(fs_err_nxt, (int32_t)(_c+1)*3+2, (_eb*1)>>4); \
        } while(0)

        /* Inner variant: no boundary checks, for columns 1..w-2 only. */
        #define _FS_DITHER_COL_PIXEL_INNER(ideal_c24) do { \
            int32_t _r8 = _FS_CLAMP8((int32_t)(((ideal_c24) >> 16) & 0xFF) + (fs_err_cur[_c*3+0] >> 4)); \
            int32_t _g8 = _FS_CLAMP8((int32_t)(((ideal_c24) >>  8) & 0xFF) + (fs_err_cur[_c*3+1] >> 4)); \
            int32_t _b8 = _FS_CLAMP8((int32_t)( (ideal_c24)        & 0xFF) + (fs_err_cur[_c*3+2] >> 4)); \
            int32_t _qr = _FS_QUANT5(_r8); \
            int32_t _qg = _FS_QUANT6(_g8); \
            int32_t _qb = _FS_QUANT5(_b8); \
            GT_COLOR_SET_R(color_row_buf[_c], _r8 >> (GT_COLOR_DEPTH == 32 ? 0 : (GT_COLOR_DEPTH == 16 ? 3 : (GT_COLOR_DEPTH == 8 ? 5 : 7)))); \
            GT_COLOR_SET_G(color_row_buf[_c], _g8 >> (GT_COLOR_DEPTH == 32 ? 0 : (GT_COLOR_DEPTH == 16 ? 2 : (GT_COLOR_DEPTH == 8 ? 5 : 7)))); \
            GT_COLOR_SET_B(color_row_buf[_c], _b8 >> (GT_COLOR_DEPTH == 32 ? 0 : (GT_COLOR_DEPTH == 16 ? 3 : (GT_COLOR_DEPTH == 8 ? 6 : 7)))); \
            int32_t _er = (_r8 - _qr) << 4; \
            int32_t _eg = (_g8 - _qg) << 4; \
            int32_t _eb = (_b8 - _qb) << 4; \
            fs_err_cur[(_c+1)*3+0] += (_er*7)>>4; \
            fs_err_cur[(_c+1)*3+1] += (_eg*7)>>4; \
            fs_err_cur[(_c+1)*3+2] += (_eb*7)>>4; \
            fs_err_nxt[(_c-1)*3+0] += (_er*3)>>4; \
            fs_err_nxt[(_c-1)*3+1] += (_eg*3)>>4; \
            fs_err_nxt[(_c-1)*3+2] += (_eb*3)>>4; \
            fs_err_nxt[(_c  )*3+0] += (_er*5)>>4; \
            fs_err_nxt[(_c  )*3+1] += (_eg*5)>>4; \
            fs_err_nxt[(_c  )*3+2] += (_eb*5)>>4; \
            fs_err_nxt[(_c+1)*3+0] += (_er*1)>>4; \
            fs_err_nxt[(_c+1)*3+1] += (_eg*1)>>4; \
            fs_err_nxt[(_c+1)*3+2] += (_eb*1)>>4; \
        } while(0)

        /* Pre-compute center and half-size for SYM_VER / SYM_HOR */
        uint16_t cx = bg_area.x + (bg_area.w >> 1);
        uint16_t cy = bg_area.y + (bg_area.h >> 1);
        uint32_t max_r = bg_area.h >> 1;   /* SYM_VER: half-height */
        uint32_t max_rx = bg_area.w >> 1;  /* SYM_HOR: half-width */

        gt_grad_dir_et dir = dsc->grad->dir;

        if (dir == GT_GRAD_DIR_VER) {
            gt_area_st line_area = { .x = bg_area.x, .w = bg_area.w, .h = 1 };
            blend_dsc.dst_area  = &line_area;
            blend_dsc.mask_area = &line_area;
            uint16_t h1 = bg_area.h > 1 ? bg_area.h - 1 : 1;
            for (uint16_t row = 0; row < (uint16_t)bg_area.h; ++row) {
                line_area.y = bg_area.y + row;
                uint16_t base_pos = (uint16_t)((uint32_t)row * 65535u / h1);
                if (color_row_buf && fs_err_cur && fs_err_nxt) {
                    uint32_t _c24 = gt_gradient_dsc_get_color24_pos(dsc->grad, base_pos);
                    uint8_t _ir = (uint8_t)((_c24 >> 16) & 0xFF);
                    uint8_t _ig = (uint8_t)((_c24 >>  8) & 0xFF);
                    uint8_t _ib = (uint8_t)( _c24        & 0xFF);
                    _FS_DITHER_ROW(_ir, _ig, _ib);
                    _FS_SWAP_ROWS();
                    blend_dsc.dst_buf    = color_row_buf;
                    blend_dsc.color_fill = gt_color_black();
                } else {
                    blend_dsc.dst_buf    = NULL;
                    blend_dsc.color_fill = gt_gradient_dsc_get_color_pos(dsc->grad, base_pos);
                }
                if (need_mask && mask_buf) {
                    gt_memset(mask_buf, mask_opa, line_area.w);
                    gt_mask_res_et res = gt_mask_get(mask_buf, line_area.x, line_area.y, line_area.w);
                    if (res == GT_MASK_RES_TRANSP) { blend_dsc.dst_buf = NULL; continue; }
                    blend_dsc.mask_buf = (res == GT_MASK_RES_FULL_COVER) ? NULL : mask_buf;
                } else {
                    blend_dsc.mask_buf = NULL;
                }
                gt_draw_blend(draw_ctx, &blend_dsc);
                blend_dsc.dst_buf = NULL;
            }
        } else if (dir == GT_GRAD_DIR_HOR) {
            gt_area_st line_area = { .x = bg_area.x, .w = bg_area.w, .h = 1 };
            blend_dsc.dst_area  = &line_area;
            blend_dsc.mask_area = &line_area;
            uint16_t w1 = bg_area.w > 1 ? bg_area.w - 1 : 1;
            for (uint16_t row = 0; row < (uint16_t)bg_area.h; ++row) {
                line_area.y = bg_area.y + row;
                if (color_row_buf && fs_err_cur && fs_err_nxt) {
                    /* HOR: each column has its own ideal color.
                     * Split into first/inner/last to avoid per-pixel boundary checks. */
                    if (bg_area.w > 0) {
                        { uint16_t _c = 0;
                          uint32_t _ideal_c24 = gt_gradient_dsc_get_color24_pos(dsc->grad, 0);
                          _FS_DITHER_COL_PIXEL(_ideal_c24); }
                        for (uint16_t _c = 1; _c < (uint16_t)bg_area.w - 1; ++_c) {
                            uint16_t _pos = (uint16_t)((uint32_t)_c * 65535u / w1);
                            uint32_t _ideal_c24 = gt_gradient_dsc_get_color24_pos(dsc->grad, _pos);
                            _FS_DITHER_COL_PIXEL_INNER(_ideal_c24);
                        }
                        if (bg_area.w > 1) {
                            uint16_t _c = (uint16_t)bg_area.w - 1;
                            uint32_t _ideal_c24 = gt_gradient_dsc_get_color24_pos(dsc->grad, 65535u);
                            _FS_DITHER_COL_PIXEL(_ideal_c24);
                        }
                    }
                    _FS_SWAP_ROWS();
                    blend_dsc.dst_buf    = color_row_buf;
                    blend_dsc.color_fill = gt_color_black();
                } else {
                    blend_dsc.dst_buf    = NULL;
                    uint16_t mid_pos = (uint16_t)((uint32_t)(bg_area.w >> 1) * 65535u / w1);
                    blend_dsc.color_fill = gt_gradient_dsc_get_color_pos(dsc->grad, mid_pos);
                }
                if (need_mask && mask_buf) {
                    gt_memset(mask_buf, mask_opa, line_area.w);
                    gt_mask_res_et res = gt_mask_get(mask_buf, line_area.x, line_area.y, line_area.w);
                    if (res == GT_MASK_RES_TRANSP) { blend_dsc.dst_buf = NULL; continue; }
                    blend_dsc.mask_buf = (res == GT_MASK_RES_FULL_COVER) ? NULL : mask_buf;
                } else {
                    blend_dsc.mask_buf = NULL;
                }
                gt_draw_blend(draw_ctx, &blend_dsc);
                blend_dsc.dst_buf = NULL;
            }
        } else if (dir == GT_GRAD_DIR_SYM_HOR) {
            gt_area_st line_area = { .x = bg_area.x, .w = bg_area.w, .h = 1 };
            blend_dsc.dst_area  = &line_area;
            blend_dsc.mask_area = &line_area;
            for (uint16_t row = 0; row < (uint16_t)bg_area.h; ++row) {
                line_area.y = bg_area.y + row;
                if (color_row_buf && fs_err_cur && fs_err_nxt) {
                    /* SYM_HOR: split into first/inner/last to avoid per-pixel boundary checks. */
                    if (bg_area.w > 0) {
                        { uint16_t _c = 0;
                          int32_t dx = (int32_t)(bg_area.x) - (int32_t)cx;
                          uint32_t r2 = (uint32_t)(dx < 0 ? -dx : dx);
                          uint16_t _pos = (max_rx > 0) ? (uint16_t)((uint32_t)r2 * 65535u / max_rx) : 0;
                          uint32_t _ideal_c24 = gt_gradient_dsc_get_color24_pos(dsc->grad, _pos);
                          _FS_DITHER_COL_PIXEL(_ideal_c24); }
                        for (uint16_t _c = 1; _c < (uint16_t)bg_area.w - 1; ++_c) {
                            int32_t dx = (int32_t)(bg_area.x + _c) - (int32_t)cx;
                            uint32_t r2 = (uint32_t)(dx < 0 ? -dx : dx);
                            uint16_t _pos = (max_rx > 0) ? (uint16_t)((uint32_t)r2 * 65535u / max_rx) : 0;
                            uint32_t _ideal_c24 = gt_gradient_dsc_get_color24_pos(dsc->grad, _pos);
                            _FS_DITHER_COL_PIXEL_INNER(_ideal_c24);
                        }
                        if (bg_area.w > 1) {
                            uint16_t _c = (uint16_t)bg_area.w - 1;
                            int32_t dx = (int32_t)(bg_area.x + _c) - (int32_t)cx;
                            uint32_t r2 = (uint32_t)(dx < 0 ? -dx : dx);
                            uint16_t _pos = (max_rx > 0) ? (uint16_t)((uint32_t)r2 * 65535u / max_rx) : 0;
                            uint32_t _ideal_c24 = gt_gradient_dsc_get_color24_pos(dsc->grad, _pos);
                            _FS_DITHER_COL_PIXEL(_ideal_c24);
                        }
                    }
                    _FS_SWAP_ROWS();
                    blend_dsc.dst_buf    = color_row_buf;
                    blend_dsc.color_fill = gt_color_black();
                } else {
                    blend_dsc.dst_buf    = NULL;
                    blend_dsc.color_fill = gt_gradient_dsc_get_color_pos(dsc->grad, 0);
                }
                if (need_mask && mask_buf) {
                    gt_memset(mask_buf, mask_opa, line_area.w);
                    gt_mask_res_et res = gt_mask_get(mask_buf, line_area.x, line_area.y, line_area.w);
                    if (res == GT_MASK_RES_TRANSP) { blend_dsc.dst_buf = NULL; continue; }
                    blend_dsc.mask_buf = (res == GT_MASK_RES_FULL_COVER) ? NULL : mask_buf;
                } else {
                    blend_dsc.mask_buf = NULL;
                }
                gt_draw_blend(draw_ctx, &blend_dsc);
                blend_dsc.dst_buf = NULL;
            }
        } else {
            /* DIAG_TL / DIAG_TR / SYM_VER */
            gt_area_st line_area = { .x = bg_area.x, .w = bg_area.w, .h = 1 };
            blend_dsc.dst_area  = &line_area;
            blend_dsc.mask_area = &line_area;
            uint16_t diag_denom = (bg_area.w + bg_area.h - 2) > 0 ? (bg_area.w + bg_area.h - 2) : 1;
            for (uint16_t row = 0; row < (uint16_t)bg_area.h; ++row) {
                line_area.y = bg_area.y + row;
                uint16_t base_pos = 0;
                if (dir == GT_GRAD_DIR_DIAG_TL) {
                    uint16_t col = bg_area.w >> 1;
                    base_pos = (uint16_t)(((uint32_t)row + col) * 65535u / diag_denom);
                } else if (dir == GT_GRAD_DIR_DIAG_TR) {
                    uint16_t inv_col = bg_area.w - 1 - (bg_area.w >> 1);
                    base_pos = (uint16_t)(((uint32_t)row + inv_col) * 65535u / diag_denom);
                } else { /* SYM_VER */
                    int32_t dy = (int32_t)(bg_area.y + row) - (int32_t)cy;
                    uint32_t r2 = (uint32_t)(dy < 0 ? -dy : dy);
                    base_pos = (max_r > 0) ? (uint16_t)((uint32_t)r2 * 65535u / max_r) : 0;
                }
                if (color_row_buf && fs_err_cur && fs_err_nxt && dir == GT_GRAD_DIR_SYM_VER) {
                    uint32_t _c24 = gt_gradient_dsc_get_color24_pos(dsc->grad, base_pos);
                    uint8_t _ir = (uint8_t)((_c24 >> 16) & 0xFF);
                    uint8_t _ig = (uint8_t)((_c24 >>  8) & 0xFF);
                    uint8_t _ib = (uint8_t)( _c24        & 0xFF);
                    _FS_DITHER_ROW(_ir, _ig, _ib);
                    _FS_SWAP_ROWS();
                    blend_dsc.dst_buf    = color_row_buf;
                    blend_dsc.color_fill = gt_color_black();
                } else {
                    blend_dsc.dst_buf    = NULL;
                    blend_dsc.color_fill = gt_gradient_dsc_get_color_pos(dsc->grad, base_pos);
                }
                if (need_mask && mask_buf) {
                    gt_memset(mask_buf, mask_opa, line_area.w);
                    gt_mask_res_et res = gt_mask_get(mask_buf, line_area.x, line_area.y, line_area.w);
                    if (res == GT_MASK_RES_TRANSP) { blend_dsc.dst_buf = NULL; continue; }
                    blend_dsc.mask_buf = (res == GT_MASK_RES_FULL_COVER) ? NULL : mask_buf;
                } else {
                    blend_dsc.mask_buf = NULL;
                }
                gt_draw_blend(draw_ctx, &blend_dsc);
                blend_dsc.dst_buf = NULL;
            }
        }
        #undef _FS_CLAMP8
        #undef _FS_QUANT5
        #undef _FS_QUANT6
        #undef _FS_ADD
        #undef _FS_DITHER_ROW
        #undef _FS_SWAP_ROWS
        #undef _FS_DITHER_COL_PIXEL
        #undef _FS_DITHER_COL_PIXEL_INNER

        if (mask_buf) gt_mem_free(mask_buf);
        if (color_row_buf) gt_mem_free(color_row_buf);
        if (fs_err_cur) gt_mem_free(fs_err_cur);
        if (fs_err_nxt) gt_mem_free(fs_err_nxt);
        if (bg_radius > 0) { gt_mask_remove_idx(bg_mask_id); gt_mask_free(&bg_mask_radius); }
        if (dsc->base_area && cl_radius > 0) { gt_mask_remove_idx(cl_mask_id); gt_mask_free(&cl_mask_radius); }
        return;
    }
#endif /* GT_USE_GRADIENT */

static void _GT_DRAW_ATTRIBUTE_SRAM_TEXT
draw_bg_fill(_gt_draw_ctx_st * draw_ctx, const gt_attr_rect_st * dsc, const gt_area_st * coords, uint16_t border_radius) {
    if (!dsc->reg.is_fill) return;
#if GT_USE_GRADIENT
    _draw_bg_fill_gradient(draw_ctx, dsc, coords, border_radius);
    if (dsc->grad && dsc->grad->stops && _gt_vector_get_count(dsc->grad->stops) > 0) return;
#endif
    _draw_bg_fill_solid(draw_ctx, dsc, coords, border_radius);
}

static void _GT_DRAW_ATTRIBUTE_SRAM_TEXT
draw_bg_border(_gt_draw_ctx_st * draw_ctx, const gt_attr_rect_st * dsc, const gt_area_st * coords, uint16_t border_radius) {
    if(dsc->border_width <= 0) return ;

    gt_draw_blend_dsc_st blend_dsc = {0};
    blend_dsc.view_area = dsc->limit_area;
    blend_dsc.opa = dsc->bg_opa;
    blend_dsc.mask_buf = NULL;
    blend_dsc.color_fill = dsc->border_color;

    gt_area_st border_area = {0};
    gt_area_copy(&border_area, coords);

    gt_area_st line_area = {0};

    if(border_radius <= 0){
        blend_dsc.dst_area = &line_area;
        // top
        line_area.x = border_area.x;
        line_area.y = border_area.y;
        line_area.w = border_area.w;
        line_area.h = dsc->border_width;
        gt_draw_blend(draw_ctx, &blend_dsc);
        // bottom
        line_area.y = border_area.y + border_area.h - dsc->border_width;
        gt_draw_blend(draw_ctx, &blend_dsc);
        // left
        line_area.x = border_area.x;
        line_area.y = border_area.y + dsc->border_width;
        line_area.w = dsc->border_width;
        line_area.h = border_area.h - dsc->border_width;
        gt_draw_blend(draw_ctx, &blend_dsc);
        // right
        line_area.x = border_area.x + border_area.w - dsc->border_width;
        gt_draw_blend(draw_ctx, &blend_dsc);
        return ;
    }

    gt_opa_t* mask_line_buf = gt_mem_malloc(border_area.w);
    if (NULL == mask_line_buf) {
        GT_LOGE(GT_LOG_TAG_GUI, "mask_line_buf is NULL!");
        goto free_lb;
    }
    gt_area_st in_area = {0};
    in_area = gt_area_reduce(border_area, dsc->border_width);
    uint16_t in_radius = (border_radius < dsc->border_width) ? 0 : border_radius - dsc->border_width;
    in_radius = GT_MIN(in_radius, (GT_MIN(in_area.w >> 1, in_area.h >> 1)));

    int16_t in_mask_id = -1;
    gt_mask_radius_st in_mask_radius = {0};
    gt_mask_radius_cfg_st in_cfg = { .outer = 1, .corner_mask = dsc->corner_mask };
    gt_mask_radius_init(&in_mask_radius, &in_area, in_radius, in_cfg);
    in_mask_id = gt_mask_add(&in_mask_radius);

    int16_t border_mask_id = -1;
    gt_mask_radius_st border_mask_radius = {0};
    gt_mask_radius_cfg_st border_cfg = { .outer = 0, .corner_mask = dsc->corner_mask };
    gt_mask_radius_init(&border_mask_radius, &border_area, border_radius, border_cfg);
    border_mask_id = gt_mask_add(&border_mask_radius);

    gt_opa_t mask_opa = dsc->bg_opa >= GT_OPA_MAX ? GT_OPA_COVER : dsc->bg_opa;

    line_area.x = border_area.x;
    line_area.w = border_area.w;
    line_area.h = 1;
    blend_dsc.dst_area = &line_area;
    blend_dsc.mask_area = &line_area;

    uint16_t i = 0;
    for(i = 0; i < border_area.h; ++i){
        line_area.y = border_area.y + i;
        gt_memset(mask_line_buf, mask_opa, line_area.w);
        gt_mask_res_et res = gt_mask_get(mask_line_buf, line_area.x, line_area.y, line_area.w);
        if(res == GT_MASK_RES_TRANSP) continue;
        if(res == GT_MASK_RES_FULL_COVER){
            blend_dsc.mask_buf = NULL;
        } else {
            blend_dsc.mask_buf = mask_line_buf;
        }
        gt_draw_blend(draw_ctx, &blend_dsc);
    }

    gt_mem_free(mask_line_buf);
    mask_line_buf = NULL;
free_lb:
    blend_dsc.mask_buf = NULL;

    gt_mask_remove_idx(border_mask_id);
    gt_mask_free(&border_mask_radius);
    gt_mask_remove_idx(in_mask_id);
    gt_mask_free(&in_mask_radius);
}

/* global functions / API interface -------------------------------------*/
void draw_bg(_gt_draw_ctx_st * draw_ctx, const gt_attr_rect_st * dsc, const gt_area_st * coords)
{
    if(dsc->bg_opa <= GT_OPA_MIN) return ;
    if(0 == coords->w || 0 == coords->h) return ;
    if(!gt_area_is_intersect_screen(&draw_ctx->buf_area, coords) ) return ;

#if GT_USE_SHADOW
    /* ---- Shadow: draw offset solid rect behind the widget ----------- */
    if (dsc->shadow && dsc->shadow->opa > GT_OPA_MIN) {
        gt_shadow_dsc_st const * sh = dsc->shadow;
        gt_area_st sh_area = {
            .x = coords->x + sh->offset_x - sh->spread,
            .y = coords->y + sh->offset_y - sh->spread,
            .w = coords->w + (sh->spread << 1),
            .h = coords->h + (sh->spread << 1),
        };
        if (sh_area.w > 0 && sh_area.h > 0 &&
            gt_area_is_intersect_screen(&draw_ctx->buf_area, &sh_area)) {
            gt_attr_rect_st sh_dsc = {0};
            gt_graph_init_rect_attr(&sh_dsc);
            sh_dsc.reg.is_fill = 1;          /* shadow must be filled */
            sh_dsc.bg_color  = sh->color;
            sh_dsc.bg_opa    = sh->opa;
            sh_dsc.radius    = dsc->radius;
            sh_dsc.limit_area = dsc->limit_area;
            uint16_t sh_radius = GT_MIN(sh_dsc.radius, (GT_MIN(sh_area.w >> 1, sh_area.h >> 1)));
            draw_bg_fill(draw_ctx, &sh_dsc, &sh_area, sh_radius);
        }
    }
#endif /* GT_USE_SHADOW */

    if(dsc->gray != 0){
        draw_dot_matrix(draw_ctx, dsc, coords);
        return ;
    }

    uint16_t border_radius = GT_MIN(dsc->radius, (GT_MIN(coords->w >> 1, coords->h >> 1)));
    draw_bg_fill(draw_ctx, dsc, coords, border_radius);
    draw_bg_border(draw_ctx, dsc, coords, border_radius);
}

_gt_draw_font_res_st draw_text(_gt_draw_ctx_st * draw_ctx, const gt_attr_font_st * dsc, const gt_area_st * coords)
{
    if (0 == dsc->font->len) {
        return _empty_string_handler(dsc);
    }
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
            if (dsc->raw_img->raw_p && false == dsc->reg.ignore_img_decoder) {
                /** MODE 2 */
                if (GT_RES_FAIL == gt_img_decoder_open_raw(&dsc_img, dsc->raw_img->raw_p, dsc->raw_img->raw_len)) {
                    GT_LOGW(GT_LOG_TAG_DATA, "Open raw data image decoder failed");
                    return;
                }
            }
            else {
                /** MODE 1 */
                if(GT_IMG_DECODER_TYPE_NONE == dsc->raw_img->header.type && NULL == dsc->raw_img->fp){
                    return;
                }
                dsc_img = *dsc->raw_img;
            }
            goto handler_lb;
        }
#if GT_USE_FILE_HEADER || GT_USE_DIRECT_ADDR || GT_USE_DIRECT_ADDR_CUSTOM_SIZE
        gt_res_t result = GT_RES_FAIL;
#endif
#if GT_USE_FILE_HEADER
        result = gt_img_decoder_fh_open(&dsc_img, dsc->file_header);
        if (GT_RES_OK == result) {
            goto handler_lb;
        } else if (GT_RES_FAIL == result) {
            GT_LOGW(GT_LOG_TAG_DATA, "Open image decoder by file header failed");
            return;
        }
#endif
#if GT_USE_DIRECT_ADDR
        result = gt_img_decoder_direct_addr_open(&dsc_img, dsc->addr);
        if (GT_RES_OK == result) {
            goto handler_lb;
        } else if (GT_RES_FAIL == result) {
            GT_LOGW(GT_LOG_TAG_DATA, "Open image decoder by direct address failed");
            return;
        }
#endif
#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
        result = gt_img_decoder_custom_size_addr_open(&dsc_img, dsc->custom_addr);
        if (GT_RES_OK == result) {
            goto handler_lb;
        } else if (GT_RES_FAIL == result) {
            GT_LOGW(GT_LOG_TAG_DATA, "Open image decoder by custom size direct address failed");
            return;
        }
#endif
        GT_LOGW(GT_LOG_TAG_DATA, "No image source");
        return;
    }
    else if (GT_RES_FAIL == gt_img_decoder_open(&dsc_img, dsc->bg_img_src)) {
        GT_LOGW(GT_LOG_TAG_DATA, "[%s] Open image decoder failed", dsc->bg_img_src);
        return;
    }
handler_lb:
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
        .masking = dsc->masking,
        .view_area = dsc->limit_area,   // TODO feat limit area range
        .opa = dsc->bg_opa
    };

    /* opa-only color tint: ignore image RGB, use fg_color with alpha channel */
    if (dsc->reg.use_opa_color) {
        blend_dsc.use_opa_color = 1;
        blend_dsc.color_fill = dsc->fg_color;
    }

    gt_size_t mask_id = -1;
    gt_size_t edge_mask_id = -1;
    gt_mask_radius_st r_mask = {0};
    gt_mask_edge_blur_st edge_blur = {0};
    if (blend_dsc.masking) {
        if (0 != blend_dsc.masking->gap) {
            gt_mask_edge_blur_init(&edge_blur, &mask_area, blend_dsc.masking->gap);
            edge_mask_id = gt_mask_add(&edge_blur);
        }

        if (0 != blend_dsc.masking->radius) {
            gt_mask_radius_init(&r_mask, &mask_area, blend_dsc.masking->radius, GT_MASK_RADIUS_CFG_INNER);
            mask_id = gt_mask_add(&r_mask);
        }
    }

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
#if GT_USE_SVG
        case GT_IMG_DECODER_TYPE_SVG: {
            _filling_by_ram_data(draw_ctx, &blend_dsc, &dsc_img);
            break;
        }
#endif
        default: {
            _filling_by_default(draw_ctx, &blend_dsc, &dsc_img);
            break;
        }
    }

    if (blend_dsc.masking) {
        if (0 != blend_dsc.masking->radius) {
            gt_mask_remove_idx(mask_id);
            gt_mask_free(&r_mask);
        }
        if (0 != blend_dsc.masking->gap) {
            gt_mask_remove_idx(edge_mask_id);
            gt_mask_free(&edge_blur);
        }
    }
    if (GT_IMG_DECODER_TYPE_RAM == dsc_img.header.type) {
        return ;
    }
close_lb:
#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    if (dsc->custom_addr) {
        gt_img_decoder_custom_size_addr_close(&dsc_img);
        return;
    }
#endif
    gt_img_decoder_close(&dsc_img);
}

GT_ATTRIBUTE_RAM_TEXT void gt_draw_point(_gt_draw_ctx_st * draw_ctx, gt_attr_point_st const * dsc, gt_area_st const * coords)
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
    uint16_t rd_val = dsc->line.width ? dsc->line.width : 4;
    gt_area_st area_line = { dsc->pos.x - (rd_val >> 1), dsc->pos.y - (rd_val >> 1), rd_val, rd_val };
    bool is_round = _is_brush_round(&dsc->line);
    gt_mask_radius_st r_mask = {0};
    gt_size_t mask_id = -1;
    gt_area_st * remark_area = draw_ctx->parent_area;
    if (is_round) {
        gt_mask_radius_init(&r_mask, &area_line, rd_val, GT_MASK_RADIUS_CFG_INNER);
        mask_id = gt_mask_add(&r_mask);
        if (-1 == mask_id) {
            goto radius_lb;
        }
    }
    draw_ctx->parent_area = (gt_area_st * )coords;  /** remark before parent area value, return will reload */

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
            gt_mask_get(blend_dsc.mask_buf, area_line.x, area_line.y, area_line.w);
        }
        ++area_line.y;
        gt_draw_blend(draw_ctx, &blend_dsc);
    }

    gt_mem_free(blend_dsc.mask_buf);
radius_lb:
    gt_mask_remove_idx(mask_id);
    gt_mask_free(&r_mask);
    draw_ctx->parent_area = remark_area;    /** reset to before value */
}

/* === Wu Anti-Aliased Line Drawing ===
 *
 * Replaces the previous mask-polygon approach (malloc + gt_mask registry) with:
 *   - Q16 fixed-point Xiaolin Wu algorithm (no FPU required)
 *   - Zero memory allocation (no mask_buf malloc)
 *   - Zero mask registry operations
 *   - Width > 1: circular disk template (O(width²) per Wu pixel)
 *   - Round caps: AA midpoint circle
 *   - Square caps: perpendicular disk bar
 *
 * Algorithm overview:
 *   1. Normalize to major-axis orientation (|dx| >= |dy|)
 *   2. Iterate along major axis (x)
 *   3. For each x, compute ideal y = y0 + x * (dy/dx) in Q16
 *   4. y_fract = fractional part of y (Q16 >> 8 = 0..255 opacity)
 *   5. Plot two pixels: (x, floor(y)) and (x, ceil(y))
 *      with opacities (255 - y_fract) and y_fract
 *   6. For width > 1: draw circular disk at each center point
 */

/**
 * @brief Integer square root (Newton-Raphson, 16-bit input → 16-bit output).
 *
 * Uses 2 Newton iterations for speed. Max error ±1 for all inputs.
 * Input range: [0, INT32_MAX]. For line length² (max 800²+480² ≈ 870400),
 * this is more than sufficient.
 *
 * @param val  Non-negative 32-bit integer
 * @return     floor(sqrt(val))
 */
static inline uint32_t _gt_isqrt(uint32_t val) {
    if (val == 0) return 0;
    /* Initial guess: 1 << (bits/2), where bits = position of highest set bit.
     * __builtin_clz(val) gives leading zeros, so highest bit = 31 - clz.
     * sqrt initial: 1 << ((31 - clz + 1) / 2) */
    uint32_t r = 1U << ((32 - __builtin_clz(val)) >> 1);
    /* Newton-Raphson iterations until convergence */
    r = (r + val / r) >> 1;
    r = (r + val / r) >> 1;
    r = (r + val / r) >> 1;
    r = (r + val / r) >> 1;
    /* Final correction: ensure r² ≤ val < (r+1)² */
    if (r > 0 && r * r > val) r--;
    return r;
}

#define _GT_WU_QSHIFT   16          /** Q16 fixed-point shift */

/**
 * @brief Write a single pixel with optional transparency blending (RGB565).
 *
 * @param draw_ctx  Drawing context (contains framebuffer base pointer)
 * @param x         Pixel X coordinate (absolute, relative to buf origin)
 * @param y         Pixel Y coordinate (absolute, relative to buf origin)
 * @param color     Source color (RGB565)
 * @param opa       Combined opacity (0..255); 255 = opaque overwrite
 */
static GT_ATTRIBUTE_RAM_TEXT void
_gt_wu_pixel(_gt_draw_ctx_st * draw_ctx,
              int32_t x, int32_t y,
              gt_color_t color, uint8_t opa)
{
    int32_t const bx = (int32_t)draw_ctx->buf_area.x;
    int32_t const by = (int32_t)draw_ctx->buf_area.y;
    if (x < bx || x >= bx + (int32_t)draw_ctx->buf_area.w) return;
    if (y < by || y >= by + (int32_t)draw_ctx->buf_area.h) return;

    gt_color_t * fb = (gt_color_t *)draw_ctx->buf;
    uint32_t const stride = draw_ctx->buf_area.w;
    int32_t x_idx = (int32_t)(x - bx);
    int32_t y_idx = (int32_t)(y - by);

#if GT_USE_SCREEN_ANIM
    if (draw_ctx->valid && false == draw_ctx->valid->layer_top) {
        x_idx += (int32_t)draw_ctx->valid->area_clip.x;
        y_idx += (int32_t)draw_ctx->valid->area_clip.y;
        if (x_idx < 0 || x_idx >= (int32_t)draw_ctx->buf_area.w) return;
        if (y_idx < 0 || y_idx >= (int32_t)draw_ctx->buf_area.h) return;
        if (draw_ctx->valid->is_hor) {
            if (x_idx >= (int32_t)(draw_ctx->valid->area_clip.x + draw_ctx->valid->area_clip.w)) return;
        } else {
            if (y_idx >= (int32_t)(draw_ctx->valid->area_clip.y + draw_ctx->valid->area_clip.h)) return;
        }
    }
#endif

    uint32_t const idx = (uint32_t)y_idx * stride + (uint32_t)x_idx;

    if (opa >= 255) {
        fb[idx] = color;
    } else if (opa > 0) {
        fb[idx] = gt_color_mix(color, fb[idx], opa);
    }
}

/**
 * @brief Distance-field filled circle for round line caps.
 *
 * Center is specified in Q16 sub-pixel coordinates (cx_f, cy_f) to align
 * with the Wu line body's actual center, which has a 0.5-minor-unit offset.
 * The integer pixel center is floor(cx_f >> 16), floor(cy_f >> 16).
 * Distance is computed from the sub-pixel center for accurate AA.
 *
 * Uses Q8 distance for sub-pixel accurate radius matching.
 *
 * @param draw_ctx  Drawing context
 * @param cx        Center X (integer pixel coordinate)
 * @param cy        Center Y (integer pixel coordinate)
 * @param r_q8      Circle radius in Q8 (= half_w * 256 * dmaj / len for angle compensation)
 * @param color     Fill color
 * @param opa       Global opacity (0..255)
 */
/**
 * @brief Draw a full AA circle cap at a line endpoint.
 * @deprecated Replaced by unified distance-field rendering in _gt_wu_line.
 */
static GT_ATTRIBUTE_RAM_TEXT void
_gt_wu_round_cap(_gt_draw_ctx_st * draw_ctx,
                  int32_t cx, int32_t cy,
                  int32_t r_q8,
                  gt_color_t color, gt_opa_t opa,
                  bool steep, int32_t maj_dir)
{
    /* maj_dir: -1 = only draw pixels where maj <= cap_maj (cap_a end)
     *          +1 = only draw pixels where maj >= cap_maj (cap_b end)
     *           0 = draw full circle (unused)
     * This restricts the cap to the half-circle outside the line body,
     * preventing double-blend with the body pixels. */
    if (r_q8 <= 0) return;

    int32_t const inner_q8 = r_q8 - 128;
    int32_t const outer_q8 = r_q8 + 128;
    int32_t const scan_r   = (outer_q8 + 255) >> 8;

    for (int32_t dy = -scan_r; dy <= scan_r; ++dy) {
        for (int32_t dx = -scan_r; dx <= scan_r; ++dx) {
            /* Restrict to the outward half only */
            int32_t const dmaj = steep ? dy : dx;
            if (maj_dir < 0 && dmaj > 0) continue;
            if (maj_dir > 0 && dmaj < 0) continue;

            int32_t const d2   = dx * dx + dy * dy;
            int32_t const d_q8 = (int32_t)(_gt_isqrt((uint32_t)d2) * 256u);
            if (d_q8 > outer_q8) continue;

            uint8_t pa;
            if (d_q8 <= inner_q8) {
                pa = (uint8_t)opa;
            } else {
                int32_t aa = ((outer_q8 - d_q8) * 255) / 256;
                if (aa <= 0) continue;
                if (aa > 255) aa = 255;
                pa = (uint8_t)GT_UDIV255(opa * (uint8_t)aa);
            }
            if (pa > 0) {
                _gt_wu_pixel(draw_ctx, cx + dx, cy + dy, color, pa);
            }
        }
    }
}

/**
 * @brief Circular disk template for thick line body.
 *
 * For width > 1, each Wu center point has a disk of radius `half_w`.
 * Uses simple circle-equation check: only draws if dx² + dy² ≤ r².
 *
 * @param draw_ctx  Drawing context
 * @param px        Center X of disk
 * @param py        Center Y of disk
 * @param half_w    Disk radius (half of line width)
 * @param color     Source color
 * @param opa       Combined opacity (0..255)
 */
static GT_ATTRIBUTE_RAM_TEXT void
_gt_wu_thick_disk(_gt_draw_ctx_st * draw_ctx,
                   int32_t px, int32_t py,
                   int16_t half_w,
                   gt_color_t color, uint8_t opa)
{
    /* width ≤ 2 → half_w ≤ 1 → trivial disk, just write center pixel */
    if (half_w <= 1) {
        _gt_wu_pixel(draw_ctx, px, py, color, opa);
        return;
    }

    int32_t const r2 = (int32_t)half_w * half_w;

    /* Scanline fill: for each dy, compute max |dx| where dx² + dy² ≤ r² */
    for (int32_t dy = -half_w; dy <= half_w; ++dy) {
        int32_t dx2_max = r2 - dy * dy;
        if (dx2_max < 0) continue;
        /* Integer sqrt approximation: binary search 0..half_w */
        int16_t dx_abs_max = 0;
        int16_t lo = 0, hi = half_w;
        while (lo <= hi) {
            int16_t mid = (lo + hi) >> 1;
            if (mid * mid <= dx2_max) {
                dx_abs_max = mid;
                lo = mid + 1;
            } else {
                hi = mid - 1;
            }
        }
        for (int32_t dx = -dx_abs_max; dx <= dx_abs_max; ++dx) {
            _gt_wu_pixel(draw_ctx, px + dx, py + dy, color, opa);
        }
    }
}

/**
 * @brief Square cap: scan along the minor axis, same width as the line body.
 *
 * The line body scans along the minor axis with range [center - half_w, center + half_w].
 * The cap does the same at the endpoint scanline, producing a flat rectangular end face.
 *
 * @param draw_ctx  Drawing context
 * @param maj_pos   Major-axis coordinate of the endpoint scanline
 * @param min_center Minor-axis center of the endpoint (from Wu gradient)
 * @param half_w    Half line width
 * @param steep     true = steep line (major=Y, minor=X)
 * @param color     Source color
 * @param opa       Combined opacity
 */
static GT_ATTRIBUTE_RAM_TEXT void
_gt_wu_square_cap(_gt_draw_ctx_st * draw_ctx,
                    int32_t maj_pos, int32_t min_center,
                    int16_t half_w, bool steep,
                    gt_color_t color, uint8_t opa)
{
    int32_t const scan_lo = (half_w & 1) ? (min_center - half_w) : (min_center - half_w + 1);
    int32_t const scan_hi = min_center + half_w;
    for (int32_t s = scan_lo; s <= scan_hi; ++s) {
        if (steep) { _gt_wu_pixel(draw_ctx, s, maj_pos, color, opa); }
        else       { _gt_wu_pixel(draw_ctx, maj_pos, s, color, opa); }
    }
}

/**
 * @brief Xiaolin Wu anti-aliased line — unified distance-field rendering.
 *
 * All pixels (body + end caps) are rendered in a single pass using the same
 * coordinate system: t = projection along line, d = perpendicular distance.
 * This eliminates the body/cap seam that caused visible gaps or arrow shapes.
 *
 * For each pixel in the bounding box:
 *   t_q16 = dot(p - p0, line_dir)   in Q16  [0..len_q16 = line body range]
 *   d_q16 = dot(p - p0, line_norm)  in Q16  perpendicular distance
 *
 * Pixel alpha is determined by the signed distance to the line shape boundary:
 *   - Body:       0 <= t <= len,  |d| <= half_w
 *   - Round cap:  (t<0 or t>len), sqrt((t_cap)²+d²) <= half_w
 *   - Square cap: (t<0 or t>len), |d| <= half_w  (extends the body rectangle)
 *
 * AA edge width = 1 pixel (Q16 = 65536).
 */
static GT_ATTRIBUTE_RAM_TEXT void
_gt_wu_line(_gt_draw_ctx_st * ctx,
             gt_attr_line_st const * dsc)
{
    gt_color_t const color     = dsc->line.color;
    gt_opa_t   const global_opa = dsc->line.opa;
    uint16_t   const width     = dsc->line.width;
    bool       const is_round  = _is_brush_round(&dsc->line) && (width > 1);
    uint8_t  const dash_w     = dsc->line.dash_width;
    uint8_t  const dash_g     = dsc->line.dash_gap;
    bool     const is_dash    = (dash_w > 0 && dash_g > 0);
    uint16_t const dash_cycle = is_dash ? (uint16_t)dash_w + (uint16_t)dash_g : 0;

    int32_t x0 = dsc->start.x, y0 = dsc->start.y;
    int32_t x1 = dsc->end.x,   y1 = dsc->end.y;
    int32_t const dx = x1 - x0, dy = y1 - y0;

    /* Line length */
    uint32_t const len2 = (uint32_t)(dx * dx + dy * dy);
    uint32_t const len  = _gt_isqrt(len2);
    if (len == 0) return;

    /* Unit direction vector (Q16): dir = (dx/len, dy/len) */
    int32_t const dir_x = (int32_t)((int64_t)dx * 65536 / (int32_t)len);
    int32_t const dir_y = (int32_t)((int64_t)dy * 65536 / (int32_t)len);
    /* Unit normal vector (Q16): norm = (-dy/len, dx/len) */
    int32_t const nor_x = -dir_y;
    int32_t const nor_y =  dir_x;

    /* half_w in integer pixels */
    int32_t const half_w = (int32_t)(width >> 1);

    /* Bounding box: body box + cap extension */
    int32_t const cap_ext = is_round ? half_w + 1 : 0;
    int32_t const bx_lo = GT_MIN(x0, x1) - half_w - cap_ext - 1;
    int32_t const bx_hi = GT_MAX(x0, x1) + half_w + cap_ext + 1;
    int32_t const by_lo = GT_MIN(y0, y1) - half_w - cap_ext - 1;
    int32_t const by_hi = GT_MAX(y0, y1) + half_w + cap_ext + 1;

    for (int32_t py = by_lo; py <= by_hi; ++py) {
        int32_t const rel_y = py - y0;
        for (int32_t px = bx_lo; px <= bx_hi; ++px) {
            int32_t const rel_x = px - x0;

            /* t, d in Q16 pixel units:
             * rel_x/rel_y are integers; dir/nor are Q16 unit vectors.
             * dot = (integer * Q16) >> 16 = integer pixels.
             * Keep as Q16 by NOT shifting: dot = integer * Q16 = Q16 pixels. */
            int64_t const t64 = (int64_t)rel_x * dir_x + (int64_t)rel_y * dir_y;
            int64_t const d64 = (int64_t)rel_x * nor_x + (int64_t)rel_y * nor_y;
            /* t64, d64 are in Q16 pixel units */
            int64_t const abs_d64 = (d64 < 0) ? -d64 : d64;

            /* half_w in Q16 for sub-pixel AA */
            int64_t const hw_q16 = (int64_t)half_w << 16;

            /* Signed distance to shape boundary in Q16 pixels (positive = inside) */
            int32_t sdf;

            if (t64 >= 0 && t64 <= ((int64_t)len << 16)) {
                /* Body: distance to side edge */
                if (is_dash) {
                    int32_t t_px = (int32_t)(t64 >> 16);
                    if (t_px < 0) t_px = 0;
                    uint16_t phase = (uint16_t)((uint32_t)t_px % (uint32_t)dash_cycle);
                    if (phase >= dash_w) continue;  /* in gap, skip pixel */
                }
                sdf = (int32_t)(hw_q16 - abs_d64);
            } else if (is_round) {
                /* Round cap: Euclidean distance to nearest endpoint */
                int64_t const t_cap64 = (t64 < 0) ? -t64 : (t64 - ((int64_t)len << 16));
                /* Both t_cap64 and d64 are Q16; sqrt(t²+d²) in Q16 */
                int32_t const t_q8 = (int32_t)(t_cap64 >> 8);
                int32_t const d_q8 = (int32_t)(abs_d64 >> 8);
                uint32_t const dist2_q8 = (uint32_t)(t_q8 * t_q8 + d_q8 * d_q8);
                int32_t const dist_q16  = (int32_t)(_gt_isqrt(dist2_q8) << 8);
                sdf = (int32_t)(hw_q16 - dist_q16);
            } else {
                /* Square cap: AA on the end face only.
                 * The body already covers t in [0, len].
                 * Here we handle the 1-pixel AA fringe just outside the endpoints. */
                if (abs_d64 > hw_q16) continue;
                int64_t const t_cap64 = (t64 < 0) ? -t64 : (t64 - ((int64_t)len << 16));
                /* t_cap64 > 0 means outside endpoint; AA within 1 pixel */
                sdf = 65536 - (int32_t)t_cap64;
            }

            if (sdf <= 0) continue;

            /* AA: map sdf [0..65536] → alpha [0..255] */
            uint8_t aa;
            if (sdf >= 65536) {
                aa = 255;
            } else {
                aa = (uint8_t)((sdf * 255) >> 16);
            }
            uint8_t const pa = (global_opa >= 255) ? aa
                                                    : (uint8_t)GT_UDIV255(global_opa * aa);
            if (pa > 0) _gt_wu_pixel(ctx, px, py, color, pa);
        }
    }
}

void GT_ATTRIBUTE_RAM_TEXT
gt_draw_line(_gt_draw_ctx_st * draw_ctx, gt_attr_line_st const * dsc, const gt_area_st * coords)
{
    /* ── Guard clauses ── */
    if (dsc->line.opa < GT_OPA_MIN)                                    return;
    if (NULL == draw_ctx->buf)                                         return;
    if (!gt_area_is_intersect_screen(&draw_ctx->buf_area, coords))     return;
    if (dsc->line.width == 0)                                          return;

    /* ── Fast path: horizontal and vertical lines ── */
    bool const is_round_cap = _is_brush_round(&dsc->line) && (dsc->line.width > 1);

    if (dsc->start.x == dsc->end.x) {
        /* Vertical line */
        int32_t const y_start = GT_MIN(dsc->start.y, dsc->end.y);
        int32_t const y_end   = GT_MAX(dsc->start.y, dsc->end.y);
        int32_t const line_w  = dsc->line.width;
        uint8_t const dw = dsc->line.dash_width;
        uint8_t const dg = dsc->line.dash_gap;

        gt_draw_blend_dsc_st bd = {
            .color_fill = dsc->line.color,
            .opa        = dsc->line.opa,
        };

        if (dw == 0 || dg == 0) {
            /* Solid line */
            gt_area_st dst = {
                .x = dsc->start.x - (line_w >> 1),
                .y = y_start,
                .w = line_w,
                .h = y_end - y_start + 1,
            };
            bd.dst_area = &dst;
            if (is_round_cap) {
                gt_mask_radius_st r_mask = {0};
                gt_mask_radius_init(&r_mask, &dst, line_w, GT_MASK_RADIUS_CFG_INNER);
                gt_size_t mask_id = gt_mask_add(&r_mask);
                gt_draw_blend(draw_ctx, &bd);
                if (mask_id >= 0) gt_mask_remove_idx(mask_id);
            } else {
                gt_draw_blend(draw_ctx, &bd);
            }
        } else {
            /* Dashed line: draw segments (round cap not applied per-segment by design) */
            uint16_t const cycle = (uint16_t)dw + (uint16_t)dg;
            int32_t y = y_start;
            while (y <= y_end) {
                int32_t seg_end = y + dw - 1;
                if (seg_end > y_end) seg_end = y_end;
                gt_area_st dst = {
                    .x = dsc->start.x - (line_w >> 1),
                    .y = y,
                    .w = line_w,
                    .h = seg_end - y + 1,
                };
                bd.dst_area = &dst;
                gt_draw_blend(draw_ctx, &bd);
                y += cycle;
            }
        }
        return;
    }

    if (dsc->start.y == dsc->end.y) {
        /* Horizontal line */
        int32_t const x_start = GT_MIN(dsc->start.x, dsc->end.x);
        int32_t const x_end   = GT_MAX(dsc->start.x, dsc->end.x);
        int32_t const line_w  = dsc->line.width;
        uint8_t const dw = dsc->line.dash_width;
        uint8_t const dg = dsc->line.dash_gap;

        gt_draw_blend_dsc_st bd = {
            .color_fill = dsc->line.color,
            .opa        = dsc->line.opa,
        };

        if (dw == 0 || dg == 0) {
            /* Solid line */
            gt_area_st dst = {
                .x = x_start,
                .y = dsc->start.y - (line_w >> 1),
                .w = x_end - x_start + 1,
                .h = line_w,
            };
            bd.dst_area = &dst;
            if (is_round_cap) {
                gt_mask_radius_st r_mask = {0};
                gt_mask_radius_init(&r_mask, &dst, line_w, GT_MASK_RADIUS_CFG_INNER);
                gt_size_t mask_id = gt_mask_add(&r_mask);
                gt_draw_blend(draw_ctx, &bd);
                if (mask_id >= 0) gt_mask_remove_idx(mask_id);
            } else {
                gt_draw_blend(draw_ctx, &bd);
            }
        } else {
            /* Dashed line: draw segments (round cap not applied per-segment by design) */
            uint16_t const cycle = (uint16_t)dw + (uint16_t)dg;
            int32_t x = x_start;
            while (x <= x_end) {
                int32_t seg_end = x + dw - 1;
                if (seg_end > x_end) seg_end = x_end;
                gt_area_st dst = {
                    .x = x,
                    .y = dsc->start.y - (line_w >> 1),
                    .w = seg_end - x + 1,
                    .h = line_w,
                };
                bd.dst_area = &dst;
                gt_draw_blend(draw_ctx, &bd);
                x += cycle;
            }
        }
        return;
    }

    /* ── Diagonal line: Wu anti-aliased algorithm ── */
    _gt_wu_line(draw_ctx, dsc);
}

void GT_ATTRIBUTE_RAM_TEXT
gt_draw_catmullrom(_gt_draw_ctx_st * draw_ctx, gt_attr_curve_st const * dsc, const gt_area_st * coords)
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
    uint16_t rd_val = dsc->line.width ? dsc->line.width : 4;
    gt_area_st area_line = { dsc->p0.x - (rd_val >> 1), dsc->p0.y - (rd_val >> 1), rd_val, rd_val };
    gt_mask_radius_st r_mask = {0};
    gt_size_t mask_id = -1;
    gt_area_st * remark_area = draw_ctx->parent_area;
    bool is_round = _is_brush_round(&dsc->line);
    if (is_round) {
        gt_mask_radius_init(&r_mask, &area_line, rd_val, GT_MASK_RADIUS_CFG_INNER);
        mask_id = gt_mask_add(&r_mask);
        if (-1 == mask_id) {
            goto radius_lb;
        }
    }
    draw_ctx->parent_area = (gt_area_st * )coords;  /** remark before parent area value, return will reload */

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
                r_mask.area.x = area_line.x;
                r_mask.area.y = area_line.y;
                gt_mask_get(blend_dsc.mask_buf, area_line.x, area_line.y, area_line.w);
            }
            area_line.x = (gt_size_t)tmp.x;
            area_line.y = (gt_size_t)tmp.y + i;
            gt_draw_blend(draw_ctx, &blend_dsc);
        }
    }

    gt_mem_free(blend_dsc.mask_buf);
radius_lb:
    gt_mask_remove_idx(mask_id);
    gt_mask_free(&r_mask);
    draw_ctx->parent_area = remark_area;    /** reset to before value */
}

GT_ATTRIBUTE_RAM_TEXT void draw_focus(gt_obj_st* obj)
{
    if (false == obj->focus) {
        return;
    }
    if (GT_ENABLED != obj->focus_dis) {
        return;
    }

    gt_size_t rd_val = gt_obj_get_focus_offset_value(obj);
    gt_area_st area = gt_area_reduce(obj->area, -rd_val);
    gt_size_t radius = obj->radius + rd_val;

    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    rect_attr.bg_opa = obj->opa;
    rect_attr.radius = radius;
    rect_attr.reg.is_fill = 0;
    rect_attr.border_width = gt_obj_get_focus_w(obj);
    rect_attr.bg_color = gt_color_focus();
    rect_attr.border_color = gt_color_focus();

    draw_bg(obj->draw_ctx, &rect_attr, &area);
}

/* ---- Arc geometry context -------------------------------------------- */
typedef struct {
    uint16_t start_angle;
    uint16_t end_angle;
    uint16_t out_radius;
    uint16_t in_radius;
    uint16_t width;
    int16_t  out_mask_id;
    int16_t  in_mask_id;
    gt_mask_radius_st out_mask;
    gt_mask_radius_st in_mask;
    gt_area_st area_out;
} _arc_geo_st;

/* ---- Gradient arc rounded end-caps ----------------------------------- */
#if GT_USE_GRADIENT
static inline void GT_ATTRIBUTE_RAM_TEXT
_arc_draw_caps_grad(_gt_draw_ctx_st * draw_ctx, gt_attr_arc_st const * dsc,
                    _arc_geo_st * geo, int32_t cx, int32_t cy,
                    uint16_t start_angle, uint16_t end_angle)
{
    if (!dsc->rounded) { return; }

    gt_area_st s_area, e_area;
    gt_rounded_area_get(start_angle, geo->out_radius, geo->width, &s_area);
    s_area.x += cx; s_area.y += cy;
    gt_rounded_area_get(end_angle, geo->out_radius, geo->width, &e_area);
    e_area.x += cx; e_area.y += cy;

    gt_color_t c_start = gt_gradient_dsc_get_color_pos(dsc->grad, 0);
    gt_color_t c_end;
    if (dsc->grad_ref_span > 0) {
        uint16_t cap_span = (end_angle > start_angle)
                          ? (end_angle - start_angle)
                          : (360 - start_angle + end_angle);
        uint16_t cap_pos = (uint16_t)(((uint32_t)cap_span * 65535u) / dsc->grad_ref_span);
        c_end = gt_gradient_dsc_get_color_pos(dsc->grad, cap_pos);
    } else {
        c_end = gt_gradient_dsc_get_color_pos(dsc->grad, 65535);
    }

    int32_t ddx = (e_area.x + e_area.w / 2) - (s_area.x + s_area.w / 2);
    int32_t ddy = (e_area.y + e_area.h / 2) - (s_area.y + s_area.h / 2);
    int32_t dist2  = ddx * ddx + ddy * ddy;
    int32_t thresh = geo->width * 2;

    gt_color_t c_end_draw = c_end;
    if (dist2 < thresh * thresh) {
        uint32_t ud = (uint32_t)dist2;
        uint32_t x0 = (uint32_t)(thresh > 0 ? thresh : 1);
        x0 = (x0 + ud / x0) >> 1;
        x0 = (x0 + ud / x0) >> 1;
        x0 = (x0 + ud / x0) >> 1;
        uint8_t t = (uint8_t)((uint32_t)x0 * 255u / (uint32_t)thresh);
        c_end_draw = gt_color_mix(c_end, c_start, t);
    }

    gt_attr_rect_st cap;
    gt_graph_init_rect_attr(&cap);
    cap.bg_opa = dsc->opa;
    cap.reg.is_fill = 1;

    cap.bg_color = c_start;
    cap.radius   = s_area.w;
    draw_bg(draw_ctx, &cap, &s_area);

    cap.bg_color = c_end_draw;
    cap.radius   = e_area.w;
    draw_bg(draw_ctx, &cap, &e_area);
}
#endif /* GT_USE_GRADIENT */

/* ---- Solid/image arc path -------------------------------------------- */
static inline void GT_ATTRIBUTE_RAM_TEXT
_arc_draw_solid(_gt_draw_ctx_st * draw_ctx, gt_attr_arc_st const * dsc,
                _arc_geo_st * geo, gt_attr_rect_st * rect_attr, bool is_img)
{
    uint16_t start_angle = geo->start_angle;
    uint16_t end_angle   = geo->end_angle;

    if (start_angle + 360 == end_angle || start_angle == end_angle + 360) {
        if (is_img) { draw_bg_img(draw_ctx, rect_attr, &geo->area_out); }
        else        { draw_bg(draw_ctx, rect_attr, &geo->area_out); }
        gt_mask_remove_idx(geo->out_mask_id); gt_mask_free(&geo->out_mask);
        if (geo->in_radius > 0) { gt_mask_remove_idx(geo->in_mask_id); gt_mask_free(&geo->in_mask); }
        return;
    }

    while (start_angle >= 360) { start_angle -= 360; }
    while (end_angle   >= 360) { end_angle   -= 360; }

    gt_point_st center = {
        .x = geo->area_out.x + geo->out_radius,
        .y = geo->area_out.y + geo->out_radius,
    };

    gt_mask_angle_st angle_mask = {0};
    gt_mask_angle_init(&angle_mask, center.x, center.y, start_angle, end_angle);
    int16_t angle_mask_id = gt_mask_add(&angle_mask);

    int32_t angle_gap = (end_angle > start_angle)
                      ? (360 - (end_angle - start_angle))
                      : (start_angle - end_angle);

    if (angle_gap > 60 && geo->out_radius > 10) {
        if (is_img) {
            draw_bg_img(draw_ctx, rect_attr, &geo->area_out);
        } else {
            _quarter_circle_st q;
            q.coords        = &geo->area_out;
            q.center        = &center;
            q.radius        = geo->out_radius;
            q.width         = geo->width;
            q.start_angle   = start_angle;
            q.end_angle     = end_angle;
            q.start_quarter = (start_angle / 90) & 0x03;
            q.end_quarter   = (end_angle   / 90) & 0x03;
            q.rect_dsc      = rect_attr;
            _draw_quarter_circle_0(draw_ctx, &q);
            _draw_quarter_circle_1(draw_ctx, &q);
            _draw_quarter_circle_2(draw_ctx, &q);
            _draw_quarter_circle_3(draw_ctx, &q);
        }
    } else {
        if (is_img) { draw_bg_img(draw_ctx, rect_attr, &geo->area_out); }
        else        { draw_bg(draw_ctx, rect_attr, &geo->area_out); }
    }

    gt_mask_remove_idx(angle_mask_id); gt_mask_free(&angle_mask);
    gt_mask_remove_idx(geo->out_mask_id); gt_mask_free(&geo->out_mask);
    if (geo->in_radius > 0) { gt_mask_remove_idx(geo->in_mask_id); gt_mask_free(&geo->in_mask); }

    if (is_img || !dsc->rounded) { return; }

    gt_area_st round_area;
    gt_rounded_area_get(start_angle, geo->out_radius, geo->width, &round_area);
    round_area.x += center.x; round_area.y += center.y;
    rect_attr->radius = round_area.w;
    draw_bg(draw_ctx, rect_attr, &round_area);

    gt_rounded_area_get(end_angle, geo->out_radius, geo->width, &round_area);
    round_area.x += center.x; round_area.y += center.y;
    rect_attr->radius = round_area.w;
    draw_bg(draw_ctx, rect_attr, &round_area);
}

void GT_ATTRIBUTE_RAM_TEXT
gt_draw_arc(_gt_draw_ctx_st * draw_ctx, gt_attr_arc_st const * dsc, const gt_area_st * coords)
{

    if(dsc->opa <= GT_OPA_MIN) { return; }
    if(dsc->width == 0) { return; }
    if(dsc->start_angle == dsc->end_angle) { return; }

    /* --- Geometry init --- */
    _arc_geo_st geo = {0};
    geo.start_angle = dsc->start_angle;
    geo.end_angle   = dsc->end_angle;
    gt_area_copy(&geo.area_out, coords);
    geo.out_radius = GT_MIN(geo.area_out.h, geo.area_out.w) >> 1;
    geo.width      = GT_MIN(dsc->width, geo.out_radius);

    gt_mask_radius_init(&geo.out_mask, &geo.area_out, geo.out_radius, GT_MASK_RADIUS_CFG_INNER);
    geo.out_mask_id = gt_mask_add(&geo.out_mask);

    gt_area_st area_in = gt_area_reduce(geo.area_out, geo.width);
    geo.in_radius = GT_MIN(area_in.h, area_in.w) >> 1;
    geo.in_mask_id = -1;
    if (geo.in_radius > 0) {
        gt_mask_radius_init(&geo.in_mask, &area_in, geo.in_radius, GT_MASK_RADIUS_CFG_OUTER);
        geo.in_mask_id = gt_mask_add(&geo.in_mask);
    }

    /* --- rect_attr: solid color or image --- */
    bool is_img = false;
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    if (dsc->img || dsc->raw_img
#if GT_USE_FILE_HEADER
        || dsc->file_header
#endif
#if GT_USE_DIRECT_ADDR
        || !gt_hal_is_invalid_addr(dsc->addr)
#endif
#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
        || !gt_hal_is_invalid_custom_size_addr(dsc->custom_addr)
#endif
    ) {
        is_img = true;
        rect_attr.bg_img_opa = dsc->opa;
        rect_attr.bg_img_src = dsc->img;
        rect_attr.raw_img    = dsc->raw_img;
#if GT_USE_FILE_HEADER
        rect_attr.file_header = dsc->file_header;
#endif
#if GT_USE_DIRECT_ADDR
        rect_attr.addr = dsc->addr;
#endif
#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
        rect_attr.custom_addr = dsc->custom_addr;
#endif
    } else {
        rect_attr.bg_opa   = dsc->opa;
        rect_attr.bg_color = dsc->color;
        rect_attr.reg.is_fill = 1;
    }

#if GT_USE_GRADIENT
    /* ---- Angular gradient path (per-pixel scanline + Floyd-Steinberg) -- */
    if (!is_img && dsc->grad && dsc->grad->stops &&
        _gt_vector_get_count(dsc->grad->stops) > 0) {

        uint16_t start_angle = geo.start_angle;
        uint16_t end_angle   = geo.end_angle;
        uint16_t out_radius  = geo.out_radius;
        uint16_t in_radius   = geo.in_radius;
        uint16_t width       = geo.width;
        gt_area_st area_out  = geo.area_out;

        while(start_angle >= 360) { start_angle -= 360; }
        while(end_angle >= 360) { end_angle -= 360; }

        /* Compute total arc span in degrees (1..360) */
        int32_t arc_span;
        bool is_full_circle = (dsc->start_angle + 360 == dsc->end_angle ||
                               dsc->start_angle == dsc->end_angle + 360 ||
                               start_angle == end_angle);
        if (is_full_circle) {
            arc_span = 360;
        } else if (end_angle > start_angle) {
            arc_span = end_angle - start_angle;
        } else {
            arc_span = 360 - start_angle + end_angle;
        }
        if (arc_span <= 0) arc_span = 360;

        int32_t cx = area_out.x + out_radius;
        int32_t cy = area_out.y + out_radius;

        /* Add angle mask for partial arcs */
        int16_t ang_mask_id = -1;
        gt_mask_angle_st ang_mask = {0};
        if (!is_full_circle) {
            gt_mask_angle_init(&ang_mask, (gt_size_t)cx, (gt_size_t)cy,
                               (gt_size_t)start_angle, (gt_size_t)end_angle);
            ang_mask_id = gt_mask_add(&ang_mask);
        }

        /* Allocate per-row buffers */
        uint16_t row_w = (uint16_t)area_out.w;
        gt_color_t * color_row_buf = (gt_color_t *)gt_mem_malloc(row_w * sizeof(gt_color_t));
        gt_opa_t   * mask_buf      = (gt_opa_t   *)gt_mem_malloc(row_w);
        int16_t    * fs_err_cur    = (int16_t    *)gt_mem_malloc(row_w * 3 * sizeof(int16_t));
        int16_t    * fs_err_nxt    = (int16_t    *)gt_mem_malloc(row_w * 3 * sizeof(int16_t));
        if (fs_err_cur) gt_memset(fs_err_cur, 0, row_w * 3 * sizeof(int16_t));
        if (fs_err_nxt) gt_memset(fs_err_nxt, 0, row_w * 3 * sizeof(int16_t));

        bool has_fs = (color_row_buf && fs_err_cur && fs_err_nxt);

        /* FS helper macros (local scope) */
        #define _ARC_FS_CLAMP8(v)  ((v) < 0 ? 0 : ((v) > 255 ? 255 : (v)))
        #define _ARC_FS_QUANT5(v8) (((v8) >> 3) * (255 / 31))
        #define _ARC_FS_QUANT6(v8) (((v8) >> 2) * (255 / 63))
        #define _ARC_FS_ADD(arr, idx, e) \
            do { if ((idx) >= 0 && (idx) < (int32_t)(row_w * 3)) (arr)[(idx)] += (e); } while(0)
        #define _ARC_FS_SWAP() do { \
            int16_t * _t = fs_err_cur; fs_err_cur = fs_err_nxt; fs_err_nxt = _t; \
            if (fs_err_nxt) gt_memset(fs_err_nxt, 0, row_w * 3 * sizeof(int16_t)); \
        } while(0)

        uint8_t fade_ow = (dsc->fade_outer_width > 0) ? GT_MIN(dsc->fade_outer_width, (uint8_t)width) : 0;
        uint8_t fade_iw = (dsc->fade_inner_width > 0 && in_radius > 0) ? GT_MIN(dsc->fade_inner_width, (uint8_t)width) : 0;

        gt_draw_blend_dsc_st blend_dsc = {0};
        blend_dsc.opa = dsc->opa;

        gt_area_st line_area = { .x = area_out.x, .w = row_w, .h = 1 };
        blend_dsc.dst_area  = &line_area;
        blend_dsc.mask_area = &line_area;

        /* shape_mask: reused each row to apply out/in/angle masks */
        gt_opa_t * shape_mask = (gt_opa_t *)gt_mem_malloc(row_w);

        for (int32_t row = 0; row < (int32_t)area_out.h; row++) {
            line_area.y = area_out.y + row;
            int32_t dy = line_area.y - cy;

            if (has_fs && color_row_buf) {
                for (uint16_t col = 0; col < row_w; col++) {
                    int32_t dx = area_out.x + (int32_t)col - cx;

                    /* Angle computation.
                     * GT_ARC_GRAD_USE_FPU=1: atan2f, full float precision, ~40 cy on M4F.
                     * GT_ARC_GRAD_USE_FPU=0: gt_atan2 integer lookup, ±1° precision,
                     *   zero soft-float calls — ~20 cy on M3/M0 vs ~10000 cy for soft atan2f. */
                    float _af_deg;
#if GT_ARC_GRAD_USE_FPU
                    float _af = atan2f((float)dy, (float)dx) * (float)(180.0 / 3.14159265358979323846);
                    if (_af < 0.0f) _af += 360.0f;
                    _af_deg = _af;
#else
                    /* gt_atan2(x, y): first arg is x, second is y.
                     * Pass (dy, dx) so it computes atan2(dy, dx) matching the FPU convention:
                     *   dx>0, dy=0 → 0°;  dx=0, dy>0 → 90°. */
                    _af_deg = (float)gt_atan2(dy, dx);
#endif

                    /* Map angle to arc position [0..65535] */
                    float rel_f = _af_deg - (float)start_angle;
                    if (rel_f < 0.0f) rel_f += 360.0f;
                    if (!is_full_circle && rel_f > (float)arc_span) rel_f = (float)arc_span;
                    float ref_span = (dsc->grad_ref_span > 0) ? (float)dsc->grad_ref_span : (float)arc_span;
                    uint16_t pos = (uint16_t)(rel_f * 65535.0f / ref_span + 0.5f);

                    /* Compute radial fade opacity — Newton sqrt with clz-based initial estimate */
                    int32_t r2 = dx * dx + dy * dy;
                    int32_t r_px = 0;
                    {
                        uint32_t ur2 = (uint32_t)(r2 < 0 ? 0 : r2);
                        uint32_t x0;
                        if (ur2 == 0) {
                            x0 = 0;
                        } else {
#if defined(__GNUC__) || defined(__clang__)
                            /* clz-based initial guess: 2^⌈(32-clz(n))/2⌉, within √2× of true sqrt */
                            x0 = 1u << ((33u - (uint32_t)__builtin_clz(ur2)) >> 1);
#else
                            x0 = (out_radius > 0) ? (uint32_t)out_radius : 1;
#endif
                            /* 2 Newton iterations — sufficient after the improved initial guess */
                            x0 = (x0 + ur2 / x0) >> 1;
                            x0 = (x0 + ur2 / x0) >> 1;
                        }
                        r_px = (int32_t)x0;
                    }

                    gt_opa_t pixel_opa = GT_OPA_COVER;
                    if (fade_ow > 0) {
                        int32_t dist = out_radius - r_px + 2; /* +2: AA guard */
                        if (dist < (int32_t)fade_ow) {
                            pixel_opa = (gt_opa_t)((uint32_t)GT_OPA_COVER * (uint32_t)(dist < 0 ? 0 : dist) / fade_ow);
                        }
                    }
                    if (fade_iw > 0 && in_radius > 0) {
                        int32_t dist = r_px - in_radius + 2; /* +2: AA guard */
                        if (dist < (int32_t)fade_iw) {
                            gt_opa_t inner_opa = (gt_opa_t)((uint32_t)GT_OPA_COVER * (uint32_t)(dist < 0 ? 0 : dist) / fade_iw);
                            if (inner_opa < pixel_opa) pixel_opa = inner_opa;
                        }
                    }

                    /* Sample gradient color (24-bit for FS accuracy) */
                    uint32_t c24 = gt_gradient_dsc_get_color24_pos(dsc->grad, pos);
                    int32_t ir = (int32_t)((c24 >> 16) & 0xFF);
                    int32_t ig = (int32_t)((c24 >>  8) & 0xFF);
                    int32_t ib = (int32_t)( c24        & 0xFF);

                    /* Apply FS error */
                    int32_t r8 = _ARC_FS_CLAMP8(ir + (fs_err_cur[col*3+0] >> 4));
                    int32_t g8 = _ARC_FS_CLAMP8(ig + (fs_err_cur[col*3+1] >> 4));
                    int32_t b8 = _ARC_FS_CLAMP8(ib + (fs_err_cur[col*3+2] >> 4));

                    int32_t qr = _ARC_FS_QUANT5(r8);
                    int32_t qg = _ARC_FS_QUANT6(g8);
                    int32_t qb = _ARC_FS_QUANT5(b8);

                    GT_COLOR_SET_R(color_row_buf[col], r8 >> (GT_COLOR_DEPTH == 32 ? 0 : (GT_COLOR_DEPTH == 16 ? 3 : (GT_COLOR_DEPTH == 8 ? 5 : 7))));
                    GT_COLOR_SET_G(color_row_buf[col], g8 >> (GT_COLOR_DEPTH == 32 ? 0 : (GT_COLOR_DEPTH == 16 ? 2 : (GT_COLOR_DEPTH == 8 ? 5 : 7))));
                    GT_COLOR_SET_B(color_row_buf[col], b8 >> (GT_COLOR_DEPTH == 32 ? 0 : (GT_COLOR_DEPTH == 16 ? 3 : (GT_COLOR_DEPTH == 8 ? 6 : 7))));

                    /* Propagate FS error */
                    int32_t er = (r8 - qr) << 4;
                    int32_t eg = (g8 - qg) << 4;
                    int32_t eb = (b8 - qb) << 4;
                    _ARC_FS_ADD(fs_err_cur, (int32_t)(col+1)*3+0, (er*7)>>4);
                    _ARC_FS_ADD(fs_err_cur, (int32_t)(col+1)*3+1, (eg*7)>>4);
                    _ARC_FS_ADD(fs_err_cur, (int32_t)(col+1)*3+2, (eb*7)>>4);
                    _ARC_FS_ADD(fs_err_nxt, (int32_t)(col-1)*3+0, (er*3)>>4);
                    _ARC_FS_ADD(fs_err_nxt, (int32_t)(col-1)*3+1, (eg*3)>>4);
                    _ARC_FS_ADD(fs_err_nxt, (int32_t)(col-1)*3+2, (eb*3)>>4);
                    _ARC_FS_ADD(fs_err_nxt, (int32_t)(col  )*3+0, (er*5)>>4);
                    _ARC_FS_ADD(fs_err_nxt, (int32_t)(col  )*3+1, (eg*5)>>4);
                    _ARC_FS_ADD(fs_err_nxt, (int32_t)(col  )*3+2, (eb*5)>>4);
                    _ARC_FS_ADD(fs_err_nxt, (int32_t)(col+1)*3+0, (er*1)>>4);
                    _ARC_FS_ADD(fs_err_nxt, (int32_t)(col+1)*3+1, (eg*1)>>4);
                    _ARC_FS_ADD(fs_err_nxt, (int32_t)(col+1)*3+2, (eb*1)>>4);

                    /* Store per-pixel fade into mask_buf (will be AND-ed with shape mask) */
                    mask_buf[col] = pixel_opa;
                }
                _ARC_FS_SWAP();
                blend_dsc.dst_buf    = color_row_buf;
                blend_dsc.color_fill = gt_color_black();
            } else {
                blend_dsc.dst_buf    = NULL;
                blend_dsc.color_fill = gt_gradient_dsc_get_color_pos(dsc->grad, 32767);
                if (mask_buf) gt_memset(mask_buf, GT_OPA_COVER, row_w);
            }

            /* Apply shape masks (out_mask, in_mask, ang_mask) on top of fade mask */
            if (mask_buf) {
                if (shape_mask) {
                    gt_memset(shape_mask, dsc->opa, row_w);
                    gt_mask_res_et res = gt_mask_get(shape_mask, line_area.x, line_area.y, row_w);
                    if (res == GT_MASK_RES_TRANSP) {
                        blend_dsc.dst_buf = NULL;
                        continue;
                    }
                    if (res != GT_MASK_RES_FULL_COVER) {
                        /* Multiply fade mask with shape mask */
                        for (uint16_t col = 0; col < row_w; col++) {
                            mask_buf[col] = (gt_opa_t)(((uint32_t)mask_buf[col] * shape_mask[col]) >> 8);
                        }
                    }
                    blend_dsc.mask_buf = mask_buf;
                } else {
                    gt_memset(mask_buf, dsc->opa, row_w);
                    gt_mask_res_et res = gt_mask_get(mask_buf, line_area.x, line_area.y, row_w);
                    if (res == GT_MASK_RES_TRANSP) { blend_dsc.dst_buf = NULL; continue; }
                    blend_dsc.mask_buf = (res == GT_MASK_RES_FULL_COVER) ? NULL : mask_buf;
                }
            } else {
                blend_dsc.mask_buf = NULL;
            }

            gt_draw_blend(draw_ctx, &blend_dsc);
            blend_dsc.dst_buf = NULL;
        }

        #undef _ARC_FS_CLAMP8
        #undef _ARC_FS_QUANT5
        #undef _ARC_FS_QUANT6
        #undef _ARC_FS_ADD
        #undef _ARC_FS_SWAP

        if (color_row_buf) gt_mem_free(color_row_buf);
        if (mask_buf)      gt_mem_free(mask_buf);
        if (shape_mask)    gt_mem_free(shape_mask);
        if (fs_err_cur)    gt_mem_free(fs_err_cur);
        if (fs_err_nxt)    gt_mem_free(fs_err_nxt);

        if (ang_mask_id >= 0) { gt_mask_remove_idx(ang_mask_id); gt_mask_free(&ang_mask); }
        gt_mask_remove_idx(geo.out_mask_id); gt_mask_free(&geo.out_mask);
        if (geo.in_radius > 0) { gt_mask_remove_idx(geo.in_mask_id); gt_mask_free(&geo.in_mask); }

        if (!is_full_circle) {
            _arc_draw_caps_grad(draw_ctx, dsc, &geo, cx, cy, start_angle, end_angle);
        }
        return;
    }
#endif /* GT_USE_GRADIENT */

    _arc_draw_solid(draw_ctx, dsc, &geo, &rect_attr, is_img);
}

#if GT_USE_IMG_TRANSFORM
void draw_bg_img_transform(_gt_draw_ctx_st * draw_ctx, const gt_attr_rect_st * dsc,
                           gt_area_st * coords, uint16_t angle, uint16_t zoom,
                           int16_t pivot_x, int16_t pivot_y)
{
    if (dsc->bg_opa < GT_OPA_MIN) return;
    if (NULL == draw_ctx->buf) return;

    _gt_img_dsc_st dsc_img = {0};

    /* Open image source (same logic as draw_bg_img) */
    if (NULL == dsc->bg_img_src) {
        if (dsc->raw_img) {
            if (dsc->raw_img->raw_p && false == dsc->reg.ignore_img_decoder) {
                if (GT_RES_FAIL == gt_img_decoder_open_raw(&dsc_img, dsc->raw_img->raw_p, dsc->raw_img->raw_len)) {
                    return;
                }
            } else {
                if (GT_IMG_DECODER_TYPE_NONE == dsc->raw_img->header.type && NULL == dsc->raw_img->fp) {
                    return;
                }
                dsc_img = *dsc->raw_img;
            }
            goto handler_lb;
        }
#if GT_USE_FILE_HEADER
        {
            gt_res_t r = gt_img_decoder_fh_open(&dsc_img, dsc->file_header);
            if (GT_RES_OK == r) goto handler_lb;
            if (GT_RES_FAIL == r) return;
        }
#endif
#if GT_USE_DIRECT_ADDR
        {
            gt_res_t r = gt_img_decoder_direct_addr_open(&dsc_img, dsc->addr);
            if (GT_RES_OK == r) goto handler_lb;
            if (GT_RES_FAIL == r) return;
        }
#endif
#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
        {
            gt_res_t r = gt_img_decoder_custom_size_addr_open(&dsc_img, dsc->custom_addr);
            if (GT_RES_OK == r) goto handler_lb;
            if (GT_RES_FAIL == r) return;
        }
#endif
        return;
    } else if (GT_RES_FAIL == gt_img_decoder_open(&dsc_img, dsc->bg_img_src)) {
        return;
    }

handler_lb:
    _gt_disp_update_max_area(coords, true);
    if (NULL == draw_ctx->buf) goto close_lb;

    /* Resolve pivot: -1 means image center */
    int16_t px = (pivot_x < 0) ? (int16_t)(dsc_img.header.w / 2) : pivot_x;
    int16_t py = (pivot_y < 0) ? (int16_t)(dsc_img.header.h / 2) : pivot_y;

    gt_area_st dst_area = *coords, mask_area = *coords;
    gt_draw_blend_dsc_st blend_dsc = {
        .dst_buf    = NULL,
        .mask_buf   = NULL,
        .color_fill = dsc->raw_img ? dsc->raw_img->fill_color : gt_color_black(),
        .dst_area   = &dst_area,
        .mask_area  = &mask_area,
        .masking    = dsc->masking,
        .view_area  = dsc->limit_area,
        .opa        = dsc->bg_opa,
    };

    gt_img_transform_draw(draw_ctx, &blend_dsc, &dsc_img, angle, zoom, px, py);

    if (GT_IMG_DECODER_TYPE_RAM == dsc_img.header.type) return;
close_lb:
#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
    if (dsc->custom_addr) {
        gt_img_decoder_custom_size_addr_close(&dsc_img);
        return;
    }
#endif
    gt_img_decoder_close(&dsc_img);
}
#endif  /** GT_USE_IMG_TRANSFORM */

GT_ATTRIBUTE_RAM_TEXT void gt_draw_polygon(
    _gt_draw_ctx_st * draw_ctx,
    gt_attr_polygon_st const * dsc,
    gt_area_st const * coords)
{
    if (NULL == draw_ctx->buf) return;
    if (NULL == dsc->points || dsc->count < 3) return;
    if (!gt_area_is_intersect_screen(&draw_ctx->buf_area, coords)) return;

    uint16_t const n = dsc->count;
    gt_point_st const * pts = dsc->points;

    /* pts[] are stored as bbox-relative coordinates (origin = widget's bbox corner).
     * coords->x/y is the widget's current obj->area.x/y (updated by scroll).
     * Adding coords->x/y converts relative vertices to current screen position.
     * This avoids the (coords->x - bbox_x) subtraction which oscillated ±1px
     * during easing animation, causing scanline jitter on near-horizontal edges. */
    int16_t const ox = (int16_t)coords->x;
    int16_t const oy = (int16_t)coords->y;

    /* ── Fill via scanline ── */
    if (dsc->fill_opa > GT_OPA_MIN) {
        int16_t y_min = (int16_t)(pts[0].y + oy), y_max = (int16_t)(pts[0].y + oy);
        for (uint16_t i = 1; i < n; i++) {
            int16_t iy = (int16_t)(pts[i].y + oy);
            if (iy < y_min) y_min = iy;
            if (iy > y_max) y_max = iy;
        }

        int16_t scan_y0 = GT_MAX(y_min, (int16_t)draw_ctx->buf_area.y);
        int16_t scan_y1 = GT_MIN(y_max,
            (int16_t)(draw_ctx->buf_area.y + (int16_t)draw_ctx->buf_area.h - 1));

        int16_t x_buf[GT_POLYGON_MAX_VERTICES];

        gt_area_st fill_line = { .h = 1 };
        gt_draw_blend_dsc_st blend_dsc = { 0 };
        blend_dsc.color_fill = dsc->fill_color;
        blend_dsc.opa        = dsc->fill_opa;
        blend_dsc.mask_buf   = NULL;
        blend_dsc.dst_area   = &fill_line;
        blend_dsc.mask_area  = &fill_line;
        blend_dsc.view_area  = NULL;

        for (int16_t y = scan_y0; y <= scan_y1; y++) {
            uint16_t cnt = 0;
            for (uint16_t i = 0; i < n; i++) {
                uint16_t j = (i + 1) % n;
                int16_t y0e = (int16_t)(pts[i].y + oy);
                int16_t y1e = (int16_t)(pts[j].y + oy);
                if (y0e == y1e) continue;
                int16_t lo = GT_MIN(y0e, y1e);
                int16_t hi = GT_MAX(y0e, y1e);
                if (y < lo || y >= hi) continue;
                if (cnt >= GT_POLYGON_MAX_VERTICES) break;
                int32_t xi = (int32_t)(pts[i].x + ox) +
                    (int32_t)(y - y0e) * (int32_t)(pts[j].x - pts[i].x)
                    / (int32_t)(y1e - y0e);
                x_buf[cnt++] = (int16_t)xi;
            }
            /* insertion sort */
            for (uint16_t a = 1; a < cnt; a++) {
                int16_t key = x_buf[a];
                int16_t b = (int16_t)(a - 1);
                while (b >= 0 && x_buf[b] > key) {
                    x_buf[b + 1] = x_buf[b];
                    b--;
                }
                x_buf[b + 1] = key;
            }
            /* fill pairs */
            for (uint16_t p = 0; p + 1 < cnt; p += 2) {
                int16_t xl = x_buf[p];
                int16_t xr = x_buf[p + 1];
                if (xl >= xr) continue;
                xl = GT_MAX(xl, (int16_t)draw_ctx->buf_area.x);
                xr = GT_MIN(xr, (int16_t)(draw_ctx->buf_area.x + (int16_t)draw_ctx->buf_area.w - 1));
                if (xl > xr) continue;
                fill_line.x = xl;
                fill_line.y = y;
                fill_line.w = (uint16_t)(xr - xl + 1);
                gt_draw_blend(draw_ctx, &blend_dsc);
            }
        }
    }

    /* ── Stroke via gt_draw_line ── */
    if (dsc->line.width > 0 && dsc->line.opa > GT_OPA_MIN) {
        gt_attr_line_st edge;
        gt_memset(&edge, 0, sizeof(edge));
        edge.line = dsc->line;
        for (uint16_t i = 0; i < n; i++) {
            uint16_t j = (i + 1) % n;
            edge.start.x = (int16_t)(pts[i].x + ox);
            edge.start.y = (int16_t)(pts[i].y + oy);
            edge.end.x   = (int16_t)(pts[j].x + ox);
            edge.end.y   = (int16_t)(pts[j].y + oy);
            gt_draw_line(draw_ctx, &edge, coords);
        }
    }
}

/* end ------------------------------------------------------------------*/
