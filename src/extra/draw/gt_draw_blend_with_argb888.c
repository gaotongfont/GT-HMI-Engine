/**
 * @file gt_draw_blend_with_argb888.c
 * @author Feyoung
 * @brief
 * @version 0.1
 * @date 2024-06-04 21:31:21
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */

/* include --------------------------------------------------------------*/
#include "./gt_draw_blend_with_argb888.h"



/* private define -------------------------------------------------------*/



/* private typedef ------------------------------------------------------*/



/* static variables -----------------------------------------------------*/



/* macros ---------------------------------------------------------------*/



/* class ----------------------------------------------------------------*/



/* static functions -----------------------------------------------------*/
/**
 * Mix two colors. Both color can have alpha value.
 * @param bg_color background color
 * @param bg_opa alpha of the background color
 * @param fg_color foreground color
 * @param fg_opa alpha of the foreground color
 * @param res_color the result color
 * @param res_opa the result opacity
 */
static inline void gt_color_mix_with_alpha(gt_color_t bg_color, gt_opa_t bg_opa,
                                            gt_color_t fg_color, gt_opa_t fg_opa,
                                            gt_color_t * res_color, gt_opa_t * res_opa)
{
    /*Pick the foreground if it's fully opaque or the Background is fully transparent*/
    if(fg_opa >= GT_OPA_MAX || bg_opa <= GT_OPA_MIN) {
        res_color->full = fg_color.full;
        *res_opa = fg_opa;
        return ;
    }
    /*Transparent foreground: use the Background*/
    if(fg_opa <= GT_OPA_MIN) {
        res_color->full = bg_color.full;
        *res_opa = bg_opa;
        return ;
    }
    /*Opaque background: use simple mix*/
    if(bg_opa >= GT_OPA_MAX) {
        *res_color = gt_color_mix(fg_color, bg_color, fg_opa);
        *res_opa = GT_OPA_COVER;
        return;
    }

    /*Both colors have alpha. Expensive calculation need to be applied*/
    /*Save the parameters and the result. If they will be asked again don't compute again*/
    static gt_opa_t fg_opa_save       = 0;
    static gt_opa_t bg_opa_save       = 0;
    static gt_color_t fg_color_save   = _GT_COLOR_ZERO_INITIALIZER;
    static gt_color_t bg_color_save   = _GT_COLOR_ZERO_INITIALIZER;
    static gt_color_t res_color_saved = _GT_COLOR_ZERO_INITIALIZER;
    static gt_opa_t res_opa_saved     = 0;

    if (fg_opa != fg_opa_save || bg_opa != bg_opa_save ||
        fg_color.full != fg_color_save.full ||
        bg_color.full != bg_color_save.full) {
        fg_opa_save        = fg_opa;
        bg_opa_save        = bg_opa;
        fg_color_save.full = fg_color.full;
        bg_color_save.full = bg_color.full;
        /** Info: https://en.wikipedia.org/wiki/Alpha_compositing#Analytical_derivation_of_the_over_operator */
        res_opa_saved = 255 - ((uint16_t)((uint16_t)(255 - fg_opa) * (255 - bg_opa)) >> 8);
        GT_ASSERT(res_opa_saved != 0);
        gt_opa_t ratio = (uint16_t)((uint16_t)fg_opa * 255) / res_opa_saved;
        res_color_saved = gt_color_mix(fg_color, bg_color, ratio);
    }

    res_color->full = res_color_saved.full;
    *res_opa = res_opa_saved;
}


#if GT_FLUSH_CONVERT_VERTICAL
static inline void _blend_rgb888_with_opacity_vertical(_gt_draw_blend_fill_cache_st const * const fc) {

}
#else   /** 0 == GT_FLUSH_CONVERT_VERTICAL */
static inline void _blend_rgb888_with_opacity(_gt_draw_blend_fill_cache_st const * const fc) {

}
#endif  /** GT_FLUSH_CONVERT_VERTICAL */


#if GT_FLUSH_CONVERT_VERTICAL
static inline void _blend_rgb888_with_no_opacity_vertical(_gt_draw_blend_fill_cache_st const * const fc) {

}
#else   /** 0 == GT_FLUSH_CONVERT_VERTICAL */
static inline void _blend_rgb888_with_no_opacity(_gt_draw_blend_fill_cache_st const * const fc) {

}
#endif  /** GT_FLUSH_CONVERT_VERTICAL */

#if GT_FLUSH_CONVERT_VERTICAL
static inline void _blend_argb888_with_opacity_vertical(_gt_draw_blend_fill_cache_st const * const fc) {

}
#else   /** 0 == GT_FLUSH_CONVERT_VERTICAL */
static inline void _blend_argb888_with_opacity(_gt_draw_blend_fill_cache_st const * const fc) {

}
#endif  /** GT_FLUSH_CONVERT_VERTICAL */

#if GT_FLUSH_CONVERT_VERTICAL
static inline void _blend_argb888_with_no_opacity_vertical(_gt_draw_blend_fill_cache_st const * const fc) {

}
#else   /** 0 == GT_FLUSH_CONVERT_VERTICAL */
static inline void _blend_argb888_with_no_opacity(_gt_draw_blend_fill_cache_st const * const fc) {

}
#endif  /** GT_FLUSH_CONVERT_VERTICAL */

/* global functions / API interface -------------------------------------*/
void gt_draw_blend_with_rgb888_opacity(_gt_draw_blend_fill_cache_st const * const fc)
{
#if GT_FLUSH_CONVERT_VERTICAL
    return _blend_rgb888_with_opacity_vertical(fc);
#else
    return _blend_rgb888_with_opacity(fc);
#endif
}

void gt_draw_blend_with_rgb888_no_opacity(_gt_draw_blend_fill_cache_st const * const fc)
{
#if GT_FLUSH_CONVERT_VERTICAL
    return _blend_rgb888_with_no_opacity_vertical(fc);
#else
    return _blend_rgb888_with_no_opacity(fc);
#endif
}

void gt_draw_blend_with_argb888_opacity(_gt_draw_blend_fill_cache_st const * const fc)
{
#if GT_FLUSH_CONVERT_VERTICAL
    return _blend_argb888_with_opacity_vertical(fc);
#else
    return _blend_argb888_with_opacity(fc);
#endif
}

void gt_draw_blend_with_argb888_no_opacity(_gt_draw_blend_fill_cache_st const * const fc)
{
#if GT_FLUSH_CONVERT_VERTICAL
    return _blend_argb888_with_no_opacity_vertical(fc);
#else
    return _blend_argb888_with_no_opacity(fc);
#endif
}

/* end ------------------------------------------------------------------*/
