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

#if GT_USE_LIB_TEX_PRINTF
#include "../extra/libtexprintf/gt_tex.h"
#endif

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

static GT_ATTRIBUTE_RAM_TEXT uint16_t _gt_gray_and_dot_byte_get(uint8_t* gray, uint8_t width, gt_font_st font, uint8_t ret_style, uint32_t unicode) {
    uint16_t dot_byte = 0;
    uint8_t font_type = ret_style % 10;
    uint8_t font_gray = (ret_style/10) > 0 ? (ret_style/10) : 1;

    dot_byte = gt_font_get_one_word_dot_width(unicode, &font);
    if (dot_byte){

        if (GT_FONT_TYPE_FLAG_VEC == font_type){
            font_gray = font.info.gray;
        }

        dot_byte = ((dot_byte + 7) >> 3) * font_gray;
        *gray = font_gray;
        return dot_byte;
    }

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

static GT_ATTRIBUTE_RAM_TEXT inline uint8_t _get_style_space_y(const gt_font_info_st* font_info) {

    if(font_info->style.reg.underline || font_info->style.reg.strikethrough) {
        return (font_info->size >> 4) ? (font_info->size >> 4) : 1;
    }
    return 0;
}

static GT_ATTRIBUTE_RAM_TEXT inline uint8_t _get_style_bold_offset_x(const gt_font_info_st* font_info)
{
    return font_info->style.reg.bold ? 1 : 0;
}

static GT_ATTRIBUTE_RAM_TEXT inline uint8_t _get_style_space_x( uint8_t org_space_x , const gt_font_info_st* font_info)
{
    return org_space_x + _get_style_bold_offset_x(font_info);
}

static GT_ATTRIBUTE_RAM_TEXT inline void _draw_blend_underline(
    _gt_draw_ctx_st * draw_ctx, const gt_attr_font_st * dsc, gt_draw_blend_dsc_st * blend_dsc_style,
    gt_area_st area_font_style, const gt_font_info_st* font_info, uint16_t width, uint16_t height) {
    area_font_style.y += font_info->size + 1;
    area_font_style.w = width;
    area_font_style.h = height;
    blend_dsc_style->color_fill = dsc->font_color;
    blend_dsc_style->dst_area = &area_font_style;

    area_font_style.w += _get_style_bold_offset_x(font_info);
    if(font_info->style.reg.italic){
        area_font_style.w += font_info->size >> 1;
    }

    gt_draw_blend(draw_ctx, blend_dsc_style);
}

static GT_ATTRIBUTE_RAM_TEXT inline void _draw_blend_strikethrough(
    _gt_draw_ctx_st * draw_ctx, const gt_attr_font_st * dsc, gt_draw_blend_dsc_st * blend_dsc_style,
    gt_area_st area_font_style, const gt_font_info_st* font_info, uint16_t width, uint16_t height) {
    area_font_style.y += (font_info->size >> 1);
    area_font_style.w = width;
    area_font_style.h = height;
    blend_dsc_style->color_fill = dsc->font_color;
    blend_dsc_style->dst_area = &area_font_style;

    area_font_style.w += _get_style_bold_offset_x(font_info);
    if(font_info->style.reg.italic){
        area_font_style.w += font_info->size >> 1;
    }

    gt_draw_blend(draw_ctx, blend_dsc_style);
}


#if _GT_FONT_ENABLE_CONVERTOR
static GT_ATTRIBUTE_RAM_TEXT _gt_draw_font_res_st _draw_text_convertor_split(
    const gt_font_st * fonts, uint8_t lan,
    _gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st * blend_dsc,
    const gt_attr_font_st * dsc, gt_size_t font_disp_h,
    gt_draw_blend_dsc_st * blend_dsc_style, const gt_area_st * coords, uint8_t flag_ol) {
    uint32_t idx = 0, uni_or_gbk = 0, tmp = 0;
    _gt_draw_font_res_st ret = { .res = GT_RES_OK };
    uint16_t * text = NULL;
    uint8_t * font_buf = NULL;
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
        goto _buf_handle;
    }
    gt_memset_0(font_buf, font_size_res.font_buff_len);

    uint8_t style_space_y = _get_style_space_y((const gt_font_info_st*)&tmp_font.info);
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

    uint8_t space_x = _get_style_space_x(dsc->space_x , (const gt_font_info_st*)&tmp_font.info);

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
            // bold and italic
            if(tmp_font.info.style.reg.bold){
                blend_dsc->dst_area->w += _get_style_bold_offset_x((const gt_font_info_st*)&tmp_font.info);
            }

            gt_point_st off_point = gt_font_offset_get(font_option, uni_or_gbk);
            blend_dsc->dst_area->x += off_point.x;
            blend_dsc->dst_area->y += off_point.y;
            gt_draw_blend_text(draw_ctx, blend_dsc, tmp_font.info.size, byte_width, font_gray,
                                &convertor.data[idx * font_size_res.font_per_size], tmp_font.info.style.reg);

            blend_dsc->dst_area->x -= off_point.x;
            blend_dsc->dst_area->y -= off_point.y;


        } else {
            byte_width = (gt_font_get_one_word_width(convertor.fontcode[idx], &tmp_font) + 7) >> 3;
            byte_width = _gt_gray_and_dot_byte_get(&font_gray, byte_width, tmp_font, ret_style, uni_or_gbk);
            blend_dsc->dst_area->w = byte_width << 3;
            // bold and italic
            if(tmp_font.info.style.reg.bold){
                blend_dsc->dst_area->w += _get_style_bold_offset_x((const gt_font_info_st*)&tmp_font.info);
            }

            gt_point_st off_point = gt_font_offset_get(font_option, uni_or_gbk);
            blend_dsc->dst_area->x += off_point.x;
            blend_dsc->dst_area->y += off_point.y;
            gt_draw_blend_text(draw_ctx, blend_dsc, tmp_font.info.size, byte_width, font_gray, &convertor.data[0],tmp_font.info.style.reg);
            blend_dsc->dst_area->x -= off_point.x;
            blend_dsc->dst_area->y -= off_point.y;

        }

        /* use style */
        if (tmp_font.info.style.reg.underline && font_disp_h == tmp_font.info.size) {
            _draw_blend_underline(draw_ctx, dsc, blend_dsc_style, *blend_dsc->dst_area,
                                    (const gt_font_info_st*)&tmp_font.info, width, style_space_y);
        }
        if (tmp_font.info.style.reg.strikethrough && (font_disp_h > ((tmp_font.info.size >> 1) + (style_space_y << 1)))) {
            _draw_blend_strikethrough(draw_ctx, dsc, blend_dsc_style, *blend_dsc->dst_area,
                                        (const gt_font_info_st*)&tmp_font.info, width, style_space_y);
        }
    _next_word_cvt_lb:
        if (_is_align_reverse(dsc->align)) {
            blend_dsc->dst_area->x -= space_x;
        } else {
            blend_dsc->dst_area->x += width + space_x;
        }
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
static GT_ATTRIBUTE_RAM_TEXT void _filling_by_sjpg_file(_gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st * blend_dsc, _gt_img_dsc_st * dsc_img) {
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

static GT_ATTRIBUTE_RAM_TEXT void _filling_by_ram_data(_gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st * blend_dsc, _gt_img_dsc_st * dsc_img) {
    blend_dsc->mask_buf = &dsc_img->alpha[0];
    blend_dsc->dst_buf = (gt_color_t *)&dsc_img->img[0];
    if (NULL == blend_dsc->dst_buf && NULL == blend_dsc->mask_buf) {
        GT_LOGW(GT_LOG_TAG_GUI, "Raw data buffer or mask is NULL");
        return;
    }

    bool check_mask = gt_mask_check(blend_dsc->mask_area);
    if(check_mask){
        gt_opa_t* alpha = NULL;
        if(dsc_img->alpha) {
            alpha = &dsc_img->alpha[0];
        }
        uint16_t w, h, y = 0;
        uint32_t offset = 0, len = 0;
        w = dsc_img->header.w;
        h = dsc_img->header.h;
        len = w * sizeof(gt_opa_t);

        blend_dsc->mask_buf = gt_mem_malloc(w);
        if (!blend_dsc->mask_buf) {
            GT_LOGE(GT_LOG_TAG_GUI, "Mask buffer malloc failed, size: %d, %d", w, h);
            return;
        }

        gt_opa_t opa = blend_dsc->opa >= GT_OPA_MAX ? GT_OPA_COVER: blend_dsc->opa;
        blend_dsc->dst_area->h = 1;
        while (y < h) {
            if (gt_area_is_intersect_screen(&draw_ctx->buf_area, blend_dsc->dst_area)) {

                if (alpha) {
                    gt_memcpy(blend_dsc->mask_buf, &alpha[offset], w);
                }
                else{
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
        return;
    }

    gt_draw_blend(draw_ctx, blend_dsc);
    blend_dsc->dst_buf = NULL;
    blend_dsc->mask_buf = NULL;
}

#if GT_USE_EXTRA_FULL_IMG_BUFFER
static GT_ATTRIBUTE_RAM_TEXT gt_res_t _filling_by_default_full_mode(_gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st * blend_dsc, _gt_img_dsc_st * dsc_img) {
    uint32_t len = 0, addr_start_alpha = 0;

    len = dsc_img->header.w * dsc_img->header.h;
    addr_start_alpha = len * sizeof(gt_color_t);

    gt_fs_fp_st * fp = dsc_img->fp;
    uint8_t is_mask = 1 == fp->msg.pic.is_alpha ? 1 : 0;
    bool check_mask = gt_mask_check(blend_dsc->mask_area);

    if (!fp) {
        GT_LOGW(GT_LOG_TAG_GUI, "gt_fs_get_fp failed");
        return GT_RES_FAIL;
    }

    _gt_gc_full_img_buffer_st * full_img = _gt_gc_get_full_img_buffer();
    if (NULL == full_img->buffer) {
        GT_LOGV(GT_LOG_TAG_GUI, "Extra buffer color or alpha pointer is NULL");
        return GT_RES_INV;
    }
    if (full_img->len < ((is_mask || check_mask) ? len : 0) + addr_start_alpha) {
        GT_LOGV(GT_LOG_TAG_GUI, "Extra buffer color or alpha size is not enough to store image data");
        return GT_RES_INV;
    }

    gt_fs_read_img_offset(fp, (uint8_t *)full_img->buffer, 0, (is_mask ? len : 0) + addr_start_alpha);

#if GT_USE_EXTRA_FULL_IMG_OFFSET
    blend_dsc->dst_buf = (gt_color_t * )&full_img->buffer[full_img->offset];
#else
    blend_dsc->dst_buf = (gt_color_t * )full_img->buffer;
#endif

    if (is_mask || check_mask) {
#if GT_USE_EXTRA_FULL_IMG_OFFSET
        blend_dsc->mask_buf = (gt_opa_t * )&full_img->buffer[addr_start_alpha + full_img->offset];
#else
        blend_dsc->mask_buf = (gt_opa_t * )&full_img->buffer[addr_start_alpha];
#endif
    }

    if(check_mask) {
        gt_opa_t opa = blend_dsc->opa >= GT_OPA_MAX ? GT_OPA_COVER: blend_dsc->opa;
        uint8_t *opa_buf = blend_dsc->mask_buf;
        for(uint32_t i = 0; i < dsc_img->header.h; ++i) {
            if(!is_mask) {
                gt_memset(opa_buf, opa, dsc_img->header.w);
            }
            gt_mask_get(opa_buf, blend_dsc->mask_area->x, blend_dsc->mask_area->y + i, dsc_img->header.w);
            opa_buf += dsc_img->header.w;
        }
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
static GT_ATTRIBUTE_RAM_TEXT void _filling_by_default_line_mode(_gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st * blend_dsc, _gt_img_dsc_st * dsc_img) {
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
#if _GT_DRAW_STATIC_LINE_BUFFER
    blend_dsc->dst_buf = _static_line_color_buffer;
#else
    blend_dsc->dst_buf = gt_mem_malloc( len );
    if (!blend_dsc->dst_buf) {
        GT_LOGW(GT_LOG_TAG_GUI, "Per-line buffer malloc failed, size: %d, %d", w, h);
        return;
    }
#endif

    gt_opa_t opa = blend_dsc->opa >= GT_OPA_MAX ? GT_OPA_COVER: blend_dsc->opa;
    bool check_mask = gt_mask_check(blend_dsc->mask_area);
    uint8_t is_mask = 1 == fp->msg.pic.is_alpha ? 1 : 0;

    if (is_mask || check_mask) {
#if _GT_DRAW_STATIC_LINE_BUFFER
        blend_dsc->mask_buf = _static_line_opa_buffer;
#else
        blend_dsc->mask_buf = gt_mem_malloc(w);
        if (!blend_dsc->mask_buf) {
            goto line_lb;
        }
#endif
    }

    while (y < h) {
        if (gt_area_is_intersect_screen(&draw_ctx->buf_area, blend_dsc->dst_area)) {
            /*read one line color data*/
            gt_fs_read_img_offset(fp, (uint8_t *)blend_dsc->dst_buf, offset, len);

            if(!is_mask && check_mask){
                gt_memset(blend_dsc->mask_buf, opa, w);
            }

            if (is_mask) {    //png
                gt_fs_read_img_offset(fp, (uint8_t *)blend_dsc->mask_buf, addr_start_alpha, w);
            }

            if(check_mask) {
                gt_mask_get(blend_dsc->mask_buf, blend_dsc->mask_area->x, blend_dsc->mask_area->y, w);
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

#if _GT_DRAW_STATIC_LINE_BUFFER
    blend_dsc->mask_buf = NULL;
    blend_dsc->dst_buf = NULL;
#else
    if (blend_dsc->mask_buf) {
        gt_mem_free(blend_dsc->mask_buf);
        blend_dsc->mask_buf = NULL;
    }
line_lb:
    if (blend_dsc->dst_buf) {
        gt_mem_free(blend_dsc->dst_buf);
        blend_dsc->dst_buf = NULL;
    }
#endif
}

static GT_ATTRIBUTE_RAM_TEXT inline void _filling_by_default(_gt_draw_ctx_st * draw_ctx,
    gt_draw_blend_dsc_st * blend_dsc, _gt_img_dsc_st * dsc_img) {

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

    uint32_t addr_start_alpha = dsc_img->header.w * dsc_img->header.h * sizeof(gt_color_t);
    uint8_t is_mask = 1 == fp->msg.pic.is_alpha ? 1 : 0;
    blend_dsc->dst_buf = (gt_color_t*)fp->buffer;
    if(is_mask){
        blend_dsc->mask_buf = (gt_opa_t*)fp->buffer + addr_start_alpha;
    }

    if(NULL == blend_dsc->dst_buf && NULL == blend_dsc->mask_buf){
        GT_LOGW(GT_LOG_TAG_GUI, "Extra buffer color or alpha pointer is NULL");
        return ;
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

static inline bool _is_multi_line(const gt_attr_font_st * dsc) {
    return false == dsc->reg.single_line ? true : false;
}

static GT_ATTRIBUTE_RAM_TEXT gt_size_t _get_align_start_y_by_line(const gt_attr_font_st * dsc,
    const gt_area_st * coords, gt_font_st * tmp_font, uint8_t * txt, uint16_t line_count) {
    gt_size_t offset_y = coords->y;
    uint16_t line_height = (tmp_font->info.size + (_get_style_space_y((const gt_font_info_st*)&tmp_font->info) << 1) + dsc->space_y) * line_count;
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

static GT_ATTRIBUTE_RAM_TEXT _text_offset_line_st _get_align_start_y(const gt_attr_font_st * dsc,
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

static GT_ATTRIBUTE_RAM_TEXT uint16_t _calc_line_height_pixel(gt_area_st * area_font, const gt_attr_font_st * dsc, uint8_t style_space_y) {
    return area_font->h + dsc->space_y + (style_space_y << 1);
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

static GT_ATTRIBUTE_RAM_TEXT _gt_draw_font_res_st
draw_text_multiple_line(_gt_draw_ctx_st * draw_ctx, const gt_attr_font_st * dsc, gt_area_st const * view) {
    const gt_area_st * coords = &dsc->logical_area;
    _gt_draw_font_res_st ret = { .res = GT_RES_OK };
    gt_area_st area_font = {
        .x = coords->x, .y = coords->y, .w = dsc->font->info.size, .h = dsc->font->info.size
    };
    gt_font_st tmp_font = {
        .info = dsc->font->info,
        .utf8 = dsc->font->utf8,
        .len = dsc->font->len,
        .res = NULL,
    };
    tmp_font.info.gray = dsc->font->info.gray == 0 ? 1 : dsc->font->info.gray;
    int8_t offset_y = _font_get_group_offset_by_type(&tmp_font);
    gt_size_t font_disp_h = tmp_font.info.size;
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(&tmp_font.info, 2, 0, 0);
#else
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(&tmp_font.info, FONT_LAN_UNKNOWN, 0);
#endif
    uint16_t data_len = font_size_res.font_per_size;
    uint8_t * mask_line = NULL;

#if GT_USE_EXTRA_FULL_FONT_BUFFER
    gt_buffer_st * gc_font_p = _gt_gc_get_full_font_buffer();
    bool is_extend_font_valid = _gt_gc_full_font_buffer_is_valid() && (data_len < gc_font_p->len);
#endif

    tmp_font.res =
#if GT_USE_EXTRA_FULL_FONT_BUFFER
        is_extend_font_valid ? gc_font_p->ptr :
#endif
        gt_mem_malloc(data_len);

    if (NULL == tmp_font.res) {
        GT_LOGE(GT_LOG_TAG_GUI, "data malloc is failed!!! size = %d", data_len);
        goto _res_handler;
    }
#if GT_USE_EXTRA_FULL_FONT_MASK_BUFFER
    gt_buffer_st * gc_font_mask_p = _gt_gc_get_full_font_mask_buffer();
    bool is_extend_font_mask_valid = _gt_gc_full_font_mask_buffer_is_valid() &&
                                    ((tmp_font.info.size << 1) < gc_font_mask_p->len);
#endif

    mask_line =
#if GT_USE_EXTRA_FULL_FONT_MASK_BUFFER
        is_extend_font_mask_valid ? gc_font_mask_p->ptr :
#endif
        gt_mem_malloc( tmp_font.info.size << 1 );

    if (NULL == mask_line) {
        GT_LOGE(GT_LOG_TAG_GUI, "mask_line malloc is failed!!! size = %d", tmp_font.info.size << 1);
        goto _mask_handler;
    }
    gt_memset(mask_line, 0xFF, tmp_font.info.size << 1);

    uint8_t * txt = (uint8_t * )dsc->font->utf8;
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
    uint8_t lan = right_to_left_lan_get(dsc->font->info.style_fl);
#else
    uint8_t lan = right_to_left_lan_get(&tmp_font);
#endif

    uint16_t txt_len = dsc->font->len;
    // lib tex printf
#if GT_USE_LIB_TEX_PRINTF
    char* latex_txt = NULL;
    if(dsc->reg.use_latex) {
        latex_txt = gt_tex_string_by_len(txt, txt_len);
        if(latex_txt) {
            txt = (uint8_t*)latex_txt;
            txt_len = strlen(latex_txt);
        }
    }
#endif

#if _GT_FONT_ENABLE_CONVERTOR
    uint8_t * ret_txt = NULL;
    if (lan == FONT_LAN_HEBREW || lan == FONT_LAN_ARABIC) {
        ret_txt = gt_mem_malloc(txt_len+1);
        if (NULL == ret_txt) {
            GT_LOGE(GT_LOG_TAG_GUI, "ret_txt malloc is failed!!! size = %d", txt_len+1);
            goto _txt_handler;
        }
        gt_memset_0(ret_txt, txt_len+1);
        if (gt_right_to_left_handler(&tmp_font, ret_txt, lan)) {
            txt = ret_txt;
        }
    }
#endif
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
    uint16_t per_line_h = (style_space_y << 1) + dsc->font->info.size + dsc->space_y;

    gt_size_t disp_w = coords->w;
    uint32_t uni_or_gbk = 0, ret_w = 0;
    uint16_t idx = 0, idx_2 = 0, idx_len = 0;
    uint8_t over_length = 0;
    int32_t idx_step = 0;

#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
    gt_font_touch_word_st tmp_word = {0};
    gt_font_lan_et prev_sty = FONT_LAN_UNKNOWN;
    gt_font_lan_et cur_sty = FONT_LAN_UNKNOWN;
#endif
    bool cov_flag_ol = false;
    bool start_x_flag = dsc->reg.enabled_start;
    bool is_first_line = true;
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
    _text_offset_line_st off_line = _get_align_start_y(dsc, coords, &tmp_font, txt, txt_len);
    area_font.y = off_line.offset_y;
    if (dsc->reg.enabled_start) {
        area_font.y = dsc->start_y;
    }
    // disp
    if (start_x_flag) {
        /** have direct position do not indent */
        disp_w = coords->w - (dsc->start_x - coords->x);
        is_first_line = false;
    } else if (1 == off_line.line_count && _is_align_center(dsc->align)) {
        /** center single line do not indent */
        is_first_line = false;
    } else if (is_first_line) {
        disp_w -= gt_font_get_indent_width(&dsc->font->info, dsc->indent);
    }

    while (idx < txt_len) {
        // out of range
        if ( (area_font.y + style_space_y + area_font.h) > (coords->y + coords->h) ) {
            font_disp_h = (coords->y + coords->h) - area_font.y;
            if(font_disp_h < 2){
                break;
            }
        }
        if(0x0A == txt[idx]){
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

        ret_w = coords->w - disp_w;
#if _GT_FONT_ENABLE_CONVERTOR
        idx_step = gt_font_split_line_str(&tmp_font, coords->w, space_x, &ret_w,
                                            &bidi, &bidi_len, &bidi_max, &over_length,
                                            is_first_line);
#else
        idx_step = gt_font_split_line_str(&tmp_font, coords->w, space_x, &ret_w,
                                            NULL, NULL, NULL, &over_length,
                                            is_first_line);
#endif
        if(idx_step < 0){
            goto _ret_handler;
        }
        else if(idx_step == 0 && coords->w != disp_w){
            if(ret_w == 0 ) {
                if(coords->w >= tmp_font.info.size) {
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
            area_font.x = _get_align_start_x(dsc->align, coords->x, disp_w, 0);
        }
        if (_is_align_reverse(dsc->align) && !cov_flag_ol) {
            area_font.x = (coords->x + coords->w) - (area_font.x - coords->x);
        }
        if (start_x_flag) {
            area_font.x = dsc->start_x;
            start_x_flag = false;
        }
        if (is_first_line) {
            is_first_line = false;
            if (_is_align_reverse(dsc->align)) {
                area_font.x -= gt_font_get_indent_width(&dsc->font->info, dsc->indent);
            } else {
                area_font.x += gt_font_get_indent_width(&dsc->font->info, dsc->indent);
            }
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
        if (ret.size_show_limit.x < ret_w) {
            ret.size_show_limit.x = ret_w;
        }
        ret.size_show_limit.y += line_height;
        if (ret.size_show_limit.y > view->h) {
            ret.size_show_limit.y = view->h;
        }
        if (dsc->reg.only_calc_display_info) { goto next_line_lb; }

        /** ------------- Display current line chars ------------- */
        while (idx_2 < idx_len) {
            tmp_font.info.size = dsc->font->info.size;
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
            if (dsc->opa < GT_OPA_MIN) {
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

                if (GT_FONT_TYPE_FLAG_EMOJI == dot_ret.type) {
                    gt_area_st emoji_area = area_font;
                    emoji_area.h = 1;

                    _gt_img_dsc_st emoji_raw = {0};
                    emoji_raw.img = (uint8_t*)tmp_font.res;
                    emoji_raw.alpha = (uint8_t*)mask_line;
                    emoji_raw.header.w = width;
                    emoji_raw.header.h = 1;
                    emoji_raw.header.type = GT_IMG_DECODER_TYPE_RAM;
                    emoji_raw.header.color_format = GT_IMG_CF_TRUE_COLOR;

                    gt_attr_rect_st dsc_emoji = {0};
                    dsc_emoji.bg_opa = dsc->opa;
                    dsc_emoji.raw_img = &emoji_raw;
                    dsc_emoji.limit_area = (gt_area_st*)view;

                    uint8_t emoji_h = gt_font_get_one_word_height(uni_or_gbk, &tmp_font);
                    emoji_area.y += ((tmp_font.info.size - emoji_h) >> 1);

                    for(uint8_t i = 0; i < emoji_h; ++i){

                        int emj_ret = gt_font_get_emoji_dot(uni_or_gbk, dot_ret.option, emoji_raw.img, emoji_raw.alpha, i);
                        if(emj_ret <= 0) {
                            GT_LOGD(GT_LOG_TAG_GUI, "get emoji dot failed!!! ret: %d",emj_ret);
                            break;
                        }
                        draw_bg_img(draw_ctx, &dsc_emoji, &emoji_area);
                        ++emoji_area.y;
                    }
                    gt_memset(mask_line, 0xFF, tmp_font.info.size << 1);
                }
                else{
                    uint8_t byte_width = (width + 7) >> 3;
                    uint8_t font_gray = 1;
                    uint8_t font_height = gt_font_get_one_word_height(uni_or_gbk, &tmp_font);
                    byte_width = _gt_gray_and_dot_byte_get(&font_gray, width, tmp_font, dot_ret.type, uni_or_gbk);
                    area_font.w = (byte_width / font_gray) << 3;
                    if (FONT_LAN_ASCII == gt_font_lan_get(uni_or_gbk, tmp_font.info.encoding)) {
                        area_font.y += offset_y;
                    }

                    // bold
                    if(tmp_font.info.style.reg.bold){
                        area_font.w += _get_style_bold_offset_x((const gt_font_info_st*)&tmp_font.info);
                    }

                    area_font.y += ((tmp_font.info.size - font_height) >> 1);

                    gt_point_st off_point = gt_font_offset_get(dot_ret.option, uni_or_gbk);
                    area_font.x += off_point.x;
                    area_font.y += off_point.y;
                    gt_draw_blend_text(draw_ctx, &blend_dsc, tmp_font.info.size,
                                        byte_width, font_gray, (const uint8_t*)tmp_font.res,tmp_font.info.style.reg);

                    area_font.x -= off_point.x;
                    area_font.y -= off_point.y;

                    area_font.y -= ((tmp_font.info.size - font_height) >> 1);

                    if (FONT_LAN_ASCII == gt_font_lan_get(uni_or_gbk, tmp_font.info.encoding)) {
                        area_font.y -= offset_y;
                    }
                }
            } else {
                uint8_t symbol_size = gt_symbol_get_size(tmp_font.info.size);

                if (symbol_size != tmp_font.info.size) {
                    gt_memset(tmp_font.res, 0, data_len);
                }
                uint8_t * temp = (uint8_t*)gt_symbol_get_mask_buf(uni_or_gbk, symbol_size);
                if( temp == NULL ){
                    area_font.x += width + space_x;
                    idx_2 += tmp_font.len;
                    continue;
                }

                gt_size_t off = (tmp_font.info.size - symbol_size) >> 1;
                area_font.y += off;
                blend_dsc.mask_buf = temp;
                area_font.w = symbol_size;
                area_font.h = symbol_size;
                gt_draw_blend(draw_ctx, &blend_dsc);
                area_font.h = font_disp_h;
                area_font.y -= off;
                blend_dsc.mask_buf = NULL;
            }

            /* use style */
            if (tmp_font.info.style.reg.underline && font_disp_h == tmp_font.info.size) {
                _draw_blend_underline(draw_ctx, dsc, &blend_dsc_style, area_font,
                                        (const gt_font_info_st*)&tmp_font.info, width, style_space_y);
            }
            if (tmp_font.info.style.reg.strikethrough && (font_disp_h > ((tmp_font.info.size >> 1) + (style_space_y << 1)))) {
                _draw_blend_strikethrough(draw_ctx, dsc, &blend_dsc_style, area_font,
                                            (const gt_font_info_st*)&tmp_font.info, width, style_space_y);
            }
        next_word_multi_lb:
            if (_is_align_reverse(dsc->align)) {
                area_font.x -= space_x;
            } else {
                area_font.x += width + space_x;
            }
            idx_2 += tmp_font.len;
        }

    next_line_lb:
#if _GT_FONT_ENABLE_CONVERTOR
        // if (_gt_font_is_convertor_language(lan) && over_length > 1) {
        if(bidi_len > 0 && over_length > 1){
            cov_flag_ol = true;
            if (_is_align_reverse(dsc->align)) {
                disp_w = area_font.x - coords->x;
            } else {
                disp_w = coords->w - (area_font.x - coords->x);
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
        if (0x0A == txt[idx - 1]) {
            is_first_line = true;
            disp_w -= gt_font_get_indent_width(&tmp_font.info, dsc->indent);
        } /** need to indent */
        idx_len = 0;
        txt_2 = &txt[idx];
        font_disp_h = dsc->font->info.size;
        area_font.h = font_disp_h;
        area_font.w = dsc->font->info.size;
    }

    if(tmp_font.info.style.reg.italic){
        area_font.x += tmp_font.info.size >> 1;
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
_txt_handler:
    if (
#if GT_USE_EXTRA_FULL_FONT_MASK_BUFFER
        false == is_extend_font_mask_valid &&
#endif
        mask_line) {
        gt_mem_free(mask_line);
    }
    mask_line = NULL;

#if GT_USE_LIB_TEX_PRINTF
    if(latex_txt) {
        gt_tex_string_free(latex_txt);
        latex_txt = NULL;
    }
#endif

_mask_handler:
    if (
#if GT_USE_EXTRA_FULL_FONT_BUFFER
        false == is_extend_font_valid &&
#endif
        tmp_font.res) {
        gt_mem_free(tmp_font.res);
    }
    tmp_font.res = NULL;

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

static GT_ATTRIBUTE_RAM_TEXT _gt_draw_font_res_st
draw_text_single_line(_gt_draw_ctx_st * draw_ctx, const gt_attr_font_st * dsc, const gt_area_st * view) {
    gt_area_st * coords = (gt_area_st * )&dsc->logical_area;
    _gt_draw_font_res_st ret = {
        .res = GT_RES_OK,
        .size_show_limit = {
            .x = 0, .y = dsc->font->info.size,
        },
    };
    gt_area_st area_font = {
        .x = coords->x, .y = coords->y, .w = dsc->font->info.size, .h = dsc->font->info.size
    };
    gt_font_st tmp_font = {
        .info = dsc->font->info,
        .utf8 = dsc->font->utf8,
        .len = dsc->font->len,
        .res = NULL,
    };
    tmp_font.info.gray = dsc->font->info.gray == 0 ? 1 : dsc->font->info.gray;
    int8_t offset_y = _font_get_group_offset_by_type(&tmp_font);
    gt_size_t font_disp_h = tmp_font.info.size;
#if (defined(GT_FONT_FAMILY_OLD_ENABLE) && (GT_FONT_FAMILY_OLD_ENABLE == 1))
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(&tmp_font.info, 2, 0, 0);
#else
    _gt_font_size_res_st font_size_res = gt_font_get_size_length_by_style(&tmp_font.info, FONT_LAN_UNKNOWN, 0);
#endif
    uint16_t data_len = font_size_res.font_per_size;
    uint8_t * mask_line = NULL;

#if GT_USE_EXTRA_FULL_FONT_BUFFER
    gt_buffer_st * gc_font_p = _gt_gc_get_full_font_buffer();
    bool is_extend_font_valid = _gt_gc_full_font_buffer_is_valid() && (data_len < gc_font_p->len);
#endif

    tmp_font.res =
#if GT_USE_EXTRA_FULL_FONT_BUFFER
        is_extend_font_valid ? gc_font_p->ptr :
#endif
        gt_mem_malloc(data_len);

    if (NULL == tmp_font.res) {
        GT_LOGE(GT_LOG_TAG_GUI, "data malloc is failed!!! size = %d", data_len);
        goto direct_handler_lb;
    }
#if GT_USE_EXTRA_FULL_FONT_MASK_BUFFER
    gt_buffer_st * gc_font_mask_p = _gt_gc_get_full_font_mask_buffer();
    bool is_extend_font_mask_valid = _gt_gc_full_font_mask_buffer_is_valid() &&
                                    ((tmp_font.info.size << 1) < gc_font_mask_p->len);
#endif

    mask_line =
#if GT_USE_EXTRA_FULL_FONT_MASK_BUFFER
        is_extend_font_mask_valid ? gc_font_mask_p->ptr :
#endif
        gt_mem_malloc( tmp_font.info.size << 1 );

    if (NULL == mask_line) {
        GT_LOGE(GT_LOG_TAG_GUI, "mask_line malloc is failed!!! size = %d", tmp_font.info.size << 1);
        goto mask_handler_lb;
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
            goto txt_handler_lb;
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
    uint8_t style_space_y = _get_style_space_y((const gt_font_info_st*)&tmp_font.info);
    uint8_t space_x = _get_style_space_x(dsc->space_x, (const gt_font_info_st *)&tmp_font.info);
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
    gt_font_touch_word_st tmp_word = {0};
    gt_font_lan_et prev_sty = FONT_LAN_UNKNOWN;
    gt_font_lan_et cur_sty = FONT_LAN_UNKNOWN;
#endif
    gt_size_t string_total_width = 0, view_width = coords->w;
    uint32_t uni_or_gbk = 0, remark_idx = 0, lan_len = 0;
    uint16_t idx = 0, idx_len = 0, txt_len = dsc->font->len;
    uint8_t over_length = 0;
    _omit_info_st omit_info = {0};
    bool start_x_flag = dsc->reg.enabled_start;
    bool is_omit_display = false;
    _gt_font_dot_ret_st dot_ret = {0};
    uint8_t width, byte_width, font_gray;

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
            ++idx_len;
            if (dsc->reg.omit_line && view_width >= 0) {
                ret.size_show_limit.x = string_total_width;
                is_omit_display = false;
            }
            continue;
        }
        tmp_font.info.size = dsc->font->info.size;
        tmp_font.utf8 = (char * )&txt[idx];
        tmp_font.len = gt_font_one_char_code_len_get(&txt[idx], &uni_or_gbk, tmp_font.info.encoding);
        width = gt_font_get_one_word_width(uni_or_gbk, &tmp_font);
        if (0 == tmp_font.len) {
            ++idx;
            continue;
        }

        if (dsc->reg.omit_line) {
            view_width -= width;
            if (idx + tmp_font.len >= txt_len) {
                /** all display string */
                if (view_width >= 0) {
                    ret.size_show_limit.x = string_total_width + width;
                }
                is_omit_display = false;
            }
            else if ( view_width >= omit_info.width) {
                remark_idx = idx + tmp_font.len;
                /** Over display string widget's width */
                ret.size_show_limit.x = string_total_width + width + omit_info.width;
                is_omit_display = true;
            } else if (view_width <= 0) {
                if (tmp_font.len) {
                    string_total_width += width;
                }
                break;
            }
        }
        if (tmp_font.len) {
            string_total_width += width;
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

        gt_memset(tmp_font.res, 0, data_len);
        tmp_font.utf8 = (char * )&txt[idx];
        tmp_font.len = gt_font_one_char_code_len_get(&txt[idx], &uni_or_gbk, tmp_font.info.encoding);
        if (0x0A == uni_or_gbk) {
            ++idx;
            continue;
        }
        width = gt_font_get_one_word_width(uni_or_gbk, &tmp_font);

        if (_is_align_reverse(dsc->align)) {
            area_font.x -= width;
        }

        if (area_font.x + width < view->x || area_font.x > view->x + view->w) {
            goto next_word_lb;
        }
        if (dsc->opa < GT_OPA_MIN) {
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

            if (GT_FONT_TYPE_FLAG_EMOJI == dot_ret.type) {
                gt_area_st emoji_area = area_font;
                emoji_area.h = 1;

                _gt_img_dsc_st emoji_raw = {0};
                emoji_raw.img = (uint8_t*)tmp_font.res;
                emoji_raw.alpha = (uint8_t*)mask_line;
                emoji_raw.header.w = width;
                emoji_raw.header.h = 1;
                emoji_raw.header.type = GT_IMG_DECODER_TYPE_RAM;
                emoji_raw.header.color_format = GT_IMG_CF_TRUE_COLOR;

                gt_attr_rect_st dsc_emoji = {0};
                dsc_emoji.bg_opa = dsc->opa;
                dsc_emoji.raw_img = &emoji_raw;
                dsc_emoji.limit_area = (gt_area_st*)view;

                uint8_t emoji_h = gt_font_get_one_word_height(uni_or_gbk, &tmp_font);
                emoji_area.y += ((tmp_font.info.size - emoji_h) >> 1);

                for(uint8_t i = 0; i < emoji_h; ++i){

                    int emj_ret = gt_font_get_emoji_dot(uni_or_gbk, dot_ret.option, emoji_raw.img, emoji_raw.alpha, i);
                    if(emj_ret <= 0) {
                        GT_LOGD(GT_LOG_TAG_GUI, "get emoji dot failed!!! ret: %d",emj_ret);
                        break;
                    }
                    draw_bg_img(draw_ctx, &dsc_emoji, &emoji_area);
                    ++emoji_area.y;
                }
                gt_memset(mask_line, 0xFF, tmp_font.info.size << 1);
            }
            else{

                byte_width = (width + 7) >> 3;
                font_gray = 1;

                byte_width = _gt_gray_and_dot_byte_get(&font_gray, width, tmp_font, dot_ret.type, uni_or_gbk);
                area_font.w = (byte_width / font_gray) << 3;
                if (FONT_LAN_ASCII == gt_font_lan_get(uni_or_gbk, tmp_font.info.encoding)) {
                    area_font.y += offset_y;
                }
                if(tmp_font.info.style.reg.bold){
                    area_font.w += _get_style_bold_offset_x((const gt_font_info_st*)&tmp_font.info);
                }

                gt_point_st off_point = gt_font_offset_get(dot_ret.option, uni_or_gbk);
                area_font.x += off_point.x;
                area_font.y += off_point.y;
                gt_draw_blend_text(draw_ctx, &blend_dsc, tmp_font.info.size,
                                    byte_width, font_gray, (const uint8_t*)tmp_font.res,tmp_font.info.style.reg);

                area_font.x -= off_point.x;
                area_font.y -= off_point.y;

                if (FONT_LAN_ASCII == gt_font_lan_get(uni_or_gbk, tmp_font.info.encoding)) {
                    area_font.y -= offset_y;
                }
            }
        } else {
            uint8_t symbol_size = gt_symbol_get_size(tmp_font.info.size);
            if (symbol_size != tmp_font.info.size) {
                gt_memset(tmp_font.res, 0, data_len);
            }
            blend_dsc.mask_buf = (uint8_t*)gt_symbol_get_mask_buf(uni_or_gbk, symbol_size);
            if (NULL == blend_dsc.mask_buf) {
                area_font.x += width + space_x;
                idx += tmp_font.len;
                continue;
            }
            gt_size_t off = (tmp_font.info.size - symbol_size) >> 1;
            area_font.y += off;
            area_font.w = symbol_size;
            area_font.h = symbol_size;
            gt_draw_blend(draw_ctx, &blend_dsc);
            area_font.h = font_disp_h;
            area_font.y -= off;
            blend_dsc.mask_buf = NULL;
        }

        /* use style */
        if (tmp_font.info.style.reg.underline && font_disp_h == tmp_font.info.size) {
            _draw_blend_underline(draw_ctx, dsc, &blend_dsc_style, area_font,
                                    (const gt_font_info_st*)&tmp_font.info, width, style_space_y);
        }
        if (tmp_font.info.style.reg.strikethrough && (font_disp_h > ((tmp_font.info.size >> 1) + (style_space_y << 1)))) {
            _draw_blend_strikethrough(draw_ctx, dsc, &blend_dsc_style, area_font,
                                        (const gt_font_info_st*)&tmp_font.info, width, style_space_y);
        }
next_word_lb:
        if (_is_align_reverse(dsc->align)) {
            area_font.x -= space_x;
        } else {
            area_font.x += width + space_x;
        }
        idx += tmp_font.len;
    }

    if (dsc->reg.omit_line && is_omit_display) {
        // Display "..." or "。。。"
        tmp_font.utf8 = omit_info.utf8;
        tmp_font.len = gt_font_one_char_code_len_get((uint8_t * )omit_info.utf8, &uni_or_gbk, tmp_font.info.encoding);
        width = gt_font_get_one_word_width(uni_or_gbk, &tmp_font);

        tmp_font.info.size = dsc->font->info.size;
        dot_ret = gt_font_get_dot(&tmp_font, uni_or_gbk);
        byte_width = (width + 7) >> 3;
        font_gray = 1;

        byte_width = _gt_gray_and_dot_byte_get(&font_gray, width, tmp_font, dot_ret.type, uni_or_gbk);
        area_font.w = (byte_width / font_gray) << 3;
        idx = 0;
        while (idx < omit_info.len)  {
            if(tmp_font.info.style.reg.bold){
                area_font.w += _get_style_bold_offset_x((const gt_font_info_st*)&tmp_font.info);
            }

            gt_point_st off_point = gt_font_offset_get(dot_ret.option, uni_or_gbk);
            area_font.x += off_point.x;
            area_font.y += off_point.y;
            gt_draw_blend_text(draw_ctx, &blend_dsc, tmp_font.info.size, byte_width,
                                font_gray, (const uint8_t*)tmp_font.res,tmp_font.info.style.reg);

            area_font.x -= off_point.x;
            area_font.y += off_point.y;

            if (_is_align_reverse(dsc->align)) {
                area_font.x -= space_x;
            } else {
                area_font.x += width + space_x;
            }
            idx += tmp_font.len;
        }
    }

    area_font.w = tmp_font.info.size;
    area_font.h = tmp_font.info.size;

    if(tmp_font.info.style.reg.italic){
        area_font.x += tmp_font.info.size >> 1;
    }

ret_handler_lb:
#if _GT_FONT_ENABLE_CONVERTOR
    if (NULL != bidi) {
        gt_mem_free(bidi);
        bidi = NULL;
    }
bidi_handler_lb:
    if (NULL != ret_txt) {
        gt_mem_free(ret_txt);
        ret_txt = NULL;
    }
#endif
txt_handler_lb:
    if (
#if GT_USE_EXTRA_FULL_FONT_MASK_BUFFER
        false == is_extend_font_mask_valid &&
#endif
        mask_line) {
        gt_mem_free(mask_line);
    }
    mask_line = NULL;
mask_handler_lb:
    if (
#if GT_USE_EXTRA_FULL_FONT_BUFFER
        false == is_extend_font_valid &&
#endif
        tmp_font.res) {
        gt_mem_free(tmp_font.res);
    }
    tmp_font.res = NULL;
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

static void GT_ATTRIBUTE_RAM_TEXT draw_bg_fill(_gt_draw_ctx_st * draw_ctx, const gt_attr_rect_st * dsc, const gt_area_st * coords) {
    if(!dsc->reg.is_fill) return ;

    gt_draw_blend_dsc_st blend_dsc = {0};
    blend_dsc.color_fill = dsc->bg_color;
    blend_dsc.view_area = dsc->limit_area;
    blend_dsc.opa = dsc->bg_opa;
    blend_dsc.mask_buf = NULL;

    uint16_t border_radius = GT_MIN(dsc->radius, (GT_MIN(coords->w >> 1, coords->h >> 1)));

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
    GT_CHECK_BACK(mask_line_buf);
    int16_t bg_mask_id = -1;
    gt_mask_radius_st bg_mask_radius = {0};

    if(bg_radius > 0){
        gt_mask_radius_init(&bg_mask_radius, &bg_area, bg_radius, false);
        bg_mask_id = gt_mask_add(&bg_mask_radius);
    }

    int16_t cl_mask_id = -1;
    uint16_t cl_radius = 0;
    gt_mask_radius_st cl_mask_radius = {0};
    if (dsc->base_area){
        cl_radius = GT_MIN(dsc->radius, (GT_MIN(dsc->base_area->w >> 1, dsc->base_area->h >> 1)));
        if(cl_radius > 0){
            gt_mask_radius_init(&cl_mask_radius, dsc->base_area, cl_radius, false);
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
    blend_dsc.mask_buf = mask_line_buf;

    uint16_t i = 0;
    for(i = 0; i < bg_area.h; ++i){
        line_area.y = bg_area.y + i;
        gt_memset(mask_line_buf, mask_opa, line_area.w);
        gt_mask_get(mask_line_buf, line_area.x, line_area.y, line_area.w);
        gt_draw_blend(draw_ctx, &blend_dsc);
    }

    gt_mem_free(mask_line_buf);
    mask_line_buf = NULL;
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

static GT_ATTRIBUTE_RAM_TEXT void draw_bg_border(_gt_draw_ctx_st * draw_ctx, const gt_attr_rect_st * dsc, const gt_area_st * coords) {
    if(dsc->border_width <= 0) return ;

    gt_draw_blend_dsc_st blend_dsc = {0};
    blend_dsc.view_area = dsc->limit_area;
    blend_dsc.opa = dsc->bg_opa;
    blend_dsc.mask_buf = NULL;
    blend_dsc.color_fill = dsc->border_color;

    gt_area_st border_area = {0};
    gt_area_copy(&border_area, coords);
    uint16_t border_radius = GT_MIN(dsc->radius, (GT_MIN(border_area.w >> 1, border_area.h >> 1)));

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
    GT_CHECK_BACK(mask_line_buf);

    gt_area_st in_area = {0};
    in_area = gt_area_reduce(border_area, dsc->border_width);
    uint16_t in_radius = (border_radius < dsc->border_width) ? 0 : border_radius - dsc->border_width;
    in_radius = GT_MIN(in_radius, (GT_MIN(in_area.w >> 1, in_area.h >> 1)));

    int16_t in_mask_id = -1;
    gt_mask_radius_st in_mask_radius = {0};
    gt_mask_radius_init(&in_mask_radius, &in_area, in_radius, true);
    in_mask_id = gt_mask_add(&in_mask_radius);

    int16_t border_mask_id = -1;
    gt_mask_radius_st border_mask_radius = {0};
    gt_mask_radius_init(&border_mask_radius, &border_area, border_radius, false);
    border_mask_id = gt_mask_add(&border_mask_radius);

    gt_opa_t mask_opa = dsc->bg_opa >= GT_OPA_MAX ? GT_OPA_COVER : dsc->bg_opa;

    line_area.x = border_area.x;
    line_area.w = border_area.w;
    line_area.h = 1;
    blend_dsc.dst_area = &line_area;
    blend_dsc.mask_area = &line_area;
    blend_dsc.mask_buf = mask_line_buf;

    uint16_t i = 0;
    for(i = 0; i < border_area.h; ++i){
        line_area.y = border_area.y + i;
        gt_memset(mask_line_buf, mask_opa, line_area.w);
        gt_mask_get(mask_line_buf, line_area.x, line_area.y, line_area.w);
        gt_draw_blend(draw_ctx, &blend_dsc);
    }

    gt_mem_free(mask_line_buf);
    mask_line_buf = NULL;
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

    if(dsc->gray != 0){
        draw_dot_matrix(draw_ctx, dsc, coords);
        return ;
    }

    draw_bg_fill(draw_ctx, dsc, coords);
    draw_bg_border(draw_ctx, dsc, coords);
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
            } else { /** MODE 1 */
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
            gt_mask_radius_init(&r_mask, &mask_area, blend_dsc.masking->radius, false);
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
        gt_mask_radius_init(&r_mask, &area_line, rd_val, false);
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

GT_ATTRIBUTE_RAM_TEXT void gt_draw_line(_gt_draw_ctx_st * draw_ctx, gt_attr_line_st const * dsc, const gt_area_st * coords)
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

    if(dsc->line.width <= 0) { return ;}

    draw_ctx->parent_area = (gt_area_st * )coords;  /** remark before parent area value, return will reload */

    uint16_t rd_val = dsc->line.width;
    uint16_t radius = rd_val >> 1;
    gt_area_st area_line = { coords->x , coords->y , coords->w, 1 };
    gt_area_st dst_area = { .w = rd_val, .h = rd_val };

    bool is_ver = dsc->start.x == dsc->end.x;
    bool is_hor = dsc->start.y == dsc->end.y;
    bool is_round = _is_brush_round(&dsc->line) && (radius > 0) ? true : false;
    bool is_straight_line = ( is_ver || is_hor ) ? true : false;

    if (is_straight_line) {
        if (is_ver) {
            dst_area.x = dsc->start.x - radius;
        } else {
            dst_area.x = GT_MIN(dsc->start.x, dsc->end.x);
            dst_area.w = GT_MAX(dsc->start.x, dsc->end.x) - dst_area.x;
        }

        if (is_hor) {
            dst_area.y = dsc->start.y - radius;
        } else {
            dst_area.y = GT_MIN(dsc->start.y, dsc->end.y);
            dst_area.h = GT_MAX(dsc->start.y, dsc->end.y) - dst_area.y;
        }
        if (is_round) {
            if (is_ver) {
                dst_area.y -= radius;
                dst_area.h += rd_val;
            }
            if (is_hor) {
                dst_area.x -= radius;
                dst_area.w += rd_val;
            }
        }
    }

    gt_draw_blend_dsc_st blend_dsc = {
        .dst_area = &area_line,
        .mask_area = &area_line,
        .color_fill = dsc->line.color,
        .opa = dsc->line.opa,
        .mask_buf =  NULL,
    };

    // straight line and not round
    if(is_straight_line && !is_round) {
        blend_dsc.dst_area = &dst_area;
        gt_draw_blend(draw_ctx, &blend_dsc);
        return ;
    }

    blend_dsc.mask_buf = gt_mem_malloc(area_line.w);
    if(!blend_dsc.mask_buf) {
        return ;
    }

    // straight line and round
    if(is_straight_line) {
        gt_mask_radius_st r_mask = {0};
        gt_size_t r_mask_id = -1;

        if(is_round) {
            gt_mask_radius_init(&r_mask, &dst_area, radius, false);
            r_mask_id = gt_mask_add(&r_mask);
        }

        for(uint16_t i = 0; i < dst_area.h; ++i) {
            area_line.y = dst_area.y + i;
            gt_memset(blend_dsc.mask_buf, blend_dsc.opa, area_line.w);
            gt_mask_get(blend_dsc.mask_buf, area_line.x, area_line.y, area_line.w);
            gt_draw_blend(draw_ctx, &blend_dsc);
        }

        if(is_round) {
            gt_mask_remove_idx(r_mask_id);
            gt_mask_free(&r_mask);
        }
    } else {
        if (is_round) {
            gt_area_st round_area = { dsc->start.x - radius, dsc->start.y - radius, rd_val, rd_val };
            area_line.x = round_area.x;
            area_line.y = round_area.y;
            area_line.w = round_area.w;
            area_line.h = 1;

            gt_mask_radius_st r_mask = {0};
            gt_size_t r_mask_id = -1;
            gt_mask_radius_init(&r_mask, &round_area, radius, false);
            r_mask_id = gt_mask_add(&r_mask);

            for (uint16_t i = 0; i < round_area.h; ++i) {
                area_line.y = round_area.y + i;
                gt_memset(blend_dsc.mask_buf, blend_dsc.opa, area_line.w);
                gt_mask_get(blend_dsc.mask_buf, area_line.x, area_line.y, area_line.w);
                gt_draw_blend(draw_ctx, &blend_dsc);
            }

            round_area.x = dsc->end.x - radius;
            round_area.y = dsc->end.y - radius;
            r_mask.area.x = round_area.x;
            r_mask.area.y = round_area.y;
            area_line.x = round_area.x;
            area_line.y = round_area.y;

            for (uint16_t i = 0; i < round_area.h; ++i) {
                area_line.y = round_area.y + i;
                gt_memset(blend_dsc.mask_buf, blend_dsc.opa, area_line.w);
                gt_mask_get(blend_dsc.mask_buf, area_line.x, area_line.y, area_line.w);
                gt_draw_blend(draw_ctx, &blend_dsc);
            }

            gt_mask_remove_idx(r_mask_id);
            gt_mask_free(&r_mask);
        }

        gt_point_st ll_start = {0};
        gt_point_st ll_end = {0};
        gt_point_st rl_start = {0};
        gt_point_st rl_end = {0};
        //
        gt_math_line_st x_line = {0};
        gt_math_circle_st circle = {0};
        gt_point_f_st p1 = {0}, p2 = {0};
        circle.center.x = (gt_float_t)(dsc->start.x);
        circle.center.y = (gt_float_t)(dsc->start.y);
        circle.radius = (gt_float_t)radius;

        p1.x = (gt_float_t)dsc->start.x;
        p1.y = (gt_float_t)dsc->start.y;
        p2.x = (gt_float_t)dsc->end.x;
        p2.y = (gt_float_t)dsc->end.y;

        gt_math_line_get_perpendicular(&x_line, &circle.center, &p1, &p2);
        gt_math_line_circle_intersect(&p1, &p2, &x_line, &circle);

        if(p1.x > p2.x) {
            ll_start.x = (gt_size_t)gt_round(p1.x);
            ll_start.y = (gt_size_t)gt_round(p1.y);
            rl_start.x = (gt_size_t)gt_round(p2.x);
            rl_start.y = (gt_size_t)gt_round(p2.y);
        } else {
            ll_start.x = (gt_size_t)gt_round(p2.x);
            ll_start.y = (gt_size_t)gt_round(p2.y);
            rl_start.x = (gt_size_t)gt_round(p1.x);
            rl_start.y = (gt_size_t)gt_round(p1.y);
        }

        circle.center.x = (gt_float_t)(dsc->end.x);
        circle.center.y = (gt_float_t)(dsc->end.y);
        p1.x = (gt_float_t)dsc->start.x;
        p1.y = (gt_float_t)dsc->start.y;
        p2.x = (gt_float_t)dsc->end.x;
        p2.y = (gt_float_t)dsc->end.y;
        gt_math_line_get_perpendicular(&x_line, &circle.center, &p1, &p2);
        gt_math_line_circle_intersect(&p1, &p2, &x_line, &circle);

        if(p1.x > p2.x) {
            ll_end.x = (gt_size_t)gt_round(p1.x);
            ll_end.y = (gt_size_t)gt_round(p1.y);
            rl_end.x = (gt_size_t)gt_round(p2.x);
            rl_end.y = (gt_size_t)gt_round(p2.y);
        } else {
            ll_end.x = (gt_size_t)gt_round(p2.x);
            ll_end.y = (gt_size_t)gt_round(p2.y);
            rl_end.x = (gt_size_t)gt_round(p1.x);
            rl_end.y = (gt_size_t)gt_round(p1.y);
        }

        gt_point_st m1, m2;
        m1.x = GT_MIN(ll_start.x, ll_end.x);
        m2.x = GT_MIN(rl_start.x, rl_end.x);
        dst_area.x = GT_MIN(m1.x, m2.x);

        m1.y = GT_MIN(ll_start.y, ll_end.y);
        m2.y = GT_MIN(rl_start.y, rl_end.y);
        dst_area.y = GT_MIN(m1.y, m2.y);

        m1.x = GT_MAX(ll_start.x, ll_end.x);
        m2.x = GT_MAX(rl_start.x, rl_end.x);
        dst_area.w = GT_MAX(m1.x, m2.x) - dst_area.x;

        m1.y = GT_MAX(ll_start.y, ll_end.y);
        m2.y = GT_MAX(rl_start.y, rl_end.y);
        dst_area.h = GT_MAX(m1.y, m2.y) - dst_area.y;

        area_line.x = dst_area.x;
        area_line.y = dst_area.y;
        area_line.w = dst_area.w;
        area_line.h = 1;

        gt_mask_line_st ll_mask = {0};
        gt_mask_line_st rl_mask = {0};
        gt_size_t ll_make_id = -1;
        gt_size_t rl_make_id = -1;

        gt_mask_line_points_init(&ll_mask, ll_start.x, ll_start.y, ll_end.x, ll_end.y, GT_MASK_LINE_SIDE_LEFT);
        ll_make_id = gt_mask_add(&ll_mask);
        gt_mask_line_points_init(&rl_mask, rl_start.x, rl_start.y, rl_end.x, rl_end.y, GT_MASK_LINE_SIDE_RIGHT);
        rl_make_id = gt_mask_add(&rl_mask);

        gt_mask_line_st ss_mask = {0};
        gt_size_t ss_make_id = -1;
        gt_mask_line_st ee_mask = {0};
        gt_size_t ee_make_id = -1;
        if (!is_round) {
            gt_mask_line_side_et ss_side, ee_side;
            if (ll_start.x > rl_start.x && ll_start.y > rl_start.y) {
                if (dsc->start.y < dsc->end.y) {
                    ss_side = GT_MASK_LINE_SIDE_LEFT;
                    ee_side = GT_MASK_LINE_SIDE_RIGHT;
                } else {
                    ss_side = GT_MASK_LINE_SIDE_RIGHT;
                    ee_side = GT_MASK_LINE_SIDE_LEFT;
                }
            }
            else if (ll_start.x > rl_start.x && ll_start.y < rl_start.y) {
                if (dsc->start.y < dsc->end.y) {
                    ss_side = GT_MASK_LINE_SIDE_RIGHT;
                    ee_side = GT_MASK_LINE_SIDE_LEFT;
                } else {
                    ss_side = GT_MASK_LINE_SIDE_LEFT;
                    ee_side = GT_MASK_LINE_SIDE_RIGHT;
                }
            }

            gt_mask_line_points_init(&ss_mask, ll_start.x, ll_start.y, rl_start.x, rl_start.y, ss_side);
            gt_mask_line_points_init(&ee_mask, ll_end.x, ll_end.y, rl_end.x, rl_end.y, ee_side);
            ss_make_id = gt_mask_add(&ss_mask);
            ee_make_id = gt_mask_add(&ee_mask);
        }

        for (uint16_t i = 0; i < dst_area.h; ++i) {
            area_line.y = dst_area.y + i;
            gt_memset(blend_dsc.mask_buf, blend_dsc.opa, area_line.w);
            gt_mask_get(blend_dsc.mask_buf, area_line.x, area_line.y, area_line.w);
            gt_draw_blend(draw_ctx, &blend_dsc);
        }

        gt_mask_remove_idx(ll_make_id);
        gt_mask_remove_idx(rl_make_id);
        gt_mask_free(&ll_mask);
        gt_mask_free(&rl_mask);

        if (!is_round) {
            gt_mask_remove_idx(ss_make_id);
            gt_mask_remove_idx(ee_make_id);
            gt_mask_free(&ss_mask);
            gt_mask_free(&ee_mask);
        }
    }
    gt_mem_free(blend_dsc.mask_buf);
}

GT_ATTRIBUTE_RAM_TEXT void gt_draw_catmullrom(_gt_draw_ctx_st * draw_ctx, gt_attr_curve_st const * dsc, const gt_area_st * coords)
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
        gt_mask_radius_init(&r_mask, &area_line, rd_val, false);
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

GT_ATTRIBUTE_RAM_TEXT void gt_draw_arc(_gt_draw_ctx_st * draw_ctx, gt_attr_arc_st const * dsc, const gt_area_st * coords)
{

    if(dsc->opa <= GT_OPA_MIN) { return ;}
    if(dsc->width == 0) { return ;}
    if(dsc->start_angle == dsc->end_angle) { return ;}

    uint16_t start_angle = dsc->start_angle;
    uint16_t end_angle = dsc->end_angle;

    gt_area_st area_out;
    gt_area_copy(&area_out, coords);
    uint16_t out_radius = GT_MIN(area_out.h, area_out.w) >> 1;
    uint16_t width = GT_MIN(dsc->width, out_radius);

    int16_t out_mask_id = -1;
    gt_mask_radius_st out_mask = {0};
    gt_mask_radius_init(&out_mask, &area_out, out_radius, false);
    out_mask_id = gt_mask_add(&out_mask);

    gt_area_st area_in;
    area_in = gt_area_reduce(area_out, width);

    uint16_t in_radius = GT_MIN(area_in.h, area_in.w) >> 1;
    int16_t in_mask_id = -1;
    gt_mask_radius_st in_mask = {0};
    if(in_radius > 0){
        gt_mask_radius_init(&in_mask, &area_in, in_radius, true);
        in_mask_id = gt_mask_add(&in_mask);
    }

    bool is_img = false;
    gt_attr_rect_st rect_attr;
    gt_graph_init_rect_attr(&rect_attr);
    if( dsc->img
#if GT_USE_FILE_HEADER
        || dsc->file_header
#endif

#if GT_USE_DIRECT_ADDR
        || !gt_hal_is_invalid_addr(dsc->addr)
#endif

#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
        || !gt_hal_is_invalid_custom_size_addr(dsc->custom_addr)
#endif
    ){
        is_img = true;
        rect_attr.bg_img_opa = dsc->opa;
        rect_attr.bg_img_src = dsc->img;

#if GT_USE_FILE_HEADER
        rect_attr.file_header = dsc->file_header;
#endif
#if GT_USE_DIRECT_ADDR
        rect_attr.addr = dsc->addr;
#endif
#if GT_USE_DIRECT_ADDR_CUSTOM_SIZE
        rect_attr.custom_addr = dsc->custom_addr;
#endif
    }
    else{
        rect_attr.bg_opa = dsc->opa;
        rect_attr.bg_color = dsc->color;
        rect_attr.reg.is_fill = 1;
    }

    if(start_angle + 360 == end_angle || start_angle == end_angle + 360){
        if(is_img){
            draw_bg_img(draw_ctx, &rect_attr, &area_out);
        }
        else{
            draw_bg(draw_ctx, &rect_attr, &area_out);
        }

        gt_mask_remove_idx(out_mask_id);
        gt_mask_free(&out_mask);

        if(in_radius > 0){
            gt_mask_remove_idx(in_mask_id);
            gt_mask_free(&in_mask);
        }
        return ;
    }

    while(start_angle >= 360) { start_angle -= 360; }
    while(end_angle >= 360) { end_angle -= 360; }

    gt_point_st center;
    center.x = area_out.x + out_radius;
    center.y = area_out.y + out_radius;

    gt_mask_angle_st angle_mask = {0};
    gt_mask_angle_init(&angle_mask, center.x, center.y, start_angle, end_angle);
    int16_t angle_mask_id = gt_mask_add(&angle_mask);

    int32_t angle_gap;
    if(end_angle > start_angle) {
        angle_gap = 360 - (end_angle - start_angle);
    }
    else {
        angle_gap = start_angle - end_angle;
    }

    if(angle_gap > 60 && out_radius > 10){

        if(is_img){
            draw_bg_img(draw_ctx, &rect_attr, &area_out);
        }
        else {
            _quarter_circle_st q;
            q.coords = &area_out;
            q.center = &center;
            q.radius = out_radius;
            q.width = width;
            q.start_angle = start_angle;
            q.end_angle = end_angle;
            q.start_quarter = (start_angle / 90) & 0x03;
            q.end_quarter = (end_angle / 90) & 0x03;
            q.rect_dsc = &rect_attr;

            _draw_quarter_circle_0(draw_ctx, &q);
            _draw_quarter_circle_1(draw_ctx, &q);
            _draw_quarter_circle_2(draw_ctx, &q);
            _draw_quarter_circle_3(draw_ctx, &q);
        }
    }
    else {
        if(is_img){
            draw_bg_img(draw_ctx, &rect_attr, &area_out);
        }
        else{
            draw_bg(draw_ctx, &rect_attr, &area_out);
        }
    }

    gt_mask_remove_idx(angle_mask_id);
    gt_mask_free(&angle_mask);

    gt_mask_remove_idx(out_mask_id);
    gt_mask_free(&out_mask);

    if(in_radius > 0){
        gt_mask_remove_idx(in_mask_id);
        gt_mask_free(&in_mask);
    }

    if(is_img || !dsc->rounded) { return ; }

    gt_area_st round_area;
    gt_rounded_area_get(start_angle, out_radius, width, &round_area);
    round_area.x += center.x;
    round_area.y += center.y;
    rect_attr.radius = round_area.w; // * Make it draw a circle
    draw_bg(draw_ctx, &rect_attr, &round_area);

    gt_rounded_area_get(end_angle, out_radius, width, &round_area);
    round_area.x += center.x;
    round_area.y += center.y;
    rect_attr.radius = round_area.w; // * Make it draw a circle
    draw_bg(draw_ctx, &rect_attr, &round_area);

}
/* end ------------------------------------------------------------------*/
