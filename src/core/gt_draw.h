/**
 * @file gt_draw.h
 * @author yongg
 * @brief Interface functions for drawing text and images
 * @version 0.1
 * @date 2022-08-09 18:29:16
 * @copyright Copyright (c) 2014-present, Company Genitop. Co., Ltd.
 */
#ifndef _GT_DRAW_H_
#define _GT_DRAW_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_graph_base.h"
#include "../font/gt_font.h"
#include "../extra/draw/gt_draw_blend.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/

/**
 * @brief The displayable area of two interfaces on the same physical screen
 */
typedef struct _gt_draw_valid_s {
#if GT_USE_SCREEN_ANIM
    gt_area_st area_clip;       ///<The clip area
#endif

    uint8_t is_hor  : 1;        ///<Toggle the effect of the screen, the direction is horizontal
    uint8_t layer_top : 1;
    uint8_t reserved: 6;
}_gt_draw_valid_st;


/**
 * @brief Draw object description
 */
typedef struct _gt_draw_ctx_s {
    _gt_draw_valid_st * valid;      ///< Only be used in the screen animation mode
    gt_area_st * parent_area;       ///< The parent area, when obj->inside is true, obj display limited to parent area

    void * buf;             ///< save the temp buffer
    gt_area_st buf_area;    ///< The display area
}_gt_draw_ctx_st;

#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
typedef struct gt_font_touch_word_s {
    char * word_p;
    uint16_t len;
}gt_font_touch_word_st;
#endif

typedef struct _gt_draw_font_res_s {
    gt_area_st area;
    gt_point_st size;   /** String total width (0xffff invalid) and height(invalid) */
#if _GT_FONT_GET_WORD_BY_TOUCH_POINT
    gt_font_touch_word_st touch_word;
#endif
}_gt_draw_font_res_st;

typedef struct _gt_radius_mask_t{
    uint8_t *buf;
    uint8_t *opa;
    uint16_t * opa_start;
    uint16_t * x_start;

    gt_area_st area;
    uint16_t radius;
    uint8_t outer : 1; ///< 0: inner, 1: outer
}gt_radius_mask_st;

typedef enum {
    R_DIR_TOP_LEFT = 0,
    R_DIR_TOP_RIGHT,
    R_DIR_BOTTOM_LEFT,
    R_DIR_BOTTOM_RIGHT
}gt_radius_dir_et;

/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/
/**
 * @brief Draw a circle
 *
 * @param draw_ctx callback function
 * @param blend_dsc draw blend dsc structures
 * @param mask_left_up circle left_up data buff
 * @param mask_right_up circle right_up data buff
 * @param coords data area
 */
void gt_draw_arch(_gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st  *blend_dsc,uint8_t *mask_left_up,uint8_t *mask_right_up,const gt_area_st * coords);

/**
 * @brief use bresenham algorithm draw a line by circle
 *
 * @param line_attr line attribute
 * @param draw_ctx callback function
 * @param blend_dsc draw blend dsc structures
 * @param mask_left_up circle left_up data buff
 * @param mask_right_up circle right_up data buff
 * @param coords data area
 */
void gt_draw_line(gt_attr_line_st * line_attr, _gt_draw_ctx_st * draw_ctx, gt_draw_blend_dsc_st * blend_dsc,uint8_t *mask_left_up,uint8_t *mask_right_up,const gt_area_st * coords);

/**
 * @brief Draw rect background
 *
 * @param draw_ctx callback function
 * @param dsc description
 * @param coords Want to display area
 */
void draw_bg(_gt_draw_ctx_st * draw_ctx, const gt_attr_rect_st * dsc, const gt_area_st * coords);

/**
 * @brief Draw text content
 *
 * @param draw_ctx callback function
 * @param dsc description
 * @param coords Want to display area
 * @return gt_area_st The display area
 */
_gt_draw_font_res_st draw_text(_gt_draw_ctx_st * draw_ctx,const gt_attr_font_st * dsc, const gt_area_st * coords);

/**
 * @brief Draw background image
 *
 * @param draw_ctx callback function
 * @param dsc description
 * @param coords Want to display area
 */
void draw_bg_img(_gt_draw_ctx_st * draw_ctx, const gt_attr_rect_st * dsc, gt_area_st * coords);

void draw_focus(gt_obj_st* obj, gt_size_t radius);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_DRAW_H_
