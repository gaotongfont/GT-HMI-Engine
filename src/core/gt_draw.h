/**
 * @file gt_draw.h
 * @author yongg
 * @brief Interface functions for drawing text and images
 * @version 0.1
 * @date 2022-08-09 18:29:16
 * @copyright Copyright (c) 2014-2022, Company Genitop. Co., Ltd.
 */
#ifndef _GT_DRAW_H_
#define _GT_DRAW_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include --------------------------------------------------------------*/
#include "gt_graph_base.h"
#include "../font/gt_font.h"


/* define ---------------------------------------------------------------*/



/* typedef --------------------------------------------------------------*/
/**
 * @brief Draw object description
 */
typedef struct _gt_draw_ctx_t{
    void * buf;     ///< save the temp buffer
    gt_area_st * buf_area;  ///< display area

    /// draw base graph
    void (*draw_rect)(struct _gt_draw_ctx_t * ctx, gt_attr_rect_st * attr, gt_area_st * area);

    /// draw background
    void (*draw_bg)(struct _gt_draw_ctx_t * ctx, gt_attr_rect_st * attr, gt_area_st * area);

}gt_draw_ctx_t;


/* macros ---------------------------------------------------------------*/



/* global functions / API interface -------------------------------------*/

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
void draw_bg_img(gt_draw_ctx_t * draw_ctx, const gt_attr_rect_st * dsc, const gt_area_st * coords);



#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif //!_GT_DRAW_H_
