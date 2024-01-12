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
    gt_area_st area_prev;       ///<The prev screen absolute display area
    gt_point_st offset_prev;    ///<prev screen display in relative position on the physical screen

    gt_area_st area_scr;        ///<The current screen absolute display area
    gt_point_st offset_scr;     ///<current screen display in relative position on the physical screen

    uint8_t is_prev : 1;        ///<Whether the previous screen is valid
    uint8_t is_hor  : 1;        ///<Toggle the effect of the screen, the direction is horizontal
    uint8_t is_over_top : 1;
    uint8_t reserved: 5;
}_gt_draw_valid_st;


/**
 * @brief Draw object description
 */
typedef struct _gt_draw_ctx_t{
    _gt_draw_valid_st * valid;  ///< Only be used in the screen animation mode
    gt_area_st * parent_area; ///< The parent area, when obj->inside is true, obj display limited to parent area

    void * buf;     ///< save the temp buffer
    gt_area_st buf_area;    ///< The display area

}gt_draw_ctx_t;


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
void gt_draw_arch(gt_draw_ctx_t * draw_ctx, gt_draw_blend_dsc_st  *blend_dsc,uint8_t *mask_left_up,uint8_t *mask_right_up,const gt_area_st * coords);

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
void gt_draw_line(gt_attr_line_st * line_attr, gt_draw_ctx_t * draw_ctx, gt_draw_blend_dsc_st * blend_dsc,uint8_t *mask_left_up,uint8_t *mask_right_up,const gt_area_st * coords);

/**
 * @brief Draw rect background
 *
 * @param draw_ctx callback function
 * @param dsc description
 * @param coords Want to display area
 */
void draw_bg(gt_draw_ctx_t * draw_ctx, const gt_attr_rect_st * dsc, const gt_area_st * coords);

/**
 * @brief Draw text content
 *
 * @param draw_ctx callback function
 * @param dsc description
 * @param coords Want to display area
 * @return gt_area_st The display area
 */
gt_area_st draw_text(gt_draw_ctx_t * draw_ctx,const gt_attr_font_st * dsc, const gt_area_st * coords);

/**
 * @brief Draw background image
 *
 * @param draw_ctx callback function
 * @param dsc description
 * @param coords Want to display area
 */
void draw_bg_img(gt_draw_ctx_t * draw_ctx, const gt_attr_rect_st * dsc, gt_area_st * coords);

void draw_focus(gt_obj_st* obj , gt_size_t radius);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_DRAW_H_
